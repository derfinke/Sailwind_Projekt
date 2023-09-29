/*
 * Localization.c
 *
 *  Created on: 28.07.2023
 *      Author: Bene
 */

#include "Localization.h"

/* defines ------------------------------------------------------------*/
#define LOC_SERIAL_FORMAT_SPECMS "%d,%d,%d,%d,%d,%d" //SPEC = "State, Pulse_count, End_pos, Center_pos, movement, start_pos"

/* private function prototypes -----------------------------------------------*/
static int8_t Localization_deserialize(Localization_t *loc_ptr, char serial_buffer[LOC_SERIAL_SIZE]);
static int16_t Localization_pulse_count_to_distance(Localization_t loc);

/* API function definitions -----------------------------------------------*/
Localization_t Localization_init(float distance_per_pulse, char serial_buffer[LOC_SERIAL_SIZE])
{
	Localization_t localization = {
			.is_localized = False,
			.is_triggered = False,
			.movement = Loc_movement_stop,
			.state = Loc_state_0_init,
			.center_pos_mm = 0,
			.pulse_count = 0,
			.distance_per_pulse = distance_per_pulse
	};
	int8_t recovery_state = Localization_deserialize(&localization, serial_buffer);
	Localization_recover(&localization, recovery_state, False);
	return localization;
}

void Localization_reset(Localization_t *loc_ptr, boolean_t direct_trigger)
{
	loc_ptr->state = Loc_state_0_init;
	loc_ptr->recovery_state = LOC_RECOVERY_RESET;
	loc_ptr->is_localized = False;
	loc_ptr->is_triggered = direct_trigger;
}
/* void Localization_set_endpos(Localization_t *loc_ptr)
 *  Description:
 *   - called, when second end-point is reached during calibration process
 *   - saves the measured range of the linear guide from the center in mm
 *   - set the current position for the first time possible
 */
void Localization_set_endpos(Localization_t *loc_ptr)
{
	loc_ptr->end_pos_mm = Localization_pulse_count_to_distance(*loc_ptr)/2;
	loc_ptr->current_pos_mm = loc_ptr->end_pos_mm;
}

/* void Localization_set_center(Localization_t *loc_ptr)
 *  Description:
 *   - save center position to finish and confirm localization
 */
void Localization_set_center(Localization_t *loc_ptr)
{
	loc_ptr->center_pos_mm = loc_ptr->current_pos_mm;
	loc_ptr->is_localized = True;
}

void Localization_set_startpos_abs(Localization_t *loc_ptr, uint16_t measured_value)
{
	loc_ptr->start_pos_abs_mm = measured_value;
	loc_ptr->pulse_count = 0;
}

void Localization_parse_distance_sensor_value(Localization_t *loc_ptr, uint16_t measured_value)
{
	loc_ptr->current_measured_pos_mm = measured_value - loc_ptr->start_pos_abs_mm - loc_ptr->end_pos_mm;
}

void Localization_callback_pulse_count(Localization_t *loc_ptr)
{
	switch(loc_ptr->movement)
	{
		case Loc_movement_backwards:
			loc_ptr->pulse_count++; break;
		case Loc_movement_forward:
			loc_ptr->pulse_count--; break;
		case Loc_movement_stop:
			break;
	}
}

/* void Localization_update_position(Localization_t *loc_ptr)
 *  Description:
 *   - convert the pulse count of the motor to a distance and center it
 *   - save the result to the localization reference
 */
int8_t Localization_update_position(Localization_t *loc_ptr)
{
	if (loc_ptr->state < Loc_state_3_approach_center)
	{
		return LOC_NOT_LOCALIZED;
	}
	uint16_t absolute_distance = Localization_pulse_count_to_distance(*loc_ptr);
	int16_t new_pos_mm = absolute_distance - loc_ptr->end_pos_mm;
	if (new_pos_mm == loc_ptr->current_pos_mm)
	{
		return LOC_POSITION_RETAINED;
	}
	loc_ptr->current_pos_mm = new_pos_mm;
	return LOC_POSITION_UPDATED;
}

void Localization_serialize(Localization_t loc, char serial_buffer[LOC_SERIAL_SIZE])
{
	sprintf(serial_buffer, LOC_SERIAL_FORMAT_SPECMS, (int)loc.state, (int)loc.pulse_count, (int)loc.end_pos_mm, (int)loc.center_pos_mm, (int)loc.movement, (int)loc.start_pos_abs_mm);
}

void Localization_adapt_to_sensor(Localization_t *loc_ptr)
{
	loc_ptr->pulse_count = (int16_t) ((loc_ptr->current_measured_pos_mm + loc_ptr->end_pos_mm) / loc_ptr->distance_per_pulse);
	loc_ptr->current_pos_mm = loc_ptr->current_measured_pos_mm;
}

/* private function definitions -----------------------------------------------*/

/* static int32_t Localization_pulse_count_to_distance(Localization_t loc)
 *  Description:
 *   - convert measured pulse count of the motor to a distance in mm, using distance per pulse parameter of the Linear guide
 */
static int16_t Localization_pulse_count_to_distance(Localization_t loc)
{
	return (int16_t) (loc.pulse_count * loc.distance_per_pulse);
}

static int8_t Localization_deserialize(Localization_t *loc_ptr, char serial_buffer[LOC_SERIAL_SIZE])
{
	Loc_movement_t last_movement;
	if (!sscanf(serial_buffer, LOC_SERIAL_FORMAT_SPECMS, (int *)&loc_ptr->state, (int *)&loc_ptr->pulse_count, (int *)&loc_ptr->end_pos_mm, (int *)&loc_ptr->center_pos_mm, (int *)&last_movement, (int *)&loc_ptr->start_pos_abs_mm))
	{
		return LOC_RECOVERY_RESET;
	}
	if (loc_ptr->state != Loc_state_5_center_pos_set)
	{
		return LOC_RECOVERY_RESET;
	}
	if (last_movement != Loc_movement_stop)
	{
		return LOC_RECOVERY_PARTIAL;
	}
	return LOC_RECOVERY_COMPLETE;
}

void Localization_recover(Localization_t *loc_ptr, int8_t recovery_state, boolean_t direct_trigger)
{
	loc_ptr->recovery_state = recovery_state;
	switch(recovery_state)
	{
		case LOC_RECOVERY_RESET:
			Localization_reset(loc_ptr, direct_trigger);
			break;
		case LOC_RECOVERY_PARTIAL:
			loc_ptr->state = Loc_state_0_init;
			loc_ptr->is_triggered = direct_trigger;
			loc_ptr->is_localized = False;
			break;
		case LOC_RECOVERY_COMPLETE:
			loc_ptr->is_localized = True;
			loc_ptr->desired_pos_mm = loc_ptr->current_pos_mm;
			Localization_update_position(loc_ptr);
			break;
	}
}

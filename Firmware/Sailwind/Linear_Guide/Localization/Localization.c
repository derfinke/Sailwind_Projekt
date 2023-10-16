/*
 * Localization.c
 *
 *  Created on: 28.07.2023
 *      Author: Bene
 */

#include "Localization.h"
#include <stdlib.h>
#include <string.h>

/* defines ------------------------------------------------------------*/

/* private function prototypes -----------------------------------------------*/
static int8_t Localization_deserialize(Localization_t *loc_ptr, uint8_t serial_buffer[sizeof(Loc_safe_data_t)]);
static int16_t Localization_pulse_count_to_distance(Localization_t loc);
static boolean_t Localization_target_on_the_way(Localization_t loc, int16_t desired_pos_mm);

/* API function definitions -----------------------------------------------*/
Localization_t Localization_init(float distance_per_pulse, uint8_t serial_buffer[sizeof(Loc_safe_data_t)])
{
	Localization_t localization = {
			.is_localized = False,
			.is_triggered = False,
			.movement = Loc_movement_stop,
			.state = Loc_state_0_init,
			.center_pos_mm = 0,
			.pulse_count = 0,
			.distance_per_pulse = distance_per_pulse,
			.desired_pos_queue = LOC_DESIRED_POS_QUEUE_EMPTY
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
	loc_ptr->desired_pos_queue = LOC_DESIRED_POS_QUEUE_EMPTY;
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
	loc_ptr->state = Loc_state_5_center_pos_set;
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
			Localization_update_position(loc_ptr);
			loc_ptr->desired_pos_mm = loc_ptr->current_pos_mm;
			loc_ptr->is_localized = True;
			break;
	}
}

void Localization_serialize(Localization_t loc, uint8_t buffer[sizeof(Loc_safe_data_t)])
{
	Loc_safe_data_t *safe_data_ptr = (Loc_safe_data_t *) malloc(sizeof(Loc_safe_data_t));
	safe_data_ptr->state = loc.state;
	safe_data_ptr->pulse_count = loc.pulse_count;
	safe_data_ptr->end_pos_mm = loc.end_pos_mm;
	safe_data_ptr->center_pos_mm = loc.center_pos_mm;
	safe_data_ptr->start_pos_abs_mm = loc.start_pos_abs_mm;
	memcpy(buffer, safe_data_ptr, sizeof(Loc_safe_data_t));
	free(safe_data_ptr);
}

void Localization_adapt_to_sensor(Localization_t *loc_ptr)
{
	loc_ptr->pulse_count = (int16_t) ((loc_ptr->current_measured_pos_mm + loc_ptr->end_pos_mm) / loc_ptr->distance_per_pulse);
	loc_ptr->current_pos_mm = loc_ptr->current_measured_pos_mm;
}

Loc_movement_t Localization_get_next_movement(Localization_t loc, int16_t desired_pos_mm)
{
	Loc_movement_t movement = Loc_movement_stop;
	if (desired_pos_mm > (loc.current_pos_mm + loc.brake_path_mm))
	{
		movement = Loc_movement_backwards;
	}
	else if (desired_pos_mm < (loc.current_pos_mm - loc.brake_path_mm))
	{
		movement = Loc_movement_forward;
	}
	return movement;
}

void Localization_set_desired_pos_queued(Localization_t *loc_ptr, int16_t desired_pos_mm, Loc_movement_t new_movement)
{
	if (Localization_target_on_the_way(*loc_ptr, desired_pos_mm))
	{
		loc_ptr->desired_pos_mm = desired_pos_mm;
		if (new_movement == loc_ptr->movement)
		{
			loc_ptr->desired_pos_queue = LOC_DESIRED_POS_QUEUE_EMPTY;
		}
	}
	else
	{
		loc_ptr->desired_pos_queue = desired_pos_mm;
	}
}

void Localization_progress_queue(Localization_t *loc_ptr)
{
	if (loc_ptr->desired_pos_queue != LOC_DESIRED_POS_QUEUE_EMPTY)
	{
		loc_ptr->desired_pos_mm = loc_ptr->desired_pos_queue;
		loc_ptr->desired_pos_queue = LOC_DESIRED_POS_QUEUE_EMPTY;
	}
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

static int8_t Localization_deserialize(Localization_t *loc_ptr, uint8_t serial_buffer[sizeof(Loc_safe_data_t)])
{
	Loc_safe_data_t *safe_data_ptr = (Loc_safe_data_t *) malloc(sizeof(Loc_safe_data_t));
	memcpy(safe_data_ptr, serial_buffer, sizeof(Loc_safe_data_t));
	loc_ptr->state = safe_data_ptr->state;
	loc_ptr->pulse_count = safe_data_ptr->pulse_count;
	loc_ptr->end_pos_mm = safe_data_ptr->end_pos_mm;
	loc_ptr->center_pos_mm = safe_data_ptr->center_pos_mm;
	loc_ptr->start_pos_abs_mm = safe_data_ptr->start_pos_abs_mm;
	free(safe_data_ptr);
	if (loc_ptr->state != Loc_state_5_center_pos_set)
	{
		return LOC_RECOVERY_RESET;
	}
	return LOC_RECOVERY_COMPLETE;
}

static boolean_t Localization_target_on_the_way(Localization_t loc, int16_t desired_pos_mm)
{
	return loc.movement == Loc_movement_stop || loc.movement == Localization_get_next_movement(loc, desired_pos_mm);
}

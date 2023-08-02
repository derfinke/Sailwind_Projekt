/*
 * Localization.c
 *
 *  Created on: 28.07.2023
 *      Author: Bene
 */

#include "Localization.h"

/* private function prototypes -----------------------------------------------*/
static int32_t Localization_pulse_count_to_distance(Localization_t loc);
static void Localization_update_current_position(Localization_t *loc_ptr);

/* API function definitions -----------------------------------------------*/
Localization_t Localization_init(float distance_per_pulse)
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
	return localization;
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

boolean_t Localization_callback_update_position(Localization_t *loc_ptr)
{
	if (loc_ptr->state < Loc_state_2_approach_back)
	{
		return loc_ptr->is_localized;
	}
	switch(loc_ptr->movement)
	{
		case Loc_movement_backwards:
			loc_ptr->pulse_count++; break;
		case Loc_movement_forward:
			loc_ptr->pulse_count--; break;
		case Loc_movement_stop:
			break;
		}
	if (loc_ptr->state < Loc_state_3_approach_center)
	{
		return loc_ptr->is_localized;
	}
	Localization_update_current_position(loc_ptr);
	return loc_ptr->is_localized;
}

/* void Localization_update_current_position(Localization_t *loc_ptr)
 *  Description:
 *   - convert the pulse count of the motor to a distance and center it
 *   - save the result to the localization reference
 */
void Localization_update_current_position(Localization_t *loc_ptr)
{
	int32_t absolute_distance = Localization_pulse_count_to_distance(*loc_ptr);
	loc_ptr->current_pos_mm = absolute_distance - loc_ptr->end_pos_mm;
}

/* private function definitions -----------------------------------------------*/

/* static int32_t Localization_pulse_count_to_distance(Localization_t loc)
 *  Description:
 *   - convert measured pulse count of the motor to a distance in mm, using distance per pulse parameter of the Linear guide
 */
static int32_t Localization_pulse_count_to_distance(Localization_t loc)
{
	return (uint32_t) (loc.pulse_count * loc.distance_per_pulse);
}

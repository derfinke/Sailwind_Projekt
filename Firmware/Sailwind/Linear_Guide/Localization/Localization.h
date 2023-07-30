/*
 * Localization.h
 *
 *  Created on: 28.07.2023
 *      Author: Bene
 */

#ifndef LOCALIZATION_LOCALIZATION_H_
#define LOCALIZATION_LOCALIZATION_H_

#include "../boolean.h"
#include <stdio.h>

/* typedefs -----------------------------------------------------------*/
typedef enum {
	Loc_state_0_init,
	Loc_state_1_approach_front,
	Loc_state_2_approach_back,
	Loc_state_3_approach_center,
	Loc_state_4_set_center_pos,
} Loc_state_t;

typedef enum {
	Loc_movement_stop,
	Loc_movement_backwards,
	Loc_movement_forward
} Loc_movement_t;

typedef struct {
	Loc_state_t state;
	Loc_movement_t movement;
	boolean_t is_localized;
	uint8_t motor_ppr;
	uint8_t distance_per_rotation;
	int32_t end_pos_mm;
	int32_t center_pos_mm;
	int32_t current_pos_mm;
	int32_t current_pulse_count;
} Localization_t;

/* API function prototypes ---------------------------------------------------*/
Localization_t Localization_init(uint8_t motor_pulse_per_rotation, uint8_t distance_per_rotation);
void Loc_set_endpos(Localization_t *loc_ptr);
void Loc_set_center(Localization_t *loc_ptr);
void Loc_update_pulse_count(Localization_t *loc_ptr);
void Loc_update_current_position(Localization_t *loc_ptr);

#endif /* LOCALIZATION_LOCALIZATION_H_ */

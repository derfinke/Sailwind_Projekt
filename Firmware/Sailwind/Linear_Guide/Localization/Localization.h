/*
 * Localization.h
 *
 *  Created on: 28.07.2023
 *      Author: Bene
 */

#ifndef LOCALIZATION_LOCALIZATION_H_
#define LOCALIZATION_LOCALIZATION_H_

#include "boolean.h"
#include <stdio.h>

/* defines ------------------------------------------------------------*/
#define LOC_SERIAL_FORMAT_SPEC "%hhu,%ld,%ld,%ld" //SPEC = "State, Pulse_count, End_pos, Center_pos"
#define LOC_SERIAL_SIZE 19 //strlen of serial string: strlen(S,PPPPP,EEEEE,CCCCC) = 1+1+5+1+5+1+5 = 19

/* typedefs -----------------------------------------------------------*/
typedef enum {
	Loc_state_0_init,
	Loc_state_1_approach_front,
	Loc_state_2_approach_back,
	Loc_state_3_approach_center,
	Loc_state_4_set_center_pos,
	Loc_state_5_center_pos_set
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
	boolean_t is_triggered;
	float distance_per_pulse;
	int32_t end_pos_mm;
	int32_t center_pos_mm;
	int32_t current_pos_mm;
	int32_t pulse_count;
} Localization_t;

/* API function prototypes ---------------------------------------------------*/
Localization_t Localization_init(float distance_per_pulse, char serial_buffer[LOC_SERIAL_SIZE]);
void Localization_set_endpos(Localization_t *loc_ptr);
void Localization_set_center(Localization_t *loc_ptr);
boolean_t Localization_callback_update_position(Localization_t *loc_ptr);
void Localization_serialize(Localization_t loc, char serial_buffer[LOC_SERIAL_SIZE]);
int32_t Localization_pulse_count_to_distance(Localization_t loc);

#endif /* LOCALIZATION_LOCALIZATION_H_ */

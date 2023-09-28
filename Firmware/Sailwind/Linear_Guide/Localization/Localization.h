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
#define LOC_SERIAL_SIZE 27 //strlen of serial string: strlen(S,PPPPP,EEEEE,CCCCC,M,SSSSS) = 1+1+5+1+5+1+5+1+1+1+5 = 27
#define LOC_NOT_LOCALIZED 1
#define LOC_POSITION_RETAINED 2
#define LOC_POSITION_UPDATED 0
#define LOC_RECOVERY_RESET -1
#define LOC_RECOVERY_COMPLETE 0
#define LOC_RECOVERY_PARTIAL 1

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
	int32_t start_pos_abs_mm;
	int32_t center_pos_mm;
	int32_t current_pos_mm;
	int32_t current_measured_pos_mm;
	int32_t pulse_count;
	int32_t desired_pos_mm;
	int8_t recovery_state;
} Localization_t;

/* API function prototypes ---------------------------------------------------*/
Localization_t Localization_init(float distance_per_pulse, char serial_buffer[LOC_SERIAL_SIZE]);
void Localization_reset(Localization_t *loc_ptr, boolean_t direct_trigger);
void Localization_recover(Localization_t *loc_ptr, int8_t recovery_state, boolean_t direct_trigger);
void Localization_set_endpos(Localization_t *loc_ptr);
void Localization_set_center(Localization_t *loc_ptr);
void Localization_set_startpos_abs(Localization_t *loc_ptr, uint16_t measured_value);
void Localization_parse_distance_sensor_value(Localization_t *loc_ptr, uint16_t measured_value);
void Localization_adapt_to_sensor(Localization_t *loc_ptr);
void Localization_callback_pulse_count(Localization_t *loc_ptr);
int8_t Localization_update_position(Localization_t *loc_ptr);
void Localization_serialize(Localization_t loc, char serial_buffer[LOC_SERIAL_SIZE]);

#endif /* LOCALIZATION_LOCALIZATION_H_ */

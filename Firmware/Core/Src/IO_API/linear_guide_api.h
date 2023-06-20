/*
 * linear_guide_api.h
 *
 *  Created on: 18.06.2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_LINEAR_GUIDE_API_H_
#define SRC_IO_API_LINEAR_GUIDE_API_H_

#include "motor_API.h"
#include "LED_API.h"

/* typedefs -----------------------------------------------------------*/
typedef enum {
	linear_guide_calibrate_button_state_0_init,
	linear_guide_calibrate_button_state_1_approach_borders,
	linear_guide_calibrate_button_state_2_set_center_pos,
} linear_guide_calibrate_button_state_t;

typedef enum {
	linear_guide_approach_borders_state_0_init,
	linear_guide_approach_borders_state_1_approach_vorne,
	linear_guide_approach_borders_state_2_approach_hinten,
	linear_guide_approach_borders_state_3_approach_center
} linear_guide_approach_borders_state_t;

typedef struct {
	linear_guide_calibrate_button_state_t calibrate_button_state;
	linear_guide_approach_borders_state_t approach_borders_state;
	int32_t end_pos_mm;
	int32_t center_pos_mm;
	int32_t current_pos_mm;
	int32_t current_pos_pulse_count;
	boolean_t is_calibrated;
} linear_guide_calibration_t;

typedef struct {
	IO_digitalPin_t vorne;
	IO_digitalPin_t hinten;
} linear_guide_endschalter_t;

typedef struct {
	IO_operating_mode_t operating_mode;
	Motor_t motor;
	linear_guide_calibration_t calibration;
	linear_guide_endschalter_t endschalter;
} Linear_guide_t;

/* defines ------------------------------------------------------------*/
#define LINEAR_GUIDE_DISTANCE_PER_ROTATION 5

/* API function prototypes -----------------------------------------------*/
Linear_guide_t linear_guide_init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim);
void linear_guide_set_operating_mode(Linear_guide_t *linear_guide_ptr, IO_operating_mode_t operating_mode);
void linear_guide_calibrate_button_state_machine(Linear_guide_t *linear_guide_ptr, LED_t *led_center_pos_set);
void linear_guide_calibrate_state_machine_set_endpoints(Linear_guide_t *linear_guide_ptr);
void linear_guide_callback_get_rpm(Linear_guide_t *linear_guide_ptr, TIM_HandleTypeDef *htim_ptr);
void linear_guide_set_center(linear_guide_calibration_t *calibration_ptr);
boolean_t linear_guide_get_manual_moving_permission(Linear_guide_t linear_guide);

#endif /* SRC_IO_API_LINEAR_GUIDE_API_H_ */

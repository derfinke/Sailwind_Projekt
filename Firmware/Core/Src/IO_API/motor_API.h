/*
 * motor_API.h
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_MOTOR_API_H_
#define SRC_IO_API_MOTOR_API_H_

#include "IO_API.h"
#include "LED_API.h"

/* typedefs -----------------------------------------------------------*/
typedef enum {
	motor_function_aus,
	motor_function_rechtslauf,
	motor_function_linkslauf,
	motor_function_stopp_mit_haltemoment,
	motor_function_drehzahlvorgabe,
	motor_function_Stromvorgabe,
	motor_function_speed1,
	motor_function_speed2
} motor_function_t;

typedef enum {
	motor_calibration_state_0_init,
	motor_calibration_state_1_save_endpoints,
	motor_calibration_state_2_set_center_pos,
} motor_calibration_state_t;

typedef enum {
	motor_set_endpoints_state_0_init,
	motor_set_endpoints_state_1_move_to_end_pos_vorne,
	motor_set_endpoints_state_2_move_to_end_pos_hinten,
} motor_set_endpoints_state_t;

typedef struct {
	uint32_t timer_cycle_count;
	IO_digitalPin_t puls;
	float currentValue;
	TIM_HandleTypeDef *htim;
} RPM_Measurement_t;

typedef struct {
	motor_calibration_state_t state;
	motor_set_endpoints_state_t set_endpoints_state;
	int32_t end_pos_mm;
	int32_t center_pos_mm;
	int32_t current_pos_mm;
	int32_t current_pos_pulse_count;
	int32_t max_distance_mm;
	boolean_t is_calibrated;
} motor_calibration_t;

typedef struct {
	IO_digitalPin_t vorne;
	IO_digitalPin_t hinten;
} motor_endschalter_t;

typedef struct {
	IO_operating_mode_t operating_mode;
	motor_calibration_t calibration;
	IO_digitalPin_t IN0;
	IO_digitalPin_t IN1;
	IO_digitalPin_t IN2;
	IO_digitalPin_t IN3;
	motor_function_t current_function;

	IO_analogActuator_t AIN_Drehzahl_Soll;

	RPM_Measurement_t OUT1_Drehzahl_Messung;
	IO_digitalPin_t OUT2_Fehler;
	IO_digitalPin_t OUT3_Drehrichtung;

	motor_endschalter_t endschalter;
} Motor_t;

/* defines ------------------------------------------------------------*/
#define MOTOR_RPM_MAX 642
#define MOTOR_PULSE_PER_ROTATION 12
#define MOTOR_DISTANCE_PER_ROTATION 5

/* API function prototypes -----------------------------------------------*/
Motor_t motor_init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim);
void motor_start_moving(Motor_t *motor_ptr, motor_function_t motor_function_direction);
void motor_stop_moving(Motor_t *motor_ptr);
void motor_set_function(Motor_t *motor_ptr, motor_function_t function);
void motor_start_rpm_measurement(Motor_t *motor_ptr);
void motor_stop_rpm_measurement(Motor_t *motor_ptr);
void motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value);
void motor_callback_get_rpm(Motor_t *motor_ptr, TIM_HandleTypeDef *htim); //only for use in timer callback function
void motor_teach_speed(Motor_t *motor_ptr, motor_function_t speed, uint32_t rpm_value, uint32_t tolerance);
void motor_set_operating_mode(Motor_t *motor_ptr, IO_operating_mode_t operating_mode);
void motor_button_calibrate_state_machine(Motor_t *motor_ptr, LED_t *led_center_pos_set);
void motor_calibrate_state_machine_set_endpoints(Motor_t *motor_ptr);


#endif /* SRC_IO_API_MOTOR_API_H_ */

/*
 * motor_API.h
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_MOTOR_API_H_
#define SRC_IO_API_MOTOR_API_H_

#include "IO_API.h"

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
	motor_moving_state_aus,
	motor_moving_state_rechtslauf,
	motor_moving_state_linkslauf
} motor_moving_state_t;

typedef struct {
	uint32_t IC_Val1;
	uint32_t IC_Val2;
	int Is_First_Captured;
	float rpm_value;
	TIM_HandleTypeDef *htim_ptr;
} RPM_Measurement_t;

typedef struct {
	motor_moving_state_t moving_state;
	motor_function_t current_function;

	IO_digitalPin_t IN0;
	IO_digitalPin_t IN1;
	IO_digitalPin_t IN2;
	IO_digitalPin_t IN3;

	IO_analogActuator_t AIN_Drehzahl_Soll;

	RPM_Measurement_t OUT1_Drehzahl_Messung;
	IO_digitalPin_t OUT2_Fehler;
	IO_digitalPin_t OUT3_Drehrichtung;
} Motor_t;

/* defines ------------------------------------------------------------*/
#define MOTOR_RPM_MAX 4378.44F // corresponds to ANALOG_MAX (4096) and max output voltage of 10.7 V -> 4092 rpm corresponds to 10 V (BG 45 SI manual)
#define MOTOR_RPM_LIMIT 700.0F// estimated normal speed for linear guide
#define MOTOR_PULSE_PER_ROTATION 12

/* API function prototypes -----------------------------------------------*/
Motor_t motor_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr);
void motor_start_moving(Motor_t *motor_ptr, motor_moving_state_t moving_state);
void motor_stop_moving(Motor_t *motor_ptr);
void motor_set_function(Motor_t *motor_ptr, motor_function_t function);
void motor_start_rpm_measurement(Motor_t *motor_ptr);
boolean_t motor_callback_measure_rpm(Motor_t *motor_ptr, TIM_HandleTypeDef *htim_ptr);
void motor_stop_rpm_measurement(Motor_t *motor_ptr);
void motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value);
float motor_pulse_to_rpm(float f_pulse);
void motor_teach_speed(Motor_t *motor_ptr, motor_function_t speed, uint32_t rpm_value, uint32_t tolerance);


#endif /* SRC_IO_API_MOTOR_API_H_ */

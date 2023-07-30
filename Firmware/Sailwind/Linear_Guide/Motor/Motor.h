/*
 * motor_API.h
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#ifndef MOTOR_MOTOR_H_
#define MOTOR_MOTOR_H_

#include "../IO/IO.h"

/* typedefs -----------------------------------------------------------*/
typedef enum {
	Motor_function_aus,
	Motor_function_rechtslauf,
	Motor_function_linkslauf,
	Motor_function_stopp_mit_haltemoment,
	Motor_function_drehzahlvorgabe,
	Motor_function_Stromvorgabe,
	Motor_function_speed1,
	Motor_function_speed2
} Motor_function_t;

typedef struct {
	uint32_t IC_Val1;
	uint32_t IC_Val2;
	boolean_t Is_First_Captured;
	float rpm_value;
	TIM_HandleTypeDef *htim_ptr;
	HAL_TIM_ActiveChannel active_channel;
	uint32_t htim_channel;
} RPM_Measurement_t;

typedef struct {
	Motor_function_t current_function;

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
Motor_t Motor_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel);
void Motor_start_moving(Motor_t *motor_ptr, Motor_function_t function);
void Motor_stop_moving(Motor_t *motor_ptr);
void Motor_set_function(Motor_t *motor_ptr, Motor_function_t function);
void Motor_start_rpm_measurement(Motor_t *motor_ptr);
boolean_t Motor_callback_measure_rpm(Motor_t *motor_ptr);
void Motor_stop_rpm_measurement(Motor_t *motor_ptr);
void Motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value);
float Motor_pulse_to_rpm(float f_pulse);
void Motor_teach_speed(Motor_t *motor_ptr, Motor_function_t speed, uint32_t rpm_value, uint32_t tolerance);


#endif /* MOTOR_MOTOR_H_ */

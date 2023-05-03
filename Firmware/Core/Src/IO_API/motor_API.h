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
	motor_function_linkslauf,
	motor_function_rechtslauf,
	motor_function_stopp_mit_haltemoment,
	motor_function_drehzahlvorgabe,
	motor_function_Stromvorgabe,
	motor_function_speed1,
	motor_speed2
} motor_function_t;

typedef enum {
	motor_operating_mode_manual,
	motor_operating_mode_automatic
} motor_operating_mode_t;

typedef struct {
	uint32_t timer_cycle_count;
	digitalPin_t puls;
	uint32_t currentValue;
	TIM_HandleTypeDef *htim;
} RPM_Measurement_t;

typedef struct {
	motor_operating_mode_t operating_mode;
	boolean_t isCalibrated;
	digitalPin_t IN0;
	digitalPin_t IN1;
	digitalPin_t IN2;
	digitalPin_t IN3;
	motor_function_t current_function;

	analogActuator_t AIN_Drehzahl_Soll;

	RPM_Measurement_t OUT1_Drehzahl_Messung;
	digitalPin_t OUT2_Fehler;
	digitalPin_t OUT3_Drehrichtung;
} Motor_t;

/* defines ------------------------------------------------------------*/
#define RPM_MAX 642
#define _motor_press_enter_to_continue() getchar()

/* API function prototypes -----------------------------------------------*/
Motor_t motor_init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim);
void motor_start_moving(Motor_t *motor, motor_function_t motor_function_direction);
void motor_stop_moving(Motor_t *motor);
void motor_set_function(Motor_t *motor, motor_function_t function);
void motor_start_rpm_measurement(Motor_t *motor);
void motor_stop_rpm_measurement(Motor_t *motor);
void motor_set_rpm(Motor_t *motor, uint16_t rpm_value);
void motor_callback_get_rpm(Motor_t *motor, TIM_HandleTypeDef *htim); //only for use in timer callback function
void motor_teach_speed(Motor_t *motor, motor_function_t speed, uint32_t rpm_value, uint32_t tolerance);
void motor_set_operating_mode(Motor_t *motor, motor_operating_mode_t operating_mode);
void motor_calibrate(Motor_t *motor);


#endif /* SRC_IO_API_MOTOR_API_H_ */

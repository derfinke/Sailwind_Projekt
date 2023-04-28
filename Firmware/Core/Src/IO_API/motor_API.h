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
typedef enum {motor_aus, linkslauf, rechtslauf, stopp_mit_haltemoment, drehzahlvorgabe, Stromvorgabe, speed1, speed2} motor_function;

typedef struct {
	uint32_t timer_cycle_count;
	digitalPin puls;
	uint32_t currentValue;
	TIM_HandleTypeDef *htim;
} RPM_Measurement;

typedef struct {
	digitalPin IN0;
	digitalPin IN1;
	digitalPin IN2;
	digitalPin IN3;
	motor_function current_function;

	analogActuator AIN_Drehzahl_Soll;

	RPM_Measurement OUT1_Drehzahl_Messung;
	digitalPin OUT2_Fehler;
	digitalPin OUT3_Drehrichtung;
} Motor;

/* defines ------------------------------------------------------------*/
#define RPM_MAX 642
#define press_enter_to_continue() getchar()

/* API function prototypes -----------------------------------------------*/
Motor motor_init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim);
void motor_set_function(Motor *motor, motor_function function);
void motor_start_rpm_measurement(Motor *motor);
void motor_stop_rpm_measurement(Motor *motor);
void motor_set_rpm(Motor *motor, uint16_t rpm_value);
void motor_callback_get_rpm(Motor *motor, TIM_HandleTypeDef *htim); //only for use in timer callback function
void motor_teach_speed(Motor *motor, motor_function speed, uint32_t rpm_value, uint32_t tolerance);

/* private function prototypes -----------------------------------------------*/
void convert_timeStep_to_rpm(RPM_Measurement *drehzahl_messung);

#endif /* SRC_IO_API_MOTOR_API_H_ */

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
	digitalPin IN0;
	digitalPin IN1;
	digitalPin IN2;
	digitalPin IN3;
	motor_function current_function;

	analogActuator AIN_Drehzahl_Sollwert;

	digitalPin OUT1_Drehzahl_Puls;
	digitalPin OUT2_Fehler;
	digitalPin OUT3_Drehrichtung;
} motor;

/* API function prototypes -----------------------------------------------*/
motor motor_init(DAC_HandleTypeDef *hdac);
void motor_set_function(motor *motor, motor_function function);

#endif /* SRC_IO_API_MOTOR_API_H_ */

/*
 * motor_API.c
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#include "motor_API.h"

/* API function prototypes -----------------------------------------------*/
motor motor_init(DAC_HandleTypeDef *hdac)
{
	motor motor = {
			.IN0 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN0_PD7_OUT_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN1 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN1_PD6_OUT_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN2 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN2_PD4_OUT_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN2 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN3_PD3_OUT_Pin,
					.state = GPIO_PIN_RESET
			},
			.current_function = motor_aus,
			.AIN_Drehzahl_Sollwert = {
					.unit = "rpm",
					.hdac = hdac,
					.channel = DAC_CHANNEL_1,
					.signalConversion = 1,
					.currentValue = 0,
					.adc_value = 0
			},
			.OUT1_Drehzahl_Puls = {
					.GPIOx = GPIOF,
					.GPIO_Pin = OUT1_PF11_IN_Pin,
					.state = GPIO_PIN_RESET
			},
			.OUT2_Fehler = {
					.GPIOx = GPIOF,
					.GPIO_Pin = OUT2_PF12_IN_Pin,
					.state = GPIO_PIN_RESET
			},
			.OUT3_Drehrichtung = {
					.GPIOx = GPIOF,
					.GPIO_Pin = OUT3_PF13_IN_Pin,
					.state = GPIO_PIN_RESET
			}
	};
	motor_set_function(&motor, motor_aus);
	return motor;
}

void motor_set_function(motor *motor, motor_function function)
{
	motor->current_function = function;
	digitalPin *motor_INs[] = {&motor->IN0, &motor->IN1, &motor->IN2, &motor->IN3};
	function -= (function >= 4) * 4;
	int function_bits = (int) function;
	unsigned int mask = 1U << 1;
	for (int i = 0; i < 2; i++) {
								//function 0-3: set IN0 + IN1; function 4-7: set IN2 + IN3
		writeDigitalOUT(motor_INs[i + (function >= 4)*2], (GPIO_PinState) (function_bits & mask) ? 1 : 0);
		function_bits <<= 1;
	}
}




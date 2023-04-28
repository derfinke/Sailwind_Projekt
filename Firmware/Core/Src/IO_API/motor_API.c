/*
 * motor_API.c
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#include "motor_API.h"

/* API function definitions -----------------------------------------------*/
Motor motor_init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim)
{
	Motor motor = {
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
			.IN3 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN3_PD3_OUT_Pin,
					.state = GPIO_PIN_RESET
			},
			.current_function = motor_aus,
			.AIN_Drehzahl_Soll = {
					.unit = "rpm",
					.hdac = hdac,
					.channel = DAC_CHANNEL_1,
					.maxValue = RPM_MAX,
					.currentValue = 0,
					.adc_value = 0
			},
			.OUT1_Drehzahl_Messung = {
					.timer_cycle_count = 0,
					.puls = {
							.GPIOx = GPIOF,
							.GPIO_Pin = OUT1_PF11_IN_Pin,
							.state = GPIO_PIN_RESET
					},
					.currentValue = 0,
					.htim = htim,
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

void motor_set_function(Motor *motor, motor_function function)
{
	motor->current_function = function;
	digitalPin *motor_INs[] = {&motor->IN0, &motor->IN1, &motor->IN2, &motor->IN3};
	function -= (function >= 4) * 4;
	int function_bits = (int) function;
	unsigned int mask = 1U << 1;
	for (int i = 0; i < 2; i++) {
								//function 0-3: set IN0 + IN1; function 4-7: set IN2 + IN3
		IO_writeDigitalOUT(motor_INs[i + (function >= 4)*2], (GPIO_PinState) (function_bits & mask) ? 1 : 0);
		function_bits <<= 1;
	}
}

void motor_set_rpm(Motor *motor, uint16_t rpm_value)
{
	motor_set_function(motor, drehzahlvorgabe);
	IO_writeAnalogValue(&motor->AIN_Drehzahl_Soll, rpm_value);
}

void motor_start_rpm_measurement(Motor *motor)
{
	RPM_Measurement *drehzahl_messung = &motor->OUT1_Drehzahl_Messung;
	HAL_TIM_Base_Start_IT(drehzahl_messung->htim);
}

void motor_stop_rpm_measurement(Motor *motor)
{
	HAL_TIM_Base_Stop_IT(motor->OUT1_Drehzahl_Messung.htim);
}

void motor_teach_speed(Motor *motor, motor_function speed, uint32_t rpm_value, uint32_t tolerance)
{
	printf("\nstoppt Motor...\n");
	motor_set_function(motor, stopp_mit_haltemoment);
	printf("Enter drücken um Lernmodus einzuleiten, sobald Motor angehalten...\n");
	press_enter_to_continue();
	printf("leite Lernmodus ein...\n");
	for (int i=0; i<5; i++)
	{
		IO_toggleDigitalOUT(&motor->IN2);
		HAL_Delay(500);
	}
	printf("Lernmodus aktiviert, wenn rote LED schnell blinkt -> Enter um fortzufahren...\n");
	press_enter_to_continue();
	printf("Motor auf Zieldrehzahl bringen...\n");
	motor_set_rpm(motor, rpm_value);
	printf("wartet bis Zieldrehzahl erreicht wurde...\n");
	while((rpm_value - motor->OUT1_Drehzahl_Messung.currentValue) > tolerance);
	printf("Zieldrehzahl erreicht -> Setze motor speed %d auf %ld rpm\n", speed-5, rpm_value);
	motor_set_function(motor, speed);
	printf("\nstoppt Motor...\n");
	motor_set_function(motor, stopp_mit_haltemoment);
	printf("Enter drücken um Lernmodus zu verlassen, sobald Motor angehalten...\n");
	press_enter_to_continue();
	printf("verlasse Lernmodus...\n");
	for (int i=0; i<5; i++)
	{
		IO_toggleDigitalOUT(&motor->IN2);
		HAL_Delay(500);
	}
	printf("Lernmodus deaktiviert, wenn rote LED langsam blinkt -> Enter um fortzufahren...\n");
	press_enter_to_continue();
}

/* private function definitions -----------------------------------------------*/
void convert_timeStep_to_rpm(RPM_Measurement *drehzahl_messung)
{
	//max rpm = 642 -> 10,7 Hz -> max f_pulse = 128,4 Hz -> ~ min 20 samples / period -> f_timer = 2500 Hz
	uint32_t f_timer = (uint32_t) HAL_RCC_GetPCLK2Freq() / drehzahl_messung->htim->Init.Prescaler / drehzahl_messung->htim->Init.Period;
	uint32_t pulse_per_rotation = 12;
	uint32_t f_pulse = f_timer / drehzahl_messung->timer_cycle_count;
	drehzahl_messung->currentValue = f_pulse / pulse_per_rotation * 60;
	drehzahl_messung->timer_cycle_count = 0;
}

void motor_callback_get_rpm(Motor *motor, TIM_HandleTypeDef *htim)
{
	RPM_Measurement *drehzahl_messung = &motor->OUT1_Drehzahl_Messung;
	if (htim==drehzahl_messung->htim)
	{
		drehzahl_messung->timer_cycle_count++;
		GPIO_PinState previous_state = drehzahl_messung->puls.state;
		GPIO_PinState current_state = IO_readDigitalIN(&drehzahl_messung->puls);
		if (previous_state == GPIO_PIN_RESET && current_state == GPIO_PIN_RESET)
		{
			convert_timeStep_to_rpm(drehzahl_messung);
		}
	}
}

/* Timer Callback implementation for rpm measurement --------------------------*/
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	callback_get_rpm(&motor, htim);
}
*/




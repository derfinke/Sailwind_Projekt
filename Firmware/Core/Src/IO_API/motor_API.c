/*
 * motor_API.c
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#include "motor_API.h"

/* private function prototypes -----------------------------------------------*/
static void _motor_convert_timeStep_to_rpm(RPM_Measurement_t *drehzahl_messung_ptr);
static void _motor_press_enter_to_continue();


/* API function definitions -----------------------------------------------*/
Motor_t motor_init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim)
{
	Motor_t motor = {
			.operating_mode = motor_operating_mode_manual,
			.isCalibrated = False,
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
			.current_function = motor_function_aus,
			.AIN_Drehzahl_Soll = {
					.unit = "rpm",
					.hdac = hdac,
					.channel = DAC_CHANNEL_1,
					.maxValue = MOTOR_RPM_MAX,
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
	motor_set_function(&motor, motor_function_aus);
	return motor;
}

void motor_start_moving(Motor_t *motor_ptr, motor_function_t motor_function_direction)
{
	//ToDo
	// motor_set_function(motor, motor_function_direction);
}

void motor_stop_moving(Motor_t *motor_ptr)
{
	//ToDo
	// motor_set_function(motor, motor_function_aus);
}

void motor_set_function(Motor_t *motor_ptr, motor_function_t function)
{
	motor_ptr->current_function = function;
	IO_digitalPin_t *motor_INs_ptr[] = {&motor_ptr->IN0, &motor_ptr->IN1, &motor_ptr->IN2, &motor_ptr->IN3};
	function -= (function >= 4) * 4;
	int function_bits = (int) function;
	unsigned int mask = 1U << 1;
	for (int i = 0; i < 2; i++) {
								//function 0-3: set IN0 + IN1; function 4-7: set IN2 + IN3
		IO_writeDigitalOUT(motor_INs_ptr[i + (function >= 4)*2], (GPIO_PinState) (function_bits & mask) ? 1 : 0);
		function_bits <<= 1;
	}
}

void motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value)
{
	motor_set_function(motor_ptr, motor_function_drehzahlvorgabe);
	IO_writeAnalogValue(&motor_ptr->AIN_Drehzahl_Soll, rpm_value);
}

void motor_start_rpm_measurement(Motor_t *motor_ptr)
{
	RPM_Measurement_t *drehzahl_messung = &motor_ptr->OUT1_Drehzahl_Messung;
	HAL_TIM_Base_Start_IT(drehzahl_messung->htim);
}

void motor_stop_rpm_measurement(Motor_t *motor_ptr)
{
	HAL_TIM_Base_Stop_IT(motor_ptr->OUT1_Drehzahl_Messung.htim);
}

void motor_teach_speed(Motor_t *motor_ptr, motor_function_t speed, uint32_t rpm_value, uint32_t tolerance)
{
	printf("\nstoppt Motor...\n");
	motor_set_function(motor_ptr, motor_function_stopp_mit_haltemoment);
	printf("Enter drücken um Lernmodus einzuleiten, sobald Motor angehalten...\n");
	_motor_press_enter_to_continue();
	printf("leite Lernmodus ein...\n");
	for (int i=0; i<5; i++)
	{
		IO_toggleDigitalOUT(&motor_ptr->IN2);
		HAL_Delay(500);
	}
	printf("Lernmodus aktiviert, wenn rote LED schnell blinkt -> Enter um fortzufahren...\n");
	_motor_press_enter_to_continue();
	printf("Motor auf Zieldrehzahl bringen...\n");
	motor_set_rpm(motor_ptr, rpm_value);
	printf("wartet bis Zieldrehzahl erreicht wurde...\n");
	while((rpm_value - motor_ptr->OUT1_Drehzahl_Messung.currentValue) > tolerance);
	printf("Zieldrehzahl erreicht -> Setze motor_ptr speed %d auf %ld rpm\n", speed-5, rpm_value);
	motor_set_function(motor_ptr, speed);
	printf("\nstoppt Motor...\n");
	motor_set_function(motor_ptr, motor_function_stopp_mit_haltemoment);
	printf("Enter drücken um Lernmodus zu verlassen, sobald Motor angehalten...\n");
	_motor_press_enter_to_continue();
	printf("verlasse Lernmodus...\n");
	for (int i=0; i<5; i++)
	{
		IO_toggleDigitalOUT(&motor_ptr->IN2);
		HAL_Delay(500);
	}
	printf("Lernmodus deaktiviert, wenn rote LED langsam blinkt -> Enter um fortzufahren...\n");
	_motor_press_enter_to_continue();
}

void motor_callback_get_rpm(Motor_t *motor_ptr, TIM_HandleTypeDef *htim)
{
	RPM_Measurement_t *drehzahl_messung_ptr = &motor_ptr->OUT1_Drehzahl_Messung;
	if (htim==drehzahl_messung_ptr->htim)
	{
		drehzahl_messung_ptr->timer_cycle_count++;
		GPIO_PinState previous_state = drehzahl_messung_ptr->puls.state;
		GPIO_PinState current_state = IO_readDigitalIN(&drehzahl_messung_ptr->puls);
		if (previous_state == GPIO_PIN_RESET && current_state == GPIO_PIN_RESET)
		{
			_motor_convert_timeStep_to_rpm(drehzahl_messung_ptr);
		}
	}
}

void motor_set_operating_mode(Motor_t *motor_ptr, motor_operating_mode_t operating_mode)
{
	motor_ptr->operating_mode = operating_mode;
	//ToDo
}

void motor_calibrate(Motor_t *motor_ptr)
{
	motor_ptr->isCalibrated = True;
	//ToDo
}
/* private function definitions -----------------------------------------------*/
static void _motor_convert_timeStep_to_rpm(RPM_Measurement_t *drehzahl_messung_ptr)
{
	//max rpm = 642 -> 10,7 Hz -> max f_pulse = 128,4 Hz -> ~ min 20 samples / period -> f_timer = 2500 Hz
	uint32_t f_timer = (uint32_t) HAL_RCC_GetPCLK2Freq() / drehzahl_messung_ptr->htim->Init.Prescaler / drehzahl_messung_ptr->htim->Init.Period;
	uint32_t pulse_per_rotation = 12;
	uint32_t f_pulse = f_timer / drehzahl_messung_ptr->timer_cycle_count;
	drehzahl_messung_ptr->currentValue = f_pulse / pulse_per_rotation * 60;
	drehzahl_messung_ptr->timer_cycle_count = 0;
}

static void _motor_press_enter_to_continue()
{
	getchar();
}



/* Timer Callback implementation for rpm measurement --------------------------*/
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	motor_callback_get_rpm(&motor, htim);
}
*/




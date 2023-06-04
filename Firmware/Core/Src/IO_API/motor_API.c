/*
 * motor_API.c
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#include "motor_API.h"

/* private function prototypes -----------------------------------------------*/
static void convert_timeStep_to_rpm(RPM_Measurement_t *drehzahl_messung_ptr);
static void press_enter_to_continue();
static void calibrate_set_endpoints(motor_calibration_t *calibration);
static void calibrate_set_center(Motor_t *motor_ptr);
static boolean_t endschalter_detected(IO_digitalPin_t *motor_endschalter);
static int32_t convert_pulse_count_to_distance(int32_t pulse_count);
static void update_current_position(Motor_t *motor_ptr);


/* API function definitions -----------------------------------------------*/
Motor_t motor_init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim)
{
	Motor_t motor =
	{
			.operating_mode = IO_operating_mode_manual,
			.calibration =
			{
					.state = motor_calibration_state_0_init,
					.set_endpoints_state = motor_set_endpoints_state_0_init,
					.current_pos_pulse_count = 0,
					.is_calibrated = False
			},
			.IN0 =
			{
					.GPIOx = GPIOD,
					.GPIO_Pin = IN_0_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN1 =
			{
					.GPIOx = GPIOD,
					.GPIO_Pin = IN_1_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN2 =
			{
					.GPIOx = GPIOD,
					.GPIO_Pin = IN_2_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN3 =
			{
					.GPIOx = GPIOD,
					.GPIO_Pin = IN_3_Pin,
					.state = GPIO_PIN_RESET
			},
			.current_function = motor_function_aus,
			.AIN_Drehzahl_Soll =
			{
					.hdac = hdac,
					.channel = DAC_CHANNEL_1,
					.maxValue = MOTOR_RPM_MAX,
					.currentValue = 0.0F,
					.adc_value = 0
			},
			.OUT1_Drehzahl_Messung =
			{
					.timer_cycle_count = 0,
					.puls =
					{
							.GPIOx = GPIOF,
							.GPIO_Pin = Drehzahl_DAC_OUT_Pin,
							.state = GPIO_PIN_RESET
					},
					.currentValue = 0.0F,
					.htim = htim,
			},
			.OUT2_Fehler =
			{
					.GPIOx = GPIOF,
					.GPIO_Pin = OUT_1_Pin,
					.state = GPIO_PIN_RESET
			},
			.OUT3_Drehrichtung =
			{
					.GPIOx = GPIOF,
					.GPIO_Pin = OUT_3_Pin,
					.state = GPIO_PIN_RESET
			}
	};
	motor_set_function(&motor, motor_function_aus);
	return motor;
}

void motor_start_moving(Motor_t *motor_ptr, motor_function_t motor_function_direction)
{
	motor_set_function(motor_ptr, motor_function_direction);
	motor_set_function(motor_ptr, motor_function_speed1);
}

void motor_stop_moving(Motor_t *motor_ptr)
{
	motor_set_function(motor_ptr, motor_function_aus);
}

void motor_set_function(Motor_t *motor_ptr, motor_function_t function)
{
	motor_ptr->current_function = function;
	IO_digitalPin_t *motor_INs_ptr[] = {&motor_ptr->IN0, &motor_ptr->IN1, &motor_ptr->IN2, &motor_ptr->IN3};
	boolean_t pin_offset = function >= 4;
	int8_t function_bits = function - pin_offset * 4; //subtract 4 to function id if its >= 4 -> convert number {0..3} to binary in following for loop
	for (int i = 0; i < 2; i++)
	{ // write IN0 and IN1 (function in {0..3}) or IN2 and IN3 (function in {4..7}
		GPIO_PinState state = function_bits & 2 ? GPIO_PIN_SET : GPIO_PIN_RESET;
		uint8_t INx = i + pin_offset*2;
		IO_digitalWrite(motor_INs_ptr[INx], state);
		function_bits <<= 1;
	}
}

void motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value)
{
	motor_set_function(motor_ptr, motor_function_drehzahlvorgabe);
	IO_analogWrite(&motor_ptr->AIN_Drehzahl_Soll, rpm_value);
}

void motor_start_rpm_measurement(Motor_t *motor_ptr)
{
	HAL_TIM_Base_Start_IT(motor_ptr->OUT1_Drehzahl_Messung.htim);
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
	press_enter_to_continue();
	printf("leite Lernmodus ein...\n");
	for (int i=0; i<5; i++)
	{
		IO_digitalToggle(&motor_ptr->IN2);
		HAL_Delay(500);
	}
	printf("Lernmodus aktiviert, wenn rote LED schnell blinkt -> Enter um fortzufahren...\n");
	press_enter_to_continue();
	printf("Motor auf Zieldrehzahl bringen...\n");
	motor_set_rpm(motor_ptr, rpm_value);
	printf("wartet bis Zieldrehzahl erreicht wurde...\n");
	while((rpm_value - motor_ptr->OUT1_Drehzahl_Messung.currentValue) > tolerance);
	printf("Zieldrehzahl erreicht -> Setze motor_ptr speed %d auf %ld rpm\n", speed-5, rpm_value);
	motor_set_function(motor_ptr, speed);
	printf("\nstoppt Motor...\n");
	motor_set_function(motor_ptr, motor_function_stopp_mit_haltemoment);
	printf("Enter drücken um Lernmodus zu verlassen, sobald Motor angehalten...\n");
	press_enter_to_continue();
	printf("verlasse Lernmodus...\n");
	for (int i=0; i<5; i++)
	{
		IO_digitalToggle(&motor_ptr->IN2);
		HAL_Delay(500);
	}
	printf("Lernmodus deaktiviert, wenn rote LED langsam blinkt -> Enter um fortzufahren...\n");
	press_enter_to_continue();
}

void motor_callback_get_rpm(Motor_t *motor_ptr, TIM_HandleTypeDef *htim)
{
	RPM_Measurement_t *drehzahl_messung_ptr = &motor_ptr->OUT1_Drehzahl_Messung;
	if (htim==drehzahl_messung_ptr->htim)
	{
		drehzahl_messung_ptr->timer_cycle_count++;
		if (IO_digitalRead_rising_edge(&drehzahl_messung_ptr->puls))
		{
			convert_timeStep_to_rpm(drehzahl_messung_ptr);
			if (motor_ptr->calibration.set_endpoints_state == motor_set_endpoints_state_2_move_to_end_pos_hinten || motor_ptr->calibration.is_calibrated)
			{
				if (motor_ptr->current_function == motor_function_rechtslauf)
				{
					motor_ptr->calibration.current_pos_pulse_count++;
				}
				else
				{
					motor_ptr->calibration.current_pos_pulse_count--;
				}
				if (motor_ptr->calibration.state == motor_calibration_state_2_set_center_pos  || motor_ptr->calibration.is_calibrated)
				{
					update_current_position(motor_ptr);
				}
			}
		}
	}
}

void motor_set_operating_mode(Motor_t *motor_ptr, IO_operating_mode_t operating_mode)
{
	boolean_t set_automatic = operating_mode == IO_operating_mode_automatic;
	boolean_t set_manual = operating_mode == IO_operating_mode_manual;
	boolean_t is_calibrated = motor_ptr->calibration.is_calibrated;
	if ((set_automatic AND is_calibrated) OR set_manual)
	{
		motor_ptr->operating_mode = operating_mode;
	}
}

void motor_button_calibrate_state_machine(Motor_t *motor_ptr, LED_t *led_center_pos_set_ptr)
{
	switch(motor_ptr->calibration.state)
	{
		case motor_calibration_state_0_init:
			printf("\nmotor_ptr->calibration.state = motor_calibration_state_1_save_endpoints;\n");
			break;
		case motor_calibration_state_1_save_endpoints:
			//wait for endpoints_set
			break;
		case motor_calibration_state_2_set_center_pos:
			calibrate_set_center(motor_ptr);
			LED_switch(led_center_pos_set_ptr, LED_ON);
			motor_stop_moving(motor_ptr);
			motor_ptr->calibration.is_calibrated = True;
			break;
	}
}

void motor_calibrate_state_machine_set_endpoints(Motor_t *motor_ptr)
{
	motor_calibration_t *calibration = &motor_ptr->calibration;
	if (calibration->state == motor_calibration_state_1_save_endpoints)
	{
		switch(calibration->set_endpoints_state)
		{
			case motor_set_endpoints_state_0_init:
				printf("//ToDo: motor_start_moving(motor_ptr, motor_function_linkslauf);");
				calibration->set_endpoints_state = motor_set_endpoints_state_1_move_to_end_pos_vorne;
				break;
			case motor_set_endpoints_state_1_move_to_end_pos_vorne:
				if (endschalter_detected(&motor_ptr->endschalter.vorne))
				{
					printf("//ToDo: motor_start_moving(motor_ptr, motor_function_rechtslauf);");
					calibration->set_endpoints_state = motor_set_endpoints_state_2_move_to_end_pos_hinten;
				}
				break;
			case motor_set_endpoints_state_2_move_to_end_pos_hinten:
				if (endschalter_detected(&motor_ptr->endschalter.hinten))
				{
					printf("//ToDo: motor_start_moving(motor_ptr, motor_function_linkslauf);");
					calibrate_set_endpoints(calibration);
					calibration->state = motor_calibration_state_2_set_center_pos;
				}
				break;
		}
	}
}

/* private function definitions -----------------------------------------------*/
static void convert_timeStep_to_rpm(RPM_Measurement_t *drehzahl_messung_ptr)
{
	//max rpm = 642 -> 10,7 Hz -> max f_pulse = 128,4 Hz -> ~ min 20 samples / period -> f_timer = 2500 Hz
	float f_timer = HAL_RCC_GetPCLK2Freq() / (float)(drehzahl_messung_ptr->htim->Init.Prescaler) / (float)(drehzahl_messung_ptr->htim->Init.Period);
	float f_pulse = f_timer / (float)(drehzahl_messung_ptr->timer_cycle_count);
	drehzahl_messung_ptr->currentValue = f_pulse / (float)(MOTOR_PULSE_PER_ROTATION) * 60;
	drehzahl_messung_ptr->timer_cycle_count = 0;
}

static void press_enter_to_continue()
{
	getchar();
}

static void calibrate_set_center(Motor_t *motor_ptr)
{
	motor_ptr->calibration.center_pos_mm = motor_ptr->calibration.current_pos_mm;
}

static boolean_t endschalter_detected(IO_digitalPin_t *motor_endschalter)
{
	return (boolean_t) IO_digitalRead(motor_endschalter);
}

static int32_t convert_pulse_count_to_distance(int32_t pulse_count)
{
	return (uint32_t) (pulse_count / (float)(MOTOR_PULSE_PER_ROTATION) * MOTOR_DISTANCE_PER_ROTATION);
}

static void calibrate_set_endpoints(motor_calibration_t *calibration)
{
	calibration->end_pos_mm = convert_pulse_count_to_distance(calibration->current_pos_pulse_count)/2;
	calibration->current_pos_mm = calibration->end_pos_mm;
}

static void update_current_position(Motor_t *motor_ptr)
{
	motor_ptr->calibration.current_pos_mm = convert_pulse_count_to_distance(motor_ptr->calibration.current_pos_pulse_count) - motor_ptr->calibration.end_pos_mm;
}

/* Timer Callback implementation for rpm measurement --------------------------*/
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	motor_callback_get_rpm(&motor, htim);
}
*/




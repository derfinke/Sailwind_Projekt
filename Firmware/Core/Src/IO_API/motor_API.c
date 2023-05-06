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
static void _motor_calibrate_set_endpoints(motor_calibration_t *calibration);
static void _motor_calibrate_set_center(Motor_t *motor_ptr);
static boolean_t _motor_endschalter_detected(IO_digitalPin_t *motor_endschalter);
static int32_t _motor_convert_pulse_count_to_distance(int32_t pulse_count);
static void _motor_update_current_position(Motor_t *motor_ptr);


/* API function definitions -----------------------------------------------*/
Motor_t motor_init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim)
{
	Motor_t motor = {
			.operating_mode = motor_operating_mode_manual,
			.calibration = {
					.state = motor_calibration_state_0_init,
					.set_endpoints_state = motor_set_endpoints_state_0_init,
					.start_pulse_count = False,
					.current_pos_pulse_count = 0
			},
			.IN0 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN_0_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN1 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN_1_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN2 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN_2_Pin,
					.state = GPIO_PIN_RESET
			},
			.IN3 = {
					.GPIOx = GPIOD,
					.GPIO_Pin = IN_3_Pin,
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
							.GPIO_Pin = Drehzahl_DAC_OUT_Pin,
							.state = GPIO_PIN_RESET
					},
					.currentValue = 0,
					.htim = htim,
			},
			.OUT2_Fehler = {
					.GPIOx = GPIOF,
					.GPIO_Pin = OUT_1_Pin,
					.state = GPIO_PIN_RESET
			},
			.OUT3_Drehrichtung = {
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
		if (previous_state == GPIO_PIN_RESET && current_state == GPIO_PIN_SET) //rising edge of pulse
		{
			_motor_convert_timeStep_to_rpm(drehzahl_messung_ptr);
			if (motor_ptr->calibration.start_pulse_count)
			{
				if (motor_ptr->current_function == motor_function_rechtslauf)
				{
					motor_ptr->calibration.current_pos_pulse_count++;
				}
				else
				{
					motor_ptr->calibration.current_pos_pulse_count--;
				}
				if (motor_ptr->calibration.state >= motor_calibration_state_2_set_center_pos)
				{
					_motor_update_current_position(motor_ptr);
				}
			}
		}
	}
}

void motor_set_operating_mode(Motor_t *motor_ptr, motor_operating_mode_t operating_mode)
{
	boolean_t set_automatic = operating_mode == motor_operating_mode_automatic;
	boolean_t set_manual = operating_mode == motor_operating_mode_manual;
	boolean_t isCalibrated = motor_ptr->calibration.state == motor_calibration_state_3_done;
	if ((set_automatic AND isCalibrated) OR set_manual)
	{
		motor_ptr->operating_mode = operating_mode;
	}
	//ToDo
}

void motor_calibrate_state_machine(Motor_t *motor_ptr, LED_t *led_center_pos_set)
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
			_motor_calibrate_set_center(motor_ptr);
			break;
		case motor_calibration_state_3_done:
			//do nothing
			break;
	}
	//ToDo
}
/* private function definitions -----------------------------------------------*/
static void _motor_convert_timeStep_to_rpm(RPM_Measurement_t *drehzahl_messung_ptr)
{
	//max rpm = 642 -> 10,7 Hz -> max f_pulse = 128,4 Hz -> ~ min 20 samples / period -> f_timer = 2500 Hz
	uint32_t f_timer = (uint32_t) HAL_RCC_GetPCLK2Freq() / drehzahl_messung_ptr->htim->Init.Prescaler / drehzahl_messung_ptr->htim->Init.Period;
	uint32_t f_pulse = f_timer / drehzahl_messung_ptr->timer_cycle_count;
	drehzahl_messung_ptr->currentValue = f_pulse / MOTOR_PULSE_PER_ROTATION * 60;
	drehzahl_messung_ptr->timer_cycle_count = 0;
}

static void _motor_press_enter_to_continue()
{
	getchar();
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
				if (_motor_endschalter_detected(&motor_ptr->endschalter.vorne))
				{
					printf("//ToDo: motor_start_moving(motor_ptr, motor_function_rechtslauf);");
					calibration->start_pulse_count = True;
					calibration->set_endpoints_state = motor_set_endpoints_state_2_move_to_end_pos_hinten;
				}
				break;
			case motor_set_endpoints_state_2_move_to_end_pos_hinten:
				if (_motor_endschalter_detected(&motor_ptr->endschalter.hinten))
				{
					printf("//ToDo: motor_start_moving(motor_ptr, motor_function_linkslauf);");
					_motor_calibrate_set_endpoints(calibration);
					calibration->state = motor_calibration_state_2_set_center_pos;
				}
				break;
		}
	}
}

static void _motor_calibrate_set_center(Motor_t *motor_ptr)
{
	//ToDo
}

static boolean_t _motor_endschalter_detected(IO_digitalPin_t *motor_endschalter)
{
	return (boolean_t) IO_readDigitalIN(motor_endschalter);
}

static int32_t _motor_convert_pulse_count_to_distance(int32_t pulse_count)
{
	return pulse_count / MOTOR_PULSE_PER_ROTATION * MOTOR_DISTANCE_PER_ROTATION;
}

static void _motor_calibrate_set_endpoints(motor_calibration_t *calibration)
{
	calibration->max_distance_pulse_count = calibration->current_pos_pulse_count;
	calibration->max_distance_mm = _motor_convert_pulse_count_to_distance(calibration->max_distance_pulse_count);
	calibration->end_pos_mm = calibration->max_distance_mm/2;
	calibration->start_pos_mm = - calibration->end_pos_mm;
	calibration->current_pos_mm = calibration->end_pos_mm;
}

static void _motor_update_current_position(Motor_t *motor_ptr)
{
	motor_ptr->calibration.current_pos_mm = _motor_convert_pulse_count_to_distance(motor_ptr->calibration.current_pos_pulse_count) - motor_ptr->calibration.end_pos_mm;
}

/* Timer Callback implementation for rpm measurement --------------------------*/
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	motor_callback_get_rpm(&motor, htim);
}
*/




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
static void calibrate_set_endpoints(motor_calibration_t *calibration_ptr);
static void calibrate_set_center(Motor_t *motor_ptr);
static boolean_t endschalter_detected(IO_digitalPin_t *motor_endschalter_ptr);
static int32_t convert_pulse_count_to_distance(int32_t pulse_count);
static void update_current_position(Motor_t *motor_ptr);
static void start_rpm_measurement(Motor_t *motor_ptr);


/* API function definitions -----------------------------------------------*/
Motor_t motor_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr)
{
	Motor_t motor =
	{
			.operating_mode = IO_operating_mode_manual,
			.moving_state = motor_moving_state_aus,
			.current_function = motor_function_aus,
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
			.AIN_Drehzahl_Soll =
			{
					.hdac_ptr = hdac_ptr,
					.hdac_channel = DAC_CHANNEL_1,
					.maxConvertedValue = MOTOR_RPM_MAX,
					.currentConvertedValue = 0.0F,
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
					.htim_ptr = htim_ptr,
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

/* void motor_start_moving(Motor_t *motor_ptr, motor_moving_state_t direction)
 *  Description:
 *   - write digital motor Inputs to start motor movement in desired direction (motor_moving_state_rechtslauf / ...linkslauf)
 *   - save the new moving state to the motor reference
 */
void motor_start_moving(Motor_t *motor_ptr, motor_moving_state_t direction)
{
	motor_ptr->moving_state = direction;
	motor_set_function(motor_ptr, (motor_function_t)(direction));
	motor_set_function(motor_ptr, motor_function_speed1);
}

/* void motor_stop_moving(Motor_t *motor_ptr)
 *  Description:
 *   - write digital motor Inputs to stop the motor
 *   - save the new moving state to the motor reference
 */
void motor_stop_moving(Motor_t *motor_ptr)
{
	motor_ptr->moving_state = motor_moving_state_aus;
	motor_set_function(motor_ptr, motor_function_aus);
}

/* void motor_set_function(Motor_t *motor_ptr, motor_function_t function)
 *  Description:
 *   - convert the "function" enum (0..7) to a binary number between 00 and 11 to set the digital motor Inputs as following:
 *   - function |IN0 	IN1 |IN2	IN3	|meaning
 *     ---------------------------------------------------------
 *   		0	|0		0	|-		-	|aus
 *   		1	|1		0	|-		-	|rechtslauf
 *   		2	|0		1	|-		-	|linkslauf
 *   		3	|1		1	|-		-	|stopp mit haltemoment
 *   		4	|-		-	|0		0	|drehzahlvorgabe
 *   		5	|-		-	|1		0	|stromvorgabe
 *   		6	|-		-	|0		1	|speed1
 *   		7	|-		-	|1		1	|speed2
 *   - the digits are calculated separately in a for loop that does a bitwise "and" operation with the value 2 and the function value and checks, if the result is != 0
 *   - afterwards the function value is left shifted and the process is repeated for the second digit
 */
void motor_set_function(Motor_t *motor_ptr, motor_function_t function)
{
	motor_ptr->current_function = function;
	IO_digitalPin_t *motor_INs_ptr[] = {&motor_ptr->IN0, &motor_ptr->IN1, &motor_ptr->IN2, &motor_ptr->IN3};
	uint8_t pin_offset = (function >= 4) * 2; // 0 or 2 -> write IN0+0, IN1+0 or IN0+2, IN1+2
	int8_t function_bits = function - pin_offset * 2; //subtract 4 to function id if its >= 4 -> convert number {0..3} to binary in following for loop
	for (int i = 0; i < 2; i++)
	{ // write IN0 and IN1 (function in {0..3}) or IN2 and IN3 (function in {4..7}
		GPIO_PinState state = function_bits & 2 ? GPIO_PIN_SET : GPIO_PIN_RESET; // example: function = 3_d = 11_b, 2_d == 10_b -> 11 & 10 = 10  -> 10 != 0 -> 1
		uint8_t INx = i + pin_offset;											 //			 function << 1 -> 110_b             -> 110 & 10 = 10 -> 10 != 0 -> 1
		IO_digitalWrite(motor_INs_ptr[INx], state);
		function_bits <<= 1;
	}
}

/* void motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value)
 *  Description:
 *   -
 */
void motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value)
{
	motor_set_function(motor_ptr, motor_function_drehzahlvorgabe);
	IO_analogWrite(&motor_ptr->AIN_Drehzahl_Soll, rpm_value);
}

/* void motor_start_rpm_measurement(Motor_t *motor_ptr)
 *  Description:
 *   -
 */
void motor_start_rpm_measurement(Motor_t *motor_ptr)
{
	HAL_TIM_Base_Start_IT(motor_ptr->OUT1_Drehzahl_Messung.htim_ptr);
}

/* void motor_stop_rpm_measurement(Motor_t *motor_ptr)
 *  Description:
 *   -
 */
void motor_stop_rpm_measurement(Motor_t *motor_ptr)
{
	HAL_TIM_Base_Stop_IT(motor_ptr->OUT1_Drehzahl_Messung.htim_ptr);
}

/* void motor_teach_speed(Motor_t *motor_ptr, motor_function_t speed, uint32_t rpm_value, uint32_t tolerance)
 *  Description:
 *   -
 */
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

/* void motor_callback_get_rpm(Motor_t *motor_ptr, TIM_HandleTypeDef *htim_ptr)
 *  Description:
 *   -
 */
void motor_callback_get_rpm(Motor_t *motor_ptr, TIM_HandleTypeDef *htim_ptr)
{
	RPM_Measurement_t *drehzahl_messung_ptr = &motor_ptr->OUT1_Drehzahl_Messung;
	if (htim_ptr==drehzahl_messung_ptr->htim_ptr)
	{
		drehzahl_messung_ptr->timer_cycle_count++;
		if (IO_digitalRead_rising_edge(&drehzahl_messung_ptr->puls))
		{
			convert_timeStep_to_rpm(drehzahl_messung_ptr);
			if (motor_ptr->calibration.set_endpoints_state == motor_set_endpoints_state_2_move_to_end_pos_hinten || motor_ptr->calibration.is_calibrated)
			{
				switch(motor_ptr->moving_state)
				{
					case motor_moving_state_rechtslauf:
						motor_ptr->calibration.current_pos_pulse_count++; break;
					case motor_moving_state_linkslauf:
						motor_ptr->calibration.current_pos_pulse_count--; break;
					default:
						;
				}
				if (motor_ptr->calibration.state == motor_calibration_state_2_set_center_pos  || motor_ptr->calibration.is_calibrated)
				{
					update_current_position(motor_ptr);
				}
			}
		}
	}
}

/* void motor_set_operating_mode(Motor_t *motor_ptr, IO_operating_mode_t operating_mode)
 *  Description:
 *   -
 */
void motor_set_operating_mode(Motor_t *motor_ptr, IO_operating_mode_t operating_mode)
{
	boolean_t set_automatic = operating_mode == IO_operating_mode_automatic;
	boolean_t set_manual = operating_mode == IO_operating_mode_manual;
	boolean_t is_calibrated = motor_ptr->calibration.is_calibrated;
	if ((set_automatic && is_calibrated) || set_manual)
	{
		motor_ptr->operating_mode = operating_mode;
	}
}

/* void motor_button_calibrate_state_machine(Motor_t *motor_ptr, LED_t *led_center_pos_set_ptr)
 *  Description:
 *   -
 */
void motor_button_calibrate_state_machine(Motor_t *motor_ptr, LED_t *led_center_pos_set_ptr)
{
	switch(motor_ptr->calibration.state)
	{
		case motor_calibration_state_0_init:
			motor_ptr->calibration.state = motor_calibration_state_1_save_endpoints;
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

/* void motor_calibrate_state_machine_set_endpoints(Motor_t *motor_ptr)
 *  Description:
 *   -
 */
void motor_calibrate_state_machine_set_endpoints(Motor_t *motor_ptr)
{
	motor_calibration_t *calibration = &motor_ptr->calibration;
	if (calibration->state == motor_calibration_state_1_save_endpoints)
	{
		switch(calibration->set_endpoints_state)
		{
			case motor_set_endpoints_state_0_init:
				motor_start_moving(motor_ptr, motor_moving_state_linkslauf);
				calibration->set_endpoints_state = motor_set_endpoints_state_1_move_to_end_pos_vorne;
				break;
			case motor_set_endpoints_state_1_move_to_end_pos_vorne:
				if (endschalter_detected(&motor_ptr->endschalter.vorne))
				{
					start_rpm_measurement(motor_ptr);
					motor_start_moving(motor_ptr, motor_moving_state_rechtslauf);
					calibration->set_endpoints_state = motor_set_endpoints_state_2_move_to_end_pos_hinten;
				}
				break;
			case motor_set_endpoints_state_2_move_to_end_pos_hinten:
				if (endschalter_detected(&motor_ptr->endschalter.hinten))
				{
					motor_start_moving(motor_ptr, motor_moving_state_linkslauf);
					calibrate_set_endpoints(calibration);
					calibration->state = motor_calibration_state_2_set_center_pos;
				}
				break;
		}
	}
}

/* private function definitions -----------------------------------------------*/

/* static void convert_timeStep_to_rpm(RPM_Measurement_t *drehzahl_messung_ptr)
 *  Description:
 *   -
 */
static void convert_timeStep_to_rpm(RPM_Measurement_t *drehzahl_messung_ptr)
{
	//max rpm = 642 -> 10,7 Hz -> max f_pulse = 128,4 Hz -> ~ min 20 samples / period -> f_timer = 2500 Hz
	float f_timer = HAL_RCC_GetPCLK2Freq() / (float)(drehzahl_messung_ptr->htim_ptr->Init.Prescaler) / (float)(drehzahl_messung_ptr->htim_ptr->Init.Period);
	float f_pulse = f_timer / (float)(drehzahl_messung_ptr->timer_cycle_count);
	drehzahl_messung_ptr->currentValue = f_pulse / (float)(MOTOR_PULSE_PER_ROTATION) * 60;
	drehzahl_messung_ptr->timer_cycle_count = 0;
}

static void press_enter_to_continue()
{
	getchar();
}

/* static void calibrate_set_center(Motor_t *motor_ptr)
 *  Description:
 *   -
 */
static void calibrate_set_center(Motor_t *motor_ptr)
{
	motor_ptr->calibration.center_pos_mm = motor_ptr->calibration.current_pos_mm;
}

/* static boolean_t endschalter_detected(IO_digitalPin_t *motor_endschalter_ptr)
 *  Description:
 *   -
 */
static boolean_t endschalter_detected(IO_digitalPin_t *motor_endschalter_ptr)
{
	return (boolean_t) IO_digitalRead(motor_endschalter_ptr);
}

/* static int32_t convert_pulse_count_to_distance(int32_t pulse_count)
 *  Description:
 *   -
 */
static int32_t convert_pulse_count_to_distance(int32_t pulse_count)
{
	return (uint32_t) (pulse_count / (float)(MOTOR_PULSE_PER_ROTATION) * MOTOR_DISTANCE_PER_ROTATION);
}

/* static void calibrate_set_endpoints(motor_calibration_t *calibration_ptr)
 *  Description:
 *   -
 */
static void calibrate_set_endpoints(motor_calibration_t *calibration_ptr)
{
	calibration_ptr->end_pos_mm = convert_pulse_count_to_distance(calibration_ptr->current_pos_pulse_count)/2;
	calibration_ptr->current_pos_mm = calibration_ptr->end_pos_mm;
}

/* static void update_current_position(Motor_t *motor_ptr)
 *  Description:
 *   -
 */
static void update_current_position(Motor_t *motor_ptr)
{
	motor_ptr->calibration.current_pos_mm = convert_pulse_count_to_distance(motor_ptr->calibration.current_pos_pulse_count) - motor_ptr->calibration.end_pos_mm;
}


/* Timer Callback implementation for rpm measurement --------------------------*/

/* void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim_ptr)
 *  Description:
 *   -
 */
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim_ptr)
{
	motor_callback_get_rpm(&motor, htim_ptr);
}
*/




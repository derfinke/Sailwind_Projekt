/*
 * Motor.c
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#include "Motor.h"

/* private function prototypes -----------------------------------------------*/
static Motor_RPM_Measurement_t Motor_OUT1_init(TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel);
static IO_analogActuator_t Motor_AIN_init(DAC_HandleTypeDef *hdac_ptr);
static float Motor_pulse_to_rpm(float f_pulse);
static void Motor_press_enter_to_continue();


/* API function definitions -----------------------------------------------*/
Motor_t Motor_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel)
{
	Motor_t motor =
	{
			.current_function = Motor_function_aus,
			.INs =
			{
					IO_digital_Out_Pin_init(IN_0_GPIO_Port, IN_0_Pin, GPIO_PIN_RESET),
					IO_digital_Out_Pin_init(IN_1_GPIO_Port, IN_1_Pin, GPIO_PIN_RESET),
					IO_digital_Out_Pin_init(IN_2_GPIO_Port, IN_2_Pin, GPIO_PIN_RESET),
					IO_digital_Out_Pin_init(IN_3_GPIO_Port, IN_3_Pin, GPIO_PIN_RESET),

			},
			.AIN_Drehzahl_Soll = Motor_AIN_init(hdac_ptr),
			.OUT1_Drehzahl_Messung = Motor_OUT1_init(htim_ptr, htim_channel, htim_active_channel),
			.OUT2_Fehler = IO_digital_Pin_init(OUT_2_GPIO_Port, OUT_2_Pin),
			.OUT3_Drehrichtung = IO_digital_Pin_init(OUT_3_GPIO_Port, OUT_3_Pin),
	};
	Motor_start_rpm_measurement(&motor);
	return motor;
}

/* void motor_start_moving(Motor_t *motor_ptr, motor_moving_state_t direction)
 *  Description:
 *   - write digital motor Inputs to start motor movement in desired direction (motor_moving_state_rechtslauf / ...linkslauf)
 *   - save the new moving state to the motor reference
 */
void Motor_start_moving(Motor_t *motor_ptr, Motor_function_t direction)
{
	printf("motor start moving\r\n");
	Motor_set_function(motor_ptr, (Motor_function_t)(direction));
	Motor_set_function(motor_ptr, Motor_function_speed1);
}

/* void motor_stop_moving(Motor_t *motor_ptr)
 *  Description:
 *   - write digital motor Inputs to stop the motor
 *   - save the new moving state to the motor reference
 */
void Motor_stop_moving(Motor_t *motor_ptr)
{
	printf("motor stop moving\r\n");
	Motor_set_function(motor_ptr, Motor_function_aus);
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
void Motor_set_function(Motor_t *motor_ptr, Motor_function_t function)
{
	motor_ptr->current_function = function;
	uint8_t pin_offset = (function >= 4) * 2; // 0 or 2 -> write IN0+0, IN1+0 or IN0+2, IN1+2
	int8_t function_bits = function - pin_offset * 2; //subtract 4 to function id if its >= 4 -> convert number {0..3} to binary in following for loop
	for (int i = 0; i < 2; i++)
	{ // write IN0 and IN1 (function in {0..3}) or IN2 and IN3 (function in {4..7}
		GPIO_PinState state = function_bits & 2 ? GPIO_PIN_SET : GPIO_PIN_RESET; // example: function = 3_d = 11_b, 2_d == 10_b -> 11 & 10 = 10  -> 10 != 0 -> 1
		uint8_t IN_idx = i + pin_offset;											 //			 function << 1 -> 110_b             -> 110 & 10 = 10 -> 10 != 0 -> 1
		IO_digitalWrite(&motor_ptr->INs[IN_idx], state);
		function_bits <<= 1;
	}
}

/* void motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value)
 *  Description:
 *   - set digital INs of the motor to configure its engine speed
 *   - send an analog value (converted from rpm - valaue) to the analog Input of the motor
 */
void Motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value)
{
	Motor_set_function(motor_ptr, Motor_function_drehzahlvorgabe);
	IO_analogWrite(&motor_ptr->AIN_Drehzahl_Soll, rpm_value);
}

/* void motor_start_rpm_measurement(Motor_t *motor_ptr)
 *  Description:
 *   - start hal timer to trigger HAL_TIM_IC_CaptureCallback() function which calls linear_guide_callback_get_rpm()
 */
void Motor_start_rpm_measurement(Motor_t *motor_ptr)
{
	Motor_RPM_Measurement_t *rpm_ptr = &motor_ptr->OUT1_Drehzahl_Messung;
	HAL_TIM_IC_Start_IT(rpm_ptr->htim_ptr, rpm_ptr->htim_channel);
}

boolean_t Motor_callback_measure_rpm(Motor_t *motor_ptr)
{
	Motor_RPM_Measurement_t *rpm_ptr = &motor_ptr->OUT1_Drehzahl_Messung;
	TIM_HandleTypeDef *htim_ptr = rpm_ptr->htim_ptr;
	if (!(htim_ptr->Channel == rpm_ptr->active_channel))
	{
		return False;
	}
	if (!rpm_ptr->Is_First_Captured) // if the first rising edge is not captured
	{
		rpm_ptr->IC_Val1 = HAL_TIM_ReadCapturedValue(htim_ptr, TIM_CHANNEL_4); // read the first value
		rpm_ptr->Is_First_Captured = True;  // set the first captured as true
		return False;
	}
	// If the first rising edge is captured, now we will capture the second edge
	float diff;
	rpm_ptr->IC_Val2 = HAL_TIM_ReadCapturedValue(htim_ptr, TIM_CHANNEL_4);  // read second value
	if (rpm_ptr->IC_Val2 > rpm_ptr->IC_Val1)
	{
		diff = rpm_ptr->IC_Val2-rpm_ptr->IC_Val1;
	}
	else
	{
		diff = (htim_ptr->Init.Period - rpm_ptr->IC_Val1) + rpm_ptr->IC_Val2;
	}
	float ref_clock = HAL_RCC_GetPCLK1Freq() / (float)(htim_ptr->Init.Prescaler);
	float f_pulse = ref_clock/diff;
	rpm_ptr->rpm_value = Motor_pulse_to_rpm(f_pulse);

	__HAL_TIM_SET_COUNTER(htim_ptr, 0);  // reset the counter
	rpm_ptr->Is_First_Captured = False; // set it back to false
	return True;
}

/* void motor_stop_rpm_measurement(Motor_t *motor_ptr)
 *  Description:
 *   - stop hal timer
 */
void Motor_stop_rpm_measurement(Motor_t *motor_ptr)
{
	Motor_RPM_Measurement_t *rpm_ptr = &motor_ptr->OUT1_Drehzahl_Messung;
	HAL_TIM_IC_Stop_IT(rpm_ptr->htim_ptr, rpm_ptr->htim_channel);
}

/* void motor_teach_speed(Motor_t *motor_ptr, motor_function_t speed, uint32_t rpm_value, uint32_t tolerance)
 *  Description:
 *   - teach speed 1 or speed 2
 *   - probably not working yet!
 */
void Motor_teach_speed(Motor_t *motor_ptr, Motor_function_t speed, uint32_t rpm_value, uint32_t tolerance)
{
	printf("\nstoppt Motor...\n");
	Motor_set_function(motor_ptr, Motor_function_stopp_mit_haltemoment);
	printf("Enter drücken um Lernmodus einzuleiten, sobald Motor angehalten...\n");
	Motor_press_enter_to_continue();
	printf("leite Lernmodus ein...\n");
	for (int i=0; i<5; i++)
	{
		IO_digitalToggle(&motor_ptr->INs[2]);
		HAL_Delay(500);
	}
	printf("Lernmodus aktiviert, wenn rote LED schnell blinkt -> Enter um fortzufahren...\n");
	Motor_press_enter_to_continue();
	printf("Motor auf Zieldrehzahl bringen...\n");
	Motor_set_rpm(motor_ptr, rpm_value);
	printf("wartet bis Zieldrehzahl erreicht wurde...\n");
	while((rpm_value - motor_ptr->OUT1_Drehzahl_Messung.rpm_value) > tolerance);
	printf("Zieldrehzahl erreicht -> Setze motor_ptr speed %d auf %ld rpm\n", speed-5, rpm_value);
	Motor_set_function(motor_ptr, speed);
	printf("\nstoppt Motor...\n");
	Motor_set_function(motor_ptr, Motor_function_stopp_mit_haltemoment);
	printf("Enter drücken um Lernmodus zu verlassen, sobald Motor angehalten...\n");
	Motor_press_enter_to_continue();
	printf("verlasse Lernmodus...\n");
	for (int i=0; i<5; i++)
	{
		IO_digitalToggle(&motor_ptr->INs[2]);
		HAL_Delay(500);
	}
	printf("Lernmodus deaktiviert, wenn rote LED langsam blinkt -> Enter um fortzufahren...\n");
	Motor_press_enter_to_continue();
}

/* private function definitions -----------------------------------------------*/

static Motor_RPM_Measurement_t Motor_OUT1_init(TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel)
{
	Motor_RPM_Measurement_t rpm_measurement =
	{
			.IC_Val1 = 0,
			.IC_Val2 = 0,
			.Is_First_Captured = False,
			.htim_ptr = htim_ptr,
			.htim_channel = htim_channel,
			.active_channel = htim_active_channel,
			.rpm_value = 0
	};
	return rpm_measurement;
}

static IO_analogActuator_t Motor_AIN_init(DAC_HandleTypeDef *hdac_ptr)
{
	IO_analogActuator_t rpm_setting =
	{
			.hdac_ptr = hdac_ptr,
			.hdac_channel = DAC_CHANNEL_1,
			.maxConvertedValue = MOTOR_RPM_MAX,
			.limitConvertedValue = MOTOR_RPM_LIMIT,
			.currentConvertedValue = 0.0F,
			.dac_value = 0
	};
	return rpm_setting;
}

/* static float motor_pulse_to_rpm(float f_pulse)
 *  Description:
 *   - convert pulse frequency to rpm value via pulse per rotation constant (12) and return the result
 */
static float Motor_pulse_to_rpm(float f_pulse)
{
	return f_pulse / (float)(MOTOR_PULSE_PER_ROTATION) * 60;
}

static void Motor_press_enter_to_continue()
{
	getchar();
}




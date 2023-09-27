/*
 * Motor.c
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#include "Motor.h"

/* defines ------------------------------------------------------------*/
#define MOTOR_RPM_MAX 4378.44F // corresponds to ANALOG_MAX (4096) and max output voltage of 10.7 V -> 4092 rpm corresponds to 10 V (BG 45 SI manual)
#define MOTOR_RPM_LIMIT 700.0F// estimated normal speed for linear guide
#define MOTOR_NORMAL_SPEED 1600
#define MOTOR_RAMP_STEP_MS 20
#define MOTOR_RAMP_STEP_RPM 15
#define MOTOR_RAMP_SPEED_UP 1
#define MOTOR_RAMP_SLOW_DOWN -1

/* private function prototypes -----------------------------------------------*/
static IO_analogActuator_t Motor_AIN_init(DAC_HandleTypeDef *hdac_ptr);
static void Motor_press_enter_to_continue();


/* API function definitions -----------------------------------------------*/
Motor_t Motor_init(DAC_HandleTypeDef *hdac_ptr) {
	Motor_t motor = {
			.current_function = Motor_function_stop,
			.INs = {
					IO_digital_Out_Pin_init(IN_0_GPIO_Port, IN_0_Pin, GPIO_PIN_RESET),
					IO_digital_Out_Pin_init(IN_1_GPIO_Port, IN_1_Pin, GPIO_PIN_RESET),
					IO_digital_Out_Pin_init(IN_2_GPIO_Port, IN_2_Pin, GPIO_PIN_RESET),
					IO_digital_Out_Pin_init(IN_3_GPIO_Port, IN_3_Pin, GPIO_PIN_RESET),
			},
			.AIN_set_rpm = Motor_AIN_init(hdac_ptr),
			.OUT2_error = IO_digital_Pin_init(OUT_2_GPIO_Port, OUT_2_Pin),
			.OUT3_rot_dir = IO_digital_Pin_init(OUT_3_GPIO_Port, OUT_3_Pin),
			.rpm_set_point = 0,
			.normal_rpm = MOTOR_NORMAL_SPEED,
			.ramp_final_rpm = 0,
			.ramp_activated = False
	};
	return motor;
}
/* void motor_start_moving(Motor_t *motor_ptr, motor_moving_state_t direction)
 *  Description:
 *   - write digital motor Inputs to start motor movement in desired direction (motor_moving_state_rechtslauf / ...linkslauf)
 */
void Motor_start_moving(Motor_t *motor_ptr, Motor_function_t direction) {
	printf("motor start moving\r\n");
	motor_ptr->rpm_set_point = 0;
	Motor_set_function(motor_ptr, direction);
	motor_ptr->ramp_final_rpm = motor_ptr->normal_rpm;
	motor_ptr->ramp_activated = True;
}
/* void motor_stop_moving(Motor_t *motor_ptr)
 *  Description:
 *   - write digital motor Inputs to stop the motor
 *   - save the new moving state to the motor reference
 */
void Motor_stop_moving(Motor_t *motor_ptr) {
	printf("motor stop moving\r\n");
	Motor_set_rpm(motor_ptr, 0);
}

int8_t Motor_speed_ramp(Motor_t *motor_ptr)
{
	if (!motor_ptr->ramp_activated)
	{
		return MOTOR_RAMP_INACTIVE;
	}
	if (motor_ptr->rpm_set_point >= motor_ptr->ramp_final_rpm)
	{
		Motor_set_rpm(motor_ptr, motor_ptr->ramp_final_rpm);
		motor_ptr->ramp_activated = False;
		return MOTOR_RAMP_DONE;
	}
	if ((HAL_GetTick()-motor_ptr->ramp_last_step_ms) < MOTOR_RAMP_STEP_MS)
	{
		return MOTOR_RAMP_WAIT;
	}
	Motor_set_rpm(motor_ptr, motor_ptr->rpm_set_point + MOTOR_RAMP_STEP_RPM);
	motor_ptr->ramp_last_step_ms = HAL_GetTick();
	return MOTOR_RAMP_NEXT_STEP;
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
void Motor_set_function(Motor_t *motor_ptr, Motor_function_t function) {
	motor_ptr->current_function = function;
	uint8_t pin_offset = (function >= 4) * 2;  // 0 or 2 -> write IN0+0, IN1+0 or IN0+2, IN1+2
	int8_t function_bits = function - pin_offset * 2;  //subtract 4 to function id if its >= 4 -> convert number {0..3} to binary in following for loop
	for (int i = 0; i < 2; i++) {  // write IN0 and IN1 (function in {0..3}) or IN2 and IN3 (function in {4..7}
		GPIO_PinState state = function_bits & 2 ? GPIO_PIN_SET : GPIO_PIN_RESET;  // example: function = 3_d = 11_b, 2_d == 10_b -> 11 & 10 = 10  -> 10 != 0 -> 1
		uint8_t IN_idx = i + pin_offset;  //       function << 1 -> 110_b             -> 110 & 10 = 10 -> 10 != 0 -> 1
		IO_digitalWrite(&motor_ptr->INs[IN_idx], state);
		function_bits <<= 1;
	}
}

/* void Motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value)
 *  Description:
 *   - set digital INs of the motor to configure its engine speed
 *   - send an analog value (converted from rpm - valaue) to the analog Input of the motor
 *   - or set default speeds
 */
void Motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value)
{
    motor_ptr->rpm_set_point = rpm_value;
    IO_analogWrite(&motor_ptr->AIN_set_rpm, (float) motor_ptr->rpm_set_point);
    Motor_function_t motor_function = Motor_function_velocity_setting;
    if (rpm_value == 0)
    {
    	motor_function = Motor_function_stop;
    }
    Motor_set_function(motor_ptr, motor_function);
}

boolean_t Motor_error(Motor_t *motor_ptr) {
	if (IO_digitalRead(&motor_ptr->OUT2_error)) {
		Motor_stop_moving(motor_ptr);
		return True;
	}
	return False;
}

/* void motor_teach_speed(Motor_t *motor_ptr, motor_function_t speed, uint32_t rpm_value, uint32_t tolerance)
 *  Description:
 *   - teach speed 1 or speed 2
 *   - probably not working yet!
 */
void Motor_teach_speed(Motor_t *motor_ptr, Motor_function_t speed, uint16_t rpm_value, uint32_t tolerance)
{
	printf("\nstoppt Motor...\n");
	Motor_set_function(motor_ptr, Motor_function_stop_holding_torque);
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
	printf("wartet bis Zieldrehzahl erreicht wurde... -> Enter um fortzufahren\n");
	Motor_press_enter_to_continue();
	printf("Zieldrehzahl erreicht -> Setze motor_ptr speed %d auf %d rpm\n", speed-5, rpm_value);
	Motor_set_function(motor_ptr, speed);
	printf("\nstoppt Motor...\n");
	Motor_set_function(motor_ptr, Motor_function_stop_holding_torque);
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

static void Motor_press_enter_to_continue()
{
	getchar();
}




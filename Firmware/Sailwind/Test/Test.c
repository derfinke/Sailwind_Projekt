/*
 * Test.c
 *
 *  Created on: 24.07.2023
 *      Author: Bene
 */

#include "Test.h"

static void Test_switch_test_ID(UART_HandleTypeDef *huart_ptr, uint16_t test_ID, Manual_Control_t *mc_ptr);
static void Test_endswitch(UART_HandleTypeDef *huart_ptr, Manual_Control_t *mc_ptr);
static void Test_LED(UART_HandleTypeDef *huart_ptr, Manual_Control_t *mc_ptr);
static void Test_Button(UART_HandleTypeDef *huart_ptr, Manual_Control_t *mc_ptr);
static void Test_Motor(UART_HandleTypeDef *huart_ptr, Manual_Control_t *mc_ptr);
static void Test_FRAM(UART_HandleTypeDef *huart_ptr);

void Test_uart_poll(UART_HandleTypeDef *huart_ptr, char *Rx_buffer, Manual_Control_t *mc_ptr)
{
	if (UART_receive(huart_ptr, Rx_buffer, TEST_ID_SIZE))
	{
		uint16_t test_ID = atoi(Rx_buffer);
		Test_switch_test_ID(huart_ptr, test_ID, mc_ptr);
	}
}

static void Test_switch_test_ID(UART_HandleTypeDef *huart_ptr, uint16_t test_ID, Manual_Control_t *mc_ptr)
{
	Motor_t *motor_ptr = &mc_ptr->lg_ptr->motor;
	UART_transmit_ln_int(huart_ptr, "Message Received: %d", test_ID);
	switch (test_ID)
	{
		case 1:
			Test_LED(huart_ptr, mc_ptr);
			break;
		case 20 ... 27:
			Motor_set_function(motor_ptr, (Motor_function_t)(test_ID - 20));
			break;
		case 30000 ... 33000:
			Motor_set_rpm(motor_ptr, test_ID - 30000);
			break;
		case 4:
			Test_endswitch(huart_ptr, mc_ptr);
			break;
		case 5:
			Test_Motor(huart_ptr, mc_ptr);
			break;
		case 51:
			UART_transmit_ln_int(huart_ptr, "motor error: %d", IO_digitalRead(&motor_ptr->OUT2_error));
			break;
		case 52:
			UART_transmit_ln_int(huart_ptr, "motor direction: %d", IO_digitalRead(&motor_ptr->OUT3_rot_dir));
			break;
		case 6:
			Test_Button(huart_ptr, mc_ptr);
			break;
		case 7:
			Test_FRAM(huart_ptr);
			break;
		default:
			UART_transmit_ln(huart_ptr, "no valid test ID!");
			break;
	}
	UART_transmit_ln_int(huart_ptr, "Test %d done!", test_ID);
}

static void Test_endswitch(UART_HandleTypeDef *huart_ptr, Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	UART_transmit_ln(huart_ptr, "switch operating mode button to start motor");
	while (!Button_state_changed(&mc_ptr->buttons.switch_mode));

	Linear_Guide_move(lg_ptr, Loc_movement_forward);
	while (!Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.front));

	Linear_Guide_move(lg_ptr, Loc_movement_backwards);
	while (!Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.back));

	Linear_Guide_move(lg_ptr, Loc_movement_forward);
	UART_transmit_ln(huart_ptr, "switch operating mode button to stop motor");
	while (!Button_state_changed(&mc_ptr->buttons.switch_mode));
}

static void Test_LED(UART_HandleTypeDef *huart_ptr, Manual_Control_t *mc_ptr)
{
	LG_LEDs_t *leds_ptr = &mc_ptr->lg_ptr->leds;
	for (LED_State_t state = LED_OFF; state <= LED_ON; state++)
	{
		UART_transmit_ln_int(huart_ptr, "switch operating mode button to set LEDs state: %d", state);
		while (!Button_state_changed(&mc_ptr->buttons.switch_mode));
		LED_switch(&leds_ptr->automatic, state);
		LED_switch(&leds_ptr->manual, state);
		LED_switch(&leds_ptr->rollung, state);
		LED_switch(&leds_ptr->trimmung, state);
		LED_switch(&leds_ptr->center_pos_set, state);
		LED_switch(&leds_ptr->error, state);
	}
}

static void Test_Button(UART_HandleTypeDef *huart_ptr, Manual_Control_t *mc_ptr)
{
	LED_t *led_ptr = &mc_ptr->lg_ptr->leds.center_pos_set;
	UART_transmit_ln(huart_ptr, "set and reset buttons and check, if center pos set led is switched on and off");

	UART_transmit_ln(huart_ptr, "1. switch mode button");
	while (!Button_state_changed(&mc_ptr->buttons.switch_mode));
	LED_switch(led_ptr, LED_ON);
	while (!Button_state_changed(&mc_ptr->buttons.switch_mode));
	LED_switch(led_ptr, LED_OFF);

	UART_transmit_ln(huart_ptr, "2. move backwards button");
	while (!Button_state_changed(&mc_ptr->buttons.move_backwards));
	LED_switch(led_ptr, LED_ON);
	while (!Button_state_changed(&mc_ptr->buttons.move_backwards));
	LED_switch(led_ptr, LED_OFF);

	UART_transmit_ln(huart_ptr, "3. move forward button");
	while (!Button_state_changed(&mc_ptr->buttons.move_forward));
	LED_switch(led_ptr, LED_ON);
	while (!Button_state_changed(&mc_ptr->buttons.move_forward));
	LED_switch(led_ptr, LED_OFF);

	UART_transmit_ln(huart_ptr, "4. localize button");
	while (!Button_state_changed(&mc_ptr->buttons.localize));
	LED_switch(led_ptr, LED_ON);
	while (!Button_state_changed(&mc_ptr->buttons.localize));
	LED_switch(led_ptr, LED_OFF);
}

static void Test_Motor(UART_HandleTypeDef *huart_ptr, Manual_Control_t *mc_ptr)
{
	Motor_t *motor_ptr = &mc_ptr->lg_ptr->motor;
	if (Motor_error(motor_ptr))
	{
		UART_transmit_ln(huart_ptr, "Motor Error! -> Test failed");
		return;
	}
	motor_ptr->normal_rpm = MOTOR_RPM_SPEED_1;
	UART_transmit_ln(huart_ptr, "switch button to start motor move clock wise with speed 1");
	while (!Button_state_changed(&mc_ptr->buttons.switch_mode));
	Motor_start_moving(motor_ptr, Motor_function_cw_rotation);
	while (IO_digitalRead(&motor_ptr->OUT3_rot_dir) != MOTOR_DIRECTION_CW);
	UART_transmit_ln(huart_ptr, "detected clockwise rotation!");

	UART_transmit_ln(huart_ptr, "switch button to change direction to counter clock wise");
	while (!Button_state_changed(&mc_ptr->buttons.switch_mode));
	Motor_start_moving(motor_ptr, Motor_function_ccw_rotation);
	while (IO_digitalRead(&motor_ptr->OUT3_rot_dir) != MOTOR_DIRECTION_CCW);
	UART_transmit_ln(huart_ptr, "detected counter clockwise rotation!");

	UART_transmit_ln(huart_ptr, "switch button to accelerate motor");
	while (!Button_state_changed(&mc_ptr->buttons.switch_mode));
	for (uint16_t rpm_value=MOTOR_RPM_SPEED_1; rpm_value <= MOTOR_RPM_SPEED_2; rpm_value+=15)
	{
		motor_ptr->normal_rpm = rpm_value;
		Motor_start_moving(motor_ptr, Motor_function_cw_rotation);
		HAL_Delay(2000);
		UART_transmit_ln_int(huart_ptr, "rpm_set_point: %d", rpm_value);
	}
	UART_transmit_ln(huart_ptr, "switch button to stop motor");
	while (!Button_state_changed(&mc_ptr->buttons.switch_mode));
	Motor_stop_moving(motor_ptr);
}

static void Test_FRAM(UART_HandleTypeDef *huart_ptr)
{
	char serial_buffer[LOC_SERIAL_SIZE];
	Localization_t test_loc = {
			.state = Loc_state_5_center_pos_set,
			.pulse_count = -1234,
			.end_pos_mm = 700,
			.center_pos_mm = 10
	};
	Localization_t loc = test_loc;

	UART_transmit_ln(huart_ptr, "save Localization:");
	Localization_serialize(loc, serial_buffer);
	UART_transmit_ln(huart_ptr, serial_buffer);
	Linear_Guide_safe_Localization(loc);

	HAL_Delay(5);

	UART_transmit_ln(huart_ptr, "read Localization:");
	loc = Linear_Guide_read_Localization();
	Localization_serialize(loc, serial_buffer);
	UART_transmit_ln(huart_ptr, serial_buffer);

	if (test_loc.state == loc.state &&
		test_loc.pulse_count == loc.pulse_count &&
		test_loc.end_pos_mm == loc.end_pos_mm &&
		test_loc.pulse_count == loc.pulse_count)
	{
		UART_transmit_ln(huart_ptr, "FRAM test passed");
	}
	else
	{
		UART_transmit_ln(huart_ptr, "FRAM test failed");
	}
}

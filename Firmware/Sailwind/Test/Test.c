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
			Motor_set_rpm(motor_ptr, test_ID - 30000, True);
			break;

		case 4:
			Test_endswitch(huart_ptr, mc_ptr);
			break;

		case 511:
			Motor_start_rpm_measurement(motor_ptr);
			break;
		case 512:
			UART_transmit_ln_float(huart_ptr, "motor rpm: %.2f", motor_ptr->OUT1_Drehzahl_Messung.rpm_value);
			break;
		case 52:
			UART_transmit_ln_int(huart_ptr, "motor error: %d", IO_digitalRead(&motor_ptr->OUT2_Fehler));
			break;
		case 53:
			UART_transmit_ln_int(huart_ptr, "motor direction: %d", IO_digitalRead(&motor_ptr->OUT3_Drehrichtung));
			break;
		default:
			UART_transmit_ln(huart_ptr, "no valid test ID!");
			break;
	}
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
	UART_transmit_ln(huart_ptr, "end switch Test done!");
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
	UART_transmit_ln(huart_ptr, "LED Test done!");
}

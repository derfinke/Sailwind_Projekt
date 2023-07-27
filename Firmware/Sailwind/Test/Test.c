/*
 * Test_API.c
 *
 *  Created on: 24.07.2023
 *      Author: Bene
 */

#include "Test.h"

static void read_endschalter(linear_guide_endschalter_t *endschalter_ptr, uint16_t test_ID);
static void switch_test_ID(UART_HandleTypeDef *huart, uint16_t test_ID, LED_bar_t *led_bar_ptr, Linear_guide_t *linear_guide_ptr);

void test_uart_poll(UART_HandleTypeDef *huart_ptr, uint8_t *Rx_buffer, LED_bar_t *led_bar_ptr, Linear_guide_t *linear_guide_ptr)
{
	if (__HAL_UART_GET_FLAG(huart_ptr, UART_FLAG_RXNE) == SET)
	{
		HAL_UART_Receive(huart_ptr, Rx_buffer, 5, 1);
		uint16_t test_ID = atoi((char*) Rx_buffer);
		switch_test_ID(huart_ptr, test_ID, led_bar_ptr, linear_guide_ptr);
	}
}

static void switch_test_ID(UART_HandleTypeDef *huart, uint16_t test_ID, LED_bar_t *led_bar_ptr, Linear_guide_t *linear_guide_ptr)
{
	Motor_t *motor_ptr = &linear_guide_ptr->motor;
	printf("Message Received: %d\r\n", test_ID);
	switch (test_ID)
	{
		case 11:
			LED_toggle(&led_bar_ptr->motor_error); break;
		case 12:
			LED_toggle_sail_adjustment_mode(led_bar_ptr); break;
		case 131:
			LED_set_operating_mode(led_bar_ptr, IO_operating_mode_manual); break;
		case 132:
			LED_set_operating_mode(led_bar_ptr, IO_operating_mode_automatic); break;
		case 14:
			LED_toggle(&led_bar_ptr->center_pos_set); break;

		case 20 ... 27:
			motor_set_function(motor_ptr, (motor_function_t)(test_ID - 20)); break;
		case 30000 ... 33000:
			motor_set_rpm(motor_ptr, test_ID - 30000); break;
		case 41 ... 42: ;
			read_endschalter(&linear_guide_ptr->endschalter, test_ID);
			break;
		default:
			printf("no valid test ID!\r\n");
			break;
	}
}

static void read_endschalter(linear_guide_endschalter_t *endschalter_ptr, uint16_t test_ID)
{
	char *endschalter_ID;
	GPIO_PinState state;
	if (test_ID == 41) {
		endschalter_ID = "vorne";
		state = IO_digitalRead(&endschalter_ptr->vorne);
	}
	else {
		endschalter_ID = "hinten";
		state = IO_digitalRead(&endschalter_ptr->hinten);
	}
	printf("Endschalter %s: %d\r\n", endschalter_ID, state);
}

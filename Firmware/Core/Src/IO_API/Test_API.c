/*
 * Test_API.c
 *
 *  Created on: 24.07.2023
 *      Author: Bene
 */

#include "Test_API.h"
#include <stdlib.h>
#include <string.h>

static void read_endschalter(char *Tx_data, linear_guide_endschalter_t *endschalter_ptr, uint16_t test_ID);
static void uart_transmit(UART_HandleTypeDef *huart, char *Tx_data);


void switch_test_ID(UART_HandleTypeDef *huart, uint16_t test_ID, LED_bar_t *led_bar_ptr, Linear_guide_t *linear_guide_ptr)
{
	Motor_t *motor_ptr = &linear_guide_ptr->motor;
	char *Tx_data = "";
	sprintf(Tx_data, "Message Received: %d", test_ID);
	uart_transmit(huart, Tx_data);
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
			char * Tx_data = "";
			read_endschalter(Tx_data, &linear_guide_ptr->endschalter, test_ID);
			uart_transmit(huart, Tx_data);
			break;
		default:
			uart_transmit(huart, "no valid test ID!");
			break;
	}
}

static void read_endschalter(char *Tx_data, linear_guide_endschalter_t *endschalter_ptr, uint16_t test_ID)
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
	sprintf(Tx_data, "Endschalter %s: %d", endschalter_ID, state);
}

static void uart_transmit(UART_HandleTypeDef *huart, char *Tx_data)
{
	char *Tx_data_line = "";
	sprintf(Tx_data_line, "%s\r\n", Tx_data);
	HAL_UART_Transmit(huart, (uint8_t *)(Tx_data_line), sizeof(Tx_data_line), 100);
}


/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	test_uart_receive_test_ID_Callback(huart, Rx_data, &led_bar, &linear_guide)
}
*/

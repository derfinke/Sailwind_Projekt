/*
 * Test.c
 *
 *  Created on: 24.07.2023
 *      Author: Bene
 */

#include "Test.h"

static void Test_read_endswitch(Linear_Guide_t *lg_ptr, uint16_t test_ID);
static void Test_switch_test_ID(UART_HandleTypeDef *huart, uint16_t test_ID, Linear_Guide_t *lg_ptr);
static void Test_uart_transmit(UART_HandleTypeDef *huart, char *Tx_data);
static void Test_uart_transmit_int(UART_HandleTypeDef *huart, char *f_string, int32_t value);
static void Test_uart_transmit_float(UART_HandleTypeDef *huart, char *f_string, float value);

void Test_uart_poll(UART_HandleTypeDef *huart_ptr, uint8_t *Rx_buffer, Linear_Guide_t *lg_ptr)
{
	if (__HAL_UART_GET_FLAG(huart_ptr, UART_FLAG_RXNE) == SET)
	{
		HAL_UART_Receive(huart_ptr, Rx_buffer, 5, 1);
		uint16_t test_ID = atoi((char*) Rx_buffer);
		Test_switch_test_ID(huart_ptr, test_ID, lg_ptr);
	}
}

static void Test_switch_test_ID(UART_HandleTypeDef *huart, uint16_t test_ID, Linear_Guide_t *lg_ptr)
{
	Motor_t *motor_ptr = &lg_ptr->motor;
	Test_uart_transmit_int(huart, "Message Received: %d", test_ID);
	switch (test_ID)
	{
		case 11:
			LED_toggle(&lg_ptr->leds.error);
			break;
		case 121:
			Linear_Guide_Test_LED_set_sail_adjustment_mode(lg_ptr, LG_sail_adjustment_mode_rollung);
			break;
		case 122:
			Linear_Guide_Test_LED_set_sail_adjustment_mode(lg_ptr, LG_sail_adjustment_mode_trimmung);
			break;
		case 131:
			Linear_Guide_Test_LED_set_operating_mode(lg_ptr, LG_operating_mode_manual);
			break;
		case 132:
			Linear_Guide_Test_LED_set_operating_mode(lg_ptr, LG_operating_mode_automatic);
			break;
		case 14:
			LED_toggle(&lg_ptr->leds.center_pos_set);
			break;

		case 20 ... 27:
			Motor_set_function(motor_ptr, (Motor_function_t)(test_ID - 20));
			break;

		case 30000 ... 33000:
			Motor_set_rpm(motor_ptr, test_ID - 30000);
			break;
		case 41 ... 42: ;
			Test_read_endswitch(lg_ptr, test_ID);
			break;

		case 511:
			Motor_start_rpm_measurement(motor_ptr);
			break;
		case 512:
			Test_uart_transmit_float(huart, "motor rpm: %.2f", motor_ptr->OUT1_Drehzahl_Messung.rpm_value);
			break;
		case 52:
			Test_uart_transmit_int(huart, "motor error: %d", IO_digitalRead(&motor_ptr->OUT2_Fehler));
			break;
		case 53:
			Test_uart_transmit_int(huart, "motor direction: %d", IO_digitalRead(&motor_ptr->OUT3_Drehrichtung));
			break;
		default:
			Test_uart_transmit(huart, "no valid test ID!");
			break;
	}
}

static void Test_read_endswitch(Linear_Guide_t *lg_ptr, uint16_t test_ID)
{
	char *endswitch_ID;
	boolean_t is_active;
	if (test_ID == 41) {
		endswitch_ID = "front";
		is_active = Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.front);
	}
	else {
		endswitch_ID = "back";
		is_active = Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.back);
	}
	printf("End Switch %s active: %d\r\n", endswitch_ID, is_active);
}

static void Test_uart_transmit(UART_HandleTypeDef *huart, char *Tx_data)
{
	char *Tx_data_line = "";
	sprintf(Tx_data_line, "%s\r\n", Tx_data);
	HAL_UART_Transmit(huart, (uint8_t *)(Tx_data_line), sizeof(Tx_data_line), 100);
}

static void Test_uart_transmit_int(UART_HandleTypeDef *huart, char *f_string, int32_t value)
{
	char *Tx_data = "";
	sprintf(Tx_data, f_string, value);
	Test_uart_transmit(huart, Tx_data);
}

static void Test_uart_transmit_float(UART_HandleTypeDef *huart, char *f_string, float value)
{
	char *Tx_data = "";
	sprintf(Tx_data, f_string, value);
	Test_uart_transmit(huart, Tx_data);
}

/*
 * Test.c
 *
 *  Created on: 24.07.2023
 *      Author: Bene
 */

#include "Test.h"

static void Test_read_endswitch(UART_HandleTypeDef *huart_ptr, Linear_Guide_t *lg_ptr, uint16_t test_ID);
static void Test_switch_test_ID(UART_HandleTypeDef *huart_ptr, uint16_t test_ID, Linear_Guide_t *lg_ptr);

void Test_uart_poll(UART_HandleTypeDef *huart_ptr, char *Rx_buffer, Linear_Guide_t *lg_ptr)
{
	if (UART_receive(huart_ptr, Rx_buffer, TEST_ID_SIZE))
	{
		uint16_t test_ID = atoi(Rx_buffer);
		Test_switch_test_ID(huart_ptr, test_ID, lg_ptr);
	}
}

static void Test_switch_test_ID(UART_HandleTypeDef *huart_ptr, uint16_t test_ID, Linear_Guide_t *lg_ptr)
{
	Motor_t *motor_ptr = &lg_ptr->motor;
	UART_transmit_ln_int(huart_ptr, "Message Received: %d", test_ID);
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
			Test_read_endswitch(huart_ptr, lg_ptr, test_ID);
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

static void Test_read_endswitch(UART_HandleTypeDef *huart_ptr, Linear_Guide_t *lg_ptr, uint16_t test_ID)
{
	char *f_string;
	boolean_t is_active;
	if (test_ID == 41) {
		f_string = "End Switch front active: %d";
		is_active = Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.front);
	}
	else {
		f_string = "End Switch back active: %d";
		is_active = Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.back);
	}
	UART_transmit_ln_int(huart_ptr, f_string, is_active);

}

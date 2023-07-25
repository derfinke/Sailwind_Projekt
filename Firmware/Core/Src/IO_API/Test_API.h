/*
 * Test_API.h
 *
 *  Created on: Jul 24, 2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_TEST_API_H_
#define SRC_IO_API_TEST_API_H_

#include "button_API.h"
#include "linear_guide_api.h"

void test_uart_receive_test_ID_Callback(UART_HandleTypeDef *huart, char *Rx_data, LED_bar_t *led_bar_ptr, Linear_guide_t *linear_guide_ptr);
void switch_test_ID(UART_HandleTypeDef *huart, uint16_t test_ID, LED_bar_t *led_bar_ptr, Linear_guide_t *linear_guide_ptr);
#endif /* SRC_IO_API_TEST_API_H_ */

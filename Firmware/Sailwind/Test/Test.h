/*
 * Test_API.h
 *
 *  Created on: Jul 24, 2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_TEST_API_H_
#define SRC_IO_API_TEST_API_H_

#include "../Linear_Guide/Linear_Guide.h"
#include <stdlib.h>

void test_uart_poll(UART_HandleTypeDef *huart_ptr, uint8_t *Rx_buffer, LED_bar_t *led_bar_ptr, Linear_guide_t *linear_guide_ptr);

#endif /* SRC_IO_API_TEST_API_H_ */

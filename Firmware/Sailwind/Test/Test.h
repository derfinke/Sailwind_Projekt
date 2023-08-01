/*
 * Test.h
 *
 *  Created on: Jul 24, 2023
 *      Author: Bene
 */

#ifndef TEST_TEST_H_
#define TEST_TEST_H_

#include <stdlib.h>
#include "../Linear_Guide/Linear_Guide.h"
#include "../UART/UART.h"

#define TEST_ID_SIZE 5

void Test_uart_poll(UART_HandleTypeDef *huart_ptr, char *Rx_buffer, Linear_Guide_t *lg_ptr);

#endif /* TEST_TEST_H_ */

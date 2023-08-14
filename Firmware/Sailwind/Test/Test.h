/*
 * Test.h
 *
 *  Created on: Jul 24, 2023
 *      Author: Bene
 */

#ifndef TEST_TEST_H_
#define TEST_TEST_H_

#include <stdlib.h>
#include "Manual_Control.h"
#include "UART.h"

#define TEST_ID_SIZE 5

void Test_uart_poll(UART_HandleTypeDef *huart_ptr, char *Rx_buffer, Manual_Control_t *mc_ptr);

#endif /* TEST_TEST_H_ */

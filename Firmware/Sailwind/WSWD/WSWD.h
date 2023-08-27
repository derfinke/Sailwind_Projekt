/*
 * WSWD.h
 *
 *  Created on: Aug 26, 2023
 *      Author: nicof
 */

#ifndef WSWD_WSWD_H_
#define WSWD_WSWD_H_

uint8_t WSWD_send_without_param(char* command);
uint8_t WSWD_send_with_param(char* command, char* param);
uint8_t WSWD_receive(char* receive_buffer);
uint8_t WSWD_init(void);

#endif /* WSWD_WSWD_H_ */

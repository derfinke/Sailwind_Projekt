/*
 * http_ssi.h
 *
 *  Created on: Sep 23, 2023
 *      Author: nicof
 */

#ifndef HTTP_SSI_H_
#define HTTP_SSI_H_

#include <stdint.h>

void http_server_init(void);
uint16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen);

#endif /* HTTP_SSI_H_ */

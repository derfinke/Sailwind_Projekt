/*
 * \file http_ssi.h
 * @date 18 Jun 2023
 * @brief ssi related functions
 */

#ifndef HTTP_SSI_H_
#define HTTP_SSI_H_

#include <stdint.h>

/**
 * @brief initialize http server and set ssi handler
 * @param none
 * @retval none
 */
void http_server_init(void);

/**
 * @brief if ssi tags are found in shtml files this function sets the value of the tags
 * @param iIndex: index of ssi tag in char array
 * @param pcInsert: string to be inserted in ssi tag
 * @param iInsertLen: max length to be inserted in ssi tag
 * @retval inserted string lentgh
 */
uint16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen);

#endif /* HTTP_SSI_H_ */

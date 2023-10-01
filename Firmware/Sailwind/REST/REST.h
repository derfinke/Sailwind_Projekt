/*
 * REST.h
 *
 *  Created on: Sep 19, 2023
 *      Author: nicof
 */

#ifndef REST_REST_H_
#define REST_REST_H_

/**
 * @brief  Handles incoming HTTP requests
 * @param  payload: pointer to received payload
 * @param  buffer: pointer to http response buffer
 * @retval none
 */
void REST_request_handler(char *payload, char *buffer);

/**
 * @brief  Gets required pointers to objects
 * @param  none
 * @retval none
 */
void REST_init(void);

#endif /* REST_REST_H_ */

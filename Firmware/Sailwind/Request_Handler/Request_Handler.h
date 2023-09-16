/*
 * Request_Handler.h
 *
 *  Created on: 16.09.2023
 *      Author: Bene
 */

#ifndef REQUEST_HANDLER_REQUEST_HANDLER_H_
#define REQUEST_HANDLER_REQUEST_HANDLER_H_

#include "Linear_Guide.h"

#define REQ_PATH_DATA "/data"
#define REQ_PATH_DATA_STATUS "/data/status"
#define REQ_PATH_DATA_STATUS_ERROR "/data/status/error"
#define REQ_PATH_DATA_STATUS_OPERATINGMODE "/data/status/operating_mode"
#define REQ_PATH_DATA_ADJUSTMENT "/data/adjustment"
#define REQ_PATH_DATA_SENSORS "/data/sensors"
#define REQ_PATH_DATA_SENSORS_WIND "/data/sensors/wind"
#define REQ_PATH_DATA_SENSORS_CURRENT "/data/sensors/current"

void Request_Handler_GET(Linear_Guide_t *lg_ptr, char *request);
void Request_Handler_PUT(Linear_Guide_t *lg_ptr, char *request, char *payload);

#endif /* REQUEST_HANDLER_REQUEST_HANDLER_H_ */

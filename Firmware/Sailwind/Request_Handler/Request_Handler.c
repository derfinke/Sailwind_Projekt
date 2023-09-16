/*
 * Request_Handler.c
 *
 *  Created on: 16.09.2023
 *      Author: Bene
 */

#include "Request_Handler.h"
#include <string.h>


void Request_Handler_GET(Linear_Guide_t *lg_ptr, char *request)
{
	if(strcmp(request, REQ_PATH_DATA) == 0)
	{
		; // ToDo
	}
	if(strcmp(request, REQ_PATH_DATA_STATUS) == 0)
	{
		; // ToDo
	}
	if(strcmp(request, REQ_PATH_DATA_ADJUSTMENT) == 0)
	{
		; // ToDo
	}
	if(strcmp(request, REQ_PATH_DATA_SENSORS) == 0)
	{
		; // ToDo
	}
	if(strcmp(request, REQ_PATH_DATA_SENSORS_WIND) == 0)
	{
		; // ToDo
	}
	if(strcmp(request, REQ_PATH_DATA_SENSORS_CURRENT) == 0)
	{
		; // ToDo
	}
}

void Request_Handler_PUT(Linear_Guide_t *lg_ptr, char *request, char *payload)
{
	if(strcmp(request, REQ_PATH_DATA_STATUS_ERROR) == 0)
	{
		; // ToDo
	}
	if(strcmp(request, REQ_PATH_DATA_STATUS_OPERATINGMODE) == 0)
	{
		; // ToDo
	}
	if(strcmp(request, REQ_PATH_DATA_ADJUSTMENT) == 0)
	{
		; // ToDo
	}
}

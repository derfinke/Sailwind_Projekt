/*
 * REST.c
 *
 *  Created on: Sep 19, 2023
 *      Author: nicof
 */

#include "string.h"

#define GET_REQUEST       "GET"
#define PUT_REQUEST       "PUT"

#define URL_OFFSET            4U
#define PATH_DATA             "/data"
#define PATH_STATUS           "/data/status"
#define PATH_SAIL_STATE       "/data/adjustment"
#define PATH_SENSORS          "/data/sensors"
#define PATH_CURRENT_SENSOR   "/data/sensors/current"
#define PATH_WIND_SENSOR      "/data/sensors/wind"

#define PATH_ERROR            "/data/status/error"
#define PATH_MODE             "/data/status/operating_mode"

static void REST_get_request(char *payload);
static void REST_put_request(char *payload);

void REST_request_handler(char *payload) {

  char http_request[3];
  memcpy(http_request, payload, 3U);

  if (strcmp(http_request, GET_REQUEST) == 0) {
    REST_get_request(payload);
  } else if (strcmp(http_request, PUT_REQUEST) == 0) {
    REST_put_request(payload);
  } else {
    //error handling
  }
}

static void REST_get_request(char *payload) {

  if (strncmp(payload + URL_OFFSET, PATH_DATA, strlen(PATH_DATA)) == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_STATUS, strlen(PATH_STATUS))
      == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_SAIL_STATE,
                     strlen(PATH_SAIL_STATE)) == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_SENSORS, strlen(PATH_SENSORS))
      == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_CURRENT_SENSOR,
                     strlen(PATH_CURRENT_SENSOR)) == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_WIND_SENSOR,
                     strlen(PATH_WIND_SENSOR)) == 0) {

  } else {
    //invalid path
  }
}

static void REST_put_request(char *payload) {

  if (strncmp(payload + URL_OFFSET, PATH_DATA, strlen(PATH_DATA)) == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_ERROR, strlen(PATH_ERROR))
      == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_SAIL_STATE,
                     strlen(PATH_SAIL_STATE)) == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_MODE, strlen(PATH_MODE)) == 0) {

  } else if (strncmp(payload + URL_OFFSET, PATH_SAIL_STATE,
                     strlen(PATH_SAIL_STATE)) == 0) {

  } else {
    //invalid path
  }
}

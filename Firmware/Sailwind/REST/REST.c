/*
 * REST.c
 *
 *  Created on: Sep 19, 2023
 *      Author: nicof
 */

#include <stdio.h>
#include "string.h"
#include "cJSON.h"
#include "Linear_Guide.h"
#include "FRAM.h"
#include "FRAM_memory_mapping.h"
#include "WSWD.h"
#include "IO.h"

#define GET_REQUEST           "GET"
#define PUT_REQUEST           "PUT"

#define PATH_DATA             "/data "
#define PATH_STATUS           "/data/status "
#define PATH_SAIL_STATE       "/data/adjustment "
#define PATH_SENSORS          "/data/sensors "
#define PATH_CURRENT_SENSOR   "/data/sensors/current "
#define PATH_WIND_SENSOR      "/data/sensors/wind "
#define PATH_SETTINGS         "/data/settings "

#define PATH_ERROR            "/data/status/error "
#define PATH_MODE             "/data/status/operating_mode "

#define HTTP_SUCCESS          "200 OK\r\n"
#define HTTP_NOT_FOUND        "404 Not Found\r\n"
#define HTTP_NOT_IMPLEMENTED  "501 Not Implemented\r\n"
#define HTTP_BAD_REQ          "400 Bad Request\r\n"
#define HOST_NAME             "Host: 192.168.0.123\r\n"
#define CONTENT_TYPE          "Content-Type: application/json\r\n"
#define END_OF_HEADER         "\r\n\r\n"
#define CONTENT_LEN           "Content-Length: "

#define HTTP_VER              "HTTP/1.1 "
#define URL_OFFSET            4U

#define KEY_ERROR             "error"
#define KEY_MODE              "operating_mode"
#define KEY_SAIL_POS          "sail_pos"
#define KEY_SAIL_STATE        "sail_state"
#define KEY_WIND              "wind"
#define KEY_SPEED             "speed"
#define KEY_DIR               "direction"
#define KEY_CURRENT           "current"
#define KEY_LOCALIZED         "localized"
#define KEY_MAX_RPM           "max_rpm"
#define KEY_MAX_DISTANCE      "max_distance_error"

typedef enum {
  HTTP_OK,
  HTTP_Not_Implemented,
  HTTP_Not_Found,
  HTTP_Bad_Request
} http_responses_t;

static Linear_Guide_t* REST_linear_guide = {0};
static IO_analogSensor_t *REST_current_sensor = {0};

/**
 * @brief  Handles HTTP GET requests
 * @param  payload: pointer to received payload
 * @param  buffer: pointer to http response buffer
 * @retval none
 */
static void REST_get_request(char *payload, char *buffer);

/**
 * @brief  Handles HTTP PUT requests
 * @param  payload: pointer to received payload
 * @param  buffer: pointer to http response buffer
 * @retval none
 */
static void REST_put_request(char *payload, char *buffer);

/**
 * @brief  Build HTTP header with given
 * @param  payload: pointer to received payload
 * @param  buffer: pointer to http response buffer
 * @retval none
 */
static void REST_create_HTTP_header(char *buffer, http_responses_t response,
                                    uint32_t size_of_anwser);

/**
 * @brief  Build the /data json
 * @param  response: pointer to HTTP response JSON
 * @retval none
 */
static void REST_create_data_json(cJSON *response);

/**
 * @brief  Build the /data/status json
 * @param  response: pointer to HTTP response JSON
 * @retval none
 */
static void REST_create_status_json(cJSON *response);

/**
 * @brief  Build the /data/sensors json
 * @param  response: pointer to HTTP response JSON
 * @retval none
 */
static void REST_create_sensors_json(cJSON *response);

/**
 * @brief  Build the /data/sensors/wind json
 * @param  response: pointer to HTTP response JSON
 * @retval none
 */
static void REST_create_wind_json(cJSON *response);

/**
 * @brief  Build the /data/adjustment json
 * @param  response: pointer to HTTP response JSON
 * @retval none
 */
static void REST_create_adjustment(cJSON *response);
/**
 * @brief  check if the received error json is valid
 * @param  error_json: pointer to received JSON
 * @retval none
 */
static uint8_t REST_check_error_json(cJSON *error_json);

/**
 * @brief  check if the received sailstate json is valid
 * @param  sailstate_json: pointer to received JSON
 * @retval none
 */
static uint8_t REST_check_sailstate_json(cJSON *sailstate_json);

/**
 * @brief  check if the received operating_mode json is valid
 * @param  mode_json: pointer to received JSON
 * @retval none
 */
static uint8_t REST_check_mode_json(cJSON *mode_json);

static uint8_t REST_check_settings_json(cJSON *settings_json);

static void REST_create_settings_json(cJSON *response);

void REST_init(void)
{
  REST_linear_guide = LG_get_Linear_Guide();
  REST_current_sensor = IO_get_current_sensor();
}

void REST_request_handler(char *payload, char *buffer) {

  char http_request[4];
  memcpy(http_request, payload, 3U);
  http_request[3] = '\0';

  /* check for GET request */
  if (strcmp(http_request, GET_REQUEST) == 0) {
    REST_get_request(payload, buffer);

    /* check for PUT request */
  } else if (strcmp(http_request, PUT_REQUEST) == 0) {
    REST_put_request(payload, buffer);

    /* if not GET or PUT return HTTP status code 501 */
  } else {
    REST_create_HTTP_header(buffer, HTTP_Not_Implemented, 0);
  }
}

static void REST_get_request(char *payload, char *buffer) {

  cJSON *response = cJSON_CreateObject();
  char JSON_response[200];
  memset(JSON_response, '\0', 1);

  /* check for path /data */
  if (strncmp(payload + URL_OFFSET, PATH_DATA, strlen(PATH_DATA)) == 0) {

    REST_create_data_json(response);
    cJSON_PrintPreallocated(response, JSON_response, 200, 1);
    REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));

    /* check for path /data/status */
  } else if (strncmp(payload + URL_OFFSET, PATH_STATUS, strlen(PATH_STATUS))
      == 0) {

    REST_create_status_json(response);
    cJSON_PrintPreallocated(response, JSON_response, 200, 1);
    REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));

    /* check for path /data/adjustment */
  } else if (strncmp(payload + URL_OFFSET, PATH_SAIL_STATE,
                     strlen(PATH_SAIL_STATE)) == 0) {

    REST_create_adjustment(response);
    cJSON_PrintPreallocated(response, JSON_response, 200, 1);
    REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));

    /* check for path /data/sensors */
  } else if (strncmp(payload + URL_OFFSET, PATH_SENSORS, strlen(PATH_SENSORS))
      == 0) {

    REST_create_sensors_json(response);
    cJSON_PrintPreallocated(response, JSON_response, 200, 1);
    REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));

    /* check for path /data/sensors/current */
  } else if (strncmp(payload + URL_OFFSET, PATH_CURRENT_SENSOR,
                     strlen(PATH_CURRENT_SENSOR)) == 0) {

    IO_Get_Measured_Value(REST_current_sensor);
    cJSON_AddNumberToObject(response, KEY_CURRENT, REST_current_sensor->measured_value);
    cJSON_PrintPreallocated(response, JSON_response, 200, 1);
    REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));

    /* check for path /data/sensors/wind */
  } else if (strncmp(payload + URL_OFFSET, PATH_WIND_SENSOR,
                     strlen(PATH_WIND_SENSOR)) == 0) {

    REST_create_wind_json(response);
    cJSON_PrintPreallocated(response, JSON_response, 200, 1);
    REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));

    /* check for path /data/status/operating_mode */
  } else if (strncmp(payload + URL_OFFSET, PATH_WIND_SENSOR,
                    strlen(PATH_WIND_SENSOR)) == 0) {

   REST_create_wind_json(response);
   cJSON_PrintPreallocated(response, JSON_response, 200, 1);
   REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));

   /* check for path /data/status/operating_mode */
 } else if (strncmp(payload + URL_OFFSET, PATH_SETTINGS, strlen(PATH_SETTINGS)) == 0) {

    REST_create_settings_json(response);
    cJSON_PrintPreallocated(response, JSON_response, 200, 1);
    REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));
  } else if (strncmp(payload + URL_OFFSET, PATH_MODE,
                     strlen(PATH_MODE)) == 0) {

    cJSON_AddNumberToObject(response, KEY_MODE, REST_linear_guide->operating_mode);
    cJSON_PrintPreallocated(response, JSON_response, 200, 1);
    REST_create_HTTP_header(buffer, HTTP_OK, strlen(JSON_response));

  } else {
    REST_create_HTTP_header(buffer, HTTP_Not_Found, 0);
  }

  cJSON_Delete(response);
  strcat(buffer, JSON_response);
}

static void REST_put_request(char *payload, char *buffer) {

  char *pEndOfHeader;

  pEndOfHeader = strstr(payload, END_OF_HEADER);
  cJSON *request = cJSON_Parse(pEndOfHeader);

  /*check for valid JSON format*/
  if (request == NULL) {

    REST_create_HTTP_header(buffer, HTTP_Bad_Request, 0);

  } else {

    /* check for path /data/status/error */
    if (strncmp(payload + URL_OFFSET, PATH_ERROR, strlen(PATH_ERROR)) == 0) {

      if (REST_check_error_json(request) != 1) {
        REST_create_HTTP_header(buffer, HTTP_OK, 0);
      } else {

        REST_create_HTTP_header(buffer, HTTP_Bad_Request, 0);
      }

      /* check for path /data/status/adjustment */
    } else if (strncmp(payload + URL_OFFSET, PATH_SAIL_STATE,
                       strlen(PATH_SAIL_STATE)) == 0) {

      if (REST_check_sailstate_json(request) != 1) {

        REST_create_HTTP_header(buffer, HTTP_OK, 0);
      } else {

        REST_create_HTTP_header(buffer, HTTP_Bad_Request, 0);
      }

      /* check for path /data/status/operating_mode */
    } else if (strncmp(payload + URL_OFFSET, PATH_MODE, strlen(PATH_MODE))
        == 0) {
      if (REST_check_mode_json(request) != 1) {

        REST_create_HTTP_header(buffer, HTTP_OK, 0);
      } else {

        REST_create_HTTP_header(buffer, HTTP_Bad_Request, 0);
      }

    } else if (strncmp(payload + URL_OFFSET, PATH_SETTINGS, strlen(PATH_SETTINGS))
        == 0) {
      if (REST_check_settings_json(request) != 1) {

        REST_create_HTTP_header(buffer, HTTP_OK, 0);
      } else {

        REST_create_HTTP_header(buffer, HTTP_Bad_Request, 0);
      }

    } else {

      REST_create_HTTP_header(buffer, HTTP_Not_Found, 0);
    }
  }
  cJSON_Delete(request);
}

static void REST_create_HTTP_header(char *buffer, http_responses_t response,
                                    uint32_t size_of_anwser) {
  char size_of_anwser_char[6];
  sprintf(size_of_anwser_char, "%lu", size_of_anwser);
  sprintf(buffer, HTTP_VER);

  switch (response) {
    case HTTP_OK:
      strcat(buffer, HTTP_SUCCESS);
      break;
    case HTTP_Not_Implemented:
      strcat(buffer, HTTP_NOT_IMPLEMENTED);
      break;
    case HTTP_Not_Found:
      strcat(buffer, HTTP_NOT_FOUND);
      break;
    case HTTP_Bad_Request:
      strcat(buffer, HTTP_BAD_REQ);
      break;
    default:
      assert(response > HTTP_Not_Found);
      break;
  }
  strcat(buffer, HOST_NAME);
  strcat(buffer, CONTENT_TYPE);
  strcat(buffer, CONTENT_LEN);
  strcat(buffer, size_of_anwser_char);
  strcat(buffer, END_OF_HEADER);
}

static void REST_create_status_json(cJSON *response) {
  cJSON_AddNumberToObject(response, KEY_ERROR, Linear_Guide_get_error());
  cJSON_AddNumberToObject(response, KEY_MODE, REST_linear_guide->operating_mode);
  cJSON_AddNumberToObject(response, KEY_LOCALIZED, REST_linear_guide->localization.is_localized);
}

static void REST_create_data_json(cJSON *response) {
  cJSON_AddNumberToObject(response, KEY_ERROR, Linear_Guide_get_error());
  cJSON_AddNumberToObject(response, KEY_MODE, REST_linear_guide->operating_mode);
  cJSON_AddNumberToObject(response, KEY_LOCALIZED, REST_linear_guide->localization.is_localized);

  REST_create_adjustment(response);

  REST_create_wind_json(response);

  IO_Get_Measured_Value(REST_current_sensor);
  cJSON_AddNumberToObject(response, KEY_CURRENT, REST_current_sensor->measured_value);
}

static void REST_create_sensors_json(cJSON *response) {
  REST_create_wind_json(response);
  IO_Get_Measured_Value(REST_current_sensor);
  cJSON_AddNumberToObject(response, KEY_CURRENT, REST_current_sensor->measured_value);
}

static void REST_create_wind_json(cJSON *response) {
  cJSON *wind;
  cJSON *wind_members;
  char NMEA_telegram[30];
  float wind_speed;
  float wind_dir;

  wind = cJSON_AddArrayToObject(response, KEY_WIND);
  wind_members = cJSON_CreateObject();

  WSWD_receive_NMEA(NMEA_telegram);
  WSWD_get_wind_infos(NMEA_telegram, &wind_speed, &wind_dir);

  cJSON_AddNumberToObject(wind_members, KEY_SPEED, wind_speed);
  cJSON_AddNumberToObject(wind_members, KEY_DIR, wind_dir);
  cJSON_AddItemToArray(wind, wind_members);
}

static void REST_create_adjustment(cJSON *response)
{
	int8_t roll_pitch_percentage = Linear_Guide_get_current_roll_pitch_percentage(*REST_linear_guide);
	cJSON_AddNumberToObject(response, KEY_SAIL_POS, roll_pitch_percentage);
}

static void REST_create_settings_json(cJSON *response)
{
  cJSON_AddNumberToObject(response, KEY_MAX_RPM, REST_linear_guide->motor.normal_rpm);
  cJSON_AddNumberToObject(response, KEY_MAX_DISTANCE, REST_linear_guide->max_distance_fault);
}

static uint8_t REST_check_error_json(cJSON *error_json) {
  cJSON *error = cJSON_GetObjectItemCaseSensitive(error_json, KEY_ERROR);
  /* Check for number of keys */
  if (!(cJSON_GetArraySize(error) < 2)) {
    return 1;
  }
  /* Check for error key */
  if (!cJSON_IsNumber(error)) {
    return 1;
  }
  /* Check for error value */
  if ((error->valueint != 2) || (error->valueint != 0)) {
    return 1;
  }
  if(error->valueint == 2)
  {
    Linear_Guide_set_error(LG_error_state_2_wind_speed_fault);
  }
  else
  {
    Linear_Guide_set_error(LG_error_state_0_normal);
  }
  /* Format is valid */
  return 0;
}

static uint8_t REST_check_sailstate_json(cJSON *sailstate_json) {
  cJSON *sail_pos = cJSON_GetObjectItemCaseSensitive(sailstate_json, KEY_SAIL_POS);

  /* Check for number of keys */
  if (!(cJSON_GetArraySize(sail_pos) < 3)) {
    return 1;
  }
  /* Check for sail_pos key */
  if (!(cJSON_IsNumber(sail_pos))) {
    return 1;
  }
    /* Check for value range */
    if (-100 <= sail_pos->valueint && sail_pos->valueint <= 100) {
    	/* Format is valid */
    	Linear_Guide_set_desired_roll_pitch_percentage(REST_linear_guide, sail_pos->valueint);
    	return 0;
    }
    return 1;
}

static uint8_t REST_check_mode_json(cJSON *mode_json) {
  cJSON *operating_mode = cJSON_GetObjectItemCaseSensitive(mode_json,
  KEY_MODE);
  /* Check for number of keys */
  if (!(cJSON_GetArraySize(operating_mode) < 2)) {
    return 1;
  }
  /* Check for operating_mode key */
  if (!cJSON_IsNumber(operating_mode)) {
    return 1;
  }
  /* Check for operating_mode value */
  if (!((operating_mode->valueint == LG_operating_mode_manual) || (operating_mode->valueint == LG_operating_mode_automatic))) {
    return 1;
  }
  /* Format is valid */
  Linear_Guide_set_operating_mode(REST_linear_guide, operating_mode->valueint);
  return 0;
}

static uint8_t REST_check_settings_json(cJSON *settings_json)
{

  cJSON *max_rpm = cJSON_GetObjectItemCaseSensitive(settings_json,
  KEY_MAX_RPM);
  cJSON *max_distance_error = cJSON_GetObjectItemCaseSensitive(settings_json,
  KEY_MAX_DISTANCE);
  /* Check for number of keys */
  if (!(cJSON_GetArraySize(settings_json) < 3)) {
    return 1;
  }
  /* Check for operating_mode key */
  if (!cJSON_IsNumber(max_rpm)) {
    return 1;
  }
  if (!cJSON_IsNumber(max_distance_error)) {
    return 1;
  }
  /* Check for operating_mode value */
  if (((max_rpm->valueint > 2000) || (max_rpm->valueint < 400))) {
    return 1;
  }
  if (((max_distance_error->valueint > 50) || (max_distance_error->valueint < 5))) {
    return 1;
  }
  /* Format is valid */
  REST_linear_guide->max_distance_fault = (uint8_t)max_distance_error->valueint;
  REST_linear_guide->motor.normal_rpm = (uint16_t)max_rpm->valueint;
  FRAM_write(&REST_linear_guide->max_distance_fault, FRAM_MAX_DELTA, 1U);
  FRAM_write((uint8_t*)&REST_linear_guide->motor.normal_rpm, FRAM_MAX_RPM, sizeof(REST_linear_guide->motor.normal_rpm));
  return 0;
}

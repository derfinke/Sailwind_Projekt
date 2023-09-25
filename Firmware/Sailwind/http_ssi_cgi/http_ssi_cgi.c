/*
 * \file http_ssi.c
 * @date 18 Jun 2023
 * @brief ssi related functions
 */

#include <http_ssi_cgi.h>
#include "string.h"
#include "stdio.h"
#include "main.h"
#include "tcp.h"
#include "httpd.h"
#include "IO.h"
#include "WSWD.h"
#include "Linear_Guide.h"

#define NUM_SSI_TAGS 9
#define UINT_TAGS 4

static IO_analogSensor_t *current_sensor;
static IO_analogSensor_t *distance_sensor;
static Linear_Guide_t *linear_guide;
static uint8_t error_flag = 0;

char const *TAGCHAR[] = { "current", "dism", "diss", "pitch", "roll", "windspd",
    "winddir", "mode", "opmod", "error" };
char const **TAGS = TAGCHAR;

const char* CGIIP_Handler(int iIndex, int iNumParams, char *pcParam[],
                          char *pcValue[]);
const char* CGIRestart_Handler(int iIndex, int iNumParams, char *pcParam[],
                               char *pcValue[]);
const char* CGIMode_Handler(int iIndex, int iNumParams, char *pcParam[],
                            char *pcValue[]);
const char* CGIControl_Handler(int iIndex, int iNumParams, char *pcParam[],
                               char *pcValue[]);
char name[30];
tCGI CGI_FORMS[4];
const tCGI FORM_IP_CGI = {"/form_IP.cgi", CGIIP_Handler};
const tCGI RESTART_CGI = {"/form_restart.cgi", CGIRestart_Handler};
const tCGI MODE_CGI = {"/form_operating_mode.cgi", CGIMode_Handler};
const tCGI CONTROL_CGI = {"/form_control.cgi", CGIControl_Handler};

uint16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {

  char NMEA_telegram[30];
  float Wind_speed;
  float Wind_dir;
  int32_t motor_pos;
  uint8_t op_mode;
  uint8_t sail_pos;

  switch (iIndex) {
    case 0:
      IO_Get_Measured_Value(current_sensor);
      uint16_t current = current_sensor->measured_value;

      (void) snprintf(pcInsert, iInsertLen, "%u", current);
      break;
    case 1:
      motor_pos = linear_guide->localization.current_pos_mm;
      (void) snprintf(pcInsert, iInsertLen, "%ld", motor_pos);
      break;
    case 2:
      IO_Get_Measured_Value(distance_sensor);
      uint16_t distance = distance_sensor->measured_value;

      (void) snprintf(pcInsert, iInsertLen, "%u", distance);
      break;
    case 3:
      (void) snprintf(pcInsert, iInsertLen, "currently unavailable");
      break;
    case 4:
      (void) snprintf(pcInsert, iInsertLen, "currently unavailable");
      break;
    case (UINT_TAGS + 1):
      WSWD_receive_NMEA(NMEA_telegram);
      WSWD_get_wind_speed(NMEA_telegram, &Wind_speed);
      (void) snprintf(pcInsert, iInsertLen, "%.2f", Wind_speed);
      break;
    case (UINT_TAGS + 2):
      WSWD_receive_NMEA(NMEA_telegram);
      WSWD_get_wind_dir(NMEA_telegram, &Wind_dir);
      (void) snprintf(pcInsert, iInsertLen, "%.1f", Wind_dir);
      break;
    case (UINT_TAGS + 3):
      sail_pos = linear_guide->operating_mode;
      switch (sail_pos) {
        case 0:
          (void) snprintf(pcInsert, iInsertLen, "%s", "roll");
          break;
        case 1:
          (void) snprintf(pcInsert, iInsertLen, "%s", "pitch");
          break;
        default:
          (void) snprintf(pcInsert, iInsertLen, "%s", "invalid");
          break;
      }
      break;
    case (UINT_TAGS + 4):
      op_mode = linear_guide->operating_mode;
      switch (op_mode) {
        case 0:
          (void) snprintf(pcInsert, iInsertLen, "%s", "manual");
          break;
        case 1:
          (void) snprintf(pcInsert, iInsertLen, "%s", "automatic");
          break;
        default:
          (void) snprintf(pcInsert, iInsertLen, "%s", "invalid");
          break;
      }
      break;
    case 9:
      if (error_flag == 1) {
        (void) snprintf(pcInsert, iInsertLen, "%s", "<label hidden></label>");
      } else if (error_flag == 2) {
        (void) snprintf(
            pcInsert, iInsertLen, "%s",
            "<label><h3>Successfully set new IP. Please restart.</h3></label>");
      } else {
        (void) snprintf(
            pcInsert,
            iInsertLen,
            "%s",
            "<label><h3>Please enter a valid IP address in the specified format.</h3></label>");
      }
      break;
    default:
      (void) snprintf(pcInsert, iInsertLen, "Error: Unknown SSI Tag!");
      break;
  }
  return strlen(pcInsert);
}

/************************ CGI HANDLER ***************************/

const char* CGIIP_Handler(int iIndex, int iNumParams, char *pcParam[],
                          char *pcValue[]) {
  char first_octet[3];
  char second_octet[3];
  char third_octet[3];
  char fourth_octet[3];

  if (iIndex == 0) {
    printf("IP\r\n");

    if (strcmp(pcParam[0], "IP_addr") == 0) {
      memset(name, '\0', 30);
      strcpy(name, pcValue[0]);
      if (strncmp(name + 3U, ".", 1) != 0) {
        error_flag = 0;
        return "/Settings.shtml";
      } else if (strncmp(name + 7U, ".", 1) != 0) {
        error_flag = 0;
        return "/Settings.shtml";
      } else if (strncmp(name + 11U, ".", 1) != 0) {
        error_flag = 0;
        return "/Settings.shtml";
      } else {
        int IP_address[4];
        strncpy(first_octet, name, 3);
        strncpy(second_octet, name + 4, 3);
        strncpy(third_octet, name + 8, 3);
        strncpy(fourth_octet, name + 12, 3);
        IP_address[0] = atoi(first_octet);
        IP_address[1] = atoi(second_octet);
        IP_address[2] = atoi(third_octet);
        IP_address[3] = atoi(fourth_octet);
        if (IP_address[0] > 255) {
          error_flag = 0;
          return "/Settings.shtml";
        } else if (IP_address[1] > 255) {
          error_flag = 0;
          return "/Settings.shtml";
        } else if (IP_address[2] > 255) {
          error_flag = 0;
          return "/Settings.shtml";
        } else if (IP_address[3] > 255) {
          error_flag = 0;
          return "/Settings.shtml";
        }
        //call flash write
        error_flag = 2;
      }
    }
  }
  return "/index.html";
}

const char* CGIRestart_Handler(int iIndex, int iNumParams, char *pcParam[],
                               char *pcValue[]) {
  if (iIndex == 1) {
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&htim2);

    printf("requesting restart\r\n");
  }

  return "/Settings.shtml";
}

const char* CGIMode_Handler(int iIndex, int iNumParams, char *pcParam[],
                            char *pcValue[]) {
  if (iIndex == 2) {
    if (strcmp(pcParam[0], "operating_mode") == 0) {
      memset(name, '\0', 30);
      strcpy(name, pcValue[0]);
      if (strcmp(name, "automatic") == 0) {
        //set operating mode to manual
      } else if (strcmp(name, "manual") == 0) {
        //set operating mode to automatic
      }
    }
  }

  return "/Settings.shtml";
}

const char* CGIControl_Handler(int iIndex, int iNumParams, char *pcParam[],
                               char *pcValue[]) {
  if (iIndex == 3) {
    if (strcmp(pcParam[0], "move") == 0) {
      memset(name, '\0', 30);
      strcpy(name, pcValue[0]);
      if (strcmp(name, "left") == 0) {
        //move to the right
      } else if (strcmp(name, "right") == 0) {
        //move to the left
      } else if (strcmp(name, "confirm") == 0) {
        //save position
      }
    }
  }

  return "/index.html";
}

void http_server_init(void) {
  httpd_init();
  http_set_ssi_handler(ssi_handler, (char const**) TAGS, NUM_SSI_TAGS);
  CGI_FORMS[0] = FORM_IP_CGI;
  CGI_FORMS[1] = RESTART_CGI;
  CGI_FORMS[2] = MODE_CGI;
  CGI_FORMS[3] = CONTROL_CGI;
  http_set_cgi_handlers(CGI_FORMS, 4);
  IO_get_distance_sensor(distance_sensor);
  IO_get_current_sensor(current_sensor);
  LG_get_Linear_Guide(linear_guide);
}


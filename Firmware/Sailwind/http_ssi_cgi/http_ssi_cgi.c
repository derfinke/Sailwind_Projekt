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
#include "FRAM.h"
#include "FRAM_memory_mapping.h"

#define NUM_SSI_TAGS 12
#define UINT_TAGS 4

static IO_analogSensor_t *ssi_current_sensor = { 0 };
static IO_analogSensor_t *ssi_distance_sensor = { 0 };
static Linear_Guide_t *ssi_linear_guide = { 0 };
static uint8_t error_flag = 1;

char const *TAGCHAR[] = { "current", "dism", "diss", "pitch", "roll", "windspd",
    "winddir", "mode", "opmod", "error", "maxrpm", "maxdel" };
char const **TAGS = TAGCHAR;

/**
 * @brief Handles the CGI IP form
 * @param iIndex: Index which cgi handler was called
 * @param iNumParams: Number of parameters in form
 * @param pcParam: Parameter name
 * @param pcValue: Value of Parameter
 * @retval form: pointer to form
 */
static const char* CGIIP_Handler(int iIndex, int iNumParams, char *pcParam[],
                                 char *pcValue[]);

/**
 * @brief Handles the CGI restart form
 * @param iIndex: Index which cgi handler was called
 * @param iNumParams: Number of parameters in form
 * @param pcParam: Parameter name
 * @param pcValue: Value of Parameter
 * @retval form: pointer to form
 */
static const char* CGIRestart_Handler(int iIndex, int iNumParams,
                                      char *pcParam[], char *pcValue[]);

/**
 * @brief Handles the CGI mode switch form
 * @param iIndex: Index which cgi handler was called
 * @param iNumParams: Number of parameters in form
 * @param pcParam: Parameter name
 * @param pcValue: Value of Parameter
 * @retval form: pointer to form
 */
static const char* CGIMode_Handler(int iIndex, int iNumParams, char *pcParam[],
                                   char *pcValue[]);

/**
 * @brief Handles the CGI control form
 * @param iIndex: Index which cgi handler was called
 * @param iNumParams: Number of parameters in form
 * @param pcParam: Parameter name
 * @param pcValue: Value of Parameter
 * @retval form: pointer to form
 */
static const char* CGIControl_Handler(int iIndex, int iNumParams,
                                      char *pcParam[], char *pcValue[]);

static const char* CGIrpm_Handler(int iIndex, int iNumParams, char *pcParam[],
                                  char *pcValue[]);

static const char* CGIdelta_Handler(int iIndex, int iNumParams, char *pcParam[],
                                    char *pcValue[]);
char name[30];
static char NMEA_telegram[30];
tCGI CGI_FORMS[6];
const tCGI FORM_IP_CGI = { "/form_IP.cgi", CGIIP_Handler };
const tCGI RESTART_CGI = { "/form_restart.cgi", CGIRestart_Handler };
const tCGI MODE_CGI = { "/form_operating_mode.cgi", CGIMode_Handler };
const tCGI CONTROL_CGI = { "/form_control.cgi", CGIControl_Handler };
const tCGI RPM_CGI = { "/form_rpm.cgi", CGIrpm_Handler };
const tCGI DELTA_CGI = { "/form_delta.cgi", CGIdelta_Handler };

uint16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {

  float Wind_speed;
  float Wind_dir;
  int32_t motor_pos;
  uint8_t op_mode;
  uint8_t sail_pos;

  switch (iIndex) {
    case 0:
      IO_Get_Measured_Value(ssi_current_sensor);
      uint16_t current = ssi_current_sensor->measured_value;

      (void) snprintf(pcInsert, iInsertLen, "%u", current);
      break;
    case 1:
      motor_pos = ssi_linear_guide->localization.current_pos_mm;
      (void) snprintf(pcInsert, iInsertLen, "%ld", motor_pos);
      break;
    case 2:
      IO_Get_Measured_Value(ssi_distance_sensor);
      uint16_t distance = ssi_distance_sensor->measured_value;
      (void) snprintf(pcInsert, iInsertLen, "%u", distance);
      break;
    case 3:
      if (ssi_linear_guide->sail_adjustment_mode
          != LG_sail_adjustment_mode_trim) {
        uint8_t roll_percentage = Linear_Guide_get_current_roll_trim_percentage(
            *ssi_linear_guide);
        (void) snprintf(pcInsert, iInsertLen, "%u", roll_percentage);
      } else {
        (void) snprintf(pcInsert, iInsertLen, "%u", 0);
      }
      break;
    case 4:
      if (ssi_linear_guide->sail_adjustment_mode
          != LG_sail_adjustment_mode_roll) {
        uint8_t pitch_percentage =
            Linear_Guide_get_current_roll_trim_percentage(*ssi_linear_guide);
        (void) snprintf(pcInsert, iInsertLen, "%u", pitch_percentage);
      } else {
        (void) snprintf(pcInsert, iInsertLen, "%u", 0);
      }
      break;
    case (UINT_TAGS + 1):
      WSWD_receive_NMEA(NMEA_telegram);
      WSWD_get_wind_speed(NMEA_telegram, &Wind_speed);
      (void) snprintf(pcInsert, iInsertLen, "%.2f", Wind_speed);
      break;
    case (UINT_TAGS + 2):
      WSWD_get_wind_dir(NMEA_telegram, &Wind_dir);
      (void) snprintf(pcInsert, iInsertLen, "%.1f", Wind_dir);
      break;
    case (UINT_TAGS + 3):
      sail_pos = ssi_linear_guide->operating_mode;
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
      op_mode = ssi_linear_guide->operating_mode;
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
    case 10:
      (void) snprintf(pcInsert, iInsertLen, "%u",
                      ssi_linear_guide->motor.normal_rpm);
      break;
    case 11:
      (void) snprintf(pcInsert, iInsertLen, "%u",
                      ssi_linear_guide->max_distance_fault);
      break;
    default:
      (void) snprintf(pcInsert, iInsertLen, "Error: Unknown SSI Tag!");
      break;
  }
  return strlen(pcInsert);
}

/************************ CGI HANDLER ***************************/

static const char* CGIIP_Handler(int iIndex, int iNumParams, char *pcParam[],
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
        //Define FRAM segments first
#if 0
        FRAM_write(IP_address, 0x0100, sizeof(IP_address));
#endif
        error_flag = 2;
      }
    }
  }
  return "/Settings.shtml";
}

static const char* CGIRestart_Handler(int iIndex, int iNumParams,
                                      char *pcParam[], char *pcValue[]) {
  if (iIndex == 1) {
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&htim2);

    printf("requesting restart\r\n");
  }

  return "/index.html";
}

static const char* CGIMode_Handler(int iIndex, int iNumParams, char *pcParam[],
                                   char *pcValue[]) {
  if (iIndex == 2) {
    if (strcmp(pcParam[0], "operating_mode") == 0) {
      memset(name, '\0', 30);
      strcpy(name, pcValue[0]);
      if (strcmp(name, "automatic") == 0) {
        Linear_Guide_set_operating_mode(ssi_linear_guide,
                                        LG_operating_mode_automatic);
      } else if (strcmp(name, "manual") == 0) {
        Linear_Guide_set_operating_mode(ssi_linear_guide,
                                        LG_operating_mode_manual);
      }
    }
  }

  return "/Settings.shtml";
}

static const char* CGIControl_Handler(int iIndex, int iNumParams,
                                      char *pcParam[], char *pcValue[]) {
  if (iIndex == 3) {
    if (strcmp(pcParam[0], "move") == 0) {
      memset(name, '\0', 30);
      strcpy(name, pcValue[0]);
      if (ssi_linear_guide->operating_mode == LG_operating_mode_manual) {
        if (strcmp(name, "left") == 0) {
          if ((ssi_linear_guide->localization.movement == Loc_movement_forward)
              || (ssi_linear_guide->localization.movement
                  == Loc_movement_backwards)) {
            Linear_Guide_manual_move(ssi_linear_guide, Loc_movement_stop);
          } else {
            Linear_Guide_manual_move(ssi_linear_guide, Loc_movement_backwards);
          }
        } else if (strcmp(name, "right") == 0) {
          if ((ssi_linear_guide->localization.movement == Loc_movement_forward)
              || (ssi_linear_guide->localization.movement
                  == Loc_movement_backwards)) {
            Linear_Guide_manual_move(ssi_linear_guide, Loc_movement_stop);
          } else {
            Linear_Guide_manual_move(ssi_linear_guide, Loc_movement_forward);
          }
        } else if (strcmp(name, "confirm") == 0) {
          if ((ssi_linear_guide->localization.movement != Loc_movement_backwards)
              || (ssi_linear_guide->localization.movement
                  != Loc_movement_forward)) {
            Localization_set_center(&ssi_linear_guide->localization);
            /*TODO: add LED blinking*/
          } else {
            /* should respond with error feedback*/
            printf("linear guide is still moving\r\n");
          }
        }
      }
    }
  }

  return "/index.html";
}

static const char* CGIrpm_Handler(int iIndex, int iNumParams, char *pcParam[],
                                  char *pcValue[]) {
  unsigned long rpm_to_be_set = 0;
  uint16_t rpm_to_be_saved = 0;
  if (iIndex == 4) {
    if (strcmp(pcParam[0], "max_rpm") == 0) {
      memset(name, '\0', 30);
      strcpy(name, pcValue[0]);
      if (strlen(name) > 4) {
        return "/Settings.shtml";
      }
      for (uint8_t i = 0; i < strlen(name); i++) {
        if (!isdigit((int )name[i])) {
          return "/Settings.shtml";
        }
      }

      rpm_to_be_set = strtoul(name, NULL, 0);

      if ((rpm_to_be_set < 400) || (rpm_to_be_set > 2000)) {
        return "/Settings.shtml";
      }
    }
    rpm_to_be_saved = (uint16_t) rpm_to_be_set;
    ssi_linear_guide->motor.normal_rpm = rpm_to_be_saved;
    FRAM_write((uint8_t*) &rpm_to_be_saved, FRAM_MAX_RPM,
               sizeof(rpm_to_be_saved));
  }
  return "/Settings.shtml";
}

static const char* CGIdelta_Handler(int iIndex, int iNumParams, char *pcParam[],
                                    char *pcValue[]) {
  unsigned long delta_to_be_set = 0;
  uint8_t delta_to_be_saved = 0;
  if (iIndex == 5) {
    if (strcmp(pcParam[0], "max_delta") == 0) {
      memset(name, '\0', 30);
      strcpy(name, pcValue[0]);
      if (strlen(name) > 3) {
        return "/Settings.shtml";
      }
      for (uint8_t i = 0; i < strlen(name); i++) {
        if (!isdigit((int )name[i])) {
          return "/Settings.shtml";
        }
      }

      delta_to_be_set = strtoul(name, NULL, 0);

      if ((delta_to_be_set < 5) || (delta_to_be_set > 50)) {
        return "/Settings.shtml";
      }
    }
    delta_to_be_saved = (uint8_t) delta_to_be_set;
    ssi_linear_guide->max_distance_fault = delta_to_be_saved;
    FRAM_write(&delta_to_be_saved, FRAM_MAX_DELTA, 1U);
  }
  return "/Settings.shtml";
}

void http_server_init(void) {
  httpd_init();
  http_set_ssi_handler(ssi_handler, (char const**) TAGS, NUM_SSI_TAGS);
  CGI_FORMS[0] = FORM_IP_CGI;
  CGI_FORMS[1] = RESTART_CGI;
  CGI_FORMS[2] = MODE_CGI;
  CGI_FORMS[3] = CONTROL_CGI;
  CGI_FORMS[4] = RPM_CGI;
  CGI_FORMS[5] = DELTA_CGI;
  http_set_cgi_handlers(CGI_FORMS, 6);
  ssi_current_sensor = IO_get_current_sensor();
  ssi_distance_sensor = IO_get_distance_sensor();
  ssi_linear_guide = LG_get_Linear_Guide();
}


/*
 * \file http_ssi.c
 * @date 18 Jun 2023
 * @brief ssi related functions
 */

#include "http_ssi.h"
#include "string.h"
#include "stdio.h"

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

char const *TAGCHAR[] = { "current", "dism", "diss", "pitch", "roll", "windspd",
    "winddir", "mode", "opmod" };
char const **TAGS = TAGCHAR;

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
    default:
      (void) snprintf(pcInsert, iInsertLen, "Error: Unknown SSI Tag!");
      break;
  }
  return strlen(pcInsert);
}

void http_server_init(void) {
  httpd_init();
  http_set_ssi_handler(ssi_handler, (char const**) TAGS, NUM_SSI_TAGS);
  IO_get_distance_sensor(distance_sensor);
  IO_get_current_sensor(current_sensor);
  LG_get_Linear_Guide(linear_guide);
}


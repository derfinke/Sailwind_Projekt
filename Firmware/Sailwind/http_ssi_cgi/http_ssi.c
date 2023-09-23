/*
 * http_ssi.c
 *
 *  Created on: Sep 23, 2023
 *      Author: nicof
 */

/*
 * http_ssi.c
 *
 *  Created on: 11-Oct-2021
 *      Author: controllerstech
 */

#include "http_ssi.h"
#include "string.h"
#include "stdio.h"

#include "lwip/tcp.h"
#include "lwip/apps/httpd.h"

#define NUM_SSI_TAGS 9
#define UINT_TAGS 4

char const *TAGCHAR[] = { "current", "dism", "diss", "pitch", "roll", "windspd",
    "winddir", "mode", "opmod" };
char const **TAGS = TAGCHAR;

uint16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {

  uint16_t Current = 100;
  char sail_pos[6] = {"pitch"};
  char op_mode[10] = {"automatic"};
  uint16_t pitch = 40;
  uint16_t roll = 0;
  uint16_t distance_motor = 280;
  uint16_t distance_sensor = 270;
  float Wind_speed = 5.0;
  float Wind_direction = 180.8;

  uint16_t *tag_values[NUM_SSI_TAGS] = { &Current, &distance_motor, &distance_sensor, &pitch, &roll };

  if (iIndex <= UINT_TAGS) {
    (void) snprintf(pcInsert, iInsertLen, "%u", *tag_values[iIndex]);
  } else {
    switch (iIndex) {
      case (UINT_TAGS + 1):
        (void) snprintf(pcInsert, iInsertLen, "%.2f", Wind_speed);
        break;
      case (UINT_TAGS + 2):
        (void) snprintf(pcInsert, iInsertLen, "%.1f", Wind_direction);
        break;
      case (UINT_TAGS + 3):
        (void) snprintf(pcInsert, iInsertLen, "%s", sail_pos);
        break;
      case (UINT_TAGS + 4):
        (void) snprintf(pcInsert, iInsertLen, "%s", op_mode);
        break;
      default:
        (void) snprintf(pcInsert, iInsertLen, "Error: Unknown SSI Tag!");
        break;
    }
  }

  return strlen(pcInsert);
}

void http_server_init(void) {
  httpd_init();

  http_set_ssi_handler(ssi_handler, (char const**) TAGS, NUM_SSI_TAGS);
}


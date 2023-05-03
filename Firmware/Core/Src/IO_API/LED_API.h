/*
 * LED_API.h
 *
 *  Created on: 03.05.2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_LED_API_H_
#define SRC_IO_API_LED_API_H_

#include "IO_API.h"

/* typedefs -----------------------------------------------------------*/
typedef IO_digitalPin_t LED_t;

typedef struct {
	LED_t manual;
	LED_t automatic;
} LED_status_operating_mode_t;

typedef struct {
	LED_t motor_error;
	LED_status_operating_mode_t operating_mode;
} LED_bar_t;

/* defines -------------------------------------------------------------------*/
#define LED_OFF GPIO_PIN_RESET
#define LED_ON GPIO_PIN_SET

/* API function prototypes ---------------------------------------------------*/
LED_bar_t LED_init_bar();
void LED_switch(LED_t *led_ptr, GPIO_PinState led_state);
void LED_toggle(LED_t *led_ptr);
void LED_toggle_operating_mode(LED_status_operating_mode_t *led_operating_mode_ptr);

#endif /* SRC_IO_API_LED_API_H_ */

/*
 * LED_API.c
 *
 *  Created on: 03.05.2023
 *      Author: Bene
 */

#include "LED_API.h"

/* API function definitions -----------------------------------------------*/
LED_bar_t LED_init_bar()
{
	LED_bar_t LED_bar = {
			.motor_error = {
					.GPIOx = GPIOE,
					.GPIO_Pin = LED_1_Red_PE2_Pin,
					.state = LED_OFF
			},
			.operating_mode = {
					.manual = {
							.GPIOx = GPIOE,
							.GPIO_Pin = LED_2_Yellow_PE3_Pin,
							.state = LED_OFF
					},
					.automatic = {
							.GPIOx = GPIOE,
							.GPIO_Pin = LED_3_Green_PE4_Pin,
							.state = LED_OFF
					}
			},
			.sail_adjustment_mode = {
					.rollung = {
							//ToDo
					},
					.trimmung = {
							//ToDo
					}
			},
			.center_pos_set = {
					//ToDo
			}
	};
	return LED_bar;
}

void LED_switch(LED_t *led_ptr, GPIO_PinState led_state)
{
	IO_writeDigitalOUT(led_ptr, led_state);
}

void LED_toggle(LED_t *led_ptr)
{
	IO_toggleDigitalOUT(led_ptr);
}

void LED_toggle_operating_mode(LED_dual_operating_mode_t *led_operating_mode_ptr)
{
	LED_toggle(&led_operating_mode_ptr->automatic);
	LED_toggle(&led_operating_mode_ptr->manual);
}

void LED_toggle_sail_adjustment_mode(LED_dual_sail_adjustment_mode_t *led_sail_adjustment_mode_ptr)
{
	LED_toggle(&led_sail_adjustment_mode_ptr->rollung);
	LED_toggle(&led_sail_adjustment_mode_ptr->trimmung);
}



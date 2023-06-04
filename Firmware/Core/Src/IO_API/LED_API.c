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
					.GPIO_Pin = LED_Stoerung_Pin,
					.state = LED_OFF
			},
			.operating_mode = {
					.manual = {
							.GPIOx = GPIOE,
							.GPIO_Pin = LED_Manuell_Pin,
							.state = LED_OFF
					},
					.automatic = {
							.GPIOx = GPIOE,
							.GPIO_Pin = LED_Automatik_Pin,
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
	IO_digitalWrite(led_ptr, led_state);
}

void LED_toggle(LED_t *led_ptr)
{
	IO_digitalToggle(led_ptr);
}

void LED_set_operating_mode(LED_dual_operating_mode_t *led_operating_mode_ptr, IO_operating_mode_t operating_mode)
{
	GPIO_PinState automatic, manual;
	switch(operating_mode)
	{
		case IO_operating_mode_manual:
			manual = LED_ON, automatic = LED_OFF; break;
		case IO_operating_mode_automatic:
			manual = LED_OFF, automatic = LED_ON; break;
	}
	LED_switch(&led_operating_mode_ptr->automatic, automatic);
	LED_switch(&led_operating_mode_ptr->manual, manual);
}

void LED_toggle_sail_adjustment_mode(LED_dual_sail_adjustment_mode_t *led_sail_adjustment_mode_ptr)
{
	LED_toggle(&led_sail_adjustment_mode_ptr->rollung);
	LED_toggle(&led_sail_adjustment_mode_ptr->trimmung);
}



/*
 * LED_API.c
 *
 *  Created on: 03.05.2023
 *      Author: Bene
 */

#include "LED.h"

/* API function definitions -----------------------------------------------*/
LED_bar_t LED_init_bar()
{
	LED_bar_t LED_bar = {
			.motor_error = IO_digitalPin_init(GPIOD, LED_Stoerung_Pin, LED_OFF),
			.operating_mode = {
					.manual = IO_digitalPin_init(GPIOD, LED_Manuell_Pin, LED_ON),
					.automatic = IO_digitalPin_init(GPIOB, LED_Automatik_Pin, LED_OFF),
			},
			.sail_adjustment_mode = {
					.rollung = IO_digitalPin_init(GPIOE, LED_Rollen_Pin, LED_OFF),
					.trimmung = IO_digitalPin_init(GPIOB, LED_Trimmen_Pin, LED_OFF),
			},
			.center_pos_set = IO_digitalPin_init(GPIOD, LED_Kalibrieren_Speichern_Pin, LED_OFF),
	};
	return LED_bar;
}

/* void LED_switch(LED_t *led_ptr, GPIO_PinState led_state)
 * 	Description:
 * 	 - switch LED on or off
 * 	 - led_state: use macros LED_ON / LED_OFF
 */
void LED_switch(LED_t *led_ptr, GPIO_PinState led_state)
{
	IO_digitalWrite(led_ptr, led_state);
}

/* void LED_toggle(LED_t *led_ptr)
 * 	Description:
 * 	 - toggle state of LED
 */
void LED_toggle(LED_t *led_ptr)
{
	IO_digitalToggle(led_ptr);
}

/* void LED_set_operating_mode(LED_bar_t *led_bar_ptr, IO_operating_mode_t operating_mode)
 * 	Description:
 * 	 - depending on parameter "operating_mode" (IO_operating_mode_manual / ..._automatic)
 * 	   switch corresponding LEDs on and off
 */
void LED_set_operating_mode(LED_bar_t *led_bar_ptr, IO_operating_mode_t operating_mode)
{
	GPIO_PinState automatic, manual;
	switch(operating_mode)
	{
		case IO_operating_mode_manual:
			manual = LED_ON, automatic = LED_OFF; break;
		case IO_operating_mode_automatic:
			manual = LED_OFF, automatic = LED_ON; break;
	}
	LED_switch(&led_bar_ptr->operating_mode.automatic, automatic);
	LED_switch(&led_bar_ptr->operating_mode.manual, manual);
}

/* void LED_toggle_sail_adjustment_mode(LED_bar_t *led_bar_ptr)
 * 	Description:
 * 	 - when sail adjustment mode changes (rollung / trimmung)
 * 	   the corresponding LEDs are switched on and off
 */
void LED_toggle_sail_adjustment_mode(LED_bar_t *led_bar_ptr)
{
	LED_toggle(&led_bar_ptr->sail_adjustment_mode.rollung);
	LED_toggle(&led_bar_ptr->sail_adjustment_mode.trimmung);
}



/*
 * LED.c
 *
 *  Created on: 03.05.2023
 *      Author: Bene
 */

#include "LED.h"

/* API function definitions -----------------------------------------------*/
LED_t LED_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, LED_State_t state)
{
	return IO_digital_Out_Pin_init(GPIOx, GPIO_Pin, state);
}
/* void LED_switch(LED_t *led_ptr, LED_state_t led_state)
 * 	Description:
 * 	 - switch LED on or off
 * 	 - led_state: use macros LED_ON / LED_OFF
 */
void LED_switch(LED_t *led_ptr, LED_State_t led_state)
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



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

void LED_blink(LED_t *led_ptr)
{
	for(uint8_t i = 0; i < 9; i++)
	{
		LED_toggle(led_ptr);
		HAL_Delay(200);
	}
	LED_switch(led_ptr, LED_OFF);
}


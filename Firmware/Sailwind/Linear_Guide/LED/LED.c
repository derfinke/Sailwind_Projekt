/*
 * LED.c
 *
 *  Created on: 03.05.2023
 *      Author: Bene
 */

#include "LED.h"

#define LED_BLINK_COUNT 9

static LED_t *led_to_blink_ptr = {0};

/* API function definitions -----------------------------------------------*/
LED_t LED_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, LED_State_t state)
{
	LED_t led = {
			.pin = IO_digital_Out_Pin_init(GPIOx, GPIO_Pin, state)
	};
	return led;
}
/* void LED_switch(LED_t *led_ptr, LED_state_t led_state)
 * 	Description:
 * 	 - switch LED on or off
 * 	 - led_state: use macros LED_ON / LED_OFF
 */
void LED_switch(LED_t *led_ptr, LED_State_t led_state)
{
	IO_digitalWrite(&led_ptr->pin, led_state);
}

/* void LED_toggle(LED_t *led_ptr)
 * 	Description:
 * 	 - toggle state of LED
 */
void LED_toggle(LED_t *led_ptr)
{
	IO_digitalToggle(&led_ptr->pin);
}

void LED_blink(LED_t *led_ptr, LED_State_t final_state, TIM_HandleTypeDef *htim_blink_ptr)
{
	led_ptr->blink_counter = 0;
	led_ptr->final_state = final_state;
	led_to_blink_ptr = led_ptr;
	HAL_TIM_Base_Start_IT(htim_blink_ptr);
}

void LED_blink_callback(TIM_HandleTypeDef *htim_blink_ptr)
{
	led_to_blink_ptr->blink_counter++;
	if (led_to_blink_ptr->blink_counter >= LED_BLINK_COUNT)
	{
		LED_switch(led_to_blink_ptr, led_to_blink_ptr->final_state);
		HAL_TIM_Base_Stop_IT(htim_blink_ptr);
		led_to_blink_ptr->blink_counter = 0;
		return;
	}
	LED_toggle(led_to_blink_ptr);
}


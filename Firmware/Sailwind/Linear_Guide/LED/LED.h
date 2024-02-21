/*
 * LED.h
 *
 *  Created on: 03.05.2023
 *      Author: Bene
 */

#ifndef LED_LED_H_
#define LED_LED_H_

#include "IO.h"

/* typedefs -----------------------------------------------------------*/
typedef GPIO_PinState LED_State_t;

typedef struct {
	IO_digitalPin_t pin;
	uint16_t blink_counter;
	LED_State_t final_state;
} LED_t;

/* defines -------------------------------------------------------------------*/
#define LED_OFF GPIO_PIN_RESET
#define LED_ON GPIO_PIN_SET

/* API function prototypes ---------------------------------------------------*/
LED_t LED_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, LED_State_t state);
void LED_switch(LED_t *led_ptr, LED_State_t led_state);
void LED_toggle(LED_t *led_ptr);
void LED_blink(LED_t *led_ptr, LED_State_t final_state, TIM_HandleTypeDef *htim_blink_ptr);
void LED_blink_callback(TIM_HandleTypeDef *htim_blink_ptr);

#endif /* LED_LED_H_ */

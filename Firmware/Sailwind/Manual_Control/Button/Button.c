/*
 * Button.c
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#include "Button.h"

/* API function definitions -----------------------------------------------*/
Button_t Button_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	return (Button_t) IO_digital_Pin_init(GPIOx, GPIO_Pin);
}

/* boolean_t Button_state_changed(Button_t *button_ptr)
 * 	Description:
 *   - returns True, if the state of the digitalPin of the given button has changed
 */
boolean_t Button_state_changed(Button_t *button_ptr)
{
	return IO_digitalRead_state_changed(button_ptr);
}


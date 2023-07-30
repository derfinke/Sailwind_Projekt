/*
 * Button.h
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#ifndef BUTTON_BUTTON_H_
#define BUTTON_BUTTON_H_

#include "../IO/IO.h"

/* typedefs ------------------------------------------------------------------*/
typedef IO_digitalPin_t Button_t;
typedef GPIO_PinState Button_state_t;

/* defines -------------------------------------------------------------------*/
#define BUTTON_PRESSED GPIO_PIN_RESET
#define BUTTON_RELEASED GPIO_PIN_SET

/* API function prototypes ---------------------------------------------------*/
Button_t Button_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
boolean_t Button_state_changed(Button_t *button_ptr);


#endif /* BUTTON_BUTTON_H_ */

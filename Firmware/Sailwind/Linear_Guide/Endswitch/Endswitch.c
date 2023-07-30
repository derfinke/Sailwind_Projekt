/*
 * Endswitch.c
 *
 *  Created on: 28.07.2023
 *      Author: Bene
 */

#include "Endswitch.h"

/* API function definitions -----------------------------------------------*/
Endswitch_t Endswitch_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	return (Endswitch_t) IO_digital_Pin_init(GPIOx, GPIO_Pin);
}

/* boolean_t Endswitch_detected(Endswitch_t *endswitch_ptr)
 *  Description:
 *   - return True, if the given end switch is reached by the linear guide
 */
boolean_t Endswitch_detected(Endswitch_t *endswitch_ptr)
{
	return (boolean_t) IO_digitalRead(endswitch_ptr);
}

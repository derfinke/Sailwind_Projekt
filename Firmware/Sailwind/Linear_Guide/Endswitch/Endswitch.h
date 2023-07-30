/*
 * Endswitch.h
 *
 *  Created on: 28.07.2023
 *      Author: Bene
 */

#ifndef ENDSWITCH_ENDSWITCH_H_
#define ENDSWITCH_ENDSWITCH_H_

#include "../IO/IO.h"

/* defines ------------------------------------------------------------*/


/* typedefs -----------------------------------------------------------*/
typedef IO_digitalPin_t Endswitch_t;


/* API function prototypes -----------------------------------------------*/
Endswitch_t Endswitch_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
boolean_t Endswitch_detected(Endswitch_t *endswitch_ptr);

#endif /* ENDSWITCH_ENDSWITCH_H_ */

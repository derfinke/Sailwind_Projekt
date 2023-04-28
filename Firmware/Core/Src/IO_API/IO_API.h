/*
 * IO_API.h
 *
 *  Created on: Apr 17, 2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_H_
#define SRC_IO_API_H_

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "main.h"

/* typedefs -----------------------------------------------------------*/

typedef struct {
	char* name;
	char* unit;
	ADC_HandleTypeDef *hadc;
	uint32_t channel;
	uint16_t maxValue;
	uint16_t currentValue;
	uint16_t adc_value;
} analogSensor;

typedef struct {
	char* unit;
	DAC_HandleTypeDef *hdac;
	uint32_t channel;
	uint16_t maxValue;
	uint16_t currentValue;
	uint32_t adc_value;
} analogActuator;

typedef struct {
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	GPIO_PinState state;
} digitalPin;



/* defines ------------------------------------------------------------*/
#define ANALOG_MAX 4096


/* API function prototypes -----------------------------------------------*/
void IO_writeDigitalOUT(digitalPin *digital_OUT, GPIO_PinState state);
void IO_toggleDigitalOUT(digitalPin *digital_OUT);
GPIO_PinState IO_readDigitalIN(digitalPin *digital_IN);
void IO_readAnalogValue(analogSensor *sensor);
void IO_printAnalogValue(analogSensor sensor);
void IO_writeAnalogValue(analogActuator *actuator, uint16_t value);


/* private function prototypes -----------------------------------------------*/
void convertFromADC(analogSensor *sensor);
void convertToDAC(analogActuator *actuator);

#endif /* SRC_IO_API_H_ */

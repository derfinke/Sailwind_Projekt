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
typedef enum {False, True} boolean_t;

typedef struct {
	char* name;
	char* unit;
	ADC_HandleTypeDef *hadc;
	uint32_t channel;
	uint16_t maxValue;
	uint16_t currentValue;
	uint16_t adc_value;
} analogSensor_t;

typedef struct {
	char* unit;
	DAC_HandleTypeDef *hdac;
	uint32_t channel;
	uint16_t maxValue;
	uint16_t currentValue;
	uint32_t adc_value;
} analogActuator_t;

typedef struct {
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	GPIO_PinState state;
} digitalPin_t;



/* defines ------------------------------------------------------------*/
#define ANALOG_MAX 4096
#define TRUE 1
#define FALSE 0


/* API function prototypes -----------------------------------------------*/
void IO_writeDigitalOUT(digitalPin_t *digital_OUT, GPIO_PinState state);
void IO_toggleDigitalOUT(digitalPin_t *digital_OUT);
GPIO_PinState IO_readDigitalIN(digitalPin_t *digital_IN);
void IO_readAnalogValue(analogSensor_t *sensor);
void IO_printAnalogValue(analogSensor_t sensor);
void IO_writeAnalogValue(analogActuator_t *actuator, uint16_t value);


#endif /* SRC_IO_API_H_ */

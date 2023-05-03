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
} IO_analogSensor_t;

typedef struct {
	char* unit;
	DAC_HandleTypeDef *hdac;
	uint32_t channel;
	uint16_t maxValue;
	uint16_t currentValue;
	uint32_t adc_value;
} IO_analogActuator_t;

typedef struct {
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	GPIO_PinState state;
} IO_digitalPin_t;



/* defines ------------------------------------------------------------*/
#define ANALOG_MAX 4096


/* API function prototypes -----------------------------------------------*/
void IO_writeDigitalOUT(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state);
void IO_toggleDigitalOUT(IO_digitalPin_t *digital_OUT_ptr);
GPIO_PinState IO_readDigitalIN(IO_digitalPin_t *digital_IN_ptr);
void IO_readAnalogValue(IO_analogSensor_t *sensor_ptr);
void IO_printAnalogValue(IO_analogSensor_t sensor);
void IO_writeAnalogValue(IO_analogActuator_t *actuator_ptr, uint16_t value);


#endif /* SRC_IO_API_H_ */

/*
 * IO.h
 *
 *  Created on: Apr 17, 2023
 *      Author: Bene
 */

#ifndef IO_IO_H_
#define IO_IO_H_

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "main.h"
#include "boolean.h"

/* typedefs -----------------------------------------------------------*/

typedef struct {
	char* name;					
	char* unit;						
	ADC_HandleTypeDef *hadc_ptr; 	
	uint32_t hadc_channel;
	float maxConvertedValue;
	float currentConvertedValue;
	uint16_t adc_value;			
} IO_analogSensor_t;

typedef struct {
	DAC_HandleTypeDef *hdac_ptr;
	uint32_t hdac_channel;
	float maxConvertedValue;
	float limitConvertedValue;
	float currentConvertedValue;
	uint16_t dac_value;
} IO_analogActuator_t;

typedef struct {
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	GPIO_PinState state;
} IO_digitalPin_t;



/* defines ------------------------------------------------------------*/
#define ANALOG_MAX 4096


/* API function prototypes -----------------------------------------------*/
IO_digitalPin_t IO_digital_Out_Pin_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState state);
IO_digitalPin_t IO_digital_Pin_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void IO_digitalWrite(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state);
void IO_digitalToggle(IO_digitalPin_t *digital_OUT_ptr);
GPIO_PinState IO_digitalRead(IO_digitalPin_t *digital_IN_ptr);
boolean_t IO_digitalRead_state_changed(IO_digitalPin_t *digital_IN_ptr);
boolean_t IO_digitalRead_rising_edge(IO_digitalPin_t *digital_IN_ptr);
void IO_analogRead(IO_analogSensor_t *sensor_ptr);
void IO_analogPrint(IO_analogSensor_t sensor);
void IO_analogWrite(IO_analogActuator_t *actuator_ptr, float value);


#endif /* IO_IO_H_ */

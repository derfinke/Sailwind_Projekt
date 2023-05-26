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
#include "std_types.h"

/* typedefs -----------------------------------------------------------*/

typedef struct {
	char* name;					//nur für print ausgabe wichtig
	char* unit;						//nur für print ausgabe wichtig
	ADC_HandleTypeDef *hadc; 	//zb "hadc1" wie bereits in der main automatisch generiert (hadc1 ist bereits ein pointer)
	uint32_t channel;			//zb "ADC_CHANNEL_0" (je nachdem wo der sensor eingetragen ist
	float maxValue;			//max wert der zu messenden Größe (wichtig, um den aktuellen wert aus dem adc wert zu berechnen
	float currentValue;		//wird automatisch gesetzt
	uint16_t adc_value;			//wird automatisch gesetzt
} IO_analogSensor_t;

typedef struct {
	DAC_HandleTypeDef *hdac;
	uint32_t channel;
	float maxValue;
	float currentValue;
	uint16_t adc_value;
} IO_analogActuator_t;

typedef struct {
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	GPIO_PinState state;
} IO_digitalPin_t;



/* defines ------------------------------------------------------------*/
#define ANALOG_MAX 4096


/* API function prototypes -----------------------------------------------*/
void IO_digitalWrite(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state);
void IO_digitalToggle(IO_digitalPin_t *digital_OUT_ptr);
GPIO_PinState IO_digitalRead(IO_digitalPin_t *digital_IN_ptr);
boolean_t IO_digitalRead_state_changed(IO_digitalPin_t *digital_IN_ptr);
boolean_t IO_digitalRead_rising_edge(IO_digitalPin_t *digital_IN_ptr);
void IO_analogRead(IO_analogSensor_t *sensor_ptr);
void IO_analogPrint(IO_analogSensor_t sensor);
void IO_analogWrite(IO_analogActuator_t *actuator_ptr, float value);


#endif /* SRC_IO_API_H_ */

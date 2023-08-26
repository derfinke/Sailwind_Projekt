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

typedef enum {
  Distance_Sensor, Wind_Sensor, Current_Sensor, Force_Sensor
} IO_SensorType_t;

/** @struct IO_analogSensor_t
 *  @brief Struct that is used to bundle all values needed for analog Signal conversion
 *  @var IO_analogSensor_t::measured_value
 *  Value output by the sensor in the unit of the measured signal
 *  @var IO_analogSensor_t::*hadc_ptr
 *  ADC pointer
 *  @var IO_analogSensor_t::ADC_value
 *  ADC Value
 *  @var IO_analogSensor_t::max_possible_value
 *  Maximum value that the sensor can output (in desired unit)
 *  @var IO_analogSensor_t::min_possible_value
 *  Minimum value that the sensor can output (in desired unit)
 *  @var IO_analogSensor_t::Analog_signal_is_current
 *  True: Current signal with values reaching from 4-20ma
 *  False: Voltage signal with values reaching from 0-3.3V
 */
typedef struct {
	ADC_HandleTypeDef *hadc_ptr;
  IO_SensorType_t Sensor_type;
  uint32_t ADC_Channel;
  uint32_t ADC_Rank;
  uint16_t ADC_value;
  uint16_t max_possible_value;
	int16_t measured_value;
	int16_t min_possible_value;
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
void IO_Get_Measured_Value(IO_analogSensor_t *Sensor);

#endif /* IO_IO_H_ */

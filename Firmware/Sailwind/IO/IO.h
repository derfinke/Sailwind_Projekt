/**
 * \file IO.h
 * @date 18 Jun 2023
 * @brief Access to Analog and Digital IO Pins
 */

#ifndef IO_IO_H_
#define IO_IO_H_

#include "stm32f4xx_hal.h"
#include "boolean.h"
#include "main.h"
#include <stdio.h>

/* typedefs -----------------------------------------------------------*/

typedef enum {
  Distance_Sensor,
  Wind_Sensor_speed,
  Wind_Sensor_direction,
  Current_Sensor,
  Force_Sensor
} IO_SensorType_t;

/** @struct IO_analogSensor_t
 *  @brief Struct that is used to bundle all values needed for analog Signal conversion
 *  @var IO_analogSensor_t::*hadc_ptr
 *  ADC pointer
 *  @var IO_analogSensor_t::Sensor_type
 *  type of sensor connected
 *  @var IO_analogSensor_t::ADC_Channel
 *  Used ADC_Channel
 *  @var IO_analogSensor_t::ADC_Rank
 *  Rank in the used ADC_Channel
 *  @var IO_analogSensor_t::measured_value
 *  Value output by the sensor in the unit of the measured signal
 *  @var IO_analogSensor_t::ADC_value
 *  Average ADC value
 *  @var IO_analogSensor_t::max_possible_value
 *  Maximum value that the sensor can output (in desired unit)
 *  @var IO_analogSensor_t::min_possible_value
 *  Minimum value that the sensor can output (in desired unit)
 *  @var IO_analogSensor_t::measured_value
 *  Sensor value in desired unit
 */
typedef struct {
  ADC_HandleTypeDef *hadc_ptr;
  IO_SensorType_t Sensor_type;
  uint32_t ADC_Channel;
  uint32_t ADC_Rank;
  uint16_t ADC_value;
  uint16_t max_possible_value;
  uint16_t measured_value;
  uint16_t min_possible_value;
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
  GPIO_TypeDef *GPIOx;
  uint16_t GPIO_Pin;
  GPIO_PinState state;
} IO_digitalPin_t;

/* API function prototypes -----------------------------------------------*/
IO_digitalPin_t IO_digital_Out_Pin_init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                                        GPIO_PinState state);
IO_digitalPin_t IO_digital_Pin_init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void IO_digitalWrite(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state);
void IO_digitalToggle(IO_digitalPin_t *digital_OUT_ptr);
GPIO_PinState IO_digitalRead(IO_digitalPin_t *digital_IN_ptr);
boolean_t IO_digitalRead_state_changed(IO_digitalPin_t *digital_IN_ptr);
boolean_t IO_digitalRead_rising_edge(IO_digitalPin_t *digital_IN_ptr);
void IO_analogRead(IO_analogSensor_t *sensor_ptr);
void IO_analogPrint(IO_analogSensor_t sensor);
void IO_analogWrite(IO_analogActuator_t *actuator_ptr, float value);

/**
 * @brief initialize adc of distance sensor
 * @param distance_sensor:ptr to distance sensor instance
 * @param hadc1:ptr to hadc1 instance
 * @retval none
 */
void IO_init_distance_sensor(ADC_HandleTypeDef *hadc1);

/**
 * @brief initialize adc of current sensor
 * @param current_sensor:ptr to distance sensor instance
 * @param hadc3:ptr to hadc3 instance
 * @retval none
 */
void IO_init_current_sensor(ADC_HandleTypeDef *hadc3);

/**
 * @brief initialize adcs of wind sensor
 * @param wind_sensor_speed: ptr to wind speed sensor instance
 * @param wind_sensor_direction: ptr to wind direction sensor instance
 * @param hadc3:ptr to hadc3 instance
 * @retval none
 */
void IO_init_wind_sensor(IO_analogSensor_t *wind_sensor_speed,
                         IO_analogSensor_t *wind_sensor_direction,
                         ADC_HandleTypeDef *hadc3);

/**
 * @brief get the measured value of a sensor as the real output value depending on the sensor type
 * @param Sensor: ptr to a Sensor
 * @retval none
 */
void IO_Get_Measured_Value(IO_analogSensor_t *Sensor);

void IO_get_distance_sensor(IO_analogSensor_t *current_sensor_ptr);

void IO_get_current_sensor(IO_analogSensor_t *current_sensor_ptr);

#endif /* IO_IO_H_ */

/*
 * IO.c
 *
 *  Created on: Apr 17, 2023
 *      Author: Bene
 */
#include "IO.h"

#define ADC_RESOLOUTION               (4096 - 1)
#define DAC_RESOLOUTION               (4096 - 1)
#define DISTANCE_SENSOR_RESISTOR      270
#define WIND_SPEED_RESISTOR           160
#define WIND_DIRECTION_RESISTOR       160
#define SENSOR_MAX_AMP                0.02
#define SENSOR_MIN_AMP                0.004
#define DISTANCE_SENSOR_MAX_DISTANCE  1500
#define DISTANCE_SENSOR_MIN_DISTANCE  25
#define NUM_OF_ADC_SAMPLES            32

/* private function prototypes -----------------------------------------------*/
static void IO_convertToDAC(IO_analogActuator_t *actuator_ptr);

static void IO_Select_ADC_CH(IO_analogSensor_t *Sensor);

static void IO_Get_ADC_Value(uint8_t num_of_adc_samples,
                             IO_analogSensor_t *Sensor);

/* IO_digitalPin_t IO_digital_Out_Pin_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState state)
 *  Description:
 *   - return an IO_digitalPin Instance with members passed as parameters
 *   - write the initial state to the GPIO Pin
 */
IO_digitalPin_t IO_digital_Out_Pin_init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                                        GPIO_PinState state) {
  IO_digitalPin_t digitalPin = IO_digital_Pin_init(GPIOx, GPIO_Pin);
  IO_digitalWrite(&digitalPin, state);
  return digitalPin;
}

IO_digitalPin_t IO_digital_Pin_init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
  IO_digitalPin_t digitalPin = { .GPIOx = GPIOx, .GPIO_Pin = GPIO_Pin, };
  return digitalPin;
}

/* void IO_digitalWrite(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state)
 * 	Description:
 * 	 - save the new state (from parameter "state") in the digitalPin structure reference
 * 	 - write the state to the HAL_GPIO_Pin
 */
void IO_digitalWrite(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state) {
  digital_OUT_ptr->state = state;
  HAL_GPIO_WritePin(digital_OUT_ptr->GPIOx, digital_OUT_ptr->GPIO_Pin,
                    digital_OUT_ptr->state);
}

/* void IO_digitalToggle(IO_digitalPin_t *digital_OUT_ptr)
 *  Description:
 *   - toggle the state of the digitalPin reference
 *   - write the new state to the HAL_GPIO_Pin
 */
void IO_digitalToggle(IO_digitalPin_t *digital_OUT_ptr) {
  digital_OUT_ptr->state ^= GPIO_PIN_SET;
  HAL_GPIO_WritePin(digital_OUT_ptr->GPIOx, digital_OUT_ptr->GPIO_Pin,
                    digital_OUT_ptr->state);
}

/* GPIO_PinState IO_digitalRead(IO_digitalPin_t *digital_IN_ptr)
 *  Description:
 *   - read current state of HAL_GPIO_Pin and save it to the digitalPin reference
 *   - return the updated state
 */
GPIO_PinState IO_digitalRead(IO_digitalPin_t *digital_IN_ptr) {
  digital_IN_ptr->state = HAL_GPIO_ReadPin(digital_IN_ptr->GPIOx,
                                           digital_IN_ptr->GPIO_Pin);
  return digital_IN_ptr->state;
}

/* boolean_t IO_digitalRead_state_changed(IO_digitalPin_t *digital_IN_ptr)
 *  Description:
 *   - read current state of the digitalPin reference and compare with previous state
 *   - return True, if the state has changed
 */
boolean_t IO_digitalRead_state_changed(IO_digitalPin_t *digital_IN_ptr) {
  GPIO_PinState previous_state = digital_IN_ptr->state;
  GPIO_PinState current_state = IO_digitalRead(digital_IN_ptr);
  return current_state ^ previous_state;
}

/* boolean_t IO_digitalRead_rising_edge(IO_digitalPin_t *digital_IN_ptr)
 *  Description:
 *   - return True, if the state of the digitalPin reference has changed from 0 to 1
 */
boolean_t IO_digitalRead_rising_edge(IO_digitalPin_t *digital_IN_ptr) {
  GPIO_PinState previous_state = digital_IN_ptr->state;
  GPIO_PinState current_state = IO_digitalRead(digital_IN_ptr);
  return previous_state == GPIO_PIN_RESET && current_state == GPIO_PIN_SET;
}

/* void IO_analogWrite(IO_analogActuator_t *actuator_ptr, float value)
 *  Description:
 *   - save analog value from parameter (limited to given "limitConvertedValue") to the analogActuator reference
 *   - convert analog to digital value and write it to the dac channel specified in the actuator reference
 */
void IO_analogWrite(IO_analogActuator_t *actuator_ptr, float value) {
  actuator_ptr->currentConvertedValue =
      value <= actuator_ptr->limitConvertedValue ?
          value : actuator_ptr->limitConvertedValue;
  IO_convertToDAC(actuator_ptr);
  HAL_DAC_SetValue(actuator_ptr->hdac_ptr, actuator_ptr->hdac_channel,
                   DAC_ALIGN_12B_R, actuator_ptr->dac_value);
  HAL_DAC_Start(actuator_ptr->hdac_ptr, actuator_ptr->hdac_channel);
}

/* static void convertToDAC(IO_analogActuator_t *actuator_ptr)
 *  Description:
 *   - convert the analog value to the corresponding digital value and save it to the actuator reference
 */
static void IO_convertToDAC(IO_analogActuator_t *actuator_ptr) {
  actuator_ptr->dac_value = (uint16_t) (actuator_ptr->currentConvertedValue
      / actuator_ptr->maxConvertedValue * DAC_RESOLOUTION);
}

static void IO_Select_ADC_CH(IO_analogSensor_t *Sensor) {
  ADC_ChannelConfTypeDef sConfig = { 0 };

  sConfig.Channel = Sensor->ADC_Channel;
  sConfig.Rank = Sensor->ADC_Rank;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(Sensor->hadc_ptr, &sConfig) != HAL_OK) {
    Error_Handler();
  }
}

void IO_Get_Measured_Value(IO_analogSensor_t *Sensor) {
  float ADC_voltage = 0.0;

  IO_Select_ADC_CH(Sensor);

  IO_Get_ADC_Value(NUM_OF_ADC_SAMPLES, Sensor);

  ADC_voltage = (float) ((Sensor->ADC_value * 3.3) / ADC_RESOLOUTION);

  switch (Sensor->Sensor_type) {
    case Distance_Sensor:
      Sensor->measured_value = (uint16_t) (((Sensor->max_possible_value
          - Sensor->min_possible_value) / (SENSOR_MAX_AMP - SENSOR_MIN_AMP))
          * ((ADC_voltage / DISTANCE_SENSOR_RESISTOR) - SENSOR_MIN_AMP))
          + Sensor->min_possible_value;
      break;
    case Wind_Sensor:
      break;
    case Current_Sensor:
      break;
    case Force_Sensor:
      break;
  }
}

static void IO_Get_ADC_Value(uint8_t num_of_adc_samples,
                             IO_analogSensor_t *Sensor) {

  uint32_t ADC_val = 0;

  for (uint8_t i = 0; i < num_of_adc_samples; i++) {
    HAL_ADC_Start(Sensor->hadc_ptr);
    HAL_ADC_PollForConversion(Sensor->hadc_ptr, HAL_MAX_DELAY);
    ADC_val = HAL_ADC_GetValue(Sensor->hadc_ptr);
    HAL_ADC_Stop(Sensor->hadc_ptr);
  }

  Sensor->ADC_value = (uint16_t) (ADC_val / num_of_adc_samples);

}

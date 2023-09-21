/**
 * \file IO.c
 * @date 18 Jun 2023
 * @brief Access to Analog and Digital IO Pins
 */
#include "IO.h"

#define ADC_RESOLOUTION                               (4096 - 1)
#define DAC_RESOLOUTION                               (4096 - 1)
#define DISTANCE_SENSOR_RESISTOR                      270
#define WIND_SPEED_RESISTOR                           160
#define WIND_DIRECTION_RESISTOR                       160
#define DISTANCE_SENSOR_MAX_AMP                       0.01106
#define DISTANCE_SENSOR_MIN_AMP                       0.00426
#define WIND_SENSOR_MAX_AMP                           0.02
#define WIND_SENSOR_MIN_AMP                           0.004
#define CURRENT_SENSOR_MAX_VOLT                       3.057
#define CURRENT_SENSOR_MIN_VOLT                       1.607
#define NUM_OF_ADC_SAMPLES_DISTANCE_SENSOR            48
#define NUM_OF_DISPERESED_SAMPLES_DISTANCE_SENSOR     44

/* private function prototypes -----------------------------------------------*/

static void IO_Select_ADC_CH(IO_analogSensor_t *Sensor);

/**
 * @brief Take adc values and remove lowest values. Takes an average of the left over values
 * @param num_of_adc_samples: number of taken adc values
 * @param num_of_disperesed_samples: number of adc values to be disposed
 * @param Sensor: ptr to a Sensor
 * @retval none
 */

static void IO_Get_ADC_Value(uint8_t num_of_adc_samples,
                             uint8_t num_of_disperesed_samples,
                             IO_analogSensor_t *Sensor);

/**
 * @brief Sort taken adc values from lowest to highest
 * @param ADC_Values: ptr to array of taken adc values
 * @param num_of_adc_samples: number of taken adc values
 * @retval none
 */
static void IO_Sort_ADC_Values(uint16_t *ADC_values, uint8_t num_of_adc_samples);

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
  IO_digitalPin_t digitalPin = { .GPIOx = GPIOx, .GPIO_Pin = GPIO_Pin};
  digitalPin.state = IO_digitalRead(&digitalPin);
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
//  actuator_ptr->currentConvertedValue = value <= actuator_ptr->limitConvertedValue ? value : actuator_ptr->limitConvertedValue;
//  IO_convertToDAC(actuator_ptr);
  HAL_DAC_SetValue(actuator_ptr->hdac_ptr, actuator_ptr->hdac_channel,
  DAC_ALIGN_12B_R, (uint16_t) value);
  HAL_DAC_Start(actuator_ptr->hdac_ptr, actuator_ptr->hdac_channel);
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

  switch (Sensor->Sensor_type) {
    case Distance_Sensor:
      IO_Get_ADC_Value(NUM_OF_ADC_SAMPLES_DISTANCE_SENSOR,
      NUM_OF_DISPERESED_SAMPLES_DISTANCE_SENSOR,
                       Sensor);
      ADC_voltage = (float) ((Sensor->ADC_value * 3.3) / ADC_RESOLOUTION);
      Sensor->measured_value =
          (uint16_t) (((Sensor->max_possible_value - Sensor->min_possible_value)
              / (DISTANCE_SENSOR_MAX_AMP - DISTANCE_SENSOR_MIN_AMP))
              * ((ADC_voltage / DISTANCE_SENSOR_RESISTOR)
                  - DISTANCE_SENSOR_MIN_AMP)) + Sensor->min_possible_value;
      break;
    case Current_Sensor:
      IO_Get_ADC_Value(48, 0, Sensor);
      ADC_voltage = (float) ((Sensor->ADC_value * 3.3) / ADC_RESOLOUTION);
      Sensor->measured_value = (uint16_t) (((Sensor->max_possible_value
          - Sensor->min_possible_value)
          / (CURRENT_SENSOR_MAX_VOLT - CURRENT_SENSOR_MIN_VOLT))
          * (ADC_voltage - CURRENT_SENSOR_MIN_VOLT))
          + Sensor->min_possible_value;
      break;
    case Wind_Sensor_speed:
      IO_Get_ADC_Value(48, 0, Sensor);
      ADC_voltage = (float) (Sensor->ADC_value * 3.3 / ADC_RESOLOUTION);
      Sensor->measured_value = (int16_t) (((Sensor->max_possible_value
          - Sensor->min_possible_value)
          / (WIND_SENSOR_MAX_AMP - WIND_SENSOR_MIN_AMP))
          * (ADC_voltage - WIND_SENSOR_MIN_AMP)) + Sensor->min_possible_value;
      break;
    case Wind_Sensor_direction:
      IO_Get_ADC_Value(48, 0, Sensor);
      ADC_voltage = (float) (Sensor->ADC_value * 3.3 / ADC_RESOLOUTION);
      Sensor->measured_value = (int16_t) (((Sensor->max_possible_value
          - Sensor->min_possible_value)
          / (WIND_SENSOR_MAX_AMP - WIND_SENSOR_MIN_AMP))
          * (ADC_voltage - WIND_SENSOR_MIN_AMP)) + Sensor->min_possible_value;
      break;
    case Force_Sensor:
      break;
    default:
      printf("no valid sensor\r\n");
  }
}

static void IO_Get_ADC_Value(uint8_t num_of_adc_samples,
                             uint8_t num_of_disperesed_samples,
                             IO_analogSensor_t *Sensor) {

  uint16_t ADC_val[num_of_adc_samples];
  uint32_t All_ADC_val = 0;

  for (uint8_t i = 0; i < num_of_adc_samples; i++) {
    HAL_ADC_Start(Sensor->hadc_ptr);
    HAL_ADC_PollForConversion(Sensor->hadc_ptr, HAL_MAX_DELAY);
    ADC_val[i] = HAL_ADC_GetValue(Sensor->hadc_ptr);
    HAL_ADC_Stop(Sensor->hadc_ptr);
  }
  IO_Sort_ADC_Values(ADC_val, num_of_adc_samples);
  for (uint8_t i = num_of_disperesed_samples / 2;
      i < num_of_adc_samples - num_of_disperesed_samples / 2; i++) {
    All_ADC_val += ADC_val[i];
  }
  Sensor->ADC_value = (uint16_t) (All_ADC_val
      / (num_of_adc_samples - num_of_disperesed_samples));

}

static void IO_Sort_ADC_Values(uint16_t *ADC_values, uint8_t num_of_adc_samples) {
  uint8_t exchange = 1U;
  uint16_t tmp = 0U;
  /* Sort tab */
  while (exchange == 1) {
    exchange = 0;
    for (uint8_t i = 0U; i < num_of_adc_samples - 1U; i++) {
      if (ADC_values[i] > ADC_values[i + 1U]) {
        tmp = ADC_values[i];
        ADC_values[i] = ADC_values[i + 1U];
        ADC_values[i + 1U] = tmp;
        exchange = 1U;
      }
    }
  }
}

void IO_init_distance_sensor(IO_analogSensor_t *distance_sensor,
                             ADC_HandleTypeDef *hadc1) {
  distance_sensor->Sensor_type = Distance_Sensor;
  distance_sensor->ADC_Channel = ADC_CHANNEL_0;
  distance_sensor->hadc_ptr = hadc1;
  distance_sensor->ADC_Rank = 1;
  distance_sensor->max_possible_value = 730;
  distance_sensor->min_possible_value = 30;
  IO_Get_Measured_Value(distance_sensor);
  printf("distance sensor init done\r\n");
  printf("init distance: %umm\r\n", distance_sensor->measured_value);
}

void IO_init_current_sensor(IO_analogSensor_t *current_sensor,
                            ADC_HandleTypeDef *hadc3) {
  current_sensor->Sensor_type = Current_Sensor;
  current_sensor->ADC_Channel = ADC_CHANNEL_8;
  current_sensor->hadc_ptr = hadc3;
  current_sensor->ADC_Rank = 1;
  current_sensor->max_possible_value = 7250;
  current_sensor->min_possible_value = 0;
  IO_Get_Measured_Value(current_sensor);
  printf("current sensor init done\r\n");
  printf("init current: %umA\r\n", current_sensor->measured_value);
}

void IO_init_wind_sensor(IO_analogSensor_t *wind_sensor_speed,
                         IO_analogSensor_t *wind_sensor_direction,
                         ADC_HandleTypeDef *hadc3) {
  wind_sensor_speed->Sensor_type = Wind_Sensor_speed;
  wind_sensor_speed->ADC_Channel = ADC_CHANNEL_7;
  wind_sensor_speed->hadc_ptr = hadc3;
  wind_sensor_speed->ADC_Rank = 2;
  wind_sensor_speed->max_possible_value = 16667;
  wind_sensor_speed->min_possible_value = 0;
  IO_Get_Measured_Value(wind_sensor_speed);
  wind_sensor_direction->Sensor_type = Wind_Sensor_direction;
  wind_sensor_direction->ADC_Channel = ADC_CHANNEL_5;
  wind_sensor_direction->hadc_ptr = hadc3;
  wind_sensor_direction->ADC_Rank = 3;
  wind_sensor_direction->max_possible_value = 0;
  wind_sensor_direction->min_possible_value = 359;
  IO_Get_Measured_Value(wind_sensor_direction);
  printf("wind sensor init done\r\n");
  printf("init speed:%umm/s\r\n init dir:%u°\r\n",
         wind_sensor_speed->measured_value,
         wind_sensor_direction->measured_value);
}

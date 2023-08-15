/*
 * Distance_Sensor.c
 *
 */
#include "Distance_Sensor.h"

#include <stdint.h>
#include <stdio.h>

#include "main.h"

#define ADC_RESOLUTION 					(4096 - 1)
#define DISTANCE_SENSOR_MAX_AMP 		0.02
#define DISTANCE_SENSOR_MIN_AMP 		0.004
#define DISTANCE_SENSOR_MAX_DISTANCE	1500
#define DISTANCE_SENSOR_MIN_DISTANCE	25
#define RESISTOR						270

static void Distance_Sensor_Select_ADC(void);

void Distance_Sensor_Get_Distance(uint16_t Distance_in_mm) {
  uint32_t ADC_val = 0;
  float ADC_voltage = 0.0;

  Distance_Sensor_Select_ADC();

  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 1000);
  ADC_val = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);

  ADC_voltage = (float) ((ADC_val * 3.3) / ADC_RESOLUTION);

  Distance_in_mm = (uint16_t) (((DISTANCE_SENSOR_MAX_DISTANCE
      - DISTANCE_SENSOR_MIN_DISTANCE)
      / (DISTANCE_SENSOR_MAX_AMP - DISTANCE_SENSOR_MIN_DISTANCE))
      * (ADC_voltage / RESISTOR) - DISTANCE_SENSOR_MIN_AMP)
      + DISTANCE_SENSOR_MIN_DISTANCE;

  printf("Distance in mm:%u\r\n", Distance_in_mm);
}

static void Distance_Sensor_Select_ADC(void) {
  ADC_ChannelConfTypeDef sConfig = { 0 };
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }
}

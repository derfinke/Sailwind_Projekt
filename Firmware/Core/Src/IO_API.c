/*
 * IO_API.c
 *
 *  Created on: Apr 17, 2023
 *      Author: Bene
 */
#include "IO_API.h"


/* function definitions -----------------------------------------------*/
void printAnalogValue(analogSensor as)
{
	printf("%s: %hn %s\r\n", as.name, &as.currentValue, as.unit);
}

void readAnalogValue(analogSensor *sensor)
{
	//(source: https://controllerstech.com/stm32-adc-multi-channel-without-dma/)
	ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = sensor->channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(sensor->hadc, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_ADC_Start(sensor->hadc);
	HAL_ADC_PollForConversion(sensor->hadc, 1000);
	sensor->adc_value = HAL_ADC_GetValue(sensor->hadc);
	convertFromADC(sensor);
	HAL_ADC_Stop(sensor->hadc);
}

void writeAnalogValue(analogActuator *actuator, uint16_t value)
{
	actuator->currentValue = value;
	convertToDAC(actuator);
	HAL_DAC_SetValue(actuator->hdac, actuator->channel, DAC_ALIGN_12B_R, actuator->adc_value);
}

void convertFromADC(analogSensor *sensor)
{
	sensor->currentValue = sensor->adc_value * sensor->signalConversion / ANALOG_MAX;
}

void convertToDAC(analogActuator *actuator)
{
	actuator->adc_value = (uint32_t) actuator->currentValue * ANALOG_MAX / actuator->signalConversion;
}

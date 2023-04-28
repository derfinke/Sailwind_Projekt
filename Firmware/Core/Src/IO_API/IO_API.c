/*
 * IO_API.c
 *
 *  Created on: Apr 17, 2023
 *      Author: Bene
 */
#include "IO_API.h"


/* API function definitions -----------------------------------------------*/

void IO_writeDigitalOUT(digitalPin *digital_OUT, GPIO_PinState state)
{
	digital_OUT->state = state;
	HAL_GPIO_WritePin(digital_OUT->GPIOx, digital_OUT->GPIO_Pin, digital_OUT->state);
}

void IO_toggleDigitalOUT(digitalPin *digital_OUT)
{
	digital_OUT->state ^= GPIO_PIN_SET;
	HAL_GPIO_TogglePin(digital_OUT->GPIOx, digital_OUT->GPIO_Pin);
}

GPIO_PinState IO_readDigitalIN(digitalPin *digital_IN)
{
	digital_IN->state = HAL_GPIO_ReadPin(digital_IN->GPIOx, digital_IN->GPIO_Pin);
	return digital_IN->state;
}

void IO_printAnalogValue(analogSensor as)
{
	printf("%s: %hn %s\r\n", as.name, &as.currentValue, as.unit);
}

void IO_readAnalogValue(analogSensor *sensor)
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

void IO_writeAnalogValue(analogActuator *actuator, uint16_t value)
{
	actuator->currentValue = value;
	convertToDAC(actuator);
	HAL_DAC_SetValue(actuator->hdac, actuator->channel, DAC_ALIGN_12B_R, actuator->adc_value);
}


/* private function definitions -----------------------------------------------*/

void convertFromADC(analogSensor *sensor)
{
	sensor->currentValue = sensor->adc_value / ANALOG_MAX * sensor->maxValue;
}

void convertToDAC(analogActuator *actuator)
{
	actuator->adc_value = (uint32_t) actuator->currentValue / actuator->maxValue * ANALOG_MAX ;
}

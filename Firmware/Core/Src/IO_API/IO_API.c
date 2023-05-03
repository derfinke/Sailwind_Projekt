/*
 * IO_API.c
 *
 *  Created on: Apr 17, 2023
 *      Author: Bene
 */
#include "IO_API.h"

/* private function prototypes -----------------------------------------------*/
static void _IO_convertFromADC(analogSensor_t *sensor);
static void _IO_convertToDAC(analogActuator_t *actuator);

/* API function definitions -----------------------------------------------*/

void IO_writeDigitalOUT(digitalPin_t *digital_OUT, GPIO_PinState state)
{
	digital_OUT->state = state;
	HAL_GPIO_WritePin(digital_OUT->GPIOx, digital_OUT->GPIO_Pin, digital_OUT->state);
}

void IO_toggleDigitalOUT(digitalPin_t *digital_OUT)
{
	digital_OUT->state ^= GPIO_PIN_SET;
	HAL_GPIO_TogglePin(digital_OUT->GPIOx, digital_OUT->GPIO_Pin);
}

GPIO_PinState IO_readDigitalIN(digitalPin_t *digital_IN)
{
	digital_IN->state = HAL_GPIO_ReadPin(digital_IN->GPIOx, digital_IN->GPIO_Pin);
	return digital_IN->state;
}

void IO_printAnalogValue(analogSensor_t as)
{
	printf("%s: %hn %s\r\n", as.name, &as.currentValue, as.unit);
}

void IO_readAnalogValue(analogSensor_t *sensor)
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
	_IO_convertFromADC(sensor);
	HAL_ADC_Stop(sensor->hadc);
}

void IO_writeAnalogValue(analogActuator_t *actuator, uint16_t value)
{
	actuator->currentValue = value;
	_IO_convertToDAC(actuator);
	HAL_DAC_SetValue(actuator->hdac, actuator->channel, DAC_ALIGN_12B_R, actuator->adc_value);
}


/* private function definitions -----------------------------------------------*/

static void _IO_convertFromADC(analogSensor_t *sensor)
{
	sensor->currentValue = sensor->adc_value / ANALOG_MAX * sensor->maxValue;
}

static void _IO_convertToDAC(analogActuator_t *actuator)
{
	actuator->adc_value = (uint32_t) actuator->currentValue / actuator->maxValue * ANALOG_MAX ;
}

/*
 * IO_API.c
 *
 *  Created on: Apr 17, 2023
 *      Author: Bene
 */
#include "IO_API.h"

/* private function prototypes -----------------------------------------------*/
static void _IO_convertFromADC(IO_analogSensor_t *sensor_ptr);
static void _IO_convertToDAC(IO_analogActuator_t *actuator_ptr);

/* API function definitions -----------------------------------------------*/

void IO_writeDigitalOUT(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state)
{
	digital_OUT_ptr->state = state;
	HAL_GPIO_WritePin(digital_OUT_ptr->GPIOx, digital_OUT_ptr->GPIO_Pin, digital_OUT_ptr->state);
}

void IO_toggleDigitalOUT(IO_digitalPin_t *digital_OUT_ptr)
{
	digital_OUT_ptr->state ^= GPIO_PIN_SET;
	HAL_GPIO_TogglePin(digital_OUT_ptr->GPIOx, digital_OUT_ptr->GPIO_Pin);
}

GPIO_PinState IO_readDigitalIN(IO_digitalPin_t *digital_IN_ptr)
{
	digital_IN_ptr->state = HAL_GPIO_ReadPin(digital_IN_ptr->GPIOx, digital_IN_ptr->GPIO_Pin);
	return digital_IN_ptr->state;
}

void IO_printAnalogValue(IO_analogSensor_t as)
{
	printf("%s: %.2f %s\r\n", as.name, as.currentValue, as.unit);
}

void IO_readAnalogValue(IO_analogSensor_t *sensor_ptr)
{
	//(source: https://controllerstech.com/stm32-adc-multi-channel-without-dma/)
	ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = sensor_ptr->channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(sensor_ptr->hadc, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_ADC_Start(sensor_ptr->hadc);
	HAL_ADC_PollForConversion(sensor_ptr->hadc, 1000);
	sensor_ptr->adc_value = HAL_ADC_GetValue(sensor_ptr->hadc);
	_IO_convertFromADC(sensor_ptr);
	HAL_ADC_Stop(sensor_ptr->hadc);
}

void IO_writeAnalogValue(IO_analogActuator_t *actuator_ptr, float value)
{
	actuator_ptr->currentValue = value;
	_IO_convertToDAC(actuator_ptr);
	HAL_DAC_SetValue(actuator_ptr->hdac, actuator_ptr->channel, DAC_ALIGN_12B_R, actuator_ptr->adc_value);
}


/* private function definitions -----------------------------------------------*/

static void _IO_convertFromADC(IO_analogSensor_t *sensor_ptr)
{
	sensor_ptr->currentValue = (float)(sensor_ptr->adc_value) / (float)(ANALOG_MAX) * sensor_ptr->maxValue;
}

static void _IO_convertToDAC(IO_analogActuator_t *actuator_ptr)
{
	actuator_ptr->adc_value = (uint16_t) (actuator_ptr->currentValue / actuator_ptr->maxValue * ANALOG_MAX) ;
}

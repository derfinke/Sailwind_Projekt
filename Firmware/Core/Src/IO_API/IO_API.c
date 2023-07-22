/*
 * IO_API.c
 *
 *  Created on: Apr 17, 2023
 *      Author: Bene
 */
#include "IO_API.h"

/* private function prototypes -----------------------------------------------*/
static void convertFromADC(IO_analogSensor_t *sensor_ptr);
static void convertToDAC(IO_analogActuator_t *actuator_ptr);

/* API function definitions -----------------------------------------------*/

/* IO_digitalPin_t IO_digitalPin_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState state)
 *  Description:
 *   - return an IO_digitalPin Instance with members passed as parameters
 *   - write the initial state to the GPIO Pin
 */
IO_digitalPin_t IO_digitalPin_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState state)
{
	IO_digitalPin_t digitalPin = {
			.GPIOx = GPIOx,
			.GPIO_Pin = GPIO_Pin,
			.state = state
	};
	IO_digitalWrite(&digitalPin, state);
	return digitalPin;
}

/* void IO_digitalWrite(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state)
 * 	Description:
 * 	 - save the new state (from parameter "state") in the digitalPin structure reference
 * 	 - write the state to the HAL_GPIO_Pin
 */
void IO_digitalWrite(IO_digitalPin_t *digital_OUT_ptr, GPIO_PinState state)
{
	digital_OUT_ptr->state = state;
	HAL_GPIO_WritePin(digital_OUT_ptr->GPIOx, digital_OUT_ptr->GPIO_Pin, digital_OUT_ptr->state);
}

/* void IO_digitalToggle(IO_digitalPin_t *digital_OUT_ptr)
 *  Description:
 *   - toggle the state of the digitalPin reference
 *   - write the new state to the HAL_GPIO_Pin
 */
void IO_digitalToggle(IO_digitalPin_t *digital_OUT_ptr)
{
	digital_OUT_ptr->state ^= GPIO_PIN_SET;
	HAL_GPIO_WritePin(digital_OUT_ptr->GPIOx, digital_OUT_ptr->GPIO_Pin, digital_OUT_ptr->state);
}

/* GPIO_PinState IO_digitalRead(IO_digitalPin_t *digital_IN_ptr)
 *  Description:
 *   - read current state of HAL_GPIO_Pin and save it to the digitalPin reference
 *   - return the updated state
 */
GPIO_PinState IO_digitalRead(IO_digitalPin_t *digital_IN_ptr)
{
	digital_IN_ptr->state = HAL_GPIO_ReadPin(digital_IN_ptr->GPIOx, digital_IN_ptr->GPIO_Pin);
	return digital_IN_ptr->state;
}

/* boolean_t IO_digitalRead_state_changed(IO_digitalPin_t *digital_IN_ptr)
 *  Description:
 *   - read current state of the digitalPin reference and compare with previous state
 *   - return True, if the state has changed
 */
boolean_t IO_digitalRead_state_changed(IO_digitalPin_t *digital_IN_ptr)
{
	GPIO_PinState previous_state = digital_IN_ptr->state;
	GPIO_PinState current_state = IO_digitalRead(digital_IN_ptr);
	return current_state ^ previous_state;
}

/* boolean_t IO_digitalRead_rising_edge(IO_digitalPin_t *digital_IN_ptr)
 *  Description:
 *   - return True, if the state of the digitalPin reference has changed from 0 to 1
 */
boolean_t IO_digitalRead_rising_edge(IO_digitalPin_t *digital_IN_ptr)
{
	GPIO_PinState previous_state = digital_IN_ptr->state;
	GPIO_PinState current_state = IO_digitalRead(digital_IN_ptr);
	return previous_state == GPIO_PIN_RESET && current_state == GPIO_PIN_SET;
}

/* void IO_analogPrint(IO_analogSensor_t as)
 *  Description:
 *   - print the current Value in its converted measurement form with the sensor name and its measurement unit
 *   - e.g.: "distance: 100.00 mm"
 */
void IO_analogPrint(IO_analogSensor_t as)
{
	printf("%s: %.2f %s\r\n", as.name, as.currentConvertedValue, as.unit);
}

/* void IO_analogRead(IO_analogSensor_t *sensor_ptr)
 *  Description:
 *   - read analog value of adc channel specified in analogSensor reference
 *   - configure adc to sample the respective channel for 15 cycles to get the 12 bit resolution (0...4096)
 *   - use polling method with 1000 ms time out
 *   - save adc value to the Sensor reference
 *   - convert adc value to the respective measured variable
 *   - stop adc polling
 */
void IO_analogRead(IO_analogSensor_t *sensor_ptr)
{
	//(source: https://controllerstech.com/stm32-adc-multi-channel-without-dma/)
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = sensor_ptr->hadc_channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	if (HAL_ADC_ConfigChannel(sensor_ptr->hadc_ptr, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_ADC_Start(sensor_ptr->hadc_ptr);
	HAL_ADC_PollForConversion(sensor_ptr->hadc_ptr, 1000);
	sensor_ptr->adc_value = HAL_ADC_GetValue(sensor_ptr->hadc_ptr);
	convertFromADC(sensor_ptr);
	HAL_ADC_Stop(sensor_ptr->hadc_ptr);
}

/* void IO_analogWrite(IO_analogActuator_t *actuator_ptr, float value)
 *  Description:
 *   - save analog value from parameter (limited to given "limitConvertedValue") to the analogActuator reference
 *   - convert analog to digital value and write it to the dac channel specified in the actuator reference
 */
void IO_analogWrite(IO_analogActuator_t *actuator_ptr, float value)
{
	actuator_ptr->currentConvertedValue = value <= actuator_ptr->limitConvertedValue? value: actuator_ptr->limitConvertedValue;
	convertToDAC(actuator_ptr);
	HAL_DAC_SetValue(actuator_ptr->hdac_ptr, actuator_ptr->hdac_channel, DAC_ALIGN_12B_R, actuator_ptr->dac_value);
}


/* private function definitions -----------------------------------------------*/

/* static void convertFromADC(IO_analogSensor_t *sensor_ptr)
 *  Description:
 *   - convert digital value to the analog Value by using the ratio of the analog and digital max value
 *   - save the calculated value to the sensor reference
 */
static void convertFromADC(IO_analogSensor_t *sensor_ptr)
{
	sensor_ptr->currentConvertedValue = sensor_ptr->adc_value / (float)(ANALOG_MAX) * sensor_ptr->maxConvertedValue;
}

/* static void convertToDAC(IO_analogActuator_t *actuator_ptr)
 *  Description:
 *   - convert the analog value to the corresponding digital value and save it to the actuator reference
 */
static void convertToDAC(IO_analogActuator_t *actuator_ptr)
{
	actuator_ptr->dac_value = (uint16_t) (actuator_ptr->currentConvertedValue / actuator_ptr->maxConvertedValue * ANALOG_MAX) ;
}

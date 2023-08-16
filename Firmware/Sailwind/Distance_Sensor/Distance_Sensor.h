/**
 * @file Distance_Sensor.h
 * @author finkbeiner
 * @date 18 Jun 2023
 * @brief readout distance sensor
 */

#ifndef DISTANCE_SENSOR_DISTANCE_SENSOR_H_
#define DISTANCE_SENSOR_DISTANCE_SENSOR_H_

#include <stdint.h>

/**
* @brief Get the measured distance of the distance sensor in mm
* @param Measured distance in mm
* @retval None
* @note Distance is calculated through the equation:
*
* (DISTANCE_SENSOR_MAX_DISTANCE - DISTANCE_SENSOR_MIN_DISTANCE)   (  ADC_Voltage                            ) \n
* ------------------------------------------------------------- x ( ------------ _  DISTANCE_SENSOR_MIN_AMP ) + DISTANCE_SENSOR_MIN_DISTANCE \n
*     (DISTANCE_SENSOR_MAX_AMP - DISTANCE_SENSOR_MIN_AMP)         (   RESISTOR                              ) \n
*/
void Distance_Sensor_Get_Distance(uint16_t Distance_in_mm);

#endif /* DISTANCE_SENSOR_DISTANCE_SENSOR_H_ */

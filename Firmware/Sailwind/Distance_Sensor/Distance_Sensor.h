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
 */
uint16_t Distance_Sensor_Get_Distance(void);

#endif /* DISTANCE_SENSOR_DISTANCE_SENSOR_H_ */

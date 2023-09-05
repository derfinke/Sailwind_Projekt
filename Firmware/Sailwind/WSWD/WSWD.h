/*
 * WSWD.h
 *
 *  Created on: Aug 26, 2023
 *      Author: nicof
 */

#ifndef WSWD_WSWD_H_
#define WSWD_WSWD_H_

uint8_t WSWD_send_without_param(char* command);
uint8_t WSWD_send_with_param(char* command, char* param);
uint8_t WSWD_receive(char* receive_buffer, uint8_t size_of_receive_buffer);
void WSWD_get_wind_infos(char* received_NMEA_telegramm, float Windspeed,  float Winddirection);
void WSWD_get_windspeed_unit(char* received_NMEA_telegramm, char unit);

#endif /* WSWD_WSWD_H_ */

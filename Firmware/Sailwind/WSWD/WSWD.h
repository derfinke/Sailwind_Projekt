/**
 * \file WSWD.h
 * @date 18 Jun 2023
 * @brief Access to Analog and Digital IO Pins
 */

#ifndef WSWD_WSWD_H_
#define WSWD_WSWD_H_

/**
 * @brief send a command code over rs485
 * @param command:ptr to a string containing the command
 * @retval succes or fail
 */
uint8_t WSWD_send_without_param(char* command);

/**
 * @brief send a command code with an additional parameter over rs485
 * @param command:ptr to a string containing the command
 * @param param:ptr to a string containing the additional parameter
 * @retval succes or fail
 */
uint8_t WSWD_send_with_param(char* command, char* param);

/**
 * @brief receive an anwser over rs485
 * @param receive_buffer:ptr to a string buffer for the received message
 * @param size_of_receive_buffer:size of the receive buffer
 * @retval succes or fail
 */
uint8_t WSWD_receive(char* receive_buffer, uint8_t size_of_receive_buffer);

/**
 * @brief extract windspeed and direction from a received NMEA telegram
 * @param received_NMEA_telegramm:ptr to the received NMEA telegram
 * @param Windspeed:Windspeed extracted from telegram
 * @param Winddirection:Winddirection extracted from telegram
 * @retval none
 */
void WSWD_get_wind_infos(char* received_NMEA_telegramm, float Windspeed,  float Winddirection);

/**
 * @brief extract the windspeed unit from a received NMEA telegram
 * @param received_NMEA_telegramm:ptr to the received NMEA telegram
 * @param unit:unit extracted from telegram
 * @retval none
 */
void WSWD_get_windspeed_unit(char* received_NMEA_telegramm, char unit);

#endif /* WSWD_WSWD_H_ */

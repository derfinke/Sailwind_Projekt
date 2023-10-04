/*
 * Manual_Control.h
 *
 *  Created on: 27.07.2023
 *      Author: Bene
 */

#ifndef MANUAL_CONTROL_MANUAL_CONTROL_H_
#define MANUAL_CONTROL_MANUAL_CONTROL_H_

#include "Button.h"
#include "Linear_Guide.h"


/* typedefs ------------------------------------------------------------------*/

typedef struct {
	Button_t switch_mode;
	Button_t move_backwards;
	Button_t move_forward;
	Button_t localize;
} MC_buttons_t;

typedef struct {
	MC_buttons_t buttons;
	Linear_Guide_t *lg_ptr;
	uint16_t longpress_time_s;
	uint16_t longpress_time_s_max;
	TIM_HandleTypeDef *htim_loc_reset_ptr;
	TIM_HandleTypeDef *htim_ip_reset_ptr;
} Manual_Control_t;



/* API function prototypes ---------------------------------------------------*/
Manual_Control_t Manual_Control_init(Linear_Guide_t *lg_ptr, TIM_HandleTypeDef *htim_loc_reset_ptr, TIM_HandleTypeDef *htim_ip_reset_ptr);
void Manual_Control_poll(Manual_Control_t *mc_ptr);
int8_t Manual_Control_Localization(Manual_Control_t *mc_ptr);
void Manual_Control_long_press_callback(Manual_Control_t *mc_ptr, TIM_HandleTypeDef *htim_ptr);

#endif /* MANUAL_CONTROL_MANUAL_CONTROL_H_ */

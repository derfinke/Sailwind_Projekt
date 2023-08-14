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

/* defines -------------------------------------------------------------------*/
#define MANUAL_CONTROL_COUNT 4
#define MANUAL_CONTROL_BUTTON_SWITCH_MANUAL GPIO_PIN_RESET	//ToDo: check correctness
#define MANUAL_CONTROL_BUTTON_SWITCH_AUTOMATIC GPIO_PIN_SET	//ToDo: check correctness

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
} Manual_Control_t;



/* API function prototypes ---------------------------------------------------*/
Manual_Control_t Manual_Control_init(Linear_Guide_t *lg_ptr);
void Manual_Control_poll(Manual_Control_t *mc_ptr);
void Manual_Control_Localization(Manual_Control_t *mc_ptr);

#endif /* MANUAL_CONTROL_MANUAL_CONTROL_H_ */

/*
 * Manual_Control.h
 *
 *  Created on: 27.07.2023
 *      Author: Bene
 */

#ifndef MANUAL_CONTROL_MANUAL_CONTROL_H_
#define MANUAL_CONTROL_MANUAL_CONTROL_H_

#include "Button/Button.h"
#include "../Linear_Guide/Linear_Guide.h"

/* defines -------------------------------------------------------------------*/
#define BUTTON_COUNT 4
#define BUTTON_SWITCH_MANUAL GPIO_PIN_RESET	//ToDo: check correctness
#define BUTTON_SWITCH_AUTOMATIC GPIO_PIN_SET	//ToDo: check correctness

/* typedefs ------------------------------------------------------------------*/
typedef enum {button_ID_switch_mode, button_ID_move_backward, button_ID_move_forward, button_ID_calibrate} button_ID_t;



/* API function prototypes ---------------------------------------------------*/
Button_t* MC_Button_bar_init();
void MC_button_eventHandler(Button_t buttons[BUTTON_COUNT], Linear_Guide_t *lg_ptr);
void MC_Localization_state_machine_loop_based(Linear_Guide_t *lg_ptr);

#endif /* MANUAL_CONTROL_MANUAL_CONTROL_H_ */

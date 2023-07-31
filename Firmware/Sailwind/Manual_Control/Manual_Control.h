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
#define MANUAL_CONTROL_COUNT 4
#define MANUAL_CONTROL_BUTTON_SWITCH_MANUAL GPIO_PIN_RESET	//ToDo: check correctness
#define MANUAL_CONTROL_BUTTON_SWITCH_AUTOMATIC GPIO_PIN_SET	//ToDo: check correctness

/* typedefs ------------------------------------------------------------------*/
typedef enum {
	Manual_Control_ID_switch_mode,
	Manual_Control_ID_move_backwards,
	Manual_Control_ID_move_forward,
	Manual_Control_ID_localize
} Manual_Control_ID_t;

typedef void (*MC_function_t)(Button_state_t button_state, Linear_Guide_t *lg_ptr);

typedef struct {
	Button_t button;
	MC_function_t function;
} Manual_Control_t;



/* API function prototypes ---------------------------------------------------*/
Manual_Control_t *Manual_Controls_init();
void Manual_Control_poll(Manual_Control_t *manual_controls, Linear_Guide_t *lg_ptr);
void Manual_Control_Localization(Linear_Guide_t *lg_ptr);

#endif /* MANUAL_CONTROL_MANUAL_CONTROL_H_ */

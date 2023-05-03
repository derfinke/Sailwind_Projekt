/*
 * button_API.h
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_BUTTON_API_H_
#define SRC_IO_API_BUTTON_API_H_

#include "motor_API.h"

/* typedefs ------------------------------------------------------------------*/
typedef GPIO_PinState button_state_t;
typedef enum {btn_switch_mode, btn_move_left, btn_move_right, btn_calibrate} button_ID_t;
typedef struct Button_t Button_t;
struct Button_t {
	button_ID_t ID;
	digitalPin_t pin;
	void (*eventHandler)(Button_t button, Motor_t *motor);
};


/* defines -------------------------------------------------------------------*/
#define BUTTON_COUNT 4
#define btn_pressed GPIO_PIN_RESET
#define btn_released GPIO_PIN_SET
#define btn_switch_manual GPIO_PIN_RESET	//nicht sicher
#define btn_switch_automatic GPIO_PIN_SET	//nicht sicher

/* API function prototypes ---------------------------------------------------*/
Button_t* button_init_array();
void button_eventHandler(Button_t buttons[4], Motor_t *motor);


#endif /* SRC_IO_API_BUTTON_API_H_ */

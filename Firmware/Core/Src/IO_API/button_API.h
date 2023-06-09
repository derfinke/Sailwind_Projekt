/*
 * button_API.h
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#ifndef SRC_IO_API_BUTTON_API_H_
#define SRC_IO_API_BUTTON_API_H_

#include "motor_API.h"
#include "LED_API.h"

/* typedefs ------------------------------------------------------------------*/
typedef enum {button_ID_switch_mode, button_ID_move_left, button_ID_move_right, button_ID_calibrate} button_ID_t;
typedef struct Button_t Button_t;
struct Button_t {
	IO_digitalPin_t pin;
	void (*eventHandler)(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr);
};


/* defines -------------------------------------------------------------------*/
#define BUTTON_COUNT 4
#define BUTTON_PRESSED GPIO_PIN_RESET
#define BUTTON_RELEASED GPIO_PIN_SET
#define BUTTON_SWITCH_MANUAL GPIO_PIN_RESET	//ToDo: check correctness
#define BUTTON_SWITCH_AUTOMATIC GPIO_PIN_SET	//ToDo: check correctness

/* API function prototypes ---------------------------------------------------*/
Button_t* button_init_array();
void button_eventHandler(Button_t buttons[4], Motor_t *motor_ptr, LED_bar_t *led_bar_ptr);


#endif /* SRC_IO_API_BUTTON_API_H_ */

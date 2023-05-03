/*
 * button_API.c
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#include "button_API.h"

/* private function prototypes -----------------------------------------------*/
static boolean_t _button_state_changed(Button_t *button);
static void _button_event_move_toggle(Button_t button, Motor_t *motor);
static void _button_event_switch_operating_mode(Button_t button, Motor_t *motor);
static void _button_event_calibrate(Button_t button, Motor_t *motor);

/* API function definitions -----------------------------------------------*/
Button_t* button_init_array()
{
	static Button_t buttons[BUTTON_COUNT];

	Button_t button_switch_mode = {
			.ID = btn_switch_mode,
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_4_Reserviert_PE15_Pin,
					.state = btn_switch_manual
			},
			.eventHandler = _button_event_switch_operating_mode
	};
	buttons[btn_switch_mode] = button_switch_mode;

	Button_t button_move_left = {
			.ID = btn_move_left,
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_1_Links_PE10_Pin,
					.state = btn_released
			},
			.eventHandler = _button_event_move_toggle
	};
	buttons[btn_move_left] = button_move_left;

	Button_t button_move_right = {
			.ID = btn_move_right,
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_3_Kalibrierung_PE14_Pin,
					.state = btn_released
			},
			.eventHandler = _button_event_move_toggle
	};
	buttons[btn_move_right] = button_move_right;

	Button_t button_calibrate = {
			.ID = btn_calibrate,
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_2_Rechts_PE12_Pin,
					.state = btn_released
			},
			.eventHandler = _button_event_calibrate
	};
	buttons[btn_calibrate] = button_calibrate;

	return buttons;
}

void button_eventHandler(Button_t buttons[4], Motor_t *motor)
{
	for (button_ID_t btn_idx = 0; btn_idx < BUTTON_COUNT; btn_idx++)
	{
		if (_button_state_changed(&buttons[btn_idx]))
		{
			buttons[btn_idx].eventHandler(buttons[btn_idx], motor);
		}
	}
}

/* private function definitions -----------------------------------------------*/
static boolean_t _button_state_changed(Button_t *button)
{
	return (boolean_t) button->pin.state ^ IO_readDigitalIN(&button->pin); // last state XOR current state -> True if changed
}

static void _button_event_move_toggle(Button_t button, Motor_t *motor)
{
	motor_function_t motor_function_direction;
	switch (button.pin.state)
	{
		case btn_pressed:
			if (button.ID == btn_move_left)
			{
				motor_function_direction = motor_function_linkslauf;
			}
			else
			{
				motor_function_direction = motor_function_rechtslauf;
			}
			motor_start_moving(motor, motor_function_direction);
			break;
		case btn_released:
			motor_stop_moving(motor);
			break;
	}
}

static void _button_event_switch_operating_mode(Button_t button, Motor_t *motor)
{
	motor_operating_mode_t operating_mode;
	switch (button.pin.state)
	{
		case btn_switch_automatic:
			operating_mode = motor_operating_mode_automatic;
			break;
		case btn_switch_manual:
			operating_mode = motor_operating_mode_manual;
			break;
	}
	motor_set_operating_mode(motor, operating_mode);
}

static void _button_event_calibrate(Button_t button, Motor_t *motor)
{
	switch (button.pin.state)
	{
		case btn_pressed:
			motor_calibrate(motor);
			break;
		case btn_released:
			break;
	}
}


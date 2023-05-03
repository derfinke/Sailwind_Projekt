/*
 * button_API.c
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#include "button_API.h"

/* private function prototypes -----------------------------------------------*/
static boolean_t _button_state_changed(Button_t *button_ptr);
static void _button_move_toggle(Button_t button, Motor_t *motor_ptr, motor_function_t motor_function_direction);

static void _button_event_move_left_toggle(Button_t button, Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr);
static void _button_event_move_right_toggle(Button_t button, Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr);
static void _button_event_switch_operating_mode(Button_t button, Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr);
static void _button_event_calibrate(Button_t button, Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr);

/* API function definitions -----------------------------------------------*/
Button_t* button_init_array()
{
	static Button_t buttons[BUTTON_COUNT];

	Button_t button_switch_mode = {
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_4_Reserviert_PE15_Pin,
					.state = BUTTON_SWITCH_MANUAL
			},
			.eventHandler = _button_event_switch_operating_mode
	};
	buttons[btn_switch_mode] = button_switch_mode;

	Button_t button_move_left = {
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_1_Links_PE10_Pin,
					.state = BUTTON_RELEASED
			},
			.eventHandler = _button_event_move_left_toggle
	};
	buttons[btn_move_left] = button_move_left;

	Button_t button_move_right = {
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_3_Kalibrierung_PE14_Pin,
					.state = BUTTON_RELEASED
			},
			.eventHandler = _button_event_move_right_toggle
	};
	buttons[btn_move_right] = button_move_right;

	Button_t button_calibrate = {
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_2_Rechts_PE12_Pin,
					.state = BUTTON_RELEASED
			},
			.eventHandler = _button_event_calibrate
	};
	buttons[btn_calibrate] = button_calibrate;

	return buttons;
}

void button_eventHandler(Button_t buttons[BUTTON_COUNT], Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr)
{
	for (button_ID_t btn_idx = 0; btn_idx < BUTTON_COUNT; btn_idx++)
	{
		if (_button_state_changed(&buttons[btn_idx]))
		{
			buttons[btn_idx].eventHandler(buttons[btn_idx], motor_ptr, led_operating_mode_ptr);
		}
	}
}

/* private function definitions -----------------------------------------------*/
static boolean_t _button_state_changed(Button_t *button_ptr)
{
	return (boolean_t) button_ptr->pin.state ^ IO_readDigitalIN(&button_ptr->pin); // last state XOR current state -> True if changed
}

static void _button_move_toggle(Button_t button, Motor_t *motor_ptr, motor_function_t motor_function_direction)
{
	switch (button.pin.state)
	{
		case BUTTON_PRESSED:
			motor_start_moving(motor_ptr, motor_function_direction);
			break;
		case BUTTON_RELEASED:
			motor_stop_moving(motor_ptr);
			break;
	}
}

static void _button_event_move_left_toggle(Button_t button, Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr)
{
	_button_move_toggle(button, motor_ptr, motor_function_linkslauf);
}

static void _button_event_move_right_toggle(Button_t button, Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr)
{
	_button_move_toggle(button, motor_ptr, motor_function_rechtslauf);
}

static void _button_event_switch_operating_mode(Button_t button, Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr)
{
	motor_operating_mode_t operating_mode;
	switch (button.pin.state)
	{
		case BUTTON_SWITCH_AUTOMATIC:
			operating_mode = motor_operating_mode_automatic;
			break;
		case BUTTON_SWITCH_MANUAL:
			operating_mode = motor_operating_mode_manual;
			break;
	}
	motor_set_operating_mode(motor_ptr, operating_mode);
	LED_toggle_operating_mode(led_operating_mode_ptr);
}

static void _button_event_calibrate(Button_t button, Motor_t *motor_ptr, LED_status_operating_mode_t *led_operating_mode_ptr)
{
	switch (button.pin.state)
	{
		case BUTTON_PRESSED:
			motor_calibrate(motor_ptr);
			break;
		case BUTTON_RELEASED:
			break;
	}
}


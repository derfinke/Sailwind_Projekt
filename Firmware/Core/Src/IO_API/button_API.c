/*
 * button_API.c
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#include "button_API.h"

/* private function prototypes -----------------------------------------------*/
static boolean_t state_changed(Button_t *button_ptr);
static void move_toggle(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr, motor_moving_state_t direction);

static void event_move_left_toggle(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr);
static void event_move_right_toggle(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr);
static void event_switch_operating_mode(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr);
static void event_calibrate(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr);

/* API function definitions -----------------------------------------------*/
Button_t* button_init_array()
{
	static Button_t buttons[BUTTON_COUNT];

	Button_t button_switch_mode = {
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = BUTTON_SWITCH_AUTOMATIC,
					.state = BUTTON_SWITCH_MANUAL
			},
			.eventHandler = event_switch_operating_mode
	};
	buttons[button_ID_switch_mode] = button_switch_mode;

	Button_t button_move_left = {
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_Zurueck_Pin,
					.state = BUTTON_RELEASED
			},
			.eventHandler = event_move_left_toggle
	};
	buttons[button_ID_move_left] = button_move_left;

	Button_t button_move_right = {
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Kalibrierung_Pin,
					.state = BUTTON_RELEASED
			},
			.eventHandler = event_move_right_toggle
	};
	buttons[button_ID_move_right] = button_move_right;

	Button_t button_calibrate = {
			.pin = {
					.GPIOx = GPIOE,
					.GPIO_Pin = Button_Vorfahren_Pin,
					.state = BUTTON_RELEASED
			},
			.eventHandler = event_calibrate
	};
	buttons[button_ID_calibrate] = button_calibrate;

	return buttons;
}

void button_eventHandler(Button_t buttons[BUTTON_COUNT], Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	for (button_ID_t btn_idx = 0; btn_idx < BUTTON_COUNT; btn_idx++)
	{
		if (state_changed(&buttons[btn_idx]))
		{
			buttons[btn_idx].eventHandler(buttons[btn_idx], motor_ptr, led_bar_ptr);
		}
	}
}

/* private function definitions -----------------------------------------------*/
static boolean_t state_changed(Button_t *button_ptr)
{
	return IO_digitalRead_state_changed(&button_ptr->pin);
}

static void move_toggle(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr, motor_moving_state_t direction)
{
	if (motor_ptr->operating_mode == IO_operating_mode_manual && (motor_ptr->calibration.is_calibrated || motor_ptr->calibration.state == motor_calibration_state_0_init))
	{
		switch (button.pin.state)
		{
			case BUTTON_PRESSED:
				if (motor_ptr->calibration.is_calibrated)
				{
					LED_switch(&led_bar_ptr->calibration, LED_OFF);
				}
				motor_start_moving(motor_ptr, direction);
				break;
			case BUTTON_RELEASED:
				motor_stop_moving(motor_ptr);
				break;
		}
	}
}

static void event_move_left_toggle(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	move_toggle(button, motor_ptr, led_bar_ptr, motor_moving_state_linkslauf);
}

static void event_move_right_toggle(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	move_toggle(button, motor_ptr, led_bar_ptr, motor_moving_state_rechtslauf);
}

static void event_switch_operating_mode(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	IO_operating_mode_t operating_mode = motor_ptr->operating_mode;
	switch (button.pin.state)
	{
		case BUTTON_SWITCH_AUTOMATIC:
			if (motor_ptr->calibration.is_calibrated)
			{
				operating_mode = IO_operating_mode_automatic;
				if (led_bar_ptr->calibration.state == LED_OFF)
				{
					LED_switch(&led_bar_ptr->calibration, LED_ON);
				}
			}
			break;
		case BUTTON_SWITCH_MANUAL:
			operating_mode = IO_operating_mode_manual;
			break;
	}
	if (operating_mode != motor_ptr->operating_mode)
	{
		LED_set_operating_mode(&led_bar_ptr->operating_mode, operating_mode);
		motor_set_operating_mode(motor_ptr, operating_mode);
	}

}

static void event_calibrate(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	if (motor_ptr->operating_mode == IO_operating_mode_manual)
	{
		switch (button.pin.state)
		{
			case BUTTON_PRESSED:
				motor_button_calibrate_state_machine(motor_ptr, &led_bar_ptr->calibration);
				break;
			case BUTTON_RELEASED:
				break;
		}
	}
}


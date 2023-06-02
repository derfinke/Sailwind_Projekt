/*
 * button_API.c
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#include "button_API.h"

/* private function prototypes -----------------------------------------------*/
static boolean_t state_changed(Button_t *button_ptr);
static void move_toggle(Button_t button, Motor_t *motor_ptr, motor_function_t motor_function_direction);

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

static void move_toggle(Button_t button, Motor_t *motor_ptr, motor_function_t motor_function_direction)
{
	if (motor_ptr->operating_mode == motor_operating_mode_manual)
	{
		switch (button.pin.state)
		{
			case BUTTON_PRESSED:
				printf("\nmotor_start_moving(motor_ptr, motor_function_direction);\n");
				break;
			case BUTTON_RELEASED:
				printf("\nmotor_stop_moving(motor_ptr);\n");
				break;
		}
	}
}

static void event_move_left_toggle(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	move_toggle(button, motor_ptr, motor_function_linkslauf);
}

static void event_move_right_toggle(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	move_toggle(button, motor_ptr, motor_function_rechtslauf);
}

static void event_switch_operating_mode(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	//motor_operating_mode_t operating_mode;
	switch (button.pin.state)
	{
		case BUTTON_SWITCH_AUTOMATIC:
			printf("\nmotor_ptr->operating_mode = motor_operating_mode_automatic;\n");
			break;
		case BUTTON_SWITCH_MANUAL:
			printf("\nmotor_ptr->operating_mode = motor_operating_mode_manual;\n");
			break;
	}
	//motor_set_operating_mode(motor_ptr, operating_mode);
	LED_toggle_operating_mode(&led_bar_ptr->operating_mode);
}

static void event_calibrate(Button_t button, Motor_t *motor_ptr, LED_bar_t *led_bar_ptr)
{
	if (motor_ptr->operating_mode == motor_operating_mode_manual)
	{
		switch (button.pin.state)
		{
			case BUTTON_PRESSED:
				motor_button_calibrate_state_machine(motor_ptr, &led_bar_ptr->center_pos_set);
				break;
			case BUTTON_RELEASED:
				break;
		}
	}
}


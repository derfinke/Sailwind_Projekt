/*
 * button_API.c
 *
 *  Created on: 29.04.2023
 *      Author: Bene
 */

#include "button_API.h"

/* private function prototypes -----------------------------------------------*/
static boolean_t state_changed(Button_t *button_ptr);
static void move_toggle(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr, motor_moving_state_t direction);
static void calibrate_button_state_machine(Linear_guide_t *linear_guide_ptr, LED_t *led_center_pos_set_ptr);

static void event_move_left_toggle(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr);
static void event_move_right_toggle(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr);
static void event_switch_operating_mode(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr);
static void event_calibrate(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr);

/* API function definitions -----------------------------------------------*/

/* Button_t* button_init_array()
 * 	Description:
 * 	 - initializes an array of 4 Buttons (switch operating mode, move motor left, ...right, calibrate motor)
 *   - each button contains an IO_digitalPin and an event handler for the event "state changed"
 */
Button_t* button_init_array()
{
	static Button_t buttons[BUTTON_COUNT];

	Button_t button_switch_mode = {
			.pin = IO_digitalPin_init(GPIOF, Switch_Betriebsmodus_Pin, BUTTON_SWITCH_MANUAL),
			.eventHandler = event_switch_operating_mode
	};
	buttons[button_ID_switch_mode] = button_switch_mode;

	Button_t button_move_left = {
			.pin = IO_digitalPin_init(GPIOB, Button_Zurueck_Pin, BUTTON_RELEASED),
			.eventHandler = event_move_left_toggle
	};
	buttons[button_ID_move_left] = button_move_left;

	Button_t button_move_right = {
			.pin = IO_digitalPin_init(GPIOB, Button_Vorfahren_Pin, BUTTON_RELEASED),
			.eventHandler = event_move_right_toggle
	};
	buttons[button_ID_move_right] = button_move_right;

	Button_t button_calibrate = {
			.pin = IO_digitalPin_init(GPIOC, Kalibrierung_Pin, BUTTON_RELEASED),
			.eventHandler = event_calibrate
	};
	buttons[button_ID_calibrate] = button_calibrate;

	return buttons;
}

/* void button_eventHandler(Button_t buttons[BUTTON_COUNT], Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr)
 * 	Description:
 * 	 - to be called in main loop
 * 	 - iterates through the button array and checks for each button, if its state has changed
 * 	 - if so, its eventHandler is called and the button itself together with the motor and the led_bar are passed as standard parameter
 * 	 - the specific eventHandler functions are defined below and must have the same parameters to be called in a loop
 */
void button_eventHandler(Button_t buttons[BUTTON_COUNT], Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr)
{
	for (button_ID_t btn_idx = 0; btn_idx < BUTTON_COUNT; btn_idx++)
	{
		if (state_changed(&buttons[btn_idx]))
		{
			buttons[btn_idx].eventHandler(buttons[btn_idx].pin.state, linear_guide_ptr, led_bar_ptr);
		}
	}
}

/* private function definitions -----------------------------------------------*/

/* static boolean_t state_changed(Button_t *button_ptr)
 * 	Description:
 *   - returns True, if the state of the digitalPin of the given button has changed
 */
static boolean_t state_changed(Button_t *button_ptr)
{
	return IO_digitalRead_state_changed(&button_ptr->pin);
}

/* static void move_toggle(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr, motor_moving_state_t direction)
 *  Description:
 *   - called within the two eventHandler to move either left or right depending on the parameter "direction"
 *   - manual moving is only possible, if operating mode is manual and if the calibration process is not running currently (so it cannot be interrupted)
 *   - if the button is pressed, a function is called to start the motor in the given direction
 *   - if it is released, the motor is commanded to stop
 */
static void move_toggle(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr, motor_moving_state_t direction)
{
	Motor_t *motor_ptr = &linear_guide_ptr->motor;

	if (!linear_guide_get_manual_moving_permission(*linear_guide_ptr))
	{
		return;
	}
	switch (button_state)
	{
		case BUTTON_PRESSED:
			if (linear_guide_ptr->calibration.is_calibrated)
			{
				LED_switch(&led_bar_ptr->center_pos_set, LED_OFF); // switch calibration LED off, so it can be switched on, after setting a new center position
			}
			motor_start_moving(motor_ptr, direction);
			break;
		case BUTTON_RELEASED:
			motor_stop_moving(motor_ptr);
			break;
	}
}

/* static void calibrate_button_state_machine(Linear_guide_t *linear_guide_ptr, LED_t *led_center_pos_set_ptr)
 *  Description:
 *   - called, when the calibration button is pressed
 *   - first press sets state to state_1 which starts the calibration process in the "approach borders state machine"
 *   - state_2 is set from the other state machine when the calculated center position is reached
 *   - pressing the button at state_2, the center position is saved, the is_calibrated flag is set True and the LED for the center position is switched on
 */
static void calibrate_button_state_machine(Linear_guide_t *linear_guide_ptr, LED_t *led_center_pos_set_ptr)
{
	switch(linear_guide_ptr->calibration.calibrate_button_state)
	{
		case linear_guide_calibrate_button_state_0_init:
			linear_guide_ptr->calibration.calibrate_button_state = linear_guide_calibrate_button_state_1_approach_borders;
			break;
		case linear_guide_calibrate_button_state_1_approach_borders:
			//wait for endpoints_set
			break;
		case linear_guide_calibrate_button_state_2_set_center_pos:
			linear_guide_set_center(&linear_guide_ptr->calibration);
			LED_switch(led_center_pos_set_ptr, LED_ON);
			linear_guide_ptr->calibration.is_calibrated = True;
			break;
	}
}

static void event_move_left_toggle(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr)
{
	move_toggle(button_state, linear_guide_ptr, led_bar_ptr, motor_moving_state_linkslauf);
}

static void event_move_right_toggle(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr)
{
	move_toggle(button_state, linear_guide_ptr, led_bar_ptr, motor_moving_state_rechtslauf);
}

/* static void event_switch_operating_mode(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr)
 *  Description:
 *   - eventHandler to switch operating mode
 *   - switching the button to automatic only works, if the calibration process is done
 *   - if so, besides the calibration LED is turned on again, if it was switched off due to moving the motor manually before
 *   - on the other hand, in automatic mode the operating mode can always be switched to manual
 *   - if the operating mode could be changed, the corresponding LEDs are set and reset
 *     and also the operating mode state of the motor object is set
 */
static void event_switch_operating_mode(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr)
{
	IO_operating_mode_t operating_mode = linear_guide_ptr->operating_mode;
	switch (button_state)
	{
		case BUTTON_SWITCH_AUTOMATIC:
			if (!linear_guide_ptr->calibration.is_calibrated)
			{
				return;
			}
			operating_mode = IO_operating_mode_automatic;
			if (led_bar_ptr->center_pos_set.state == LED_OFF)
			{
				LED_switch(&led_bar_ptr->center_pos_set, LED_ON);
			}
			break;
		case BUTTON_SWITCH_MANUAL:
			operating_mode = IO_operating_mode_manual;
			break;
	}
	if (operating_mode != linear_guide_ptr->operating_mode)
	{
		LED_set_operating_mode(led_bar_ptr, operating_mode);
		linear_guide_set_operating_mode(linear_guide_ptr, operating_mode);
	}

}

/* static void event_calibrate(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr)
 *  Description:
 *   - eventHandler for the calibration button
 *   - only works in manual mode
 *   - if the button is pressed, the calibration state machine is called
 *   - 1. press starts the automatic calibration process
 *   - 2. press can be made after the motor stopped at the calculated center to confirm and save the position of the linear guide
 *   - if necessary, the position can be adjusted manually with the moving buttons before pressing the button
 *   - further presses can be made afterwards to adjust the center position again
 */
static void event_calibrate(GPIO_PinState button_state, Linear_guide_t *linear_guide_ptr, LED_bar_t *led_bar_ptr)
{
	if (linear_guide_ptr->operating_mode != IO_operating_mode_manual)
	{
		return;
	}
	switch (button_state)
	{
		case BUTTON_PRESSED:
			calibrate_button_state_machine(linear_guide_ptr, &led_bar_ptr->center_pos_set);
			break;
		case BUTTON_RELEASED:
			break;
	}
}


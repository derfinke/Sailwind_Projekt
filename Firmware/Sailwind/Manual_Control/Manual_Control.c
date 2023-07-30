/*
 * Manual_Control.c
 *
 *  Created on: 27.07.2023
 *      Author: Bene
 */

#include "Manual_Control.h"

/* private function prototypes -----------------------------------------------*/
static void MC_move_toggle(GPIO_PinState button_state, Linear_Guide_t *lg_ptr, Loc_movement_t movement);
static void MC_Localization_state_machine_event_based(Linear_Guide_t *lg_ptr);

static void MC_event_move_backwards_toggle(GPIO_PinState button_state, Linear_Guide_t *linear_guide_ptr);
static void MC_event_move_forward_toggle(GPIO_PinState button_state, Linear_Guide_t *linear_guide_ptr);
static void MC_event_switch_operating_mode(GPIO_PinState button_state, Linear_Guide_t *lg_ptr);
static void MC_event_localization(GPIO_PinState button_state, Linear_Guide_t *lg_ptr);

/* API function definitions -----------------------------------------------*/

Button_t *MC_Button_bar_init()
{
	static Button_t buttons[BUTTON_COUNT];
	buttons[button_ID_switch_mode] = Button_init(GPIOF, Switch_Betriebsmodus_Pin);
	buttons[button_ID_move_backward] = Button_init(GPIOB, Button_Zurueck_Pin);
	buttons[button_ID_move_forward] = Button_init(GPIOB, Button_Vorfahren_Pin);
	buttons[button_ID_calibrate] = Button_init(GPIOC, Kalibrierung_Pin);
	return buttons;
}

/* void MC_button_eventHandler(Button_t buttons[BUTTON_COUNT], Linear_guide_t *lg_ptr)
 * 	Description:
 * 	 - to be called in main loop
 * 	 - iterates through the button array and checks for each button, if its state has changed
 * 	 - if so, its eventHandler is called and the button itself together with the motor and the led_bar are passed as standard parameter
 * 	 - the specific eventHandler functions are defined below and must have the same parameters to be called in a loop
 */
void MC_button_eventHandler(Button_t buttons[BUTTON_COUNT], Linear_Guide_t *lg_ptr)
{
	void (*event[BUTTON_COUNT])(Button_state_t button_state, Linear_Guide_t *lg_ptr);
	event[button_ID_switch_mode] = MC_event_switch_operating_mode;
	event[button_ID_move_backward] = MC_event_move_backwards_toggle;
	event[button_ID_move_forward] = MC_event_move_forward_toggle;
	event[button_ID_calibrate] = MC_event_localization;

	for (button_ID_t btn_idx = 0; btn_idx < BUTTON_COUNT; btn_idx++)
	{
		if (Button_state_changed(&buttons[btn_idx]))
		{
			printf("Event of Button %d\r\n", btn_idx);
			event[btn_idx](buttons[btn_idx].state, lg_ptr);
		}
	}
}

/* void MC_Localization_state_machine_loop_based(Linear_guide_t *linear_guide_ptr)
 *  Description:
 *   - state machine to be called in main loop
 *   - controls the process of approaching both end points and finally the calculated center
 *   - state machine only runs, after first press of calibration button
 *   - from initial state 0 the motor starts moving to the first end point and state is set to 1 (approach front)
 *   - state 1 is active until the front end switch is detected (high)
 *   		-> set state to 2 (approach back)
 *   		-> start motor moving to other end point
 *   		-> start rpm measurement to calculate linear guide range
 *   - state 2 is active until the back end switch is detected (high)
 *   		-> set state to 3 (approach center)
 *   		-> calculate range of linear guide
 *   		-> start motor moving to the calculated center
 *   - state 3 waits until center is reached
 *   		-> motor is stopped
 *   		-> state is set to state 4 (set center)
 */
void MC_Localization_state_machine_loop_based(Linear_Guide_t *lg_ptr)
{
	Loc_state_t *state = &lg_ptr->localization.state;
	switch(*state)
	{
		case Loc_state_1_approach_front:
			if (LG_Endswitch_detected(lg_ptr, LG_Endswitch_front))
			{
				printf("new state approach back\r\n");
				LG_move(lg_ptr, Loc_movement_backwards);
				*state = Loc_state_2_approach_back;
			}
			break;
		case Loc_state_2_approach_back:
			if (LG_Endswitch_detected(lg_ptr, LG_Endswitch_back))
			{
				printf("new state approach center\r\n");
				LG_move(lg_ptr, Loc_movement_forward);
				LG_set_endpos(lg_ptr);
				*state = Loc_state_3_approach_center;
			}
			break;
		case Loc_state_3_approach_center:
			if (lg_ptr->localization.current_pos_mm == 0)
			{
				printf("new state set center\r\n");
				LG_move(lg_ptr, Loc_movement_stop);
				*state = Loc_state_4_set_center_pos;
			}
			break;
		default:
			break;
	}
}

/* private function definitions -----------------------------------------------*/

/* static void MC_Localization_state_machine_event_based(Linear_guide_t *linear_guide_ptr)
 *  Description:
 *   - called, when the calibration button is pressed
 *   - first press sets state to state_1 which starts the calibration process in the "approach borders state machine"
 *   - state_4 is set from the other state machine when the calculated center position is reached
 *   - pressing the button at state_4, the center position is saved, the is_calibrated flag is set True and the LED for the center position is switched on
 */
static void MC_Localization_state_machine_event_based(Linear_Guide_t *lg_ptr)
{
	switch(lg_ptr->localization.state)
	{
		case Loc_state_0_init:
			printf("localization init\r\n");
			LG_move(lg_ptr, Loc_movement_forward);
			lg_ptr->localization.state = Loc_state_1_approach_front;
			printf("new state approach vorne\r\n");
			break;
		case Loc_state_4_set_center_pos:
			printf("center set!\r\n");
			LG_set_center(lg_ptr);
			break;
		default:
			break;
	}
}

/* static void MC_move_toggle(Button_state_t button_state, Linear_guide_t *lg_ptr, motor_moving_state_t direction)
 *  Description:
 *   - called within the two event to move either left or right depending on the parameter "direction"
 *   - manual moving is only possible, if operating mode is manual and if the calibration process is not running currently (so it cannot be interrupted)
 *   - if the button is pressed, a function is called to start the motor in the given direction
 *   - if it is released, the motor is commanded to stop
 */
static void MC_move_toggle(Button_state_t button_state, Linear_Guide_t *lg_ptr, Loc_movement_t movement)
{
	if (!LG_get_manual_moving_permission(*lg_ptr))
	{
		printf("no moving permission\r\n");
		return;
	}
	switch (button_state)
	{
		case BUTTON_PRESSED:
			LG_move(lg_ptr, movement);
			break;
		case BUTTON_RELEASED:
			LG_move(lg_ptr, Loc_movement_stop);
			break;
	}
}

static void MC_event_move_backwards_toggle(Button_state_t button_state, Linear_Guide_t *linear_guide_ptr)
{
	MC_move_toggle(button_state, linear_guide_ptr, Loc_movement_backwards);
}

static void MC_event_move_forward_toggle(Button_state_t button_state, Linear_Guide_t *linear_guide_ptr)
{
	MC_move_toggle(button_state, linear_guide_ptr, Loc_movement_forward);
}

/* static void MC_event_switch_operating_mode(Button_state_t button_state, Linear_guide_t *lg_ptr)
 *  Description:
 *   - event to switch operating mode
 *   - switching the button to automatic only works, if the calibration process is done
 *   - if so, besides the center pos LED is turned on again, if it was switched off due to moving the motor manually before
 *   - on the other hand, in automatic mode the operating mode can always be switched to manual
 *   - if the operating mode could be changed, the corresponding LEDs are set and reset
 */
static void MC_event_switch_operating_mode(Button_state_t button_state, Linear_Guide_t *lg_ptr)
{
	LG_operating_mode_t new_operating_mode = lg_ptr->operating_mode;
	switch (button_state)
	{
		case BUTTON_SWITCH_AUTOMATIC:
			if (!lg_ptr->localization.is_localized)
			{
				printf("not calibrated yet\r\n");
				return;
			}
			printf("set to automatic\r\n");
			new_operating_mode = LG_operating_mode_automatic;
			break;
		case BUTTON_SWITCH_MANUAL:
			printf("set to manual\r\n");
			new_operating_mode = LG_operating_mode_manual;
			break;
	}
	if (new_operating_mode != lg_ptr->operating_mode)
	{
		printf("operating mode changed\r\n");
		LG_set_operating_mode(lg_ptr, new_operating_mode);
	}

}

/* static void MC_event_localization(GPIO_PinState button_state, Linear_guide_t *lg_ptr)
 *  Description:
 *   - event for the localization button
 *   - only works in manual mode
 *   - if the button is pressed, the localization state machine is called
 *   - 1. press starts the automatic localization process
 *   - 2. press can be made after the motor stopped at the calculated center to confirm and save the position of the linear guide
 *   - if necessary, the position can be adjusted manually with the moving buttons before pressing the button
 *   - further presses can be made afterwards to adjust the center position again
 */
static void MC_event_localization(GPIO_PinState button_state, Linear_Guide_t *lg_ptr)
{
	if (lg_ptr->operating_mode != LG_operating_mode_manual)
	{
		return;
	}
	switch (button_state)
	{
		case BUTTON_PRESSED:
			MC_Localization_state_machine_event_based(lg_ptr);
			break;
		case BUTTON_RELEASED:
			break;
	}
}


/*
 * Manual_Control.c
 *
 *  Created on: 27.07.2023
 *      Author: Bene
 */

#include "Manual_Control.h"

/* private function prototypes -----------------------------------------------*/
static void Manual_Control_move_toggle(Manual_Control_t *mc_ptr, Button_t btn, Loc_movement_t movement);

static void Manual_Control_function_move_backwards_toggle(Manual_Control_t *mc_ptr);
static void Manual_Control_function_move_forward_toggle(Manual_Control_t *mc_ptr);
static void Manual_Control_function_switch_operating_mode(Manual_Control_t *mc_ptr);
static void Manual_Control_function_localization(Manual_Control_t *mc_ptr);

static boolean_t Manual_Control_get_moving_permission(Manual_Control_t mc);
static void Manual_Control_set_center(Manual_Control_t *mc_ptr);
static void Manual_Control_set_endpos(Manual_Control_t *mc_ptr);

/* API function definitions -----------------------------------------------*/

Manual_Control_t Manual_Control_init(Linear_Guide_t *lg_ptr)
{
	MC_buttons_t buttons = {
			.switch_mode = Button_init(Switch_Betriebsmodus_GPIO_Port, Switch_Betriebsmodus_Pin),
			.move_backwards = Button_init(Button_Zurueck_GPIO_Port, Button_Zurueck_Pin),
			.move_forward = Button_init(Button_Vorfahren_GPIO_Port, Button_Vorfahren_Pin),
			.localize = Button_init(Kalibrierung_GPIO_Port, Kalibrierung_Pin),
	};

	Manual_Control_t manual_control = {
			.buttons = buttons,
			.lg_ptr = lg_ptr
	};

	return manual_control;
}

/* void Manual_Control_poll(Manual_Control_t *mc_ptr)
 * 	Description:
 * 	 - to be called in main loop
 * 	 - iterates through the button array and checks for each button, if its state has changed
 * 	 - if so, its eventHandler is called passing its state and the linear guide object
 * 	 - the specific eventHandler functions are defined below and must have the same parameters to be called in a loop
 */
void Manual_Control_poll(Manual_Control_t *mc_ptr)
{
	MC_buttons_t *btns = &mc_ptr->buttons;
	if (Button_state_changed(&btns->switch_mode))
	{
		Manual_Control_function_switch_operating_mode(mc_ptr);
	}
	if (Button_state_changed(&btns->move_backwards))
	{
		Manual_Control_function_move_backwards_toggle(mc_ptr);
	}
	if (Button_state_changed(&btns->move_forward))
	{
		Manual_Control_function_move_forward_toggle(mc_ptr);
	}
	if (Button_state_changed(&btns->localize))
	{
		Manual_Control_function_localization(mc_ptr);
	}
}

/* void Manual_Control_Localization(Manual_Control_t *mc_ptr)
 *  Description:
 *   - state machine to be called in main loop
 *   - controls the process of approaching both end points and finally the calculated center
 *
 *   - state 0 waits until first press of localization button
 *   		-> the motor starts moving to the first end point and state is set to 1 (approach front)
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
 *   - state 4 waits for second button press to confirm or update the center pos
 *   		-> Localization process finished: ready for automatic mode
 */
void Manual_Control_Localization(Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	Loc_state_t *state = &lg_ptr->localization.state;
	switch(*state)
	{
		case Loc_state_0_init:
			if (lg_ptr->localization.is_triggered)
			{
				Linear_Guide_move(lg_ptr, Loc_movement_forward);
				lg_ptr->localization.state = Loc_state_1_approach_front;
				printf("new state approach front\r\n");
				lg_ptr->localization.is_triggered = False;
			}
			break;
		case Loc_state_1_approach_front:
			if (Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.front))
			{
				printf("new state approach back\r\n");
				Linear_Guide_move(lg_ptr, Loc_movement_backwards);
				*state = Loc_state_2_approach_back;
			}
			break;
		case Loc_state_2_approach_back:
			if (Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.back))
			{
				printf("new state approach center\r\n");
				Linear_Guide_move(lg_ptr, Loc_movement_forward);
				Manual_Control_set_endpos(mc_ptr);
				*state = Loc_state_3_approach_center;
			}
			break;
		case Loc_state_3_approach_center:
			if (lg_ptr->localization.current_pos_mm == 0)
			{
				printf("new state set center\r\n");
				Linear_Guide_move(lg_ptr, Loc_movement_stop);
				*state = Loc_state_4_set_center_pos;
			}
			break;
		case Loc_state_4_set_center_pos:
			if (lg_ptr->localization.is_triggered)
			{
				printf("center set at: %ld mm!\r\n", lg_ptr->localization.current_pos_mm);
				Manual_Control_set_center(mc_ptr);
				lg_ptr->localization.is_triggered = False;
			}
			break;
		default:
			break;
	}
}

/* private function definitions -----------------------------------------------*/

/* static void Manual_Control_move_toggle(Manual_Control_t *mc_ptr, Button_t btn, Loc_movement_t movement)
 *  Description:
 *   - called within the two event to move either left or right depending on the parameter "direction"
 *   - manual moving is only possible, if operating mode is manual and if the calibration process is not running currently (so it cannot be interrupted)
 *   - if the button is pressed, a function is called to start the motor in the given direction
 *   - if it is released, the motor is commanded to stop
 */
static void Manual_Control_move_toggle(Manual_Control_t *mc_ptr, Button_t btn, Loc_movement_t movement)
{
	if (Manual_Control_get_moving_permission(*mc_ptr))
	{
		printf("no moving permission\r\n");
		return;
	}
	switch (btn.state)
	{
		case BUTTON_PRESSED:
			Linear_Guide_move(mc_ptr->lg_ptr, movement);
			break;
		case BUTTON_RELEASED:
			Linear_Guide_move(mc_ptr->lg_ptr, Loc_movement_stop);
			break;
	}
}

static void Manual_Control_function_move_backwards_toggle(Manual_Control_t *mc_ptr)
{
	Manual_Control_move_toggle(mc_ptr, mc_ptr->buttons.move_backwards, Loc_movement_backwards);
}

static void Manual_Control_function_move_forward_toggle(Manual_Control_t *mc_ptr)
{
	Manual_Control_move_toggle(mc_ptr, mc_ptr->buttons.move_forward, Loc_movement_forward);
}

/* static void MC_event_switch_operating_mode(Manual_Control_t *mc_ptr)
 *  Description:
 *   - event to switch operating mode
 *   - switching the button to automatic only works, if the calibration process is done
 *   - if so, besides the center pos LED is turned on again, if it was switched off due to moving the motor manually before
 *   - on the other hand, in automatic mode the operating mode can always be switched to manual
 *   - if the operating mode could be changed, the corresponding LEDs are set and reset
 */
static void Manual_Control_function_switch_operating_mode(Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	LG_operating_mode_t new_operating_mode = lg_ptr->operating_mode;
	switch (mc_ptr->buttons.switch_mode.state)
	{
		case MANUAL_CONTROL_BUTTON_SWITCH_AUTOMATIC:
			if (!lg_ptr->localization.is_localized)
			{
				printf("not calibrated yet\r\n");
				return;
			}
			printf("set to automatic\r\n");
			new_operating_mode = LG_operating_mode_automatic;
			break;
		case MANUAL_CONTROL_BUTTON_SWITCH_MANUAL:
			printf("set to manual\r\n");
			new_operating_mode = LG_operating_mode_manual;
			break;
	}
	if (new_operating_mode != lg_ptr->operating_mode)
	{
		printf("operating mode changed\r\n");
		Linear_Guide_set_operating_mode(lg_ptr, new_operating_mode);
	}

}

/* static void MC_event_localization(Manual_Control_t *mc_ptr)
 *  Description:
 *   - event for the localization button
 *   - only works in manual mode
 *   - if the button is pressed, the localization state machine is triggered in state 0 or state 4 by setting the is_triggered flag
 *   - 1. press starts the automatic localization process
 *   - 2. press can be made after the motor stopped at the calculated center to confirm and save the position of the linear guide
 *   - if necessary, the position can be adjusted manually with the moving buttons before pressing the button
 *   - further presses can be made afterwards to adjust the center position again
 */
static void Manual_Control_function_localization(Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	if (lg_ptr->operating_mode != LG_operating_mode_manual)
	{
		return;
	}
	switch (mc_ptr->buttons.localize.state)
	{
		case BUTTON_PRESSED:
			LED_switch(&lg_ptr->leds.center_pos_set, LED_OFF);
			break;
		case BUTTON_RELEASED:
			lg_ptr->localization.is_triggered = True;
			break;
	}
}

/* static boolean_t Manual_Control_get_moving_permission(Manual_Control_t mc)
 *  Description:
 *   - return True, if operating mode is manual and motor is not currently moving automatically due to localization process
 */
static boolean_t Manual_Control_get_moving_permission(Manual_Control_t mc)
{
	Linear_Guide_t lg = *mc.lg_ptr;
	return
			lg.operating_mode == LG_operating_mode_manual
			&&
			(
				lg.localization.state == Loc_state_4_set_center_pos
				||
				lg.localization.state == Loc_state_0_init
			);
}

static void Manual_Control_set_center(Manual_Control_t *mc_ptr)
{
	Localization_set_center(&mc_ptr->lg_ptr->localization);
	LED_switch(&mc_ptr->lg_ptr->leds.center_pos_set, LED_ON);
}

static void Manual_Control_set_endpos(Manual_Control_t *mc_ptr)
{
	Localization_set_endpos(&mc_ptr->lg_ptr->localization);
}


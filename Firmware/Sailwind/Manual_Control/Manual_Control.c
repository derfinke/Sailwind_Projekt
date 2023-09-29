/*
 * Manual_Control.c
 *
 *  Created on: 27.07.2023
 *      Author: Bene
 */

#include "Manual_Control.h"


/* defines -------------------------------------------------------------------*/
#define MC_COUNT 4
#define MC_BUTTON_SWITCH_MANUAL GPIO_PIN_RESET	//ToDo: check correctness
#define MC_BUTTON_SWITCH_AUTOMATIC GPIO_PIN_SET	//ToDo: check correctness
#define MC_OPERATING_MODE_SWITCH_DENIED 1
#define MC_OPERATING_MODE_SWITCH_OK 0
#define MC_LOCALIZATION_DENIED 1
#define MC_LOCALIZATION_OK 0
#define MC_MOVE_DENIED 1
#define MC_MOVE_OK 0
#define MC_SET_CENTER_OK 0
#define MC_SET_CENTER_NOT_TRIGGERED 1
#define MC_LONG_PRESS_TIME_S 3

static IO_analogSensor_t *MC_distance_sensor_ptr = {0};

/* private function prototypes -----------------------------------------------*/
static int8_t Manual_Control_move_toggle(Manual_Control_t *mc_ptr, Button_t btn, Loc_movement_t movement);

static int8_t Manual_Control_function_move_backwards_toggle(Manual_Control_t *mc_ptr);
static int8_t Manual_Control_function_move_forward_toggle(Manual_Control_t *mc_ptr);
static int8_t Manual_Control_function_localization(Manual_Control_t *mc_ptr);
static int8_t Manual_Control_function_switch_operating_mode(Manual_Control_t *mc_ptr);

static int8_t Manual_Control_set_center(Manual_Control_t *mc_ptr);
static void Manual_Control_set_endpos(Manual_Control_t *mc_ptr);
static LG_operating_mode_t Manual_Control_get_operating_mode_button_state(GPIO_PinState btn_state);
/**
 * @brief measure and set minimum absolute distance value from sensor
 * @param mc_ptr: manual_control reference
 * @retval none
 */
static void Manual_Control_set_startpos(Manual_Control_t *mc_ptr);


/* API function definitions -----------------------------------------------*/

Manual_Control_t Manual_Control_init(Linear_Guide_t *lg_ptr, TIM_HandleTypeDef *htim_ptr)
{
	MC_buttons_t buttons = {
			.switch_mode = Button_init(Switch_Betriebsmodus_GPIO_Port, Switch_Betriebsmodus_Pin),
			.move_backwards = Button_init(Button_Backwards_GPIO_Port, Button_Backwards_Pin),
			.move_forward = Button_init(Button_Forward_GPIO_Port, Button_Forward_Pin),
			.localize = Button_init(Kalibrierung_GPIO_Port, Kalibrierung_Pin),
	};

	Manual_Control_t manual_control = {
			.buttons = buttons,
			.lg_ptr = lg_ptr,
			.longpress_time_s = 0,
			.htim_ptr = htim_ptr
	};
	lg_ptr->operating_mode = Manual_Control_get_operating_mode_button_state(buttons.switch_mode.state);
	lg_ptr->leds = Linear_Guide_LEDs_init(lg_ptr->operating_mode);
	MC_distance_sensor_ptr = IO_get_distance_sensor();
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
int8_t Manual_Control_Localization(Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	if(lg_ptr->operating_mode == LG_operating_mode_automatic)
	{
		return MC_LOCALIZATION_DENIED;
	}
	Localization_t *loc_ptr = &lg_ptr->localization;
	Loc_state_t *state = &loc_ptr->state;
	switch(*state)
	{
		case Loc_state_0_init:
			if (loc_ptr->is_triggered)
			{
				Linear_Guide_move(lg_ptr, Loc_movement_forward, False);
				*state = Loc_state_1_approach_front;
				printf("new state approach front\r\n");
				loc_ptr->is_triggered = False;
			}
			break;
		case Loc_state_1_approach_front:
			if (Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.front))
			{
				printf("new state approach back\r\n");
				Manual_Control_set_startpos(mc_ptr);
				Linear_Guide_move(lg_ptr, Loc_movement_stop, True);
				if (loc_ptr->recovery_state == LOC_RECOVERY_PARTIAL)
				{
					*state = Loc_state_5_center_pos_set;
					loc_ptr->is_localized = True;
					Localization_update_position(loc_ptr);
					loc_ptr->desired_pos_mm = loc_ptr->current_pos_mm;
				}
				else
				{
					*state = Loc_state_2_approach_back;
					HAL_Delay(1000);
					Linear_Guide_move(lg_ptr, Loc_movement_backwards, False);
				}

			}
			break;
		case Loc_state_2_approach_back:
			if (Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.back))
			{
				Linear_Guide_move(lg_ptr, Loc_movement_stop, True);
				HAL_Delay(1000);
				Manual_Control_set_endpos(mc_ptr);
				printf("pulses:%d\r\n", loc_ptr->pulse_count);
				printf("endpos:%d\r\n", loc_ptr->end_pos_mm);
				*state = Loc_state_3_approach_center;
				loc_ptr->desired_pos_mm = 0;
				printf("new state approach center\r\n");
			}
			break;
		case Loc_state_3_approach_center:
			if(loc_ptr->current_pos_mm == 0)
			{
				printf("new state set center\r\n");
				*state = Loc_state_4_set_center_pos;
			}
			break;
		case Loc_state_4_set_center_pos:
			if (Manual_Control_set_center(mc_ptr) == MC_SET_CENTER_OK)
			{
				*state = Loc_state_5_center_pos_set;
			}
			break;
		case Loc_state_5_center_pos_set:
			Manual_Control_set_center(mc_ptr);
			break;
		default:
			break;
	}
	return MC_LOCALIZATION_OK;
}

void Manual_Control_long_press_callback(Manual_Control_t *mc_ptr)
{
	mc_ptr->longpress_time_s++;
	if (mc_ptr->longpress_time_s >= MC_LONG_PRESS_TIME_S)
	{
		Localization_reset(&mc_ptr->lg_ptr->localization, True);
		HAL_TIM_Base_Stop_IT(mc_ptr->htim_ptr);
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
static int8_t Manual_Control_move_toggle(Manual_Control_t *mc_ptr, Button_t btn, Loc_movement_t movement)
{
	if (!Linear_Guide_get_moving_permission(*mc_ptr->lg_ptr))
	{
		printf("no moving permission\r\n");
		return MC_MOVE_DENIED;
	}
	switch (btn.state)
	{
		case BUTTON_PRESSED:
			Linear_Guide_manual_move(mc_ptr->lg_ptr, movement);
			break;
		case BUTTON_RELEASED:
			Linear_Guide_manual_move(mc_ptr->lg_ptr, Loc_movement_stop);
			break;
	}
	return MC_MOVE_OK;
}

static int8_t Manual_Control_function_move_backwards_toggle(Manual_Control_t *mc_ptr)
{
	return Manual_Control_move_toggle(mc_ptr, mc_ptr->buttons.move_backwards, Loc_movement_backwards);
}

static int8_t Manual_Control_function_move_forward_toggle(Manual_Control_t *mc_ptr)
{
	return Manual_Control_move_toggle(mc_ptr, mc_ptr->buttons.move_forward, Loc_movement_forward);
}

/* static void MC_event_switch_operating_mode(Manual_Control_t *mc_ptr)
 *  Description:
 *   - event to switch operating mode
 *   - switching the button to automatic only works, if the localization process is done
 *   - on the other hand, in automatic mode the operating mode can always be switched to manual
 *   - if the operating mode could be changed, the corresponding LEDs are set and reset
 */
static int8_t Manual_Control_function_switch_operating_mode(Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	LG_operating_mode_t new_operating_mode = lg_ptr->operating_mode;
	switch (mc_ptr->buttons.switch_mode.state)
	{
		case MC_BUTTON_SWITCH_AUTOMATIC:
			if (!lg_ptr->localization.is_localized)
			{
				printf("not calibrated yet\r\n");
				return MC_OPERATING_MODE_SWITCH_DENIED;
			}
			printf("set to automatic\r\n");
			new_operating_mode = LG_operating_mode_automatic;
			Linear_Guide_safe_Localization(lg_ptr->localization);
			break;
		case MC_BUTTON_SWITCH_MANUAL:
			printf("set to manual\r\n");
			new_operating_mode = LG_operating_mode_manual;
			break;
	}
	if (new_operating_mode != lg_ptr->operating_mode)
	{
		printf("operating mode changed\r\n");
		Linear_Guide_set_operating_mode(lg_ptr, new_operating_mode);
	}
	return MC_OPERATING_MODE_SWITCH_OK;
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
static int8_t Manual_Control_function_localization(Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	if (lg_ptr->operating_mode != LG_operating_mode_manual)
	{
		return MC_LOCALIZATION_DENIED;
	}
	switch (mc_ptr->buttons.localize.state)
	{
		case BUTTON_PRESSED:
			HAL_TIM_Base_Start_IT(mc_ptr->htim_ptr);
			break;
		case BUTTON_RELEASED:
			if (mc_ptr->longpress_time_s < MC_LONG_PRESS_TIME_S)
			{
				lg_ptr->localization.is_triggered = True;
			}
			mc_ptr->longpress_time_s = 0;
			HAL_TIM_Base_Stop_IT(mc_ptr->htim_ptr);
			break;
	}
	return MC_LOCALIZATION_OK;
}

static int8_t Manual_Control_set_center(Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	if (!lg_ptr->localization.is_triggered)
	{
		return MC_SET_CENTER_NOT_TRIGGERED;
	}
	printf("pulses:%d\r\n", lg_ptr->localization.pulse_count);
	printf("center set at: %d mm!\r\n", lg_ptr->localization.current_pos_mm);
	Localization_set_center(&mc_ptr->lg_ptr->localization);
	for(uint8_t i = 0; i < 5; i++)
	{
		LED_switch(&mc_ptr->lg_ptr->leds.center_pos_set, LED_ON);
		HAL_Delay(200);
		LED_switch(&mc_ptr->lg_ptr->leds.center_pos_set, LED_OFF);
		HAL_Delay(200);
	}
	lg_ptr->localization.is_triggered = False;
	return MC_SET_CENTER_OK;
}

static void Manual_Control_set_endpos(Manual_Control_t *mc_ptr)
{
	Localization_set_endpos(&mc_ptr->lg_ptr->localization);
}

static void Manual_Control_set_startpos(Manual_Control_t *mc_ptr)
{
	IO_Get_Measured_Value(MC_distance_sensor_ptr);
	Localization_set_startpos_abs(&mc_ptr->lg_ptr->localization, MC_distance_sensor_ptr->measured_value);
}

static LG_operating_mode_t Manual_Control_get_operating_mode_button_state(GPIO_PinState btn_state)
{
	return btn_state == MC_BUTTON_SWITCH_AUTOMATIC ? LG_operating_mode_automatic : LG_operating_mode_manual;
}

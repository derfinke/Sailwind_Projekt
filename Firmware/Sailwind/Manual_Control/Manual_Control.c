/*
 * Manual_Control.c
 *
 *  Created on: 27.07.2023
 *      Author: Bene
 */

#include "Manual_Control.h"
#include "boolean.h"
#include "FRAM.h"
#include "FRAM_memory_mapping.h"


/* defines -------------------------------------------------------------------*/
#define MC_COUNT 4
#define MC_LOCALIZATION_DENIED 1
#define MC_LOCALIZATION_OK 0
#define MC_MOVE_DENIED 1
#define MC_MOVE_OK 0
#define MC_LONG_PRESS_TIME_S_LOC 3
#define MC_LONG_PRESS_TIME_S_IP 10


/* private function prototypes -----------------------------------------------*/
static int8_t Manual_Control_move_toggle(Manual_Control_t *mc_ptr, Button_t btn, Loc_movement_t movement);

static int8_t Manual_Control_function_move_backwards_toggle(Manual_Control_t *mc_ptr);
static int8_t Manual_Control_function_move_forward_toggle(Manual_Control_t *mc_ptr);
static int8_t Manual_Control_function_localization(Manual_Control_t *mc_ptr);
static int8_t Manual_Control_function_switch_operating_mode(Manual_Control_t *mc_ptr);


/* API function definitions -----------------------------------------------*/

Manual_Control_t Manual_Control_init(Linear_Guide_t *lg_ptr, TIM_HandleTypeDef *htim_reset_ptr)
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
			.htim_reset_ptr = htim_reset_ptr,
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
			if (!loc_ptr->is_triggered)
			{
				break;
			}
			Linear_Guide_move(lg_ptr, Loc_movement_forward, False);
			*state = Loc_state_1_approach_front;
			printf("new state approach front\r\n");
			loc_ptr->is_triggered = False;
			break;
		case Loc_state_1_approach_front:
			if (!Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.front))
			{
				break;
			}
			printf("new state approach back\r\n");
			Linear_Guide_set_startpos(lg_ptr);
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
			break;
		case Loc_state_2_approach_back:
			if (!Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.back))
			{
				break;
			}
			Linear_Guide_move(lg_ptr, Loc_movement_stop, True);
			HAL_Delay(1000);
			Localization_set_endpos(loc_ptr);
			printf("pulses:%d\r\n", loc_ptr->pulse_count);
			printf("endpos:%d\r\n", loc_ptr->end_pos_mm);
			*state = Loc_state_3_approach_center;
			loc_ptr->desired_pos_mm = 0;
			printf("new state approach center\r\n");
			break;
		case Loc_state_3_approach_center:
			if(loc_ptr->current_pos_mm != 0)
			{
				break;
			}
			printf("new state set center\r\n");
			*state = Loc_state_4_set_center_pos;
			break;
		case Loc_state_4_set_center_pos:
		case Loc_state_5_center_pos_set:
			Linear_Guide_set_center(lg_ptr);
			break;
		default:
			break;
	}
	return MC_LOCALIZATION_OK;
}

void Manual_Control_long_press_callback(Manual_Control_t *mc_ptr)
{
	mc_ptr->longpress_time_s++;
	if (mc_ptr->longpress_time_s < mc_ptr->longpress_time_s_max)
	{
		return;
	}
	if (mc_ptr->longpress_time_s_max == MC_LONG_PRESS_TIME_S_LOC)
	{
		Localization_reset(&mc_ptr->lg_ptr->localization, True);
	}
	else if (mc_ptr->longpress_time_s_max == MC_LONG_PRESS_TIME_S_IP)
	{
		boolean_t set_default = True;
		FRAM_write((uint8_t *) &set_default, FRAM_IP_SET_DEFAULT_FLAG, sizeof(set_default));
		LED_blink(&mc_ptr->lg_ptr->leds.power, LED_ON, mc_ptr->lg_ptr->leds.htim_blink_ptr);
	}
	HAL_TIM_Base_Stop_IT(mc_ptr->htim_reset_ptr);

}

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
			if (mc_ptr->buttons.move_backwards.state == BUTTON_PRESSED && mc_ptr->buttons.move_forward.state == BUTTON_PRESSED)
			{
				if (mc_ptr->longpress_time_s == 0)
				{
					mc_ptr->longpress_time_s_max = MC_LONG_PRESS_TIME_S_IP;
					HAL_TIM_Base_Start_IT(mc_ptr->htim_reset_ptr);
					Linear_Guide_manual_move(mc_ptr->lg_ptr, Loc_movement_stop);
				}
			}
			else
			{
				Linear_Guide_manual_move(mc_ptr->lg_ptr, movement);
			}
			break;
		case BUTTON_RELEASED:
			if (mc_ptr->longpress_time_s < MC_LONG_PRESS_TIME_S_IP)
			{
				Linear_Guide_manual_move(mc_ptr->lg_ptr, Loc_movement_stop);
			}
			if (mc_ptr->longpress_time_s_max == MC_LONG_PRESS_TIME_S_IP)
			{
				mc_ptr->longpress_time_s = 0;
				HAL_TIM_Base_Stop_IT(mc_ptr->htim_reset_ptr);
			}
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

static int8_t Manual_Control_function_switch_operating_mode(Manual_Control_t *mc_ptr)
{
	Linear_Guide_t *lg_ptr = mc_ptr->lg_ptr;
	return Linear_Guide_set_operating_mode(lg_ptr, !lg_ptr->operating_mode);
}

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
			if (mc_ptr->longpress_time_s == 0)
			{
				mc_ptr->longpress_time_s_max = MC_LONG_PRESS_TIME_S_LOC;
				HAL_TIM_Base_Start_IT(mc_ptr->htim_reset_ptr);
			}

			break;
		case BUTTON_RELEASED:
			if (mc_ptr->longpress_time_s < MC_LONG_PRESS_TIME_S_LOC)
			{
				lg_ptr->localization.is_triggered = True;
			}
			if (mc_ptr->longpress_time_s_max == MC_LONG_PRESS_TIME_S_LOC)
			{
				mc_ptr->longpress_time_s = 0;
				HAL_TIM_Base_Stop_IT(mc_ptr->htim_reset_ptr);
			}
			break;
	}
	return MC_LOCALIZATION_OK;
}

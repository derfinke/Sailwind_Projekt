/*
 * Linear_Guide.c
 *
 *  Created on: 18.06.2023
 *      Author: Bene
 */

#include "Linear_Guide.h"

/* private function prototypes -----------------------------------------------*/
static Endswitch_t *LG_Endswitch_bar_init();
static LED_t* LG_LED_bar_init();
static void LG_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode);
static void LG_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode);

/* API function definitions --------------------------------------------------*/
Linear_Guide_t Linear_Guide_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel)
{
	Linear_Guide_t linear_guide = {
			.operating_mode = LG_operating_mode_manual,
			.motor = Motor_init(hdac_ptr, htim_ptr, htim_channel, htim_active_channel),
			.localization = Localization_init(MOTOR_PULSE_PER_ROTATION, LG_DISTANCE_PER_ROTATION),
			.endswitch_bar = LG_Endswitch_bar_init(),
			.led_bar = LG_LED_bar_init()
	};
	return linear_guide;
}

/* void LG_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
 *  Description:
 *   - set operating mode depending on current state of the system
 *   - operating mode can always be switched to manual
 *   - operating mode can only be switched to automatic, if the linear guide is localized
 */
void LG_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
{
	boolean_t set_automatic = operating_mode == LG_operating_mode_automatic;
	boolean_t set_manual = operating_mode == LG_operating_mode_manual;
	boolean_t is_localized = lg_ptr->localization.is_localized;
	if ((set_automatic && is_localized) || set_manual)
	{
		lg_ptr->operating_mode = operating_mode;
		LG_LED_set_operating_mode(lg_ptr, operating_mode);
	}
}

void LG_callback_motor_pulse_capture(Linear_Guide_t *lg_ptr)
{
	if (Motor_callback_measure_rpm(&lg_ptr->motor))
	{
		Loc_update_pulse_count(&lg_ptr->localization);
	}
}

/* boolean_t linear_guide_get_manual_moving_permission(Linear_guide_t lg)
 *  Description:
 *   - return True, if operating mode is manual and motor is not currently moving automatically due to localization process
 */
boolean_t LG_get_manual_moving_permission(Linear_Guide_t lg)
{
	return
			lg.operating_mode == LG_operating_mode_manual
			&&
			(
				lg.localization.state == Loc_state_4_set_center_pos
				||
				lg.localization.state == Loc_state_0_init
			);
}

void LG_move(Linear_Guide_t *lg_ptr, Loc_movement_t movement)
{
	switch(movement)
	{
		case Loc_movement_stop:
			Motor_stop_moving(&lg_ptr->motor); break;
		case Loc_movement_backwards:
			Motor_start_moving(&lg_ptr->motor, Motor_function_rechtslauf); break;
		case Loc_movement_forward:
			Motor_start_moving(&lg_ptr->motor, Motor_function_linkslauf); break;
	}
	lg_ptr->localization.movement = movement;
	LED_switch(&lg_ptr->led_bar[LG_LED_center_pos_set], LED_OFF);
}

void LG_set_center(Linear_Guide_t *lg_ptr)
{
	Loc_set_center(&lg_ptr->localization);
	LED_switch(&lg_ptr->led_bar[LG_LED_center_pos_set], LED_ON);
}

void LG_set_endpos(Linear_Guide_t *lg_ptr)
{
	Loc_set_endpos(&lg_ptr->localization);
}

boolean_t LG_Endswitch_detected(Linear_Guide_t *lg_ptr, LG_Endswitch_ID_t endswitch_ID)
{
	return Endswitch_detected(&lg_ptr->endswitch_bar[endswitch_ID]);
}

/* private function test definitions -------------------------------------------*/
void LG_Test_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
{
	LG_LED_set_operating_mode(lg_ptr, operating_mode);
}
void LG_Test_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode)
{
	LG_LED_set_sail_adjustment_mode(lg_ptr, sail_adjustment_mode);
}

/* private function definitions -----------------------------------------------*/

static Endswitch_t *LG_Endswitch_bar_init()
{
	static Endswitch_t endswitch_bar[LG_ENDSWITCH_COUNT];
	endswitch_bar[LG_Endswitch_front] = Endswitch_init(GPIOB, Endschalter_Vorne_Pin);
	endswitch_bar[LG_Endswitch_back] = Endswitch_init(GPIOB, Endschalter_Hinten_Pin);
	return endswitch_bar;
}

static LED_t* LG_LED_bar_init()
{
	static LED_t led_bar[LG_LED_COUNT];
	led_bar[LG_LED_error] = LED_init(GPIOD, LED_Stoerung_Pin, LED_OFF);
	led_bar[LG_LED_manual] = LED_init(GPIOD, LED_Manuell_Pin, LED_ON);
	led_bar[LG_LED_error] = LED_init(GPIOB, LED_Automatik_Pin, LED_OFF);
	led_bar[LG_LED_error] = LED_init(GPIOE, LED_Rollen_Pin, LED_OFF);
	led_bar[LG_LED_error] = LED_init(GPIOB, LED_Trimmen_Pin, LED_OFF);
	led_bar[LG_LED_error] = LED_init(GPIOD, LED_Kalibrieren_Speichern_Pin, LED_OFF);
	return led_bar;
}

/* static void LG_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
 * 	Description:
 * 	 - depending on parameter "operating_mode" (IO_operating_mode_manual / ..._automatic)
 * 	   switch corresponding LEDs on and off
 * 	 - when automatic mode is set, the center pos LED is assured to be switched on (in case it was turned off due to manually adapting the center pos)
 */
static void LG_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
{
	LED_State_t automatic, manual;
	switch(operating_mode)
	{
		case LG_operating_mode_manual:
			manual = LED_ON, automatic = LED_OFF;
			break;
		case LG_operating_mode_automatic:
			manual = LED_OFF, automatic = LED_ON;
			LED_switch(&lg_ptr->led_bar[LG_LED_center_pos_set], LED_ON);
			break;
	}
	LED_switch(&lg_ptr->led_bar[LG_LED_automatic], automatic);
	LED_switch(&lg_ptr->led_bar[LG_LED_manual], manual);
}

/* static void LG_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode)
 * 	Description:
 * 	 - when sail adjustment mode changes (rollung / trimmung)
 * 	   the corresponding LEDs are switched on and off
 */
static void LG_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode)
{
	LED_State_t rollung, trimmung;
	switch(sail_adjustment_mode)
	{
		case LG_sail_adjustment_mode_rollung:
			rollung = LED_ON, trimmung = LED_OFF;
			break;
		case LG_sail_adjustment_mode_trimmung:
			rollung = LED_OFF, trimmung = LED_ON;
			break;
	}
	LED_switch(&lg_ptr->led_bar[LG_LED_rollung], rollung);
	LED_switch(&lg_ptr->led_bar[LG_LED_trimmung], trimmung);
}
/* Timer Callback implementation for rpm measurement --------------------------*/

/* void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim_ptr)
 *  Description:
 *   - must be redefined in main.c
 *   - triggered, when a rising edge of the motor pulse signal (OUT1) is detected
 *   - calls function to measure the frequency between two pulses and converts it to an rpm value
 */
/*
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim_ptr)
{
	LG_callback_motor_pulse_capture(&linear_guide);
}
*/

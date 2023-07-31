/*
 * Linear_Guide.c
 *
 *  Created on: 18.06.2023
 *      Author: Bene
 */

#include "Linear_Guide.h"

/* private function prototypes -----------------------------------------------*/
static LG_LEDs_t Linear_Guide_LEDs_init();
static LG_Endswitches_t Linear_Guide_Endswitches_init();
static void Linear_Guide_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode);
static void Linear_Guide_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode);

/* API function definitions --------------------------------------------------*/
Linear_Guide_t Linear_Guide_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel)
{
	Linear_Guide_t linear_guide = {
			.operating_mode = LG_operating_mode_manual,
			.motor = Motor_init(hdac_ptr, htim_ptr, htim_channel, htim_active_channel),
			.localization = Localization_init(MOTOR_PULSE_PER_ROTATION, LG_DISTANCE_PER_ROTATION),
			.endswitches = Linear_Guide_Endswitches_init(),
			.leds = Linear_Guide_LEDs_init()
	};
	return linear_guide;
}

/* void Linear_Guide_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
 *  Description:
 *   - set operating mode depending on current state of the system
 *   - operating mode can always be switched to manual
 *   - operating mode can only be switched to automatic, if the linear guide is localized
 */
void Linear_Guide_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
{
	boolean_t set_automatic = operating_mode == LG_operating_mode_automatic;
	boolean_t set_manual = operating_mode == LG_operating_mode_manual;
	boolean_t is_localized = lg_ptr->localization.is_localized;
	if ((set_automatic && is_localized) || set_manual)
	{
		lg_ptr->operating_mode = operating_mode;
		Linear_Guide_LED_set_operating_mode(lg_ptr, operating_mode);
	}
}

void Linear_Guide_callback_motor_pulse_capture(Linear_Guide_t *lg_ptr)
{
	if (Motor_callback_measure_rpm(&lg_ptr->motor))
	{
		Localization_callback_pulse_count(&lg_ptr->localization);
	}
}

void Linear_Guide_move(Linear_Guide_t *lg_ptr, Loc_movement_t movement)
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
}

boolean_t Linear_Guide_Endswitch_detected(Endswitch_t *endswitch_ptr)
{
	return Endswitch_detected(endswitch_ptr);
}

/* test function definitions -------------------------------------------*/
void Linear_Guide_Test_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
{
	Linear_Guide_LED_set_operating_mode(lg_ptr, operating_mode);
}
void Linear_Guide_Test_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode)
{
	Linear_Guide_LED_set_sail_adjustment_mode(lg_ptr, sail_adjustment_mode);
}

/* private function definitions -----------------------------------------------*/

static LG_Endswitches_t Linear_Guide_Endswitches_init()
{
	LG_Endswitches_t lg_endswitches = {
			.front = Endswitch_init(Endschalter_Vorne_GPIO_Port, Endschalter_Vorne_Pin),
			.back = Endswitch_init(Endschalter_Hinten_GPIO_Port, Endschalter_Hinten_Pin)
	};
	return lg_endswitches;
}

static LG_LEDs_t Linear_Guide_LEDs_init()
{
	LG_LEDs_t lg_leds = {
			.error = LED_init(LED_Stoerung_GPIO_Port, LED_Stoerung_Pin, LED_OFF),
			.manual = LED_init(LED_Manuell_GPIO_Port, LED_Manuell_Pin, LED_ON),
			.automatic = LED_init(LED_Automatik_GPIO_Port, LED_Automatik_Pin, LED_OFF),
			.rollung = LED_init(LED_Rollen_GPIO_Port, LED_Rollen_Pin, LED_OFF),
			.trimmung = LED_init(LED_Trimmen_GPIO_Port, LED_Trimmen_Pin, LED_OFF),
			.center_pos_set = LED_init(LED_Kalibrieren_Speichern_GPIO_Port, LED_Kalibrieren_Speichern_Pin, LED_OFF)
	};
	return lg_leds;
}

/* static void Linear_Guide_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
 * 	Description:
 * 	 - depending on parameter "operating_mode" (IO_operating_mode_manual / ..._automatic)
 * 	   switch corresponding LEDs on and off
 */
static void Linear_Guide_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
{
	LED_State_t automatic, manual;
	switch(operating_mode)
	{
		case LG_operating_mode_manual:
			manual = LED_ON, automatic = LED_OFF;
			break;
		case LG_operating_mode_automatic:
			manual = LED_OFF, automatic = LED_ON;
			break;
	}
	LED_switch(&lg_ptr->leds.automatic, automatic);
	LED_switch(&lg_ptr->leds.manual, manual);
}

/* static void Linear_Guide_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode)
 * 	Description:
 * 	 - when sail adjustment mode changes (rollung / trimmung)
 * 	   the corresponding LEDs are switched on and off
 */
static void Linear_Guide_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode)
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
	LED_switch(&lg_ptr->leds.rollung, rollung);
	LED_switch(&lg_ptr->leds.trimmung, trimmung);
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
	Linear_Guide_callback_motor_pulse_capture(&linear_guide);
}
*/

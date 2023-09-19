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
static void Linear_Guide_LED_set_operating_mode(Linear_Guide_t *lg_ptr);
static void Linear_Guide_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr);
static uint16_t Linear_Guide_rpm_to_speed_mms(uint16_t rpm_value);
static uint16_t Linear_Guide_speed_mms_to_rpm(uint16_t speed_mms);

/* API function definitions --------------------------------------------------*/
Linear_Guide_t Linear_Guide_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel)
{
	Linear_Guide_t linear_guide = {
			.operating_mode = HAL_GPIO_ReadPin(Switch_Betriebsmodus_GPIO_Port, Switch_Betriebsmodus_Pin) == GPIO_PIN_SET ? LG_operating_mode_manual : LG_operating_mode_automatic,
			.motor = Motor_init(hdac_ptr, htim_ptr, htim_channel, htim_active_channel),
			.localization = Linear_Guide_read_Localization(),
			.endswitches = Linear_Guide_Endswitches_init(),
			.leds = Linear_Guide_LEDs_init(),
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
		Linear_Guide_LED_set_operating_mode(lg_ptr);
	}
}

void Linear_Guide_callback_motor_pulse_capture(Linear_Guide_t *lg_ptr)
{
	if (Localization_callback_update_position(&lg_ptr->localization))
	{
		Linear_Guide_update_sail_adjustment_mode(lg_ptr);
	}
}

void Linear_Guide_move(Linear_Guide_t *lg_ptr, Loc_movement_t movement)
{
	switch(movement)
	{
		case Loc_movement_stop:
			Motor_stop_moving(&lg_ptr->motor); break;
		case Loc_movement_backwards:
			Motor_start_moving(&lg_ptr->motor, Motor_function_cw_rotation); break;
		case Loc_movement_forward:
			Motor_start_moving(&lg_ptr->motor, Motor_function_ccw_rotation); break;
	}
	lg_ptr->localization.movement = movement;
}

void Linear_Guide_speed_ramp(Linear_Guide_t *lg_ptr)
{
	Motor_speed_ramp(&lg_ptr->motor);
}

void Linear_Guide_change_speed_mms(Linear_Guide_t *lg_ptr, uint16_t speed_mms)
{
	lg_ptr->motor.normal_rpm = Linear_Guide_speed_mms_to_rpm(speed_mms);
	if (lg_ptr->localization.movement != Loc_movement_stop && lg_ptr->motor.ramp_activated)
	{
		lg_ptr->motor.ramp_final_rpm = lg_ptr->motor.normal_rpm;
	}
}

uint16_t Linear_Guide_get_speed_mms(Linear_Guide_t *lg_ptr)
{
	return Linear_Guide_rpm_to_speed_mms(lg_ptr->motor.rpm_set_point);
}

boolean_t Linear_Guide_Endswitch_detected(Endswitch_t *endswitch_ptr)
{
	return Endswitch_detected(endswitch_ptr);
}

void Linear_Guide_safe_Localization(Localization_t loc)
{
	char FRAM_buffer[LOC_SERIAL_SIZE];
	Localization_serialize(loc, FRAM_buffer);
	FRAM_write((uint8_t *)FRAM_buffer, 0x0000, LOC_SERIAL_SIZE);
}

Localization_t Linear_Guide_read_Localization()
{
	char FRAM_buffer[LOC_SERIAL_SIZE];
	FRAM_init();
	FRAM_read(0x0000, (uint8_t *) FRAM_buffer, LOC_SERIAL_SIZE);
	return Localization_init(LG_DISTANCE_MM_PER_PULSE, FRAM_buffer);
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

void Linear_Guide_update_sail_adjustment_mode(Linear_Guide_t *lg_ptr)
{
  int32_t current_pos = Localization_pulse_count_to_distance(lg_ptr->localization);
  int32_t center_pos = lg_ptr->localization.center_pos_mm;
	lg_ptr->sail_adjustment_mode = current_pos < center_pos ? LG_sail_adjustment_mode_rollung : LG_sail_adjustment_mode_trimmung;
	Linear_Guide_LED_set_sail_adjustment_mode(lg_ptr);
}

/* static void Linear_Guide_LED_set_operating_mode(Linear_Guide_t *lg_ptr)
 * 	Description:
 * 	 - depending on parameter "operating_mode" (IO_operating_mode_manual / ..._automatic)
 * 	   switch corresponding LEDs on and off
 */
static void Linear_Guide_LED_set_operating_mode(Linear_Guide_t *lg_ptr)
{
	LED_State_t automatic, manual;
	switch(lg_ptr->operating_mode)
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

/* static void Linear_Guide_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr)
 * 	Description:
 * 	 - when sail adjustment mode changes (rollung / trimmung)
 * 	   the corresponding LEDs are switched on and off
 */
static void Linear_Guide_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr)
{
	LED_State_t rollung, trimmung;
	switch(lg_ptr->sail_adjustment_mode)
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

static uint16_t Linear_Guide_rpm_to_speed_mms(uint16_t rpm_value)
{
	return (uint16_t) (LG_DISTANCE_MM_PER_ROTATION * rpm_value / 60.0F);
}

static uint16_t Linear_Guide_speed_mms_to_rpm(uint16_t speed_mms)
{
	return (uint16_t) (speed_mms / (float) LG_DISTANCE_MM_PER_ROTATION * 60);
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

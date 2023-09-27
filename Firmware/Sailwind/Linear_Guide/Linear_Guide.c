/**
 * \file Linear_Guide.c
 * @date 18 Jun 2023
 * @brief Interaction of all physical components of the linear guide system controlled both in manual and automatic mode
 */

#include "Linear_Guide.h"
#include "FRAM.h"
#include <stdlib.h>


/* defines ------------------------------------------------------------*/
#define LG_DISTANCE_MM_PER_ROTATION 5.62
#define LG_DISTANCE_MM_PER_PULSE LG_DISTANCE_MM_PER_ROTATION/MOTOR_PULSE_PER_ROTATION
#define LG_DISTANCE_MIN_MM 30
#define LG_DISTANCE_MAX_MM 730
#define LG_DISTANCE_FAULT_TOLERANCE_MM 3
#define LG_FAULT_CHECK_POSITIVE -1
#define LG_FAULT_CHECK_NEGATIVE 0
#define LG_FAULT_CHECK_SKIPPED 1

/* private function prototypes -----------------------------------------------*/

/**
 * @brief initialise the two endswitches of the linear guide
 * @param none
 * @retval lg_endswitches: struct object with the endswitches as members
 */
static LG_Endswitches_t Linear_Guide_Endswitches_init();
/**
 * @brief update the operating mode LEDs when mode has changed
 * @param lg_ptr: linear_guide reference
 * @retval none
 */
static void Linear_Guide_LED_set_operating_mode(Linear_Guide_t *lg_ptr);
/**
 * @brief update the sailadjustment mode LEDs, when mode has changed
 * @param lg_ptr: linear_guide reference
 * @retval none
 */
static void Linear_Guide_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr);
/**
 * @brief update error LED
 * @param lg_ptr: linear_guide reference
 * @retval none
 */
static void Linear_Guide_LED_set_error(Linear_Guide_t *lg_ptr);
/**
 * @brief update sailadjustment mode, if current position crossed the center position
 * @param lg_ptr: linear_guide reference
 * @retval update_status
 */
static int8_t Linear_Guide_update_sail_adjustment_mode(Linear_Guide_t *lg_ptr);
/**
 * @brief update speed ramp and movement depending on desired position
 * @param lg_ptr: linear_guide reference
 * @retval none
 */
static void Linear_Guide_update_movement(Linear_Guide_t *lg_ptr);
/**
 * @brief handle all detectable errors
 * @param lg_ptr: linear_guide reference
 * @retval update_status
 */
static int8_t Linear_Guide_error_handler(Linear_Guide_t *lg_ptr);
/**
 * @brief check, if distance sensor and calculated distance from pulse signal match within a defined tolerance
 * @param lg_ptr: linear_guide reference
 * @retval fault_check_status
 */
static int8_t Linear_Guide_check_distance_fault(Linear_Guide_t *lg_ptr);
/**
 * @brief check, if motor sends out an error signal
 * @param lg_ptr: linear_guide reference
 * @retval fault_check_status
 */
static int8_t Linear_guide_check_motor_fault(Linear_Guide_t *lg_ptr);
/**
 * @brief convert the measured value of the distance sensor to the linear guide relative distance
 * @param lg_ptr: linear_guide reference
 * @retval relative_distance
 */
static int32_t Linear_Guide_parse_distance_sensor_value(Linear_Guide_t *lg_ptr);
/**
 * @brief convert given rpm value of the motor to movement speed of the linear guide in mm/s
 * @param rpm_value
 * @retval speed_mms
 */
static uint16_t Linear_Guide_rpm_to_speed_mms(uint16_t rpm_value);
/**
 * @brief convert given movement speed of the linear guide in mm/s to rpm value of the motor
 * @param speed_mms
 * @retval rpm_value
 */
static uint16_t Linear_Guide_speed_mms_to_rpm(uint16_t speed_mms);

/* API function definitions --------------------------------------------------*/
Linear_Guide_t Linear_Guide_init(DAC_HandleTypeDef *hdac_ptr, ADC_HandleTypeDef *hadc_distance_ptr)
{
	Linear_Guide_t linear_guide = {
			.motor = Motor_init(hdac_ptr),
			.localization = Linear_Guide_read_Localization(),
			.endswitches = Linear_Guide_Endswitches_init(),
			.distance_sensor = IO_init_distance_sensor(hadc_distance_ptr, LG_DISTANCE_MIN_MM, LG_DISTANCE_MAX_MM),
	};
	return linear_guide;
}

LG_LEDs_t Linear_Guide_LEDs_init(LG_operating_mode_t op_mode)
{
	LG_LEDs_t lg_leds = {
			.error = LED_init(LED_Stoerung_GPIO_Port, LED_Stoerung_Pin, LED_OFF),
			.manual = LED_init(LED_Manuell_GPIO_Port, LED_Manuell_Pin, op_mode == LG_operating_mode_manual),
			.automatic = LED_init(LED_Automatik_GPIO_Port, LED_Automatik_Pin, op_mode == LG_operating_mode_automatic),
			.roll = LED_init(LED_Roll_GPIO_Port, LED_Roll_Pin, LED_OFF),
      .trim = LED_init(LED_Trim_GPIO_Port, LED_Trim_Pin, LED_OFF),
      .center_pos_set = LED_init(LED_Kalibrieren_Speichern_GPIO_Port, LED_Kalibrieren_Speichern_Pin, LED_OFF)
	};
	return lg_leds;
}

int8_t Linear_Guide_update(Linear_Guide_t *lg_ptr)
{
	int8_t update_status = Linear_Guide_error_handler(lg_ptr);
	if (update_status == LG_UPDATE_EMERGENCY_SHUTDOWN);
	{
		return update_status;
	}
	Linear_Guide_update_movement(lg_ptr);
	if (Localization_update_position(&lg_ptr->localization) == LOC_POSITION_UPDATED)
	{
		Linear_Guide_update_sail_adjustment_mode(lg_ptr);
	}
	return update_status;
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
	Localization_callback_pulse_count(&lg_ptr->localization);
}

int8_t Linear_Guide_move(Linear_Guide_t *lg_ptr, Loc_movement_t movement)
{
	if (movement == lg_ptr->localization.movement)
	{
		return LG_MOVEMENT_RETAINED;
	}
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
	Linear_Guide_safe_Localization(lg_ptr->localization);
	return LG_MOVEMENT_CHANGED;
}

void Linear_Guide_set_desired_roll_trim_percentage(Linear_Guide_t *lg_ptr, uint8_t percentage, LG_sail_adjustment_mode_t adjustment_mode)
{
	Localization_t *loc_ptr = &lg_ptr->localization;
	int8_t sign = adjustment_mode == LG_sail_adjustment_mode_roll ? 1 : -1;
	loc_ptr->desired_pos_mm = (int32_t) (- sign * ((loc_ptr->end_pos_mm + sign * loc_ptr->center_pos_mm) * (percentage / 100.0F) - sign * loc_ptr->center_pos_mm));
}

uint8_t Linear_Guide_get_current_roll_trim_percentage(Linear_Guide_t lg)
{
	Localization_t loc = lg.localization;
	int8_t sign = lg.sail_adjustment_mode == LG_sail_adjustment_mode_roll ? 1 : -1;
	return (uint8_t) ((sign * (loc.center_pos_mm - loc.current_pos_mm)) / (float) (loc.end_pos_mm + sign * loc.center_pos_mm) * 100);
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

int8_t Linear_Guide_safe_Localization(Localization_t loc)
{
	if (!loc.is_localized)
	{
		return LG_NOT_LOCALIZED;
	}
	char FRAM_buffer[LOC_SERIAL_SIZE];
	Localization_serialize(loc, FRAM_buffer);
	if(FRAM_write((uint8_t *)FRAM_buffer, 0x0000, LOC_SERIAL_SIZE) != HAL_OK)
	{
	  printf("Saving Position failed!\r\n");
    return -1;
	}
  return LG_LOCALIZATION_SAFED;
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

static int8_t Linear_Guide_update_sail_adjustment_mode(Linear_Guide_t *lg_ptr)
{
	if (!lg_ptr->localization.is_localized)
	{
		return LG_NOT_LOCALIZED;
	}
	int32_t current_pos = lg_ptr->localization.current_pos_mm;
	int32_t center_pos = lg_ptr->localization.center_pos_mm;
	lg_ptr->sail_adjustment_mode = current_pos < center_pos ? LG_sail_adjustment_mode_roll : LG_sail_adjustment_mode_trim;
	Linear_Guide_LED_set_sail_adjustment_mode(lg_ptr);
	return LG_ADJUSTMENT_MODE_UPDATED;
}

static void Linear_Guide_update_movement(Linear_Guide_t *lg_ptr)
{
	if (lg_ptr->operating_mode == LG_operating_mode_automatic)
	{
		Loc_movement_t movement;
		Localization_t *loc_ptr = &lg_ptr->localization;
		if (loc_ptr->desired_pos_mm > loc_ptr->current_pos_mm)
		{
			movement = Loc_movement_backwards;
		}
		else if (loc_ptr->desired_pos_mm < loc_ptr->current_pos_mm)
		{
			movement = Loc_movement_forward;
		}
		else
		{
			movement = Loc_movement_stop;
		}
		Linear_Guide_move(lg_ptr, movement);
	}
	Motor_speed_ramp(&lg_ptr->motor);
}

static int8_t Linear_Guide_error_handler(Linear_Guide_t *lg_ptr)
{
	int8_t update_status = LG_UPDATE_NORMAL;
	LG_error_state_t new_error_state = LG_error_state_0_normal;
	if (Linear_Guide_check_distance_fault(lg_ptr) == LG_FAULT_CHECK_POSITIVE)
	{
		new_error_state = LG_error_state_1_distance_fault;
		Linear_Guide_set_operating_mode(lg_ptr, LG_operating_mode_manual);
		Localization_recover(&lg_ptr->localization, LOC_RECOVERY_PARTIAL, True);
	}
	if (Linear_guide_check_motor_fault(lg_ptr) == LG_FAULT_CHECK_POSITIVE)
	{
		new_error_state = LG_error_state_2_motor_fault;
		Linear_Guide_move(lg_ptr, Loc_movement_stop);
		update_status = LG_UPDATE_EMERGENCY_SHUTDOWN;
	}
	if (new_error_state != lg_ptr->error_state)
	{
		lg_ptr->error_state = new_error_state;
		Linear_Guide_LED_set_error(lg_ptr);
	}
	return update_status;
}

static int8_t Linear_Guide_check_distance_fault(Linear_Guide_t *lg_ptr)
{
	if (!lg_ptr->localization.is_localized)
	{
		return LG_FAULT_CHECK_SKIPPED;
	}
	IO_Get_Measured_Value(&lg_ptr->distance_sensor);
	int32_t measured_value = Linear_Guide_parse_distance_sensor_value(lg_ptr);
	if (abs(measured_value - lg_ptr->localization.current_pos_mm) > LG_DISTANCE_FAULT_TOLERANCE_MM)
	{
		return LG_FAULT_CHECK_POSITIVE;
	}
	return LG_FAULT_CHECK_NEGATIVE;
}

static int8_t Linear_guide_check_motor_fault(Linear_Guide_t *lg_ptr)
{
	if (Motor_error(&lg_ptr->motor) == True)
	{
		return LG_FAULT_CHECK_POSITIVE;
	}
	return LG_FAULT_CHECK_NEGATIVE;
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
	LED_State_t roll, trim;
	switch(lg_ptr->sail_adjustment_mode)
	{
		case LG_sail_adjustment_mode_roll:
			roll = LED_ON, trim = LED_OFF;
			break;
		case LG_sail_adjustment_mode_trim:
			roll = LED_OFF, trim = LED_ON;
			break;
	}
	LED_switch(&lg_ptr->leds.roll, roll);
	LED_switch(&lg_ptr->leds.trim, trim);
}

static void Linear_Guide_LED_set_error(Linear_Guide_t *lg_ptr)
{
	LED_State_t new_state;
	if (lg_ptr->error_state == LG_error_state_0_normal)
	{
		new_state = LED_OFF;
	}
	else
	{
		new_state = LED_ON;
	}
	LED_switch(&lg_ptr->leds.error, new_state);
}

static int32_t Linear_Guide_parse_distance_sensor_value(Linear_Guide_t *lg_ptr)
{
	IO_analogSensor_t ds = lg_ptr->distance_sensor;
	return (int32_t) (ds.measured_value - ds.min_possible_value - lg_ptr->localization.end_pos_mm);
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

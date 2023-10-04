/**
 * \file Linear_Guide.c
 * @date 18 Jun 2023
 * @brief Interaction of all physical components of the linear guide system controlled both in manual and automatic mode
 */

#include "Linear_Guide.h"
#include "FRAM.h"
#include <stdlib.h>
#include "FRAM_memory_mapping.h"


/* defines ------------------------------------------------------------*/
#define LG_DISTANCE_MM_PER_ROTATION 1.12F
#define LG_DISTANCE_MM_PER_PULSE LG_DISTANCE_MM_PER_ROTATION/MOTOR_PULSE_PER_ROTATION
#define LG_STANDARD_MAX_DISTANCE_DELTA_MM 10
#define LG_CURRENT_FAULT_TOLERANCE_MA 4000
#define LG_FAULT_CHECK_POSITIVE -1
#define LG_FAULT_CHECK_NEGATIVE 0
#define LG_FAULT_CHECK_SKIPPED 1
#define LG_BRAKE_PATH_OFFSET_REL 1.25F
#define LG_SET_CENTER_OK 0
#define LG_SET_CENTER_NOT_TRIGGERED 1


static IO_analogSensor_t *LG_distance_sensor_ptr = {0};
static IO_analogSensor_t *LG_current_sensor_ptr = {0};
/* private function prototypes -----------------------------------------------*/

static Linear_Guide_t LG_linear_guide = {0};
/**
 * @brief initialise the two endswitches of the linear guide
 * @param none
 * @retval lg_endswitches: struct object with the endswitches as members
 */
static LG_Endswitches_t Linear_Guide_Endswitches_init();
/**
 * @brief initialise all status LEDs
 * @param op_mode: depending on the value, either the manual or automatic LED is switched on
 * @retval lg_leds: struct of all LEDs as members
 */
static LG_LEDs_t Linear_Guide_LEDs_init(LG_operating_mode_t op_mode);
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
static int8_t Linear_Guide_get_adjustment_mode(LG_sail_adjustment_mode_t last_mode, int16_t distance_to_center);
static uint16_t Linear_Guide_get_adjustment_range_mm(Linear_Guide_t lg, LG_sail_adjustment_mode_t adjustment_mode);
/**
 * @brief update speed ramp and movement depending on desired position
 * @param lg_ptr: linear_guide reference
 * @param update_status: immediate stop, if emergency shutdown is set
 * @retval none
 */
static void Linear_Guide_update_movement(Linear_Guide_t *lg_ptr, int8_t update_status);
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
static int8_t Linear_Guide_check_motor_fault(Linear_Guide_t *lg_ptr);
/**
 * @brief check, if motor current value [mA] is within a defined tolerance
 * @param lg_ptr: linear_guide reference
 * @retval fault_check_status
 */
static int8_t Linear_Guide_check_current_fault(Linear_Guide_t *lg_ptr);
/**
 * @brief check, if wind_speed_fault error was set by controllino
 * @param lg_ptr: linear_guide reference
 * @retval fault_check_status
 */
static int8_t Linear_Guide_check_wind_fault(Linear_Guide_t *lg_ptr);
/**
 * @brief calculate brake path for current normal speed and safe it in localization member
 * @param lg_ptr: linear_guide reference
 * @retval none
 */
static void Linear_Guide_calculate_break_path(Linear_Guide_t *lg_ptr);

/**
 * @brief readout saved max distance delta
 * @param none
 * @retval max distance delta in mm
 */
static uint8_t Linear_Guide_read_max_distance_delta(void);

/* API function definitions --------------------------------------------------*/
void Linear_Guide_init(DAC_HandleTypeDef *hdac_ptr)
{
	LG_linear_guide.error_state = LG_error_state_0_normal;
	LG_linear_guide.operating_mode = LG_operating_mode_manual;
	LG_linear_guide.motor = Motor_init(hdac_ptr);
	LG_linear_guide.localization = Linear_Guide_read_Localization();
	LG_linear_guide.endswitches = Linear_Guide_Endswitches_init();
	LG_distance_sensor_ptr = IO_get_distance_sensor();
	LG_current_sensor_ptr = IO_get_current_sensor();
	LG_linear_guide.max_distance_fault = Linear_Guide_read_max_distance_delta();
	Linear_Guide_calculate_break_path(&LG_linear_guide);
	Linear_Guide_set_operating_mode(&LG_linear_guide, LG_operating_mode_automatic);
	LG_linear_guide.leds = Linear_Guide_LEDs_init(LG_linear_guide.operating_mode);
}

LG_LEDs_t Linear_Guide_LEDs_init(LG_operating_mode_t op_mode)
{
	LG_LEDs_t lg_leds = {
			.error = LED_init(LED_Stoerung_GPIO_Port, LED_Stoerung_Pin, LED_OFF),
			.manual = LED_init(LED_Manuell_GPIO_Port, LED_Manuell_Pin, op_mode == LG_operating_mode_manual),
			.automatic = LED_init(LED_Automatik_GPIO_Port, LED_Automatik_Pin, op_mode == LG_operating_mode_automatic),
			.roll = LED_init(LED_Roll_GPIO_Port, LED_Roll_Pin, LED_OFF),
      .trim = LED_init(LED_Trim_GPIO_Port, LED_Trim_Pin, LED_OFF),
      .center_pos_set = LED_init(LED_set_center_GPIO_Port, LED_set_center_Pin, LED_OFF)
	};
	return lg_leds;
}

int8_t Linear_Guide_update(Linear_Guide_t *lg_ptr)
{
	int8_t update_status = Linear_Guide_error_handler(lg_ptr);
	Linear_Guide_update_movement(lg_ptr, update_status);
	if (Localization_update_position(&lg_ptr->localization) == LOC_POSITION_UPDATED)
	{
		Linear_Guide_safe_Localization(lg_ptr->localization);
	}
	Linear_Guide_update_sail_adjustment_mode(lg_ptr);
	return update_status;
}

int8_t Linear_Guide_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode)
{
	if (!lg_ptr->localization.is_localized)
	{
		return LG_SWITCH_OPERATING_MODE_DENIED;
	}
	lg_ptr->operating_mode = operating_mode;
	Linear_Guide_LED_set_operating_mode(lg_ptr);
	return LG_SWITCH_OPERATING_MODE_OK;
}

void Linear_Guide_callback_motor_pulse_capture(Linear_Guide_t *lg_ptr)
{
	Localization_callback_pulse_count(&lg_ptr->localization);
}

int8_t Linear_Guide_move(Linear_Guide_t *lg_ptr, Loc_movement_t movement, boolean_t immediate)
{
	if (movement == lg_ptr->localization.movement && (movement == Loc_movement_stop || !Motor_is_currently_braking(lg_ptr->motor)))
	{
		return LG_MOVEMENT_RETAINED;
	}
	switch(movement)
	{
		case Loc_movement_stop:
			Motor_stop_moving(&lg_ptr->motor, immediate); break;
		case Loc_movement_backwards:
			Motor_start_moving(&lg_ptr->motor, Motor_function_cw_rotation); break;
		case Loc_movement_forward:
			Motor_start_moving(&lg_ptr->motor, Motor_function_ccw_rotation); break;
	}
	if (movement != Loc_movement_stop || immediate)
	{
		lg_ptr->localization.movement = movement;
	}
	return LG_MOVEMENT_CHANGED;
}

void Linear_Guide_manual_move(Linear_Guide_t *lg_ptr, Loc_movement_t movement)
{
	if (!Linear_Guide_get_moving_permission(*lg_ptr))
	{
		return;
	}
	Localization_t *loc_ptr = &lg_ptr->localization;
	if (movement == Loc_movement_stop)
	{
		int8_t sign = loc_ptr->movement == Loc_movement_backwards ? 1 : -1;
		loc_ptr->desired_pos_mm = loc_ptr->current_pos_mm + sign * loc_ptr->brake_path_mm;
		loc_ptr->desired_pos_queue = loc_ptr->current_pos_mm;
	}
	else
	{
		int8_t sign = movement == Loc_movement_backwards ? 1 : -1;
		int16_t desired_pos_mm = sign * loc_ptr->end_pos_mm;
		Localization_set_desired_pos_queued(loc_ptr, desired_pos_mm, movement);
	}
}

boolean_t Linear_Guide_get_moving_permission(Linear_Guide_t lg)
{
	return
			lg.operating_mode == LG_operating_mode_manual
			&&
			(
				lg.localization.state >= Loc_state_4_set_center_pos
			);
}

void Linear_Guide_set_desired_roll_trim_percentage(Linear_Guide_t *lg_ptr, int8_t percentage)
{
	Localization_t *loc_ptr = &lg_ptr->localization;
	LG_sail_adjustment_mode_t mode = Linear_Guide_get_adjustment_mode(lg_ptr->sail_adjustment_mode, percentage);
	uint16_t range_mm = Linear_Guide_get_adjustment_range_mm(*lg_ptr, mode);
	int16_t desired_pos_mm = (int16_t) (loc_ptr->center_pos_mm + range_mm * (percentage / 100.0F));
	Localization_set_desired_pos_queued(loc_ptr, desired_pos_mm, Localization_get_next_movement(*loc_ptr, desired_pos_mm));
}

int8_t Linear_Guide_get_current_roll_trim_percentage(Linear_Guide_t lg)
{
	Localization_t loc = lg.localization;
	LG_sail_adjustment_mode_t mode = lg.sail_adjustment_mode;
	uint16_t range_mm = Linear_Guide_get_adjustment_range_mm(lg, mode);
	return (int8_t) ((loc.current_pos_mm - loc.center_pos_mm) / (float) range_mm * 100);
}

void Linear_Guide_change_speed_rpm(Linear_Guide_t *lg_ptr, uint16_t speed_rpm)
{
	lg_ptr->motor.normal_rpm = speed_rpm;
	if (lg_ptr->localization.movement != Loc_movement_stop)
	{
		lg_ptr->motor.ramp_final_rpm = lg_ptr->motor.normal_rpm;
		lg_ptr->motor.ramp_activated = True;
	}
	Linear_Guide_calculate_break_path(lg_ptr);
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
	if(FRAM_write((uint8_t *)FRAM_buffer, LINEAR_GUIDE_INFOS, LOC_SERIAL_SIZE) != HAL_OK)
	{
		printf("Saving Position failed!\r\n");
		return LG_LOCALIZATION_FAILED;
	}
  return LG_LOCALIZATION_SAFED;
}

Localization_t Linear_Guide_read_Localization()
{
	char FRAM_buffer[LOC_SERIAL_SIZE];
	FRAM_init();
	FRAM_read(LINEAR_GUIDE_INFOS, (uint8_t *) FRAM_buffer, LOC_SERIAL_SIZE);
	return Localization_init(LG_DISTANCE_MM_PER_PULSE, FRAM_buffer);
}

int8_t Linear_Guide_set_center(Linear_Guide_t *lg_ptr)
{
	Localization_t *loc_ptr = &lg_ptr->localization;
	if (!loc_ptr->is_triggered)
	{
		return LG_SET_CENTER_NOT_TRIGGERED;
	}
	printf("pulses:%d\r\n", loc_ptr->pulse_count);
	printf("center set at: %d mm!\r\n", loc_ptr->current_pos_mm);
	Localization_set_center(loc_ptr);
	Linear_Guide_safe_Localization(*loc_ptr);
	LED_blink(&lg_ptr->leds.center_pos_set);
	lg_ptr->localization.is_triggered = False;
	return LG_SET_CENTER_OK;
}

void Linear_Guide_set_startpos(Linear_Guide_t *lg_ptr)
{
	IO_Get_Measured_Value(LG_distance_sensor_ptr);
	Localization_set_startpos_abs(&lg_ptr->localization, LG_distance_sensor_ptr->measured_value);
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

static int8_t Linear_Guide_get_adjustment_mode(LG_sail_adjustment_mode_t last_mode, int16_t distance_to_center)
{
	if (distance_to_center == 0)
	{
		return last_mode;
	}
	return distance_to_center/abs(distance_to_center);
}

static uint16_t Linear_Guide_get_adjustment_range_mm(Linear_Guide_t lg, LG_sail_adjustment_mode_t adjustment_mode)
{
	Localization_t loc = lg.localization;
	return loc.end_pos_mm - adjustment_mode * loc.center_pos_mm;
}

static int8_t Linear_Guide_update_sail_adjustment_mode(Linear_Guide_t *lg_ptr)
{
	if (!lg_ptr->localization.is_localized)
	{
		return LG_NOT_LOCALIZED;
	}
	int32_t current_pos = lg_ptr->localization.current_pos_mm;
	int32_t center_pos = lg_ptr->localization.center_pos_mm;
	lg_ptr->sail_adjustment_mode = Linear_Guide_get_adjustment_mode(lg_ptr->sail_adjustment_mode, current_pos - center_pos);
	Linear_Guide_LED_set_sail_adjustment_mode(lg_ptr);
	return LG_ADJUSTMENT_MODE_UPDATED;
}

static void Linear_Guide_update_movement(Linear_Guide_t *lg_ptr, int8_t update_status)
{
	if (update_status == LG_UPDATE_EMERGENCY_SHUTDOWN)
	{
		Linear_Guide_move(lg_ptr, Loc_movement_stop, True);
		return;
	}
	Localization_t *loc_ptr = &lg_ptr->localization;
	if (loc_ptr->state >= Loc_state_3_approach_center)
	{
		boolean_t immediate = False;
		Loc_movement_t movement = Localization_get_next_movement(*loc_ptr, loc_ptr->desired_pos_mm);
		if (Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.front))
		{
			IO_Get_Measured_Value(LG_distance_sensor_ptr);
			Localization_set_startpos_abs(loc_ptr, LG_distance_sensor_ptr->measured_value);
			immediate = True;
		}
		else if (Linear_Guide_Endswitch_detected(&lg_ptr->endswitches.back))
		{
			Localization_set_endpos(loc_ptr);
			immediate = True;
		}
		if (immediate)
		{
			movement = Loc_movement_stop;
		}
		Linear_Guide_move(lg_ptr, movement, immediate);
	}
	int8_t speed_ramp_status = Motor_speed_ramp(&lg_ptr->motor);
	if (speed_ramp_status >= MOTOR_RAMP_NEXT_STEP)
	{
		Linear_Guide_calculate_break_path(lg_ptr);
		if (speed_ramp_status == MOTOR_RAMP_STOPPED)
		{
			loc_ptr->movement = Loc_movement_stop;
			Localization_progress_queue(loc_ptr);
		}
	}
}

static void Linear_Guide_calculate_break_path(Linear_Guide_t *lg_ptr)
{
	float dv = MOTOR_RAMP_STEP_RPM / 60.0F * LG_DISTANCE_MM_PER_ROTATION;
	float a = dv / (MOTOR_RAMP_STEP_MS / 1000.0F);
	float v_current = lg_ptr->motor.rpm_set_point / 60.0F * LG_DISTANCE_MM_PER_ROTATION;
	float tb = v_current / a;
	lg_ptr->localization.brake_path_mm = (v_current * tb - 0.5 * a * tb * tb)*LG_BRAKE_PATH_OFFSET_REL;
}

static int8_t Linear_Guide_error_handler(Linear_Guide_t *lg_ptr)
{
	int8_t update_status = LG_UPDATE_NORMAL;
	LG_error_state_t new_error_state = LG_error_state_0_normal;
	if (Linear_Guide_check_distance_fault(lg_ptr) == LG_FAULT_CHECK_POSITIVE)
	{
		new_error_state = LG_error_state_1_distance_fault;
	}
	if (Linear_Guide_check_wind_fault(lg_ptr) == LG_FAULT_CHECK_POSITIVE)
	{
		new_error_state = LG_error_state_2_wind_speed_fault;
	}
	if (Linear_Guide_check_motor_fault(lg_ptr) == LG_FAULT_CHECK_POSITIVE)
	{
		new_error_state = LG_error_state_3_motor_fault;
	}
	if (Linear_Guide_check_current_fault(lg_ptr) == LG_FAULT_CHECK_POSITIVE)
	{
		new_error_state = LG_error_state_4_current_fault;
	}

	if (new_error_state >= LG_error_state_3_motor_fault)
	{
		update_status = LG_UPDATE_EMERGENCY_SHUTDOWN;
	}
	else if(new_error_state == LG_error_state_2_wind_speed_fault)
	{
		Linear_Guide_set_desired_roll_trim_percentage(lg_ptr, 100 * LG_sail_adjustment_mode_roll);
	}
	lg_ptr->error_state = new_error_state;
	Linear_Guide_LED_set_error(lg_ptr);

	return update_status;
}

static int8_t Linear_Guide_check_distance_fault(Linear_Guide_t *lg_ptr)
{
	if (!lg_ptr->localization.is_localized)
	{
		return LG_FAULT_CHECK_SKIPPED;
	}
	IO_Get_Measured_Value(LG_distance_sensor_ptr);
	uint16_t measured_value = LG_distance_sensor_ptr->measured_value;
	Localization_parse_distance_sensor_value(&lg_ptr->localization, measured_value);
	Localization_update_position(&lg_ptr->localization);
	if (abs(lg_ptr->localization.current_measured_pos_mm - lg_ptr->localization.current_pos_mm) > (int)LG_linear_guide.max_distance_fault)
	{
		return LG_FAULT_CHECK_POSITIVE;
	}
	return LG_FAULT_CHECK_NEGATIVE;
}

static int8_t Linear_Guide_check_motor_fault(Linear_Guide_t *lg_ptr)
{
	if (Motor_error(&lg_ptr->motor) == True)
	{
		return LG_FAULT_CHECK_POSITIVE;
	}
	return LG_FAULT_CHECK_NEGATIVE;
}

static int8_t Linear_Guide_check_current_fault(Linear_Guide_t *lg_ptr)
{
	IO_Get_Measured_Value(LG_current_sensor_ptr);
	if (LG_current_sensor_ptr->measured_value > LG_CURRENT_FAULT_TOLERANCE_MA)
	{
		return LG_FAULT_CHECK_POSITIVE;
	}
	return LG_FAULT_CHECK_NEGATIVE;
}

static int8_t Linear_Guide_check_wind_fault(Linear_Guide_t *lg_ptr)
{
	if (lg_ptr->error_state == LG_error_state_2_wind_speed_fault)
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

Linear_Guide_t *LG_get_Linear_Guide(void)
{
  return &LG_linear_guide;
}

void Linear_Guide_set_error(LG_error_state_t error)
{
  LG_linear_guide.error_state = error;
}

LG_error_state_t Linear_Guide_get_error(void)
{
  return LG_linear_guide.error_state;
}

static uint8_t Linear_Guide_read_max_distance_delta(void)
{
  uint8_t max_delta = 0;
  FRAM_read(FRAM_MAX_DELTA, (uint8_t*)&max_delta, 1U);
  if((max_delta < 5) || (max_delta > 50))
  {
    return LG_STANDARD_MAX_DISTANCE_DELTA_MM;
  }

  return max_delta;
}

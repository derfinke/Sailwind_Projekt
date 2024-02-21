/**
 * \file Linear_Guide.h
 * @date 18 Jun 2023
 * @brief Interaction of all physical components of the linear guide system controlled both in manual and automatic mode
 */

#ifndef LINEAR_GUIDE_LINEAR_GUIDE_H_
#define LINEAR_GUIDE_LINEAR_GUIDE_H_

#include "LED.h"
#include "Motor.h"
#include "Endswitch.h"
#include "Localization.h"

#define LG_MOVEMENT_CHANGED 0
#define LG_MOVEMENT_RETAINED 1
#define LG_LOCALIZATION_SAFED 0
#define LG_LOCALIZATION_FAILED -1
#define LG_NOT_LOCALIZED 1
#define LG_ADJUSTMENT_MODE_UPDATED 0
#define LG_UPDATE_NORMAL 0
#define LG_UPDATE_EMERGENCY_SHUTDOWN -1
#define LG_SWITCH_OPERATING_MODE_DENIED -1
#define LG_SWITCH_OPERATING_MODE_OK 0


/* typedefs -----------------------------------------------------------*/
typedef enum {
	LG_error_state_0_normal,
	LG_error_state_1_distance_fault,
	LG_error_state_2_wind_speed_fault,
	LG_error_state_3_motor_fault,
	LG_error_state_4_current_fault,
} LG_error_state_t;
typedef enum {
	LG_operating_mode_manual,
	LG_operating_mode_automatic
} LG_operating_mode_t;

typedef enum {
	LG_sail_adjustment_mode_roll=-1,
	LG_sail_adjustment_mode_pitch=1
} LG_sail_adjustment_mode_t;

typedef struct {
	LED_t power;
	LED_t error;
	LED_t manual;
	LED_t automatic;
	LED_t roll;
	LED_t pitch;
	LED_t center_pos_set;
	TIM_HandleTypeDef *htim_blink_ptr;
} LG_LEDs_t;

typedef struct {
	Endswitch_t front;
	Endswitch_t back;
} LG_Endswitches_t;

typedef struct {
	LG_error_state_t error_state;
	LG_operating_mode_t operating_mode;
	LG_sail_adjustment_mode_t sail_adjustment_mode;
	Motor_t motor;
	Localization_t localization;
	LG_Endswitches_t endswitches;
	LG_LEDs_t leds;
	uint8_t max_distance_fault;
} Linear_Guide_t;


/* API function prototypes -----------------------------------------------*/
/**
 * @brief initialise the linear_guide object
 * @param hdac_ptr: dac handle object passed to motor member, that uses an analog signal for speed control
 * @retval none
 */
void Linear_Guide_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_blink_ptr);
/**
 * @brief update status variables of the linear guide (movement, position, sail adjustment mode, errors)
 * @param lg_ptr: linear_guide reference
 * @retval update_status
 */
int8_t Linear_Guide_update(Linear_Guide_t *lg_ptr);
/**
 * @brief switch operating mode to manual or automatic
 * @param lg_ptr: linear_guide reference
 * @param operating_mode
 * @retval operating_mode_switch_status
 */
int8_t Linear_Guide_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode);
/**
 * @brief count up or down pulse count depending on the movement direction (to be called in external interrupt callback from motor pulse signal)
 * @param lg_ptr: linear_guide reference
 * @retval none
 */
void Linear_Guide_callback_motor_pulse_capture(Linear_Guide_t *lg_ptr);
/**
 * @brief start / (stop) motor in given direction (activates motor speed ramp)
 * @param direction
 * @param immediate: if immediate stop is required
 * @retval movement_status: (if movement has changed or not)
 */
int8_t Linear_Guide_move(Linear_Guide_t *lg_ptr, Loc_movement_t direction, boolean_t immediate);
/**
 * @brief overrides desired position to move in given direction as long as stop is called
 * @param direction
 * @retval none
 */
void Linear_Guide_manual_move(Linear_Guide_t *lg_ptr, Loc_movement_t movement);
/**
 * @brief check if system is allowed to move depending on operating mode and Localization state
 * @param linear_guide
 * @retval permission_state
 */
boolean_t Linear_Guide_get_moving_permission(Linear_Guide_t lg);
/**
 * @brief converts the desired roll / pitch percentage to a target position in mm and safes it to the linear_guide reference
 * @param lg_ptr: linear_guide reference
 * @param percentage: relative position in given area (roll/pitch -> -/+ percentage)
 * @retval none
 */
void Linear_Guide_set_desired_roll_pitch_percentage(Linear_Guide_t *lg_ptr, int8_t percentage);
/**
 * @brief returns the roll / pitch percentage converted from the current position
 * @param lg: linear_guide
 * @retval roll_or_pitch_percentage
 */
int8_t Linear_Guide_get_current_roll_pitch_percentage(Linear_Guide_t lg);
/**
 * @brief returns True, if the linear_guide has reached the given endswitch
 * @param endswitch_ptr: enswitch reference (front / back)
 * @retval endswitch_active
 */
boolean_t Linear_Guide_Endswitch_detected(Endswitch_t *endswitch_ptr);
/**
 * @brief set normal speed of the motor (final value of speed ramp)
 * @param lg_ptr: linear_guide reference
 * @param speed_rpm: pass speed value in rpm
 * @retval none
 */
void Linear_Guide_change_speed_rpm(Linear_Guide_t *lg_ptr, uint16_t speed_rpm);
/**
 * @brief serialize and store essential localization values in FRAM
 * @param loc: Localization struct
 * @retval storage_status
 */
int8_t Linear_Guide_safe_Localization(Localization_t loc);
/**
 * @brief deserialize and restore localization struct from FRAM
 * @param none
 * @retval localization struct
 */
Localization_t Linear_Guide_read_Localization();

int8_t Linear_Guide_set_center(Linear_Guide_t *lg_ptr);
/**
 * @brief measure and set minimum absolute distance value from sensor
 * @param lg_ptr: linear_guide reference
 * @retval none
 */
void Linear_Guide_set_startpos(Linear_Guide_t *lg_ptr);

Linear_Guide_t *LG_get_Linear_Guide(void);

LG_error_state_t Linear_Guide_get_error(void);

void Linear_Guide_set_error(LG_error_state_t error);
#endif /* LINEAR_GUIDE_LINEAR_GUIDE_H_ */

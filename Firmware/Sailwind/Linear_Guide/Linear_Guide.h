/*
 * Linear_Guide.h
 *
 *  Created on: 18.06.2023
 *      Author: Bene
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
#define LG_NOT_LOCALIZED 1
#define LG_ADJUSTMENT_MODE_UPDATED 0


/* typedefs -----------------------------------------------------------*/
typedef enum {
	LG_operating_mode_manual,
	LG_operating_mode_automatic
} LG_operating_mode_t;

typedef enum {
	LG_sail_adjustment_mode_roll,
	LG_sail_adjustment_mode_trim
} LG_sail_adjustment_mode_t;

typedef struct {
	LED_t error;
	LED_t manual;
	LED_t automatic;
	LED_t roll;
	LED_t trim;
	LED_t center_pos_set;
} LG_LEDs_t;

typedef struct {
	Endswitch_t front;
	Endswitch_t back;
} LG_Endswitches_t;

typedef struct {
	LG_operating_mode_t operating_mode;
	LG_sail_adjustment_mode_t sail_adjustment_mode;
	Motor_t motor;
	Localization_t localization;
	LG_Endswitches_t endswitches;
	LG_LEDs_t leds;
} Linear_Guide_t;


/* API function prototypes -----------------------------------------------*/
Linear_Guide_t Linear_Guide_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel);
LG_LEDs_t Linear_Guide_LEDs_init(LG_operating_mode_t op_mode);
void Linear_Guide_update(Linear_Guide_t *lg_ptr);
void Linear_Guide_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode);
void Linear_Guide_callback_motor_pulse_capture(Linear_Guide_t *lg_ptr);
int8_t Linear_Guide_move(Linear_Guide_t *lg_ptr, Loc_movement_t direction);
void Linear_Guide_set_desired_roll_trim_percentage(Linear_Guide_t *lg_ptr, uint8_t percentage, LG_sail_adjustment_mode_t adjustment_mode);
uint8_t Linear_Guide_get_current_roll_trim_percentage(Linear_Guide_t lg);
boolean_t Linear_Guide_Endswitch_detected(Endswitch_t *endswitch_ptr);
void Linear_Guide_change_speed_mms(Linear_Guide_t *lg_ptr, uint16_t speed_mms);
uint16_t Linear_Guide_get_speed_mms(Linear_Guide_t *lg_ptr);
int8_t Linear_Guide_safe_Localization(Localization_t loc);
Localization_t Linear_Guide_read_Localization();


#endif /* LINEAR_GUIDE_LINEAR_GUIDE_H_ */

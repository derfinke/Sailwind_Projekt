/*
 * Linear_Guide.h
 *
 *  Created on: 18.06.2023
 *      Author: Bene
 */

#ifndef LINEAR_GUIDE_LINEAR_GUIDE_H_
#define LINEAR_GUIDE_LINEAR_GUIDE_H_

#include "LED/LED.h"
#include "Motor/Motor.h"
#include "Endswitch/Endswitch.h"
#include "Localization/Localization.h"

/* defines ------------------------------------------------------------*/
#define LG_DISTANCE_PER_ROTATION 5
#define LG_ENDSWITCH_COUNT 2
#define LG_LED_COUNT 6

/* typedefs -----------------------------------------------------------*/
typedef enum {
	LG_operating_mode_manual,
	LG_operating_mode_automatic
} LG_operating_mode_t;

typedef enum {
	LG_sail_adjustment_mode_rollung,
	LG_sail_adjustment_mode_trimmung
} LG_sail_adjustment_mode_t;

typedef enum {
	LG_LED_error,
	LG_LED_manual,
	LG_LED_automatic,
	LG_LED_rollung,
	LG_LED_trimmung,
	LG_LED_center_pos_set
} LG_LED_ID_t;

typedef enum {
	LG_Endswitch_front,
	LG_Endswitch_back
} LG_Endswitch_ID_t;

typedef struct {
	LG_operating_mode_t operating_mode;
	 LG_sail_adjustment_mode_t sail_adjustment_mode;
	Motor_t motor;
	Localization_t localization;
	Endswitch_t *endswitch_bar;
	LED_t *led_bar;
} Linear_Guide_t;


/* API function prototypes -----------------------------------------------*/
Linear_Guide_t Linear_Guide_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr, uint32_t htim_channel, HAL_TIM_ActiveChannel htim_active_channel);
void LG_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode);
void LG_callback_motor_pulse_capture(Linear_Guide_t *lg_ptr);
void LG_move(Linear_Guide_t *lg_ptr, Loc_movement_t direction);
boolean_t LG_get_manual_moving_permission(Linear_Guide_t linear_guide);
void LG_set_center(Linear_Guide_t *lg_ptr);
void LG_set_endpos(Linear_Guide_t *lg_ptr);
boolean_t LG_Endswitch_detected(Linear_Guide_t *lg_ptr, LG_Endswitch_ID_t endswitch_ID);

/* private function test prototypes ------------------ -------------------*/
void LG_Test_LED_set_operating_mode(Linear_Guide_t *lg_ptr, LG_operating_mode_t operating_mode);
void LG_Test_LED_set_sail_adjustment_mode(Linear_Guide_t *lg_ptr, LG_sail_adjustment_mode_t sail_adjustment_mode);

#endif /* LINEAR_GUIDE_LINEAR_GUIDE_H_ */

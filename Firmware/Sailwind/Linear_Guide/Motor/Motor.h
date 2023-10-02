/*
 * motor_API.h
 *
 *  Created on: 20.04.2023
 *      Author: Bene
 */

#ifndef MOTOR_MOTOR_H_
#define MOTOR_MOTOR_H_

#include "IO.h"

/* defines ------------------------------------------------------------*/
#define MOTOR_PULSE_PER_ROTATION 12 //public
#define MOTOR_RPM_SPEED_1 75
#define MOTOR_RPM_SPEED_2 150
#define MOTOR_DIRECTION_CCW GPIO_PIN_SET
#define MOTOR_DIRECTION_CW GPIO_PIN_RESET
#define MOTOR_IN_COUNT 4
#define MOTOR_RAMP_STEP_MS 13
#define MOTOR_RAMP_STEP_RPM 10
#define MOTOR_RAMP_INACTIVE 1
#define MOTOR_RAMP_WAIT 2
#define MOTOR_RAMP_NEXT_STEP 3
#define MOTOR_RAMP_NORMAL_SPEED 4
#define MOTOR_RAMP_STOPPED 5


/* typedefs -----------------------------------------------------------*/
typedef enum {
	Motor_function_stop,
	Motor_function_cw_rotation,
	Motor_function_ccw_rotation,
	Motor_function_stop_holding_torque,
	Motor_function_velocity_setting,
	Motor_function_current_setting,
	Motor_function_speed1,
	Motor_function_speed2
} Motor_function_t;

typedef IO_digitalPin_t Motor_INs_t[MOTOR_IN_COUNT];


typedef struct {
	Motor_function_t current_function;
	Motor_INs_t INs;
	IO_analogActuator_t AIN_set_rpm;
	IO_digitalPin_t OUT2_error;
	IO_digitalPin_t OUT3_rot_dir;
	uint16_t normal_rpm;
	uint16_t rpm_set_point;
	uint16_t ramp_final_rpm;
	uint32_t ramp_last_step_ms;
	boolean_t ramp_activated;
} Motor_t;


/* API function prototypes -----------------------------------------------*/
Motor_t Motor_init(DAC_HandleTypeDef *hdac_ptr);
void Motor_start_moving(Motor_t *motor_ptr, Motor_function_t function);
void Motor_stop_moving(Motor_t *motor_ptr, boolean_t immediate);
int8_t Motor_speed_ramp(Motor_t *motor_ptr);
void Motor_set_function(Motor_t *motor_ptr, Motor_function_t function);
void Motor_set_rpm(Motor_t *motor_ptr, uint16_t rpm_value);
boolean_t Motor_error(Motor_t *motor_ptr);
void Motor_teach_speed(Motor_t *motor_ptr, Motor_function_t speed, uint16_t rpm_value, uint32_t tolerance);
boolean_t Motor_is_currently_braking(Motor_t motor);


#endif /* MOTOR_MOTOR_H_ */

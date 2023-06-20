/*
 * linear_guide_api.c
 *
 *  Created on: 18.06.2023
 *      Author: Bene
 */

#include "linear_guide_api.h"

/* private function prototypes -----------------------------------------------*/
static linear_guide_calibration_t calibration_init();
static linear_guide_endschalter_t endschalter_init();
static void calibrate_set_endpoints(linear_guide_calibration_t *calibration_ptr);
static boolean_t endschalter_detected(IO_digitalPin_t *endschalter_ptr);
static void update_current_position(linear_guide_calibration_t *calibration_ptr);
static int32_t convert_pulse_count_to_distance(int32_t pulse_count);

/* API function definitions --------------------------------------------------*/
Linear_guide_t linear_guide_init(DAC_HandleTypeDef *hdac_ptr, TIM_HandleTypeDef *htim_ptr)
{
	Linear_guide_t linear_guide = {
			.operating_mode = IO_operating_mode_manual,
			.motor = motor_init(hdac_ptr, htim_ptr),
			.calibration = calibration_init(),
			.endschalter = endschalter_init()
	};
	return linear_guide;
}

void linear_guide_set_operating_mode(Linear_guide_t *linear_guide_ptr, IO_operating_mode_t operating_mode)
{
	boolean_t set_automatic = operating_mode == IO_operating_mode_automatic;
	boolean_t set_manual = operating_mode == IO_operating_mode_manual;
	boolean_t is_calibrated = linear_guide_ptr->calibration.is_calibrated;
	if ((set_automatic && is_calibrated) || set_manual)
	{
		linear_guide_ptr->operating_mode = operating_mode;
	}
}

/* void linear_guide_calibrate_state_machine_set_endpoints(Linear_guide_t *linear_guide_ptr)
 *  Description:
 *   -
 */
void linear_guide_calibrate_state_machine_set_endpoints(Linear_guide_t *linear_guide_ptr)
{
	linear_guide_calibration_t *calibration_ptr = &linear_guide_ptr->calibration;
	Motor_t *motor_ptr = &linear_guide_ptr->motor;
	linear_guide_endschalter_t *endschalter_ptr = &linear_guide_ptr->endschalter;
	if (calibration_ptr->calibrate_button_state == linear_guide_calibrate_button_state_1_approach_borders)
	{
		switch(calibration_ptr->approach_borders_state)
		{
			case linear_guide_approach_borders_state_0_init:
				motor_start_moving(motor_ptr, motor_moving_state_linkslauf);
				calibration_ptr->approach_borders_state = linear_guide_approach_borders_state_1_approach_vorne;
				break;
			case linear_guide_approach_borders_state_1_approach_vorne:
				if (endschalter_detected(&endschalter_ptr->vorne))
				{
					motor_start_rpm_measurement(motor_ptr);
					motor_start_moving(motor_ptr, motor_moving_state_rechtslauf);
					calibration_ptr->approach_borders_state = linear_guide_approach_borders_state_2_approach_hinten;
				}
				break;
			case linear_guide_approach_borders_state_2_approach_hinten:
				if (endschalter_detected(&linear_guide_ptr->endschalter.hinten))
				{
					motor_start_moving(motor_ptr, motor_moving_state_linkslauf);
					calibrate_set_endpoints(calibration_ptr);
					calibration_ptr->approach_borders_state = linear_guide_approach_borders_state_3_approach_center;
				}
				break;
			case linear_guide_approach_borders_state_3_approach_center:
				if (calibration_ptr->current_pos_mm == 0)
				{
					motor_stop_moving(motor_ptr);
					calibration_ptr->calibrate_button_state = linear_guide_calibrate_button_state_2_set_center_pos;
				}
		}
	}
}

/* void linear_guide_callback_get_rpm(Linear_guide_t *linear_guide_ptr, TIM_HandleTypeDef *htim_ptr)
 *  Description:
 *   -
 */
void linear_guide_callback_get_rpm(Linear_guide_t *linear_guide_ptr, TIM_HandleTypeDef *htim_ptr)
{
	linear_guide_calibration_t *calibration_ptr = &linear_guide_ptr->calibration;
	Motor_t *motor_ptr = &linear_guide_ptr->motor;
	RPM_Measurement_t *drehzahl_messung_ptr = &motor_ptr->OUT1_Drehzahl_Messung;
	if (htim_ptr!=drehzahl_messung_ptr->htim_ptr)
	{
		return;
	}
	drehzahl_messung_ptr->timer_cycle_count++;
	if (!IO_digitalRead_rising_edge(&drehzahl_messung_ptr->puls))
	{
		return;
	}
	motor_convert_timeStep_to_rpm(drehzahl_messung_ptr);
	if (calibration_ptr->approach_borders_state < linear_guide_approach_borders_state_2_approach_hinten)
	{
		return;
	}
	switch(motor_ptr->moving_state)
	{
		case motor_moving_state_rechtslauf:
			calibration_ptr->current_pos_pulse_count++; break;
		case motor_moving_state_linkslauf:
			calibration_ptr->current_pos_pulse_count--; break;
		default:
			;
	}
	if (calibration_ptr->approach_borders_state >= linear_guide_approach_borders_state_3_approach_center)
	{
		update_current_position(calibration_ptr);
	}
}

/* void calibrate_set_center(Motor_t *motor_ptr)
 *  Description:
 *   -
 */
void linear_guide_set_center(linear_guide_calibration_t *calibration_ptr)
{
	calibration_ptr->center_pos_mm = calibration_ptr->current_pos_mm;
}

/* boolean_t linear_guide_get_manual_moving_permission(Linear_guide_t linear_guide)
 *  Description:
 *   -
 */
boolean_t linear_guide_get_manual_moving_permission(Linear_guide_t linear_guide)
{
	return
			linear_guide.operating_mode == IO_operating_mode_manual
			&&
			(
				linear_guide.calibration.calibrate_button_state == linear_guide_calibrate_button_state_2_set_center_pos
				||
				linear_guide.calibration.calibrate_button_state == linear_guide_calibrate_button_state_0_init
			);
}

/* private function definitions -----------------------------------------------*/
static linear_guide_calibration_t calibration_init()
{
	linear_guide_calibration_t calibration =
	{
			.calibrate_button_state = linear_guide_calibrate_button_state_0_init,
			.approach_borders_state = linear_guide_approach_borders_state_0_init,
			.current_pos_pulse_count = 0,
			.is_calibrated = False
	};
	return calibration;
}

static linear_guide_endschalter_t endschalter_init()
{
	linear_guide_endschalter_t endschalter = {
			.hinten = IO_digitalPin_init(GPIOB, Endschalter_Hinten_Pin, GPIO_PIN_RESET),
			.vorne = IO_digitalPin_init(GPIOB, Endschalter_Vorne_Pin, GPIO_PIN_RESET),
	};
	return endschalter;
}

/* static boolean_t endschalter_detected(IO_digitalPin_t *endschalter_ptr)
 *  Description:
 *   -
 */
static boolean_t endschalter_detected(IO_digitalPin_t *endschalter_ptr)
{
	return (boolean_t) IO_digitalRead(endschalter_ptr);
}

/* static void calibrate_set_endpoints(motor_calibration_t *calibration_ptr)
 *  Description:
 *   -
 */
static void calibrate_set_endpoints(linear_guide_calibration_t *calibration_ptr)
{
	calibration_ptr->end_pos_mm = convert_pulse_count_to_distance(calibration_ptr->current_pos_pulse_count)/2;
	calibration_ptr->current_pos_mm = calibration_ptr->end_pos_mm;
}

/* static void update_current_position(linear_guide_calibration_t *calibration_ptr)
 *  Description:
 *   -
 */
static void update_current_position(linear_guide_calibration_t *calibration_ptr)
{
	calibration_ptr->current_pos_mm = convert_pulse_count_to_distance(calibration_ptr->current_pos_pulse_count) - calibration_ptr->end_pos_mm;
}

/* static int32_t convert_pulse_count_to_distance(int32_t pulse_count)
 *  Description:
 *   -
 */
static int32_t convert_pulse_count_to_distance(int32_t pulse_count)
{
	return (uint32_t) (pulse_count / (float)(MOTOR_PULSE_PER_ROTATION) * LINEAR_GUIDE_DISTANCE_PER_ROTATION);
}

/* Timer Callback implementation for rpm measurement --------------------------*/

/* void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim_ptr)
 *  Description:
 *   -
 */
/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim_ptr)
{
	linear_guide_callback_get_rpm(&linear_guide, htim_ptr);
}
*/


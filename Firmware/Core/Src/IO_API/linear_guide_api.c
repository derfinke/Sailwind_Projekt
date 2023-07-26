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
static void callback_update_pulse_count(linear_guide_calibration_t *calibration_ptr, motor_moving_state_t motor_moving_state);
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

/* void linear_guide_calibrate_state_machine_set_endpoints(Linear_guide_t *linear_guide_ptr)
 *  Description:
 *   - set operating mode depending on current state of the system
 *   - operating mode can always be switched to manual
 *   - operating mode can only be switched to automatic, if the linear guide is calibrated
 */
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

/* void linear_guide_calibrate_state_machine_approach_borders(Linear_guide_t *linear_guide_ptr)
 *  Description:
 *   - state machine to be called in main loop
 *   - controls the process of approaching both end points and finally the calculated center
 *   - state machine only runs, after first press of calibration button
 *   - from initial state 0 the motor starts moving to the first end point and state is set to 1 (approach front)
 *   - state 1 is active until the front end switch is detected (high)
 *   		-> set state to 2 (approach back)
 *   		-> start motor moving to other end point
 *   		-> start rpm measurement to calculate linear guide range
 *   - state 2 is active until the back end switch is detected (high)
 *   		-> set state to 3 (approach center)
 *   		-> calculate range of linear guide
 *   		-> start motor moving to the calculated center
 *   - state 3 waits until center is reached
 *   		-> motor is stopped
 *   		-> state is set to state 4 (set center)
 */
void linear_guide_calibrate_state_machine_approach_borders(Linear_guide_t *linear_guide_ptr)
{
	linear_guide_calibration_t *calibration_ptr = &linear_guide_ptr->calibration;
	Motor_t *motor_ptr = &linear_guide_ptr->motor;
	linear_guide_endschalter_t *endschalter_ptr = &linear_guide_ptr->endschalter;
	switch(calibration_ptr->state)
	{
		case linear_guide_calibration_state_1_approach_vorne:
			if (endschalter_detected(&endschalter_ptr->vorne))
			{
				printf("new state approach hinten\r\n");
				motor_start_rpm_measurement(motor_ptr);
				motor_start_moving(motor_ptr, motor_moving_state_rechtslauf);
				calibration_ptr->state = linear_guide_calibration_state_2_approach_hinten;
			}
			break;
		case linear_guide_calibration_state_2_approach_hinten:
			if (endschalter_detected(&endschalter_ptr->hinten))
			{
				printf("new state approach center\r\n");
				motor_start_moving(motor_ptr, motor_moving_state_linkslauf);
				calibrate_set_endpoints(calibration_ptr);
				calibration_ptr->state = linear_guide_calibration_state_3_approach_center;
			}
			break;
		case linear_guide_calibration_state_3_approach_center:
			if (calibration_ptr->current_pos_mm == 0)
			{
				printf("new state set center\r\n");
				motor_stop_moving(motor_ptr);
				calibration_ptr->state = linear_guide_calibration_state_4_set_center_pos;
			}
			break;
		default:
			break;
	}
}

void linear_guide_callback_motor_pulse_capture(Linear_guide_t *linear_guide_ptr, TIM_HandleTypeDef *htim_ptr)
{
	if (motor_callback_measure_rpm(&linear_guide_ptr->motor, htim_ptr))
	{
		callback_update_pulse_count(&linear_guide_ptr->calibration, linear_guide_ptr->motor.moving_state);
	}
}

/* void linear_guide_set_center(linear_guide_calibration_t *calibration_ptr)
 *  Description:
 *   - save center position to finish and confirm calibration
 */
void linear_guide_set_center(linear_guide_calibration_t *calibration_ptr)
{
	calibration_ptr->center_pos_mm = calibration_ptr->current_pos_mm;
}

/* boolean_t linear_guide_get_manual_moving_permission(Linear_guide_t linear_guide)
 *  Description:
 *   - return True, if operating mode is manual and motor is not currently moving automatically due to calibration process
 */
boolean_t linear_guide_get_manual_moving_permission(Linear_guide_t linear_guide)
{
	return
			linear_guide.operating_mode == IO_operating_mode_manual
			&&
			(
				linear_guide.calibration.state == linear_guide_calibration_state_4_set_center_pos
				||
				linear_guide.calibration.state == linear_guide_calibration_state_0_init
			);
}

/* private function definitions -----------------------------------------------*/
static linear_guide_calibration_t calibration_init()
{
	linear_guide_calibration_t calibration =
	{
			.state = linear_guide_calibration_state_0_init,
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
 *   - return True, if the given end switch is reached by the linear guide
 */
static boolean_t endschalter_detected(IO_digitalPin_t *endschalter_ptr)
{
	return (boolean_t) IO_digitalRead(endschalter_ptr);
}

/* static void calibrate_set_endpoints(linear_guide_calibration_t *calibration_ptr)
 *  Description:
 *   - called, when second end-point is reached during calibration process
 *   - saves the measured range of the linear guide from the center in mm
 *   - set the current position for the first time possible
 */
static void calibrate_set_endpoints(linear_guide_calibration_t *calibration_ptr)
{
	calibration_ptr->end_pos_mm = convert_pulse_count_to_distance(calibration_ptr->current_pos_pulse_count)/2;
	calibration_ptr->current_pos_mm = calibration_ptr->end_pos_mm;
}

static void callback_update_pulse_count(linear_guide_calibration_t *calibration_ptr, motor_moving_state_t motor_moving_state)
{
	if (calibration_ptr->state < linear_guide_calibration_state_2_approach_hinten)
	{
		return;
	}
	switch(motor_moving_state)
	{
		case motor_moving_state_rechtslauf:
			calibration_ptr->current_pos_pulse_count++; break;
		case motor_moving_state_linkslauf:
			calibration_ptr->current_pos_pulse_count--; break;
		case motor_moving_state_aus:
			break;
		}
	if (calibration_ptr->state >= linear_guide_calibration_state_3_approach_center)
	{
		update_current_position(calibration_ptr);
	}
}

/* static void update_current_position(linear_guide_calibration_t *calibration_ptr)
 *  Description:
 *   - convert the pulse count of the motor to a distance and center it
 *   - save the result to the calibration reference
 */
static void update_current_position(linear_guide_calibration_t *calibration_ptr)
{
	calibration_ptr->current_pos_mm = convert_pulse_count_to_distance(calibration_ptr->current_pos_pulse_count) - calibration_ptr->end_pos_mm;
}

/* static int32_t convert_pulse_count_to_distance(int32_t pulse_count)
 *  Description:
 *   - convert measured pulse count of the motor to a distance in mm, using pulse per rotation constant (12) and distance per rotation constant (5[mm])
 */
static int32_t convert_pulse_count_to_distance(int32_t pulse_count)
{
	return (uint32_t) (pulse_count / (float)(MOTOR_PULSE_PER_ROTATION) * LINEAR_GUIDE_DISTANCE_PER_ROTATION);
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
	linear_guide_callback_motor_pulse_capture(&linear_guide, htim_ptr);
}
*/


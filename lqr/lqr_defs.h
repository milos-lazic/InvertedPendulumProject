/*
 * lqr_defs.h
 *
 *  Created on: Mar 31, 2018
 *      Author: Milos Lazic
 */

#ifndef LQR_LQR_DEFS_H_
#define LQR_LQR_DEFS_H_


#include <stdlib.h>
#include <stdint.h>



/* Name: LQR_ctrl_blk_type
 *
 * Description: LQR controller control block data type
 *
 * Members: num_states - field contains number of states being monitored
 *                       by the LQR controller
 *          K          - pointer to array containing state feedback gains
 *                       (length of array must match value stored in num_states
 *                       field)
 *          Nbar       - field contains the pre-compensation constant to multiply
 *                       the reference input to the controller (for steady-state
 *                       error tracking)
 *          sp         - field contains the controller reference (set point)
 *
 * Notes:
 *
 */
struct LQR_ctrl_blk_type
{
	uint32_t num_states;
	const float *K; // controller state feedback gain vector
	const float Nbar; // input precompensation coefficient (for steady-state error elimination)
	float sp; // system input (set point);
};


/* Name: LQR_pt_type
 *
 * Description: Data type used to represnet individual points
 *              in mapping arrays (i.e. conversion from voltage
 *              input to power)
 *
 * Members: x - x coordinate
 *          y - y coordinate
 *
 * Notes:
 *
 */
struct LQR_pt_type
{
	float x;
	float y;
};

#endif /* LQR_LQR_DEFS_H_ */

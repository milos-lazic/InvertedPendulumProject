/*
 * lqr_balance.c
 *
 *  Created on: Mar 31, 2018
 *      Author: Milos Lazic
 */

#include "lqr_defs.h"
#include "lqr_proto.h"
#include "../sys/device/device.h"
#include <inc/tm4c123gh6pm.h>


#define MAX_STATE 5              /* number of feedback states */

#define SHAFT_RADIUS 0.0069358   /* shaft radius (m) */


/*
 * LQR_Balance_CtrlIn calculates the voltage that should be applied
 * across motor terminals based on the current state measurements
 *
 * VOLTAGE_TO_POWER_MAP is a lookup table used to convert between
 * the caluclated voltage and the power (%) that should be output
 * to the motor. The corresponding power is decoded by the ESC driver
 * which calculates the appropriate duty cycle and direction signal
 */
static const struct LQR_pt_type VOLTAGE_TO_POWER_MAP[] =
{
		/* voltage, input power to motor (%) */
		{ -12.0,  -100.0 },
		{  12.0,   100.0 },
};
#define VOLTAGE_TO_POWER_MAP_LEN (sizeof(VOLTAGE_TO_POWER_MAP)/sizeof(VOLTAGE_TO_POWER_MAP[0]))



// controller state feedback gains
static const float K_vec[MAX_STATE] = {0.0029, 20, 20.9179, -65.3129, -8};


// inverted pendulum LQR controller control block
static struct LQR_ctrl_blk_type gcb =
{
		.num_states = MAX_STATE,
		.K = K_vec,
		.Nbar = 20,
		.sp = 0, // intial set point (x position)
};


/*
 * Name: LQR_Balance_CtrlVIn
 *
 * Descr: Subroutine of inverted pendulum balancing algorithm. Reads
 *        current state values and computes controller output (input to plant)
 *        through state feedback gain vector.
 *
 * Args:     none
 *
 * Return:   Controller output (input to plant)
 *
 * Notes: The input to the system being controlled is voltage
 *        (pulse width modulated) accross the motor terminals
 *
 */
static float LQR_Balance_CtrlVIn( void)
{
	float x_vec[MAX_STATE];
	float val;

	/* read state variable values
	 *
	 * x_vec(0) = i(t) [armature current] // NOTE: cannot be measured with available hardware; assume 0 for now...
	 * x_vec(1) = x(t) [cart position]
	 * x_vec(2) = x'(t) [cart velocity]
	 * x_vec(3) = th(t) [pendulum angle]
	 * x_vec(4) = th'(t) [pendulum angular velocity]
	 */

	/* get current value (assumed to be negligible based on system dynamics analysis) */
	x_vec[0] = 0;

	/* get position of the cart along the track */
	(void) dev_ioctl(eDEV_QEI1, eQEI_IOCTL_R_POS_RAD, &val);
	x_vec[1] = val * (float)SHAFT_RADIUS;
	/* get velocity of the cart along the track */
	(void) dev_ioctl(eDEV_QEI1, eQEI_IOCTL_R_VEL_RAD, &val);
	x_vec[2] = val * (float)SHAFT_RADIUS;
	/* get angular position of the pendulum */
	(void) dev_ioctl(eDEV_QEI0, eQEI_IOCTL_R_POS_RAD, &x_vec[3]);
	/* get angular velocity of the pendulum */
	(void) dev_ioctl(eDEV_QEI0, eQEI_IOCTL_R_VEL_RAD, &x_vec[4]);


	/* setpoint (XPOS * Nbar) - Kx */
	/* returns a required input voltage */
	return ((gcb.sp * gcb.Nbar) - LQR_dot_f( x_vec, gcb.K, gcb.num_states));
}


/*
 * Name: LQR_Balance_CtrlRun
 *
 * Descr: Main inverted pendulum controller wrapper function; computes
 *        required controller output (input to plant) and calls motor
 *        driver (DEV_ESC) routine to set required power level (driver
 *        handles power level to duty cycle conversion)
 *
 * Args:     val - new value of set point
 *
 * Return:   none
 *
 * Notes: In this context, the set point represents the physical
 *        position (x) of the cart along the tracks (relative to
 *        position at QEI initialization.
 *
 */
void LQR_Balance_CtrlRun( void)
{
	float v_in, power_in;

	// calculate the voltage input to the controller
	v_in = LQR_Balance_CtrlVIn();

	// convert voltage input to power input (%)
	power_in = LQR_linmap( v_in, VOLTAGE_TO_POWER_MAP, VOLTAGE_TO_POWER_MAP_LEN);

#if 0 // DEBUGGING: turn on GREEN LED if power to actuator equals
	  //            or exceeds 100 percent (absolute value)
	if ( power_in >= (float)100.0 || power_in <= (float)-100.0)
		GPIO_PORTF_DATA_R |= 0x00000008;
	else
		GPIO_PORTF_DATA_R &= ~(0x00000008);
#endif

	dev_ioctl(eDEV_ESC0, eESC_IOCTL_SET_POWER, (int)power_in);
}



/*
 * Name: LQR_Balance_SetPoint
 *
 * Descr: Routine to adjust controller reference (set point)
 *
 * Args:     val - new value of set point
 *
 * Return:   none
 *
 * Notes: In this context, the set point represents the physical
 *        position (x) of the cart along the tracks (relative to
 *        position at QEI initialization.
 *
 */
void LQR_Balance_SetPoint( float val)
{
	gcb.sp = val;
}


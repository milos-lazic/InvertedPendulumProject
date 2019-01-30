/*
 * fsm.c
 *
 *  Created on: Mar 11, 2018
 *      Author: vlazic
 */


#include "inc/tm4c123gh6pm.h"
#include "fsm.h"
#include "../sys/device/device.h"


/* macros */
#define PI    (3.14159265359)   /* pi */
#define DIA   (60.4)            /* gear diamter (mm) */
#define PPR   (2400)            /* encoder pulses per revolution */


volatile int32_t sv_x;        /* state variable x (position along track) */
volatile int32_t sv_xdot;     /* state variable xdot (velocity along track) */
volatile int32_t sv_theta;    /* state variable theta (angular position of pendulum) */
volatile int32_t sv_thetadot; /* state variable thetadot (angular veolcity of pendulum) */



static void init_state_function(void)
{
	int32_t sval = 0;

	// enable FPU
	NVIC_CPAC_R |= (0xF << 20);

	// call proximity sensor read subroutine
	sval = dev_ioctl(eDEV_PRS0, 0); // returns cart distance in mm
	/* MLAZIC_TBD: error checking required */
#if 0
	if ( sval > PRS_READMAX || sval < 0)
	{
		// handle error
	}
#endif

	// initialize global state variable for track position (sv_x)
	sv_x = (int32_t)((float)((sval/(PI*DIA))*PPR));
	// initialize QEI_1_POS with sv_x
	dev_ioctl(eDEV_QEI1, 1, sv_x);

}



static fsm_event_t state_init_read_event(void)
{
	/* MLAZIC_TBD: start systick timer here? */

	return eFSM_EVENT_DONE;
}


/*
 * Name: state_calib_read_event
 * Descr: event detecting subroutine called while FSM is
 *        in STATE_CALIB (calibration state)
 * Args:     none
 * Return:   event type enumerator (from fsm_event_t enumeration)
 *           of highest-priority event detected
 * Notes:
 */
static fsm_event_t state_calib_read_event(void)
{
	/* MLAZIC_TBD: not yet implemented */



	return eFSM_EVENT_NONE;
}


static fsm_event_t state_swingup_read_event(void)
{
	/* MLAZIC_TBD: not yet implemented */
	return eFSM_EVENT_NONE;
}


static fsm_event_t state_balance_read_event(void)
{
	/* MLAZIC_TBD: not yet implemented */
	return eFSM_EVENT_NONE;
}


static fsm_event_t state_emgbrake_read_event(void)
{
	/* MLAZIC_TBD: not yet implemented */
	return eFSM_EVENT_NONE;
}



struct fsm_state_struct FSM[eFSM_STATE_MAX] = {
                                                                                        /* eFSM_EVENT_NONE     eFSM_EVENT_DONE   eFSM_EVENT_FAIL  eFSM_EVENT_COLLISIONWARN*/
		/* eFSM_STATE_INIT */       { init_state_function, state_init_read_event,     {      eFSM_STATE_INV,   eFSM_STATE_CALIB,   eFSM_STATE_INV,      eFSM_STATE_INV} },
		/* eFSM_STATE_CALIB */      { NULL, state_calib_read_event,                   {    eFSM_STATE_CALIB, eFSM_STATE_SWINGUP,   eFSM_STATE_INV,      eFSM_STATE_INV} },
		/* eFSM_STATE_SWINGUP */    { NULL, state_swingup_read_event,                 {  eFSM_STATE_SWINGUP, eFSM_STATE_BALANCE,   eFSM_STATE_INV, eFSM_STATE_EMGBRAKE} },
		/* eFSM_STATE_BALANCE */    { NULL, state_balance_read_event,                 {  eFSM_STATE_BALANCE,     eFSM_STATE_INV, eFSM_STATE_CALIB, eFSM_STATE_EMGBRAKE} },
		/* eFSM_STATE_EMGBRAKE */   { NULL, state_emgbrake_read_event,                { eFSM_STATE_EMGBRAKE,   eFSM_STATE_CALIB,   eFSM_STATE_INV,      eFSM_STATE_INV} },
};





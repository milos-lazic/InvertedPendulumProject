/*
 * fsm_defs.h
 *
 *  Created on: Mar 8, 2018
 *      Author: vlazic
 */

#ifndef FSM_FSM_DEFS_H_
#define FSM_FSM_DEFS_H_

#define MAX_INPUT 10 /* MLAZIC_TBD: revise when state graph is defined */

typedef enum {
	eFSM_STATE_INIT = 0,
	eFSM_STATE_CALIB,
	eFSM_STATE_SWINGUP,
	eFSM_STATE_BALANCE,
	eFSM_STATE_EMGBRAKE,
	eFSM_STATE_INV,
	eFSM_STATE_MAX,
} fsm_state_t;


typedef enum {
	eFSM_EVENT_NONE = 0,
	eFSM_EVENT_DONE,
	eFSM_EVENT_FAIL,
	eFSM_EVENT_COLLISIONWARN,
	eFSM_EVENT_MAX,
} fsm_event_t;


struct fsm_state_struct
{
	void (*state_function)(void);
	fsm_event_t (*state_event_read)(void);
	fsm_state_t state_transition_map[eFSM_STATE_MAX];

};


#endif /* FSM_FSM_DEFS_H_ */

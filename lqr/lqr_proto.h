/*
 * lqr_proto.h
 *
 *  Created on: Mar 31, 2018
 *      Author: Milos Lazic
 */

#ifndef LQR_LQR_PROTO_H_
#define LQR_LQR_PROTO_H_


#include <stdint.h>
#include "lqr_defs.h"


/* module scope routines */
extern float LQR_linmap( float input, const struct LQR_pt_type *p_map, const size_t map_len);
extern float LQR_dot_f( const float *v1,  const float *v2, const uint32_t size);

/* global scope routines */
extern void LQR_Balance_SetPoint( float val);
extern void LQR_Balance_CtrlRun( void);


#endif /* LQR_LQR_PROTO_H_ */

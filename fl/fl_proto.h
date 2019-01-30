/*
 * fl_proto.h
 *
 *  Created on: Feb 25, 2018
 *      Author: vlazic
 */

#ifndef FL_FL_PROTO_H_
#define FL_FL_PROTO_H_

#include <stdint.h>
#include "fl_defs.h"


/* fl_utils.c function protorypes */
extern int32_t fl_AND( int32_t i1, int32_t i2);
extern int32_t fl_OR( int32_t i1, int32_t i2);
extern int32_t fl_linmap( int32_t input, const struct fl_mf_pt_type *p_map, const size_t map_len);
extern int32_t fl_calculate_membership( int32_t input, const struct fl_mf_pt_type *mf_map, const size_t map_len);
extern int32_t fl_mf_centroid( const struct fl_mf_pt_type *mf_map, const size_t map_len);




#endif /* FL_FL_PROTO_H_ */

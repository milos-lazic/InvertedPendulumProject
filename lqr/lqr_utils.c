/*
 * lqr_utils.c
 *
 *  Created on: Mar 31, 2018
 *      Author: Milos Lazic
 */

#include "lqr_defs.h"
#include "lqr_proto.h"


/*
 * Name: LQR_linmap
 *
 * Descr: Routine to evaluate a piece-wise (array-mapped) function
 *        by linear interpolation
 *
 * Args:     input - argument to function
 *           p_map - pointer to function mapping (lookup table)
 *           map_len - length of function mapping
 *
 * Return:   Function value at 'input'
 *
 * Notes:
 *
 */
float LQR_linmap( float input, const struct LQR_pt_type *p_map, const size_t map_len)
{
	int32_t i = 0;

	while( i < map_len)
	{
		if ( input < p_map[i].x)
			break;
		i++;
	}

	if ( i == 0)
		return p_map[i].y;
	else if ( i == map_len)
		return p_map[i-1].y;
	else
		return ( p_map[i-1].y +(((input - p_map[i-1].x)*(p_map[i].y - p_map[i-1].y))/(p_map[i].x - p_map[i-1].x)));
}




/*
 * Name: LQR_dot_f
 *
 * Descr: Routine to compute the dot product of two equal-size
 *        vectors (vectors are represented as arrays of type float)
 *
 * Args:     v1 - field contains reference to vector (array) 1
 *           v2 - field contains reference to vector (array) 2
 *           size - length of vectors
 *
 * Return:   dot product of v1 and v2
 *
 * Notes:
 *
 */
float LQR_dot_f( const float *v1,  const float *v2, const uint32_t size)
{
	float res = 0.0;
	int i = 0;

	for ( ; i < size; i++)
	{
		res += v1[i] * v2[i];
	}

	return res;
}



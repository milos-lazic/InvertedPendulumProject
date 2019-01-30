/*
 * fl_utils.c
 *
 *  Created on: Feb 25, 2018
 *      Author: Milos Lazic
 */

#include "fl_defs.h"
#include "fl_proto.h"


/*
 * Name: fl_AND
 * Descr: fuzzy logic AND operator; returns the minimum
 *        of two operands
 * Args:     i1 - operand 1
 *           i2 - operand 2
 * Return:   minimum of i1 and i2
 * Notes:
 */
int32_t fl_AND( int32_t i1, int32_t i2)
{
	return ( i1 < i2) ? i1 : i2;
}


/*
 * Name: fl_OR
 * Descr: fuzzy logic OR operator; returns the maximum
 *        of two operands
 * Args:     i1 - operand 1
 *           i2 - operand 2
 * Return:   maximum of i1 and i2
 * Notes:
 */
int32_t fl_OR( int32_t i1, int32_t i2)
{
	return ( i1 > i2 ) ? i1 : i2;
}


/*
 * Name: fl_linmap
 * Descr: evaluate a piece-wise function by linear interpolation
 * Args:     input   - function input
 *           p_map   - pointer to function map
 *           map_len - number of entries in function map
 * Return:   value of piece-wise linear function at input
 *
 * Notes:
 */
int32_t fl_linmap( int32_t input, const struct fl_mf_pt_type *p_map, const size_t map_len)
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
		return ( p_map[i-1].y + (((input - p_map[i-1].x)*(p_map[i].y - p_map[i-1].y))/(p_map[i].x - p_map[i-1].x)));
}


/*
 * Name: fl_calculate_membership
 * Descr: calculate degree of membership of input in a fuzzy set
 * Args:     input   - system (crisp) input
 *           mf_map  - pointer to membership function map
 *           map_len - number of entries in function map
 * Return:   degree of membership of input in the fuzzy set
 *
 * Notes:
 */
int32_t fl_calculate_membership( int32_t input, const struct fl_mf_pt_type *mf_map, const size_t map_len)
{
	return fl_linmap( input, mf_map, map_len);
}


/*
 * Name: fl_mf_centroid
 * Descr: calculate centroid of a membership function
 * Args:     mf_map  - pointer to membership function map
 *           map_len - number of entries in function map
 * Return:   centroid of membership function (along x-axis)
 *
 * Notes:
 */
int32_t fl_mf_centroid( const struct fl_mf_pt_type *mf_map, const size_t map_len)
{
	int32_t num = 0, den = 0, i = 0;

	while( i < map_len)
	{
		num += (mf_map[i].x*mf_map[i].y);
		den += mf_map[i].y;

		i++;
	}

	return (num/den);
}


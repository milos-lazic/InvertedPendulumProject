/*
 * fl_defs.h
 *
 *  Created on: Feb 25, 2018
 *      Author: vlazic
 */

#ifndef FL_FL_DEFS_H_
#define FL_FL_DEFS_H_

#include <stdlib.h>
#include <stdint.h>

#define MAXNAME        16
#define MAX_INPUT     255
#define MIN_INPUT       0
#define MAX_OUTPUT    255
#define MIN_OUTPUT      0

/* FL module data structure prototypes */
struct fl_io_type;
struct fl_mf_type;
struct fl_mf_pt_type;
struct fl_rule_type;


/* Name: fl_mf_pt_type
 * Description: membership function map point type
 * Members: x - x coordinate
 *          y - y coordinate
 *
 *  Notes: - membership functions are represented as an array
 *           of points; i/o values (x) are mapped to fuzzy set
 *           membership values (y) using linear interpolation
 */
struct fl_mf_pt_type {
	int32_t x;
	int32_t y;
};


/* Name: fl_io_type
 * Description: controller I/O type
 * Members: name                     - name of system input/output
 *          value                    - current value of system input/output
 *          membership_functions     - pointer to array of membership functions
 *                                     (fuzzy sets) into which the input is classified
 *          num_membership_functions - number of entries in membership function
 *                                     array
 *
 *  Notes:
 */
struct fl_io_type {
	const char name[MAXNAME];
	int32_t value;
	struct fl_mf_type *membership_functions;
	const size_t num_membership_functions;
};



/* Name: fl_mf_type
 * Description: fuzzy set membership function type
 * Members: name       - name of membership function
 *          value      - input's degree of membership in the fuzzy set
 *          map        - pointer to membership function map
 *          map_len    - entries in membership function map
 *
 *  Notes: - used during fuzzification of system inputs
 */
struct fl_mf_type {
	const char name[MAXNAME];
	int32_t value;
	const struct fl_mf_pt_type *map;
	const size_t map_len;
};



/* Name: fl_rule_type
 * Description: fuzzy logic rule type
 * Members: if_side_pvals     - pointers to buffers containing values of
 *                              conditions for IF side of rule evaluation
 *                              expression
 *          then_side_pvals   - pointer to buffer in which to save the
 *                              result of the rule evaluation
 *
 *  Notes: - IF (condition1 AND condition2) THEN (output_level_N)
 *         - if_side_pvals array should be initialized to contain
 *           addresses of .value members in input membership function
 *           arrays
 *         - then_side_pvals should be initialized to contain address
 *           of .value member in an output membership function array
 */
struct fl_rule_type {
	int32_t *if_side_pvals[2];
	int32_t *then_side_pvals;
};



#endif /* FL_FL_DEFS_H_ */

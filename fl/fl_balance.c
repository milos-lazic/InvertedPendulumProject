/*
 * fl_balance.c
 *
 *  Created on: Mar 11, 2018
 *      Author: vlazic
 */

#include "fl_defs.h"
#include "fl_proto.h"
#include "../sys/device/device.h"


#define POS_SETPOINT 0

// system input enumeration
enum SYS_INPUT
{
	eSI_ERROR = 0,
	eSI_DERROR,
	eSI_MAX,
};

// system output enumeration
enum SYS_OUTPUT
{
	eSO_FORCE = 0,
	eSO_MAX,
};

// I/0 fuzzy membership set enumeration
enum IO_MEMBERSHIP_SET
{
	eIOM_NL = 0,
	eIOM_NM,
	eIOM_NS,
	eIOM_ZE,
	eIOM_PS,
	eIOM_PM,
	eIOM_PL,
	eIOM_MAX,
};



/* error input normalization map - convert raw system error to normalized system error */
static const struct fl_mf_pt_type ERROR_INPUT_NORMALIZE_MAP[] = // encoder position (error) input normalization map
{
	/*  { QEI_POS, NORMALIZED INPUT } */
		{  -15, 0 },
		{     0, 127 },
		{   15, 255 },
};
#define ERROR_INPUT_NORMALIZE_MAP_LEN (sizeof(ERROR_INPUT_NORMALIZE_MAP)/sizeof(ERROR_INPUT_NORMALIZE_MAP[0]))


/* derror input normalization map - convert raw system input error derivative to normalized system error derivative */
static const struct fl_mf_pt_type DERROR_INPUT_NORMALIZE_MAP[] = // encoder speed (derror) input normalization map
{
	/*  { QEI_SPEED, NORMALIZED INPUT } */
		{  -3, 0 },
		{    0, 127 },
		{    3, 255 },
};
#define DERROR_INPUT_NORMALIZE_MAP_LEN (sizeof(DERROR_INPUT_NORMALIZE_MAP)/sizeof(DERROR_INPUT_NORMALIZE_MAP[0]))


/* convert normalized system output to raw system output (motor input power) */
static const struct fl_mf_pt_type OUTPUT_RAW_MAP[] =
{
	/*  { raw output, motor input power (%) } */
		{ 0, -100 },
		{ 255, 100 },
};
#define OUTPUT_RAW_MAP_LEN (sizeof(OUTPUT_RAW_MAP)/sizeof(OUTPUT_RAW_MAP[0]))


/* NEGATIVE LARGE membership function map */
static const struct fl_mf_pt_type NL_mf_map[] = {
		{  0, 255 },
		{ 31, 255 },
		{ 63, 0 },
};
#define NL_MF_MAP_LEN (sizeof(NL_mf_map)/sizeof(NL_mf_map[0]))


/* NEGATIVE MEDIUM membership function map */
static const struct fl_mf_pt_type NM_mf_map[] = {
		{ 31, 0 },
		{ 63, 255 },
		{ 95, 0 },
};
#define NM_MF_MAP_LEN (sizeof(NM_mf_map)/sizeof(NM_mf_map[0]))


/* NEGATIVE SMALL membership function map */
static const struct fl_mf_pt_type NS_mf_map[] = {
		{ 63, 0 },
		{ 95, 255 },
		{ 127, 0 },
};
#define NS_MF_MAP_LEN (sizeof(NS_mf_map)/sizeof(NS_mf_map[0]))


/* ZERO membership function map */
static const struct fl_mf_pt_type ZE_mf_map[] = {
		{ 95, 0 },
		{ 127, 255 },
		{ 159, 0 },
};
#define ZE_MF_MAP_LEN (sizeof(ZE_mf_map)/sizeof(ZE_mf_map[0]))


/* POSITIVE SMALL membership function map */
static const struct fl_mf_pt_type PS_mf_map[] = {
		{ 127, 0 },
		{ 159, 255 },
		{ 191, 0 },
};
#define PS_MF_MAP_LEN (sizeof(PS_mf_map)/sizeof(PS_mf_map[0]))


/* POSITIVE MEDIUM membership function map */
static const struct fl_mf_pt_type PM_mf_map[] = {
		{ 159, 0 },
		{ 191, 255 },
		{ 223, 0 },
};
#define PM_MF_MAP_LEN (sizeof(PM_mf_map)/sizeof(PM_mf_map[0]))


/* POSITIVE LARGE membership function map */
static const struct fl_mf_pt_type PL_mf_map[] = {
		{ 191, 0 },
		{ 223, 255 },
		{ 255, 255 },
};
#define PL_MF_MAP_LEN (sizeof(PL_mf_map)/sizeof(PL_mf_map[0]))


/* membership functions for system input: error */
struct fl_mf_type sysin_error_membership_functions[] =
{
		/* IMPORTANT: order of entries must match enumeration IO_MEMBERSHIP */
		{ .name = "error_NL", .value = 0, .map = NL_mf_map, .map_len = NL_MF_MAP_LEN },
		{ .name = "error_NM", .value = 0, .map = NM_mf_map, .map_len = NM_MF_MAP_LEN },
		{ .name = "error_NS", .value = 0, .map = NS_mf_map, .map_len = NS_MF_MAP_LEN },
		{ .name = "error_ZE", .value = 0, .map = ZE_mf_map, .map_len = ZE_MF_MAP_LEN },
		{ .name = "error_PS", .value = 0, .map = PS_mf_map, .map_len = PS_MF_MAP_LEN },
		{ .name = "error_PM", .value = 0, .map = PM_mf_map, .map_len = PM_MF_MAP_LEN },
		{ .name = "error_PL", .value = 0, .map = PL_mf_map, .map_len = PL_MF_MAP_LEN },
};
#define SYSIN_ERROR_MAX_MF (sizeof(sysin_error_membership_functions)/sizeof(sysin_error_membership_functions[0]))


/* membership functions for system input: derror (derivative of errror) */
struct fl_mf_type sysin_derror_membership_functions[] =
{
		/* IMPORTANT: order of entries must match enumeration IO_MEMBERSHIP */
		{ .name = "derror_NL", .value = 0, .map = NL_mf_map, .map_len = NL_MF_MAP_LEN },
		{ .name = "derror_NM", .value = 0, .map = NM_mf_map, .map_len = NM_MF_MAP_LEN },
		{ .name = "derror_NS", .value = 0, .map = NS_mf_map, .map_len = NS_MF_MAP_LEN },
		{ .name = "derror_ZE", .value = 0, .map = ZE_mf_map, .map_len = ZE_MF_MAP_LEN },
		{ .name = "derror_PS", .value = 0, .map = PS_mf_map, .map_len = PS_MF_MAP_LEN },
		{ .name = "derror_PM", .value = 0, .map = PM_mf_map, .map_len = PM_MF_MAP_LEN },
		{ .name = "derror_PL", .value = 0, .map = PL_mf_map, .map_len = PL_MF_MAP_LEN },
};
#define SYSIN_DERROR_MAX_MF (sizeof(sysin_derror_membership_functions)/sizeof(sysin_derror_membership_functions[0]))

/* system inputs */
static struct fl_io_type System_Inputs[] =
{
		{ .name = "error",   .value = 0, .membership_functions = sysin_error_membership_functions,  .num_membership_functions = SYSIN_ERROR_MAX_MF },
		{ .name = "derror",  .value = 0, .membership_functions = sysin_derror_membership_functions, .num_membership_functions = SYSIN_DERROR_MAX_MF },
};
#define FLC_BALANCE_MAX_INPUTS (sizeof(System_Inputs)/sizeof(System_Inputs[0]))



/* membership functions for system output: force */
struct fl_mf_type sysout_force_membership_functions[] =
{
		/* IMPORTANT: order of entries must match enumeration IO_MEMBERSHIP */
		{ .name = "force_NL", .value = 0, .map = NL_mf_map, .map_len = NL_MF_MAP_LEN },
		{ .name = "force_NM", .value = 0, .map = NM_mf_map, .map_len = NM_MF_MAP_LEN },
		{ .name = "force_NS", .value = 0, .map = NS_mf_map, .map_len = NS_MF_MAP_LEN },
		{ .name = "force_ZE", .value = 0, .map = ZE_mf_map, .map_len = ZE_MF_MAP_LEN },
		{ .name = "force_PS", .value = 0, .map = PS_mf_map, .map_len = PS_MF_MAP_LEN },
		{ .name = "force_PM", .value = 0, .map = PM_mf_map, .map_len = PM_MF_MAP_LEN },
		{ .name = "force_PL", .value = 0, .map = PL_mf_map, .map_len = PL_MF_MAP_LEN },
};
#define SYSOUT_FORCE_MAX_MF (sizeof(sysout_force_membership_functions)/sizeof(sysout_force_membership_functions[0]))

/* system outputs */
static struct fl_io_type System_Outputs[] =
{
		{ .name = "force", .value = 0, .membership_functions = sysout_force_membership_functions, .num_membership_functions = SYSOUT_FORCE_MAX_MF },
};
#define FLC_BALANCE_MAX_OUTPUTS (sizeof(System_Outputs)/sizeof(System_Outputs[0]))



const struct fl_rule_type Rule_Set[] = {
		/* IF (error is NL) AND (derror is NL) THEN (force is NL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NL].value), &(sysin_derror_membership_functions[eIOM_NL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NL].value) },
		/* IF (error is NL) AND (derror is ZE) THEN (force is NL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NL].value), &(sysin_derror_membership_functions[eIOM_ZE].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NL].value) },
		/* IF (error is NM) AND (derror is NM) THEN (force is NM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NM].value), &(sysin_derror_membership_functions[eIOM_NM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NM].value) },
		/* IF (error is NM) AND (derror is ZE) THEN (force is NM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NM].value), &(sysin_derror_membership_functions[eIOM_ZE].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NM].value) },
		/* IF (error is NS) AND (derror is NS) THEN (force is NS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NS].value), &(sysin_derror_membership_functions[eIOM_NS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NS].value) },
		/* IF (error is NS) AND (derror is ZE) THEN (force is NS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NS].value), &(sysin_derror_membership_functions[eIOM_ZE].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NS].value) },
		/* IF (error is ZE) AND (derror is NL) THEN (force is NL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_ZE].value), &(sysin_derror_membership_functions[eIOM_NL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NL].value) },
		/* IF (error is ZE) AND (derror is NM) THEN (force is NM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_ZE].value), &(sysin_derror_membership_functions[eIOM_NM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NM].value) },
		/* IF (error is ZE) AND (derror is NS) THEN (force is NS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_ZE].value), &(sysin_derror_membership_functions[eIOM_NS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NS].value) },
		/* IF (error is ZE) AND (derror is ZE) THEN (force is ZE) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_ZE].value), &(sysin_derror_membership_functions[eIOM_ZE].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_ZE].value) },
		/* IF (error is ZE) AND (derror is PS) THEN (force is PS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_ZE].value), &(sysin_derror_membership_functions[eIOM_PS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PS].value) },
		/* IF (error is ZE) AND (derror is PM) THEN (force is PM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_ZE].value), &(sysin_derror_membership_functions[eIOM_PM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PM].value) },
		/* IF (error is ZE) AND (derror is PL) THEN (force is PL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_ZE].value), &(sysin_derror_membership_functions[eIOM_PL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PL].value) },
		/* IF (error is PS) AND (derror is ZE) THEN (force is PS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PS].value), &(sysin_derror_membership_functions[eIOM_ZE].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PS].value) },
		/* IF (error is PS) AND (derror is PS) THEN (force is PS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PS].value), &(sysin_derror_membership_functions[eIOM_PS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PS].value) },
		/* IF (error is PM) AND (derror is ZE) THEN (force is PM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PM].value), &(sysin_derror_membership_functions[eIOM_ZE].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PM].value) },
		/* IF (error is PM) AND (derror is PM) THEN (force is PM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PM].value), &(sysin_derror_membership_functions[eIOM_PM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PM].value) },
		/* IF (error is PL) AND (derror is ZE) THEN (force is PL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PL].value), &(sysin_derror_membership_functions[eIOM_ZE].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PL].value) },
		/* IF (error is PL) AND (derror is PL) THEN (force is PL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PL].value), &(sysin_derror_membership_functions[eIOM_PL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PL].value) },


		/* IF (error is PL) AND (derror is NL) THEN (force is ZE) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PL].value), &(sysin_derror_membership_functions[eIOM_NL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_ZE].value) },
		/* IF (error is PM) AND (derror is NM) THEN (force is ZE) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PM].value), &(sysin_derror_membership_functions[eIOM_NM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_ZE].value) },
		/* IF (error is PS) AND (derror is NS) THEN (force is ZE) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PS].value), &(sysin_derror_membership_functions[eIOM_NS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_ZE].value) },
		/* IF (error is NS) AND (derror is PS) THEN (force is ZE) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NS].value), &(sysin_derror_membership_functions[eIOM_PS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_ZE].value) },
		/* IF (error is NM) AND (derror is PM) THEN (force is ZE) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NM].value), &(sysin_derror_membership_functions[eIOM_PM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_ZE].value) },
		/* IF (error is NL) AND (derror is PM) THEN (force is ZE) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NL].value), &(sysin_derror_membership_functions[eIOM_PL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_ZE].value) },


		/* IF (error is NM) AND (derror is NL) THEN (force is NL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NM].value), &(sysin_derror_membership_functions[eIOM_NL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NL].value) },
		/* IF (error is NS) AND (derror is NL) THEN (force is NL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NS].value), &(sysin_derror_membership_functions[eIOM_NL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NL].value) },
		/* IF (error is PS) AND (derror is NL) THEN (force is NM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PS].value), &(sysin_derror_membership_functions[eIOM_NL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NM].value) },
		/* IF (error is PM) AND (derror is NL) THEN (force is NS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PM].value), &(sysin_derror_membership_functions[eIOM_NL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NS].value) },


		/* IF (error is NM) AND (derror is PL) THEN (force is PS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NM].value), &(sysin_derror_membership_functions[eIOM_PL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PS].value) },
		/* IF (error is NS) AND (derror is PL) THEN (force is PM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NS].value), &(sysin_derror_membership_functions[eIOM_PL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PM].value) },
		/* IF (error is PS) AND (derror is PL) THEN (force is PL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PS].value), &(sysin_derror_membership_functions[eIOM_PL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PL].value) },
		/* IF (error is PM) AND (derror is PL) THEN (force is PL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PM].value), &(sysin_derror_membership_functions[eIOM_PL].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PL].value) },


		/* IF (error is NL) AND (derror is NM) THEN (force is NL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NL].value), &(sysin_derror_membership_functions[eIOM_NM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NL].value) },
		/* IF (error is NL) AND (derror is NS) THEN (force is NL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NL].value), &(sysin_derror_membership_functions[eIOM_NS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NL].value) },
		/* IF (error is NL) AND (derror is PS) THEN (force is NM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NL].value), &(sysin_derror_membership_functions[eIOM_PS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NM].value) },
		/* IF (error is NL) AND (derror is PM) THEN (force is NS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NL].value), &(sysin_derror_membership_functions[eIOM_PM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NS].value) },


		/* IF (error is PL) AND (derror is NM) THEN (force is PS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PL].value), &(sysin_derror_membership_functions[eIOM_NM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PS].value) },
		/* IF (error is PL) AND (derror is NS) THEN (force is PM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PL].value), &(sysin_derror_membership_functions[eIOM_NS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PM].value) },
		/* IF (error is PL) AND (derror is PS) THEN (force is PL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PL].value), &(sysin_derror_membership_functions[eIOM_PS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PL].value) },
		/* IF (error is PL) AND (derror is PM) THEN (force is PL) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PL].value), &(sysin_derror_membership_functions[eIOM_PM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PL].value) },


		/* IF (error is NS) AND (derror is NM) THEN (force is NM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NS].value), &(sysin_derror_membership_functions[eIOM_NM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NM].value) },
		/* IF (error is PS) AND (derror is NM) THEN (force is NS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PS].value), &(sysin_derror_membership_functions[eIOM_NM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NS].value) },
		/* IF (error is NM) AND (derror is NS) THEN (force is NM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NM].value), &(sysin_derror_membership_functions[eIOM_NS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NM].value) },
		/* IF (error is PM) AND (derror is NS) THEN (force is PS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PM].value), &(sysin_derror_membership_functions[eIOM_NS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PS].value) },
		/* IF (error is NM) AND (derror is PS) THEN (force is NS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NM].value), &(sysin_derror_membership_functions[eIOM_PS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_NS].value) },
		/* IF (error is PM) AND (derror is PS) THEN (force is PM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PM].value), &(sysin_derror_membership_functions[eIOM_PS].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PM].value) },
		/* IF (error is NS) AND (derror is PM) THEN (force is PS) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_NS].value), &(sysin_derror_membership_functions[eIOM_PM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PS].value) },
		/* IF (error is PS) AND (derror is PM) THEN (force is PM) */
		{ .if_side_pvals = { &(sysin_error_membership_functions[eIOM_PS].value), &(sysin_derror_membership_functions[eIOM_PM].value) }, .then_side_pvals = &(sysout_force_membership_functions[eIOM_PM].value) },
};
#define FLC_BALANCE_MAX_RULES (sizeof(Rule_Set)/sizeof(Rule_Set[0]))



static void flcBalance_ReadSysInputs( void)
{
	// normalize the raw error input; map (SP - QEI_POS) on range [0, 255]
	System_Inputs[eSI_ERROR].value = fl_linmap( POS_SETPOINT - dev_ioctl(eDEV_QEI0, eQEI_IOCTL_R_POS), ERROR_INPUT_NORMALIZE_MAP, ERROR_INPUT_NORMALIZE_MAP_LEN);
	/* MLAZIC_TBD: dummy values for module testing */


	//System_Inputs[eSI_DERROR].value = 127; // ZE (speed = 0)
	//System_Inputs[eSI_DERROR].value = fl_linmap( vel , DERROR_INPUT_NORMALIZE_MAP, DERROR_INPUT_NORMALIZE_MAP_LEN);
	System_Inputs[eSI_DERROR].value = fl_linmap( -1*dev_ioctl(eDEV_QEI0, eQEI_IOCTL_READ_SPEED), DERROR_INPUT_NORMALIZE_MAP, DERROR_INPUT_NORMALIZE_MAP_LEN);
}


static void flcBalance_Fuzzification( void)
{
	struct fl_io_type *p_in = System_Inputs;
	struct fl_mf_type *p_mf;
	int32_t idx_mf, idx_in = 0;

	/* for each system input */
	while( idx_in < FLC_BALANCE_MAX_INPUTS)
	{
		p_mf = (struct fl_mf_type *)p_in[idx_in].membership_functions;
		idx_mf = 0;
		/* for each membership function associated with current system input */
		while( idx_mf < p_in[idx_in].num_membership_functions)
		{
			/* calculate degree of membership of current system input for each input fuzzy set */
			p_mf[idx_mf].value = fl_calculate_membership( p_in[idx_in].value, p_mf[idx_mf].map, p_mf[idx_mf].map_len);
			idx_mf++;
		}

		idx_in++;
	}
}


static void flcBalance_RuleEvaluation( void)
{
	int32_t idx_rule = 0, idx_out = 0, idx_mf;
	struct fl_io_type *p_out = System_Outputs;

	// reset all output fuzzy variables

	/* for each system output */
	while ( idx_out < FLC_BALANCE_MAX_OUTPUTS)
	{
		idx_mf = 0;
		/* for each membership  */
		while ( idx_mf < p_out->num_membership_functions)
		{
			p_out[idx_out].membership_functions[idx_mf].value = 0;
			idx_mf++;
		}

		idx_out++;
	}

	// for each controller rule
	while( idx_rule < FLC_BALANCE_MAX_RULES)
	{
		/* OUT_STRENGTH = OUT_STRENGTH & (IN1_STRENGTH | IN2_STRENGHT) */
		*(Rule_Set[idx_rule].then_side_pvals) = fl_OR( *(Rule_Set[idx_rule].then_side_pvals),
				                                     fl_AND( *(Rule_Set[idx_rule].if_side_pvals[0]),
				                                    		 *(Rule_Set[idx_rule].if_side_pvals[1])));
		idx_rule++;
	}

}


static void flcBalance_Defuzzification( void)
{
	int32_t num , den;
	struct fl_io_type *p_out = System_Outputs;
	struct fl_mf_type *p_mf = NULL;
	int32_t idx_mf, idx_out = 0;

	// loop through system outputs
	while( idx_out < FLC_BALANCE_MAX_OUTPUTS)
	{
		idx_mf = 0;
		num = 0;
		den = 0;
		p_mf = (struct fl_mf_type *)p_out[idx_out].membership_functions;

		// calculate output based on discrete weighted averages of output fuzzy membership sets
		while( idx_mf < p_out[idx_out].num_membership_functions)
		{
			num += ( fl_mf_centroid( p_mf[idx_mf].map, p_mf[idx_mf].map_len) * p_mf[idx_mf].value);
			den += p_mf[idx_mf].value;
			idx_mf++;
		}

		p_out[idx_out].value = (num/den);
		idx_out++;
	}
}


static void flcBalance_GenerateSysOutput( void)
{
	int32_t raw_out = 0;

	raw_out = fl_linmap( System_Outputs[eSO_FORCE].value, OUTPUT_RAW_MAP, OUTPUT_RAW_MAP_LEN);

	/* set power */
	(void) dev_ioctl(eDEV_ESC0, eESC_IOCTL_SET_POWER, raw_out);
}


/* MLAZIC_TBD: FOR TESTING */
void flcBalance_Run( void)
{
	flcBalance_ReadSysInputs();
	flcBalance_Fuzzification();
	flcBalance_RuleEvaluation();
	flcBalance_Defuzzification();
	flcBalance_GenerateSysOutput();
}







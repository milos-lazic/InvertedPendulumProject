/*
 * esc_dev.c
 *
 *  Created on: Mar 2, 2018
 *      Author: vlazic
 */

#include "device.h"


/* macro for calculating the PWM generator load value for
 * SYS_CLOCK = 80 MHz and PWM_DIV = 16; the frequency parameter
 * specified refers to the PWM frequency in units of Hertz
 */
#define PWM_LOAD(frequency) \
	((80000000/8)/frequency - 1)

/* macro for calculating the duty cycle based on the configured
 * PWM frequency; duty parameter specified is a duty cycle of
 * integral value from 0 to 100
 */
#define PWM_CMP(duty) \
		((PWM1_1_LOAD_R - 1) - ((duty*(PWM1_1_LOAD_R - 1))/100))



/* Device operations prototypes */
void esc_dev_init (void *self_attr);
int  esc_dev_write (void *self_attr, const char *buf, size_t count);
int  esc_dev_read (void *self_attr, char *buf, size_t count);
int  esc_dev_ioctl (void *self_attr, int request, va_list args);
void esc_dev_deinit (void *self_attr);


struct device_operations esc_devops = {
		.dev_init_r   = esc_dev_init,
		.dev_write_r  = esc_dev_write,
		.dev_read_r   = esc_dev_read,
		.dev_ioctl_r  = esc_dev_ioctl,
		.dev_deinit_r = esc_dev_deinit,
};


struct esc_pt_type
{
	int32_t x;
	int32_t y;
};

/* map power (%) to duty cycle */
static const struct esc_pt_type ESC_MAP_POWER_TO_DUTY[] =
{
		/* power, duty cycle */
		{ -100, 99 },
		{    0, 0 },
		{  100, 99 },
};
#define ESC_MAP_POWER_TO_DUTY_LEN (sizeof(ESC_MAP_POWER_TO_DUTY)/sizeof(ESC_MAP_POWER_TO_DUTY[0]))


/* same as fl_linmap */
static uint32_t esc_calculate_duty(int32_t input, const struct esc_pt_type *p_map, size_t map_len)
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


// esc0 (motor driver) device structure
struct device esc0_dev = {
		.name = "esc0",
		.self_attr = NULL,
		.dev_ops = &esc_devops
};


void esc_dev_init (void *self_attr)
{
	// enable system bus clock to GPIO Port A
	SYSCTL_RCGCGPIO_R |= 0x00000001;
	// wait until clock is stable
	while((SYSCTL_PRGPIO_R & 0x00000001) == 0) {};

	// enable system bus clock to GPIO Port F
	SYSCTL_RCGCGPIO_R |= 0x00000020;
	// wait until clock is stable
	while((SYSCTL_PRGPIO_R & 0x00000020) == 0) {};

	// ===== START_CONFIG: PF2 as Digital Output =====
	// disable alternate function on PF2
	GPIO_PORTF_DIR_R |= 0x00000004;
	// set PE1 direction as output
	GPIO_PORTF_AFSEL_R &= ~(0x00000004);
	// disable open-draing configuration on PF2
	GPIO_PORTF_ODR_R &= ~(0x00000004);
	// enable pull-down resistor on PF2
	GPIO_PORTF_PDR_R |= 0x00000004;
	// enable digital function on PF2
	GPIO_PORTF_DEN_R |= 0x00000004;
	// set PE1 to LOW
	GPIO_PORTF_DATA_R &= ~(0x00000004);
	// ===== STOP_CONFIG =====

	// ===== START_CONFIG: PF3 as Digital Output =====
	// disable alternate function on PF3
	GPIO_PORTF_DIR_R |= 0x00000008;
	// set PE1 direction as output
	GPIO_PORTF_AFSEL_R &= ~(0x00000008);
	// disable open-draing configuration on PF3
	GPIO_PORTF_ODR_R &= ~(0x00000008);
	// enable pull-down resistor on PF3
	GPIO_PORTF_PDR_R |= 0x00000008;
	// enable digital function on PF3
	GPIO_PORTF_DEN_R |= 0x00000008;
	// set PE1 to LOW
	GPIO_PORTF_DATA_R &= ~(0x00000008);
	// ===== STOP_CONFIG =====

	// ===== START_CONFIG: PA6 as M1PWM2 (pwm module 1, gen 1, out A) =====
	// configure PA6 as output
	GPIO_PORTA_DIR_R |= 0x00000040;
	// enable alternate function on PA6
	GPIO_PORTA_AFSEL_R |= 0x00000040;
	// select alternate function 5 (M1PWM2) on PA6
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & ~(0x0F000000)) | 0x05000000;
	// disable open-drain ocnfiguration on PA6
	GPIO_PORTA_ODR_R &= ~(0x00000040);
	// enable pull-down resistor on PA6
	GPIO_PORTA_PDR_R |= 0x00000040;
	// enable digital function on PA6
	GPIO_PORTA_DEN_R |= 0x00000040;
	// ===== STOP_CONFIG =====


	SYSCTL_RCGC0_R |= 0x00100000;
	// enable PWM module 1
	SYSCTL_RCGCPWM_R |= 0x00000002;
	// wait until PWM is ready for access
	while((SYSCTL_PRPWM_R & 0x00000002) == 0) {};

	// enable PWM clock divider
	SYSCTL_RCC_R |= 0x00100000;
	// set pwm divider to divide by 16 (80Mhz/8 = 10Mhz)
	SYSCTL_RCC_R = (SYSCTL_RCC_R & ~(0x000E0000)) | (0x00000002 << 17);

	// ===== START_CONFIG: PA6 as M1PWM2 =====
	// reset PWM 1 GEN 1 control register
	PWM1_1_CTL_R = 0x00000000;
	// enable generator counter to run in Debug mode (required)
	PWM1_1_CTL_R |= 0x00000004;
	// configure PWM 1 GEN 1 Signal A Generation
	// reset
	PWM1_1_GENA_R = 0x00000000;
	// drive pwm output A low when counter matches cmp A while counting down
	PWM1_1_GENA_R |= 0x00000002 << 6;
	// drive pwm output A high when counter matches LOAD value
	PWM1_1_GENA_R |= 0x00000003 << 2;
	// set PWM refresh rate (period) at 2000 Hz ( (SYS_CLK/PWMDIV)/PWM_FREQ ) - 1)
	// (80MHz/8)/2000Hz - 1 = 2499 = 0x1387
	PWM1_1_LOAD_R = PWM_LOAD(250); // set PWM frequency to 250 Hz
	// initialize duty cycle to 0 (set CMPA value to maximum)
	PWM1_1_CMPA_R = PWM_CMP(0); // set initial duty cycle to 0
	// enable PWM generator block
	PWM1_1_CTL_R |= 0x00000001;
	// enable PWM module
	PWM1_ENABLE_R |= 0x00000004;
	// ===== STOP_CONFIG =====




}


int esc_dev_write (void *self_attr, const char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int esc_dev_read (void *self_attr, char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int esc_dev_ioctl (void *self_attr, int request, va_list args)
{
	int rv = -1;
	int power = 0;

	switch(request)
	{
	case eESC_IOCTL_SET_POWER:
		power = va_arg(args, int);
		PWM1_1_CMPA_R = PWM_CMP(esc_calculate_duty( power,
				ESC_MAP_POWER_TO_DUTY, ESC_MAP_POWER_TO_DUTY_LEN));

		if ( power > 0)
		{
			// set motor direction
			/* MLAZIC_TBD: forward/reverse designation */
			GPIO_PORTF_DATA_R &= ~(0x00000004);
		}
		else
		{
			// set motor direction
			/* MLAZIC_TBD: forward/reverse designation */
			GPIO_PORTF_DATA_R |= 0x00000004;
		}

		rv = 0;
		break;

	default:
		break;
	}

	return rv;
}


void esc_dev_deinit (void *self_attr)
{
	/* not implemented */
}

/*
 * timer_dev.c
 *
 *  Created on: Apr 1, 2018
 *      Author: Milos Lazic
 */


#include "device.h"


/* Device operations prototypes */
void timer_dev_init(void *self_attr);
int  timer_dev_write(void *self_attr, const char *buf, size_t count);
int  timer_dev_read(void *self_attr, char *buf, size_t count);
int  timer_dev_ioctl(void *self_attr, int request, va_list args);
void timer_dev_deinit(void *self_attr);


struct device_operations timer_devops = {
		.dev_init_r = timer_dev_init,
		.dev_write_r = timer_dev_write,
		.dev_read_r = timer_dev_read,
		.dev_ioctl_r = timer_dev_ioctl,
		.dev_deinit_r = timer_dev_deinit,
};


struct device timer0_dev = {
		.name = "timer0",
		.self_attr = NULL,
		.dev_ops = &timer_devops,
};



void timer_dev_init(void *self_attr)
{
	/* General Purpose Timer Module Configuration */
	// enable GPTM0
	SYSCTL_RCGCTIMER_R |= 0x00000001;
	// wait until peripheral is ready
	while ((SYSCTL_PRTIMER_R & 0x00000001) == 0) {};

	// disable GPTM_0 A
	TIMER0_CTL_R &= ~(0x00000001);
	// select 32-bit timer configuration
	TIMER0_CFG_R = 0x00000000;
	// select one-shot mode
	TIMER0_TAMR_R |= 0x00000001;
	// set count-up mode
	TIMER0_TAMR_R |= (0x00000001 << 4);
	// set timeout event upper bound (maximum)
	TIMER0_TAILR_R = 0xFFFFFFFF;
}


int  timer_dev_write(void *self_attr, const char *buf, size_t count)
{
	// not implemented
	return -1;
}


int  timer_dev_read(void *self_attr, char *buf, size_t count)
{
	// not implemented
	return -1;
}


int  timer_dev_ioctl(void *self_attr, int request, va_list args)
{
	int rv = 0;

	switch(request)
	{
	case eTIMER_DISABLE: //stop timer
		TIMER0_CTL_R &= ~(0x00000001);
		break;

	case eTIMER_ENABLE: // start timer
		TIMER0_CTL_R |= 0x00000001;
		break;

	case eTIMER_RESET:
		TIMER0_TAV_R = 0x00000000;
		break;

	case eTIMER_READ:
		rv = TIMER0_TAV_R;
		break;
	default:
		break;
	}

	return rv;
}


void timer_dev_deinit(void *self_attr)
{
	// not implemented
}



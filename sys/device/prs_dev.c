/*
 * prs_dev.c
 *
 *  Created on: Mar 12, 2018
 *      Author: vlazic
 */

#include "device.h"

#define SYS_CLOCK 80000000  /* processor clock rate, hertz */
#define US_SPEED  340000    /* ultrasonic spread velocity, mm/sec */

/* macro to convert time (microseconds) to timer counter load value (clock cycles) */
#define TIMER_LR_USEC(us) \
			((us * (SYS_CLOCK/1000000)) - 1)


void prs_dev_init (void *self_attr);
int  prs_dev_write (void *self_attr, const char *buf, size_t count);
int  prs_dev_read (void *self_attr, char *buf, size_t count);
int  prs_dev_ioctl (void *self_attr, int request, va_list args);
void prs_dev_deinit (void *self_attr);


struct device_operations prs_devops = {
		.dev_init_r = prs_dev_init,
		.dev_write_r = prs_dev_write,
		.dev_read_r = prs_dev_read,
		.dev_ioctl_r = prs_dev_ioctl,
		.dev_deinit_r = prs_dev_deinit,
};



struct device prs0_dev = {
		.name = "prs0",
		.self_attr = NULL,
		.dev_ops = &prs_devops
};


/*
 * Name: sensor_r
 * Descr: trigger module measurement and return
 *        proximity in mm
 * Args:     none
 * Return:   proximity measurement in mm
 * Notes:    check datasheet for minimum object distance
 *           to get valid measurement
 */
static uint32_t sensor_r( void)
{
	uint32_t distance = 0, count = 0;

	/* OPERATING PROCEDURE
	 *  1) transmit at least 10 us high level pulse to module Trig pin
	 *     (PE2 -> Trig)
	 *  2) wait until rising edge asserted by Echo pin
	 *     (PE3 <- Echo)
	 *  3) start timer
	 *  4) wait until falling edge asserted by Echo pin
	 *     (PE3 <- Echo)
	 *  5) stop timer; read counter value
	 */

	// disable timer
	TIMER0_CTL_R &= ~(0x00000001);
	// set timeout value (10 us)
	TIMER0_TAILR_R = TIMER_LR_USEC(10);
	// reset timer counter
	TIMER0_TAV_R = 0x00000000;

	/* trigger measurement */
	// assert high level to module Trig pin (connected to PE2)
	GPIO_PORTE_DATA_R |= 0x00000004;
	// start (enable) timer
	TIMER0_CTL_R |= 0x00000001;
	// wait until timeout
	while ((TIMER0_RIS_R & 0x00000001) == 0) {};
	// assert low level to module Trig pin (connected to PE2)
	GPIO_PORTE_DATA_R &= ~(0x00000004);


	// disable timer
	TIMER0_CTL_R &= ~(0x00000001);
	// clear timeout flag (set in TIMER_RIS register bit 0)
	TIMER0_ICR_R |= 0x00000001;
	// reset timer counter
	TIMER0_TAV_R = 0x00000000;
	// set timeout value to maximum (for 32-bit timer, 80 MHz sysclock, this
	// is approximately 53 seconds)
	TIMER0_TAILR_R = 0xFFFFFFFF;

	// wait for module Echo port to assert rising edge
	while ((GPIO_PORTE_DATA_R & 0x00000008) == 0) {};
	// start timer
	TIMER0_CTL_R |= 0x00000001;
	// wait for module Echo port to assert falling edge
	while ((GPIO_PORTE_DATA_R & 0x00000008) == 0x000000008) {};
	// read timer (counter value)
	count = TIMER0_TAR_R;
	// disable timer
	TIMER0_CTL_R &= ~(0x00000001);


	// conversion to distance units (mm)
	// NOTE: conversion must be done using intermediate
	//       division to prevent buffer overflow
	distance = ((count/80/2)*US_SPEED)/1000000;

	return distance;
}



void prs_dev_init (void *self_attr)
{
	// initialize timer0, PE2, PE3

	// enable system peripheral GPIO Port E
	SYSCTL_RCGCGPIO_R |= 0x00000010;
	// wait until clock is stable
	while((SYSCTL_PRGPIO_R & 0x00000010) == 0) {};

	// enable system peripheral Timer 0 (GPTM)
	SYSCTL_RCGCTIMER_R |= 0x00000001;
	// wait until clock is stable
	while ((SYSCTL_PRTIMER_R & 0x00000001) == 0) {};


	// ===== START_CONFIG: PE2 as Digital Output =====
	// Note: controls US proximity sensor modules trigger signal
	// disable alternate function on PE2
	GPIO_PORTE_AFSEL_R &= ~(0x00000004);
	// configure PE2 as output
	GPIO_PORTE_DIR_R |= 0x00000004;
	// disable open-drain configuration on PE2
	GPIO_PORTE_ODR_R &= ~(0x00000004);
	// enable pull-down resistor on PE2
	GPIO_PORTE_PDR_R |= 0x00000004;
	// enable digital function on PE2
	GPIO_PORTE_DEN_R |= 0x00000004;
	// set PE2 to LOW
	GPIO_PORTE_DATA_R &= ~(0x00000004);
	// ===== STOP_CONFIG =====


	// ===== START_CONFIG: PE3 as Digital Input =====
	// disable alternate function on PE3
	GPIO_PORTE_AFSEL_R &= ~(0x00000008);
	// configure PE3 as input
	GPIO_PORTE_DIR_R &= ~(0x00000008);
	//disable open-drain configuration on PE3
	GPIO_PORTE_ODR_R &= ~(0x00000008);
	// enable pull-down resistor on PE3
	GPIO_PORTE_PDR_R |= 0x00000008;
	// enable digital function on PE3
	GPIO_PORTE_DEN_R |= 0x00000008;
	// set PE3 to LOW
	GPIO_PORTE_DATA_R &= ~(0x00000008);
	// ===== STOP_CONFIG =====


	// ===== START_CONFIG: GPTM 0 =====
	// disable timer
	TIMER0_CTL_R &= ~(0x00000001);
	// select 32-bit timer configuration
	TIMER0_CFG_R = 0x00000000;
	// configure one-shot mode operation
	TIMER0_TAMR_R |= 0x00000001;
	//configure count-up mode operation
	TIMER0_TAMR_R |= (0x00000001 << 4);
#if 0 /* not needed */
	// set timeout event upper bound
	TIMER0_TAILR_R = 0xFFFFFFFF;
#endif
	// ===== STOP_CONFIG =====
}


int  prs_dev_write (void *self_attr, const char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int  prs_dev_read (void *self_attr, char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int  prs_dev_ioctl (void *self_attr, int request, va_list args)
{
	int rv = -1;

	switch(request)
	{
	case ePRS_IOCTL_DISTMSR:
		/* measure object proximity */
		rv = sensor_r();
		break;

	default:
		break;
	}

	return rv;
}



void prs_dev_deinit (void *self_attr)
{
	/* not implemented */
}

/*
 * pll_dev.c
 *
 *  Created on: Mar 1, 2018
 *      Author: vlazic
 */

#include "device.h"
#include "inc/tm4c123gh6pm.h"

/* Device operations prototypes */
void pll_dev_init (void *self_attr);
int  pll_dev_write (void *self_attr, const char *buf, size_t count);
int  pll_dev_read (void *self_attr, char *buf, size_t count);
int  pll_dev_ioctl (void *self_attr, int request, va_list args);
void pll_dev_deinit (void *self_attr);


struct device_operations pll_devops = {
		.dev_init_r   = pll_dev_init,
		.dev_write_r  = pll_dev_write,
		.dev_read_r   = pll_dev_read,
		.dev_ioctl_r  = pll_dev_ioctl,
		.dev_deinit_r = pll_dev_deinit,
};


struct device pll_dev =
{
		.name = "pll",
		.self_attr = NULL,
		.dev_ops = &pll_devops
};


// configure system bus clock rate as 80 MHz
void pll_dev_init (void *self_attr)
{
	// use RCC2
	SYSCTL_RCC2_R |= 0x80000000;
	// bypass (disable) PLL during initialization
	SYSCTL_RCC2_R |= (0x00000001 << 11);
	// specify external crystal frequency (16MHz external crystal for TM4C)
	SYSCTL_RCC_R = (SYSCTL_RCC_R & ~(0x000007C0)) | (0x15 << 6);
	// configure for main oscillator source
	SYSCTL_RCC2_R &= ~(0x00000070);
	// clear PWRDN2 to activate PLL
	SYSCTL_RCC2_R &= ~(0x00002000);
	// use 400 MHz PLL
	SYSCTL_RCC2_R |= (0x00000001 << 30);
	// configure clock divider and enable PLL
	SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~(0x1FC00000)) | (0x00000004 << 22);
	// wait for PLL to stabilize (stabilized when bit 6 in SYSCTL_RIS_R is set)
	while( (SYSCTL_RIS_R & 0x00000040) == 0) { /* loop */ };
	// clear PLL bypass bit
	SYSCTL_RCC2_R &= ~(0x00000001 << 11);
}


int pll_dev_write (void *self_attr, const char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int pll_dev_read (void *self_attr, char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int pll_dev_ioctl (void *self_attr, int request, va_list args)
{
	/* not implemented */
	return -1;
}


void pll_dev_deinit (void *self_attr)
{
	/* not implemented */
}

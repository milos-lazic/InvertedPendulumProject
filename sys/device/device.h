/*
 * device.h
 *
 *  Created on: Feb 26, 2018
 *      Author: Milos Lazic
 */

#ifndef SYS_DEVICE_DEVICE_H_
#define SYS_DEVICE_DEVICE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>

#define DEV_MAX_NAME 8


typedef uint32_t dev_t;

enum E_DEVICE
{
	eDEV_PLL = 0,
	eDEV_QEI0,
	eDEV_QEI1,
	eDEV_ESC0,
	eDEV_PRS0,
	eDEV_SSD1306, // LCD
	eDEV_TIMER0,
	eDEV_MAX,
};


// device ioctl request enumeration
enum E_IOCTL_REQ
{
	/* QEI_DEV */
	eQEI_IOCTL_R_POS = 0,    /* read position (raw) */
	eQEI_IOCTL_W_POS,        /* write position (raw) */
	eQEI_IOCTL_READ_SPEED,   /* read velocity (raw), signed */
	eQEI_IOCTL_R_POS_RAD,    /* read position (converted to radians) */
	eQEI_IOCTL_R_VEL_RAD,    /* read velocity (converted to radians/sec), signed */

	/* ESC_DEV */
	eESC_IOCTL_SET_POWER,

	/* PRS_DEV */
	ePRS_IOCTL_DISTMSR,

	/* SSD1306_DEV */
	eSSD1306_IOCTL_CLEARALL,

	/* TIMER_DEV */
	eTIMER_DISABLE,
	eTIMER_ENABLE,
	eTIMER_RESET,
	eTIMER_READ,


	eIOCTL_REQ_MAX,
};



/* DEV_LSEEK */
#define DEV_SEEK_SET     0
#define DEV_SEEK_CUR     1
#define DEV_SEEK_END     2


struct device_operations
{
	void (*dev_init_r) (void *self_attr);
	int  (*dev_write_r) (void *self_attr, const char *buf, size_t count);
	int  (*dev_read_r) (void *self_attr, char *buf, size_t count);
	int  (*dev_ioctl_r) (void *self_attr, int request, va_list args);
	int  (*dev_lseek_r) (void *self_attr, int offset, int whence);
	void (*dev_deinit_r) (void *self_attr);
};


struct device
{
	char name[DEV_MAX_NAME]; // device name
	void *self_attr; // pointer to device-specific attributes container
	struct device_operations *dev_ops; // pointer to device (driver) routines
};



/* function prototypes */
extern void dev_init(dev_t devno);
extern int  dev_write(dev_t devno, const char *buf, size_t count);
extern int  dev_read(dev_t devno, char *buf, size_t count);
extern int  dev_ioctl(dev_t devno, int request, ...);
extern int  dev_lseek(dev_t devno, int offset, int whence);
extern void dev_deinit(dev_t devno);




#endif /* SYS_DEVICE_DEVICE_H_ */

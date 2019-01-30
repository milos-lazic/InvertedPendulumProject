/*
 * device.c
 *
 *  Created on: Feb 27, 2018
 *      Author: Milos Lazic
 */



#include "device.h"
#include "string.h"


extern struct device pll_dev;
extern struct device qei0_dev;
extern struct device qei1_dev;
extern struct device esc0_dev;
extern struct device prs0_dev;
extern struct device ssd1306_dev;
extern struct device timer0_dev;


/* NOTE: must be in same order as enumeration E_DEVICE in device.h */
static struct device *pdev[eDEV_MAX] =
{
		&pll_dev,
		&qei0_dev,
		&qei1_dev,
		&esc0_dev,
		&prs0_dev,
		&ssd1306_dev,
		&timer0_dev,
};



void dev_init(dev_t devno)
{
	pdev[devno]->dev_ops->dev_init_r(pdev[devno]->self_attr);
}


int dev_write(dev_t devno, const char *buf, size_t count)
{
	return pdev[devno]->dev_ops->dev_write_r(pdev[devno]->self_attr, buf, count);
}


int dev_read(dev_t devno, char *buf, size_t count)
{
	return pdev[devno]->dev_ops->dev_read_r(pdev[devno]->self_attr, buf, count);
}


int dev_lseek(dev_t devno, int offset, int whence)
{
	return pdev[devno]->dev_ops->dev_lseek_r(pdev[devno]->self_attr, offset, whence);
}


int dev_ioctl(dev_t devno, int request, ...)
{
	int rv;

	va_list args;
	va_start(args, request);
	rv = pdev[devno]->dev_ops->dev_ioctl_r(pdev[devno]->self_attr, request, args);
	va_end(args);

	return rv;
}


void dev_deinit(dev_t devno)
{
	pdev[devno]->dev_ops->dev_deinit_r(pdev[devno]->self_attr);
}


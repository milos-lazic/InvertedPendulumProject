/*
 * qei_dev.c
 *
 *  Created on: Mar 1, 2018
 *      Author: Milos Lazic
 */

#include "device.h"

#define QEI0_BASE_ADDR        ((uint32_t *)0x4002C000)
#define QEI1_BASE_ADDR        ((uint32_t *)0x4002D000)

#define QEICTL_REG_OFST       0x000
#define QEISTAT_REG_OFST      0x004
#define QEIPOS_REG_OFST       0x008
#define QEIMAXPOS_REG_OFST    0x00C
#define QEILOAD_REG_OFST      0x010
#define QEITIME_REG_OFST      0x014
#define QEICOUNT_REG_OFST     0x018
#define QEISPEED_REG_OFST     0x01C
#define QEIINTEN_REG_OFST     0x020
#define QEIRIS_REG_OFST       0x024
#define QEIISC_REG_OFST       0x028


#define QEI_PPR 2400  /* encoder pulses per revolution */
#define PI 3.14159    /* Pi (arithmetic constant) */


/* Device operations prototypes */
void qei_dev_init (void *self_attr);
int  qei_dev_write (void *self_attr, const char *buf, size_t count);
int  qei_dev_read (void *self_attr, char *buf, size_t count);
int  qei_dev_ioctl (void *self_attr, int request, va_list args);
void qei_dev_deinit (void *self_attr);


struct device_operations qei_devops = {
		.dev_init_r   = qei_dev_init,
		.dev_write_r  = qei_dev_write,
		.dev_read_r   = qei_dev_read,
		.dev_ioctl_r  = qei_dev_ioctl,
		.dev_deinit_r = qei_dev_deinit,
};



struct qei_attr
{
	uint32_t *BASE_ADDR;
};

struct qei_attr qei0_attr = { .BASE_ADDR = QEI0_BASE_ADDR };
struct qei_attr qei1_attr = { .BASE_ADDR = QEI1_BASE_ADDR };


struct device qei0_dev = {
		.name = "qei0",
		.self_attr = &qei0_attr,
		.dev_ops = &qei_devops
};

struct device qei1_dev = {
		.name = "qei1",
		.self_attr = &qei1_attr,
		.dev_ops = &qei_devops
};


void qei_dev_init(void *self_attr)
{
	struct qei_attr *attr = (struct qei_attr *) self_attr;

	/* IF QEI0 */
	if ( attr->BASE_ADDR == QEI0_BASE_ADDR)
	{

		// enable system bus clock to GPIO Port D
		SYSCTL_RCGCGPIO_R |= 0x00000008;
		// loop until clock is stable
		while((SYSCTL_PRGPIO_R & 0x00000008) == 0) {};

		// unlock GPIO Port D GPIOCR (commit control) register
		GPIO_PORTD_LOCK_R = 0x4C4F434B;
		// enable reconfiguration of PD0-7
		GPIO_PORTD_CR_R = 0x000000FF; // enable reconfiguration of Port D pins

		// set alternate function mode on PD3,PD6,PD7 (QEI)
		GPIO_PORTD_AFSEL_R |= (0x00000080|0x00000040|0x00000008);

		// select alternate function on PD3,PD6,PD7 (QEI)
		GPIO_PORTD_PCTL_R |= (0x00000006 << 12); // mux QEI 0 Idx signal to PD3
		GPIO_PORTD_PCTL_R |= (0x00000006 << 24); // mux QEI 0 PhA signal to PD6
		GPIO_PORTD_PCTL_R |= (0x00000006 << 28); // mux QEI 0 PhB signal to PD7

		// enable pull-up resistors on PD3,PD6,PD7
		GPIO_PORTD_PUR_R |= (0x00000080|0x00000040|0x00000008);

		// enable digital function on PD6,PD7
		GPIO_PORTD_DEN_R |= (0x00000080|0x00000040|0x00000008);


		// enable system bus clock to QEI module 0
		SYSCTL_RCGCQEI_R |= 0x00000001;
		// loop until clock is stable
		while((SYSCTL_PRQEI_R & 0x00000001) == 0) {};

		// capture PhA and PhB edges (2400 ppr)
		QEI0_CTL_R = 0x00000008;
		// enable velocity capture
		QEI0_CTL_R |= 0x00000020;
		// velocity predivider = 0x00 (+1)
		QEI0_CTL_R = (QEI0_CTL_R &= ~(0x000001C0));
		// swap input signals (requirement based on simulation model)
		QEI0_CTL_R |= 0x00000002;

		// set maximum position counter value
		QEI0_MAXPOS_R = 0xFFFFFFFF;

		// set timer load; period in CPU (system bus clock) cycles
		QEI0_LOAD_R = 0x3D08FF; // (50 ms)

		// enable QEI
		QEI0_CTL_R |= 0x00000001;

		// set QEI_POS_R initial value
		QEI0_POS_R = 0x00000000;

	} /* ENDIF QEI0 */
	/* ELIF QEI_1 */
	else if ( attr->BASE_ADDR == QEI1_BASE_ADDR)
	{
		/* MLAZIC_TBD: not implemented yet */
		// enable system bus clock to GPIO Port C
		SYSCTL_RCGCGPIO_R |= 0x00000004;
		// loop until clock is stable
		while((SYSCTL_PRGPIO_R & 0x00000004) == 0) {};

		// set alternate function mode on PC4,PC5,PDC6 (QEI)
		GPIO_PORTC_AFSEL_R |= (0x00000010|0x00000020|0x00000040);

		// select alternate function on PC4,PC5,PC6 (QEI)
		GPIO_PORTC_PCTL_R |= (0x00000006 << 16); // mux QEI 1 Idx signal to PC4
		GPIO_PORTC_PCTL_R |= (0x00000006 << 20); // mux QEI 1 PhA signal to PC5
		GPIO_PORTC_PCTL_R |= (0x00000006 << 24); // mux QEI 1 PhB signal to PC6

		// enable pull-up resistors on PC4,PC5,PC6
		GPIO_PORTC_PUR_R |= (0x00000010|0x00000020|0x00000040);

		// enable digital function on PC4,PC5, PC6
		GPIO_PORTC_DEN_R |= (0x00000010|0x00000020|0x00000040);


		// enable system bus clock to QEI module 0
		SYSCTL_RCGCQEI_R |= 0x00000002;
		// loop until clock is stable
		while((SYSCTL_PRQEI_R & 0x00000002) == 0) {};

		// capture PhA and PhB edges (2400 ppr)
		QEI1_CTL_R = 0x00000008;
		// enable velocity capture
		QEI1_CTL_R |= 0x00000020;
		// velocity predivider = 0x00 (+1)
		QEI1_CTL_R = (QEI1_CTL_R &= ~(0x000001C0));

		// set maximum position counter value
		QEI1_MAXPOS_R = 0xFFFFFFFF;

		// set timer load; period in CPU (system bus clock) cycles
		QEI1_LOAD_R = 0x3D08FF; // (50 ms)


		// enable QEI
		QEI1_CTL_R |= 0x00000001;

		// set QEI_POS_R initial value
		QEI1_POS_R = 0x00000000;
	} /* ENDIF QEI_1 */
	else
	{
		/* invalid device; do nothing */
	}
}


int  qei_dev_write (void *self_attr, const char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int  qei_dev_read (void *self_attr, char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int  qei_dev_ioctl (void *self_attr, int request, va_list args)
{
	int rv = 0;
	int dir = 0;
	int raw_pos = 0, raw_vel = 0;
	float *buf = NULL;

	struct qei_attr *attr = (struct qei_attr *) self_attr;

	switch(request)
	{
	case eQEI_IOCTL_R_POS:
		/* intermediate cast to (uint8_t *) required for pointer arithmetic */
		/* cast back to (uint32_t *) required for 32-bit dereference and read */
		rv =  *((uint32_t *)((uint8_t *)attr->BASE_ADDR + QEIPOS_REG_OFST));
		break;

	case eQEI_IOCTL_W_POS:
		/* intermediate cast to (uint8_t *) required for pointer arithmetic */
		/* cast back to (uint32_t *) required for 32-bit dereference and write */
		*((int32_t *)((uint8_t *) attr->BASE_ADDR + QEIPOS_REG_OFST)) = va_arg(args, int);
		break;

	case eQEI_IOCTL_READ_SPEED:
		if ( attr->BASE_ADDR == QEI0_BASE_ADDR)
		{
			/* NOTE: QEI1_STAT_R, BIT(1) <DIRECTION>
			 * DIR=0: CCW   ---> set direction to negative
			 * DIR=1: CW    ---> set direction to positive
			 */

			/* intermediate cast to (uint8_t *) required for pointer arithmetic */
			/* cast back to (uint32_t *) required for 32-bit dereference and write */
			dir = ( *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEISTAT_REG_OFST)) & 0x00000002 ) ? -1 : 1;
			rv = *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEISPEED_REG_OFST)) * dir;

		}
		else if ( attr->BASE_ADDR == QEI1_BASE_ADDR)
		{
			/* NOTE: QEI1_STAT_R, BIT(1) <DIRECTION>
			 * DIR=0: CW   ---> set direction to positive
			 * DIR=1: CCW  ---> set direction to negative
			 */

			/* intermediate cast to (uint8_t *) required for pointer arithmetic */
			/* cast back to (uint32_t *) required for 32-bit dereference and write */
			dir = ( *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEISTAT_REG_OFST)) & 0x00000002 ) ? -1 : 1;
			rv = *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEISPEED_REG_OFST)) * dir;
		}
		break;

	case eQEI_IOCTL_R_POS_RAD:
		/* intermediate cast to (uint8_t *) required for pointer arithmetic
		 * logic that follows; cast back to (uint32_t *) required for 32-bit
		 * dereference and read
		 *
		 * NOTE: caller passes in pointer to buffer (float *) to store the result
		 *
		 */
		buf = va_arg(args, float *); // reference to storage buffer
		raw_pos = *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEIPOS_REG_OFST));
		*buf = ((float)raw_pos/(float)QEI_PPR)*2.0*PI;
		break;

	case eQEI_IOCTL_R_VEL_RAD:
		/* intermediate cast to (uint8_t *) required for pointer arithmetic
		 * logic that follows; cast back to (uint32_t *) required for 32-bit
		 * dereference and read
		 *
		 * NOTE: caller passes in pointer to buffer (float *) to store the result
		 */


		buf = va_arg(args, float *); // reference to storage buffer
		if ( attr->BASE_ADDR == QEI0_BASE_ADDR)
		{
			/* NOTE: QEI1_STAT_R, BIT(1) <DIRECTION>
			 * DIR=0: CCW   ---> set direction to negative
			 * DIR=1: CW    ---> set direction to positive
			 */

			/* intermediate cast to (uint8_t *) required for pointer arithmetic */
			/* cast back to (uint32_t *) required for 32-bit dereference and write */
			raw_vel = *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEISPEED_REG_OFST));
			dir = ( *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEISTAT_REG_OFST)) & 0x00000002 ) ? -1 : 1;
			*buf = ((dir * raw_vel * 2.0 * PI)/QEI_PPR)/( (float)*((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEILOAD_REG_OFST))/80000000);
		}
		else if ( attr->BASE_ADDR == QEI1_BASE_ADDR)
		{
			/* NOTE: QEI1_STAT_R, BIT(1) <DIRECTION>
			 * DIR=0: CW   ---> set direction to positive
			 * DIR=1: CCW  ---> set direction to negative
			 */

			/* intermediate cast to (uint8_t *) required for pointer arithmetic */
			/* cast back to (uint32_t *) required for 32-bit dereference and write */
			raw_vel = *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEISPEED_REG_OFST));
			dir = ( *((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEISTAT_REG_OFST)) & 0x00000002 ) ? -1 : 1;
			*buf = ((dir * raw_vel * 2.0 * PI)/QEI_PPR)/( (float)*((uint32_t *)((uint8_t *) attr->BASE_ADDR + QEILOAD_REG_OFST))/80000000);
		}

		break;

	default:
		break;
	}
	/* not implemented */
	return rv;
}


void qei_dev_deinit (void *self_attr)
{
	/* not implemented */
}

/*
 * main.c
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <inc/tm4c123gh6pm.h>

#include "fl/fl.h"
#include "lqr/lqr.h"
#include "sys/device/device.h"
#include "driverlib/sysctl.h"



#define container_of(ptr, type, member) ({  \
        const void *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

// forward function declarations
static void UART_write(const char *buf);



// enable FPU
void FPU_enable(uint32_t s)
{
	if (s)
		NVIC_CPAC_R = (NVIC_CPAC_R & ~(0x00F00000)) | (0x00F00000);
	else
		NVIC_CPAC_R = (NVIC_CPAC_R & ~(0x00F00000));
}


// intialize SysTick timer; enable periodic interrupts
static void SysTick_Init( uint32_t period)
{
	// clear ENABLE bit to disable systick during initialization
	NVIC_ST_CTRL_R = 0x00000000;
	// set RELOAD register; stores number of system bus clock cycles per
	// SysTick period (max val = 0x00FFFFFF)
	NVIC_ST_RELOAD_R = period -1;
	// reset CURRENT register (holds current value of counter)
	NVIC_ST_CURRENT_R = 0x00000000;
	// set SysTick interrupt priority level
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & ~(0xFF000000)) | (0x00000002 << 29);
	// enable timer with core clock and interrupts
	NVIC_ST_CTRL_R |= 0x00000007;
}



//#define __DEBUG__
void SysTick_Handler(void)
{
#ifndef __DEBUG__
	LQR_Balance_CtrlRun();
#else
	/* The following code is used to record the system response
	 * for system identification purposes. Data is serially transmitted
	 * to PC via serial port (UART).
	 */

	/* NOTE: time_5ms counter contains number of 5ms periods that have expired */
	static uint32_t time_5ms = 0;
	float val;
	float x, xdot, th, thdot;
	uint32_t time_val= 0;
	char buf[128];


	(void) dev_ioctl(eDEV_QEI1, eQEI_IOCTL_R_POS_RAD, &val); /* TBD: write new driver routine to return position in meters */
	x = val * 0.0069358; /* belt pulley radius (from calibration); macro defined in LQR module */

	(void) dev_ioctl(eDEV_QEI1, eQEI_IOCTL_R_VEL_RAD, &val); /* TBD: write new driver routine to return velocity in meters/sec */
	xdot = val * 0.0069358; /* belt pulley radius (from calibration); macro defined in LQR module */

	(void) dev_ioctl(eDEV_QEI0, eQEI_IOCTL_R_POS_RAD, &th); /* TBD: write new driver routine to return angle in rad */
	(void) dev_ioctl(eDEV_QEI0, eQEI_IOCTL_R_VEL_RAD, &thdot); /* TBD: wrtie new driver routine to return angular velocity in rad/sec */

	/* for MATLAB, {time(ms), input(PWM Comparator value), output(QEI_SPEED) } */
	snprintf(buf, sizeof(buf)-1, "%u, %.4f, %.4f, %.4f, %.4f;\r\n",time_5ms, x, xdot, th, thdot);
	UART_write(buf);


	if ( ++time_5ms == 150) // run for 1000 ms
	{
		time_val = TIMER0_TAR_R;
		// disable
		TIMER0_CTL_R &= ~(0x00000001);
		dev_ioctl(eDEV_ESC0, eESC_IOCTL_SET_POWER, 0);
		while(1);
	}
#endif
}




// debugging
static void UART_write(const char *buf)
{
	char *p = (char *)buf;

	while(*p != '\0')
	{
		UART0_DR_R = *p++;
		while(UART0_FR_R & 0x00000008) {};
	}
}


// debugging
volatile float setpoint = 0;
void UART0_InterruptHandler(void)
{
	uint8_t data;

	while(!(UART0_FR_R & 0x00000010))
	{
		data = UART0_DR_R;
		// echo data

		if ( data == 'a')
		{
			setpoint = setpoint + 0.15;
		}
		else if ( data == 'd')
		{
			setpoint = setpoint - 0.15;
		}

		LQR_Balance_SetPoint(setpoint);

		UART_write("\r\n RXd: ");
		UART0_DR_R = data;
	}

	/* acknowledge interrupt (clear interrupt flag */
	UART0_ICR_R |= (0x00000010 | 0x00000040); // clear RXRIS bit
}

// debugging
static void UART_Init(void)
{
	// enable and provide clock to UART module 0
	SYSCTL_RCGCUART_R |= 0x00000001;
	// wait until clock stabilizes
	while(SYSCTL_PRUART_R & 0x00000001 == 0) {};

	// enable and provide clock to GPIO Port A
	SYSCTL_RCGCGPIO_R |= 0x00000001;
	// wait until clock stabilizes
	while(SYSCTL_PRGPIO_R & 0x00000001 == 0) {};

	// unlock GPIO Port A commit control register
	GPIO_PORTA_LOCK_R = 0x4C4F434B;
	// enable reconfiguration of PA7:0
	GPIO_PORTA_CR_R |= 0x000000FF;

	// enable alternate function on PA0 (U0Rx)
	GPIO_PORTA_AFSEL_R |= 0x00000001;
	// select alternate function on PA0 (1 = U0Rx)
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & ~(0x0000000F)) | 0x00000001;
	// disable open drain configuration on PA0
	GPIO_PORTA_ODR_R &= ~(0x00000001);
	// enable digital function on PA0
	GPIO_PORTA_DEN_R |= 0x00000001;

	// enable alternate function on PA1 (U0Tx)
	GPIO_PORTA_AFSEL_R |= 0x00000002;
	// select alternat function on PA1 (1 = U0Tx)
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & ~(0x000000F0)) | (0x00000001 << 4);
	// disable open drain configuration on PA1
	GPIO_PORTA_ODR_R &= ~(0x00000002);
	// enable digital function on PA1
	GPIO_PORTA_DEN_R |= 0x00000002;

	// disable UART Module 0
	UART0_CTL_R &= ~(0x00000001);
	// set baud rate to 115200
	UART0_IBRD_R = 86; // write integer portion of baud rate divisor
	UART0_FBRD_R = 52; // write fractional portion of baud rate divisor
	// set serial parameters (8N1)
	UART0_LCRH_R = 0x00000000;
	UART0_LCRH_R = (UART0_LCRH_R & ~(0x00000060)) | (0x3 << 5); // 8 data bits
	UART0_LCRH_R &= ~(0x00000010); // flush transmit FIFO
	UART0_LCRH_R |= 0x00000010; // enable FIFOs
	// set FIFO level that tirgger TXRIS and RXRIS
	UART0_IFLS_R = 0x00000000;
	// set ClkDiv to 8
	UART0_CTL_R |= 0x00000020;
	// set UART Clock source to SYS_CLOCK
	UART0_CC_R = 0x00000000;

	/* interrupts */
	// configure device to send interrupt to NVIC when RXRIS/RTRIS bits are set
	UART0_IM_R |= (0x00000010 | 0x00000040);
	// enable interrupt (UART0 interrupt number = 5)
	NVIC_EN0_R |= 0x00000020;
	// set interrupt priority level (2)
	NVIC_PRI1_R = (NVIC_PRI1_R & ~(0x0000E000)) | (0x02 << 13);

	// enable UART Module 0
	UART0_CTL_R |= 0x00000001;


}




// debugging
static void sandbox(void)
{
	dev_init(eDEV_PLL); // 80 MHz
	dev_init(eDEV_QEI0);
	dev_init(eDEV_QEI1);
	dev_init(eDEV_ESC0);

	FPU_enable(1);

	UART_Init();

	dev_ioctl(eDEV_QEI0, eQEI_IOCTL_W_POS, 0); // zero the position
	dev_ioctl(eDEV_QEI1, eQEI_IOCTL_W_POS, 0); // zero the position

	dev_ioctl(eDEV_ESC0, eESC_IOCTL_SET_POWER, -100);

	//SysTick_Init(0x270FF); // send interrupt to NVIC every 2ms (500 Hz)
	SysTick_Init(0x61A7F); // send interrupt to NVIC every 5ms (200 Hz)
}



int main(void)
{

#ifdef __DEBUG__
	sandbox();
#else
	int32_t val = 0;

	dev_init(eDEV_PLL); // 80 MHz
	dev_init(eDEV_QEI0);
	dev_init(eDEV_QEI1);
	dev_init(eDEV_ESC0);
	dev_init(eDEV_PRS0);
	dev_init(eDEV_TIMER0);


	dev_ioctl(eDEV_QEI0, eQEI_IOCTL_W_POS, 0);
	dev_ioctl(eDEV_QEI1, eQEI_IOCTL_W_POS, 0);


	FPU_enable(1);
	UART_Init();


	/* wait until pendulum is upright to start control algorithm */
	/* TBD: will be replaced when swingup is implemented */
	SysCtlDelay(SysCtlClockGet()/3); // delay ~ 1 second
	dev_ioctl(eDEV_QEI0, eQEI_IOCTL_W_POS, 0x00000000);
	val = dev_ioctl(eDEV_QEI0, eQEI_IOCTL_R_POS);
	while( val <= 1199 && val >= -1199)
	{
		val = dev_ioctl(eDEV_QEI0, eQEI_IOCTL_R_POS);
	}
	dev_ioctl(eDEV_QEI0, eQEI_IOCTL_W_POS, 0x00000000);


	/* initialize SysTick timer */
	SysTick_Init(0x1f3f);// 10000 Hz
#endif

	while(1)
	{
		// loop forever
	}
}

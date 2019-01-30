/*
 * ssd1306_dev.c
 *
 *  Created on: Mar 15, 2018
 *      Author: vlazic
 */

#include "device.h"
#include "driverlib/sysctl.h"

#define SSD1306_DEFAULT_ADDRESS      0x78
#define SSD1306_SETCONTRAST          0x81
#define SSD1306_DISPLAYALLON_RESUME  0xA4
#define SSD1306_DISPLAYALLON         0xA5
#define SSD1306_NORMALDISPLAY        0xA6
#define SSD1306_INVERTDISPLAY        0xA7
#define SSD1306_DISPLAYOFF           0xAE
#define SSD1306_DISPLAYON            0xAF
#define SSD1306_SETDISPLAYOFFSET     0xD3
#define SSD1306_SETCOMPINS           0xDA
#define SSD1306_SETVCOMDETECT        0xDB
#define SSD1306_SETDISPLAYCLOCKDIV   0xD5
#define SSD1306_SETPRECHARGE         0xD9
#define SSD1306_SETMULTIPLEX         0xA8
#define SSD1306_SETLOWCOLUMN         0x00
#define SSD1306_SETHIGHCOLUMN        0x10
#define SSD1306_SETSTARTLINE         0x40
#define SSD1306_MEMORYMODE           0x20
#define SSD1306_COLUMNADDR           0x21
#define SSD1306_PAGEADDR             0x22
#define SSD1306_COMSCANINC           0xC0
#define SSD1306_COMSCANDEC           0xC8
#define SSD1306_SEGREMAP             0xA0
#define SSD1306_CHARGEPUMP           0x8D
#define SSD1306_SWITCHCAPVCC         0x2
#define SSD1306_NOP                  0xE3

// scrolling commands
#define SSD1306_DEACTIVATESCROLL     0x2E
#define SSD1306_ACTIVATESCROLL       0x2F
#define SSD1306_RIGHTHORIZONTAL      0x26
#define SSD1306_LEFTHORIZONTAL       0x27

#define SSD1306_COLS 128
#define SSD1306_ROWS 64
#define SSD1306_BUFFERSIZE (SSD1306_COLS*SSD1306_ROWS)/8
#define SLAVE_ADDR 0x3C


const uint8_t ASCII_8_5[][5] =  // ASCII 8x5 font
{
			{ 0x00, 0x00, 0x00, 0x00, 0x00 }, // 20 space
			{ 0x00, 0x00, 0x5f, 0x00, 0x00 }, // 21 !
			{ 0x00, 0x07, 0x00, 0x07, 0x00 }, // 22 "
			{ 0x14, 0x7f, 0x14, 0x7f, 0x14 }, // 23 #
			{ 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // 24 $
			{ 0x23, 0x13, 0x08, 0x64, 0x62 }, // 25 %
			{ 0x36, 0x49, 0x55, 0x22, 0x50 }, // 26 &
			{ 0x00, 0x05, 0x03, 0x00, 0x00 }, // 27 '
			{ 0x00, 0x1c, 0x22, 0x41, 0x00 }, // 28 (
			{ 0x00, 0x41, 0x22, 0x1c, 0x00 }, // 29 )
			{ 0x14, 0x08, 0x3e, 0x08, 0x14 }, // 2a *
			{ 0x08, 0x08, 0x3e, 0x08, 0x08 }, // 2b +
			{ 0x00, 0x50, 0x30, 0x00, 0x00 }, // 2c ,
			{ 0x08, 0x08, 0x08, 0x08, 0x08 }, // 2d -
			{ 0x00, 0x60, 0x60, 0x00, 0x00 }, // 2e .
			{ 0x20, 0x10, 0x08, 0x04, 0x02 }, // 2f /
			{ 0x3e, 0x51, 0x49, 0x45, 0x3e }, // 30 0
			{ 0x00, 0x42, 0x7f, 0x40, 0x00 }, // 31 1
			{ 0x42, 0x61, 0x51, 0x49, 0x46 }, // 32 2
			{ 0x21, 0x41, 0x45, 0x4b, 0x31 }, // 33 3
			{ 0x18, 0x14, 0x12, 0x7f, 0x10 }, // 34 4
			{ 0x27, 0x45, 0x45, 0x45, 0x39 }, // 35 5
			{ 0x3c, 0x4a, 0x49, 0x49, 0x30 }, // 36 6
			{ 0x01, 0x71, 0x09, 0x05, 0x03 }, // 37 7
			{ 0x36, 0x49, 0x49, 0x49, 0x36 }, // 38 8
			{ 0x06, 0x49, 0x49, 0x29, 0x1e }, // 39 9
			{ 0x00, 0x36, 0x36, 0x00, 0x00 }, // 3a :
			{ 0x00, 0x56, 0x36, 0x00, 0x00 }, // 3b ;
			{ 0x08, 0x14, 0x22, 0x41, 0x00 }, // 3c <
			{ 0x14, 0x14, 0x14, 0x14, 0x14 }, // 3d =
			{ 0x00, 0x41, 0x22, 0x14, 0x08 }, // 3e >
			{ 0x02, 0x01, 0x51, 0x09, 0x06 }, // 3f ?
			{ 0x32, 0x49, 0x79, 0x41, 0x3e }, // 40 @
			{ 0x7e, 0x11, 0x11, 0x11, 0x7e }, // 41 A
			{ 0x7f, 0x49, 0x49, 0x49, 0x36 }, // 42 B
			{ 0x3e, 0x41, 0x41, 0x41, 0x22 }, // 43 C
			{ 0x7f, 0x41, 0x41, 0x22, 0x1c }, // 44 D
			{ 0x7f, 0x49, 0x49, 0x49, 0x41 }, // 45 E
			{ 0x7f, 0x09, 0x09, 0x09, 0x01 }, // 46 F
			{ 0x3e, 0x41, 0x49, 0x49, 0x7a }, // 47 G
			{ 0x7f, 0x08, 0x08, 0x08, 0x7f }, // 48 H
			{ 0x00, 0x41, 0x7f, 0x41, 0x00 }, // 49 I
			{ 0x20, 0x40, 0x41, 0x3f, 0x01 }, // 4a J
			{ 0x7f, 0x08, 0x14, 0x22, 0x41 }, // 4b K
			{ 0x7f, 0x40, 0x40, 0x40, 0x40 }, // 4c L
			{ 0x7f, 0x02, 0x0c, 0x02, 0x7f }, // 4d M
			{ 0x7f, 0x04, 0x08, 0x10, 0x7f }, // 4e N
			{ 0x3e, 0x41, 0x41, 0x41, 0x3e }, // 4f O
			{ 0x7f, 0x09, 0x09, 0x09, 0x06 }, // 50 P
			{ 0x3e, 0x41, 0x51, 0x21, 0x5e }, // 51 Q
			{ 0x7f, 0x09, 0x19, 0x29, 0x46 }, // 52 R
			{ 0x46, 0x49, 0x49, 0x49, 0x31 }, // 53 S
			{ 0x01, 0x01, 0x7f, 0x01, 0x01 }, // 54 T
			{ 0x3f, 0x40, 0x40, 0x40, 0x3f }, // 55 U
			{ 0x1f, 0x20, 0x40, 0x20, 0x1f }, // 56 V
			{ 0x3f, 0x40, 0x38, 0x40, 0x3f }, // 57 W
			{ 0x63, 0x14, 0x08, 0x14, 0x63 }, // 58 X
			{ 0x07, 0x08, 0x70, 0x08, 0x07 }, // 59 Y
			{ 0x61, 0x51, 0x49, 0x45, 0x43 }, // 5a Z
			{ 0x00, 0x7f, 0x41, 0x41, 0x00 }, // 5b [
			{ 0x02, 0x04, 0x08, 0x10, 0x20 }, // 5c backslash
			{ 0x00, 0x41, 0x41, 0x7f, 0x00 }, // 5d ]
			{ 0x04, 0x02, 0x01, 0x02, 0x04 }, // 5e ^
			{ 0x40, 0x40, 0x40, 0x40, 0x40 }, // 5f _
			{ 0x00, 0x01, 0x02, 0x04, 0x00 }, // 60 `
			{ 0x20, 0x54, 0x54, 0x54, 0x78 }, // 61 a
			{ 0x7f, 0x48, 0x44, 0x44, 0x38 }, // 62 b
			{ 0x38, 0x44, 0x44, 0x44, 0x20 }, // 63 c
			{ 0x38, 0x44, 0x44, 0x48, 0x7f }, // 64 d
			{ 0x38, 0x54, 0x54, 0x54, 0x18 }, // 65 e
			{ 0x08, 0x7e, 0x09, 0x01, 0x02 }, // 66 f
			{ 0x0c, 0x52, 0x52, 0x52, 0x3e }, // 67 g
			{ 0x7f, 0x08, 0x04, 0x04, 0x78 }, // 68 h
			{ 0x00, 0x44, 0x7d, 0x40, 0x00 }, // 69 i
			{ 0x20, 0x40, 0x44, 0x3d, 0x00 }, // 6a j
			{ 0x7f, 0x10, 0x28, 0x44, 0x00 }, // 6b k
			{ 0x00, 0x41, 0x7f, 0x40, 0x00 }, // 6c l
			{ 0x7c, 0x04, 0x18, 0x04, 0x78 }, // 6d m
			{ 0x7c, 0x08, 0x04, 0x04, 0x78 }, // 6e n
			{ 0x38, 0x44, 0x44, 0x44, 0x38 }, // 6f o
			{ 0x7c, 0x14, 0x14, 0x14, 0x08 }, // 70 p
			{ 0x08, 0x14, 0x14, 0x18, 0x7c }, // 71 q
			{ 0x7c, 0x08, 0x04, 0x04, 0x08 }, // 72 r
			{ 0x48, 0x54, 0x54, 0x54, 0x20 }, // 73 s
			{ 0x04, 0x3f, 0x44, 0x40, 0x20 }, // 74 t
			{ 0x3c, 0x40, 0x40, 0x20, 0x7c }, // 75 u
			{ 0x1c, 0x20, 0x40, 0x20, 0x1c }, // 76 v
			{ 0x3c, 0x40, 0x30, 0x40, 0x3c }, // 77 w
			{ 0x44, 0x28, 0x10, 0x28, 0x44 }, // 78 x
			{ 0x0c, 0x50, 0x50, 0x50, 0x3c }, // 79 y
			{ 0x44, 0x64, 0x54, 0x4c, 0x44 }, // 7a z
			{ 0x00, 0x08, 0x36, 0x41, 0x00 }, // 7b {
			{ 0x00, 0x00, 0x7f, 0x00, 0x00 }, // 7c |
			{ 0x00, 0x41, 0x36, 0x08, 0x00 }, // 7d }
			{ 0x10, 0x08, 0x08, 0x10, 0x08 }, // 7e ~
			{ 0x00, 0x00, 0x00, 0x00, 0x00 } // 7f
};

/* ssd1306 device operations prototypes */
void ssd1306_dev_init (void *self_attr);
int  ssd1306_dev_write (void *self_attr, const char *buf, size_t count);
int  ssd1306_dev_read (void *self_attr, char *buf, size_t count);
int  ssd1306_dev_iotcl (void *self_attr, int request, va_list args);
int  ssd1306_dev_lseek (void *self_attr, int offset, int whence);
void ssd1306_dev_deinit (void *self_attr);

/* ssd1306 helper function prototypes */
static void sendCmd (uint8_t cmd);
static void sendData (uint8_t data);
static void clearDisplay(void);
static void putChar (uint8_t ch);


struct device_operations ssd1306_devops = {
		.dev_init_r = ssd1306_dev_init,
		.dev_write_r = ssd1306_dev_write,
		.dev_read_r = ssd1306_dev_read,
		.dev_ioctl_r = ssd1306_dev_iotcl,
		.dev_lseek_r = ssd1306_dev_lseek,
		.dev_deinit_r = ssd1306_dev_deinit,
};




struct ssd1306_attr
{
	uint32_t offset;
};

struct ssd1306_attr ssd1306_attr = { .offset = 0 };



struct device ssd1306_dev = {
		.name = "ssd1306",
		.self_attr = &ssd1306_attr,
		.dev_ops = &ssd1306_devops
};



static void sendCmd (uint8_t cmd)
{
	// set slave address
	I2C2_MSA_R = SLAVE_ADDR << 1;

	// write CONTROL BYTE to data register
	I2C2_MDR_R = 0x00;
	// start transmission: STOP=0, START=1, RUN=1
	I2C2_MCS_R = 0x03;

	// wait until master is ready (BUSY = 0)
	while((I2C2_MCS_R & 0x00000001) == 0x00000001) {};

	// write command to data register
	I2C2_MDR_R = cmd;
	// start transmission: STOP=1, START=0, RUN=1
	I2C2_MCS_R = 0x05;

	// wait until master is ready (BUSY = 0)
	while((I2C2_MCS_R & 0x00000001) == 0x00000001) {};
}


static void sendData (uint8_t data)
{
	// set slave address
	I2C2_MSA_R = SLAVE_ADDR << 1;

	// write CONTROL BYTE to data register
	I2C2_MDR_R = 0x40;
	// start transmission: STOP=0, START=1, RUN=1
	I2C2_MCS_R = 0x03;

	// wait until master is ready (BUSY = 0)
	while((I2C2_MCS_R & 0x00000001) == 0x00000001) {};

	// write command to data register
	I2C2_MDR_R = data;
	// start transmission: STOP=1, START=0, RUN=1
	I2C2_MCS_R = 0x05;

	// wait until master is ready (BUSY = 0)
	while((I2C2_MCS_R & 0x00000001) == 0x00000001) {};

	// increment offset counter
	if (ssd1306_attr.offset == SSD1306_BUFFERSIZE - 1)
		ssd1306_attr.offset = 0;
	else
		ssd1306_attr.offset += 1;
}


static void clearDisplay(void)
{
	uint32_t i;

	for (i = 0; i < SSD1306_BUFFERSIZE; i++)
	{
		sendData(0x00);
	}
}


static void putChar (uint8_t ch)
{
	uint32_t i;

	for(i = 0; i < 5; i++)
		sendData( ASCII_8_5[ch - 0x20][i]);

	sendData(0x00);
}



void ssd1306_dev_init (void *self_attr)
{
	/* Pin Configurations:
	 *
	 * I2C2SCL PE4(3) - clock line;has active pull-up; DISABLE open-drain configuration
	 *
	 * I2C2SDA PE5(3) - data line
	 *
	 * GPIO D/OUT PA6 - reset
	 */

	// enable system peripheral GPIO Port E
	SYSCTL_RCGCGPIO_R |= 0x00000010;
	// wait until clock is stable
	while((SYSCTL_PRGPIO_R & 0x00000010) == 0) {};

	// enable system peripheral GPIO Port A
	SYSCTL_RCGCGPIO_R |= 0x00000001;
	// wait until clock is stable
	while((SYSCTL_PRGPIO_R & 0x00000001) == 0) {};

	// enable system peripheral I2C 2
	SYSCTL_RCGCI2C_R |= 0x00000004;
	// wait until clock is stable
	while((SYSCTL_PRI2C_R & 0x00000004) == 0) {};

	// ===== START_CONFIG: PA5 as D/OUT =====
	// unlock GPIO Port A GPIOCR
	GPIO_PORTA_LOCK_R = 0x4C4F434B;
	// enable reconfiguration of PA 0-7
	GPIO_PORTA_CR_R = 0x000000FF;
	// disable alternate function on PA5
	GPIO_PORTA_AFSEL_R &= ~(0x00000020);
	// configure PA5 direciton as OUT
	GPIO_PORTA_DIR_R |= 0x00000020;
	// disable open-drain configuration on PA5
	GPIO_PORTA_ODR_R &= ~(0x00000020);
	// enable pull-down resistor on PA5
	GPIO_PORTA_PDR_R |= 0x00000020;
	// enable digital function on PA5
	GPIO_PORTA_DEN_R |= 0x00000020;
	// set PA5 level to HIGH (remains high for normal operation)
	GPIO_PORTA_DATA_R |= 0x00000020;
	// ===== STOP_CONFIG =====

	// ===== START_CONFIG: PE4 as I2C_2_SCL =====
	// configure PE4 as D/OUT
	GPIO_PORTE_DIR_R |= 0x00000010;
	// enable alternate function on PE4
	GPIO_PORTE_AFSEL_R |= 0x00000010;
	// select alternate function 3 (SDA)
	GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R & ~(0x000F0000)) | 0x00030000;
	// disable open-drain configuration on PE4
	GPIO_PORTE_ODR_R &= ~(0x00000010);
	// enable internal pull-up on PE4
	GPIO_PORTE_PUR_R |= 0x00000010;
	// enable digital function on PE4
	GPIO_PORTE_DEN_R |= 0x00000010;
	// ===== STOP_CONFIG =====


	// ===== START_CONFIG: PE5 as I2C_2_SDA =====
	// configure PE5 as D/OUT
	GPIO_PORTE_DIR_R |= 0x00000020;
	// enable alternate function on PE5
	GPIO_PORTE_AFSEL_R |= 0x00000020;
	// select alternate function 3 (SCL)
	GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R & ~(0x00F00000)) | 0x00300000;
	// enable open-drain configuration on PE5
	GPIO_PORTE_ODR_R |= 0x00000020;
	// enable internal pull-up resistor
	GPIO_PORTE_PUR_R |= 0x00000020;
	// enable digital function on PE5
	GPIO_PORTE_DEN_R |= 0x00000020;
	// ===== STOP_CONFIG =====


	// ===== START_CONFIG: I2C 2 =====
	// enable master mode on I2C_2
	I2C2_MCR_R |= 0x00000010;
	// disable high speed mode
	I2C2_MTPR_R = 0x27; // 100 Kbps; check datasheet p. 1016 for formula
	// ===== STOP_CONFIG =====


	// SSD1306 controller intialization procedure
	// drive PA5 LOW
	GPIO_PORTA_DATA_R &= ~(0x00000020);
	SysCtlDelay(200000);
	// drive PA5 HIGH
	GPIO_PORTA_DATA_R |= 0x00000020;
	SysCtlDelay(200000);

	sendCmd(SSD1306_DISPLAYOFF);

	sendCmd(SSD1306_SETDISPLAYCLOCKDIV);
	sendCmd(0x80);

	sendCmd(SSD1306_SETMULTIPLEX);
	sendCmd(0x3F);

	sendCmd(SSD1306_SETDISPLAYOFFSET);
	sendCmd(0x00);

	sendCmd(SSD1306_SETSTARTLINE  | 0x00);

	sendCmd(SSD1306_CHARGEPUMP);
	sendCmd(0x14);

	sendCmd(SSD1306_MEMORYMODE);
	sendCmd(0x00); // horizontal

	sendCmd(SSD1306_SEGREMAP | 0x01);

	sendCmd(SSD1306_COMSCANDEC);

	sendCmd(SSD1306_SETCOMPINS);
	sendCmd(0x12);

	sendCmd(SSD1306_SETCONTRAST);
	sendCmd(0x8F);

	sendCmd(SSD1306_SETPRECHARGE);
	sendCmd(0xF1);

	sendCmd(SSD1306_SETVCOMDETECT);
	sendCmd(0x40);

	sendCmd(SSD1306_DISPLAYALLON_RESUME);

	sendCmd(SSD1306_NORMALDISPLAY);

	sendCmd(SSD1306_DISPLAYON);

	// send some test characters
	clearDisplay();

}


int  ssd1306_dev_write (void *self_attr, const char *buf, size_t count)
{
	char *pbuf = (char *)buf;
	uint32_t i = 0;
	/* not implemented */
	while(pbuf != '\0' && i < count)
	{
		putChar(*pbuf);
		pbuf++;
		i++;
	}

	return i;
}


int  ssd1306_dev_read (void *self_attr, char *buf, size_t count)
{
	/* not implemented */
	return -1;
}


int  ssd1306_dev_iotcl (void *self_attr, int request, va_list args)
{
	int rv = 0;

	switch(request)
	{
	case eSSD1306_IOCTL_CLEARALL:
		clearDisplay();
		break;
	default:
		break;
	}

	return rv;
}


int  ssd1306_dev_lseek (void *self_attr, int offset, int whence)
{
	uint32_t page;
	uint32_t index;
	int rv = 0;

	page = (offset / SSD1306_COLS) % SSD1306_ROWS;
	index = offset % SSD1306_COLS;

	switch(whence)
	{
	case DEV_SEEK_SET:
		// switch to page adressing mode
		sendCmd(SSD1306_MEMORYMODE);
		sendCmd(0x02);

		// set page address (0-7)
		sendCmd(0xB0 | page);

		// set index (column address)
		sendCmd(0x00 | (index & 0x0F));
		sendCmd(0x10 | (index >> 4));

		// switch back to horizontal adressing mode
		sendCmd(SSD1306_MEMORYMODE);
		sendCmd(0x00);

		rv = (page * SSD1306_COLS) + index;
		ssd1306_attr.offset = rv;

		break;

	case DEV_SEEK_CUR:
		/* not implemented */
		break;

	case DEV_SEEK_END:
		/* not implemented */
		break;

	default:
		break;
	}

	return rv;
}


void ssd1306_dev_deinit (void *self_attr)
{
	/* not implemented */
}






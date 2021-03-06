/**
  * @file spidev.c
  *
  * Copyright 2019 OPEN-EYES S.r.l.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  **/

#include <stdint.h>
#include <unistd.h>
/**
 * @file spidev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include "spidev.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static char spi_file[64];
static int spi_initialized;
static int spi_id;

struct spi_ioc_transfer xfer[2];

#define SPI_ITF_SPEED 20000000

//////////
// Init SPIdev
//////////
int spi_open(char *dev)
{
	int fd;
    uint8_t    	mode, lsb, bits;
    uint32_t 	speed=SPI_ITF_SPEED;

	strcpy(spi_file,dev);
	spi_initialized=0;

	if ((fd = open(spi_file,O_RDWR)) < 0)
		return ENOENT;

	if (ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0)
		return EACCES;

	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
		return EAGAIN;

	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
		return EAGAIN;

	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0)
		return EAGAIN;

	spi_id=fd;

    //printf("%s: spi mode %d, %d bits %sper word, %d Hz max file desc=%d\n",spi_file, mode, bits, lsb ? "(lsb first) " : "", speed,spi_id);

    xfer[0].len = 3; /* Length of  command to write*/
    xfer[0].cs_change = 0; /* Keep CS activated */
    xfer[0].delay_usecs = 0, //delay in us
    xfer[0].speed_hz = SPI_ITF_SPEED, //speed
    xfer[0].bits_per_word = 8, // bites per word 8

    //xfer[1].rx_buf = (unsigned long) buf2;
    xfer[1].len = 4; /* Length of Data to read */
    xfer[1].cs_change = 0; /* Keep CS activated */
    xfer[0].delay_usecs = 0;
    xfer[0].speed_hz = SPI_ITF_SPEED;
    xfer[0].bits_per_word = 8;

    spi_initialized=1;

    return 0;
}

void spi_close(void)
{
	close(spi_id);
	spi_initialized=0;
}
//////////
// Read n bytes from the 2 bytes add1 add2 address
//////////

char spi_read(int nbytes, char cmd, char *rxbuf)
{
    unsigned char   buf[64];
    int status;

    if(!spi_initialized)
    	return -1;

    if(nbytes>64)
     {
 		//perror("SPI_NBYTES_ERROR");
 		return -1;
 	}

    memset(buf, 0, sizeof buf);
    buf[0] = cmd;
    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = 1; /* Length of  command to write*/
    xfer[1].rx_buf = (unsigned long) rxbuf;
    xfer[1].len = nbytes; /* Length of Data to read */
    status = ioctl(spi_id, SPI_IOC_MESSAGE(2), xfer);
    if (status < 0)
	{
    	//perror("SPI_IOC_MESSAGE");
    	return -1;
	}
    return 0;
}

//////////
// Write n bytes int the 2 bytes address add1 add2
//////////
int spi_write(uint16_t nbytes, uint8_t *pt)
{
    unsigned char   buf[64];
    int status;

    if(!spi_initialized)
    	return -1;

    if(nbytes>64)
    {
		//perror("SPI_NBYTES_ERROR");
		return -1;
	}

    memset(buf, 0, 64);
    memcpy(buf,pt,nbytes);

    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = nbytes; /* Length of  command to write*/
    status = ioctl(spi_id, SPI_IOC_MESSAGE(1), xfer);
    if (status < 0)
	{
		//printf("WRITE SPI_IOCTL_MESSAGE file desc=%d",spi_id);
		return -1;
	}
    return 0;
}

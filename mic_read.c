// Author: Connor Hoang 
#include <stdio.h> 

#include <fcntl.h> 
#include <unistd.h> 

#include <sys/ioctl.h> 
#include <linux/i2c-dev.h> 

#include "mic_read.h"

int mic_read(char addr) { 

	// open i2c
	int fd = open("/dev/i2c-1", O_RDWR); 
	if (fd < 0) { 
		perror("Failed to open I2C bus"); 
		return 1; 
	}
	
	// connect to ADC
	if (ioctl(fd, I2C_SLAVE, addr) < 0) { 
		perror("Failed to connect to ADC"); 
		return 1; 
	}
	
	unsigned char control = 0x84; 
	unsigned char value; 
		
	write(fd, &control, 1); 
	read(fd, &value, 1); 
	read(fd, &value, 1); 

	close(fd);
	return value;
} 
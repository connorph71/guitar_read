// Author: Connor Hoang 
#include <stdio.h> 

#include <fcntl.h> 
#include <unistd.h> 

#include <sys/ioctl.h> 
#include <linux/i2c-dev.h> 

#include "mic_read.h"

int mic_read(int fd) { 
	
	unsigned char control = 0x84; 
	unsigned char value; 
		
	write(fd, &control, 1); 
	read(fd, &value, 1); 
	read(fd, &value, 1); 

	return value;
} 
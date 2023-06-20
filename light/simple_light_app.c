#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "simple_motor.h"

#define SPI_LIGHT_CHANNEL 1
#define MCP_DATA_TRANSFER _IOWR('x', 0x80, unsigned long*)

int spi_fd, motor_fd;
int is_curtain_set;

int main(int argc, char *argv[]) {
	char spi_data[11] =""; /* 12bits for the signal! */
	int ret = 0, light_value = 0;

	spi_data[0] = SPI_LIGHT_CHANNEL; // light sensor uses channel 1

	/* open device drivers */
	if((spi_fd = open("/dev/mcp3208dev0.0", O_RDWR)) < 0) {
		perror("mcp3208 dev open error\n");
		return 0;
	}

	if((motor_fd = open("/dev/simple_motor_dev", O_RDWR)) < 0) {
		perror("motor dev open error\n");
		return 0;
	}
	
	while(1) {
		spi_data[0] = SPI_LIGHT_CHANNEL;
		ret = read(spi_fd, spi_data, 11);
		light_value = atoi(spi_data);
		printf("light value: %d\n", light_value);

		if(!is_curtain_set && light_value < 500) {
			is_curtain_set = 1;
			printf("curtain set!\n\n");
			ioctl(motor_fd, MOTOR_ROTATE_FORWARD, 1500);
		} else if(is_curtain_set && light_value >= 500) {
			is_curtain_set = 0;
			printf("curtain cleared!\n\n");
			ioctl(motor_fd, MOTOR_ROTATE_BACKWARD, 1500);
		}
		sleep(2);
	}
	
	close(spi_fd);
	close(motor_fd);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "gas_mod.h"
#include "alert_mod.h"

int main(void) {
    int spi_fd, ret, alert_fd, gas_fd, value;
    char data[11];
    if ((gas_fd = open("/dev/gas_dev", O_RDWR)) < 0) {
        printf("open /dev/gas_dev error \n");
        exit(1);
    }
    if ((alert_fd = open("/dev/alert_dev", O_RDWR)) < 0) {
        printf("open /dev/alert_dev error \n");
        exit(1);
    }
    if ((spi_fd = open("/dev/mcp3208dev0.0", O_RDWR)) < 0) {
        printf("open /dev/mcp3208dev0.0 error \n");
        exit(1);
    }

    while (1)
    {
        ioctl(gas_fd, WAIT, NULL);
        ioctl(alert_fd, PLAY, NULL);
        ioctl(alert_fd, ON, NULL);
        while (1)
        {
            data[0] = 0;
            ret = read(spi_fd, data, 11);
            value = atoi(data);
            printf("sensor = %d, length = %d\n", atoi(data), ret);
            if (value < 500)  {
                break;
            }
        }
        ioctl(alert_fd, STOP, NULL);
        ioctl(alert_fd, OFF, NULL);
    }
    
}

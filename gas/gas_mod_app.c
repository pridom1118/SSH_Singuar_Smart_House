#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
    int spi_fd, ret;
    char data[5];
    if ((spi_fd = open("/dev/gasdev0.0", O_RDWR)) < 0) {
        printf("open /dev/gasdev error \n");
        exit(1);
    }
    ret = read(spi_fd, data, 5);
    printf("sensor = %d, length = %d\n", atoi(data), ret);
}
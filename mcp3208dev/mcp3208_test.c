#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define MCP_DATA_TRANSFER _IOWR('x', 0x80, unsigned long*)

int main(int argc, char* argv[]) {
    int spi_fd, ret;
    char data[11];

    if (argc != 2) {
        printf(" neet argument for chennel \n");
        return -1;
    }

    if ((spi_fd = open("/dev/mcp3208dev0.0", O_RDWR)) < 0) {
        perror("open /dev/mcp3208dev0.0 error \n");
        return -1;
    }
    data[0] = atoi(argv[1]);
    ret = ioctl(spi_fd, MCP_DATA_TRANSFER, atoi(argv[1]));
    printf("sensor = %d, length = %d\n", atoi(data), ret);
    return 0;
}

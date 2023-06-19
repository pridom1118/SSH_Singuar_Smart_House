#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#include "alert_mod.h"

int main(int argc, char *argv[]) {
    int fd;
    if ((fd = open("/dev/alert_dev", O_RDWR)) < 0 ) {
        printf("can not open alert_dev");
        return -1;
    }
    ioctl(fd, PLAY, NULL);
    ioctl(fd, ON, NULL);
    sleep(5);
    ioctl(fd, STOP, NULL);
    ioctl(fd, OFF, NULL);
    return 0;
}
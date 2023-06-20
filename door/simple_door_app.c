#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "simple_door.h"

int door_fd, alert_fd;

time_t timer;
struct tm *t;

void print_current_time() {
	t = localtime(&timer);
	char msg[100] = {0, };

	sprintf(msg, "Door opened at [%04d/%02d/%02d] %02d:%02d\n",
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			t->tm_hour,
			t->tm_min);
	printf("%s", msg);
}

int main(int argc, char *argv[]) {
	
	if((door_fd = open("/dev/simple_door_dev", O_RDWR)) < 0) {
		perror("smart door dev open error\n");
		return -1;
	}

	if((alert_fd = open("/dev/alert_dev", O_RDWR)) < 0) {
		perror("alert dev open error\n");
		return -1;
	}

	timer = time(NULL);

	/* Wait for the door to be opened */
	while(1) {
		ioctl(door_fd, WAIT_OPEN, NULL);
		print_current_time();
		ioctl(alert_fd, PLAY, NULL);
		sleep(1);
		ioctl(alert_fd, STOP, NULL);
	}

	close(door_fd);
	close(alert_fd);
	return 0;
}

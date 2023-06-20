obj-m := simple_door.o

KDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

default:
		$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
		arm-linux-gnueabi-gcc -o simple_door_app simple_door_app.c
debug:
		$(MAKE) EXTRA_CFLAGS=-DMY_DEBUG -C $(KDIR) M=$(PWD) $(ARM) modules
		arm-linux-gnueabi-gcc -o simple_door_app simple_door_app.c
clean:
		$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
		rm simple_door_app
scp:
		scp simple_door_app mknod_door.sh simple_door.ko pi@10.10.10.12:~/

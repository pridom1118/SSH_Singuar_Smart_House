obj-m := simple_motor.o

KDIR :=~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

default:
		$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
		arm-linux-gnueabi-gcc simple_light_app.c -o simple_light_app
clean:
		$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
		rm simple_light_app
scp:
		scp $(obj-m:.o=.ko) mknod_motor.sh simple_light_app pi@10.10.10.12:~/

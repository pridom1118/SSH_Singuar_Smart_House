obj-m := mcp3208_spi_dev.o

KERNELDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(ARM) modules
debug:
	$(MAKE) EXTRA_CFLAGS=-DMY_DEBUG -C $(KERNELDIR) M=$(PWD) $(ARM) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(ARM) clean

obj-m := gas_mod.o

KERNELDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc $(obj-m:.o=_app.c) -o $(obj-m:.o=_app)
debug:
	$(MAKE) EXTRA_CFLAGS=-DMY_DEBUG -C $(KERNELDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc $(obj-m:.o=_app.c) -o $(obj-m:.o=_app)
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(ARM) clean
	rm $(obj-m:.o=_app)
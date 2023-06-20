#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/gpio.h>

#include "simple_door.h"

#ifdef MY_DEBUG
		#define DEBUG_MSG(fmt, args...) \
				printk(KERN_DEBUG "LSWITCH: " fmt, ##args);
#else
		#define DEBUG_MSG(fmt, args...)
#endif

MODULE_LICENSE("GPL");

#define DEV_NAME "simple_door_dev"

#define LSWITCH 12

static dev_t dev_num;
static struct cdev *cd_cdev;
static int irq_num, is_door_open;

wait_queue_head_t door_wq;

static irqreturn_t lswitch_irq_isr(int irq, void *dev_id) {
	DEBUG_MSG("INTERRUPT - Door open\n");

	is_door_open = 1;
	wake_up(&door_wq);
	return IRQ_HANDLED;
}

static long simple_door_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int ret = -1;
	unsigned long flags;

	switch(cmd) {
		case WAIT_OPEN:		
			ret = wait_event_interruptible(door_wq, is_door_open != 0);

			if(ret < 0) return ret;


			local_irq_save(flags);
			is_door_open = 0;
			local_irq_restore(flags);
			return 1;
		default:
			return -1;
	}
		return 0;
}

static int simple_door_open(struct inode *inode, struct file *file) {
	DEBUG_MSG("open\n");
	return 0;
}

static int simple_door_release(struct inode *inode, struct file *file) {
	DEBUG_MSG("release\n");
	return 0;
}

struct file_operations simple_door_fops = {
	.unlocked_ioctl = simple_door_ioctl,
	.open = simple_door_open,
	.release = simple_door_release,
};

static int __init simple_door_init(void) {
	int ret = 0;

	DEBUG_MSG("init\n");
	gpio_request_one(LSWITCH, GPIOF_IN, "LSWITCH");

	irq_num = gpio_to_irq(LSWITCH);
	ret = request_irq(irq_num, lswitch_irq_isr, IRQF_TRIGGER_FALLING, "lswitch_irq", NULL);

	if(ret) {
		DEBUG_MSG("Unable to reset IRQ for lswitch\n");
		free_irq(irq_num, NULL);
	} else {
		DEBUG_MSG("Enable to set IRQ for lswitch!\n");
	}

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &simple_door_fops);
	cdev_add(cd_cdev, dev_num, 1);

	init_waitqueue_head(&door_wq);

	return 0;
}

static void __exit simple_door_exit(void) {
	DEBUG_MSG("exit\n");

	gpio_free(LSWITCH);
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(simple_door_init);
module_exit(simple_door_exit);

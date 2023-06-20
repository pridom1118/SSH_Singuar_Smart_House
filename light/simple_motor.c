#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/cdev.h>

#include "simple_motor.h"

#ifdef LIGHT_DEBUG
	#define DEBUG_MSG(fmt, args...) \
		printk(KERN_DEBUG "simple_motor: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

MODULE_LICENSE("GPL");

/* GPIO Pins */
#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

/* Motor config */
#define STEPS 8
#define ONEROUND 512
#define DEV_NAME "simple_motor_dev"

int blue[8]   = {1, 1, 0, 0, 0, 0, 0, 1};
int pink[8]   = {0, 1, 1, 1, 0, 0, 0, 0};
int yellow[8] = {0, 0, 0, 1, 1, 1, 0, 0};
int orange[8] = {0, 0, 0, 0, 0, 1, 1, 1};

void setstep(int p1, int p2, int p3, int p4) {
	gpio_set_value(PIN1, p1);
	gpio_set_value(PIN2, p2);
	gpio_set_value(PIN3, p3);
	gpio_set_value(PIN4, p4);
}

void backward(int round, int delay) {
	int i = 0, j = 0;

	for(i = 0; i < ONEROUND * round; i++) {
		for(j = STEPS; j > 0; j--) {
			setstep(blue[j], pink[j], yellow[j], orange[j]);
			udelay(delay);
		}
	}
	setstep(0,0,0,0);
}

void forward(int round, int delay) {
	int i = 0, j = 0;

	for(i = 0; i < ONEROUND * round; i++) {
		for(j = 0; j < STEPS; j++) {
			setstep(blue[j], pink[j], yellow[j], orange[j]);
			udelay(delay);
		}
	}
	setstep(0,0,0,0);
}

static int simple_motor_open(struct inode *inode, struct file *file) {
	printk("motor: open\n");
	return 0;
}

static int simple_motor_release(struct inode *inode, struct file *file) {
	printk("motor: release\n");
	return 0;
}

static long simple_mioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	switch(cmd) {
		case MOTOR_ROTATE_FORWARD:
			forward(1, arg);
			break;
		case MOTOR_ROTATE_BACKWARD:
			backward(1, arg);
			break;
		default:
			return -1;
	}
	return 0;
}

struct file_operations simple_mioctl_fops = {
	.open = simple_motor_open,
	.release = simple_motor_release,
	.unlocked_ioctl = simple_mioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_motor_init(void) {
	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

	printk("motor: module init, GPIOs set.\n");
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &simple_mioctl_fops);
	cdev_add(cd_cdev, dev_num, 1);
	return 0;
}

static void __exit simple_motor_exit(void) {
	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);

	printk("motor: module exit, GPIOs free.\n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(simple_motor_init);
module_exit(simple_motor_exit);

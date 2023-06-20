#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include "simple_speaker_dev"

MODULE_LICENSE("GPL");

#define SPEAKER 16
#define DEV_NAME "simple_speaker_dev"

#ifdef SPEAKER_DEBUG
	#define DEBUG_MSG(fmt, args...) \
		printk(KERN_DEBUG "simple_speaker: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

static dev_t dev_num;
static struct cdev *cd_cdev;

static void play(int note) {
	int i = 0;
	for(i = 0; i < 100; i++) {
		gpio_set_value(SPEAKER, 1);
		udelay(note);
		gpio_set_value(SPEAKER, 0);
		udelay(note);
	}
}

static int simple_speaker_open(struct inode *inode, struct file *file) {

	return 0;
}

static int simple_speaker_release(struct inode *inode, struct file *file) {
	
	return 0;
}

static l



static int __init simple_speaker_init(void) {
	

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include "alert_mod.h"

#define LED 17
#define SPEAKER 27

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...)\
		printk(KERN_DEBUG "alert: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

#define DEV_NAME "alert_dev"

const struct gpio gpios[2] = {
    {LED, GPIOF_OUT_INIT_LOW, "LED"},
    {SPEAKER, GPIOF_OUT_INIT_LOW, "SPEAKER"}
};

static dev_t dev_num;
static struct cdev *cd_cdev;
spinlock_t my_lock;

int play(void *dats) {
    int i = 0;
    while ( !kthread_should_stop() ) {
        for (i = 0 ; i < 100; i ++ ){
            gpio_set_value(SPEAKER, 1);
            udelay(1136);
            gpio_set_value(SPEAKER, 0);
            udelay(1136);
        }
        msleep(1);
    }
    return 0;
}

struct task_struct *my_kthread = NULL;

static long alert_mod_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    
    switch(cmd) {
        case PLAY:
            DEBUG_MSG("play speaker\n");
            if(my_kthread) {
                kthread_stop(my_kthread);
                DEBUG_MSG("kernel thread STOP \n");
            }
            spin_lock(&my_lock);
            my_kthread = kthread_create(play, NULL, "PLAY ALERT SOUND");
            if(IS_ERR(my_kthread)) {
                my_kthread = NULL;
                DEBUG_MSG("kernel thread ERROR \n");
            }
            spin_unlock(&my_lock);
            wake_up_process(my_kthread);
            break;
        case STOP:
            DEBUG_MSG("stop speaker\n");
            spin_lock(&my_lock);
            if(my_kthread) {
                kthread_stop(my_kthread);
                printk("simple_kthread: my kernel thread STOP \n");
                my_kthread = NULL;
            }
            spin_unlock(&my_lock);
            break;
        case ON:
            gpio_set_value(LED, 1);
            break;
        case OFF:
            gpio_set_value(LED, 0);
            break;
        default:
            return -1;
    }
    return 0;
}

static int alert_mod_open(struct inode *inode, struct file *file) {
    return 0;
}

static int alert_mod_release(struct inode *inode, struct file *file) {
    return 0;
}

struct file_operations alert_mod_fops = {
    .unlocked_ioctl = alert_mod_ioctl,
    .open = alert_mod_open,
    .release = alert_mod_release
};

static int __init alert_mod_init(void) {
    gpio_request_array(gpios, 2);
    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &alert_mod_fops);
    cdev_add(cd_cdev, dev_num, 1);
    spin_lock_init(&my_lock);

    return 0;
}

static void __exit alert_mod_exit(void) {
    gpio_free_array(gpios, 2);
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(alert_mod_init);
module_exit(alert_mod_exit);

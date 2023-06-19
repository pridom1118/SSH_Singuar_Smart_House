#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include "gas_mod.h"

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...)\
		printk(KERN_DEBUG "gas_detection: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

#define MQ9_DIGITAL_OUT 22
#define LED 17
#define DEV_NAME "gas_dev"


static dev_t dev_num;
static struct cdev *cd_cdev;

static int my_data;
spinlock_t my_lock;
wait_queue_head_t my_wq;

static int irq_num;

static irqreturn_t mq9_irq_isr(int irq, void* dev_id) {
    unsigned long flags;
    DEBUG_MSG("GAS DETECT Wake up all\n");
    gpio_set_value(LED, 1);
    spin_lock_irqsave(&my_lock, flags);
    my_data = 1;
    spin_unlock_irqrestore(&my_lock, flags);
    wake_up_interruptible_all(&my_wq);
    return IRQ_HANDLED;
}

static long gas_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret;

    switch (cmd) {
        case WAIT:
            DEBUG_MSG("Process %i (%s) sleep\n", current->pid, current->comm);
            ret = wait_event_interruptible(my_wq, my_data > 0);
            if (ret < 0)
                return ret;
            break;
    }

    my_data--;

    return 0;
}

static int gas_open(struct inode *inode, struct file *file) {
    return 0;
}

static int gas_release(struct inode *inode, struct file *file) {
    return 0;
}

struct file_operations gas_fops = {
    .unlocked_ioctl = gas_ioctl,
    .open = gas_open,
    .release = gas_release
};

static int __init gas_init(void) {
    int ret;
    DEBUG_MSG("Init Module\n");

    gpio_request_one(MQ9_DIGITAL_OUT, GPIOF_IN, "MQ9_DIGITAL_OUT");
    gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "LED");

    irq_num = gpio_to_irq(MQ9_DIGITAL_OUT);
    ret = request_irq(irq_num, mq9_irq_isr, IRQF_TRIGGER_FALLING, "mq9_irq_isr", NULL);
    if (ret) {
        free_irq(irq_num, NULL);
    }

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &gas_fops);
    cdev_add(cd_cdev, dev_num, 1);

    spin_lock_init(&my_lock);
    init_waitqueue_head(&my_wq);

    return 0;
}

static void __exit gas_exit(void) {
    DEBUG_MSG("Exit Module \n");
    gpio_set_value(LED, 0);
    free_irq(irq_num, NULL);
    gpio_free(LED);
    gpio_free(MQ9_DIGITAL_OUT);
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(gas_init);
module_exit(gas_exit);

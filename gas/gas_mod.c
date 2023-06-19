#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...)\
		printk(KERN_DEBUG "gas_detection: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

#define MQ9_DIGITAL_OUT 22
#define LED 17

static int irq_num;

static irqreturn_t mq9_irq_isr(int irq, void* dev_id) {
    DEBUG_MSG("GAS DETECT");
    gpio_set_value(LED, 1);
    return IRQ_HANDLED;
}

static int __init gas_init(void) {
    int ret;

    gpio_request_one(MQ9_DIGITAL_OUT, GPIOF_IN, "MQ9_DIGITAL_OUT");
    gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "LED");

    irq_num = gpio_to_irq(MQ9_DIGITAL_OUT);
    ret = request_irq(irq_num, mq9_irq_isr, IRQF_TRIGGER_FALLING, "mq9_irq_isr", NULL);
    if (ret) {
        free_irq(irq_num, NULL);
    }
    return 0;
}

static void __exit gas_exit(void) {
    gpio_set_value(LED, 0);
    free_irq(irq_num, NULL);
    gpio_free(LED);
    gpio_free(MQ9_DIGITAL_OUT);
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(gas_init);
module_exit(gas_exit);

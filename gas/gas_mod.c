#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/spi/spi.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

#define DEV_NAME "gasdev0.0"

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...)\
		printk(KERN_DEBUG "gas_detection: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

#define SPI_BUS_NUM 0

#define MQ9_DIGITAL_OUT 22
#define LED 17

static struct spi_device *etx_spi_device;

struct spi_board_info etx_spi_device_info = 
{
  .modalias     = "etx-spi-mcp3208-driver",
  .max_speed_hz = 1000000,              
  .bus_num      = SPI_BUS_NUM,          // SPI 1
  .chip_select  = 0,                    // Use 0
  .mode         = SPI_MODE_0            // SPI mode 0
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int irq_num;

static ssize_t gas_read(struct file *file, char * data, size_t size, loff_t * offset) {
    /* int spi_write_then_read(struct spi_device * spi, const void * txbuf, unsigned n_tx, void * rxbuf, unsigned n_rx)
    */
    u8 tbuf[3], rbuf[3], channel = 0;
    int gas = 0;
    struct spi_transfer  tr = 
    {
      .tx_buf  = tbuf,
      .rx_buf = rbuf,
      .len    = 3,
    };

    tbuf[0] = 0x06 | ((channel & 0x07) >> 2);
    tbuf[1] = ((channel & 0x07) << 6);
    tbuf[2] = 0x00;

    if ( spi_sync_transfer(etx_spi_device, &tr, 1) < 0) {
        DEBUG_MSG("get data from spi error \n");
    }

    rbuf[1] = 0x0F & rbuf[1];
    gas = (rbuf[1] << 8) | rbuf[2];

    sprintf(data, "%d", gas);
    DEBUG_MSG("sensor = %d\n", gas);
    DEBUG_MSG("tbuf : %02x %02x %02x\n", tbuf[0], tbuf[1], tbuf[2]);
    DEBUG_MSG("rbuf : %02x %02x %02x\n", rbuf[0], rbuf[1], rbuf[2]);

    return strlen(data);
}

static int gas_open(struct inode *inode, struct file *file) {
    DEBUG_MSG("open\n");
    return 0;
}

static int gas_relase(struct inode *inode, struct file *file) {
    DEBUG_MSG("release\n");
    return 0;
}

struct file_operations gas_fops = {
    .open = gas_open,
    .read = gas_read,
    .release = gas_relase,
};

static irqreturn_t mq9_irq_isr(int irq, void* dev_id) {
    DEBUG_MSG("GAS DETECT");
    gpio_set_value(LED, 1);
    return IRQ_HANDLED;
}

static int __init gas_init(void) {
    int ret;
    struct  spi_master *master;

    master = spi_busnum_to_master( etx_spi_device_info.bus_num );
    if( master == NULL ) {
        DEBUG_MSG("SPI Master not found.\n");
        return -ENODEV;
    }

    // create a new slave device, given the master and device info
    etx_spi_device = spi_new_device( master, &etx_spi_device_info );
    if( etx_spi_device == NULL )  {
        DEBUG_MSG("FAILED to create slave.\n");
        return -ENODEV;
    }
    
    // 8-bits in a word
    etx_spi_device->bits_per_word = 8;
    // setup the SPI slave device
    ret = spi_setup( etx_spi_device );
    if( ret )
    {
        DEBUG_MSG("FAILED to setup slave.\n");
        spi_unregister_device( etx_spi_device );
        return -ENODEV;
    }

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
    ret = cdev_add(cd_cdev, dev_num, 1);
    if (ret < 0) {
        DEBUG_MSG("fail to add character device \n");
        return -1;
    }

    return 0;
}

static void __exit gas_exit(void) {
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
    spi_unregister_device( etx_spi_device );    // Unregister the SPI slave
    gpio_set_value(LED, 0);
    free_irq(irq_num, NULL);
    gpio_free(LED);
    gpio_free(MQ9_DIGITAL_OUT);
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(gas_init);
module_exit(gas_exit);

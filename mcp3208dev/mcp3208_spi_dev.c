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

#define DEV_NAME "mcp3208dev0.0"

#ifdef MY_DEBUG
	#define DEBUG_MSG(fmt, args...)\
		printk(KERN_DEBUG "mcp3208_dev: " fmt, ##args);
#else
	#define DEBUG_MSG(fmt, args...)
#endif

#define SPI_BUS_NUM 0

static struct spi_device *etx_spi_device;

struct spi_board_info etx_spi_device_info = 
{
  .modalias     = "etx-spi-mcp3208-driver",
  .max_speed_hz = 1000000,              // mcp3208 using 1MHz
  .bus_num      = SPI_BUS_NUM,          // SPI Master: 0
  .chip_select  = 0,                    // SPI Slave: 0
  .mode         = SPI_MODE_0            // SPI mode 0
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static ssize_t mcp3208_spi_read(struct file *file, char * data, size_t size, loff_t * offset) {
    u8 tbuf[3], rbuf[3], channel = 0;
    int value = 0;
    struct spi_transfer  tr = 
    {
      .tx_buf  = tbuf,
      .rx_buf = rbuf,
      .len    = 3,
    };

    DEBUG_MSG("data: %d \n", data[0]);
    if ( data[0] < 0 || data[0] > 7 )) { /* mcp3208 has only 0-7 channel */
        return -EINVAL; /* Invalid argument */
    }
    channel = *data;

    tbuf[0] = 0x06 | ((channel & 0x07) >> 2);
    tbuf[1] = ((channel & 0x07) << 6);
    tbuf[2] = 0x00;

    if ( spi_sync_transfer(etx_spi_device, &tr, 1) < 0) {
        DEBUG_MSG("get data from spi error \n");
    }

    rbuf[1] = 0x0F & rbuf[1];
    value = (rbuf[1] << 8) | rbuf[2];

    sprintf(data, "%d", value);
    DEBUG_MSG("chenel = %d | sensor = %d", channel, value);
    DEBUG_MSG("tbuf : %02x %02x %02x\n", tbuf[0], tbuf[1], tbuf[2]);
    DEBUG_MSG("rbuf : %02x %02x %02x\n", rbuf[0], rbuf[1], rbuf[2]);

    return strlen(data);
}

static int mcp3208_spi_open(struct inode *inode, struct file *file) {
    DEBUG_MSG("open\n");
    return 0;
}

static int mcp3208_spi_release(struct inode *inode, struct file *file) {
    DEBUG_MSG("release\n");
    return 0;
}

struct file_operations spi_fops = {
    .open = mcp3208_spi_open,
    .read = mcp3208_spi_read,
    .release = mcp3208_spi_release,
};

static int __init spi_dev_init(void) {
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

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &spi_fops);
    ret = cdev_add(cd_cdev, dev_num, 1);
    if (ret < 0) {
        DEBUG_MSG("fail to add character device \n");
        return -EPERM;
    }

    return 0;
}

static void __exit spi_dev_exit(void) {
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
    spi_unregister_device( etx_spi_device );    // Unregister the SPI slave
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Peter Park <holy.people@kakao.com>");
MODULE_DESCRIPTION("MCP3208 SPI Device Driver");
module_init(spi_dev_init);
module_exit(spi_dev_exit);

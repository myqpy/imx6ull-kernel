#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/device.h>

// #include "LEDinit.h"

#define NEWCHRLED_NAME "newchrled"
#define NEWCHRLEDCOUNT 1

#define LEDOFF 0 /*关闭*/
#define LEDON 1  /*打开*/

/*寄存器物理地址*/
#define CCM_CCGR1_BASE (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE (0X020E02F4)
#define GPIO1_DR_BASE (0X0209C000)
#define GPIO1_GDIR_BASE (0X0209C004)

/*地址映射后的虚拟地址指针*/
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

/*LED设备结构体*/
typedef struct newchrled_dev
{
    struct cdev cdev;      /*字符设备*/
    dev_t devid;           /*设备号*/
    int major;             /*主设备号*/
    int minor;             /*次设备号*/
    struct class *class;   /*类*/
    struct device *device; /*设备*/
} newchrled_str;

newchrled_str newchrled;

void led_switch(u8 status)
{
    u32 val = 0;
    // printk("status:%x", status);
    if (status == LEDOFF)
    {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3); // 清零
        val |= (1 << 3);  // 关闭LED
        writel(val, GPIO1_DR);
    }
    if (status == LEDON)
    {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3); // 清零
        val |= ~(1 << 3); // 打开LED
        writel(val, GPIO1_DR);
    }
}

void led_init(int val)
{
    /*1.初始化LED灯，地址映射*/
    IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
    SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
    SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
    GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);

    /*2. 初始化*/
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3 << 26);
    val |= 3 << 26;
    writel(val, IMX6U_CCM_CCGR1);

    writel(5, SW_MUX_GPIO1_IO03);      // 设置复用
    writel(0x10B0, SW_PAD_GPIO1_IO03); // 设置输出

    val = readl(GPIO1_GDIR);
    val &= ~(1 << 3);
    val |= (1 << 3); // 设置为输出
    writel(val, GPIO1_GDIR);

    val = readl(GPIO1_DR);
    val &= ~(1 << 3); // 清零
    val |= (1 << 3);  // 初始化默认关闭LED灯
    writel(val, GPIO1_DR);
}

void led_deinit(void)
{
    int val = 0;
    val = readl(GPIO1_DR);
    val &= ~(1 << 3); // 清零
    val |= (1 << 3);  // 关闭LED灯
    writel(val, GPIO1_DR);

    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);
}

static int newchrled_open(struct inode *inode, struct file *file)
{
    printk("led_open \r\n");
    return 0;
}

static int newchrled_close(struct inode *inode, struct file *file)
{
    printk("led_close \r\n");
    return 0;
}

static ssize_t newchrled_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int retvalue;
    unsigned char databuf[1];
    printk("led_write \r\n");

    retvalue = copy_from_user(databuf, buf, count);
    if (retvalue < 0)
    {
        printk("kernel write failed \r\n");
        return -EFAULT;
    }
    printk("buf[0]:%x \r\n", buf[0]);
    printk("databuf:%x \r\n", databuf[0]);
    /*判断是开灯还是关灯*/
    led_switch(buf[0]);
    return 0;
}

static const struct file_operations newchrled_fops = {
    .owner = THIS_MODULE,
    .write = newchrled_write,
    .open = newchrled_open,
    .release = newchrled_close,
};

/*入口*/
static int __init newChrled_init(void)
{
    int retvalue = 0;
    printk("newchrled init!\r\n");
    /*1. 初始化LED*/
    led_init(retvalue);

    /*2. 注册字符设备*/
    if (newchrled.major) /*给定主设备号*/
    {
        newchrled.devid = MKDEV(newchrled.major, 0);
        retvalue = register_chrdev_region(newchrled.devid, NEWCHRLEDCOUNT, NEWCHRLED_NAME);
    }
    else
    {
        retvalue = alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLEDCOUNT, NEWCHRLED_NAME);
        newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid);
    }

    if (retvalue < 0)
    {
        printk("newchrled chrdev_region err!\r\n");
        return -1;
    }

    printk("newchrled major = %d, minor = %d \r\n", newchrled.major, newchrled.minor);

    /*3. 添加字符设备*/
    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &newchrled_fops);
    retvalue = cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLEDCOUNT);

    if (retvalue < 0)
    {
        printk("newchrled chrdev_register err!\r\n");
        return -1;
    }

    newchrled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
    if (IS_ERR(newchrled.class))
        return PTR_ERR(newchrled.class);

    newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, NEWCHRLED_NAME);
    if (IS_ERR(newchrled.device))
        return PTR_ERR(newchrled.device);

    printk("newchrled register success!\r\n");
    return 0;
}

/*出口*/
static void __exit newChrled_exit(void)
{
    printk("newchrled exit!\r\n");
    led_deinit();
    /*删除字符设备*/
    cdev_del(&newchrled.cdev);

    /*注销设备号*/
    unregister_chrdev_region(newchrled.devid, NEWCHRLEDCOUNT);

    class_destroy(newchrled.class);
    printk("newchrled unregister success!\r\n");
}

/*注册模块*/
module_init(newChrled_init);
module_exit(newChrled_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pengyu");
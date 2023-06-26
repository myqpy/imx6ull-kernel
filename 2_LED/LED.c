#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define LED_MAJOR 200
#define LED_NAME "led"

#define LEDOFF 0 /* 关灯 */
#define LEDON 1  /* 开灯 */

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

#define LEDOFF 0 /*关闭*/
#define LEDON 1  /*打开*/
static void led_switch(u8 status)
{
    u32 val = 0;
    printk("status:%x",status);
    if (status == LEDOFF)
    {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3); // 清零
        val |= (1 << 3); // 关闭LED
        writel(val,GPIO1_DR);
    }
    if (status == LEDON)
    {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3); // 清零
        val |= ~(1 << 3); // 打开LED
        writel(val,GPIO1_DR);
    }
}

static int led_open(struct inode *inode, struct file *file)
{
    printk("led_open \r\n");
    return 0;
}

static int led_close(struct inode *inode, struct file *file)
{
    printk("led_close \r\n");
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf,
                         size_t count, loff_t *ppos)
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
    printk("buf[0]:%x \r\n",buf[0]);
    printk("databuf:%x \r\n",databuf[0]);
    /*判断是开灯还是关灯*/
    led_switch(buf[0]);
    return 0;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write,
    .open = led_open,
    .release = led_close,
};

static int __init led_init(void)
{
    int ret = 0;
    int val = 0;
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
    val |= ~(1 << 3); // 打开LED灯
    writel(val, GPIO1_DR);

    ret = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
    if (ret < 0)
    {
        printk("register chrdev failed \r\n");
        return -EIO;
    }
    printk("led init\r\n");
    return 0;
}

static void __exit led_exit(void)
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
    unregister_chrdev(LED_MAJOR, LED_NAME);
    // if (ret < 0)
    // {
    //     printk("unregister chrdev failed \r\n");
    // }
    printk("led exit\r\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pengyu");
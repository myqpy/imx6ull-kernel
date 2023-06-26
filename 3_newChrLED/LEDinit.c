#include <linux/module.h>
#include <linux/kernel.h>
#include "LEDinit.h"


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

void led_switch(u8 status)
{
    u32 val = 0;
    printk("status:%x", status);
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

void led_init(void)
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
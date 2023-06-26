#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#define CHRDEVBASE_MAJOR 200
#define CHRDEVBASE_NAME "chrDevBase"

static int chrdevbase_open(struct inode *inode, struct file *file)
{
	printk("chrdevbase_open \r\n");
	return 0;
}

static int chrdevbase_release(struct inode *inode, struct file *file)
{
	printk("chrdevbase_release \r\n");
	return 0;
}

static ssize_t chrdevbase_read(struct file *filp, char __user *buf,
							   size_t count, loff_t *ppos)
{
	printk("chrdevbase_read \r\n");
	return 0;
}

static ssize_t chrdevbase_write(struct file *filp,const char __user *buf,
								size_t count, loff_t *ppos)
{
	printk("chrdevbase_write \r\n");
	return 0;
}

static const struct file_operations chrdevbase_fops =
	{
		.owner = THIS_MODULE,
		.open = chrdevbase_open,
		.release = chrdevbase_release,
		.read = chrdevbase_read,
		.write = chrdevbase_write,
};

static int __init chrdevbase_init(void)
{
	int ret = 0;
	printk("chrdevbase init\r\n");
	/*注册字符设备*/
	// int register_chrdev(unsigned int major, const char* name,
	// 					const struct file_operation *fops);
	ret = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME,
						  &chrdevbase_fops);

	if(ret <0) printk("chrdevbase init failed\r\n");
	return 0;
}

static void __exit chrdevbase_exit(void)
{
	printk("chrdevbase exit\r\n");
	/*注销字符设备*/
}

/*模块入口*/
module_init(chrdevbase_init);

/*模块出口*/
module_exit(chrdevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qupengyu");
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *thirddrv_class;
static struct class_device	*thirddrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;
volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;


static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
static volatile int ev_press = 0;	/* 中断事件标志, 中断服务程序将它置1，read将它清0 */

/* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */
/* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

struct pin_desc pins_desc[4] = {
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04},
};


//按下按键，中断服务程序被调用，确定按键值
//确定哪个按键有动作: 根据irq或根据封装的dev_id
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = s3c2410_gpio_getpin(pindesc->pin);	//系统函数: 读出引脚值,之前例子是读reg/移位/判断

	if (pinval)
	{
		/* 松开 */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* 按下 */
		key_val = pindesc->key_val;
	}

 	ev_press = 1;	/* 表示中断发生了 */
 	wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int third_drv_open(struct inode *inode, struct file *file)
{
	/* 配置GPF0,2为输入引脚 */
	/* 配置GPG3,11为输入引脚 */
	//request_irq()内会根据触发方式等设置为中断引脚
	//IRQ_EINT0看原理图和kernel代码确定，中断号在s3c24xx_init_irq()，触发方式在s3c_irqext_type()
	request_irq(IRQ_EINT0,  buttons_irq, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq(IRQ_EINT2,  buttons_irq, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", &pins_desc[3]);	

	return 0;
}

ssize_t third_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	/* 没动作就休眠(休眠:让出cpu并返回),否则和轮询一样 */	//把进程挂在 button_waitq 队列上
	wait_event_interruptible(button_waitq, ev_press);

	/* 有动作,返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

int third_drv_close(struct inode *inode, struct file *file)	//关闭设备时 free_irq
{
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
	return 0;
}


static struct file_operations sencod_drv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  third_drv_open,     
	.read	 =	third_drv_read,	   
	.release =  third_drv_close,	   
};


int major;
static int third_drv_init(void)
{
	major = register_chrdev(0, "third_drv", &sencod_drv_fops);

	thirddrv_class = class_create(THIS_MODULE, "third_drv");
	thirddrv_class_dev = class_device_create(thirddrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void third_drv_exit(void)
{
	unregister_chrdev(major, "third_drv");
	class_device_unregister(thirddrv_class_dev);
	class_destroy(thirddrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}

module_init(third_drv_init);
module_exit(third_drv_exit);
MODULE_LICENSE("GPL");


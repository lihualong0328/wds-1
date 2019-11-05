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
#include <linux/poll.h>


static struct class *fifthdrv_class;
static struct class_device	*fifthdrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;
volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static struct fasync_struct *button_async;

/* ��ֵ: ����ʱ, 0x01, 0x02, 0x03, 0x04 */
/* ��ֵ: �ɿ�ʱ, 0x81, 0x82, 0x83, 0x84 */
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


static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if (pinval)
	{
		/* �ɿ� */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* ���� */
		key_val = pindesc->key_val;
	}

	kill_fasync(&button_async, SIGIO, POLL_IN);	//�ж���,�ͷ��źŸ�app => (����˭,��ʲô�ź�,ԭ��(POLL_IN�����ݵȴ���ȡ))
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int fifth_drv_open(struct inode *inode, struct file *file)
{
	/* ����GPF0,2Ϊ�������� */
	/* ����GPG3,11Ϊ�������� */
	request_irq(IRQ_EINT0,  buttons_irq, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq(IRQ_EINT2,  buttons_irq, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "S5", &pins_desc[3]);	

	return 0;
}

ssize_t fifth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	/* ����а�������, ���ؼ�ֵ */
	copy_to_user(buf, &key_val, 1);

	return 1;
}

int fifth_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
	return 0;
}

static int fifth_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver: fifth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);	//��ʼ�����ͷ� button_async
}

static struct file_operations sencod_drv_fops = {
    .owner   =  THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open    =  fifth_drv_open,     
	.read	 =	fifth_drv_read,	   
	.release =  fifth_drv_close,
	.fasync	 =  fifth_drv_fasync,	//app����fasyncʱ��ȥ��ʼ�� button_async
};

int major;
static int fifth_drv_init(void)
{
	major = register_chrdev(0, "fifth_drv", &sencod_drv_fops);

	fifthdrv_class = class_create(THIS_MODULE, "fifth_drv");
	fifthdrv_class_dev = class_device_create(fifthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void fifth_drv_exit(void)
{
	unregister_chrdev(major, "fifth_drv");
	class_device_unregister(fifthdrv_class_dev);
	class_destroy(fifthdrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}

module_init(fifth_drv_init);
module_exit(fifth_drv_exit);
MODULE_LICENSE("GPL");


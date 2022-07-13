#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/poll.h>

#define CALL_DEV_NAME	"ledkey_poll"
#define CALL_DEV_MAJOR	240

#define DEBUG 0
#define IMX_GPIO_NR(bank, nr)       (((bank) - 1) * 32 + (nr))

DECLARE_WAIT_QUEUE_HEAD(WaitQueue_Read);
static unsigned long long time_old = 0;
static int sw_irq[8] = {0};
static unsigned long sw_no = 0;

int led[] = {
	IMX_GPIO_NR(1, 16),   //16
	IMX_GPIO_NR(1, 17),	  //17
	IMX_GPIO_NR(1, 18),   //18
	IMX_GPIO_NR(1, 19),   //19
};
static int key[] = {
	IMX_GPIO_NR(1, 20),
	IMX_GPIO_NR(1, 21),
	IMX_GPIO_NR(4, 8),
	IMX_GPIO_NR(4, 9),
  	IMX_GPIO_NR(4, 5),
  	IMX_GPIO_NR(7, 13),
  	IMX_GPIO_NR(1, 7),
 	IMX_GPIO_NR(1, 8),
};
static int led_init(void)
{
	int ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(led); i++) {
		ret = gpio_request(led[i], "gpio led");
		if(ret<0){
			printk("#### FAILED Request gpio %d. error : %d \n", led[i], ret);
		} 
		gpio_direction_output(led[i], 0);
	}

	return ret;
}
static void led_exit(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(led); i++){
		gpio_free(led[i]);
	}
}

void led_write(char data)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(led); i++){
//		gpio_direction_output(led[i], (data >> i ) & 0x01);
		gpio_set_value(led[i], (data >> i ) & 0x01);
	}
#if DEBUG
	printk("#### %s, data = %d\n", __FUNCTION__, data);
#endif
}
//irq function : init,exit,irq
irqreturn_t sw_isr(int irq, void *unuse)
{
	int i;
	for(i=0; i< ARRAY_SIZE(key); i++){
		if(irq == sw_irq[i]) {
			sw_no = i+1;
			break;
		}
	}
	printk("IRQ : %d, %ld\n",irq,sw_no);
	wake_up_interruptible(&WaitQueue_Read);
	return IRQ_HANDLED;
}
static int key_irq_init(void)
{
	int ret=0;
	int i;
	char * irq_name[8]={"irq sw1","irq sw2","irq sw3","irq sw4",
		"irq sw5","irq sw6","irq sw7","irq sw8"};
	
	for(i=0; i< ARRAY_SIZE(key); i++){
		sw_irq[i]=gpio_to_irq(key[i]);
	}
	for(i=0; i< ARRAY_SIZE(key); i++){
		ret = request_irq(sw_irq[i],sw_isr,IRQF_TRIGGER_RISING,irq_name[i],NULL);
		if(ret) {
			printk("### FAILED Request irq %d. error : %d\n",sw_irq[i],ret);
		}
	}
	return ret;
}
static void key_irq_exit(void)
{
	int i;
	for(i=0; i< ARRAY_SIZE(key); i++){
		free_irq(sw_irq[i],NULL);
	}
}



static int ledkey_open(struct inode *inode, struct file *filp)
{
	int num = MINOR(inode->i_rdev);
	printk("call open-> minor : %d\n", num);
	num = MAJOR(inode->i_rdev);
	printk("call open-> major : %d\n", num);
	return 0;
}
static loff_t ledkey_llseek(struct file *filp, loff_t off, int whence)
{
	printk("call llseek -> off : %08X, whence : %08X\n",(unsigned int)off, whence);
	return 0x23;
}
static ssize_t ledkey_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)

{
	unsigned long long  INTVAL = 0;
	char kbuf;

	if(!(filp->f_flags & O_NONBLOCK))
	{
//		if(sw_no==0)
//			interruptible_sleep_on(&WaitQueue_Read);
//			wait_event_interruptible(WaitQueue_Read,sw_no);
			wait_event_interruptible_timeout(WaitQueue_Read,sw_no,100);
//			if(sw_no == 0)
//				return 0;
	}
	kbuf = (char)sw_no;
	if(sw_no!=0){ 
		INTVAL = (get_jiffies_64()-time_old);
		time_old = get_jiffies_64();
//		printk("time : %ld.%ld\n",INTVAL/100,INTVAL%100);
		printk("time : %lld\n", INTVAL);
	}
	count=copy_to_user(buf,&kbuf,count);
	sw_no = 0;
	return count;
}
static ssize_t ledkey_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	char kbuf;
//	printk("call write -> buf : %08X, count : %08X \n", (unsigned int)buf,count);
//	get_user(kbuf,buf);
	count=copy_from_user(&kbuf,buf,count);
	led_write(kbuf);
	return count;
}
static long ledkey_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	printk("call write -> cmd : %08X, arg :%08X\n",cmd,(unsigned int)arg);
	return 0x53;
}
static int ledkey_release(struct inode *inode, struct file *filp)
{
	led_write(0);
	printk("call release \n");
	return 0;
}
static unsigned int ledkey_poll(struct file *filp, struct poll_table_struct *wait)
{
	int mask=0;
	poll_wait(filp, &WaitQueue_Read, wait);
	if(sw_no > 0)
		mask = POLLIN;
	return mask;
}
struct file_operations ledkey_fops = 
{
	.owner	= THIS_MODULE,
	.llseek	= ledkey_llseek,
	.read	= ledkey_read,
	.write	= ledkey_write,
	.poll 	= ledkey_poll,
	.unlocked_ioctl	= ledkey_ioctl,
	.open	= ledkey_open,
	.release	= ledkey_release,
};
static int ledkey_init(void)
{
	int result;
	time_old = get_jiffies_64();
	led_init();
	key_irq_init();
	printk("call call_init \n");
	result = register_chrdev(CALL_DEV_MAJOR,CALL_DEV_NAME, &ledkey_fops);
	if(result < 0) return result;
	return 0;
}
static void ledkey_exit(void)
{
	printk("call call_exit \n");
	led_exit();
	key_irq_exit();
	unregister_chrdev(CALL_DEV_MAJOR,CALL_DEV_NAME);
}
module_init(ledkey_init);
module_exit(ledkey_exit);
MODULE_LICENSE("Dual BSD/GPL");

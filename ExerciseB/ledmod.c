#include <linux/gpio.h> 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/timer.h>
#include "common.h"

#define SYSLED_MAJOR 20
#define SYSLED_MINOR 0
#define SYSLED_NO 1
#define SYSLED_GPIO 164
#define SYSLED_BUFFER_LENGTH 16

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Driver for I/O with SYS_LED_4");
MODULE_AUTHOR("DETMPS");

int sysled_open(struct inode *inode, struct file *filep);
int sysled_release(struct inode *inode, struct file *filep);
ssize_t sysled_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos);
ssize_t sysled_write(struct file *filep, char const __user *buf, size_t count, loff_t *f_pos);
long sysled_unlocked_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);
static void ledTimerFunc(unsigned long arg);

struct file_operations sysled_fops = {
	.owner = THIS_MODULE,
	.open = sysled_open,
	.release = sysled_release,
	.read = sysled_read,
	.write = sysled_write,
	.unlocked_ioctl = sysled_unlocked_ioctl,
};

// Declared globally 
struct cdev *cdevStruct;
int devno;
struct timer_list ledTimer;
int timeout_in_secs = 1;

static int __init sysled_init(void)
{
	// Setup devno and lasterr
	devno = MKDEV(SYSLED_MAJOR, SYSLED_MINOR);
	int lasterr = 0;

	printk(KERN_ALERT "SYS_LED module inserted.\n");

	// Requesting GPIO
	lasterr = gpio_request(SYSLED_GPIO, "sysled");
	if (lasterr < 0)
	{
		printk(KERN_ALERT "Error requesting gpio: %d for GPIO %d\n", lasterr, SYSLED_GPIO);
		goto err_exit;
	}

	// GPIO direction
	lasterr = gpio_direction_output(SYSLED_GPIO, 0);
	if (lasterr < 0)
	{
		printk(KERN_ALERT "Error changing gpio direction: %d for GPIO %d\n", lasterr, SYSLED_GPIO);
		goto err_gpiofree;
	}

	// Registering char device number regions (static)
	lasterr = register_chrdev_region(devno, SYSLED_NO, "sysled");
	if (lasterr < 0)
	{
		printk(KERN_ALERT "Error registering char device: %d\n", lasterr);
		goto err_gpiofree;
	}

	// Init char device
	cdevStruct = cdev_alloc();
	cdev_init(cdevStruct, &sysled_fops);

	// Add char device
	lasterr = cdev_add(cdevStruct, devno, SYSLED_NO);
	if (lasterr < 0)
	{
		printk(KERN_ALERT "Error adding char device: %d\n", lasterr);
		goto err_unregister_chrdev;
	}

	// All succesful
	return 0;

	// Proper cleanup
	err_unregister_chrdev:
		unregister_chrdev_region(devno, SYSLED_NO);
	err_gpiofree:
		gpio_free(SYSLED_GPIO);
	err_exit:
		return lasterr;
}

static void __exit sysled_exit(void)
{
	// Stop
	del_timer_sync(&ledTimer);

	// Remove char device from system
	cdev_del(cdevStruct);
	
	// Unregister char device
	unregister_chrdev_region(devno, SYSLED_NO);

	// Free gpio
	gpio_free(SYSLED_GPIO);

	printk(KERN_ALERT "SYS_LED driver unloaded.\n");
}

module_init(sysled_init);
module_exit(sysled_exit);


int sysled_open(struct inode *inode, struct file *filep)
{	
	// Reading major and minor
	int major, minor;
	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);

	// Printing opening of file
	printk("Opening cdev [%d %d].\n", major, minor);

	return 0;
}

int sysled_release(struct inode *inode, struct file *filep)
{
	// Reading major and minor
	int major, minor;
	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);

	// Printing closing of file
	printk("Closing cdev [%d, %d].\n", major, minor);

	return 0;
}

ssize_t sysled_read(struct file *filep, char __user *buf,
				 size_t count, loff_t *f_pos)
{	
	// Make sure that the buffer is big enough to fit read value plus null termination
	// readLED will be either zero or one (a single char), therefore 2 is a fit size.
	char buffer[SYSLED_BUFFER_LENGTH];
	int bufferLength = sizeof(buffer);

	int readLED = gpio_get_value(SYSLED_GPIO);
	
	bufferLength = sprintf(buffer, "%d", readLED);

	if ( copy_to_user(buf, buffer, bufferLength) )
	{
		printk(KERN_ALERT "Could not copy to user space.\n");
	}

	// Update file position with the amount read
	*f_pos += bufferLength;

	// return amount read
	return bufferLength;    
}

ssize_t sysled_write(struct file *filep, char const __user *buf,
				 size_t count, loff_t *f_pos)
{	
	char buffer[SYSLED_BUFFER_LENGTH];

	if ( copy_from_user(buffer, buf, count) )
	{
		printk(KERN_ALERT "Could not copy from userspace.\n");
	}
	
	int value = 0;
	sscanf(buffer, "%d", &value);
	
	gpio_set_value(SYSLED_GPIO, value);

	*f_pos += count;

	// return amount read
	return count;    
}

long sysled_unlocked_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
		case SYSLED_IOC_TIMER_START:
			printk(KERN_ALERT "Starting timer.\n");
			init_timer(&ledTimer);
			ledTimer.expires = jiffies + timeout_in_secs*HZ;
			ledTimer.function = ledTimerFunc;
			ledTimer.data = 0;
			add_timer(&ledTimer);
			break;	 
		case SYSLED_IOC_TIMER_STOP:
			printk(KERN_ALERT "Stopping timer.\n");
			del_timer_sync(&ledTimer);
			break;
		case SYSLED_IOC_TIMER_FREQ:
			printk(KERN_ALERT "Timer timeout set to %ld (seconds).\n", arg);
			timeout_in_secs = arg;
			break;
		default:
			printk(KERN_ALERT "Unknown IOCTL call - %d\n", cmd);
	}

	return 0;
}

static void ledTimerFunc(unsigned long arg)
{
	ledTimer.expires = jiffies + timeout_in_secs*HZ;
	add_timer(&ledTimer);
	
	static int nextState = 0;
	nextState = !nextState;
	gpio_set_value(SYSLED_GPIO, nextState);
}













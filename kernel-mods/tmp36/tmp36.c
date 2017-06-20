/**
 * @file   tmp36.c
 * @author Santiago Pagola
 * @license GPL 
 * @date   14/06/2017
 * @version 0.1
 * @brief   A character driver to interact with the TMP36
 * temperature sensor. This kernel module shall be accessed
 * from user space and request the temperature from there.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/random.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
//#include "gpio-utils.h"

#define  DEVICE_NAME "tmp36" //device under /dev
#define  CLASS_NAME  "tmp36sensor"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Santiago Pagola");
MODULE_DESCRIPTION("Linux driver for the TMP36 temperature sensor with Beaglebone Black");
MODULE_VERSION("0.1");

static int    majorNumber;
static char   cmd_out[10] = {0}; // Value of temperature sent back to user space
static size_t size_out; //Size of outcoming buffer
static int    openCnt = 0; // Counts the number of times the temperature has been requested
static unsigned int gpioADCP9_40 = 0;
static struct class*  tmp36Class  = NULL; // The device-driver class struct pointer
static struct device* tmp36Device = NULL; // The device-driver device struct pointer

// Prototype functions for the character driver
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char*, size_t, loff_t *);


static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

/** @function tmp36_init
 *  @brief Init function
 *  Will initialize the necessary parameters such as major number, device/class pointers etc
 *  @return returns 0 if successful
 */
static int __init tmp36_init(void)
{
	printk(KERN_INFO "TMP36: Initializing the tmp36 driver\n");

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber < 0)
	{
		printk(KERN_ALERT "tmp36 driver failed to register major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "TMP36: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	tmp36Class = class_create(THIS_MODULE, CLASS_NAME);
	if ( IS_ERR(tmp36Class) )
	{
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(tmp36Class);
	}
	printk(KERN_INFO "TMP36: device class registered correctly\n");

	// Register the device driver
	tmp36Device = device_create(tmp36Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if ( IS_ERR(tmp36Device) )
	{
		class_destroy(tmp36Class);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(tmp36Device);
	}
	printk(KERN_INFO "TMP36: device class created correctly\n");
	// Set up the analog pin P9_40
	return 0;
}

/** @function tmp36_exit
 *  @brief Cleanup function
 Called on exit
 */
static void __exit tmp36_exit(void)
{
	device_destroy(tmp36Class, MKDEV(majorNumber, 0)); // remove the device
	class_unregister(tmp36Class); // unregister the device class
	class_destroy(tmp36Class); // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME); // unregister the major number
	printk(KERN_INFO "TMP36: Exiting now\n");
}

/** @function dev_open
 *  @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep)
{
	openCnt++;
	printk(KERN_INFO "TMP36: Temperature request count: %d\n", openCnt);
	return 0;
}

/** @function dev_read
 *  @brief Function to be used to copy a given buffer to user space (i.e. when
 *  user space has requested a read operation from this device) 
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int err = 0;
	int tmpval, tmp;
	get_random_bytes(&tmpval, sizeof(tmpval));
	tmp = tmpval % 5 + 20;
	sprintf(cmd_out, "%d", tmp);
	printk(KERN_INFO "TMP36: received temperature read request. Returning random: [%s]\n", cmd_out);
	size_out = strlen(cmd_out);
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	err = copy_to_user(buffer, cmd_out, size_out);

	if (err==0)
	{
		printk(KERN_INFO "TMP36: Sent %d bytes to user-space\n", size_out);
		return (size_out=0);
	}
	else
	{
		printk(KERN_INFO "TMP36: Failed to send %d bytes to user space\n", err);
		return -EFAULT;
	}
}

/** @function dev_write
 *  @brief Dummy function to cover the case where data may have been written to this device
 *  @param filep A pointer to the file object
 *  @param buffer The data to write - not even going to be used!
 *  @param len The length of the data to be written
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char* buffer, size_t len, loff_t *offset)
{
	//Some data was written from user space
	printk(KERN_INFO "Received buffer size: %d (Did you attempt to write to a sensor?)\n", len);
	return -EFAULT; //Always non-zero, should not write to this module!
}

/** @function dev_release
 *  @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "TMP36: Device successfully closed\n");
	return 0;
}

/** @brief Mandatory function calls 
*/
module_init(tmp36_init);
module_exit(tmp36_exit);

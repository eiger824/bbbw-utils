/**
 * @file   tl-led.c
 * @author Santiago Pagola 
 * @date   21/06/2017
 * @brief  A kernel module for controlling a trio of LEDs (traffic light)
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "tl-led"
#define CLASS_NAME "tl-led-drv"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Santiago Pagola");
MODULE_DESCRIPTION("A kernel module for controlling a trio of LEDs (traffic light)");
MODULE_VERSION("0.1");

static int major_number; //major number to be allocated to the character device

/** To be on the safe side, let's allocate 4 bytes for this message,
 *  byte 0: holds the future led state
 *  byte 1: holds the LED ID
 *  byte 2: (might) hold an additional newline character that follows
 *  	    if /dev/tl-led is accessed via a shell (e.g. echo 1 > /dev/tl-led),
 *	    this could of course be ommited if the -n flag was given to echo,
 *	    but it doesn't hurt to include the possibility for people like me
 *	    who like to type as less as possible :-)
 *  byte 3: zero-termination
 */
static char message[4] = {0};

static struct class* tlledClass = NULL;
static struct device* tlledDev = NULL;

static unsigned int gpio_led0 = 66; // Red LED ,on P8_7
static unsigned int gpio_led1 = 69; // Yellow LED, on P8_9
static unsigned int gpio_led2 = 45; // Green LED, on P8_11

static bool ledOn_0 = 0; // Red led state
static bool ledOn_1 = 0; // Yellow led state
static bool ledOn_2 = 0; // Green led state

//File operations
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char*, size_t, loff_t *);

//Setup/release leds
static void setup_leds(void);
static void release_leds(void);

//Tiny animation
static void animation(void);

static struct file_operations fops = 
{
	.open = dev_open,
	.write = dev_write,
	.release = dev_release,
};

/** @function setup
 *  @brief This function will export our desired LEDS so we are able to access them
 *  All of them will be set as output pins
 */
static void setup_leds(void)
{
	//LEDS
	gpio_request(gpio_led0, "sysfs");
	gpio_direction_output(gpio_led0, ledOn_0);
	gpio_export(gpio_led0, false);
	gpio_request(gpio_led1, "sysfs");
	gpio_direction_output(gpio_led1, ledOn_1);
	gpio_export(gpio_led1, false);
	gpio_request(gpio_led2, "sysfs");
	gpio_direction_output(gpio_led2, ledOn_2);
	gpio_export(gpio_led2, false);
}

/** @function release_leds
 *  @brief Releases the resources allocated by the LEDS
 */
static void release_leds(void)
{
	//Turn off leds first
	gpio_set_value(gpio_led0, 0);
	gpio_set_value(gpio_led1, 0);
	gpio_set_value(gpio_led2, 0);
	//Unexport them
	gpio_unexport(gpio_led0);
	gpio_unexport(gpio_led1);
	gpio_unexport(gpio_led2);
	//And free them
	gpio_free(gpio_led0);
	gpio_free(gpio_led1);
	gpio_free(gpio_led2);
}

/** @function animation
 *  @brief Animation when initalizing/removing this module
 */

static void animation(void)
{
	unsigned int i;
	for (i=0; i<4; ++i)
	{
		ledOn_0 = true;
		ledOn_1 = true;
		ledOn_2 = true;
		gpio_set_value(gpio_led0, ledOn_0);
		msleep(75);
		gpio_set_value(gpio_led1, ledOn_1);
		msleep(75);
		gpio_set_value(gpio_led2, ledOn_2);
		msleep(75);
		//Invert state
		ledOn_0 = false;
		ledOn_1 = false;
		ledOn_2 = false;
		gpio_set_value(gpio_led2, ledOn_2);
		msleep(75);
		gpio_set_value(gpio_led1, ledOn_1);
		msleep(75);
		gpio_set_value(gpio_led0, ledOn_0);
		msleep(75);
	}
	
	//Last triple flash
	for (i=0; i<3; ++i)
	{
		ledOn_0 = true;
		ledOn_1 = true;
		ledOn_2 = true;
		gpio_set_value(gpio_led0, ledOn_0);
		gpio_set_value(gpio_led1, ledOn_1);
		gpio_set_value(gpio_led2, ledOn_2);
		msleep(150);
		ledOn_0 = false;
		ledOn_1 = false;
		ledOn_2 = false;
		gpio_set_value(gpio_led0, ledOn_0);
		gpio_set_value(gpio_led1, ledOn_1);
		gpio_set_value(gpio_led2, ledOn_2);
		msleep(150);
		
	}
}

/** @function tlled_init
 *  @brief Init function to call when loading the module, which will set up
 *  stuff like the 2 GPIOs to use, register the character device , etc.
 *  @return 0 on success, something else (-ENODEV, PTR_ERR(...)) otherwise
 */
static int __init tlled_init(void)
{
	int result = 0;
	printk(KERN_INFO "TL-LED: Initializing module\n");
	if (!gpio_is_valid(gpio_led0)){
		printk(KERN_INFO "TL-LED: invalid LED GPIO %d\n", gpio_led0);
		return -ENODEV;
	}
	if (!gpio_is_valid(gpio_led1)){
		printk(KERN_INFO "TL-LED: invalid LED GPIO %d\n", gpio_led1);
		return -ENODEV;
	}
	if (!gpio_is_valid(gpio_led2)){
		printk(KERN_INFO "TL-LED: invalid LED GPIO %d\n", gpio_led2);
		return -ENODEV;
	}

	//Setup the leds
	setup_leds();

	//Initial animation
	animation();

	//Now register character device
	//Register major number
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number<0)
	{
		printk(KERN_ALERT "TL-LED failed to register major number\n");
		return major_number;
	}
	printk(KERN_INFO "TL-LED: registered major number %d\n", major_number);

	//Register class
	tlledClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(tlledClass))
	{
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(tlledClass);
	}
	printk(KERN_INFO "TL-LED: device class successfully registered\n");

	//Register device driver
	tlledDev = device_create(tlledClass, NULL, MKDEV(major_number,0), NULL, DEVICE_NAME);
	if (IS_ERR(tlledDev))
	{
		class_destroy(tlledClass);
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(tlledDev);
	}
	printk(KERN_INFO "TL-LED: device class created successfully\n");
	return result;
}

/** @function tlled_exit
 *  @brief Cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required. Used to release the
 *  GPIOs and display cleanup messages.
 *  @return Nothing
 */
static void __exit tlled_exit(void)
{
	//Show final animation
	animation();
	//Release leds
	release_leds();
	//unregister character device
	device_destroy(tlledClass, MKDEV(major_number, 0));
	class_unregister(tlledClass);
	class_destroy(tlledClass);
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "TL-LED: Exiting now\n");
}

/** @function dev_open
 *  @brief Function to call when opening the device
 *  @param inodep Pointer to the inode struct
 *  @param filep Pointer to the file struct
 *  @return 0, no need to handle anything
 */
static int dev_open(struct inode* inodep, struct file *filep)
{
	printk(KERN_INFO "TL-LED: Device opened\n");
	return 0;
}

/** @function dev_release
 *  @brief Function to be called when closing the device (equivalent to dev_close())
 *  @param inodep Pointer to the inode struct
 *  @param filep Pointer to the file
 *  @return 0 on success, no need to handle anything
 */
static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "TL-LED: Device closed\n");
	return 0;
}

/** @function dev_write
 *  @brief Write function used to write data from user-space to this character device
 *  Note the use of copy_from_user function, since we need to copy the data coming from
 *  user-space (i.e. buffer) to our static array "message" in order to be able to use it
 *  @param filep A pointer to the file structure, defined in fs.h
 *  @param buffer The data coming from user-space
 *  @param len Then length of the copied bytes from user-space
 *  @param offset An optional offset that may be given
 *  @return the number of bytes copied to the character device
 */
static ssize_t dev_write(struct file *filep, const char* buffer, size_t len, loff_t *offset)
{
	//Some data was written from user space
	printk(KERN_INFO "Received buffer size: %d\n", len);
	int err = 0;
	err = copy_from_user(message, buffer, len);
	message[len] = 0;
	printk(KERN_INFO "Received buffer [%s], led status for led \"%c\" is: [%c]\n", message, message[1], message[0]);
	if (message[1] == '0')
	{
		if (message[0] == '0')
		{
			gpio_set_value(gpio_led0, false);
			ledOn_0 = false;
		}
		else if (message[0] == '1')
		{
			gpio_set_value(gpio_led1, false);
			ledOn_1 = false;
		}
		else if (message[0] == '2')
		{
			gpio_set_value(gpio_led2, false);
			ledOn_2 = false;
		}
		else
		{
			printk(KERN_INFO "Unknown LED-ID received: %c\n", message[1]);
		}
	}
	else if (message[1] == '1')
	{
		if (message[0] == '0')
		{
			gpio_set_value(gpio_led0, true);
			ledOn_0 = true;
		}
		else if (message[0] == '1')
		{
			gpio_set_value(gpio_led1, true);
			ledOn_1 = true;
		}
		else if (message[0] == '2')
		{
			gpio_set_value(gpio_led2, true);
			ledOn_2 = true;
		}
		else
		{
			printk(KERN_INFO "Unknown LED-ID received: %c\n", message[1]);
		}
	}
	else
	{
		printk(KERN_INFO "Unknown stream received: %s\n", buffer);
	}
	return len;
}

/// This next calls are  mandatory -- they identify the initialization function
/// and the cleanup function (as above).
module_init(tlled_init);
module_exit(tlled_exit);

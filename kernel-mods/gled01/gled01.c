/**
 * @file   gled01.c
 * @author Santiago Pagola 
 * @date   15/06/2017
 * @brief  A kernel module for controlling a GPIO LED/button pair.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "gled01"
#define CLASS_NAME "bbbw-gled"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Santiago Pagola");
MODULE_DESCRIPTION("A Button/LED test driver for the BBB");
MODULE_VERSION("0.1");

static int major_number;
static char message[2] = {0};
static struct class* gled01Class = NULL;
static struct device* gled01Dev = NULL;
static unsigned int gpio_led = 49;// Led to use, on P9_23
static unsigned int gpio_btn = 115; // Push button to use, on P9_27
static unsigned int irq_n; // Used to share the IRQ number within this file
static unsigned int press_cnt = 0; // For information, store the number of button presses
static unsigned int req_cnt = 0; // Number of read requests from user space
static bool ledOn = 0; // Led state

// Hander for the IRQ
static irq_handler_t  ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

//File operations
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *, const char*, size_t, loff_t *);

static struct file_operations fops = 
{
	.open = dev_open,
	.write = dev_write,
	.release = dev_release,
};

//Init function
static int __init gled01_init(void){
	int result = 0;
	printk(KERN_INFO "GLED01: Initializing the GPIO LED 01 LKM\n");
	if (!gpio_is_valid(gpio_led)){
		printk(KERN_INFO "GLED01: invalid LED GPIO\n");
		return -ENODEV;
	}
	// Going to set up the LED. It is a GPIO in output mode and will be on by default
	ledOn = true;
	gpio_request(gpio_led, "sysfs");          // gpio_led is hardcoded to 49, request it
	gpio_direction_output(gpio_led, ledOn);   // Set the gpio to be in output mode and on
	gpio_export(gpio_led, false);             // Causes gpio49 to appear in /sys/class/gpio
	// the bool argument prevents the direction from being changed
	gpio_request(gpio_btn, "sysfs");       // Set up the gpio_btn
	gpio_direction_input(gpio_btn);        // Set the button GPIO to be an input
	gpio_set_debounce(gpio_btn, 200);      // Debounce the button with a delay of 200ms
	gpio_export(gpio_btn, false);          // Causes gpio115 to appear in /sys/class/gpio
	// the bool argument prevents the direction from being changed
	// Perform a quick test to see that the button is working as expected on LKM load
	printk(KERN_INFO "GLED01: The button state is currently: %d\n", gpio_get_value(gpio_btn));

	// GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
	irq_n = gpio_to_irq(gpio_btn);
	printk(KERN_INFO "GLED01: The button is mapped to IRQ: %d\n", irq_n);

	// This next call requests an interrupt line
	result = request_irq(irq_n,             // The interrupt number requested
			(irq_handler_t) ebbgpio_irq_handler, // The pointer to the handler function below
			IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
			"ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
			NULL);                 // The *dev_id for shared interrupt lines, NULL is okay

	printk(KERN_INFO "GLED01: The interrupt request result is: %d\n", result);

	//Now register character device
	//Register major number
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number<0)
	{
		printk(KERN_ALERT "GLED01 failed to register major number\n");
		return major_number;
	}
	printk(KERN_INFO "GLED01: registered major number %d\n", major_number);
	
	//Register class
	gled01Class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(gled01Class))
	{
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(gled01Class);
	}
	printk(KERN_INFO "GLED01: device class successfully registered\n");

	//Register device driver
	gled01Dev = device_create(gled01Class, NULL, MKDEV(major_number,0), NULL, DEVICE_NAME);
	if (IS_ERR(gled01Dev))
	{
		class_destroy(gled01Class);
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(gled01Dev);
	}
	printk(KERN_INFO "GLED01: device class created successfully\n");
	return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required. Used to release the
 *  GPIOs and display cleanup messages.
 */
static void __exit gled01_exit(void){
	printk(KERN_INFO "GLED01: The button state is currently: %d\n", gpio_get_value(gpio_btn));
	printk(KERN_INFO "GLED01: The button was pressed %d times\n", press_cnt);
	gpio_set_value(gpio_led, 0);              // Turn the LED off, makes it clear the device was unloaded
	gpio_unexport(gpio_led);                  // Unexport the LED GPIO
	free_irq(irq_n, NULL);               // Free the IRQ number, no *dev_id required in this case
	gpio_unexport(gpio_btn);               // Unexport the Button GPIO
	gpio_free(gpio_led);                      // Free the LED GPIO
	gpio_free(gpio_btn);                   // Free the Button GPIO
	//unregister character device
	device_destroy(gled01Class, MKDEV(major_number, 0));
	class_unregister(gled01Class);
	class_destroy(gled01Class);
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "GLED01: Goodbye from the LKM!\n");
}

static int dev_open(struct inode* inodep, struct file *filep)
{
	printk(KERN_INFO "GLED01: Device opened\n");
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "GLED01: Device closed\n");
	return 0;
}

static ssize_t dev_write(struct file *filep, const char* buffer, size_t len, loff_t *offset)
{
	//Some data was written from user space
	printk(KERN_INFO "Received buffer size: %d\n", len);
	int err = 0;
	err = copy_from_user(message, buffer, len);
	message[len] = 0;
	printk(KERN_INFO "Received led status is: [%s]\n", message);
	if (!strcmp(message,"0"))
	{
		gpio_set_value(gpio_led, false);
	}
	else if (!strcmp(message, "1"))
	{
		gpio_set_value(gpio_led, true);
	}
	else
	{
		printk(KERN_INFO "Unknown stream received: %s\n", buffer);
	}
	++req_cnt;
	//Switch led state
	ledOn = !ledOn;
	printk(KERN_INFO "Request number: %d\n", req_cnt);
	return len;
}

/** @brief The GPIO IRQ Handler function
 *  This function is a custom interrupt handler that is attached to the GPIO above. The same interrupt
 *  handler cannot be invoked concurrently as the interrupt line is masked out until the function is complete.
 *  This function is static as it should not be invoked directly from outside of this file.
 *  @param irq    the IRQ number that is associated with the GPIO -- useful for logging.
 *  @param dev_id the *dev_id that is provided -- can be used to identify which device caused the interrupt
 *  Not used in this example as NULL is passed.
 *  @param regs   h/w specific register values -- only really ever used for debugging.
 *  return returns IRQ_HANDLED if successful -- should return IRQ_NONE otherwise.
 */
static irq_handler_t ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
	ledOn = !ledOn;                          // Invert the LED state on each button press
	gpio_set_value(gpio_led, ledOn);          // Set the physical LED accordingly
	printk(KERN_INFO "GLED01: Interrupt! (button state is %d)\n", ledOn);
	press_cnt++;                         // Global counter, will be outputted when the module is unloaded
	return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}

/// This next calls are  mandatory -- they identify the initialization function
/// and the cleanup function (as above).
module_init(gled01_init);
module_exit(gled01_exit);

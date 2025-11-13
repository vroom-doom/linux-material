#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>       // file_operations
#include <linux/cdev.h>     // cdev
#include <linux/device.h>   // class_create, device_create
#include <linux/wait.h>     // wait_queue
#include <linux/sched.h>    // TASK_INTERRUPTIBLE
#include <linux/uaccess.h>  // copy_to_user
#include <linux/atomic.h>   // atomic_t

#include "button_sim.h"     // Include our shared ioctl header

#define DEVICE_NAME "button_sim"
#define CLASS_NAME  "button"

static dev_t dev_num;
static struct cdev my_cdev;
static struct class* my_class = NULL;
static struct device* my_device = NULL;

// 1. The wait queue for the sleeping process
static DECLARE_WAIT_QUEUE_HEAD(my_wait_queue);

// 2. The flag to signal an event (the "interrupt")
//    We use atomic_t for thread-safe access
static atomic_t event_occurred = ATOMIC_INIT(0);

// --- The "Interrupt Handler" ---
// This function is what our ioctl will call to simulate the IRQ.
static void button_sim_irq_handler(void)
{
    // Action 1: Log a message (this is our "top-half")
    printk(KERN_INFO "button_sim: **INTERRUPT** Button pressed! Waking up process.\n");

    // Action 2: Set the flag and wake up the sleeping process (this is our "bottom-half")
    atomic_set(&event_occurred, 1);
    wake_up_interruptible(&my_wait_queue);
}

// --- File Operations ---

// Called when a process tries to read() from our device
static ssize_t button_sim_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "button_sim: Process (PID %d) is going to sleep.\n", current->pid);

    // Wait until the condition (event_occurred != 0) is true
    // This puts the process to sleep interruptibly.
    wait_event_interruptible(my_wait_queue, atomic_read(&event_occurred) != 0);

    // We were woken up! Reset the flag for the next event.
    atomic_set(&event_occurred, 0);

    // Check if we were woken by a signal (e.g., Ctrl+C)
    if (signal_pending(current)) {
        return -ERESTARTSYS;
    }

    // Send a single byte to the user to confirm the event
    if (copy_to_user(buf, "B", 1)) {
        return -EFAULT;
    }

    // Return the number of bytes read
    return 1;
}

// Called when a process calls ioctl() on our device
static long button_sim_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    // Check if it's our specific ioctl command
    if (cmd == BUTTON_SIM_IOCTL_TRIGGER) {
        printk(KERN_INFO "button_sim: IOCTL trigger received.\n");
        
        // --- THIS IS THE TRIGGER ---
        // Call our "handler" function to simulate the IRQ
        button_sim_irq_handler();
        
        return 0; // Success
    }

    return -ENOTTY; // Command not recognized
}

// Simple open/release functions
static int button_sim_open(struct inode *inode, struct file *filp)
{
    return 0; // Success
}

static int button_sim_release(struct inode *inode, struct file *filp)
{
    return 0; // Success
}

// Map our functions to the file_operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = button_sim_open,
    .release = button_sim_release,
    .read = button_sim_read,
    .unlocked_ioctl = button_sim_ioctl,
};

// --- Module Init and Exit ---

static int __init button_sim_init(void)
{
    // 1. Allocate a major/minor number
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "button_sim: Failed to allocate major number\n");
        return -1;
    }

    // 2. Create a device class
    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_class);
    }

    // 3. Create the device file in /dev
    my_device = device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_device);
    }

    // 4. Initialize and add the character device
    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        device_destroy(my_class, dev_num);
        class_destroy(my_class);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    printk(KERN_INFO "button_sim: Module loaded. Device /dev/%s created.\n", DEVICE_NAME);
    return 0;
}

static void __exit button_sim_exit(void)
{
    // Clean up in reverse order
    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "button_sim: Module unloaded.\n");
}

module_init(button_sim_init);
module_exit(button_sim_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Button press interrupt simulator");
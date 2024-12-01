#include <linux/module.h>

#include <linux/usb.h>
#include <asm-generic/errno-base.h>
#include <linux/init.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");

#define ML_VENDOR_ID	0x275d
#define ML_PRODUCT_ID	0x0ba6

#ifdef CONFIG_USB_DYNAMIC_MINORS
#define ML_MINOR_BASE	0
#else
#define ML_MINOR_BASE	96
#endif

struct usb_ml {
    struct usb_device 	*udev;
	struct usb_interface 	*interface;
	unsigned char		minor;
	char			serial_number[8];

	int			open_count;     /* Open count for this port */
	struct 			semaphore sem;	/* Locks this structure */
	spinlock_t		cmd_spinlock;	/* locks dev->command */

	char				*int_in_buffer;
	struct usb_endpoint_descriptor  *int_in_endpoint;
	struct urb 			*int_in_urb;
	int				int_in_running;

	char			*ctrl_buffer; /* 8 byte buffer for ctrl msg */
	struct urb		*ctrl_urb;
	struct usb_ctrlrequest  *ctrl_dr;     /* Setup packet information */
	int			correction_required;

	__u8			command;/* Last issued command */
};

static struct usb_device_id ml_table [] = {
    { USB_DEVICE(ML_VENDOR_ID, ML_PRODUCT_ID) },
    { }
};
MODULE_DEVICE_TABLE(usb, ml_table);

static struct usb_driver ml_driver;

static void ml_int_in_callback(struct urb *urb){
    printk(KERN_DEBUG "ml_int_in_callback called with status %d", urb->status);
}

static void ml_ctrl_callback(struct urb *urb){
	printk(KERN_DEBUG "ml_ctrl_callback called with status %d", urb->status);
}

static int ml_release(struct inode *inode, struct file *file){
    return 0;
}


static ssize_t ml_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos){
	return 0;
}

static int ml_open(struct inode *inode, struct file *file){
	struct usb_ml *dev = NULL;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	printk(KERN_DEBUG "Open device");
	subminor = iminor(inode);

	interface = usb_find_interface(&ml_driver, subminor);
	if (!interface) {
		printk(KERN_DEBUG "can't find device for minor %d", subminor);
		return -ENODEV;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		return -ENODEV;
	}

	file->private_data = dev;

    return 0;
}

static ssize_t ml_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
	int bytes_read = 0; 
	struct usb_ml *dev;
	int retval;

	dev = file->private_data;

    char *data = kmalloc(255, GFP_KERNEL);

	retval = usb_control_msg(dev->udev, usb_rcvctrlpipe(dev->udev, 0x80), 0x06, 0x80, 0x0301, 0x0409, data, 255, 1000);

	if (retval < 0){
		printk(KERN_DEBUG "usb_control_msg (%d)", retval);
	 	return 0;
	}

	copy_to_user(buffer, data, retval);

    return retval;
}

static struct file_operations ml_fops = {
	.owner =	THIS_MODULE,
	.write =	ml_write,
	.read =     ml_read,
	.open =		ml_open,
	.release =	ml_release,
};

static struct usb_class_driver ml_class = {
	.name = "ml%d",
	.fops = &ml_fops,
	.minor_base = ML_MINOR_BASE,
};

static int ml_probe(struct usb_interface *interface, const struct usb_device_id*id){
    struct usb_device *udev = interface_to_usbdev(interface);
    struct usb_ml *dev = NULL;
    struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
    int i, int_end_size, retval;
    
    printk(KERN_DEBUG "ml_probe called");

	if (!udev) {
		printk(KERN_DEBUG "udev is NULL");
		return -ENODEV;
	}

    dev = kzalloc(sizeof(struct usb_ml), GFP_KERNEL);
	if (!dev) {
		printk(KERN_DEBUG "cannot allocate memory for struct usb_ml");
		return -ENODEV;
	}

	dev->udev = udev;
	dev->interface = interface;

	usb_set_intfdata(interface, dev);

    retval = usb_register_dev(interface, &ml_class);
	if (retval) {
		printk(KERN_DEBUG "not able to get a minor for this device.");
		return retval;
	}

	dev->minor = interface->minor;

	printk(KERN_DEBUG "Device in /dev/ml%d", interface->minor - ML_MINOR_BASE);

    return 0;
}

static void ml_disconnect(struct usb_interface *interface){
    printk(KERN_DEBUG "ml_disconnect called");

	usb_deregister_dev(interface, &ml_class);
}

static struct usb_driver ml_driver = {
    .name = "mouse_test_driver",
    .id_table = ml_table,
    .probe = ml_probe,
    .disconnect = ml_disconnect,
};

static int __init usb_ml_init(void){
    int result;

	result = usb_register(&ml_driver);
	if (result) {
		printk(KERN_DEBUG "registering driver failed");
	} else {
		printk(KERN_DEBUG "driver registered successfully");
	}

	return result;
}

static void __exit usb_ml_exit(void){
    usb_deregister(&ml_driver);
	printk(KERN_DEBUG "module deregistered");
}

module_init(usb_ml_init);
module_exit(usb_ml_exit);

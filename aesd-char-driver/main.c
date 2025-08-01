/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("hangon1234");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    struct aesd_dev * dev;
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    // We initialized aesd_dev.lock in aesd_init_module
    // So nothing need to be released
    // TODO: free each entry
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    int i = 0;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    // Retrieve private_data
    struct aesd_dev *dev = (struct aesd_dev*) filp->private_data;
    struct aesd_circular_buffer buffer = dev->aesd_circular_buffer;
    
    // TODO: need to add appropriate locking 
    // f_pos is char_offset which is a specific position of circular buffer linear content
    size_t entry_offset_byte_rtn;
    struct aesd_buffer_entry* entry = aesd_circular_buffer_find_entry_offset_for_fpos(&buffer, *f_pos, &entry_offset_byte_rtn);
    if (entry != NULL) {
        if (count <= entry->size) {
            retval = count; // data is available
        } else {
            // only partial data available
            retval = entry->size;
        }
    } else {
        // no data is available
        retval = 0;
    }
    // update f_pos
    *f_pos = *f_pos + retval;

    // Copy buffer to user space
    copy_to_user(buf, entry->buffptr, entry->size);

    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle write
     */
    char* buffer;

    // Retrieve private_data
    struct aesd_dev *dev = (struct aesd_dev*) filp->private_data;
    struct aesd_circular_buffer aesd_buffer = dev->aesd_circular_buffer;
    struct aesd_buffer_entry entry;
    struct aesd_buffer_entry* entry_ptr;

    buffer = kmalloc(count, GFP_KERNEL);
    // to, from, count
    copy_from_user(buffer, buf, count);
    entry.buffptr = buffer;
    entry.size = count;

    // Add to the circular buffer
    entry_ptr = aesd_circular_buffer_add_entry(&aesd_buffer, &entry);

    // Free removed entry
    kfree(entry_ptr);

    return retval;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    mutex_init(&aesd_device.lock);
    // Initialize circular buffer
    aesd_circular_buffer_init(&aesd_device.aesd_circular_buffer);

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /* Destroy initialized mutex */
    mutex_destroy(&aesd_device.lock);

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

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

static int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    struct aesd_dev * dev;
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    PDEBUG("<%s:%d> dev addr: %p\n", __FUNCTION__, __LINE__, dev);
    PDEBUG("<%s:%d> aesd_circular_buffer addr: %p\n", __FUNCTION__, __LINE__, &dev->aesd_circular_buffer);
    filp->private_data = dev;
    return 0;
}

static int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release\n");
    // We initialized aesd_dev.sem in aesd_init_module
    // So nothing need to be released
    return 0;
}

static ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    // Retrieve private_data
    struct aesd_dev *dev = (struct aesd_dev*) filp->private_data;
    struct aesd_circular_buffer* buffer = &dev->aesd_circular_buffer;
    PDEBUG("<%s:%d> aesd_circular_buffer addr: 0x%llx\n", __FUNCTION__, __LINE__, (long long unsigned int)  &buffer);
    PDEBUG("<%s:%d> aesd_circular_buffer in_offs: %d\n", __FUNCTION__, __LINE__, buffer->in_offs);
    
    // TODO: need to add appropriate locking 
    // f_pos is char_offset which is a specific position of circular buffer linear content
    size_t entry_offset_byte_rtn;
    // down_read_trylock returns 1 if success
    if (down_read_trylock(&dev->sem) != 1) 
    {
        return -ERESTARTSYS;
    }
    PDEBUG("<%s:%d> lock obtained\n", __FUNCTION__, __LINE__);
    struct aesd_buffer_entry* entry = aesd_circular_buffer_find_entry_offset_for_fpos(buffer, *f_pos, &entry_offset_byte_rtn);
    up_read(&dev->sem);
    PDEBUG("<%s:%d> lock released\n", __FUNCTION__, __LINE__);

    if (entry != NULL) {

        if (count <= entry->size) {
            retval = count; // data is available
        } else {
            // only partial data available
            retval = entry->size;
        }
        // update f_pos
        *f_pos = *f_pos + retval;

        PDEBUG("<%s:%d> string from buffer: %s\n", __FUNCTION__, __LINE__, entry->buffptr);
        // Copy buffer to user space
        if (copy_to_user(buf, entry->buffptr, entry->size)) 
        {
            retval = -EFAULT;
        }

        // Initialize size and buffptr
        entry->buffptr = NULL;
        entry->size = 0;
    } else {
        // no data is available
        retval = 0;
        PDEBUG("<%s:%d> entry returned by aesd_circular_buffer_find_entry_offset_for_fpos is NULL\n", __FUNCTION__, __LINE__);
    }

    return retval;
}

static ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
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
    struct aesd_circular_buffer* aesd_buffer = &dev->aesd_circular_buffer;
    PDEBUG("<%s:%d> dev addr: %p\n", __FUNCTION__, __LINE__, dev);
    PDEBUG("<%s:%d> aesd_circular_buffer addr: %p\n", __FUNCTION__, __LINE__, &dev->aesd_circular_buffer);

    struct aesd_buffer_entry entry;
    const struct aesd_buffer_entry* entry_ptr;

    buffer = kmalloc(count, GFP_KERNEL);

    if (buffer == NULL) {
        PDEBUG("<%s:%d> Failed to allocate buffer, size: %lu", __FUNCTION__, __LINE__, count);
        retval = -ENOMEM;
        goto aesd_write_fail;
    }
    // to, from, count
    if (copy_from_user(buffer, buf, count))
    {
        retval = -EFAULT;
        goto aesd_write_fail;
    }
    entry.buffptr = buffer;
    PDEBUG("<%s:%d> copied from userspace: %s\n", __FUNCTION__, __LINE__, entry.buffptr);
    entry.size = count;
    retval = count;

    // down_write_trylock returns 1 if success;
    if (down_write_trylock(&dev->sem) != 1)
    {
        goto aesd_write_fail;
    }

    PDEBUG("<%s:%d> lock obtained\n", __FUNCTION__, __LINE__);
    // Add to the circular buffer
    entry_ptr = aesd_circular_buffer_add_entry(aesd_buffer, &entry);
    PDEBUG("<%s:%d> added\n", __FUNCTION__, __LINE__);
    up_write(&dev->sem);
    PDEBUG("<%s:%d> lock released\n", __FUNCTION__, __LINE__);

    PDEBUG("<%s:%d> in_offs: %d\n", __FUNCTION__, __LINE__, aesd_buffer->in_offs);
    // Free removed entry
    if (entry_ptr != NULL)
    {
        PDEBUG("<%s:%d> buffer freed after add entry\n", __FUNCTION__, __LINE__);
        kfree(entry_ptr->buffptr);
    }

    return retval;

  aesd_write_fail:
    if (buffer != NULL) 
    {
        kfree(buffer);
    }
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



static int aesd_init_module(void)
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

    init_rwsem(&aesd_device.sem);
    // Initialize circular buffer
    aesd_circular_buffer_init(&aesd_device.aesd_circular_buffer);

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    PDEBUG("<%s:%d> aesd_init_module done\n", __FUNCTION__, __LINE__);
    return result;

}

static void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

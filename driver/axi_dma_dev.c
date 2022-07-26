#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = {
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int major;

static int __init axi_dma_init(void) {
    major = register_chrdev(0, KBUILD_MODNAME, &fops);

    if (major < 0) {
        pr_alert("module loading failed register_chrdev returned %d", major);
        return major;
    }
    pr_info("module loaded major number %d\n", major);
    return 0;
}

static int dev_open(struct inode *inodep, struct file *filep) {
   pr_info("device opened\n");
   return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer,
                         size_t len, loff_t *offset) {
   pr_info("device write\n");
   return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
   pr_info("device close\n");
   return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    char *message = "pynq axi dma\n";
    ssize_t m_len = strlen(message);
    ssize_t bytes = len < (m_len-(*offset)) ? len : (m_len-(*offset));
    if(copy_to_user(buffer, message, m_len)){
        return -EFAULT;
    }
    (*offset) += bytes;
    return bytes;
}

static void __exit axi_dma_exit(void) {
    unregister_chrdev(major, KBUILD_MODNAME);
    pr_info("module has been unloaded\n");
}

module_init(axi_dma_init);
module_exit(axi_dma_exit);
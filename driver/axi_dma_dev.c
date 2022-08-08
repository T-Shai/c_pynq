#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <asm/io.h>

#include "pynq_utils.h"

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

volatile dma_registers_t *dma_regs;

static int __init axi_dma_init(void) {
    major = register_chrdev(0, KBUILD_MODNAME, &fops);
    if(major < 0) {
        pr_err("Failed to register character device\n");
        return major;
    }

    dma_regs = (dma_registers_t*)ioremap_nocache(DMA_BASE);
    if(!dma_regs) {
        pr_err("Failed to map DMA registers\n");
        return -ENOMEM;
    }


    // pr_info("DMA registers mapped to %p\n", dma_regs);
    // pr_info("%#x\n", readl(dma_regs));
    // pr_info("module loaded major number %d\n", major);
    return 0;
}

static void __exit axi_dma_exit(void) {
    unregister_chrdev(major, KBUILD_MODNAME);
    pr_info("module has been unloaded\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
   pr_info("device opened\n");
   return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer,
                         size_t len, loff_t *offset) {
   pr_info("device write %d\n", len);
   char tmp[100] = {0};
   copy_from_user(tmp, buffer, len);
   pr_info("copied %s", tmp);
   return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
   pr_info("device close\n");
   return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    pr_info("device read\n");
    // return in user buffer the dma status register
    uint32_t dma_status;
    dma_status = dma_regs->MM2S_DMASR
    return strlen(dma_status_str);
}

module_init(axi_dma_init);
module_exit(axi_dma_exit);
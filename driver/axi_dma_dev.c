#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <linux/ioctl.h>
#include "axi_dma_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("T-Sai");
MODULE_DESCRIPTION("AXI DMA driver");
MODULE_VERSION("0.1");



#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

/*
    File operations
*/

static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);
static long dev_ioctl(struct file*, unsigned int, unsigned long);

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
};

/*
    xilinx specific dma registers and constants

    https://docs.xilinx.com/r/en-US/pg021_axi_dma/AXI-DMA-Register-Address-Map

*/

typedef struct
{
    uint32_t MM2S_DMACR;    // 0x0 MM2S DMA Control register
    uint32_t MM2S_DMASR;    // 0x4 MM2S DMA Status register
    uint32_t RESERVED0;     // 0x8 Reserved
    uint32_t RESERVED1;     // 0xC Reserved
    uint32_t RESERVED2;     // 0x10 Reserved
    uint32_t RESERVED3;     // 0x14 Reserved
    uint32_t MM2S_SA;       // 0x18 MM2S Source Address. Lower 32 bits of address.
    uint32_t MM2S_SA_MSB;   // 0x1C MM2S Source Address. Upper 32 bits of address.
    uint32_t RESERVED4;     // 0x20 Reserved
    uint32_t RESERVED5;     // 0x24 Reserved
    uint32_t MM2S_LENGTH;   // 0x28 MM2S Transfer Length (Bytes)
    uint32_t RESERVED6;     // 0x2C Reserved
    uint32_t S2MM_DMACR;    // 0x30 S2MM DMA Control register
    uint32_t S2MM_DMASR;    // 0x34 S2MM DMA Status register
    uint32_t RESERVED7;     // 0x38 Reserved
    uint32_t RESERVED8;     // 0x3C Reserved
    uint32_t RESERVED9;     // 0x40 Reserved
    uint32_t RESERVED10;    // 0x44 Reserved
    uint32_t S2MM_DA;       // 0x48 S2MM Destination Address. Lower 32 bits of address.
    uint32_t S2MM_DA_MSB;   // 0x4C S2MM Destination Address. Upper 32 bits of address.
    uint32_t RESERVED11;    // 0x50 Reserved
    uint32_t RESERVED12;    // 0x54 Reserved
    uint32_t S2MM_LENGTH;   // 0x58 S2MM Transfer Length (Bytes)
} dma_registers_t;

typedef enum {
                            // Default value    Access type     Description
    DMA_CR_RUN_STOP = 0,           // 0                R/W             0 = STOP, 1 = RUN
    // RESERVED = 1,
    DMA_CR_RESET = 2,              // 0                R/W             0 = Normal operation, 1 = Reset in progress
    DMA_CR_KEYHOLE = 3,            // 0                R/W             0 = Normal operation, 1 = AXI DMA initiate MM2S reads
    DMA_CR_CYCLIC_BD_EN = 4,       // 0                R/W             0 = cyclic BDs disabled, 1 = cyclic BDs enabled
    // RESERVED = 5,
    // RESERVED = 6,
    // RESERVED = 7,
    // RESERVED = 8,
    // RESERVED = 9,
    // RESERVED = 10,
    // RESERVED = 11,
    DMA_CR_IOC_IRQ_EN = 12,        // 0                R/W             Interrupt on Complete 0 = IRQ disabled, 1 = IRQ enabled
    DMA_CR_Dly_IRQ_EN = 13,        // 0                R/W             Interrupt on delay    0 = IRQ disabled, 1 = IRQ enabled
    DMA_CR_ERR_IRQ_EN = 14,        // 0                R/W             Interrupt on error    0 = IRQ disabled, 1 = IRQ enabled

    // RESERVED = 15,
    // rest are ignored in direct register mode. used in scather gather mode
} DMA_CR;

typedef enum {
                            // Default value    Access type     Description
    DMA_SR_HALTED = 0,             // 1                RO              0 = DMA channel is running, 1 = DMA channel is halted
    DMA_SR_IDLE = 1,               // 0                RO              0 = DMA channel is not idle, 1 = DMA channel is idle
    // RESERVED = 2,
    DMA_SR_SG_INCLD = 3,           // 0                RO              0 = Scatter/gather mode disabled, 1 = Scatter/gather mode enabled
    DMA_SR_DMA_INT_ERR = 4,        // 0                RO              0 = No internal error, 1 = Internal error occurred and DMA channel is stopped
    DMA_SR_DMA_SLV_ERR = 5,        // 0                RO              0 = No slave error, 1 = Slave error occurred and DMA channel is stopped
    DMA_SR_DMA_DEC_ERR = 6,        // 0                RO              0 = No decode error, 1 = Decode error occurred and DMA channel is stopped
    // RESERVED = 7,
    DMA_SR_SG_INT_ERR = 8,         // 0                RO              0 = No SG internal error, 1 = SG Internal error occurred and DMA channel is stopped
    DMA_SR_SG_SLV_ERR = 9,         // 0                RO              0 = No SG slave error, 1 = SG Slave error occurred and DMA channel is stopped
    DMA_SR_SG_DEC_ERR = 10,        // 0                RO              0 = No SG decode error, 1 = SG Decode error occurred and DMA channel is stopped
    // RESERVED = 11,
    DMA_SR_IOC_IRQ = 12,           // 0                R/WC           0 = No IOC interrupt, 1 = ioc Interrupt occurred. Writing a 1 to this bit clears it.
    // (ignored in DIRECT REGISTER MODE) Dly_IRQ = 13,           // 0                R/WC           0 = No delay interrupt, 1 = Delay interrupt occurred. Writing a 1 to this bit clears it.
    DMA_SR_ERR_IRQ = 14,           // 0                R/WC           0 = No error interrupt, 1 = Error interrupt occurred. Writing a 1 to this bit clears it.
    // RESERVED = 15,
    // rest are ignored in direct register mode. used in scather gather mode
} DMA_SR;

/*

    dma device
*/

typedef struct  
{
    uint64_t base_addr;
    
    /* resources */
    struct resource *regs;
    void __iomem *virtual_addr;

    /* channels */
    uint64_t send_chan_addr;
    uint64_t send_chan_size;

    uint64_t rcv_chan_addr;
    uint64_t rcv_chan_size;
} dma_device;

/* device driver */
static int major;

/* dma device */
dma_device *dma_dev;

static int __init axi_dma_init(void) {
    major = register_chrdev(0, KBUILD_MODNAME, &fops);
    if(major < 0) {
        pr_err("Failed to register character device\n");
        return major;
    } else {
        pr_info("Registered character device with major number %d\n", major);
    }

    dma_dev = kzalloc(sizeof(dma_device), GFP_KERNEL);
    if(!dma_dev) {
        pr_err("Failed to allocate memory for dma_dev\n");
        return -ENOMEM;
    }

    return 0;
}

static void __exit axi_dma_exit(void) {
    unregister_chrdev(major, KBUILD_MODNAME);
    // iounmap(dma_dev.virtual_addr);
    // release_mem_region(DMA_BASE, REGISTER_SIZE);
    kfree(dma_dev);
    pr_info("module has been unloaded\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
   pr_info("device opened\n");
   return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer,
                         size_t len, loff_t *offset) {
    pr_info("device written\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
   pr_info("device close\n");
   return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    pr_info("device read\n");
    // return base addr and channel sizes to user
    char *buf = kzalloc(1024*4, GFP_KERNEL);
    sprintf(buf, "Base addr : 0x%llx\n", dma_dev->base_addr);
    sprintf(buf + strlen(buf), "Send chan size : %lld\n", dma_dev->send_chan_size);
    sprintf(buf + strlen(buf), "Recv chan size : %lld\n", dma_dev->rcv_chan_size);    
    sprintf(buf + strlen(buf), "Send chan addr : 0x%llx\n", dma_dev->send_chan_addr);    
    sprintf(buf + strlen(buf), "Recv chan addr : 0x%llx\n", dma_dev->rcv_chan_addr);    
    size_t wrote_len = copy_to_user(buffer, buf, strlen(buf));
    kfree(buf);
    return wrote_len;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    pr_info("device ioctl\n");
    dma_regs_info_t user_data;
    switch(cmd) {
        case AXIDMA_IOC_SET_DATA:
            if(copy_from_user(&user_data, (uint64_t *)arg, sizeof(dma_regs_info_t))) {
                pr_err("Failed to copy base addr from user\n");
                return -EFAULT;
            }
            dma_dev->base_addr = user_data.base_addr;
            dma_dev->send_chan_addr = user_data.send_chan_addr;
            dma_dev->send_chan_size = user_data.send_chan_size;
            dma_dev->rcv_chan_addr = user_data.rcv_chan_addr;
            dma_dev->rcv_chan_size = user_data.rcv_chan_size;
            break;

        default:
            return -EINVAL;
    }
    return 1;
}

module_init(axi_dma_init);
module_exit(axi_dma_exit);
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
#include <linux/stddef.h>
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



/* bit operations */
#define SET_BIT(x, pos) (x |= (1U << pos))
#define CLEAR_BIT(x, pos) (x &= (~(1U<< pos)))
#define GET_BIT(x, pos) ((x>>pos) & 1U)

/* read write 32 bit from dma registers */
#define DMA_RD(virt_addr, field)  ioread32(virt_addr + offsetof(dma_registers_t, field))
#define DMA_WR(virt_addr, field, value)  iowrite32(value, virt_addr + offsetof(dma_registers_t, field))

volatile void __iomem *regs_vaddr = NULL;
struct resource *regs_res = NULL;
dma_info_t* user_info;

void dma_status_info(volatile void* regs_vaddr) 
{
    unsigned int status = DMA_RD(regs_vaddr, MM2S_DMASR);
    pr_info("MM2S :");
    if (status & 0x00000001) pr_info(" halted"); else pr_info(" running");
    if (status & 0x00000002) pr_info(" idle");
    if (status & 0x00000008) pr_info(" SGIncld");
    if (status & 0x00000010) pr_info(" DMAIntErr");
    if (status & 0x00000020) pr_info(" DMASlvErr");
    if (status & 0x00000040) pr_info(" DMADecErr");
    if (status & 0x00000100) pr_info(" SGIntErr");
    if (status & 0x00000200) pr_info(" SGSlvErr");
    if (status & 0x00000400) pr_info(" SGDecErr");
    if (status & 0x00001000) pr_info(" IOC_Irq");
    if (status & 0x00002000) pr_info(" Dly_Irq");
    if (status & 0x00004000) pr_info(" Err_Irq");

    pr_info("S2MM :");
    status = DMA_RD(regs_vaddr, S2MM_DMASR);
    if (status & 0x00000001) pr_info(" halted"); else pr_info(" running");
    if (status & 0x00000002) pr_info(" idle");
    if (status & 0x00000008) pr_info(" SGIncld");
    if (status & 0x00000010) pr_info(" DMAIntErr");
    if (status & 0x00000020) pr_info(" DMASlvErr");
    if (status & 0x00000040) pr_info(" DMADecErr");
    if (status & 0x00000100) pr_info(" SGIntErr");
    if (status & 0x00000200) pr_info(" SGSlvErr");
    if (status & 0x00000400) pr_info(" SGDecErr");
    if (status & 0x00001000) pr_info(" IOC_Irq");
    if (status & 0x00002000) pr_info(" Dly_Irq");
    if (status & 0x00004000) pr_info(" Err_Irq");
}

void reset_DMA(void)
{
    if(!regs_vaddr) return;
    u32 val = 0;
    SET_BIT(val, DMA_CR_RESET);
    DMA_WR(regs_vaddr, MM2S_DMACR, val);
    DMA_WR(regs_vaddr, S2MM_DMACR, val);
}

void halt_DMA(void)
{
    if(!regs_vaddr) return;
    DMA_WR(regs_vaddr, MM2S_DMACR, 0);
    DMA_WR(regs_vaddr, S2MM_DMACR, 0);
}

void start_transfer(void)
{   
    if(!regs_vaddr) return;
    // start with IOC and ERR interrupts enabled
    u32 value = 0;
    SET_BIT(value, DMA_CR_RUN_STOP);
    SET_BIT(value, DMA_CR_IOC_IRQ_EN);
    SET_BIT(value, DMA_CR_ERR_IRQ_EN);

    DMA_WR(regs_vaddr, MM2S_DMACR, value);
    DMA_WR(regs_vaddr, S2MM_DMACR, value);
}

void attach_buffers(u64 saddr, u64 raddr)
{
    if(!regs_vaddr) return;
    DMA_WR(regs_vaddr, MM2S_SA, saddr);
    DMA_WR(regs_vaddr, S2MM_DA, raddr);
}

void set_lengths(u64 ssize, u64 rsize)
{
    if(!regs_vaddr) return;
    DMA_WR(regs_vaddr, MM2S_LENGTH, ssize);
    DMA_WR(regs_vaddr, S2MM_LENGTH, rsize);
}

void cleanup(void)
{
    if(regs_res)
    {
        release_mem_region(regs_res->start, resource_size(regs_res));
        regs_res = NULL;
    }

    if(regs_vaddr)
    {
        iounmap(regs_vaddr);
        regs_vaddr = NULL;
    }
}

int request_and_map(u64 base_addr)
{
    cleanup();
    regs_res = request_mem_region( base_addr, sizeof(dma_registers_t), KBUILD_MODNAME);
    if(!regs_res)
    {
        pr_err("request mem region failed for 0x%lx", base_addr);
        return 0;
    }

    regs_vaddr = (dma_registers_t*) ioremap_nocache(base_addr, sizeof(dma_registers_t));
    if(!regs_vaddr)
    {

        pr_err("ioremap_nocache failed");
        return 0;
    }
    return 1;
}
/* device driver */
static int major;

static int __init axi_dma_init(void) 
{
    major = register_chrdev(0, KBUILD_MODNAME, &fops);
    if(major < 0) {
        pr_err("Failed to register character device\n");
        return major;
    } else {
        pr_info("Registered character device with major number %d\n", major);
    }

    user_info = kmalloc(sizeof(dma_info_t), GFP_KERNEL);

    return 0;
}

static void __exit axi_dma_exit(void) 
{
    cleanup();
    kfree(user_info);
    unregister_chrdev(major, KBUILD_MODNAME);
    pr_info("module has been unloaded\n");
}

static int dev_open(struct inode *inodep, struct file *filep) 
{
    pr_info("device opened\n");

    // if(!request_and_map(0x40400000))
    // {
    //     return 0;
    // }

    // dma_status_info(regs_vaddr);

    // reset_DMA();
    // dma_status_info(regs_vaddr);

    // halt_DMA();
    // dma_status_info(regs_vaddr);

    // start_transfer();
    // dma_status_info(regs_vaddr);

    return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer,
                         size_t len, loff_t *offset) 
{
    pr_info("device written\n");
    if(! (regs_res && regs_vaddr))
    {
        pr_err("resource not map");
        return -EINVAL;
    }

    // dma_status_info(regs_vaddr);

    reset_DMA();
    // dma_status_info(regs_vaddr);

    halt_DMA();
    dma_status_info(regs_vaddr);

    attach_buffers(user_info->saddr, user_info->raddr);

    start_transfer();
    dma_status_info(regs_vaddr);

    set_lengths(user_info->ssize, user_info->rsize);
    dma_status_info(regs_vaddr);

    return len;
}


static ssize_t dev_read(struct file *filep, char *buffer,
                        size_t len, loff_t *offset)
{
    pr_info("device read\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) 
{
   pr_info("device close\n");
   return 0;
}


static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    pr_info("device ioctl\n");
    // dma_info_t ioctl_uinfo; // error if not in stack
    switch (cmd)
    {
    case AXIDMA_IOC_INFO:
        if(copy_from_user(user_info, (uint64_t *)arg, sizeof(dma_info_t))) {
                pr_err("Failed to copy base addr from user\n");
                return -EFAULT;
        }

        if(!request_and_map(user_info->base_addr))
        {
            return -EINVAL;
        }
        break;
    
    default:
        break;
    }


    return 1;
}
module_init(axi_dma_init);
module_exit(axi_dma_exit);
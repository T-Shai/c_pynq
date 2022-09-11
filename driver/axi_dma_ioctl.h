#ifndef KBUILD_MODNAME
#include <stdint.h>
#define  adi_u64  uint64_t
#define  adi_u32  uint32_t

#else
#include <linux/types.h>
#define  adi_u64  u64
#define  adi_u32  u32
#endif


typedef struct
{
    adi_u64 base_addr;  /* dma base address */

    adi_u64 saddr;      /* send buffer physical address */
    adi_u64 ssize;      /* send buffer size */

    adi_u64 raddr;      /* rcv buffer physical address */
    adi_u64 rsize;      /* rcv buffer size */
} dma_info_t;


typedef struct
{
    adi_u64 base_addr;  /* dma base address */
    adi_u64 offset;      /* send buffer physical address */
    adi_u32 value;      /* send buffer size */
} mmio_info_t;


#define AXIDMA_IOC_MAGIC 'a'

#define AXIDMA_IOC_INFO             _IO(AXIDMA_IOC_MAGIC, 1)    /* info from user to device driver */
#define AXIDMA_IOC_MMIO_WR          _IO(AXIDMA_IOC_MAGIC, 2)    /* write to phys address from user */
#define AXIDMA_IOC_MMIO_RD          _IO(AXIDMA_IOC_MAGIC, 3)    /* NOT IMPLEMENTED */
#define AXIDMA_IOC_START            _IO(AXIDMA_IOC_MAGIC, 4)    /* start transfert */


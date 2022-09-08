#ifndef KBUILD_MODNAME
#include <stdint.h>
#define  adi_u64  int64_t
#else
#include <linux/types.h>
#define  adi_u64  u64
#endif


typedef struct
{
    adi_u64 base_addr;  /* dma base address */

    adi_u64 saddr;      /* send buffer physical address */
    adi_u64 ssize;      /* send buffer size */

    adi_u64 raddr;      /* rcv buffer physical address */
    adi_u64 rsize;      /* rcv buffer size */
} dma_info_t;



#define AXIDMA_IOC_MAGIC 'a'

#define AXIDMA_IOC_INFO        _IO(AXIDMA_IOC_MAGIC, 1)
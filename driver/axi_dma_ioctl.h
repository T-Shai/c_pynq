typedef struct
{
    unsigned long long base_addr;
    /* channels */
    unsigned long long send_chan_addr;
    unsigned long long send_chan_size;

    unsigned long long rcv_chan_addr;
    unsigned long long rcv_chan_size;
} dma_regs_info_t;



#define AXIDMA_IOC_MAGIC 'a'

#define AXIDMA_IOC_SET_DATA        _IO(AXIDMA_IOC_MAGIC, 1)
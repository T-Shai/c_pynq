#include "pynq_utils.h"
#include "mem_alloc.h"
#include "libxlnk_cma.h"

#include "stdlib.h"

int mem_alloc1(unsigned int** send_buff, unsigned int** recv_buff, size_t len)
{
    *send_buff = get_mmio(DMA_SEND_ADDR, len);
    if(!send_buff) return 0;
    *recv_buff = get_mmio(DMA_RECV_ADDR, len);
    if(!recv_buff) return 0;

    return 1;
}

int mem_alloc2(unsigned int** send_buff, unsigned int** recv_buff, size_t len)
{
    *send_buff = cma_alloc(len, 0);
    if(!send_buff) return 0;
    *recv_buff = cma_alloc(len,0);
    if(!recv_buff) return 0;
    return 1;
}
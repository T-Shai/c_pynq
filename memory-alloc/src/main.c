#include "pynq_utils.h"
#include "mem_alloc.h"
#include "libxlnk_cma.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#define THRES       127
#define DATA_SIZE   10

void fill_random(unsigned int* buff, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        buff[i] = rand()%255;
    }
}

int compare_arrays(unsigned int* ibuff1, unsigned int* ibuff2, size_t data_size)
{
    unsigned char *buff1 =  ibuff1;
    unsigned char *buff2 =  ibuff2;

    for(size_t i = 0; i < data_size; i++ )
    {
        if( buff1[i] != buff2[i]){
            printf("buff[%u] : %02x != %02x \n", i,  buff1[i], buff2[i]);
            return 0;
        }
    }
    
    return 1;
}


void memdump(void* virtual_address, int byte_count)
{
    char *p = virtual_address;
    int offset;
    for (offset = 0; offset < byte_count; offset++) {
        printf("%02x", p[offset]);
        if (offset % 4 == 3) { printf(" "); }
    }
    printf("\n");
}

// #define DIRTY_MMAP
// #define MALLOC
// #define CMA_ALLOC

#if defined DIRTY_MMAP
#define MYALLOC mem_alloc1
#define MYALLOC_STR "get_mmio"
#define MYFREE(b, s) free_mmio(b, s)
#elif defined MALLOC
#define MYALLOC mem_alloc2
#define MYALLOC_STR "malloc"
#define MYFREE(b, s) free(b)
#elif defined CMA_ALLOC
#define MYALLOC mem_alloc3
#define MYALLOC_STR "cma_alloc"
#define MYFREE(b, s) cma_free(b)
#else
#define MYALLOC mem_alloc1
#define MYALLOC_STR "get_mmio"
#define MYFREE(b, s) free_mmio(b, s)
#endif

#ifdef CMA_ALLOC
#define CACHE   cma_invalidate_cache(send_buff, cma_get_phy_addr(recv_buff), DATA_SIZE*sizeof(unsigned int));\
                cma_flush_cache(send_buff, cma_get_phy_addr(recv_buff), DATA_SIZE*sizeof(unsigned int));\
                cma_invalidate_cache(recv_buff, cma_get_phy_addr(recv_buff), DATA_SIZE*sizeof(unsigned int));\
                cma_flush_cache(recv_buff, cma_get_phy_addr(recv_buff), DATA_SIZE*sizeof(unsigned int));
#else
#define CACHE
#endif

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    // loading bitstream
    load_bitstream("/home/xilinx/pynq/overlays/threshold/threshold.bit");

    //getting the status of DMA channels
    unsigned int* dma_mmio = get_mmio(DMA_BASE, REGISTER_SIZE);
    if(!dma_mmio) return 1;
    unsigned int* send_buff;
    unsigned int* recv_buff;

    if(!MYALLOC(&send_buff, &recv_buff, DATA_SIZE*sizeof(unsigned int))) {
        printf("%s(...) failed !\n", MYALLOC_STR);
        return 1;
    }
    printf("%s(...) sucess !\n", MYALLOC_STR);

    unsigned int* ip_mmio = get_mmio(IP_AXI_LITE_BASE, REGISTER_SIZE);
    if(!ip_mmio) return 1;

    ip_mmio[0x0] = 0x81;
    
    memset(recv_buff, 0, DATA_SIZE*sizeof(unsigned int));
    fill_random(send_buff, DATA_SIZE);

CACHE

    printf("Resetting DMA\n");
    reset_DMA(dma_mmio);

    printf("Halting DMA\n");
    halt_DMA(dma_mmio);

    printf("attaching buffers...\n");
    attach_recv_buff(dma_mmio, DMA_RECV_ADDR);
    attach_send_buff(dma_mmio, DMA_SEND_ADDR);

    printf("starting transfer...\n");
    start_transfer(dma_mmio);

    printf("setting lengths...\n");
    set_transfer_lengths(dma_mmio, DATA_SIZE, DATA_SIZE);

    printf("waiting transfer's end...\n");
    wait_transfer(dma_mmio);
CACHE

    for (size_t i = 0; i < DATA_SIZE; i++)
    {
        send_buff[i] = (send_buff[i] <= THRES) ? 0 : 255;
    }
CACHE

    int check = compare_arrays(send_buff, recv_buff, DATA_SIZE);

    printf("Source memory block:      \n"); memdump(send_buff, DATA_SIZE*sizeof(unsigned int));
    printf("Destination memory block: \n"); memdump(recv_buff, DATA_SIZE*sizeof(unsigned int));    
    
    if(check)
    {
        printf("Software and hardware are equals !\n");
    } else {
        printf("Software and hardware are NOT equals !\n");
    }

    MYFREE(recv_buff, DATA_SIZE*sizeof(unsigned int));
    MYFREE(send_buff, DATA_SIZE*sizeof(unsigned int));
    free_mmio(dma_mmio, REGISTER_SIZE);
    free_mmio(ip_mmio, REGISTER_SIZE);

    return 0;
}

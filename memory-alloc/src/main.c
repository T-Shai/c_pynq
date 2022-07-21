#include "pynq_utils.h"
#include "mem_alloc.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#define THRES       127
#define DATA_SIZE   100

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

    if(!mem_alloc1(&send_buff, &recv_buff, DATA_SIZE*sizeof(unsigned int))) {
        printf("mem_alloc1(...) failed !\n");
        return 1;
    }

    // if(!mem_alloc2(&send_buff, &recv_buff, DATA_SIZE*sizeof(unsigned int))) {
    //     printf("mem_alloc2(...) failed !\n");
    //     return 1;
    // }

    unsigned int* ip_mmio = get_mmio(IP_AXI_LITE_BASE, REGISTER_SIZE);
    if(!ip_mmio) return 1;

    ip_mmio[0x0] = 0x81;
    
    memset(recv_buff, 0, DATA_SIZE*4);

    fill_random(send_buff, DATA_SIZE);

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

    for (size_t i = 0; i < DATA_SIZE; i++)
    {
        send_buff[i] = (send_buff[i] <= THRES) ? 0 : 255;
    }

    
    int check = compare_arrays(send_buff, recv_buff, DATA_SIZE);

    printf("Source memory block:      \n"); memdump(send_buff, 4*4);
    printf("Destination memory block: \n"); memdump(recv_buff, 4*4);    
    
    if(check)
    {
        printf("Software and hardware are equals !\n");
    } else {
        printf("Software and hardware are NOT equals !\n");
    }

    /*
        Todo free
    */
    free_mmio(dma_mmio, REGISTER_SIZE);
    free_mmio(ip_mmio, REGISTER_SIZE);

    return 0;
}

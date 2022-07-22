#include "pynq_utils.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int load_bitstream(const char* bitstream_name)
{
    char cmd_buff[256];
    strcpy(cmd_buff, "python3 src/bitstream_loader.py ");
    strcat(cmd_buff, bitstream_name);
    return system(cmd_buff);
}


void print_status(unsigned int* dma_virtual_address)
{
    printf("status of S2MM: \n");
    unsigned int status = dma_virtual_address[S2MM_STATUS_REGISTER];
    if (status & 0x00000001) printf(" halted"); else printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n");

    printf("status of MM2S: \n");
    status = dma_virtual_address[MM2S_STATUS_REGISTER];
    if (status & 0x00000001) printf(" halted"); else printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n");
}

unsigned int* get_mmio(off_t mem_addr, size_t size) 
{
    int dh = open("/dev/mem", O_RDWR | O_SYNC);
    if(dh < 0){
        printf("Error occured while opening /dev/mem.\n");
        return NULL;
    }
    unsigned int* virtual_source_address  = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dh, mem_addr);
    if(!virtual_source_address){
        printf("Error occured while mapping to /dev/mem \n");
        return NULL;
    }
    return virtual_source_address;
}

void free_mmio(unsigned int* virtual_address, size_t size)
{
    munmap((void *)virtual_address, size);
}

void reset_DMA(unsigned int* dma_virtual_address)
{
    dma_virtual_address[S2MM_CONTROL_REGISTER] = 4;
    dma_virtual_address[MM2S_CONTROL_REGISTER] = 4;
}

void halt_DMA(unsigned int* dma_virtual_address)
{
    dma_virtual_address[S2MM_CONTROL_REGISTER] = 0;
    dma_virtual_address[MM2S_CONTROL_REGISTER] = 0;
}

void attach_send_buff(unsigned int* DMA_base_addr, off_t mem_addr)
{
    DMA_base_addr[MM2S_START_ADDRESS] = mem_addr;
}

void attach_recv_buff(unsigned int* DMA_base_addr, off_t mem_addr)
{
    DMA_base_addr[S2MM_DESTINATION_ADDRESS] = mem_addr;
}

void set_transfer_lengths(unsigned int* DMA_base_addr, size_t len_send, size_t len_recv)
{
    DMA_base_addr[MM2S_LENGTH] = len_send;
    DMA_base_addr[S2MM_LENGTH] = len_recv;
}

void start_transfer(unsigned int* DMA_base_addr)
{
    DMA_base_addr[MM2S_CONTROL_REGISTER] = 0xf001;
    DMA_base_addr[S2MM_CONTROL_REGISTER] = 0xf001;  // 0xf001 stands for starting with all interrupts masked
}

void wait_transfer(unsigned int* DMA_base_addr)
{
    /*
        1 << 1  : idle;
        1 << 12 : IOC_Irq; interrupt on complete
    */
    unsigned int status = DMA_base_addr[MM2S_STATUS_REGISTER];
    while ( ((status>> 1) & 0x00000001) != 1 ){}

    status = DMA_base_addr[S2MM_STATUS_REGISTER];
    while ( ((status>> 1) & 0x00000001) != 1 ){}
}

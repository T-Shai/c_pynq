#ifndef PYNQ_UTILS_H
#define PYNQ_UTILS_H

#include <sys/types.h>

#define DMA_BASE (0x41E00000)

#define MM2S_CONTROL_REGISTER (0x00 >> 2)
#define MM2S_STATUS_REGISTER (0x04 >> 2)
#define MM2S_START_ADDRESS (0x18 >> 2)
#define MM2S_LENGTH (0x28 >> 2)

#define S2MM_CONTROL_REGISTER (0x30 >> 2)
#define S2MM_STATUS_REGISTER (0x34 >> 2)
#define S2MM_DESTINATION_ADDRESS (0x48 >> 2)
#define S2MM_LENGTH (0x58 >> 2)

#define REGISTER_SIZE (0x1000)

#define DMA_SEND_ADDR (0x0e000000)
#define DMA_RECV_ADDR (0x0f000000)

#define IP_AXI_LITE_BASE (0x40000000)

int load_bitstream(const char* bitstream_name);

/*
    print the status S2MM and MM2S.
*/

void print_status(unsigned int* dma_virtual_address);

/*
    unmaps the given DMA base address.
*/

void reset_DMA(unsigned int* dma_virtual_address);

void halt_DMA(unsigned int* dma_virtual_address);

unsigned int* get_mmio( off_t mem_addr, size_t size);

void free_mmio(unsigned int* virtual_address, size_t size);

void attach_send_buff(unsigned int* DMA_base_addr, off_t mem_addr);

void attach_recv_buff(unsigned int* DMA_base_addr, off_t mem_addr);

void set_transfer_lengths(unsigned int* DMA_base_addr, size_t len_send, size_t len_recv);

void start_transfer(unsigned int* DMA_base_addr);

void wait_transfer(unsigned int* DMA_base_addr);



#endif // PYNQ_UTILS_H
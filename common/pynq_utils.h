#ifndef PYNQ_UTILS_H
#define PYNQ_UTILS_H

#include <sys/types.h>
#include "stdint.h"
#include "string.h"

// set base's nth bit to 0
#define SET_TO_ZERO(base, n_bit) (base &= ~(1 << n_bit))

// set base's nth bit to 1
#define SET_TO_ONE(base, n_bit) (base |= (1 << n_bit))

// get the value of the nth bit of base
#define GET_BIT(base, n_bit) ((base >> n_bit) & 1)


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


#define DMA_BASE (0x41E00000)

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
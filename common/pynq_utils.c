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
    dma_registers_t *regs = (dma_registers_t *)dma_virtual_address;
    uint32_t status_reg = regs->MM2S_DMASR;
    if (GET_BIT(status_reg, DMA_SR_HALTED)) printf(" halted"); else printf(" running");
    if (GET_BIT(status_reg, DMA_SR_IDLE)) printf(" idle");
    if (GET_BIT(status_reg, DMA_SR_SG_INCLD)) printf(" SGIncld");
    if (GET_BIT(status_reg, DMA_SR_DMA_INT_ERR)) printf(" DMAIntErr");
    if (GET_BIT(status_reg, DMA_SR_DMA_SLV_ERR)) printf(" DMASlvErr");
    if (GET_BIT(status_reg, DMA_SR_DMA_DEC_ERR)) printf(" DMADecErr");
    if (GET_BIT(status_reg, DMA_SR_SG_INT_ERR)) printf(" SGIntErr");
    if (GET_BIT(status_reg, DMA_SR_SG_SLV_ERR)) printf(" SGSlvErr");
    if (GET_BIT(status_reg, DMA_SR_SG_DEC_ERR)) printf(" SGDecErr");
    if (GET_BIT(status_reg, DMA_SR_IOC_IRQ)) printf(" IOC_Irq");
    if (GET_BIT(status_reg, DMA_SR_ERR_IRQ)) printf(" Err_Irq");
    printf("\n");

    printf("status of MM2S: \n");
    
    status_reg = regs->S2MM_DMASR;
    if (GET_BIT(status_reg, DMA_SR_HALTED)) printf(" halted"); else printf(" running");
    if (GET_BIT(status_reg, DMA_SR_IDLE)) printf(" idle");
    if (GET_BIT(status_reg, DMA_SR_SG_INCLD)) printf(" SGIncld");
    if (GET_BIT(status_reg, DMA_SR_DMA_INT_ERR)) printf(" DMAIntErr");
    if (GET_BIT(status_reg, DMA_SR_DMA_SLV_ERR)) printf(" DMASlvErr");
    if (GET_BIT(status_reg, DMA_SR_DMA_DEC_ERR)) printf(" DMADecErr");
    if (GET_BIT(status_reg, DMA_SR_SG_INT_ERR)) printf(" SGIntErr");
    if (GET_BIT(status_reg, DMA_SR_SG_SLV_ERR)) printf(" SGSlvErr");
    if (GET_BIT(status_reg, DMA_SR_SG_DEC_ERR)) printf(" SGDecErr");
    if (GET_BIT(status_reg, DMA_SR_IOC_IRQ)) printf(" IOC_Irq");
    if (GET_BIT(status_reg, DMA_SR_ERR_IRQ)) printf(" Err_Irq");
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
    dma_registers_t *regs = (dma_registers_t *)dma_virtual_address;
    SET_TO_ONE(regs->S2MM_DMACR, DMA_CR_RESET);
    SET_TO_ONE(regs->MM2S_DMACR, DMA_CR_RESET);
}

void halt_DMA(unsigned int* dma_virtual_address)
{
    dma_registers_t *regs = (dma_registers_t *)dma_virtual_address;
    SET_TO_ZERO(regs->S2MM_DMACR, DMA_CR_RUN_STOP);
    SET_TO_ZERO(regs->MM2S_DMACR, DMA_CR_RUN_STOP);
}

void attach_send_buff(unsigned int* DMA_base_addr, off_t mem_addr)
{
    dma_registers_t *regs = (dma_registers_t *)DMA_base_addr;
    regs->MM2S_SA = mem_addr;
}

void attach_recv_buff(unsigned int* DMA_base_addr, off_t mem_addr)
{
    dma_registers_t *regs = (dma_registers_t *)DMA_base_addr;
    regs->S2MM_DA = mem_addr;
}

void set_transfer_lengths(unsigned int* DMA_base_addr, size_t len_send, size_t len_recv)
{
    dma_registers_t *regs = (dma_registers_t *)DMA_base_addr;
    regs->MM2S_LENGTH = len_send;
    regs->S2MM_LENGTH = len_recv;
}

void start_transfer(unsigned int* DMA_base_addr)
{   
    // start with IOC and ERR interrupts enabled
    dma_registers_t *regs = (dma_registers_t *)DMA_base_addr;
    SET_TO_ONE(regs->MM2S_DMACR, DMA_CR_RUN_STOP);
    SET_TO_ONE(regs->MM2S_DMACR, DMA_CR_IOC_IRQ_EN);
    SET_TO_ONE(regs->MM2S_DMACR, DMA_CR_ERR_IRQ_EN);

    SET_TO_ONE(regs->S2MM_DMACR, DMA_CR_RUN_STOP);
    SET_TO_ONE(regs->S2MM_DMACR, DMA_CR_IOC_IRQ_EN);
    SET_TO_ONE(regs->S2MM_DMACR, DMA_CR_ERR_IRQ_EN);
}

void wait_transfer(unsigned int* DMA_base_addr)
{
    /*
        1 << 1  : idle;
        1 << 12 : IOC_Irq; interrupt on complete
    */
    dma_registers_t *regs = (dma_registers_t *)DMA_base_addr;
    while (( GET_BIT(regs->MM2S_DMASR, DMA_SR_IDLE) != 1 ) || ( GET_BIT(regs->S2MM_DMASR, DMA_SR_IDLE) != 1 )){}

}

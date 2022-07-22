#include "pynq_utils.h"
#include "mem_alloc.h"
#include "libxlnk_cma.h"

#include <linux/ioctl.h>
#include "stdlib.h"

int mem_alloc1(unsigned int** send_buff, unsigned int** recv_buff, size_t len)
{
    *send_buff = get_mmio(DMA_SEND_ADDR, len);
    if(!send_buff) return 0;
    *recv_buff = get_mmio(DMA_RECV_ADDR, len);
    if(!recv_buff) return 0;

    return 1;
}

uintptr_t vtop(uintptr_t vaddr) {
    FILE *pagemap;
    intptr_t paddr = 0;
    int offset = (vaddr / sysconf(_SC_PAGESIZE)) * sizeof(uint64_t);
    uint64_t e;

    // https://www.kernel.org/doc/Documentation/vm/pagemap.txt
    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
            if (fread(&e, sizeof(uint64_t), 1, pagemap)) {
                if (e & (1ULL << 63)) { // page present ?
                    paddr = e & ((1ULL << 54) - 1); // pfn mask
                    paddr = paddr * sysconf(_SC_PAGESIZE);
                    // add offset within page
                    paddr = paddr | (vaddr & (sysconf(_SC_PAGESIZE) - 1));
                }   
            }   
        }   
        fclose(pagemap);
    }   

    return paddr;
}

int mem_alloc2(unsigned int** send_buff, unsigned int** recv_buff, size_t len)
{
    *send_buff = malloc(len);
    if(!send_buff) return 0;
    *recv_buff = malloc(len);
    if(!recv_buff) return 0;

    printf("malloc () : Phys addr %p -> %p and %p -> %p\n", *send_buff, vtop(*send_buff), *recv_buff, vtop(*recv_buff));
    return 1;
}

int mem_alloc3(unsigned int** send_buff, unsigned int** recv_buff, size_t len)
{
    
    *send_buff = cma_alloc(len, 0);
    if(!send_buff) return 0;
    *recv_buff = cma_alloc(len, 0);
    if(!recv_buff) return 0;
    printf("cma_alloc1 () : Phys addr %p -> %p and %p -> %p\n", *send_buff, cma_get_phy_addr(*send_buff), *recv_buff, cma_get_phy_addr(*recv_buff));
    return 1;
}
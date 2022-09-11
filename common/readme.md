# PYNQ UTILS

api for userspace manipulation of AXI DMA

```c
unsigned int* get_mmio( off_t mem_addr, size_t size);

void free_mmio(unsigned int* virtual_address, size_t size);

```
Wrapper around mmap using `\dev\mem` to map shared physical address

---

```c
void reset_DMA(unsigned int* dma_virtual_address);
void halt_DMA(unsigned int* dma_virtual_address);
void attach_send_buff(unsigned int* DMA_base_addr, off_t mem_addr);
void attach_recv_buff(unsigned int* DMA_base_addr, off_t mem_addr);
void set_transfer_lengths(unsigned int* DMA_base_addr, size_t len_send, size_t len_recv);
void start_transfer(unsigned int* DMA_base_addr);
void wait_transfer(unsigned int* DMA_base_addr); /* busy wait */
```
Functions to perform axi dma transfer from userspace 

---
## MEMORY (CMA ALLOCATION)

```c
typedef struct {
    unsigned int* virtual_address;
    unsigned int physical_address;
} cma_buffer_t;
```
structure representing a shared contiguous memory (cma) buffer for dma transfer.

---
```c
int allocate_cma_buffer(cma_buffer_t* buffer, size_t size);
void free_cma_buffer(cma_buffer_t* buffer);
```
used to allocate cma buffers and free





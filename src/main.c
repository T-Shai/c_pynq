#include "pynq_utils.h"
#include "image.h"
#include "process.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#define THRES 127

void test1() 
{
    sImg* img;
    printf("loading image...\n");
    load_image(&img, "data/women.bmp");

    for (size_t i = 0; i < 32; i++)
    {
        printf("%u : %u\n",i,  img->data[i]);
    }
    

    // printf("processing...\n");
    // threshold(img,THRES);

    printf("saving image...\n");
    save_image(img,  "data/output/threshold.bmp");
    free_image(img);
}

void test2()
{
    // loading bitstream
    load_bitstream("/home/xilinx/pynq/overlays/threshold/threshold.bit");
    
    // getting the status of DMA channels
    unsigned int* dma_mmio = get_mmio(DMA_BASE, REGISTER_SIZE);
    if(!dma_mmio) return;
    print_status(dma_mmio);
    free_mmio(dma_mmio, REGISTER_SIZE);

}


void memdump(void* virtual_address, int byte_count) {
    char *p = virtual_address;
    int offset;
    for (offset = 0; offset < byte_count; offset++) {
        printf("%02x", p[offset]);
        if (offset % 4 == 3) { printf(" "); }
    }
    printf("\n");
}

void test3()
{
    // loading bitstream
    load_bitstream("/home/xilinx/pynq/overlays/threshold/threshold.bit");

    // create image as c array
    sImg* img;
    load_image(&img, "data/car.pgm");
    

    const unsigned int data_size = img->width*img->height;
    //getting the status of DMA channels
    unsigned int* dma_mmio = get_mmio(DMA_BASE, REGISTER_SIZE);
    if(!dma_mmio) return;
    unsigned int* send_buff = get_mmio(DMA_SEND_ADDR, data_size*4);
    if(!send_buff) return;
    unsigned int* recv_buff = get_mmio(DMA_RECV_ADDR, data_size*4);
    if(!recv_buff) return;

    unsigned int* ip_mmio = get_mmio(IP_AXI_LITE_BASE, REGISTER_SIZE);
    if(!ip_mmio) return;

    ip_mmio[0] = 0x81;
    
    memset(send_buff, 0, data_size);
    memset(recv_buff, 0, data_size);
    
    for (size_t i = 0; i < data_size; i++)
    {
        send_buff[i] = img->data[i];
    }
    
    printf("image memory block:      \n"); memdump(img->data, 100);
    printf("Source memory block:      \n"); memdump(send_buff, 100);
    printf("Destination memory block: \n"); memdump(recv_buff, 100);

    printf("Resetting DMA\n");
    reset_DMA(dma_mmio);
    print_status(dma_mmio);

    printf("Halting DMA\n");
    halt_DMA(dma_mmio);
    print_status(dma_mmio);

    printf("attaching buffers...\n");
    attach_recv_buff(dma_mmio, DMA_RECV_ADDR);
    attach_send_buff(dma_mmio, DMA_SEND_ADDR);
    print_status(dma_mmio);
    
    printf("starting transfer...\n");
    start_transfer(dma_mmio);
    print_status(dma_mmio);

    printf("setting lengths...\n");
    set_transfer_lengths(dma_mmio, data_size, data_size);
    print_status(dma_mmio);

    printf("waiting transfer's end...\n");
    wait_transfer(dma_mmio);
    print_status(dma_mmio);

    printf("Source memory block:      \n"); memdump(send_buff, 100);
    printf("Destination memory block: \n"); memdump(recv_buff, 100);

    save_image(img,  "data/IP_threshold.pgm");

    free_image(img);
    free_mmio(recv_buff, data_size);
    free_mmio(send_buff, data_size);
    free_mmio(dma_mmio, REGISTER_SIZE);
    free_mmio(dma_mmio, REGISTER_SIZE);
}

int main(int argv, char** argc){
    
    // test1();
    // test2();
    test3();
    return 0;
}
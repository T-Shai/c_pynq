#include "pynq_utils.h"
#include "image.h"
#include "process.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#define THRES       127
#define DATA_SIZE   1000


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
            printf("%u, %02x != %02x \n", i,  buff1[i], buff2[i]);
            return 0;
        }
    }
    printf("buff1 = buff2\n");
    
    return 1;
}
void test3()
{
    srand(time(NULL));
    // loading bitstream
    load_bitstream("/home/xilinx/pynq/overlays/threshold/threshold.bit");

    //getting the status of DMA channels
    unsigned int* dma_mmio = get_mmio(DMA_BASE, REGISTER_SIZE);
    if(!dma_mmio) return;
    unsigned int* send_buff = get_mmio(DMA_SEND_ADDR, DATA_SIZE*4);
    if(!send_buff) return;
    unsigned int* recv_buff = get_mmio(DMA_RECV_ADDR, DATA_SIZE*4);
    if(!recv_buff) return;

    unsigned int* ip_mmio = get_mmio(IP_AXI_LITE_BASE, REGISTER_SIZE);
    if(!ip_mmio) return;


    /*
        starting threshold IP trough axi lite :
            0x0  : offset of the control register
            0x81 enables bit corresponding to ap_start and auto_restart
    */
    ip_mmio[0x0] = 0x81;
    
    memset(recv_buff, 0, DATA_SIZE);

    fill_random(send_buff, DATA_SIZE);

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
    set_transfer_lengths(dma_mmio, DATA_SIZE, DATA_SIZE);
    print_status(dma_mmio);

    printf("waiting transfer's end...\n");
    
    clock_t hw_start, hw_end;
    double hw_time;
    hw_start = clock();
    wait_transfer(dma_mmio);
    hw_end = clock();
    hw_time = (hw_end - hw_start) / (CLOCKS_PER_SEC / (double) 1000.0);

    print_status(dma_mmio);

    printf("Source memory block:      \n"); memdump(send_buff, 100);
    printf("Destination memory block: \n"); memdump(recv_buff, 100);
    
    
    clock_t sw_start, sw_end;
    double sw_time;
    sw_start = clock();
    
    for (size_t i = 0; i < DATA_SIZE; i++)
    {
        send_buff[i] = (send_buff[i] <= THRES) ? 0 : 255;
    }
    
    sw_end = clock();
    sw_time = (sw_end - sw_start) / (CLOCKS_PER_SEC / (double) 1000.0);
    
    printf("Source memory block:      \n"); memdump(send_buff, 100);
    printf("Destination memory block: \n"); memdump(recv_buff, 100);

    int check = compare_arrays(send_buff, recv_buff, DATA_SIZE);
    
    if(check)
    {
        printf("Software and hardware are equals !\n");
    } else {
        printf("Software and hardware are NOT equals !\n");
    }

    printf("Software took %f secs and hardware took %f secs\n", sw_time, hw_time);
    printf("Hardware is %f times faster than Software\n", sw_time/hw_time);

    free_mmio(recv_buff, DATA_SIZE);
    free_mmio(send_buff, DATA_SIZE);
    free_mmio(dma_mmio, REGISTER_SIZE);
    free_mmio(dma_mmio, REGISTER_SIZE);
}

int main(int argv, char** argc){
    
    // test1();
    // test2();
    test3();
    return 0;
}
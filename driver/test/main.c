#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"
#include "sys/ioctl.h"

#include "pynq_utils.h"

/* ioctl api */
#include "../axi_dma_ioctl.h"

#define DATA_SIZE 10

int main(int argc, char const *argv[])
{
    int fd = open("/dev/axi_dma_dev", O_RDWR);
    
    if(fd < 0) {
        perror("Error opening axi_dma_dev");
        return 1;
    }

    // load_bitstream("/home/xilinx/pynq/overlays/threshold/threshold.bit");
    const char* bit_filename = "/home/xilinx/pynq/overlays/copy/copy.bit";
    const char* hwh_filename = "/home/xilinx/pynq/overlays/copy/copy.hwh";

    bitstream_header_t* header;
    c_load_bitstream(bit_filename, 0, &header);
    printf("Loading :\n");
    print_header(header);
    free_header(header);

    dma_regs_info_t data;
    /* set base address */
    unsigned long base_addr = get_dma_base(hwh_filename);

    cma_buffer_t cma_send_buffer;
    cma_buffer_t cma_recv_buffer;

    if(!allocate_cma_buffer(&cma_send_buffer, DATA_SIZE*sizeof(unsigned int))) return 1;
    if(!allocate_cma_buffer(&cma_recv_buffer, DATA_SIZE*sizeof(unsigned int))) return 1;

    memset(cma_recv_buffer.virtual_address, 0, DATA_SIZE*sizeof(unsigned int));
    for(int i=0; i < DATA_SIZE; i++)
    {
        cma_send_buffer.virtual_address[i] = i*2;
    }

    /* set send channel size */
    unsigned long send_chan_size = DATA_SIZE;
    unsigned long send_chan_addr = cma_send_buffer.physical_address;
    
    /* set receive channel size */
    unsigned long rcv_chan_size = DATA_SIZE;
    unsigned long rcv_chan_addr = cma_recv_buffer.physical_address;

    data.base_addr  = base_addr; 
    data.send_chan_size = send_chan_size; 
    data.send_chan_addr = send_chan_addr; 
    data.rcv_chan_size = rcv_chan_size; 
    data.rcv_chan_addr = rcv_chan_addr; 



    if(ioctl(fd, AXIDMA_IOC_SET_DATA, &data) < 0) {
        perror("Error setting base address");
        return 1;
    }

    char read_buf[1024*4];
    read(fd, read_buf, 1024*4);
    printf("%s\n", read_buf);

    for(int i=0; i < DATA_SIZE; i++)
    {
       printf("[%d] : %d\n" ,i,cma_recv_buffer.virtual_address[i]);
    }
    
    close(fd);
    free_cma_buffer(&cma_send_buffer);
    free_cma_buffer(&cma_recv_buffer);
    return 0;
}

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

#define DATA_SIZE 200

int main(int argc, char const *argv[])
{
    const char* bit_filename = "/home/xilinx/pynq/overlays/threshold/threshold.bit";
    const char* hwh_filename = "/home/xilinx/pynq/overlays/threshold/threshold.hwh";

    if(access(bit_filename, F_OK) != 0)
    {
        printf("\n[ERROR] missing bitstream @ \"%s\" can't test driver.\n\n", bit_filename);
        return 1;
    }

    if(access(hwh_filename, F_OK) != 0)
    {
        printf("\n[ERROR] missing .hwh @ \"%s\" can't test driver.\n\n", hwh_filename);
        return 1;
    }

    bitstream_header_t* header;
    c_load_bitstream(bit_filename, 0, &header);
    printf("Loading :\n");
    print_header(header);
    free_header(header);

    int fd = open("/dev/axi_dma_dev", O_RDWR);
    
    if(fd < 0) {
        perror("Error opening axi_dma_dev");
        return 1;
    }

    /* get base address */
    unsigned long base_addr = get_dma_base(hwh_filename);
    printf("base addr : 0x%lx \n", base_addr);

    /* allocate cma buffer */
    cma_buffer_t cma_send_buffer;
    cma_buffer_t cma_recv_buffer;

    if(!allocate_cma_buffer(&cma_send_buffer, DATA_SIZE*sizeof(unsigned int))) return 1;
    if(!allocate_cma_buffer(&cma_recv_buffer, DATA_SIZE*sizeof(unsigned int))) return 1;

    /* initializing buffers */
    memset(cma_recv_buffer.virtual_address, -1, DATA_SIZE*sizeof(unsigned int));
    
    unsigned int *sbuff = cma_send_buffer.virtual_address;
    unsigned int *rbuff = cma_recv_buffer.virtual_address;

    for(unsigned int i=0; i < DATA_SIZE; i++)
    {
        sbuff[i] = i;
    }
    
    unsigned long ctrl_base_addr = get_axi_ctrl_base(hwh_filename);
    printf("axi base addr : 0x%lx \n", ctrl_base_addr);

    /* starting threshold IP trough axi lite */
    mmio_info_t axi_lite = {
        .base_addr = ctrl_base_addr,
        .offset = 0,                    /* 0x0  : offset of the control register */
        .value = 0x81                   /* 0x81 : set to 1 bits corresponding to ap_start and auto_restart */
    };

    if(ioctl(fd, AXIDMA_IOC_MMIO_WR, &axi_lite) < 0) {
        perror("Error setting base address");
        return 1;
    }

    /* send axi dma info from user */
    dma_info_t data = {
        .base_addr  = base_addr,
        .ssize = DATA_SIZE*sizeof(unsigned int),
        .saddr = cma_send_buffer.physical_address, 
        .rsize = DATA_SIZE*sizeof(unsigned int),
        .raddr = cma_recv_buffer.physical_address 
    };

    if(ioctl(fd, AXIDMA_IOC_INFO, &data) < 0) {
        perror("Error setting base address");
        return 1;
    }
    
    /* start transfer */
    if(ioctl(fd, AXIDMA_IOC_START, NULL) < 0) {
        perror("Error resource not map");
        return 1;
    }
    
    int res = 1;
    for(int i=0; i < DATA_SIZE; i++)
    {
        if(rbuff[i] != ((sbuff[i] <= 127) ? 0 : 255)){
            res = 0;
            break;
        }
    }

    printf("#------------------------------------#\n");
    if(res) {
        printf("[OK] test passed.\n");
    } else {
        printf("[FAILED] test failed arrays are not equal.\n");
    }
    printf("#------------------------------------#\n");

    
    close(fd);
    free_cma_buffer(&cma_send_buffer);
    free_cma_buffer(&cma_recv_buffer);
    return 0;
}

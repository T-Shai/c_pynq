#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"
#include "sys/ioctl.h"

/* ioctl api */
#define AXIDMA_IOC_MAGIC 'a'
#define AXIDMA_IOC_SET_BASE_ADDR _IO(AXIDMA_IOC_MAGIC, 0)
#define AXIDMA_IOC_SET_SEND_CHAN_SIZE _IO(AXIDMA_IOC_MAGIC, 1)
#define AXIDMA_IOC_SET_RECV_CHAN_SIZE _IO(AXIDMA_IOC_MAGIC, 2)


int main(int argc, char const *argv[])
{
    int fd = open("/dev/axi_dma_dev", O_RDWR);
    
    if(fd < 0) {
        perror("Error opening axi_dma_dev");
        return 1;
    }

    /* set base address */
    unsigned long base_addr = 0x41E00000;
    if(ioctl(fd, AXIDMA_IOC_SET_BASE_ADDR, &base_addr) < 0) {
        perror("Error setting base address");
        return 1;
    }

    /* set send channel size */
    unsigned long send_chan_size = 0x1000;
    if(ioctl(fd, AXIDMA_IOC_SET_SEND_CHAN_SIZE, &send_chan_size) < 0) {
        perror("Error setting send channel size");
        return 1;
    }

    /* set receive channel size */
    unsigned long recv_chan_size = 0x1000;
    if(ioctl(fd, AXIDMA_IOC_SET_RECV_CHAN_SIZE, &recv_chan_size) < 0) {
        perror("Error setting receive channel size");
        return 1;
    }


    char read_buf[1024];
    read(fd, read_buf, 1024);
    printf("%s\n", read_buf);

    close(fd);

    return 0;
}

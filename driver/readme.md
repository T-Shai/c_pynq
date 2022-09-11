# DRIVER

character device driver that performs an axi dma transfer

---

driver will be automaticaly loaded using (commands must be performed in sudo mode) :

```sh
cd <this directory>
make
```
After loading the driver a test will be performed. It will fail if you donn't have : 
`
/home/xilinx/pynq/overlays/threshold/threshold.bit
/home/xilinx/pynq/overlays/threshold/threshold.hwh
`

Bitstreams and .hwh files are available in the overlays folder of this repository

To unload the device driver use :
```sh
.\unload_dev.sh
```
( don't forget to give execution right to the shell script )

---

```c
typedef struct
{
    adi_u64 base_addr;  /* dma base address */

    adi_u64 saddr;      /* send buffer physical address */
    adi_u64 ssize;      /* send buffer size */

    adi_u64 raddr;      /* rcv buffer physical address */
    adi_u64 rsize;      /* rcv buffer size */
} dma_info_t;
```
structure containing information for the dma transfer.

```c
int fd = open("/dev/axi_dma_dev", O_RDWR);
```
Get file descriptor by opening `/dev/axi_dma_dev`

---

To send information from user to driver

```c
    /* send axi dma info from user */
    dma_info_t data = {
        .base_addr  = ...   ,
        .ssize = ...    ,
        .saddr = ...    , 
        .rsize = ...    ,
        .raddr = ... 
    };

    if(ioctl(fd, AXIDMA_IOC_INFO, &data) < 0) {
        perror("Error setting base address");
        return 1;
    }
```

---
To start a transfer 

```c
    /* start transfer */
    if(ioctl(fd, AXIDMA_IOC_START, NULL) < 0) {
        perror("Error resource not map");
        return 1;
    }
```
---
To write to physical address (useful for axi lite control)

```c
    /* starting threshold IP trough axi lite */
    mmio_info_t axi_lite = {
        .base_addr = ctrl_base_addr,
        .offset = 0, 
        .value = 0x81
    };

    if(ioctl(fd, AXIDMA_IOC_MMIO_WR, &axi_lite) < 0) {
        perror("Error setting base address");
        return 1;
    }
```
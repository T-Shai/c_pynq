#include "pynq_utils.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "libxlnk_cma.h"

#include <arpa/inet.h>
#include "errno.h"

int load_bitstream(const char* bitstream_name)
{
    char cmd_buff[256];
    strcpy(cmd_buff, "python3 ../common/bitstream_loader.py ");
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

int allocate_cma_buffer(cma_buffer_t* buffer, size_t size)
{
    buffer->virtual_address = (unsigned int*)cma_alloc(size, 0);
    if(!buffer->virtual_address){
        printf("Error occured while allocating CMA buffer of size %d.\n", size);
        return 0;
    }
    buffer->physical_address = cma_get_phy_addr(buffer->virtual_address);
    return 1;
}

void free_cma_buffer(cma_buffer_t* buffer)
{
    cma_free(buffer->virtual_address);
}

unsigned int get_available_cma_size()
{
    return (sysconf(_SC_PAGESIZE) * cma_pages_available());
}

void print_header(bitstream_header_t* header)
{
    printf("Design name: %s\n", header->design_name);
    printf("Part name: %s\n", header->part_name);
    printf("Date: %s\n", header->date);
    printf("Time: %s\n", header->time);
    printf("Bitstream length: %u\n", header->bitstream_length);
}

void parse_bitstream(FILE* fd, bitstream_header_t* header)
{
    char c;
    
    // design name
    while (1)
    {
        c = fgetc(fd);
        if (c == 'a')
        {
            break;
        }
    }
    uint16_t length;
    fread(&length, sizeof(length), 1, fd);
    length = ntohs(length);

    char* design_name = (char*)malloc(length);
    fread(design_name, sizeof(char), length, fd);
    header->design_name = design_name;

    // part name
    while (1)
    {
        c = fgetc(fd);
        if (c == 'b')
        {
            break;
        }
    }
    fread(&length, sizeof(length), 1, fd);
    length = ntohs(length);
    char* part_name = (char*)malloc(length);
    fread(part_name, sizeof(char), length, fd);
    header->part_name = part_name;

    // date
    while (1)
    {
        c = fgetc(fd);
        if (c == 'c')
        {
            break;
        }
    }
    fread(&length, sizeof(length), 1, fd);
    length = ntohs(length);
    char* date = (char*)malloc(length);
    fread(date, sizeof(char), length, fd);
    header->date = date;

    // time
    while (1)
    {
        c = fgetc(fd);
        if (c == 'd')
        {
            break;
        }
    }
    fread(&length, sizeof(length), 1, fd);
    length = ntohs(length);
    char* time = (char*)malloc(length);
    fread(time, sizeof(char), length, fd);
    header->time = time;

    // bitstream
    while (1)
    {
        c = fgetc(fd);
        if (c == 'e')
        {
            break;
        }
    }
    uint32_t bitstream_length;
    fread(&bitstream_length, sizeof(bitstream_length), 1, fd);
    bitstream_length = ntohl(bitstream_length);
    header->bitstream_length = bitstream_length;
}



void free_header(bitstream_header_t* header)
{
    if(!header) return;
    if(header->design_name) free(header->design_name);
    if(header->part_name) free(header->part_name);
    if(header->date) free(header->date);
    if(header->time) free(header->time);
    free(header);
}

int copy_bitstream(const char *bit_filename, const char* bin_filename, bitstream_header_t* header)
{ 
    FILE* bit_fd = fopen(bit_filename, "rb");
    if(!bit_fd)
    {
        printf("Error: could not open bitstream file %s\n", bit_filename);
        return -1;
    }
    FILE* bin_fd = fopen(bin_filename, "wb");
    if(!bin_fd)
    {
        printf("Error: could not open binary file %s\n", bin_filename);
        return -1;
    }

    // copy bitstream data
    char c;
    while (1)
    {
        c = fgetc(bit_fd);
        if (c == 'e')
        {
            break;
        }
    }
    
    printf("start copying bitstream ... \n");
    char* buffer[1024*5];
    uint32_t bytes_read = 0;
    while(bytes_read < header->bitstream_length)
    {
        uint32_t bytes_to_read = header->bitstream_length - bytes_read;
        if(bytes_to_read > sizeof(buffer))
        {
            bytes_to_read = sizeof(buffer);
        }
        uint32_t bytes_read_this_time = fread(buffer, sizeof(char), bytes_to_read, bit_fd);
        if(bytes_read_this_time != bytes_to_read)
        {
            printf("Error: could not read %u bytes from bitstream file %s\n", bytes_to_read, bit_filename);
            printf("error code: %d\n", ferror(bit_fd));
            printf("perror %s", strerror(errno));
            return -1;
        }
        bytes_read += bytes_read_this_time;
        fwrite(buffer, sizeof(char), bytes_read_this_time, bin_fd);
    }


    return 0;
}


/*

    https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841847/Solution+ZynqMP+PL+Programming

    Exercising FPGA programming using fpga framework attributes  
FPGA programming using sysfs attributes 
 Linux FPGA Manager driver creates /sys/class/fpga_manager/<fpga> folder, which contains file attributes that provides control and state. 

firmware:  /sys/class/fpga_manager/<fpga>/firmware
firmware attribute requests an fpga image using the firmware class, then write output to the FPGA
flags:  /sys/class/fpga_manager/<fpga>/flags
flags attribute determines the type of Bitstream 
0 - Full Bitstream 1 - Partial Bitstream (default: 0)
state:  /sys/class/fpga_manager/<fpga>/state
state attribute is a superset of FPGA states and FPGA Manager driver states
name:  /sys/class/fpga_manager/<fpga>/name
name attribute is name of the low level FPGA Manager driver 
key: /sys/class/fpga_manager/<fpga>/key 
key attribute stores the key value useful for Encrypted Bitstream loading to read the userkey
 
Steps for programming the Full Bitstream 
Set flags for Full Bitstream

echo 0 > /sys/class/fpga_manager/fpga0/flags
Load the Bitstream 

mkdir -p /lib/firmware

cp /media/design_1_wrapper.bit.bin /lib/firmware/

echo design_1_wrapper.bit.bin > /sys/class/fpga_manager/fpga0/firmware

Steps for programming the Partial Bitstream
Set flags for Partial Bitstream

echo 1 > /sys/class/fpga_manager/fpga0/flags
Load the Bitstream Partial Bitstream

mkdir -p /lib/firmware

cp /media/partail_wrapper.bit.bin /lib/firmware/

echo partail_wrapper.bit.bin  > /sys/class/fpga_manager/fpga0/firmware

FPGA Readback using debugfs attributes
Linux FPGA Manager driver creates /sys/kernel/debug/fpga/ folder, which contains image attributes that provides readback contents.

readback_type:  /sys/module/zynqmp_fpga/parameters/readback_type
readback_type is a module parameter which tells the readback type (default : 0) 
 0 – Configuration Register read 1 - Configuration Data read 
image:  /sys/kernel/debug/fpga/<fpga>/image
image is a debugfs attribute which provides the readback information based on the readback_type module param
Steps for Readback of Configuration Registers
Set flags for readback type

echo 0 > /sys/module/zynqmp_fpga/parameters/readback_type

Perform Readback operation by reading the image debug attribute 

cat /sys/kernel/debug/fpga/fpga0/image

teps for Readback of Configuration DataFrames 
Set flags for readback type

echo 1 > /sys/module/zynqmp_fpga/parameters/readback_type

Perform Readback operation by reading the image debug attribute 

cat /sys/kernel/debug/fpga/fpga0/image

*/

int c_load_bitstream(const char * bitsteam_name, int partial, bitstream_header_t** header)
{
    const char* fpga_flags = "/sys/class/fpga_manager/fpga0/flags";
    const char* fpga_firmware = "/sys/class/fpga_manager/fpga0/firmware";

    if(partial) {
        // not implemented yet
        printf("Partial bitstream not implemented yet\n");
        return -1;
    } else {
        // full bitstream
        FILE* fp = fopen(fpga_flags, "w");
        if(fp) {
            fprintf(fp, "0");
            fclose(fp);
        } else {
            printf("Failed to open %s\n", fpga_flags);
            return 0;
        }
    }

    const char* lib_firmware = "/lib/firmware";
    // extracting the header and the binary data from the bitstream file
    // bitstream_header_t* header = *pheader;
    *header = (bitstream_header_t*)malloc(sizeof(bitstream_header_t));
    FILE* fp = fopen(bitsteam_name, "r");
    if(!fp) {
        printf("Failed to open %s\n", bitsteam_name);
        free_header(*header);
        return 0;
    }
    parse_bitstream(fp, *header);
    // get bitfile name from full path
    char* bitfile_name = strrchr(bitsteam_name, '/');
    if(!bitfile_name) {
        bitfile_name = (char*) bitsteam_name;
    } else {
        bitfile_name++;
    }
    // remove the .bit extension if any and replace with .bin
    char* bin_name = (char*)malloc(strlen(bitfile_name) + 5);
    strcpy(bin_name, bitfile_name);
    char* dot = strrchr(bin_name, '.');
    if(dot) {
        *dot = '\0';
    }
    strcat(bin_name, ".bin");

    // search for the binfile in the firmware directory
    char* bin_path = (char*)malloc(strlen(lib_firmware) + strlen(bin_name) + 2);
    strcpy(bin_path, lib_firmware);
    strcat(bin_path, "/");
    strcat(bin_path, bin_name);
    if(access(bin_path, F_OK) != 0) {
        printf("Failed to find %s\n", bin_path);
        printf("need to create .bin and move it to %s\n", lib_firmware);
        printf("right now you can launch the python Overlay to do this the first time\n");
        // copy_bitstream(bitsteam_name, "test.bin", header);
        free_header(*header);
        free(bin_name);
        free(bin_path);
        return 0;
    }

    // write bin filename to the firmware attribute
    FILE* fp_firmware = fopen(fpga_firmware, "w");
    if(fp_firmware) {
        fprintf(fp_firmware, "%s", bin_name);
        fclose(fp_firmware);
    } else {
        printf("Failed to open %s\n", fpga_firmware);
        free(bin_name);
        free(bin_path);
        free_header(*header);
        return 0;
    }
    // frees 
    free(bin_path);
    free(bin_name);
    fclose(fp);
    return 1;
}

/*
    parsing a .hwh file which is a xml file
    this function will get the base address of the dma base address
*/
uint64_t get_dma_base(const char *hwh_filename)
{
    FILE* fp = fopen(hwh_filename, "r");
    if(!fp) {
        printf("Failed to open %s\n", hwh_filename);
        return 0;
    }

    const char* dma_name = "/axi_dma_0";
    const char* dma_base_addr = "<PARAMETER NAME=\"C_BASEADDR\" VALUE=\"";

    char line[1024*4];
    char in_dma = 0;
    while(fgets(line, sizeof(line), fp)) {
        if(strstr(line, dma_name)) {
            in_dma = 1;
        }
        if(in_dma && strstr(line, dma_base_addr)) {
            char* base_addr = strstr(line, dma_base_addr) + strlen(dma_base_addr);
            char* end_addr = strstr(base_addr, "\"");
            *end_addr = '\0';
            return strtoull(base_addr, NULL, 16);
        }
    }
    fclose(fp);
    return -1;
}


uint64_t get_axi_ctrl_base(const char *hwh_filename)
{
    FILE* fp = fopen(hwh_filename, "r");
    if(!fp) {
        printf("Failed to open %s\n", hwh_filename);
        return 0;
    }

    const char* ip_base_addr = "<PARAMETER NAME=\"C_S_AXI_CONTROL_BASEADDR\" VALUE=\"";

    char line[1024*4];
    while(fgets(line, sizeof(line), fp)) {
        if(strstr(line, ip_base_addr)) {
            char* base_addr = strstr(line, ip_base_addr) + strlen(ip_base_addr);
            char* end_addr = strstr(base_addr, "\"");
            *end_addr = '\0';
            return strtoull(base_addr, NULL, 16);
        }
    }
    fclose(fp);
    return -1;
}
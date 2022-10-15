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

char* get_file_content(const char* filename, size_t* pbufsize)
{
    char* source= NULL;
    FILE *fp = fopen(filename, "rb");
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            size_t bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }
            *pbufsize = bufsize; 
            /* Allocate our buffer to that size. */
            source = malloc(sizeof(char) * (bufsize));

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
            }
        }
        fclose(fp);
    }
    return source;
}

int copy_bitstream(const char *bit_filename, const char* bin_filename, bitstream_header_t* header)
{
    size_t bit_content_l;
    char *bit_content = get_file_content(bit_filename, &bit_content_l);
    
    size_t bin_content_l = header->bitstream_length;
    char *bin_content = malloc(sizeof(char) * bin_content_l);

    const size_t header_size = bit_content_l - bin_content_l;
    size_t offset = 0;

    while (offset < bin_content_l)
    {
        uint32_t data32;
        memcpy(&data32, &bit_content[header_size + offset], sizeof(uint32_t));
        data32 = ntohl(data32);
        memcpy(&bin_content[offset], &data32, sizeof(uint32_t));
        offset += 4;
    }

    FILE *bin_fp = fopen(bin_filename, "wb");
    fwrite(bin_content, sizeof(char), bin_content_l, bin_fp);
    fclose(bin_fp);

    free(bin_content);
    free(bit_content);
}

/*

    https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841847/Solution+ZynqMP+PL+Programming

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
    
    copy_bitstream(bitsteam_name, bin_name, header);

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
    if(in_dma) printf("found %s cell in %s but couldn't find base address\n", dma_name, hwh_filename);
    else printf("couldn't found %s cell in %s\n", dma_name, hwh_filename);
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
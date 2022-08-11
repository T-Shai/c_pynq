#include "loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*

bitsteam  format :

Field 1
2 bytes          length            (big endian)
9 bytes          some sort of header

Field 2
2 bytes          length 
1 byte           key 0x61                (The letter "a")

Field 3
2 bytes          length            (value depends on file name length)
10 bytes         string design name "xform.ncd" (including a trailing 0x00)

Field 4
1 byte           key                 (The letter "b")
2 bytes          length            (value depends on part name length)
12 bytes         string part name "v1000efg860" (including a trailing 0x00)

Field 4
1 byte           key                 (The letter "c")
2 bytes          length 
11 bytes         string date   (including a trailing 0x00)

Field 5
1 byte           key                 (The letter "d")
2 bytes          length 
9 bytes          string time     (including a trailing 0x00)

Field 6
1 byte           key                  (The letter "e")
4 bytes          length        (value depends on device type,
                                           and maybe design details)
rest of  bytes    raw bit stream starting with 0xffffffff aa995566 sync

*/

typedef struct {
    char* design_name;
    char* part_name;
    char* date;
    char* time;
    uint32_t bitstream_length;
} bitstream_header_t ;

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
int load_bitstream(const char * bitsteam_name, int partial)
{
    const char* fpga_flags = "/sys/class/fpga_manager/fpga0/flags";
    const char* fpga_firmware = "/sys/class/fpga_manager/fpga0/firmware";

    if(partial) {
        // not implemented yet
        printf("Partial bitstream not implemented yet\n");
        return -1;
    } else {
        // full bitstream
        printf("Full bitstream\n");
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
    bitstream_header_t* header = (bitstream_header_t*)malloc(sizeof(bitstream_header_t));
    FILE* fp = fopen(bitsteam_name, "r");
    if(!fp) {
        printf("Failed to open %s\n", bitsteam_name);
        return 0;
    }
    parse_bitstream(fp, header);
    print_header(header);
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
        return 0;
    }

    // write bin filename to the firmware attribute
    FILE* fp_firmware = fopen(fpga_firmware, "w");
    if(fp_firmware) {
        fprintf(fp_firmware, "%s", bin_name);
        fclose(fp_firmware);
    } else {
        printf("Failed to open %s\n", fpga_firmware);
        return 0;
    }
    free_header(header);
    fclose(fp);
    return 1;
}
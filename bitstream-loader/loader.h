#ifndef __LOADER_H__
#define __LOADER_H__

#include "stdint.h"
/*
    loader.h  - Bitstream loader
    FPGA programming using fpga framework attributes
*/


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

#include <stdio.h>


typedef struct {
    char* design_name;
    char* part_name;
    char* date;
    char* time;
    uint32_t bitstream_length;
} bitstream_header_t ;

void print_header(bitstream_header_t* header);

void free_header(bitstream_header_t* header);

void parse_bitstream(FILE* fd, bitstream_header_t* header);

int c_load_bitstream(const char * bitsteam_name, int partial, bitstream_header_t** header);

uint64_t get_dma_base(const char *hwh_filename);

#endif // __LOADER_H__
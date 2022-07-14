#include "pynq_utils.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int load_bitstream(const char* bitstream_name)
{
    char cmd_buff[256];
    strcpy(cmd_buff, "python3 bitstream_loader.py ");
    strcat(cmd_buff, bitstream_name);
    return system(cmd_buff);
}

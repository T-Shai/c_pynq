#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#include "pynq_utils.h"

int main(int argc, char const *argv[])
{
    const char* bit_filename = "/home/xilinx/pynq/overlays/threshold/threshold.bit";
    const char* hwh_filename = "/home/xilinx/pynq/overlays/threshold/threshold.hwh";

    bitstream_header_t* header;
    
    c_load_bitstream(bit_filename, 0, &header);

    printf("%s - info :\n", bit_filename);
    print_header(header);
    free_header(header);

    unsigned long base_addr = get_dma_base(hwh_filename);
    printf("base addr : 0x%lx \n", base_addr);

    unsigned long axi_lite_base_addr = get_axi_ctrl_base(hwh_filename);
    printf("axi lite base addr : 0x%lx \n", axi_lite_base_addr);
}

#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#include "pynq_utils.h"


int compare_file(const char *file_1, const char *file_2)
{
    FILE *fp1 = fopen(file_1,"r");

    if (fp1 == 0)
        return 0;

    FILE *fp2 = fopen(file_2,"r");

    if (fp2 == 0) {
        fclose(fp1);
        return 0;
    }

    char line1[1000];
    char line2[1000];
    char * r1, * r2;
    int result;

    for (;;) {
        r1 = fgets(line1, sizeof(line1), fp1);
        r2 = fgets(line2, sizeof(line2), fp2);

        if ((r1 == 0) || (r2 == 0)) {
        result = (r1 == r2);
        break;
        }

        if (strcmp(line1,line2) != 0) {
        result = 0;
        break;
        }
    }

    fclose(fp1);
    fclose(fp2);

    return result;
}


int main(int argc, char const *argv[])
{
    const char* bit_filename = "/home/xilinx/pynq/overlays/threshold/threshold.bit";
    const char* hwh_filename = "/home/xilinx/pynq/overlays/threshold/threshold.hwh";
    const char* firmbin_filename = "/lib/firmware/threshold.bin";
    const char* bin_filename = "threshold.bin";
    bitstream_header_t* header;
    
    c_load_bitstream(bit_filename, 0, &header);

    printf("%s - info :\n", bit_filename);
    print_header(header);
    copy_bitstream(bit_filename, bin_filename, header);
    if(compare_file(firmbin_filename, bin_filename)){
        printf("[OK] BIN FILE ARE EQUAL.\n");
    } else {
        printf("[FAIL] BIN FILE ARE NOT EQUAL !\n");
    }
    free_header(header);

    unsigned long base_addr = get_dma_base(hwh_filename);
    printf("base addr : 0x%lx \n", base_addr);

    unsigned long axi_lite_base_addr = get_axi_ctrl_base(hwh_filename);
    printf("axi lite base addr : 0x%lx \n", axi_lite_base_addr);
}

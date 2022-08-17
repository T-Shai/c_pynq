#include <stdio.h>
#include <stdlib.h>
#include "loader.h"
#include "string.h"

int main(int argc, char const *argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s <bitstream_file>\n", argv[0]);
        return -1;
    }

    bitstream_header_t* header = NULL;
    
    c_load_bitstream(argv[1], 0, &header);

    // replace bitfile name extension with .hwh
    char *hwh_filename = malloc(strlen(argv[1]) + 3);
    strcpy(hwh_filename, argv[1]);
    hwh_filename[strlen(hwh_filename) - 3] = '\0';
    strcat(hwh_filename, "hwh");

    uint64_t dma_base = get_dma_base(hwh_filename);
    print_header(header);
    printf("DMA base: %llx\n", dma_base);

    free_header(header);
    free(hwh_filename);
}

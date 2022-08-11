#include <stdio.h>
#include <stdlib.h>
#include "loader.h"

int main(int argc, char const *argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s <bitstream_file>\n", argv[0]);
        return -1;
    }
    return load_bitstream(argv[1], 0);
}

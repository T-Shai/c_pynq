#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"

int main(int argc, char const *argv[])
{
    int fd = open("/dev/axi_dma_dev", O_RDWR);
    
    if(fd < 0) {
        perror("Error opening axi_dma_dev");
        return 1;
    }

    char tmp[1024];
    read(fd, tmp, 1024);
    // write(fd, tmp, strlen(tmp));
    printf("%s\n", tmp);
    
    close(fd);

    return 0;
}

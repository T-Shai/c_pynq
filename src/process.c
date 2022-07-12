#include "process.h"
#include "stdlib.h"


int threshold(sImg* img_source, unsigned char thres){
    for (size_t i = 0; i < (img_source->width * img_source->height) ; i++)
        img_source->data[i] = (img_source->data[i] < thres) ? 0 : 255;
    return 0;
}
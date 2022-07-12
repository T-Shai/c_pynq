#pragma once
#include "image.h"


/*
    process an image structure with a threshold value in order to turn an image into black and white 

    returns : error code
*/

int threshold(sImg* img_source, unsigned char thres);
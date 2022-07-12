#include "image.h"
#include "process.h"

#include "stdio.h"
#define THRES 127

int main(int argv, char** argc){
    
    sImg* img;
    load_image(&img, "data/car_3000.pgm");
    threshold(img,THRES);
    save_image(img,  "data/threshold.pgm");
    free_image(img);

    return 0;
}
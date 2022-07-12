#include "image.h"
#include "stdio.h"

int main(int argv, char** argc){
    
    sImg* img;
    load_image(&img, "data/car_3000.pgm");
    save_image(img, "data/copy.pgm");
    free_image(img);

    return 0;
}
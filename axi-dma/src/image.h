#pragma once

typedef struct
{
    int width;              // image width
    int height;             // image height
    unsigned char data[0];  // image data
} sImg;

/*
    allocate a sImg struct and load a pgm P5 image with the name filename

    returns : error code
*/
int load_image(sImg** img, const char* filename);

/*
    save the sImg struct as filename

    returns : error code
*/
int save_image(sImg* img, const char* filename);


/*
    free the sImg struct
*/
void free_image(sImg* img);
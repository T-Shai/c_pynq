#include "image.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int load_image(sImg** pimg, const char* filename){
    FILE * fp;
    fp = fopen (filename, "r");
    char line[256];
    int err;
    int width = -1, height = -1;
    while (fgets(line, sizeof(line), fp)) {
        if(line[0] == '#')  continue; // skipping comments
        if(line[0] == 'P')  {
            if(strncmp(line, "P5", 2) != 0) {
                // version not supported
                err = 1;
                break;
            } 
            continue;
        }
        if(!strncmp(line, "255\n", 4)) {
            // end of header
            err = 0;
            break;
        }
        
        // extracting idth and height
        char * token = strtok(line, " ");
        width = atoi(token);
        token = strtok(NULL, " ");
        height = atoi(token);
    }
    sImg* img;
    img = malloc(sizeof(sImg)+(sizeof(char)*width*height));
    img->height = height;
    img->width = width;

    char img_data[320 * 240]; // TODO MALLOC DYNAMIC
    fgets(img_data, sizeof(img_data), fp);
    strcpy(img->data, img_data);
    *pimg = img;
    return 0;
}

int save_image(sImg* img, const char* filename)
{
    FILE *fptr = fopen(filename, "w");
    fprintf(fptr, "P5\n%d %d\n255\n", img->width, img->height);
    fwrite(img->data, sizeof(unsigned char), img->width*img->height, fptr);
    fclose(fptr);
    return 0;
}

void free_image(sImg* img) 
{
    free(img);
}
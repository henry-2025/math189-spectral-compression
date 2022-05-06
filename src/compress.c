/*
Henry Pick 2022
Harvey Mudd College
hpick@hmc.edu

DFT image compression using my own implementation of the discrete cosine transform

This is intended to demonstrate the feasibility of using a ground-up approach 
to develop robust signal processing applications with only a signal model and
a basic engineering objective. I intentionally did not use the fftw3 library 
for this reason, but in most contexts you would probably want to use a
standard fft library for readability.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fftw3.h>

#define COMPRESSION_RATE 0.5

struct pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

typedef struct pixel * image;

// read a ppm file and return an array of pixel structs
int load_ppm(char *file_name, image *im, int *rows, int *cols)
{
    char type[3];
    FILE *file = fopen(file_name, "rb");
    int numRow, numCol, maxNum;

    struct pixel currentPix;

    char buf[5];

    if (file == NULL) {
        printf(stderr, "Could not open file %s for reading");
        return -1;
    }

    fscanf(file, "%2s", type);
    type[2] = '\0';
    if(strcmp(type, "P6") != 0) {
        printf(stderr, "This file is not of type P6");
        return -1;
    }

    fscanf(file, "%d", &numCol);
    fscanf(file, "%d", &numRow);
    fscanf(file, "%d", &maxNum);

    *rows = numRow;
    *cols = numCol;

    fseek(file, 1, SEEK_CUR);

    //fread(&currentPix, sizeof(struct pixel), 1, file);

    *im = (image)malloc(numRow * numCol * sizeof(struct pixel));
    if(im == NULL) {
        printf("memory not allocated!");
    }

    for (int i = 0; i < numCol * numRow; i++) {
        fread(*im + i, sizeof(struct pixel), 1, file);
    }

    fclose(file);
    
    return 0;
}

#define BLOCK_SIZE 8

// run dct on a given block and return the compression subset
int dct_compress() {
    // create float buffer and run dft2d on the blocks
    fftw_plan plan = fftw_plan_r2r_2d(8, 8, in, out, FFTW_REDFT10, FFTW_ESTIMATE);
    fftw_execute(plan);

    return 0;
}

fast_dct8_2d(float *in, float *out) {
    // don't know if this is currently feasible, would be nice to actually implement though this being a signal processing focus
}



int main(int argc, char* argv[])
{
    if (argc != 2 && argc != 3) {
        printf("Usage: compress -i INPUT [-o OUTPUT] -c COMPRESSION\nInput ppm binary encoding and output in the \"hpic\" compression format :)");
        return 0;
    }

    image im;
    int rows, cols;
    load_ppm(argv[1], &im, &rows, &cols);
    
    
    free(im);
    return 0;
}
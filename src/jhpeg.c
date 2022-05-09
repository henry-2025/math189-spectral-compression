
#include "jhpeg.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <fftw3.h>

#define BLOCK_SIZE 8


// read a ppm file and return an array of pixel structs
int load_ppm_rgb(char *file_name, image_rgb *im, int *rows, int *cols)
{
    char type[3];
    FILE *file = fopen(file_name, "rb");
    int numRow, numCol, maxNum;

    char buf[5];

    if (file == NULL) {
        fprintf(stderr, "Could not open file %s for reading", file_name);
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

    *im = (image_rgb)malloc(numRow * numCol * sizeof(struct pixel_rgb));
    if(im == NULL) {
        printf("memory not allocated!");
    }

    for (int i = 0; i < numCol * numRow; i++) {
        fread(*im + i, sizeof(struct pixel_rgb), 1, file);
    }

    fclose(file);
    
    return 0;
}

int write_ppm_rgb(char *file_name, image_rgb *im, int rows, int cols) {
    FILE *file = fopen(file_name, "wb");

    if (file == NULL) {
        printf(stderr, "Could not open file %s for writing", file_name);
        return -1;
    }

    //write file header
    fprintf(file, "%2s\n%d %d\n%d\n", "P6", rows, cols, 255);

    fwrite(im, sizeof(struct pixel_rgb), rows * cols, file);
    fclose(file);
    return 0;
}


// run dct on a given block and return the compression subset
int dct_compress() {
    // create float buffer and run dft2d on the blocks

    return 0;
}

/*
* for array size N, stride size s, and offset o, compute out-of-place 
* cooley tukey in -> out
*/
int cooley_tukey_1d(int N, int s, int o, float *in, complex *out) {
    if (N == 1) {
        out[o] = in[o]; 
    } else {
        cooley_tukey_1d(N/2, 2*s, o, in, out);

        cooley_tukey_1d(N/2, 2*s, o + s, in, out);

        for (int k = 0; k < N/2; k++) {
            complex t = cexp(-I * M_PI*k/N) * out[o + 2*s*k];
            out[o + 2*s*k] = out[o + s + 2*s*k] + t;
            
            out[N/2 + o + 2*s*k] = out[o + s + 2*s*k] - t;
        }
    }
}

int fast_dct8_2d(float *in, float *out) {

    // TODO: how to handle image edges

    for(int i = 0; i < 8; i++) {
        fftwf_plan plan = fftwf_plan_r2r_1d(8, in + i*8, out + i*8, FFTW_REDFT10, FFTW_ESTIMATE);
        fftwf_execute(plan);
    }
    float slice_in[8];
    float slice_out[8];

    for(int i = 0; i < 8; i++) {
        fftwf_plan plan = fftwf_plan_r2r_1d(8, slice_in, slice_out, FFTW_REDFT10, FFTW_ESTIMATE);
        for(int j = 0; j < 8; j++) {
            slice_in[j] = out[i*8 + j];
        }
        fftwf_execute(plan);
        for(int j = 0; j < 8; j++) {
            out[i*8 + j] = slice_out[j];
        }
    }
}

int fast_idct8_2d(float *in, float *out){ 

}

void usage() {
        printf("Usage:\t hpic -c INPUT OUTPUT COMPRESSION\t Input ppm binary encoding and output in the hpic compression format\n\t\t hpic -d INPUT OUTPUT Input hpic compression file, output decompressed ppm binary");
}

void delete_image(image_rgb *im) {
    free(*im);
}

// int main(int argc, char* argv[])
// {
//     if (argc != 2 && argc != 3) {
//         usage();
//         return 0;
//     }
//     else if (strcmp(argv[1], "-c") == 0) {
//         char *input_path = argv[2];
//         char *output_path = argv[3];
//         float compression = atof(argv[4]);
//         return compress_ppm(input_path, output_path, compression);
//     } else if (strcmp(argv[1], "-d") == 0) {
//         char *input_path = argv[2];
//         char *output_path = argv[3];

//         return decompress_hpic(input_path, output_path);
//     } else {
//         usage();
//     }


    
//     return 0;
// }

// test main
int main(int argc, char* argv[]) {
    //testing image reading, seems to work
    char *input_path = "../figures/b.ppm";

    image_rgb im;
    int rows, columns;
    if(load_ppm_rgb(input_path, &im, &rows, &columns) != 0) {
        printf("could not load ppm");
        return -1;
    }

    for (int i = 0; i < 50; i++) {
        printf("%d, %d, %d\n", im[i].red, im[i].green, im[i].blue);
    }
    delete_image(&im);
    return 0;

   // write_ppm_rgb("test.ppm", NULL, 10, 20);
}
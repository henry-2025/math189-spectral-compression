#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <fftw3.h>

#include "jhpeg.h"

#define BLOCK_SIZE 8


void create_compressed_im(struct compressed_im *compressed, int rows, int cols, uint8_t compression) {
    int comp_size = 8 - compression;
    int n = (ceil(rows / 8.0) * ceil(cols / 8.0))*comp_size*comp_size;

    compressed->red = (int16_t*)malloc(n * sizeof(int16_t));
    compressed->green = (int16_t*)malloc(n * sizeof(int16_t));
    compressed->blue = (int16_t*)malloc(n * sizeof(int16_t));
}

void delete_compressed_im(struct compressed_im *compressed) {
    free(compressed->red);
    free(compressed->green);
    free(compressed->blue);
}

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

    fread(*im, sizeof(struct pixel_rgb), numRow*numCol, file);

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

    fwrite(*im, sizeof(struct pixel_rgb), rows * cols, file);
    fclose(file);
    return 0;
}

int load_jhpeg_rgb(char *file_name, struct compressed_im *im, int *rows, int *cols, uint8_t *compression) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s for reading", file_name);
        return -1;
    }

    // don't have any filetype safety, seems overkill
    fscanf(file, "%d", rows);
    fscanf(file, "%d", cols);
    int tmp;
    fscanf(file, "%d", &tmp);
    *compression = tmp;

    create_compressed_im(im, *rows, *cols, *compression);

    int comp_r = ceil(*rows / 8.0);
    int comp_c = ceil(*cols / 8.0);
    int cell_size = 8 - *compression;
    size_t len = cell_size*cell_size*comp_r*comp_c;

    fseek(file, 1, SEEK_CUR);

    fread(im->red, sizeof(int16_t), len, file);
    fread(im->green, sizeof(int16_t), len, file);
    fread(im->blue, sizeof(int16_t), len, file);

    fclose(file);
    return 0;
}



void fast_dct8_2d(uint8_t *in, int16_t *out) {

    // TODO: how to handle image edges
    float float_mat[64];
    for(int i = 0; i < 64; i++) {
        float_mat[i] = in[i];
    }

    for(int i = 0; i < 8; i++) {
        fftwf_plan plan = fftwf_plan_r2r_1d(8, float_mat + i*8, float_mat + i*8, FFTW_REDFT10, FFTW_ESTIMATE);
        fftwf_execute(plan);
    }
    float slice[8];

    for(int i = 0; i < 8; i++) {
        fftwf_plan plan = fftwf_plan_r2r_1d(8, slice, slice, FFTW_REDFT10, FFTW_ESTIMATE);
        for(int j = 0; j < 8; j++) {
            slice[j] = float_mat[j*8 + i];
        }
        fftwf_execute(plan);
        for(int j = 0; j < 8; j++) {
            out[j*8 + i] = (int16_t)(slice[j]/256);
        }
    }
}

int fast_idct8_2d(int16_t *in, uint8_t *out){
    float slice[8];
    float float_mat[64];

    for(int i = 0; i < 8; i++) {
        fftwf_plan plan = fftwf_plan_r2r_1d(8, slice, slice, FFTW_REDFT01, FFTW_ESTIMATE);
        for(int j = 0; j < 8; j++) {
            slice[j] = in[j*8 + i];
        }
        fftwf_execute(plan);
        for(int j = 0; j < 8; j++) {
            float_mat[j*8 + i] = slice[j];
        }
    }

    for(int i = 0; i < 8; i++) {
        fftwf_plan plan = fftwf_plan_r2r_1d(8, float_mat + i*8, float_mat + i*8, FFTW_REDFT01, FFTW_ESTIMATE);
        fftwf_execute(plan);
    }

    for (int i = 0; i < 64; i++) {
        out[i] = (uint8_t)float_mat[i]; //apply normalization
    }
}


// run dct on a given block and return the length of the compression array
void dct_compress(image_rgb *im, struct compressed_im *compressed, int rows, int cols, uint8_t compression) {
    // create float buffer and run dft2d on the blocks

    create_compressed_im(compressed, rows, cols, compression);

    uint8_t red_in[64];
    int16_t red_out[64];
    uint8_t green_in[64];
    int16_t green_out[64];
    uint8_t blue_in[64];
    int16_t blue_out[64];

    int comp_r = ceil(rows / 8.0);
    int comp_c = ceil(cols / 8.0);
    int cell_size = 8 - compression;

    for(int r = 0; r < comp_r; r++) {
        for(int c = 0; c < comp_c; c++) {
            for(int i = 0; i < 8; i++) {
                for(int j = 0; j < 8; j++) {
                    // pad with 255
                    if (r*8 + i >= rows || c*8 + j >= cols) {
                        red_in[j + i*8] = 255;
                        green_in[j + i*8] = 255;
                        blue_in[j + i*8] = 255;

                    } else {
                        red_in[j + i*8] = (*im)[c + j + (r+i)*cols].red;
                        green_in[j + i*8] = (*im)[c + j + (r+i)*cols].green;
                        blue_in[j + i*8] = (*im)[c + j + (r+i)*cols].blue;
                    }
                }
            }
            fast_dct8_2d(red_in, red_out);
            fast_dct8_2d(blue_in, blue_out);
            fast_dct8_2d(green_in, green_out);

            for(int i = 0; i < cell_size; i++) {
                for(int j = 0; j < cell_size; j++) {

                    int compression_index =  (r*comp_c*cell_size*cell_size + i*comp_c*cell_size) + (c*cell_size + j);

                    compressed->red[compression_index] = red_out[i*8 + j];
                    compressed->green[compression_index] = green_out[i*8 + j];
                    compressed->blue[compression_index] = blue_out[i*8 + j];
                }
            }
        }
    }
}

void dct_decompress(struct compressed_im *compressed, image_rgb *im, int rows, int cols, uint8_t compression) {
    *im = (image_rgb)malloc(rows * cols * sizeof(struct pixel_rgb));
    int16_t red_in[64];
    uint8_t red_out[64];
    int16_t green_in[64];
    uint8_t green_out[64];
    int16_t blue_in[64];
    uint8_t blue_out[64];

    int comp_r = ceil(rows / 8.0);
    int comp_c = ceil(cols / 8.0);
    int cell_size = 8 - compression;

    for(int r = 0; r < comp_r; r++) {
        for(int c = 0; c < comp_c; c++) {
            for(int i = 0; i < cell_size; i++) {
                for (int j = 0; j < cell_size; j++) {
                    int compression_index =  (r*comp_c*cell_size*cell_size + i*comp_c*cell_size) + (c*cell_size + j);

                    red_in[i*8 + j] = compressed->red[compression_index];
                    green_in[i*8 + j] = compressed->green[compression_index];
                    blue_in[i*8 + j] = compressed->blue[compression_index];
                }
            }

            fast_idct8_2d(red_in, red_out);
            fast_idct8_2d(blue_in, blue_out);
            fast_idct8_2d(green_in, green_out);

            for(int i = 0; i < 8; i++) {
                for(int j = 0; j < 8; j++) {
                    // pad with 255
                    if (r*8 + i >= rows || c*8 + j >= cols) {
                        continue;

                    } else {
                        (*im)[c + j + (r+i)*cols].red = red_out[j + i*8];
                        (*im)[c + j + (r+i)*cols].green = green_out[j + i*8];
                        (*im)[c + j + (r+i)*cols].blue = blue_out[j + i*8];
                    }
                }
            }
        }
    }

}


void usage() {
        printf("Usage:\t hpic -c INPUT OUTPUT COMPRESSION\t Input ppm binary encoding and output in the hpic compression format\n\t\t hpic -d INPUT OUTPUT Input hpic compression file, output decompressed ppm binary");
}

void delete_image(image_rgb *im) {
    free(*im);
}

int write_compressed(char *output_path, struct compressed_im *comp, uint8_t compression, int rows, int cols) {
    FILE *file = fopen(output_path, "wb");
    if (file == NULL) {
        fprintf(stderr, "unable to open file %s", output_path);
        return -1;
    }
    fprintf(file, "%d %d\n%d\n", rows, cols, compression);

    int comp_r = ceil(rows / 8.0);
    int comp_c = ceil(cols / 8.0);
    int cell_size = 8 - compression;

    size_t len = cell_size*cell_size*comp_r*comp_c;
    fwrite(comp->red, sizeof(int16_t), len, file);
    fwrite(comp->green, sizeof(int16_t), len, file);
    fwrite(comp->blue, sizeof(int16_t), len, file);

    fclose(file);
    return 0;
}

void compress_ppm(char *input_path, char *output_path, uint8_t compression) {
    image_rgb im;
    struct compressed_im comp;
    int rows, cols;
    load_ppm_rgb(input_path, &im, &rows, &cols);

    dct_compress(&im, &comp, rows, cols, compression);

    delete_image(&im);

    write_compressed(output_path, &comp, compression, rows, cols);

    delete_compressed_im(&comp);
}

void decompress_jhpeg(char *input_path, char *output_path) {
    image_rgb im;
    struct compressed_im comp;
    int rows, cols;
    uint8_t compression;
    load_jhpeg_rgb(input_path, &comp, &rows, &cols, &compression);

    dct_decompress(&comp, &im, rows, cols, compression);

    delete_compressed_im(&comp);

    write_ppm_rgb(output_path, &im, rows, cols);
    delete_image(&im);
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
    // testing image writing, also seems to work
    // fft forward and backward functions both work with casting
    compress_ppm("../figures/b.ppm", "./b.jhpeg", 6);

    decompress_jhpeg("./b.jhpeg", "./b.ppm");
}
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

The output files are in a custom encoding, the "jhpeg" format, which is only meant
to be read and decompressed by this program
*/

#include <stdint.h>
#include <complex.h>

// rgb pixel
struct pixel_rgb {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

// struct channel_im {
//     uint8_t *red;
//     uint8_t *green;
//     uint8_t *blue;
// };


struct compressed_im {
    int16_t *red;
    int16_t *green;
    int16_t *blue;
};

typedef struct pixel_rgb* image_rgb;


/*
* read ppm file and return pixel array
*/
int load_ppm_rgb(char *file_name, image_rgb *im, int *rows, int *cols);

/*
* write ppm file from pixel array
*/ 
int write_ppm_rgb(char *file_name, image_rgb *im, int rows, int cols);

/*
* read ppm file and return pixel array
*/
int load_jhpeg_rgb(char *file_name, struct compressed_im *im, int *rows, int *cols, uint8_t *compression);

/*
* read in ppm at input_path and output compressed jhpeg format at output_path
* with float compression specifying the proportion of frequencies to remove
*/
void compress_ppm(char *input_path, char *output_path, uint8_t compression);

/*
* read in compressed jhpeg at input_path, write ppm at output_path
*/
void decompress_jhpeg(char *input_path, char *output_path);
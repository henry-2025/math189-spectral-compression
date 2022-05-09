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

// bw pixel
typedef uint8_t pixel_bw;

struct compressed_rgb {
    float red;
    float green;
    float blue;
};


typedef struct pixel_rgb* image_rgb;
typedef struct pixel_bw* image_bw;


typedef struct compressed_pix_rgb* compressed_image_rgb;
typedef struct compressed_pix_bw* compressed_image_bw;

/*
* read ppm file and return pixel array
*/
int load_ppm_bw(char *file_name, image_bw *im, int *rows, int *cols);
int load_ppm_rgb(char *file_name, image_rgb *im, int *rows, int *cols);


/*
* write ppm file from pixel array
*/ 
int write_ppm_rgb(char *file_name, image_rgb *im, int rows, int cols);

/*
* read in ppm at input_path and output compressed jhpeg format at output_path with float compression specifying the proportion of frequencies to remove
*/
int compress_ppm_bw(char *input_path, char *output_path, float compression);
int compress_ppm_rgb(char *input_path, char *output_path, float compression);

/*
* read in compressed jhpeg at input_path, write ppm at output_path
*/
int decompress_jhpeg_bw(char *input_path, char *output_path);
int decompress_jhpeg_rgb(char *input_path, char *output_path);


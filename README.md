# Image Compression Demonstration
As the name suggests, this was inspired and tries to emulate jpeg with a very basic lossy data compression scheme. It's much much slower than any good jpeg encoder out there, I am guessing because they have developed 2-dimensional dcts that operate natively on 8-bit unsigned integers. I used dct algorithms that operate on floating point numbers so there was an enormous amount of type conversion that had to take place. My scheme also does not use other optimizations like quantization and zigzag redundancy removal which makes the encoding far less efficient. Still, I feel like it was a good demonstration of the core principles involved in compression techniques that use harmonic analysis.

Source files are found in `src/` and the demonstration is compiled by building the default target in the makefile (ie. just running `make` in `src/`). Output binary is a terminal interface, which tells you how to compress and decompress images.

The process of compressing and decompressing an image is the following:
1. Get an image that in binary ppm format (there are a few prepared examples in `figures`)
2. the program to compress the ppm into a jhpeg
3. decompress the jhpeg back into a ppm to view the lossy compression results

This requires that you have installed:
1. gcc, I don't have a fancy makefile so if you want to use another compiler it must be manually edited
2. fftw3 library, I didn't do much research into whether this was a good cross platform library but I know it's fairly popular on linux
3. an image viewer that supports binary ppm files, I used sxiv

# BUGS/TODO
- [ ] Currently only supports the compression of square images, there is an indexing bug for images with rectangular dimensions
- [ ] Test padding on images with non 8-divisible dimensions
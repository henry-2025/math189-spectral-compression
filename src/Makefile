all: debug

jhpeg: jhpeg.c jhpeg.h
	gcc -lm -lfftw3 -lfftw3f jhpeg.c -o jhpeg 
debug: jhpeg.c jhpeg.h
	gcc -g -lm -lfftw3 -lfftw3f jhpeg.c -o jhpeg
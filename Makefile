all:
	gcc -o mandelbrot main.c times.c -lm -fopenmp -lpthread

clean:
	rm -f mandelbrot *.pgm times.txt
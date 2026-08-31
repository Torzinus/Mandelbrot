all:
	gcc -o mandelbrot main.c -lm -fopenmp -lpthread

clean:
	rm -f mandelbrot *.pgm
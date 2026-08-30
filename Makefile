all:
	gcc -o mandelbrot main.c -lm -fopenmp

clean:
	rm -f mandelbrot *.pgm
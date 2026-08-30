all:
	gcc -o mandelbrot main.c -lm

clean:
	rm -f mandelbrot
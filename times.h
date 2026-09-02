#define TIMES_H

#include <time.h>

double medirTempo(struct timespec start, struct timespec end);
void gravarTempo(const char * metodo, double tempo);
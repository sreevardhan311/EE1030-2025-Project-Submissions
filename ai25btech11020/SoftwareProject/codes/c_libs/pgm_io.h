#ifndef PGM_IO_H
#define PGM_IO_H

float* read_pgm(const char *path, int *rows, int *cols);
void write_pgm(const char *path, const float *buf, int rows, int cols);

#endif

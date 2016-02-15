#ifndef _MY_MATRIX_H
#define _MY_MATRIX_H

#include <simd_types.h>

void new_random_matrix(double **A, unsigned N);
void new_matrix(double **A, unsigned N);
void print_matrix(double* A, unsigned n);
void MxM_square(v4d* A, v4d* B, v4d* C, unsigned N);
void MxM_fma_square_scalar(double* A, double* B, double* C, unsigned N);

#endif

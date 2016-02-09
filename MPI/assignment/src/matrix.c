#include <matrix.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>
#include <malloc.h>


void new_random_matrix(double **A, unsigned N){
  *A = malloc(N*N*sizeof(double));//Allign to cachelines instead.
  for(unsigned n = 0; n < N*N; n++){
    (*A)[n] = rand() / ((double) RAND_MAX);
  }
}

void new_matrix(double **A, unsigned N){
  *A = malloc(N*N*sizeof(double));//Allign to cachelines instead.
}

void print_matrix(double *A, unsigned n){
  for(unsigned y = 0; y < n; y++){
    for(unsigned x = 0; x < n; x++)
      printf("%1.2f ", A[y*n+x]);
    puts("");
  }
}

void MxM_square_scalar(double* A, double* B, double* C, unsigned N){
  for (int yA = 0; yA < N; yA++) {
    for (int xB = 0; xB < N; xB++) {
      double tmp = 0.0;
      for (int loop = 0; loop < N; loop++) {
        tmp += A[yA*N+loop] * B[xB+N*loop];
      }
      C[yA*N+xB] = tmp;
    }
  }
}

void MxM_square(v4d* A, v4d* B, v4d* C, unsigned N){
  const unsigned N_reduced = N/4;
  for(unsigned yA = 0; yA < N; yA++){
    for(unsigned xB = 0; xB < N_reduced; xB+=2){
      v4d sum[2] = {{0,0,0,0}, {0,0,0,0}};
      for(unsigned xA = 0; xA < N; xA++){
        v4d broadcast = _mm256_broadcast_sd(((double*)(&(A[yA*N_reduced])))+xA);
        sum[0] += broadcast*B[N_reduced*xA+xB];
        sum[1] += broadcast*B[N_reduced*xA+xB+1];
      }
      C[xB+N_reduced*yA] = sum[0];
      C[xB+N_reduced*yA+1] = sum[1];
    }
  }
}

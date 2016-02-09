#ifndef _MY_SIMD_H
#define _MY_SIMD_H
//avx
typedef double v4d __attribute__ ((vector_size (32)));
typedef float v8f __attribute__ ((vector_size (32)));
//sse (2?)
typedef double v2d __attribute__ ((vector_size (16)));
typedef float v4f __attribute__ ((vector_size (16)));
#endif

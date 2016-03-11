#include <multiX.h>

extern int MAX_FORK_DEPTH;
int partition(double *array, int N);
void quicksort(double *array, int N);
void quicksort_w_fork(double *array, int N);
void quicksort_w_tasks(workQ_t *Q, double *array, int N);
void quicksort_w_peers(double* array, unsigned N, const unsigned thread_count);

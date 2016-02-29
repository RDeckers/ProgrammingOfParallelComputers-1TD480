#include <common.h>
#include <utilities/logging.h>
#include <utilities/benchmarking.h>
#include <quicksort.h>
#include <string.h>
#include <float.h>



int main(int argc, char **argv){
  REPORT_TIMESTAMP = 1;
  struct timespec t;
  double *array = NULL;
  int ret;
  double elapsed_base, elapsed;
  const int N_TRIALS_MAX = 4096;
  int N_TRIALS = 5;
  double temporary_timing[N_TRIALS_MAX];
  printf("#array_size, ... speedup with 2^n threads max.");
  for(size_t array_size = (1 << 27); array_size >= (1 << 6); array_size = array_size >> 1){
    printf("\n%d ", array_size);
    //N_TRIALS *= 2;
    for(int trial = 0; trial < N_TRIALS; trial++){
      if(0 != (ret = reallocate_and_fill_array(&array, array_size))){
        return ret;
      }
      tick(&t);
      quicksort(array, array_size);
      temporary_timing[trial] = elapsed_since(&t);
      report(INFO, "\t trial %d: %e", trial, temporary_timing[trial]);
    }
    quicksort(temporary_timing, N_TRIALS);
    elapsed = temporary_timing[N_TRIALS/2];
    report(PASS, "Sorted array in %1.2e ns, spread = %1.2e", elapsed, temporary_timing[N_TRIALS-1]-temporary_timing[0]);
    printf("%e", elapsed);
  }
  free(array);
  return 0;
}

#include <sor_grid.h>
#include <math.h>
#include <stdlib.h>
#include <utilities/logging.h>
#include <utilities/benchmarking.h>

double sin_field(double x, double y){
  return sin(2*M_PI*x*x);
}

int main(int argc, char** argv){
  bounded_field_t bounded_field_naive, bounded_field;
  for(unsigned size = 16; size <= 512; size += 16){
    bounded_field_initialize(&bounded_field, size, size, 0,1, 0,1, sin_field, sin_field);
    bounded_field_initialize(&bounded_field_naive, size, size, 0,1, 0,1, sin_field, sin_field);
    stopping_criterion_t stopping_criterion = DEFAULT_STOPPING_CRITERION;
    struct timespec T;
    tick(&T);
    bounded_field_run_naive(&bounded_field_naive, 1.6, NULL);
    double time_taken_naive = tock(&T);
    bounded_field_run(&bounded_field, 1.6, NULL);
    double time_taken = tock(&T);
    printf("%d %e %e %f\n", size, time_taken_naive, time_taken, time_taken_naive/time_taken);
    bounded_field_clean(&bounded_field);
    bounded_field_clean(&bounded_field_naive);
  }
  return 0;

}

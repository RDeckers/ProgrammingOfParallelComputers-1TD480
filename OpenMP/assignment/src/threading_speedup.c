#include <sor_grid.h>
#include <math.h>
#include <stdlib.h>
#include <utilities/logging.h>
#include <utilities/benchmarking.h>
#include <omp.h>

double random_init(double x, double y){
  return drand48();
}

double sin_field(double x, double y){
  return sin(2*M_PI*x*x);
}

int main(int argc, char** argv){
  REPORT_TIMESTAMP = 1;
  bounded_field_t bounded_field;
  for(unsigned n_threads = 1; n_threads <= 32; n_threads*=2){
    srand48(0);
    bounded_field_initialize(&bounded_field, 200, 200, 0,1, 0,1, sin_field, random_init);
    omp_set_num_threads(n_threads);
    struct timespec T;
    tick(&T);
    bounded_field_run(&bounded_field, 1.7, NULL);
    double time_taken = tock(&T);
    report(INFO,"%u %e\n", n_threads, time_taken);
    printf("%u %e\n", n_threads, time_taken);
    bounded_field_clean(&bounded_field);
  }
  return 0;

}

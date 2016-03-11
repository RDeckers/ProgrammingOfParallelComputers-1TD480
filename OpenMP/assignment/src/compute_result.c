#include <sor_grid.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <utilities/logging.h>

double random_init(double x, double y){
  return drand48();
}

double sin_field(double x, double y){
  return sin(2*M_PI*x*x);
}

int main(int argc, char** argv){
  bounded_field_t bounded_field;
  bounded_field_initialize(&bounded_field, 64, 64, 0,1, 0,1, sin_field, random_init);
  stopping_criterion_t stopping_criterion = DEFAULT_STOPPING_CRITERION;
  bounded_field_run(&bounded_field, 1.7, NULL);
  bounded_field_print_interior(&bounded_field);
  puts("");
  bounded_field_print_residual_field(&bounded_field);
  return 0;

}

#include <sor_grid.h>
#include <utilities/logging.h>
#include <math.h>

double sin_field(double x, double y){
  return sin(2*M_PI*x*x);
}

double random_init(double x, double y){
  return drand48();
}

int main(int argc, char** argv){
  REPORT_TIMESTAMP = 1;;
  const unsigned max_iterations = 1 << 12;
  bounded_field_t bounded_field;
  const double tolerance = 1e-6;
  for(double omega = 0.5; omega <= 2; omega+=0.025){
    srand48(0);
    bounded_field_initialize(&bounded_field, 32, 32, 0,1, 0,1, sin_field, random_init);
    bounded_field_copy_edges(&bounded_field);
    double residual = bounded_field_residual_L2_norm(&bounded_field);
    printf("%d %1.2f %e\n",omega, 0, omega, residual);
    for(unsigned w = 0; (w < max_iterations) && (residual >= tolerance); w++){
      bounded_field_update(&bounded_field, omega);
      residual = bounded_field_residual_L2_norm(&bounded_field);
      printf("%d %f %e\n", w+1, omega, residual);
    }
    printf("\n");
    bounded_field_clean(&bounded_field);
    report(PASS, "Finsished omega=%f with %f", omega, residual);
  }
  return 0;
}

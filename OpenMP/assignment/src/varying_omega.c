#include <sor_grid.h>
#include <utilities/logging.h>
#include <math.h>

double sin_field(double x, double y){
  return sin(2*M_PI*x*x);
}



int main(int argc, char** argv){
  REPORT_TIMESTAMP = 1;
  double omegas[] = {0.5, 1.0, 1.5, 1.6, 1.7, 1.8, 2.0 };
  const unsigned n_omegas = sizeof(omegas)/sizeof(double);
  report(INFO, "testing %u omegas", n_omegas);
  const unsigned max_iterations = 1 << 12;
  bounded_field_t bounded_field;
  const double tolerance = 1e-6;
  for(unsigned u = 0; u < n_omegas; u++){
    bounded_field_initialize(&bounded_field, 32, 32, 0,1, 0,1, sin_field, sin_field);
    bounded_field_copy_edges(&bounded_field);
    double residual = bounded_field_residual_L2_norm(&bounded_field);
    printf("\"w=%1.2f\"\n%d %e\n",omegas[u], 0, residual);
    for(unsigned w = 0; (w < max_iterations) && (residual >= tolerance); w++){
      bounded_field_update(&bounded_field, omegas[u]);
      residual = bounded_field_residual_L2_norm(&bounded_field);
      printf("%d %e\n", w+1, residual);
    }
    printf("\n\n\n");
    bounded_field_clean(&bounded_field);
    report(PASS, "Finsished omega=%f with %f", omegas[u], residual);
  }
  return 0;
}

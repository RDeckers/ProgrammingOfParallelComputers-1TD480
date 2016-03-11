#ifndef _SOR_GRID_H
#define _SOR_GRID_H

typedef struct{
  unsigned max_iterations;
  double tolerance;
  double minimum_residual_ratio;
}stopping_criterion_t;

extern const stopping_criterion_t DEFAULT_STOPPING_CRITERION;

typedef struct{
  unsigned nx; //interior size horizontal
  unsigned ny; //interior size vertical
  double x0;
  double x1;
  double y0;
  double y1;
  double dx;
  double dy;
  double *f_field;
  double (*f)(double, double); //TODO: can be optimized out, but needed for benchmarking change.
  double *field;
}bounded_field_t;

void bounded_field_set_at_index(bounded_field_t *bounded_field, double value, int x, int y);

int bounded_field_initialize(
  bounded_field_t *bounded_field,
  unsigned nx, unsigned ny,
  double x0, double x1,
  double y0, double y1,
  double (*f)(double, double),
  double (*initializer)(double,double)
);

unsigned bounded_field_run(
  bounded_field_t *bounded_field,
  double omega,
  const stopping_criterion_t *stopping_criterion
);

unsigned bounded_field_run_naive(
  bounded_field_t *bounded_field,
  double omega,
  const stopping_criterion_t *stopping_criterion
);

void bounded_field_copy_edges(bounded_field_t *bounded_field);

double bounded_field_update(
  bounded_field_t *bounded_field,
  double omega
);

double bounded_field_update_naive(
  bounded_field_t *bounded_field,
  double omega
);

double bounded_field_at_index(
  bounded_field_t *bounded_field,
  int x,
  int y
);

double bounded_field_index_to_coord_x(
  bounded_field_t *bounded_field,
  int x
);

double bounded_field_index_to_coord_y(
  bounded_field_t *bounded_field,
  int y
);

double bounded_field_f_at_index(
  bounded_field_t *bounded_field,
  int x,
  int y
);
double bounded_field_f_at_index_naive(
  bounded_field_t *bounded_field,
  int x,
  int y
);

double bounded_field_dx(bounded_field_t* bounded_field);
double bounded_field_dy(bounded_field_t* bounded_field);
double bounded_field_dx_naive(bounded_field_t* bounded_field);
double bounded_field_dy_naive(bounded_field_t* bounded_field);

double bounded_field_laplacian_at_index(
  bounded_field_t* bounded_field,
  int x,
  int y
);

double bounded_field_laplacian_at_index_naive(
  bounded_field_t* bounded_field,
  int x,
  int y
);

double bounded_field_update_point(
  bounded_field_t *bounded_field,
  int x,
  int y,
  double omega
);

double bounded_field_residual_at_index(
  bounded_field_t *bounded_field,
  int x,
  int y
);

double bounded_field_residual_L2_norm(bounded_field_t *bounded_field);
double bounded_field_update_point_naive(
  bounded_field_t *bounded_field,
  int x,
  int y,
  double omega
);

double bounded_field_residual_at_index_naive(
  bounded_field_t *bounded_field,
  int x,
  int y
);

double bounded_field_residual_L2_norm_naive(bounded_field_t *bounded_field);

void bounded_field_print(bounded_field_t *bounded_field);
void bounded_field_print_interior(bounded_field_t *bounded_field);
void bounded_field_print_residual_field(bounded_field_t *bounded_field);
void bounded_field_clean(bounded_field_t *bounded_field);

#endif

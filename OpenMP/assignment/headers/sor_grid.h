#ifndef _SOR_GRID_H
#define _SOR_GRID_H

typedef struct{
  unsigned max_iterations; //stop when it's taken to long
  double tolerance;//stop when good enough
  double minimum_residual_ratio; //the minimum ratio of old vs. new residual, can be used to stop when the convergence becomes too slow.
}stopping_criterion_t;

extern const stopping_criterion_t DEFAULT_STOPPING_CRITERION;

typedef struct{
  unsigned nx; //interior size horizontal
  unsigned ny; //interior size vertical
  double x0;
  double x1;
  double y0;
  double y1;
  double dx;//implicit but cached
  double dy;
  double *f_field; //f is constant so we cache it, sin is expensive.
  double (*f)(double, double); //TODO: can be optimized out, but needed for benchmarking change.
  double *field; // the field including boundaries
}bounded_field_t;

//conveince wrapper for setting the interior field with zero-indexing.
void bounded_field_set_at_index(bounded_field_t *bounded_field, double value, int x, int y);

/*initializes all the neccesary variables and allocates arrays*/
int bounded_field_initialize(
  bounded_field_t *bounded_field,
  unsigned nx, unsigned ny,
  double x0, double x1,
  double y0, double y1,
  double (*f)(double, double),
  double (*initializer)(double,double)
);

/* iterates until one of the conditions in stopping_criterion is reached, can be NULL in which case the default is used */
unsigned bounded_field_run(
  bounded_field_t *bounded_field,
  double omega,
  const stopping_criterion_t *stopping_criterion
);

/*Does what it says*/
void bounded_field_copy_edges(bounded_field_t *bounded_field);

/*updates all the interior points*/
double bounded_field_update(
  bounded_field_t *bounded_field,
  double omega
);

/*Conveinice wrapper*/
double bounded_field_at_index(
  bounded_field_t *bounded_field,
  int x,
  int y
);

/*can be used to get the x-coord from an index.*/
double bounded_field_index_to_coord_x(
  bounded_field_t *bounded_field,
  int x
);

/*can be used to get the y-coord from an index.*/
double bounded_field_index_to_coord_y(
  bounded_field_t *bounded_field,
  int y
);

double bounded_field_f_at_index(
  bounded_field_t *bounded_field,
  int x,
  int y
);

/*evaluates f at the the point corresponding to x,y*/
double bounded_field_eval_f_at_index(
  bounded_field_t *bounded_field,
  int x,
  int y
);

double bounded_field_dx(bounded_field_t* bounded_field);
double bounded_field_dy(bounded_field_t* bounded_field);

/*fetches the cached laplacian*/
double bounded_field_laplacian_at_index(
  bounded_field_t* bounded_field,
  int x,
  int y
);

/*updates a single point*/
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

/*computes the L2 norm of the residual in parallel*/
double bounded_field_residual_L2_norm(bounded_field_t *bounded_field);

/*display functions*/
void bounded_field_print(bounded_field_t *bounded_field);
void bounded_field_print_interior(bounded_field_t *bounded_field);
void bounded_field_print_residual_field(bounded_field_t *bounded_field);
/*releases the memory held by bounded_field*/
void bounded_field_clean(bounded_field_t *bounded_field);

#endif

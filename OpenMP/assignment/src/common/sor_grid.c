#include <sor_grid.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const stopping_criterion_t DEFAULT_STOPPING_CRITERION = {
  .max_iterations = (1 << 16),
  .tolerance = 1e-9,
  .minimum_residual_ratio = 1e-4
};

int bounded_field_initialize(
  bounded_field_t *bounded_field,
  unsigned nx, unsigned ny,
  double x0, double x1,
  double y0, double y1,
  double (*f)(double, double),
  double (*initializer)(double,double)
){
  if(!f){
    return 0;
  }
  bounded_field->field = calloc((2+nx)*(2+ny), sizeof(double));
  if(!bounded_field->field){
    return 0;
  }
  if(!(bounded_field->f_field = malloc(nx*ny*sizeof(double)))){
    free(bounded_field->field);
    return 0;
  }
  bounded_field->f = f;
  bounded_field->nx = nx;
  bounded_field->ny = ny;
  bounded_field->x0 = x0;
  bounded_field->x1 = x1;
  bounded_field->y0 = y0;
  bounded_field->y1 = y1;
  bounded_field->dx = (x1-x0)/nx;
  bounded_field->dy = (y1-y0)/ny;
  if(initializer){
    double dx = (x1-x0)/nx;
    double dy = (y1-y0)/ny;
    for(unsigned u = 0; u < ny; u++){
      double y = y0 + (u+0.5)*dy;
      for(unsigned w = 0; w < nx; w++){
        double x = x0 + (w+0.5)*dx;
        bounded_field->field[(u+1)*(nx+2)+w+1] = initializer(x,y);
      }
    }
  }
  for(unsigned y = 0; y < ny; y++){
    for(unsigned x = 0; x < nx; x++){
      bounded_field->f_field[y*nx+x] = bounded_field_eval_f_at_index(bounded_field, x, y);
    }
  }
  return 1;
}

void bounded_field_copy_edges(bounded_field_t *bounded_field){
  int nx = bounded_field->nx;
  int ny = bounded_field->ny;
  double *field =bounded_field->field;
  for(int x = 0; x < nx; x++){
    field[1+x] = bounded_field_at_index(bounded_field, x, 0);
    field[(ny+1)*(nx+2)+1+x] = bounded_field_at_index(bounded_field, x, ny-1);
  }
  for(int y = 0; y < ny; y++){
    field[(nx+2)*(y+1)] = bounded_field_at_index(bounded_field, 0, y);
    field[(nx+2)*(y+1)+nx+1] = bounded_field_at_index(bounded_field, nx-1, y);
  }
}

inline double bounded_field_update(bounded_field_t *bounded_field, double omega){
  int nx = bounded_field->nx;
  int ny = bounded_field->ny;
  //red loop
  double sum = 0;
  #pragma omp parallel
  {
      #pragma omp for schedule(static) reduction(+:sum)
      for(int y = 0; y < ny; y ++){
        for(int x = y&1; x < nx; x += 2){
          sum += bounded_field_update_point(bounded_field, x, y, omega);
        }
      }
      //black loop
      #pragma omp for schedule(static) //16 doubles in a cache-line, so this touches 2 cha
      for(int y = 0; y < ny; y ++){
        for(int x = !(y&1); x < nx; x += 2){
          sum += bounded_field_update_point(bounded_field, x, y, omega);
        }
      }
    double avg = sum / (nx*ny);
    #pragma omp for schedule(static)
    for(int y = 0; y < ny; y++){
      for(int x = 0; x < nx; x++){
        double old_value = bounded_field_at_index(bounded_field, x, y);
        double new_value = old_value - avg;
        bounded_field_set_at_index(bounded_field, new_value, x, y);
      }
    }
  }
  bounded_field_copy_edges(bounded_field);
}

unsigned bounded_field_run(
  bounded_field_t *bounded_field,
  double omega,
  const stopping_criterion_t *stopping_criterion
){
  if(!stopping_criterion){
    stopping_criterion = &DEFAULT_STOPPING_CRITERION;
  }
  double min_relative_change = stopping_criterion->minimum_residual_ratio;
  double residual = stopping_criterion->tolerance;
  unsigned max_iterations = stopping_criterion->max_iterations;
  unsigned iterations;

  bounded_field_copy_edges(bounded_field);
  double residual_norm = bounded_field_residual_L2_norm(bounded_field);
  double relative_change = 2*min_relative_change;
  for(iterations = 0; (residual_norm >= residual) && (relative_change >= min_relative_change) && (iterations < max_iterations); iterations++){
    bounded_field_update(bounded_field, omega);
    double new_residual_norm = bounded_field_residual_L2_norm(bounded_field);
    double relative_change = residual_norm/new_residual_norm;
    residual_norm = new_residual_norm;
  }
  return iterations;
}



inline void bounded_field_set_at_index(bounded_field_t *bounded_field, double value, int x, int y){
  bounded_field->field[(y+1)*(bounded_field->nx+2)+(x+1)] = value;
}

inline double bounded_field_at_index(bounded_field_t *bounded_field, int x, int y){
  return bounded_field->field[(y+1)*(bounded_field->nx+2)+(x+1)];
}

inline double bounded_field_index_to_coord_x(bounded_field_t *bounded_field, int x){
  return bounded_field->x0+bounded_field_dx(bounded_field)*x;
}

inline double bounded_field_index_to_coord_y(bounded_field_t *bounded_field, int y){
  return bounded_field->y0+bounded_field_dx(bounded_field)*y;
}

inline double bounded_field_eval_f_at_index(bounded_field_t *bounded_field, int x, int y){
  return bounded_field->f(
    bounded_field_index_to_coord_x(bounded_field,x),
    bounded_field_index_to_coord_y(bounded_field, y)
  );
}

inline double bounded_field_f_at_index(bounded_field_t *bounded_field, int x, int y){
  return bounded_field->f_field[y*bounded_field->nx+x];
}

inline double bounded_field_dx(bounded_field_t* bounded_field){
  return bounded_field->dx;
}

inline double bounded_field_dy(bounded_field_t* bounded_field){
  return bounded_field->dy;
}

inline double bounded_field_laplacian_at_index(bounded_field_t* bounded_field, int x, int y){
  double field_n = bounded_field_at_index(bounded_field,x,y+1);
  double field_e = bounded_field_at_index(bounded_field,x+1,y);
  double field_s = bounded_field_at_index(bounded_field,x,y-1);
  double field_w = bounded_field_at_index(bounded_field,x-1,y);
  double dx = bounded_field_dx(bounded_field);
  double dy = bounded_field_dy(bounded_field);
  return (field_n+field_s)/(dy*dy)+(field_e+field_w)/(dx*dx);
}

double bounded_field_update_point(bounded_field_t *bounded_field, int x, int y, double omega){
  double field_here = bounded_field_at_index(bounded_field,x,y);
  double old_term = (1-omega)*field_here;
  double dx = bounded_field_dx(bounded_field);
  double dy = bounded_field_dy(bounded_field);
  double laplacian = bounded_field_laplacian_at_index(bounded_field, x, y);
  double f = bounded_field_f_at_index(bounded_field, x, y);
  double new_term = omega/(2/(dx*dx)+2/(dy*dy))*(laplacian-f);
  bounded_field_set_at_index(bounded_field, old_term+new_term, x, y);
  return old_term+new_term;
}

double bounded_field_residual_at_index(bounded_field_t *bounded_field, int x, int y){
  double laplacian = bounded_field_laplacian_at_index(bounded_field, x, y);
  double f = bounded_field_f_at_index(bounded_field, x, y);
  double here = bounded_field_at_index(bounded_field, x, y);
  double dx = bounded_field_dx(bounded_field);
  double dy = bounded_field_dy(bounded_field);
  return f-(laplacian-2*here*(1/(dx*dx)+1/(dy*dy)));
}

inline double bounded_field_residual_L2_norm(bounded_field_t *bounded_field){
  int nx = bounded_field->nx;
  int ny = bounded_field->ny;
  double residual_sum = 0;
  #pragma omp parallel
  {
      double local_sum = 0.0;
      #pragma omp for schedule(static) reduction(+:residual_sum)
      for(int y = 0; y < ny; y++){
        for(int x = 0; x < nx; x++){
          double residual = bounded_field_residual_at_index(bounded_field, x, y);
          residual_sum += residual*residual;
        }
      }
  }
  return sqrt(residual_sum/(nx*ny));
}

void bounded_field_print(bounded_field_t *bounded_field){
  unsigned nx = bounded_field->nx;
  unsigned ny = bounded_field->ny;
  for(unsigned u = 0; u < ny+2; u++){
    for(unsigned w= 0; w < nx+2; w++){
      printf("%+1.3e ", bounded_field->field[u*(nx+2)+w]);
    }
    puts("");
  }
}

void bounded_field_print_interior(bounded_field_t *bounded_field){
  int nx = bounded_field->nx;
  int ny = bounded_field->ny;
  for(int y = 0; y < ny; y++){
    for(int x= 0; x < nx; x++){
      printf("%+1.3f ", bounded_field_at_index(bounded_field, x, y));
    }
    puts("");
  }
}

void bounded_field_print_residual_field(bounded_field_t *bounded_field){
  int nx = bounded_field->nx;
  int ny = bounded_field->ny;
  for(int y = 0; y < ny; y++){
    for(int x= 0; x < nx; x++){
      printf("%+1.3f ", bounded_field_residual_at_index(bounded_field, x, y));
    }
    puts("");
  }
}

void bounded_field_clean(bounded_field_t *bounded_field){
  free(bounded_field->field);
  free(bounded_field->f_field);
}

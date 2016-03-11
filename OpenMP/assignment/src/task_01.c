#include <sor_grid.h>
#include <utilities/logging.h>

double one_field(double x, double y){
  return 1;
}

int main(int argc, char** argv){
  bounded_field_t bounded_field;
  bounded_field_initialize(&bounded_field, 10, 10, 0,1, 0,1, one_field, one_field);
  bounded_field_set_at_index(&bounded_field, 5, 2,3);//set to 5 at 3,4 (starting from 1)
  bounded_field_print(&bounded_field);
  return 0;
}

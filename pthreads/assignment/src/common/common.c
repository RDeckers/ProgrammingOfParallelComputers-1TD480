#include <common.h>
#include <utilities/logging.h>
#include <errno.h>
#include <string.h>

int reallocate_and_fill_array(double **array, size_t array_size){
  free(*array);
  *array = malloc(array_size*sizeof(double));
  if(NULL == *array){
    report(FAIL, "Allocation failed: %s!", strerror(errno));
    return errno;
  }
  for(unsigned u = 0; u < array_size; u++){
    (*array)[u] = drand48();
  }
  return 0;
}

#include <quicksort.h>
#include <pthread.h>
#include <utilities/logging.h>
#include <multiX.h>

//returns the median of the middle, start and end of the array.
//Measured to perform slightly better thank taking just the middle with random data for small arrays.
double get_pivot(double *array, int N){
  double start = array[0];
  double middle = array[N/2];
  double end = array[N-1];
  if((start <= middle && middle <= end) || (end <= middle && middle <= start)){
    return middle;
  }else if((middle <= start && start <= end) || (end <= start && start <= middle)){
    return start;
  }
  else if((start <= end && end <= middle) || (middle <= end && end <= start)){
    return end;
  }
}

int partition(double *array, int N){
  double pivot = get_pivot(array, N);//select pivot halfway in the list
  int i, j;
  for (i = 0, j = N - 1;; i++, j--) {//loop i forwards, j backwards
    while (array[i] < pivot) //find the next element larger or eq then the pivot in array, forwards
      i++;
    while (pivot < array[j]) //find the next element smaller or eq then the pivot in array, backwards
      j--;
    if (i >= j) //If the forward and backward search cross-over, exit the loop.
      break;
    //swap the two elements.
    double tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
  }
  return i; //cross-over point.
}

void get_largest_and_smallest(
  double *array, int N, int i,
  double **largest, int *large_size,
  double **smallest, int *small_size
){
  if(i > N - i){
    *smallest = array+i;
    *small_size = N-i;
    *largest = array;
    *large_size = i;
  }else{
    *smallest = array;
    *small_size = i;
    *largest = array+i;
    *large_size = N-i;
  }
}

void quicksort(double *array, int N){
  /*Initially taken from http://rosettacode.org/wiki/Sorting_algorithms/Quicksort#C which is a straightfoward
  implementation of https://en.wikipedia.org/wiki/Quicksort#Hoare_partition_scheme
  modified slightly for consistency and slight optimizations*/
  if (N < 2)//done sorting
    return;
  int i = partition(array, N);
  //recurse into the next arrays, smallest one first so the large one can be tail-call optimized.
  //We expect the smaller ones to have a smaller recursion depth.
  double *smallest, *largest;
  int large_size, small_size;
  get_largest_and_smallest(array, N, i, &largest, &large_size, &smallest, &small_size);
  quicksort(smallest, small_size);
  quicksort(largest, large_size);
}

struct quicksort_subroutine_args{
  double* array;
  int N;
  int depth;
};

typedef struct{
  double* array;
  int N;
} function_args_t;


void quicksort_task(work_item_dq_t* parent, void* args){
  function_args_t *casted_args = args;
  //read ou the data pointed to
  double *array = casted_args->array;
  int N = casted_args->N;
  //and free the pointer
  free(casted_args);
  //report(PASS, "walking back up free-tree, %p", casted_args);
  if (N < 2)//done sorting
    return;
  else if(N <= 1024){
    quicksort(array, N);
  }

  int i = partition(array, N);

  double *smallest, *largest;
  int large_size, small_size;
  get_largest_and_smallest(array, N, i, &largest, &large_size, &smallest, &small_size);

  function_args_t *large_args = malloc(sizeof(function_args_t));
  function_args_t *small_args = malloc(sizeof(function_args_t));
  //report(WARN, "walking down the malloc-tree, %p %p", small_args, large_args);
  large_args->array = largest;
  small_args->array = smallest;

  large_args->N = i;
  small_args->N = N-i;
  add_work_item_to_dq(parent, &quicksort_task, small_args); //small args will be deallocated by whomever gets this
  quicksort_task(parent, large_args); //we will deallocate large_args
}

void quicksort_w_tasks(workQ_t *Q, double *array, int N){
  //add the first work-item to the queue, it will automatically add more.
  function_args_t *args = malloc(sizeof(function_args_t));
  args->array = array;
  args->N = N;
  add_work_item(Q, &quicksort_task, args);
  //report(WARN, "starting the Q");
  finish(Q);
  //report(WARN, "FINISHED the Q");
}

void actual_quicksort_w_fork(double *array, int N, int depth);

void* quicksort_subroutine_wrapper(void *args){
  struct quicksort_subroutine_args *casted_args = args;
  actual_quicksort_w_fork(casted_args->array, casted_args->N, casted_args->depth);
  return NULL;
}

void quicksort_w_fork(double *array, int N){
  actual_quicksort_w_fork(array, N, 0);
}

int MAX_FORK_DEPTH = 7;
void actual_quicksort_w_fork(double *array, int N, int depth){
  const int min_fork_N = 1024; /*if N is less than this, just run serial. hard to determine due to incosistent run-times*/
  if (N < 2)//done sorting
    return;
  int i = partition(array, N);
  double *smallest, *largest;
  int large_size, small_size;
  get_largest_and_smallest(array, N, i, &largest, &large_size, &smallest, &small_size);

  if( (N >= min_fork_N) && (depth < MAX_FORK_DEPTH)){
    //report(WARN, "forking on N = %d and depth = %d", N, depth);
    struct quicksort_subroutine_args args = {.array = smallest, .N = small_size, .depth = depth+1};
    pthread_t thread;
    pthread_create(&thread, NULL, &quicksort_subroutine_wrapper, &args);
    actual_quicksort_w_fork(largest, large_size, depth+1);
    pthread_join(thread, NULL);
  }
  else{
    //report(PASS, "not forking on N = %d and depth = %d", N, depth);
    quicksort(smallest, small_size);
    quicksort(largest, large_size);
  }
}

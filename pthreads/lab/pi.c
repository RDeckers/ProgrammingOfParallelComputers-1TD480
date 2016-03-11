/**********************************************************************
 * This program calculates pi using C
 *
 **********************************************************************/
#include <stdio.h>
#include <pthread.h>

double glob_sum;
pthread_mutex_t lock;

typedef struct{
  double x0; //start of interval
  double x1; //end of interval
  unsigned steps; //points in interval
}thread_arg_t;

void* work(void* args){
  thread_arg_t* c_args = args;
  const double x0 = c_args->x0;
  const double x1 = c_args->x1;
  const unsigned steps = c_args->steps;
  const double dx = (x1-x0)/steps;
  double sum = 0;
  for(unsigned u =0; u < steps; u++){
    double x = x0+dx*(u+0.5);
    sum += dx*4.0/(1.0+x*x);
  }

  pthread_mutex_lock(&lock);
  glob_sum += sum;
  pthread_mutex_unlock(&lock);
  //Abuse pointers to cast a double to a void*.
  size_t *test = (size_t*)&sum;
  return (void*)(*test);
}

int main(int argc, char *argv[]) {
  glob_sum = 0.0;
  pthread_mutex_init(&lock, NULL);
  const unsigned num_threads = 8;
  const double x0 = 0;
  const double x1 = 1;
  const double dx = (x1-x0)/num_threads; //dx between thread intervals
  const unsigned steps = (1 << 20)/num_threads; //total of 2^20 (~= 1 million) points.
  pthread_t threads[num_threads];
  thread_arg_t args[num_threads];
  for(unsigned u = 0; u < num_threads; u++){
    args[u] = (thread_arg_t){.x0 = x0+u*dx, .x1 = x0+(u+1)*dx, .steps = steps};
    pthread_create(&threads[u], NULL, &work, &args[u]);
  }
  double sum = 0.0;
  for(unsigned u = u; u < num_threads; u++){
    double partial_sum;
    pthread_join(threads[u], (void**)&partial_sum);
    sum += partial_sum;
  }
  printf("(using ret)\tPI is approx. %.16f\n",  sum);//this one is always consistent because the order of summation is always the same.
  printf("(using globals)\tPI is approx. %.16f\n",  glob_sum); //this one can vary in the last digit depending on the order of execution.

  return 0;
}

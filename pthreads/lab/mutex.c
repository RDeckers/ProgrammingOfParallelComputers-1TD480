/*****************************************************************************
* FILE: mutex
 ******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS	5
double sum=0;
pthread_mutex_t mutexsum;

/*
 without the mutex multiple threads might try to update sum at the same time, making it possible to have
 thread0	thread1
 Read									sum =0
 					Read				sum =0
					increase		local copy = 1
 increase							local copy = 1
 write								sum = 1
 					write				sum = 1, should be 2.

	so the final result will be anything <= 500k. If we compile with O3 the compiler recognizes that the for loop increases
	sum by 100k, so the possible reuslts are 0,100k,200k,300k,400k,500k without locks.
*/

void *addone(void *arg)
{
	int i;
	//pthread_mutex_lock (&mutexsum);
	for (i=0;i<100000;i++)
		sum = sum + 1.0;
  //pthread_mutex_unlock (&mutexsum);
  pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   pthread_t thread[NUM_THREADS];
   pthread_attr_t attr;
	int t;
   void *status;

   /* Initialize and set thread detached attribute */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   pthread_mutex_init(&mutexsum, NULL);

   for(t=0; t<NUM_THREADS; t++)
	   pthread_create(&thread[t], &attr, addone, NULL);

   for(t=0; t<NUM_THREADS; t++)
	   pthread_join(thread[t], &status);

    printf("Sum: %f\n",sum);
	pthread_mutex_destroy (&mutexsum);
	pthread_attr_destroy(&attr);
    pthread_exit(NULL);
}

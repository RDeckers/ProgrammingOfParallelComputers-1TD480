/*****************************************************************************
* FILE: join.c
*
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define NUM_THREADS	4

/*
  A joinable thread is one which can be pthread_join()'d, of course, which is a means of detecting when a thread has finished
  executing and this call frees up the thread and it's resources. A thread which is detached can no longer be joined (or it can, it's undefined behaviour)
  and will clean up after itself when it terminates.

  If we create detached threads without joining them, then main waits for all other threads to call 
  pthread_exit() after which it to terminates. If we do not use pthread_exit in the main thread then it uses its implicit return and
  the process terminates immidiatly.
*/

void *BusyWork(void *t)
{
   int i;
   long tid;
   double result=0.0;
   tid = (long)t;
   printf("Thread %ld starting...\n",tid);
   for (i=0; i<10000000; i++)
   {
      result = result + sin(i) * tan(i);
   }
   printf("Thread %ld done. Result = %e\n",tid, result);
   pthread_exit((void*) t);
}

int main (int argc, char *argv[])
{
   pthread_t thread[NUM_THREADS];
   pthread_attr_t attr;
   int rc;
   long t;
   void *status;

   /* Initialize and set thread detached attribute */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

   for(t=0; t<NUM_THREADS; t++) {
      printf("Main: creating thread %ld\n", t);
      rc = pthread_create(&thread[t], &attr, BusyWork, (void *)t);
      if (rc) {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
         }
      }

   /* Free attribute and wait for the other threads */
   pthread_attr_destroy(&attr);
  //  for(t=0; t<NUM_THREADS; t++) {
  //     rc = pthread_join(thread[t], &status);
  //     if (rc) {
  //        printf("ERROR; return code from pthread_join() is %d\n", rc);
  //        exit(-1);
  //        }
  //     printf("Main: completed join with thread %ld having a status of %ld\n",t,(long)status);
  //     }

printf("Main: program completed. Exiting.\n");
//pthread_exit(NULL);
}

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS	8

/*
without the volatile qualifier on state it doesn't work because state can/will get cached in a register (or the entire
spinloop gets optimized out).
*/

pthread_mutex_t lock;
pthread_cond_t mysignal;
int waiting=0;
volatile int state=0;

void barrier(){
  int mystate;
  pthread_mutex_lock (&lock);
  mystate=state;
  waiting++;
  if (waiting==NUM_THREADS){
    waiting=0; state=-1;
  }
  pthread_mutex_unlock (&lock);
  while (mystate==state) ;//state needs to be volatile so that this loop is always read from memory and never optimized out or cached in registers.

}

void *HelloWorld(void *arg)
{
  long id=(long)arg;
  printf("Hello World! %d\n",id);
  barrier();
  printf("Bye Bye World! %d\n",id);
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  pthread_t threads[NUM_THREADS];
  long t;
  pthread_cond_init(&mysignal,NULL);
  pthread_mutex_init(&lock,NULL);
  for(t=0;t<NUM_THREADS;t++)
  pthread_create(&threads[t], NULL, HelloWorld,(void *)t);
  pthread_exit(NULL);
}

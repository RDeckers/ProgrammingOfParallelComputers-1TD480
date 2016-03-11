#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS	5

void *HelloWorld(void *arg)
{
   size_t id = (size_t)arg; //uncast arg back into size_t.
   printf("Hello World from %d!\n", id);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
pthread_t threads[NUM_THREADS];
//changed this to size_t from int because sizeof(int) != sizeof(void*).
size_t t;
for(t=0;t<NUM_THREADS;t++)
  pthread_create(&threads[t], NULL, HelloWorld, (void*)t);//cast t to a void*.
  pthread_exit(NULL);
}

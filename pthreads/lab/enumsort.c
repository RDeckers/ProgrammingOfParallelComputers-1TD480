/**********************************************************************
* Enumeration sort
*
**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#define NUM_THREADS	8
#define len 100000


/*
first method, 8 threads, takes about 1.58.
secod method, 8 threads, takes about 0.66.
*/
double indata[len], outdata[len];

void* find_ranks(void *arg){
	uint64_t fused = arg;
	uint32_t start = fused, end = (fused >> 32) > len? len: (fused >> 32);
	for(unsigned j = start; j < end; j++){
		unsigned rank=0;
		for (int i=0;i<len;i++){
			if (indata[i]<indata[j]){
				rank++;
			}
		}
		outdata[rank]=indata[j];
	}
	pthread_exit(NULL);
}

void *findrank(void *arg)
{
	int rank,i;
	long j=(long)arg;

	rank=0;
	for (i=0;i<len;i++)
	if (indata[i]<indata[j]) rank++;
	outdata[rank]=indata[j];
	pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	int seed,i,j,rank,nthreads,ttime,t;
	long el;
	void *status;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


	// Generate random numbers
	for (i=0;i<len;i++){
		indata[i]=drand48();
		outdata[i]=-1.0;
	}


	// Enumeration sort
	ttime=timer();
	uint64_t work_per_thread = 1+(len-1)/NUM_THREADS;
	for(unsigned j = 0; j < NUM_THREADS; j++){
		uint64_t arg = (j*work_per_thread)+(((j+1)*work_per_thread) << 32);
		pthread_create(&threads[j], NULL, find_ranks, (void* )arg);
	}
	for(unsigned j = 0; j < NUM_THREADS; j++){
		pthread_join(threads[j], NULL);
	}
	// for (j=0;j<len;j+=NUM_THREADS)
	// {
	// 	for(t=0; t<NUM_THREADS; t++) {
	// 		el=j+t;
	// 		pthread_create(&threads[t], &attr, findrank, (void *)el);
	// 	}
	//
	// 	for(t=0; t<NUM_THREADS; t++)
	// 	pthread_join(threads[t], &status);
	// }
	ttime=timer()-ttime;
	printf("Time: %f %d\n",ttime/1000000.0,NUM_THREADS);

	// Check results, -1 implies data same as the previous element
	for (i=0; i<len-1; i++)
	if (outdata[i]>outdata[i+1] && outdata[i+1]>-1)
	printf("ERROR: %f,%f\n", outdata[i],outdata[i+1]);

	return 0;
}

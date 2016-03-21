/* PINGPONG */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void processor_A(void);
void processor_B(void);

double buff[100000] ={0};

int main(int argc, char *argv[]) {
  int rank, size;

  MPI_Init(&argc, &argv);               /* Initialize MPI               */
  MPI_Comm_size(MPI_COMM_WORLD, &size); /* Get the number of processors */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* Get my number                */

  if (size != 2) { /* This if-block makes sure only two processors
                    * takes part in the execution of the code, pay no
                    * attention to it */
    if (rank == 0)
      fprintf(stdout, "\aRun on two processors only!\n");
  }
  else {
    MPI_Buffer_attach(buff, 100000*sizeof(double));
    if ( rank == 0)
      processor_A();
    else
      processor_B();
  }
  MPI_Finalize();
  return 0;
}

void processor_A(void){

  double message[100000];
  double timer1;
  const int ping=101, pong=102;
  MPI_Status status;

  for (int len=1000; len<=8000; len+=1000) {
     //fprintf(stdout, "A] Starting on %d\n", len);
     timer1=MPI_Wtime();

     for (int i=1; i<=100; i++) {
       //printf("A sending %d\n", i);
       if(MPI_SUCCESS != MPI_Ssend(message, len, MPI_DOUBLE, 1, ping+i+len, MPI_COMM_WORLD)){
         puts("A ERROR SSEND");
         return;
       }
       //printf("A recieving %d\n", i);
       if(MPI_SUCCESS != MPI_Recv(message, len, MPI_DOUBLE, 1, pong+i+len, MPI_COMM_WORLD, &status)){
         puts("A ERROR RECV");
         return;
       }
     }

     timer1=MPI_Wtime()-timer1;
     fprintf(stdout,"%d %.6f %f \n",len,timer1/200.0,2.0*8.0*100*len/timer1);
  }
}

void processor_B(void){

  double message[100000];
  const int ping=101, pong=102;
  MPI_Status status;
  int number_amount;
  for (int len=1000; len<=8000; len+=1000){
     //fprintf(stdout, "B] Starting on %d\n", len);
     for (int i=1; i<=100; i++) {
       //printf("B recieving %d\n",i);
       if(MPI_SUCCESS != MPI_Recv(message, len, MPI_DOUBLE, 0, ping+i+len, MPI_COMM_WORLD, &status)){
         puts("ERRORS!");
         return;
       }
       if(MPI_SUCCESS != MPI_Ssend(message, len, MPI_DOUBLE, 0, pong+i+len, MPI_COMM_WORLD)){
         puts("ERRORS!");
         return;
       }
     }
   }
}

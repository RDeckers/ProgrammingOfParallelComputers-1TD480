/**********************************************************************
 * Point-to-point communication using MPI
 *
 **********************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int rank, size;
  double a, b;
  MPI_Status status;

  MPI_Init(&argc, &argv);               /* Initialize MPI               */
  MPI_Comm_size(MPI_COMM_WORLD, &size); /* Get the number of processors */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* Get my number                */

  a = 100.0 + (double) rank;  /* Different a on different processors */


  MPI_Request send_request, recv_request;
  if(rank < 2){
    int other_rank = rank^1;
    printf("My rank is %d, sending to %d\n", rank, rank^1);
    MPI_Isend(&a, 1, MPI_DOUBLE, /*destination*/other_rank, /*tag*/rank, MPI_COMM_WORLD, &send_request);
    MPI_Irecv(&b, 1, MPI_DOUBLE, /*origin*/other_rank, /*tag*/other_rank, MPI_COMM_WORLD, &recv_request);
    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
    printf("Processor %d got %f from processor %d\n", rank, b, other_rank);
    MPI_Wait(&send_request, MPI_STATUS_IGNORE);
  }

  MPI_Finalize();

  return 0;
}

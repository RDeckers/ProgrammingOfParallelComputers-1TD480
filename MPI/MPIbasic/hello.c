/**********************************************************************
 * A simple "hello world" program for MPI/C
 *
 **********************************************************************/

#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

  MPI_Init(&argc, &argv);               /* Initialize MPI               */
  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("Hello from %d/%d\n", rank, size);             /* Print a message              */
  MPI_Finalize();                       /* Shut down and clean up MPI   */

  return 0;
}

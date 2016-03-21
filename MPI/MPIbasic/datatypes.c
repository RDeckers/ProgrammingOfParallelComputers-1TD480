/**********************************************************************
 * Derived datatypes in MPI/C
 *
 **********************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int rank, size,Ny,Nx,y,x,count,blocklen,stride;
  double *A;
  MPI_Status status;
  MPI_Datatype newtype;

  MPI_Init(&argc, &argv);               /* Initialize MPI               */
  MPI_Comm_size(MPI_COMM_WORLD, &size); /* Get the number of processors */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); /* Get my number                */

  Ny=8; Nx=16;
  A=(double *)calloc(Ny*Nx,sizeof(double));
  if (rank==0){
    printf("Matrix A on proc 0\n");
    for (y=0; y<Ny;y++){
      for (x=0; x<Nx;x++){
	      A[y*Nx+x]=(double)x+100*y+1000;
        printf("%d ", (int)A[y*Nx+x]);
      }
      printf("\n");
    }
    printf("\n");
  }

  count=Ny; blocklen=1; stride=Nx;
  MPI_Type_vector(count,blocklen,stride,MPI_DOUBLE,&newtype);
  MPI_Type_commit(&newtype);

  MPI_Datatype block_type;
  count=Ny/2; blocklen=Nx/2; stride=Nx;
  MPI_Type_vector(count,blocklen,stride,MPI_DOUBLE,&block_type);
  MPI_Type_commit(&block_type);

  /* Send last column, notice the message length = 1 ! */
  if (rank == 0) {
    MPI_Send(&A[Nx-1], 1, newtype, 1, 111, MPI_COMM_WORLD);
    MPI_Send(&A[Nx*Ny/2+Nx/2], 1, block_type, 1, 111, MPI_COMM_WORLD);
  } else if (rank==1) {
    MPI_Recv(&A[0], 1, newtype, 0, 111, MPI_COMM_WORLD, &status);
    MPI_Recv(&A[Nx*Ny/2], 1, block_type, 0, 111, MPI_COMM_WORLD, &status);
    printf("Matrix A on proc 1\n");
    for (y=0; y<Ny; y++)
    {
       for (x=0; x<Nx; x++)
          printf("%4d ", (int)A[y*Nx+x]);
       printf("\n");
    }
  }

  free(A);
  MPI_Type_free(&newtype);
  MPI_Finalize();

  return 0;
}

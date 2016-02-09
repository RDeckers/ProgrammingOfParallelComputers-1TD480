#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <matrix.h>

int main(int argc, char **argv){
  /* Initialize MPI  */
  //MPI_Init(&argc, &argv);
  int p;
  //MPI_Comm_size(MPI_COMM_WORLD,&p);
  MPI_Comm proc_grid, proc_row, proc_col;
  int coords[2],pos[2],reorder=1,ndim=2,dims[2]={0,0},periods[2]={0,0};

  int N = -1;
  for(int i = 0; i < argc-1; i++){
    if(!strcmp(argv[i], "-n")){
      N = atoi(argv[i+1]);
    }
  }

  if(0 >= N){
    puts("Please specify a valid matrix size using -n MATRIX_SIZE");
    return -1;
  }

  printf("using N = %d\n", N);
  double *A, *B, *C;
  new_random_matrix(&A, N);
  new_random_matrix(&B, N);
  new_matrix(&C, N);
  // print_matrix(A, N);
  // puts("*");
  // print_matrix(B, N);
  // MxM_square_scalar(A,B,C,N);
  // puts("=");
  // print_matrix(C,N);
}

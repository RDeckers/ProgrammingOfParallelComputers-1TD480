#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <matrix.h>

int main(int argc, char **argv){
  /* Initialize MPI  */
  MPI_Init(&argc, &argv);
  int n_proccessors, grid_dimensions[2]={0,0}, current_proccessor;
  MPI_Comm comm_grid, comm_row, comm_column;
  MPI_Comm_size(MPI_COMM_WORLD,&n_proccessors);

  /* Create a virtual 2D-grid topology */
  MPI_Dims_create(n_proccessors, 2, grid_dimensions);
  MPI_Cart_create(MPI_COMM_WORLD,2,grid_dimensions, (int[]){1,1}, 1, &comm_grid);
  MPI_Comm_rank(comm_grid,&current_proccessor);

  /* get size of the matrix */
  int N = -1; //size of matrix
  for(int i = 0; i < argc-1; i++){
    if(!strcmp(argv[i], "-n")){
      N = atoi(argv[i+1]);
    }
  }
  if(0 >= N){
    if(0 == current_proccessor)
      puts("Please specify a valid matrix size using -n MATRIX_SIZE");
    MPI_Comm_free(&comm_grid);
    MPI_Finalize();
    return 0;
  }

  int grid_coords[2];
  MPI_Cart_coords(comm_grid,current_proccessor,2, grid_coords);
  /* Create a communicator for each row */
  int row_rank, column_rank;
  MPI_Comm_split(comm_grid, grid_coords[0],grid_coords[1],&comm_row);
  MPI_Comm_rank(comm_row,&row_rank); //TODO: Doesn't ^^^^^ already take a rank as argument?

  /* Create a communicator for each column */
  MPI_Comm_split(comm_grid, grid_coords[1],grid_coords[0],&comm_column);
  MPI_Comm_rank(comm_column,&column_rank); //TODO: Doesn't ^^^^^ already take a rank as argument?

  if(0 == current_proccessor)
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
  MPI_Comm_free(&comm_grid);
  MPI_Comm_free(&comm_row);
  MPI_Comm_free(&comm_column);
  MPI_Finalize();
  return 0;
}

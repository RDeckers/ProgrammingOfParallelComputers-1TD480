#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <matrix.h>

void print_flat_matrix(double *A, unsigned n){
  int p;
  MPI_Comm_rank(MPI_COMM_WORLD,&p);
  printf("%d [",p);
  for(unsigned u = 0; u < n*n; u++){
    printf("%3.0f ", A[u]);
  }
  puts("]");
}

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
  if(grid_dimensions[0] != grid_dimensions[1]){
    if(0 == current_proccessor){
      puts("Grid non-square! please specify a square number of processors.");
    }
    MPI_Comm_free(&comm_grid);
    MPI_Finalize();
    return 0;
  }
  if(0 == current_proccessor){
    printf("Using a %d x %d grid.\n", grid_dimensions[0], grid_dimensions[1]);
  }
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
  if(0 == current_proccessor)
    printf("using N = %d\n", N);
  //We need to send block-matrices, of dimension N/sqrt(p) x N/sqrt(p)
  int block_matrix_size = N / grid_dimensions[0];
  if(N % grid_dimensions[0]){
    if(0 == current_proccessor)
      puts("Invalid N & p: sqrt(p) does not evenly divide N.");
    MPI_Comm_free(&comm_grid);
    MPI_Finalize();
    return 0;
  }
  if(0 == current_proccessor)
    printf("using N_block = %d\n", block_matrix_size);

  int grid_coords[2];
  MPI_Cart_coords(comm_grid,current_proccessor,2, grid_coords);
  /* Create a communicator for each row */
  int row_rank, column_rank;
  MPI_Comm_split(comm_grid, grid_coords[0],grid_coords[1],&comm_row);
  MPI_Comm_rank(comm_row,&row_rank); //TODO: Doesn't ^^^^^ already take a rank as argument?

  /* Create a communicator for each column */
  MPI_Comm_split(comm_grid, grid_coords[1],grid_coords[0],&comm_column);
  MPI_Comm_rank(comm_column,&column_rank); //TODO: Doesn't ^^^^^ already take a rank as argument?


  double *A, *B, *C;
  double *block_A, *block_B, *block_C;
  if(0 == current_proccessor){
    new_random_matrix(&A, N);
    new_random_matrix(&B, N);
    new_matrix(&C, N);
  }

  new_matrix(&block_A, block_matrix_size);
  new_matrix(&block_B, block_matrix_size);
  new_matrix(&block_C, block_matrix_size);

  MPI_Datatype block_matrix, block_matrix_t;
  //MPI_Type_vector(block_matrix_size, block_matrix_size, N, MPI_DOUBLE, &block_matrix_t);

  MPI_Type_create_subarray(
    2, //2 dimensional grid
    (int[]){N,N}, //of N x N
    (int[]){block_matrix_size,block_matrix_size}, //with blocks of this
    (int[]){0,0}, //Start counting at element (0,0)
    MPI_ORDER_C, //we're using C
    MPI_DOUBLE, //array of doubles
    &block_matrix //store here.
  );
  MPI_Type_commit(&block_matrix);// <--- fuck this shit
  MPI_Type_create_resized(block_matrix, 0, sizeof(double), &block_matrix_t);

  if(0 == current_proccessor)
   print_matrix(A, N);

  int displacements_B[n_proccessors];
  int element_counts_B[n_proccessors];
  if(0 == current_proccessor)
    for(int y = 0; y < grid_dimensions[1]; y++)
      for(int x = 0; x < grid_dimensions[0]; x++){
        int p = x+y*grid_dimensions[0];
        element_counts_B[p] = 1;
        displacements_B[p] = x*block_matrix_size+y*N*block_matrix_size;
      }
  MPI_Scatterv(//send a block-element of B to each processor.
    B, element_counts_B, displacements_B, block_matrix_t,
    block_B, block_matrix_size*block_matrix_size, MPI_DOUBLE,
    0, comm_grid
  );
  for(int stage = 0; stage < grid_dimensions[0]; stage++){
    //hand-out new off-diagonals of A from the root.
    int displacements[grid_dimensions[0]];
    int element_counts[grid_dimensions[0]];
    if(0 == current_proccessor)
      for(int i =0; i < grid_dimensions[0]; i++){
        displacements[i] = (N*block_matrix_size)*i+(((i+stage)*block_matrix_size)%N);
        element_counts[i] = 1;
      }
    if(0 == row_rank){//Send Diagonals of A to the rows.
      MPI_Scatterv(
        A, element_counts, displacements, block_matrix_t,//sending args
        block_A, block_matrix_size*block_matrix_size, MPI_DOUBLE, //recieving args
        0, comm_column
      );
    }
    //broadcast the (off-)diagonals to each element in the row
    MPI_Bcast(block_A, block_matrix_size*block_matrix_size, MPI_DOUBLE,0, comm_row);

    MPI_Request send_req;
    int sender = (column_rank-1) >= 0 ? column_rank-1 : grid_dimensions[0]-1;
    int reciever =(column_rank+1)%grid_dimensions[1];
    //print_flat_matrix(block_B, block_matrix_size);
    MPI_Isend(block_B, block_matrix_size*block_matrix_size, MPI_DOUBLE, reciever, stage, comm_column, &send_req);
    MxM_fma_square_scalar(block_A, block_B, block_C, block_matrix_size);
    MPI_Wait(&send_req, MPI_STATUS_IGNORE);
    MPI_Recv(block_B, block_matrix_size*block_matrix_size, MPI_DOUBLE, sender, stage, comm_column, NULL);
      // c += a*b
  }
  print_flat_matrix(block_C, block_matrix_size);
  MPI_Gatherv(
    block_C, block_matrix_size*block_matrix_size, MPI_DOUBLE,
    C, element_counts_B, displacements_B, block_matrix_t,
    0, comm_grid
  );
  if(0 == current_proccessor){
    print_matrix(C,N);
  }
  //print_flat_matrix(block_C, block_matrix_size);
  //gather all block_C elements into C.

  MPI_Comm_free(&comm_grid);
  MPI_Type_free(&block_matrix);
  MPI_Type_free(&block_matrix_t);
  MPI_Comm_free(&comm_row);
  MPI_Comm_free(&comm_column);
  MPI_Finalize();
  return 0;
}

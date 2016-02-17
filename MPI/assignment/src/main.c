#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <matrix.h>
#include <time.h>
#include <float.h>

//Used to compute the difference in time (in ns).
double time_diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp.tv_sec*1.0e9+temp.tv_nsec;
}

//prints the matrix as a flat array, used for debugging.
void print_flat_matrix(double *A, unsigned n){
	int p;
	MPI_Comm_rank(MPI_COMM_WORLD,&p);
	printf("%d [",p);
	for(unsigned u = 0; u < n*n; u++){
		printf("%3.0f ", A[u]);
	}
	puts("]");
}

//how many times we the code to get a better estimate of the runtime.
#define N_RUNS 5

int main(int argc, char **argv){

	//Lowest and average run-time for N_RUNS
	double lowest_time = DBL_MAX, average_time = 0;
	struct timespec T0, T1;
	int N, n_proccessors, current_proccessor;
	//Don't include the init in the loop.
	MPI_Init(&argc, &argv);
	//loop N_RUNS times
	for(int X = 0; X < N_RUNS; X++){
		clock_gettime(CLOCK_MONOTONIC, &T0);
		int grid_dimensions[2]={0,0};
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

		/* get size of the matrix */
		N = -1; //size of matrix
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

		//We need to send block-matrices, of dimension N/sqrt(p) x N/sqrt(p)
		int block_matrix_size = N / grid_dimensions[0];
		if(N % grid_dimensions[0]){
			if(0 == current_proccessor)
			puts("Invalid N & p: sqrt(p) does not evenly divide N.");
			MPI_Comm_free(&comm_grid);
			MPI_Finalize();
			return 0;
		}

		int grid_coords[2];
		MPI_Cart_coords(comm_grid,current_proccessor,2, grid_coords);
		int row_rank, column_rank;

		/* Create a communicator for each row */
		MPI_Comm_split(comm_grid, grid_coords[0],grid_coords[1],&comm_row);
		MPI_Comm_rank(comm_row,&row_rank); //TODO: Doesn't ^^^^^ already take a rank as argument?

		/* Create a communicator for each column */
		MPI_Comm_split(comm_grid, grid_coords[1],grid_coords[0],&comm_column);
		MPI_Comm_rank(comm_column,&column_rank); //TODO: Doesn't ^^^^^ already take a rank as argument?

		//the whole matrices, only really used
		double *A, *B, *C;
		//have two blocks for A and B, so we can compute with one while we
		// write the next block to the other.
		double *block_A[2], *block_B[2], *block_C;
		//Also have an addition block matrix for storing the part of A one should broadcast in the correct stage.
		double *my_A;
		//which block of A and B is active at the time
		int active_block = 0;

		//proccess 0 will create space to hold the full matrices A, B and C
		if(0 == current_proccessor){
			new_random_matrix(&A, N);
			new_random_matrix(&B, N);
			new_matrix(&C, N);
		}
		//All others only allocate space for their local blocks (but so does 0).
		new_matrix(&my_A, block_matrix_size);
		new_matrix(&block_A[0], block_matrix_size);
		new_matrix(&block_A[1], block_matrix_size);
		new_matrix(&block_B[0], block_matrix_size);
		new_matrix(&block_B[1], block_matrix_size);
		new_matrix(&block_C, block_matrix_size);

		MPI_Datatype block_matrix_t;

		//create a block matrix type.
		MPI_Type_create_subarray(
			2, //2 dimensional grid
			(int[]){N,N}, //of N x N
			(int[]){block_matrix_size,block_matrix_size}, //with blocks of this
			(int[]){0,0}, //Start counting at element (0,0)
			MPI_ORDER_C, //we're using C
			MPI_DOUBLE, //array of doubles
			&block_matrix_t //store here.
		);
		MPI_Type_commit(&block_matrix_t);
		//redefine the size of the block_matrix type to be the size of 1 double to make it easy/possible to specify the starting point
		MPI_Type_create_resized(block_matrix_t, 0, sizeof(double), &block_matrix_t);

		int displacements[n_proccessors];
		int element_counts[n_proccessors];
		if(0 == current_proccessor)//only the first processor has to bother doing this
		for(int y = 0; y < grid_dimensions[1]; y++)
		for(int x = 0; x < grid_dimensions[0]; x++){
			int p = x+y*grid_dimensions[0];
			element_counts[p] = 1; //we always only copy 1 block matrix.
			displacements[p] = x*block_matrix_size+y*N*block_matrix_size;//And each block starts at these coordinates.
		}
		MPI_Scatterv(//send a block-element of B to each processor.
			B, element_counts, displacements, block_matrix_t,
			block_B[active_block], block_matrix_size*block_matrix_size, MPI_DOUBLE,
			0, comm_grid
		);
		//send a block-element of A to each processor.
		//Alternatively we could scatter in the column communicator at each stage, this would use a smaller comm (column)
		//and possibly a more optimized route depending on the topology of the network.
		//However, there is no non-blocking scatter, let alone scatterv to our knowledge.
		//By doing the scatter ahead of time, we can use Ibcast instead which could be useful given that
		//Matrix multiplication scales as ~O(n^3) and bandwith usage as O(p*n^2) and thus for large n we should
		//always be able to finish communication before the computations are finished.
		MPI_Scatterv(
			A, element_counts, displacements, block_matrix_t,
			my_A, block_matrix_size*block_matrix_size, MPI_DOUBLE,
			0, comm_grid
		);

		if(row_rank == column_rank){//if we are on the diagonal
			free(block_A[active_block]); //this could be optimized out by not allocating in the first place.
			block_A[active_block] = my_A;//flag "our" A as the active one so we can broadcast it to the rest.
		}
		//now the diagonals broadcast their A blocks to all.
		MPI_Bcast(block_A[active_block], block_matrix_size*block_matrix_size, MPI_DOUBLE, column_rank, comm_row);
		//loop over all stages, save the last one. As we don't need to
		//communicate in that one.
		for(int stage = 1; stage < grid_dimensions[0]; stage++){
			MPI_Request requests[3];
			//broadcast the (off-)diagonals to each element in the row to the other blocks in the A_block buffer.
			int off_diagonal_rank = (column_rank+stage)%grid_dimensions[0];
			if(row_rank == off_diagonal_rank){//if we are on the next off diagonal.
				free(block_A[active_block^1]);//NOTE: "^" is the XOR operator in C: 0^1 = 1, 1^1 = 0.
				block_A[active_block^1] = my_A;
			}
			MPI_Ibcast(block_A[active_block^1], block_matrix_size*block_matrix_size, MPI_DOUBLE, off_diagonal_rank, comm_row, requests+2);

			int reciever = (column_rank-1) >= 0 ? column_rank-1 : grid_dimensions[0]-1;
			int sender =(column_rank+1)%grid_dimensions[1];
			//start sending our active block of B and recieve it in the inactive buffer.
			MPI_Isend(block_B[active_block], block_matrix_size*block_matrix_size, MPI_DOUBLE, reciever, stage, comm_column, requests);
			MPI_Irecv(block_B[active_block^1], block_matrix_size*block_matrix_size, MPI_DOUBLE, sender, stage, comm_column, requests+1);
			// c += a*b
			MxM_fma_square_scalar(block_A[active_block], block_B[active_block], block_C, block_matrix_size);
			MPI_Waitall(3, requests, MPI_STATUSES_IGNORE);
			//flip the active and inactive buffers.
			active_block ^= 1;
		}
		//perform the final computation.
		MxM_fma_square_scalar(block_A[active_block], block_B[active_block], block_C, block_matrix_size);
		free(block_A[active_block]);
		free(block_B[active_block]);
		free(block_A[active_block^1]);
		free(block_B[active_block^1]);
		MPI_Gatherv(
			block_C, block_matrix_size*block_matrix_size, MPI_DOUBLE,
			C, element_counts, displacements, block_matrix_t,
			0, comm_grid
		);
		free(block_C);

		MPI_Comm_free(&comm_grid);
		//MPI_Type_free(&block_matrix);
		MPI_Type_free(&block_matrix_t);
		MPI_Comm_free(&comm_row);
		MPI_Comm_free(&comm_column);
		clock_gettime(CLOCK_MONOTONIC, &T1);
		if(0 == current_proccessor){
			double time_taken = time_diff(T0, T1);
			average_time += time_taken;
			if(time_taken < lowest_time){
				lowest_time = time_taken;
			}
		}
	}
	MPI_Finalize();
	if(0 == current_proccessor){
		printf("%d %d %e %e\n", N, n_proccessors, lowest_time, average_time/N_RUNS);
	}
	return 0;
}

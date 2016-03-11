#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
/*
  our parallel implementation is faster from about n = 100 t0 n = ~750 and then from 1000 onwards it rises again.
  I believe this is due to some cache/sharing effects as there is also a jump in the serial runtime after 1k.
  The maximum speedup is 4, which is in line with the expected maximum as my laptop has 4 floating point units.
  
  speedup_1 *******speedup_2 #######speedup_3 $$$$$$$

4 +-+-------+---------+---------+--------+---------+---------+-------+-+
+      ***+    ##   $         +        +         +         +         +
|      *  #    # *** $  **                                           |
3.5 +-+    * #**  # * #$****# *                                        +-+
|     * #  #****$ $# #$ $$*                                          |
3 +-+   *#   #$#   $  # $$ #$*  $                                    +-+
|     *#   $##            #* $ $                                     |
|     *#   $#             #*$  $                          $$$$$      |
2.5 +-+   *   $                #*   $                        $         +-+
|     *   $                #*    $                      $            |
2 +-+  *#  $                  *     $                     $          +-+
|    *#  $                  #*    $                    $             |
|    *# $                    *     $            **    $              |
1.5 +-+ *#  $                    *     $           *     $             +-+
|   *# $                      ***** $        **      $               |
1 +-+* #$$                      *#   **#   ****       $              +-+
|  *#                           #### ****##       $$                 |
+ *$      +         +         +      $$##$$$$$$ $$         +         +
0.5 +-$-------+---------+---------+--------+-------$-+---------+-------+-+
0        200       400       600      800       1000      1200      1400

(see matmul.png and matmul.dat for a clearer overview)
*/

double **A,**B,**C;
int n;

void* compute_C_columns(void* arg){
  uint64_t fused = (uint64_t) arg;
  uint32_t col_start = fused, col_end= (fused >> 32) > n? n : (fused >> 32);
  for(unsigned rC = 0; rC < n; rC++){
    for(unsigned cC = col_start; cC < col_end; cC++){
      for(unsigned u = 0; u < n; u++)
      C[rC][cC] +=  A[rC][u]*B[u][cC];
    }
  }
  //printf("[%d, %d) done!\n", col_start, col_end);
}


int main(int argc, char *argv[]) {
  int i,j,k,time;
  double t;

  int n_max = atoi(argv[1]);

  //Allocate and fill matrices
  A = (double **)malloc(n_max*sizeof(double *));
  B = (double **)malloc(n_max*sizeof(double *));
  C = (double **)malloc(n_max*sizeof(double *));
  for(i=0;i<n_max;i++){
    A[i] = (double *)malloc(n_max*sizeof(double));
    B[i] = (double *)malloc(n_max*sizeof(double));
    C[i] = (double *)malloc(n_max*sizeof(double));
  }
  for(n = 50; n <= n_max; n+=50){
    for (i = 0; i<n; i++)
    for(j=0;j<n;j++){
      A[i][j] = rand() % 5 + 1;
      B[i][j] = rand() % 5 + 1;
      C[i][j] = 0.0;
    }

    printf("%d ", n);
    time=timer();
    // Multiply C=A*B
    for(i=0;i<n;i++)
    for (j=0;j<n;j++)
    for (k=0;k<n;k++)
    C[i][j]+=A[i][k]*B[k][j];
    time=timer()-time;
    double time_serial = time/1000000.0;
    printf("%e ",time_serial);

    time=timer();
    const size_t num_threads = 8;
    pthread_t threads[num_threads];
    uint64_t work_per_thread = 1+(n-1)/num_threads;
    for(unsigned j = 0; j < num_threads; j++){
      uint64_t arg = (j*work_per_thread)+(((j+1)*work_per_thread) << 32);
      pthread_create(&threads[j], NULL, compute_C_columns, (void* )arg);
    }
    for(unsigned j = 0; j < num_threads; j++){
      pthread_join(threads[j], NULL);
    }
    time=timer()-time;
    double time_parallel = time/1000000.0;
    double speedup = time_serial/time_parallel;
    printf("%e %e\n",time_parallel, speedup);
  }
  return 0;

}

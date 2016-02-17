#!/bin/bash
for N in 120 480 840 1200; do
 echo starting on $N
 for SQRT_P in 1 2 3 4 5 6 8 10 12 15; do
   P=$[$SQRT_P*$SQRT_P];
   echo starting on $N-$P
   mpirun -machinefile ./nodes -n $P ./bin/main -n $N |tee -a timings_local_$N.dat
 done
done

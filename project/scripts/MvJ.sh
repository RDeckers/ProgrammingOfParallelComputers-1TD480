#!/bin/bash

PROJECT_PATH=$( cd $(dirname $0) ; pwd -P )/..
OUT_FILE=$PROJECT_PATH/data/MvJ.dat
LOOPS=12
S=7
echo -n "" > $OUT_FILE
for j in $(seq 0.0 0.05 1.0)
do
 $PROJECT_PATH/bin/naive_rng.exe -s $S -l $LOOPS -J $j >> $OUT_FILE
done
echo -e "\n" >> $OUT_FILE
for j in $(seq 0.0 0.05 1.0)
do
 $PROJECT_PATH/bin/naive_copy.exe -s $S -l $LOOPS -J $j >> $OUT_FILE
done

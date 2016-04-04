#!/bin/bash

PROJECT_PATH=$( cd $(dirname $0) ; pwd -P )/..
OUT_FILE=$PROJECT_PATH/data/timing.dat
LOOPS=11
echo -n "" > $OUT_FILE
for S in {5..9}
do
 $PROJECT_PATH/bin/naive_copy.exe -s $S -l $LOOPS >> $OUT_FILE
done
echo -e "\n" >> $OUT_FILE
for S in {5..12}
do
 $PROJECT_PATH/bin/naive_rng.exe -s $S -l $LOOPS >> $OUT_FILE
done
echo -e "\n" >> $OUT_FILE
for S in {8..14}
do
 $PROJECT_PATH/bin/optimized.exe -s $S -l $LOOPS >> $OUT_FILE
done
echo -e "\n" >> $OUT_FILE
for S in {8..14}
do
 $PROJECT_PATH/bin/optimized.exe -k 4 -s $S -l $LOOPS >> $OUT_FILE
done
echo -e "\n" >> $OUT_FILE
for S in {8..14}
do
 $PROJECT_PATH/bin/optimized.exe -k 8 -s $S -l $LOOPS >> $OUT_FILE
done

#!/bin/bash

PROJECT_PATH=$( cd $(dirname $0) ; pwd -P )/..
OUT_FILE=$PROJECT_PATH/data/naive_rng_timing.dat
LOOPS=13
echo -n "" > $OUT_FILE
for S in {4..12}
do
 $PROJECT_PATH/bin/naive_rng.exe -s $S -l $LOOPS --transfers-only >> $OUT_FILE
done
echo -e "\n" >> $OUT_FILE
for S in {4..12}
do
 $PROJECT_PATH/bin/naive_rng.exe -s $S -l $LOOPS >> $OUT_FILE
done

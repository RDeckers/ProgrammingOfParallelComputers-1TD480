#!/bin/bash

PROJECT_PATH=$( cd $(dirname $0) ; pwd -P )/..
OUT_FILE=$PROJECT_PATH/data/optimized_timing.dat
LOOPS=13
echo -n "" > $OUT_FILE
for S in {4..12}
do
 $PROJECT_PATH/bin/optimized.exe -s $S -l $LOOPS >> $OUT_FILE
done

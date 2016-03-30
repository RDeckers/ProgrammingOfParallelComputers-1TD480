#!/bin/bash

PROJECT_PATH=$( cd $(dirname $0) ; pwd -P )/..
OUT_FILE=$PROJECT_PATH/data/naive_copy_timing.dat
LOOPS=12
echo -n "" > $OUT_FILE
for S in {4..10}
do
 $PROJECT_PATH/bin/naive_copy.exe -s $S -l $LOOPS --transfers-only >> $OUT_FILE
done
echo -e "\n" >> $OUT_FILE
for S in {4..10}
do
 $PROJECT_PATH/bin/naive_copy.exe -s $S -l $LOOPS >> $OUT_FILE
done

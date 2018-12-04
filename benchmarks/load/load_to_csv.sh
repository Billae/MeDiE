#!/bin/bash

LOG_FILES=$(ls ~/mesure_perf/server/load*)
RES_FILE=servers_load.csv

echo ${LOG_FILES} | tr ' ' ';' > $RES_FILE
paste -d ';' $LOG_FILES >> $RES_FILE

python ./scripts/load_to_percentage.py $RES_FILE

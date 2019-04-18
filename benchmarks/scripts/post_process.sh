#!/bin/bash

#This script process brut data to generate csv file readable by all display_* scripts
#It take in argument the path to the data folder (>server+client+temps)
module load python3
module load gnuplot

if ([ -z $1 ] || [ -z $2 ])
then
    echo "please give a path and the type of run"
    exit 1 
fi

path=$1
run=$2

LOG_FILES=$(ls $path/server/load*)
RES_FILE=$path/servers_load.csv

echo ${LOG_FILES} | tr ' ' ';' > $RES_FILE
paste -d ';' $LOG_FILES >> $RES_FILE

python3 scripts/server_post_process.py $path $RES_FILE
python3 scripts/percentage_to_error.py $path

gnuplot -c scripts/display_load.gnu $path $run
gnuplot -c scripts/display_percentage_load.gnu $path $run

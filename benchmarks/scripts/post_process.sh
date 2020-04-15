#!/bin/bash

#This script process brut data to generate csv file readable by all display_* $SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts
#It take in argument the path to the data folder (>server+client+temps)
module load python3
module load gnuplot

if ([ -z $1 ] || [ -z $2 ] || [ -z $3 ])
then
    echo "please give a path, the type of run, the traces prefix and the number of step"
    exit 1 
fi

path=$1
run=$2
traces=$3
n_step=$4
LOG_FILES=$(ls $path/server/load*)
RES_FILE=$path/servers_load.csv

rm $RES_FILE
echo ${LOG_FILES} | tr ' ' ';' > $RES_FILE
paste -d ';' $LOG_FILES >> $RES_FILE

#sum of each line
awk -F";" '{for(i=1;i<=NF;i++) t+=$i; print t; t=0}' $RES_FILE > $path/load_total.csv

#if ([ $run == "dh" ])
#then
    #8 is the number of line to delete because of the run script ends after rebalancing
    #sed -i -e :a -e '$d;N;2,8ba' -e 'P;D' $RES_FILE
#fi

python3 $SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/server_post_process.py $path $RES_FILE
python3 $SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/percentage_to_error.py $path
python $SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/client_post_process.py $path $traces $n_step

gnuplot -c $SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/display_load.gnu $path $run
gnuplot -c $SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/display_total_load.gnu $path $run
gnuplot -c $SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/display_req_time.gnu $path $run
gnuplot -c $SCRATCHDIR/prototype_MDS_scratch/benchmarks/scripts/display_percentage_load.gnu $path $run

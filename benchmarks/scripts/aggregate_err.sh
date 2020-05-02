#!/bin/bash


if [[ ! -d $1 ]]
then
    echo "Please give a valid result dir"
    exit
fi

path=$1

header="alpha;N_entry;redistribution_interval;nb_redistribution;cost;average_deviation;max_deviation;score_av;score_max"

echo "$header" > $path/deviation_recap_all.csv
find $path -name deviation_recap.txt |xargs -n 1 tail -n1 >> $path/deviation_recap_all.csv



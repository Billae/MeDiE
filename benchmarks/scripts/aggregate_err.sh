#!/bin/bash


if [[ ! -d $1 ]]
then
    echo "Please give a valid result dir"
    exit
fi

path=$1

#aggregate deviation files
header="alpha;N_entry;redistribution_interval;redistribution_useless;redistribution_useful;score"

echo "$header" > $path/deviation_recap_all.csv
find $path -name deviation_recap.txt |xargs -n 1 tail -n1 >> $path/deviation_recap_all.csv

#aggregate rebalancing files


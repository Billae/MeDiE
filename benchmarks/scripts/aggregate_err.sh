#!/bin/bash


if [[ ! -d $1 ]] || [[ ! $# -eq 2 ]]
then
    echo "Please give a valid result dir and a run type"
    exit
fi

path=$1
type=$2
#aggregate deviation files

if [[ $type == "dh" ]]
then
    header="alpha;N_entry;redistribution_interval;redistribution_useless;redistribution_useful;redistribution_needed;deviation_av;deviation_max;global_max_deviation;cost;global_max_cost"
elif [[ $type == "sh" ]]
then
    header="deviation_av;deviation_max"
elif [[ $type == "indedh" ]]
then
    header="percent;N_entry;redistribution_interval;redistribution_useless;redistribution_useful;redistribution_needed;deviation_av;deviation_max;cost;max_cost"
elif [[ $type == "windowed" ]]
then
    header="alpha;size;redistribution_interval;redistribution_useless;redistribution_useful;redistribution_needed;deviation_av;deviation_max;cost;max_cost"
fi


echo "$header" > $path/deviation_recap_all.csv
find $path -name deviation_recap.txt |xargs -n 1 tail -n1 >> $path/deviation_recap_all.csv

#aggregate rebalancing files


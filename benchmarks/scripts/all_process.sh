#!/bin/bash

if [[ ! -d $1 ]]
then
    echo "Please give a valid result dir"
    exit
fi

result_dir=$1
for run in $(find $result_dir -name "post.sh")
do
    echo $run
    $run &>> log_all_process &
done
wait

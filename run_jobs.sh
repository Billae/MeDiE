#!/bin/bash

#this script has to be launched with the nohup option !

if [ -d $1 ]
then
    echo "Please give a valid job dir"
    exit
fi

job_dir=$1

#launch all jobs in the job list run_jobxx
for job in $(ls $job_dir)
do
    $job &> $file.log
done

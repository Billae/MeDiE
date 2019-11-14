#! /bin/bash

if [ -z $1 ] || [ -z $2 ] || [ -z $3 ]
then
    echo "please give the file path, a number of steps and clients"
    exit 1
fi

file=$1
n_step=$2
n_client=$3

for((turn = 0; turn < n_step; turn++))
do
    python3 protocol_test/spread.py $file-$(($turn)).csv $n_client
done

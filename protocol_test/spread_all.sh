#! /bin/bash

if [ -z $1 ] || [ -z $2 ]
then
    echo "please give a number of steps and clients"
    exit 1
fi

n_step=$1
n_client=$2

for((turn = 0; turn < n_step; turn++))
do
    python3 protocol_test/spread.py $SCRATCHDIR/scratch_vm/traces/generated/traces-$(($turn)).csv $n_client
done

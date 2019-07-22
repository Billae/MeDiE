#!/bin/sh
#This script call different parts of the traces creation processus.
#To modify the tempoal part, please refer to the protocol_test/gen_temp_pattern.py script
#To modify the request part, please refer to the src/gen_collision_prefix.c program

module load python
#create temporal traces
python protocol_test/gen_temp_pattern.py 3600

size=`wc -l /ccc/scratch/cont001/ocre/billae/scratch_vm/traces/generated/generated_temp | cut -d" " -f1`
#name=`hostname| cut -d'.' -f1`

#create as many requests as in the temporal file
./bin/generator prefix 4 $size


path="/ccc/scratch/cont001/ocre/billae/scratch_vm/traces/generated/"

echo "timestamp,operation,key,jobid" > $path/traces.csv
#merge time and requests
paste -d, $path/generated_temp $path/colliding_id >> $path/traces.csv

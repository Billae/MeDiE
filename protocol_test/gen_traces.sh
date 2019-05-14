#!/bin/sh
#This script call different parts of the traces creation processus.
#To modify the tempoal part, please refer to the protocol_test/gen_temp_pattern.py script
#To modify the request part, please refer to the src/gen_collision_prefix.c program

#create temporal traces
python protocol_test/gen_temp_pattern.py 100

size=`wc -l etc/generated_temp | cut -d" " -f1`
name=`hostname| cut -d'.' -f1`

#create as many requests as in the temporal file
./bin/generator $name 4 $size

#merge time and requests
paste -d, etc/generated_temp etc/colliding_id > etc/traces.csv

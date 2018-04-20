#!/bin/bash

prefix="`hostname| cut -d'.' -f1`_"

if [ -z $1 ]
then
    echo "Please give a number of available servers"
else
    result=`./prototype_MDS/bin/client $1 $prefix`
    #create a result file
    echo $result >"/mnt/$prefix"
fi

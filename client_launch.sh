#!/bin/bash

prefix="`hostname| cut -d'.' -f1`"

if ([ -z $1 ] || [ -z $2 ] || [ -z $3 ])
then
    echo "Please give a number of available servers a number of line and a path"
else
    result=`./prototype_MDS/bin/client $1 $2 $3`
    #create a result file
    echo $result >>"/mnt/client/$prefix"
fi

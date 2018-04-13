#!/bin/bash

prefix="`hostname| cut -d'.' -f1`_"

if [ -z $1 ]
then
    echo "Please give a number of available servers"
else
    ./prototype_MDS/bin/client $1 $prefix
fi

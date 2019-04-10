#!/bin/bash

prefix="`hostname| cut -d'.' -f1`"

id_self=`hostname| cut -c 3-`

if ([ -z $1 ] || [ -z $2 ])
then
    echo "Please give a number of available servers and a path"
else
    path=$2-$(($id_self-$1)).csv
    n_line=`wc -l <$path`
    #arg1= nb_srv, arg2= nb_line in file, arg3= path of file
    result=`./prototype_MDS/bin/client $1 $n_line $path`
    #create a result file
    echo $result >>"/mnt/dh/client/$prefix"
fi

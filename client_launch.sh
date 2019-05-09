#!/bin/bash

prefix="`hostname| cut -d'.' -f1`"

id_self=`echo $prefix| cut -c 3-`

if ([ -z $1 ]  || [ -z $2 ]|| [ -z $3 ])
then
    echo "Please give a number of available servers, a input path and an output path"
else
    inputpath=$2-$(($id_self-$1)).csv
    outputpath=$3
    n_line=`wc -l <$inputpath`
    #arg1= nb_srv, arg2= nb_line in file, arg3= path of file
    result=`./prototype_MDS/bin/client $1 $n_line $inputpath`
    #create a result file
    echo $result >>"$outputpath/client/$prefix"
fi

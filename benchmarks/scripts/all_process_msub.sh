#!/bin/bash

if [[ ! -d $1 ]]
then
    echo "Please give a valid result dir"
    exit
fi

list_file=$(mktemp -p $CCCSCRATCHDIR)
echo "" > $list_file

result_dir=$1
for run in $(find $result_dir -name "post.sh")
do
    echo $run >> $list_file
done

msub_file=$(mktemp -p $CCCSCRATCHDIR)
cat << EOF > $msub_file
#MSUB -q sandy
#MSUB -n 331

module load glost
ccc_mprun glost_launch $list_file

EOF

ccc_msub $msub_file

#! /bin/bash

echo "warning you will modify all rebalancing file !  you don't want to do that, i know it"
exit 1

process_file(){
    new_content=""
    for content in $(cat $file |tr ' ' '\n')
    do
        new_content="${new_content}$(echo $content - 1 |bc)\n"
    done
    echo -e $new_content > $file
}



files=$(find $WORKDIR/results/ -name "rebalancing")

for file in $(echo $files |tr " " "\n")
do
    process_file $file
done

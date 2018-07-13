#!/bin/bash
# This script take a number of client and a datadir. Output is "scale_n.csv": it takes all line of mean_n_srv files for a particular number of client.
# it is a processing for the max_speed gnuplot script

PROGNAME=$(basename $0)
function USAGE() {
    echo "USAGE: ./${PROGNAME} N_CLIENTS DATADIR"
}

if [[ $# != 2 ]]; then
    echo "Error (${PROGNAME}): bad number of arguments!" 1>&2
    USAGE
    exit 1
fi

N_CLIENTS=$1
DATADIR=$2
OUTPUT_FILE="scale_${N_CLIENTS}.csv"
MEANS_PATTERN="${DATADIR}/means_"
HEADER="N_SERVERS;N_CLIENTS;MEAN_TIME"

grep "^${N_CLIENTS};" ${MEANS_PATTERN}* | sed "s|^${MEANS_PATTERN}\([[:digit:]]*\)_srv:${N_CLIENTS};|\1;${N_CLIENTS};|" | sort -g > ${OUTPUT_FILE}
#sed -i "1i${HEADER}" ${OUTPUT_FILE} # Insert HEADER at the first line of OUTPUT_FILE

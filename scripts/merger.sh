#!/bin/bash

pipe=fil_list.txt
out_pipe=merged_fil_list.fifo

trap "rm -f $pipe" EXIT
trap "rm -f $out_pipe" EXIT

if [[ ! -p $pipe ]]; then
    mkfifo $pipe
fi

if [[ ! -p $out_pipe ]]; then
    mkfifo $out_pipe
fi

while true
do
    if read line <$pipe; then
        if [[ "$line" == 'quit' ]]; then
            break
        fi
        echo
        date
        echo $line
        first_file=`echo $line | awk -F "," '{print $1;}'`
        out_fil_file=${first_file%%_cc0.fil}.fil
        
        echo "merge_coarse_channels $line $out_fil_file"
        merge_coarse_channels $line $out_fil_file
        echo $out_fil_file > $out_pipe
        echo
    fi
done

echo "Reader exiting"

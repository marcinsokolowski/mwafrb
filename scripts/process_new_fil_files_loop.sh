#!/bin/bash

# list of processed fil files : processed_fil_files.txt
off_line_processing=0
if [[ -n "$1" && "$1" != "-" ]]; then
   off_line_processing=$1
fi

data_dir=/datax2/mwa_fil/
if [[ -n "$2" && "$2" != "-" ]]; then
   data_dir=$2
fi

working_dir=${data_dir}/frb/
if [[ -n "$3" && "$3" != "-" ]]; then
   working_dir=$3
fi

template="*_02.fil"
if [[ -n "$4" && "$4" != "-" ]]; then
   template=$4
fi


wait_time=60

while [ 1 ];
do
   echo
   date
   echo "process_new_fil_files.sh $off_line_processing $data_dir $working_dir \"$template\""
   process_new_fil_files.sh $off_line_processing $data_dir $working_dir "$template"
   
   echo "sleep $wait_time"
   sleep $wait_time
done

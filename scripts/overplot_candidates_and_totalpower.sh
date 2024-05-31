#!/bin/bash

candfile=merged_channels_1715803113.613304_TOTALPOWER.cand
if [[ -n "$1" && "$1" != "-" ]]; then
   candfile="$1"
fi
candfile_plot=${candfile%%cand}cand_vs_time

infile=total_power.txt
if [[ -n "$2" && "$2" != "-" ]]; then
   infile="$2"  
fi

if [[ ! -s ${infile} ]]; then
   echo "Input file $infile does not exist -> exiting now"
   exit -1
fi

awk '{print $1" "$3-$4*5;}' ${infile} > down.txt
awk '{print $1" "$3+$4*5;}' ${infile} > up.txt

awk '{if($1!="#"){print $2" "$11}}' ${candfile} > ${candfile_plot}

# ls total_power.txt ${candfile_plot} down.txt up.txt > list
echo total_power.txt > list
echo ${candfile_plot} >> list
echo down.txt >> list
echo up.txt >> list
cp ~/github/mwafrb/scripts/root/plot_total_power_list.C .


root -l "plot_total_power_list.C+(\"list\")"


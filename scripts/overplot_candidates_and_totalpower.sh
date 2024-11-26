#!/bin/bash

candfile=merged_channels_1715803113.613304_TOTALPOWER.cand
if [[ -n "$1" && "$1" != "-" ]]; then
   candfile="$1"
fi
candfile_plot=${candfile%%cand}cand_vs_time
is_merged=`echo $candfile | awk '{print index($1,".cand_merged");}'`


infile=total_power.txt
if [[ -n "$2" && "$2" != "-" ]]; then
   infile="$2"  
fi

thresh=5
if [[ -n "$3" && "$3" != "-" ]]; then
   thresh=$3
fi

root_options="-l"
if [[ -n "$4" && "$4" != "-" ]]; then
   root_options="$4"
fi


if [[ ! -s ${infile} ]]; then
   echo "Input file $infile does not exist -> exiting now"
   exit -1
fi

awk -v thresh=${thresh} '{print $1" "$3-$4*thresh;}' ${infile} > down.txt
awk -v thresh=${thresh} '{print $1" "$3+$4*thresh;}' ${infile} > up.txt
awk '{print $1" "$5;}' ${infile} > median.txt
awk '{print $1" "$3;}' ${infile} > median_of_medians.txt


# ls total_power.txt ${candfile_plot} down.txt up.txt > list
echo total_power.txt > list
if [[ $is_merged -gt 0 ]]; then
   echo ${candfile} >> list
else
   awk '{if($1!="#"){print $2" "$11}}' ${candfile} > ${candfile_plot}
   echo ${candfile_plot} >> list
fi   
echo down.txt >> list
echo up.txt >> list
echo median.txt >> list
echo median_of_medians.txt >> list
cp ~/github/mwafrb/scripts/root/plot_total_power_list.C .

mkdir -p images/
echo "root ${root_options} plot_total_power_list.C+"
root ${root_options} "plot_total_power_list.C+(\"list\")"


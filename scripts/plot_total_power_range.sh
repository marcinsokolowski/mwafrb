#!/bin/bash

start_time=0
if [[ -n "$1" && "$1" != "-" ]]; then
   start_time="$1"     
fi

end_time=0
if [[ -n "$2" && "$2" != "-" ]]; then
   end_time="$2"
fi

extra=1000
if [[ -n "$3" && "$3" != "-" ]]; then
   extra="$3"
fi

start_time=$(($start_time-$extra))
end_time=$(($end_time+$extra))


n_timesteps=$(($end_time-$start_time))

awk -v start_time=${start_time} -v end_time=${end_time} '{if($1>=start_time && $1<=end_time){print $0;}}' total_power.txt > total_power_${start_time}-${end_time}.txt

ls total_power_${start_time}-${end_time}.txt > list_${start_time}-${end_time}
echo merged_channels_1715803113.613304_TOTALPOWER_4sec.cand_vs_time >> list_${start_time}-${end_time}
echo down.txt >> list_${start_time}-${end_time}
echo up.txt >> list_${start_time}-${end_time}
echo median.txt >> list_${start_time}-${end_time} 
echo median_of_medians.txt >> list_${start_time}-${end_time} 

if [[ ! -s plot_total_power_list.C ]]; then
   echo "cp ~/github/mwafrb/scripts/root/plot_total_power_list.C ."
   cp ~/github/mwafrb/scripts/root/plot_total_power_list.C .
fi

mkdir -p images/
root -l "plot_total_power_list.C+(\"list_${start_time}-${end_time}\",${n_timesteps})"





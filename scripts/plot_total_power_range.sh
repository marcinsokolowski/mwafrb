#!/bin/bash

start_time=0
if [[ -n "$1" && "$1" != "-" ]]; then
   start_time="$1"     
fi

end_time=0
if [[ -n "$2" && "$2" != "-" ]]; then
   end_time="$2"
fi

extra=2000
if [[ -n "$3" && "$3" != "-" ]]; then
   extra="$3"
fi

candfile=merged_channels_1715803113.613304_TOTALPOWER_4sec.cand
if [[ -n "$4" && "$4" != "-" ]]; then
   candfile="$4"
fi
candfile_plot=${candfile%%cand}cand_vs_time
is_merged=`echo $candfile | awk '{print index($1,".cand_merged");}'`

echo "###########################################"
echo "PARAMETERS:"
echo "###########################################"
echo "start_time = $start_time - $extra"
echo "end_time   = $end_fime   + $extra"
echo "candfile   = $candfile (is_merged = $is_merged)"
echo "###########################################"


start_time=$(($start_time-$extra))
end_time=$(($end_time+$extra))


n_timesteps=$(($end_time-$start_time))

awk -v start_time=${start_time} -v end_time=${end_time} '{if($1>=start_time && $1<=end_time){print $0;}}' total_power.txt > total_power_${start_time}-${end_time}.txt

ls total_power_${start_time}-${end_time}.txt > list_${start_time}-${end_time}
echo ${candfile} >> list_${start_time}-${end_time}
echo down.txt >> list_${start_time}-${end_time}
echo up.txt >> list_${start_time}-${end_time}
echo median.txt >> list_${start_time}-${end_time} 
echo median_of_medians.txt >> list_${start_time}-${end_time} 

if [[ ! -s plot_total_power_list.C ]]; then
   echo "cp ~/github/mwafrb/scripts/root/plot_total_power_list.C ."
   cp ~/github/mwafrb/scripts/root/plot_total_power_list.C .
fi

mkdir -p images/

if [[ $is_merged -gt 0 ]]; then
   awk -v start_time=${start_time} -v end_time=${end_time} '{if($5>=start_time && $5<=end_time){print $4;}}' ${candfile} > merged_channels_1715803113.613304_TOTALPOWER_4sec.dm
   awk -v start_time=${start_time} -v end_time=${end_time} '{if($5>=start_time && $5<=end_time){print $13;}}' ${candfile} > merged_channels_1715803113.613304_TOTALPOWER_4sec.idt
else
   awk '{if($1!="#"){print $2" "$11}}' ${candfile} > ${candfile_plot}
   awk -v start_time=${start_time} -v end_time=${end_time} '{if($2>=start_time && $2<=end_time){print $6;}}' ${candfile_plot} > merged_channels_1715803113.613304_TOTALPOWER_4sec.dm
   awk -v start_time=${start_time} -v end_time=${end_time} '{if($2>=start_time && $2<=end_time){print $5;}}' ${candfile_plot} > merged_channels_1715803113.613304_TOTALPOWER_4sec.idt
fi

root -l "histofile.C(\"merged_channels_1715803113.613304_TOTALPOWER_4sec.dm\",0,0)"   
root -l "histofile.C(\"merged_channels_1715803113.613304_TOTALPOWER_4sec.idt\",0,0)"

root -l "plot_total_power_list.C+(\"list_${start_time}-${end_time}\",${n_timesteps})"


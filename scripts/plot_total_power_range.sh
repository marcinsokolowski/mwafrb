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

root_options="-l"
if [[ -n "$5" && "$5" != "-" ]]; then
   root_options="$5"
fi

title="Total Power in Stokes I"
if [[ -n "$6" && "$6" != "-" ]]; then
   title="$6"
fi

candname="unknown"
if [[ -n "$7" && "$7" != "-" ]]; then
   candname="$7"
fi

show_ds9=1
if [[ -n "$8" && "$8" != "-" ]]; then
   show_ds9=$8
fi


echo "###########################################"
echo "PARAMETERS:"
echo "###########################################"
echo "start_time = $start_time - $extra"
echo "end_time   = $end_fime   + $extra"
echo "candfile   = $candfile (is_merged = $is_merged)"
echo "root_options = $root_options"
echo "title      = $title"
echo "candname   = $candname"
echo "show_ds9   = $show_ds9"
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
   dm_file=${candfile%%cand_merged}dm
   idt_file=${candfile%%cand_merged}idt
   echo "awk -v start_time=${start_time} -v end_time=${end_time} '{if(\$5>=start_time && \$5<=end_time){print \$4;}}' ${candfile}"
   awk -v start_time=${start_time} -v end_time=${end_time} '{if($5>=start_time && $5<=end_time){print $4;}}' ${candfile} > ${dm_file}
   awk -v start_time=${start_time} -v end_time=${end_time} '{if($5>=start_time && $5<=end_time){print $13;}}' ${candfile} > ${idt_file}
else
   dm_file=${candfile%%cand}dm
   idt_file=${candfile%%cand}idt

   awk '{if($1!="#"){print $2" "$11}}' ${candfile} > ${candfile_plot}
   awk -v start_time=${start_time} -v end_time=${end_time} '{if($2>=start_time && $2<=end_time){print $6;}}' ${candfile_plot} > ${dm_file}
   awk -v start_time=${start_time} -v end_time=${end_time} '{if($2>=start_time && $2<=end_time){print $5;}}' ${candfile_plot} > ${idt_file}
fi

echo "root -q -l ${root_options} \"histofile.C(\"${dm_file}\",0,0)\""
root -q -l ${root_options} "histofile.C(\"${dm_file}\",0,0)"
root -q -l ${root_options} "histofile.C(\"${idt_file}\",0,0)"

mkdir -p candidates_fits/${candname}

if [[ -d candidates_fits ]]; then
   cd candidates_fits/${candname}
   candfits=`ls ../mergedcand${candname}*.fits | tail -1`
   pwd
   echo "ln -s ${candfits}"
   ln -s ${candfits}
   candfits=`ls mergedcand${candname}*.fits | tail -1`
   pngfile=${candfits%%fits}png
   
   if [[ $show_ds9 -gt 0 ]]; then
      echo "ds9 -scale zscale -geometry 2000x1200 ${candfits} -zoom to fit -saveimage ${pngfile} &"
      ds9 -scale zscale -geometry 2000x1200 ${candfits} -zoom to fit -saveimage ${pngfile} &
   else
      echo "WARNING : ds9 is not required"
   fi
   cd ../../
fi

root ${root_options} "plot_total_power_list.C+(\"list_${start_time}-${end_time}\",${n_timesteps},\"${title}\",\"candidates_fits/${candname}/\")"

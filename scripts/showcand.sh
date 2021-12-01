#!/bin/bash

filfile=1322310880_20211130123422_ch143_02.fil
if [[ -n "$1" && "$1" != "-" ]]; then
   filfile=$1
fi

candfile=${filfile%%fil}cand
sorted_cand=${candfile%%.cand}_sorted.cand
cat $candfile | sort -r --key=2 > ${sorted_cand}

prev_sampno=-1000
step=100
double_step=$(($step*2))

while read line # example 
do
   snr=`echo $line | awk '{print $1;}'`
   
   if [[ $snr != "#" ]]; then
      sampno=`echo $line | awk '{print $2;}'`               
      
      if [[ $sampno -gt $prev_sampno ]]; then
         sampno_start=$(($sampno-$step))
         sampno_end=$(($sampno+$step))
            
         echo "python /home/msok/askap/craft/fredda/20211201/craft/python/plot_allbeams.py -d -3 $filfile --times ${sampno_start},$double_step"   
         python /home/msok/askap/craft/fredda/20211201/craft/python/plot_allbeams.py -d -3 $filfile --times ${sampno_start},$double_step
      
         prev_sampno=$sampno_end
      else
         echo "DEBUG : sampno = $sampno before the end of previously plotted range ($sampno_start - $sampno_end)"
      fi
   else
      echo "DEBUG : comment skipped"
   fi
done < $sorted_cand

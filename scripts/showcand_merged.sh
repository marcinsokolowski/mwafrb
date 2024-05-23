#!/bin/bash

filfile=1322310880_20211130123422_ch143_02.fil
if [[ -n "$1" && "$1" != "-" ]]; then
   filfile=$1
fi

min_snr=10
if [[ -n "$2" && "$2" != "-" ]]; then
   min_snr=$2
fi

min_dm=0
if [[ -n "$3" && "$3" != "-" ]]; then
   min_dm=$3
fi

max_candidates=1000
if [[ -n "$4" && "$4" != "-" ]]; then
   max_candidates=$4
fi

candfile=${filfile%%fil}cand_merged
sorted_cand=${filfile%%.fil}_sorted.cand_merged

n_cand=`cat $candfile | wc -l`
if [[ $n_cand -gt $max_candidates ]]; then
   echo "WARNING : number of candidates larger than limit !!! $n_cand > $max_candidates -> cannot generate png files -> cannot continue because it happened for .cand_merged file !"
   exit
fi


prev_sampno=-1000
step=100
double_step=$(($step*2))

# INDEX  SNR   DM   TIMESTAMP   |TIMESTAMP_RANGE|    FILFILE
# 00000 : 011.14 00000.00 0002136.5000  |0001997.0000 - 0002276.0000|   1322340464_20211130204726_ch169_02_norm.cand
while read line # example 
do
   index=`echo $line | awk '{print $1;}'`
   
   if [[ $index != "#" ]]; then
      snr=`echo $line | awk '{print $3;}'`
      sampno=`echo $line | awk '{printf("%d\n",$5);}'`               
      dm=`echo $line | awk '{print $4;}'`
      
           
      dm_ok=`echo $dm $min_dm | awk '{if($1>=$2){print 1;}else{print 0;}}'`
      snr_ok=`echo $snr $min_snr | awk '{if($1>=$2){print 1;}else{print 0;}}'`           
           
      if [[ $dm_ok -gt 0 && $snr_ok -gt 0 ]]; then
         info="SNR = $snr , DM = $dm"
         output_file=${filfile%%.fil}_${index}.png

         # 2024-05-23 - I am not sure why I had "-d -3" before but it introduces "artficial" and untrue DM in the created png file !
         #              changed to "-d 0" so that the image is as original data
         echo "python ~/github/mwafrb/scripts/viewer/plot_allbeams.py -d 0 $filfile --times ${sampno},$double_step --info \"$info\" --output_file=$output_file"   
         python ~/github/mwafrb/scripts/viewer/plot_allbeams.py -d 0 $filfile --times ${sampno},$double_step --info "$info" --output_file=$output_file 
      else
         echo "DM = $dm is smaller than limit = $min_dm OR SNR = $snr < $min_snr -> candidate skipped"
      fi
   fi
done < $candfile   
# done < $sorted_cand

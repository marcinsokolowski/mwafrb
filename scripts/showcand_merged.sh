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


candfile=${filfile%%fil}cand_merged

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
         echo "python ~/github/mwafrb/scripts/viewer/plot_allbeams.py -d -3 $filfile --times ${sampno},$double_step"   
         python ~/github/mwafrb/scripts/viewer/plot_allbeams.py -d -3 $filfile --times ${sampno},$double_step
      else
         echo "DM = $dm is smaller than limit = $min_dm OR SNR = $snr < $min_snr -> candidate skipped"
      fi
   fi
done < $candfile

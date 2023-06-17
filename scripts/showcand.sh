#!/bin/bash

path=`pwd`
bdir=`basename $path`
# filfile=1322310880_20211130123422_ch143_02.fil
filfile=${bdir}.fil
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

skip_time_in_sec=2 # skip events in the first two seconds as power drops occur then :
if [[ -n "$5" && "$5" != "-" ]]; then
   skip_time_in_sec=$5
fi

echo "###############################################"
echo "# PARAMETERS :"
echo "###############################################"
echo "skip_time_in_sec = $skip_time_in_sec"
echo "###############################################"



candfile=${filfile%%fil}cand
sorted_cand=${candfile%%.cand}_sorted.cand
# cat $candfile | sort -r --key=2 > ${sorted_cand}
cat $candfile | sort --key=8 > ${sorted_cand}

n_cand=`cat $candfile | wc -l`
echo "INFO : number of candidates = $n_cand -> comparing with limit of $max_candidates"
if [[ $n_cand -gt $max_candidates ]]; then
   echo "WARNING : number of candidates larger than limit !!! $n_cand > $max_candidates -> cannot generate png files -> trying to use cand_merged"
   
   echo "showcand_merged.sh ${filfile} ${min_snr} ${min_dm} ${max_candidates}"
   showcand_merged.sh ${filfile} ${min_snr} ${min_dm} ${max_candidates}
   
   echo "WARNING : script showcand.sh will not continue to individual events due to too many"
   exit;
fi

prev_sampno=-1000
step=100
double_step=$(($step*2))

# S/N, sampno, secs from file start, boxcar, idt, dm, beamno, mjd
# 10.02 650 6.5000 25 36 207.80 0 59548.523940972
while read line # example 
do
   snr=`echo $line | awk '{print $1;}'`

   if [[ $snr != "#" ]]; then
      sampno=`echo $line | awk '{print $2;}'`               
      timesec=`echo $line | awk '{print $3;}'`
      dm=`echo $line | awk '{print $6;}'`
      dm_ok=`echo $dm $min_dm | awk '{if($1>=$2){print 1;}else{print 0;}}'`
      snr_ok=`echo $snr $min_snr | awk '{if($1>=$2){print 1;}else{print 0;}}'`
      timesec_ok=`echo $timesec | awk -v skip_time_in_sec=${skip_time_in_sec} '{if($1>skip_time_in_sec){print 1;}else{print 0;}}'`
 
      if [[ $dm_ok -gt 0 && $snr_ok -gt 0 ]]; then   
         if [[ $sampno -gt $prev_sampno ]]; then
            sampno_start=$(($sampno-$step))
            sampno_end=$(($sampno+$step))
            
            info="SNR = $snr , DM = $dm"
            
            output_file=${filfile%%.fil}_${sampno}.png
         
            echo "python ~/mwafrb/scripts/viewer/plot_allbeams.py -d -3 $filfile --times ${sampno_start},$double_step --info "$info" --output_file=$output_file"   
            python ~/mwafrb/scripts/viewer/plot_allbeams.py -d -3 $filfile --times ${sampno_start},$double_step --info "$info" --output_file=$output_file
         
            prev_sampno=$sampno_end
         else
            echo "DEBUG : sampno = $sampno before the end of previously plotted range ($sampno_start - $sampno_end)"
         fi
     else
        echo "DM = $dm is smaller than limit = $min_dm OR SNR = $snr < $min_snr -> candidate skipped"
     fi
   else
      echo "DEBUG : comment skipped"
   fi
done < $sorted_cand

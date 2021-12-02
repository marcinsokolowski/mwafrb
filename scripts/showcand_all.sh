#!/bin/bash

dir_template="*_ch1??_02"
if [[ -n "$1" && "$1" != "-" ]]; then
   dir_template=$1
fi

min_snr=10
if [[ -n "$2" && "$2" != "-" ]]; then
   min_snr=$2
fi

min_dm=0
if [[ -n "$3" && "$3" != "-" ]]; then
   min_dm=$3
fi


for dir in `ls -d ${dir_template=}`
do
   cd ${dir}/FREDDA/
   
   for filfile in `ls *_norm.fil`
   do
      echo "showcand_merged.sh $filfile $min_snr $min_dm"
      showcand_merged.sh $filfile $min_snr $min_dm
   done
   cd ../..
done

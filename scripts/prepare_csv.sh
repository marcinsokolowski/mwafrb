#!/bin/bash

csv_file=observations.csv
if [[ -n "$1" && "$1" != "-" ]]; then
   csv_file=$1
fi

txt_file=${csv_file%%csv}txt
csv2txt_any! ${csv_file} - - - - 4 5

mv ${txt_file} ${txt_file}.tmp
awk '{if(substr($1,1,1)!="#"){print $0;}}' ${txt_file}.tmp > ${txt_file}

awk '{l=length($2);obj=substr($2,2);i=index(obj,"_");obj=substr(obj,1,i-1);i2=index($3,"_");ch=substr($3,i2+1,5);print obj"_"ch;}' ${txt_file} | sort -u > objects_full.txt
awk '{l=length($2);obj=substr($2,2);i=index(obj,"_");obj=substr(obj,1,i-1);i2=index($3,"_");ch=substr($3,i2+1,5);print obj;}' ${txt_file} | sort -u > objects.txt

for object in `cat objects_full.txt`
do
   name=`echo $object | awk '{i=index($1,"_");print substr($1,1,i-1);}'`
   ch=`echo $object | awk '{i=index($1,"_");print substr($1,i+1);}'`
   
   echo "$name / $ch"
   
   obsid_file=${object}.obsid
   grep $name ${txt_file} | grep $ch | awk '{print $1;}' > $obsid_file
done


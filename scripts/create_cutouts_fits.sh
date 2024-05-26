#!/bin/bash

fits=merged_16channels_1709032218.737421_out_t.fits
if [[ -n "$1" && "$1" != "-" ]]; then
   fits=$1
fi


merged_cand=merged_16channels_1709032218.737421.cand_merged
if [[ -n "$2" && "$2" != "-" ]]; then
   merged_cand=$2
fi

outdir=candidates_fits/
if [[ -n "$3" && "$3" != "-" ]]; then
   outdir=$3
fi

evt_required=""
if [[ -n "$4" && "$4" != "-" ]]; then
   evt_required="$4"
fi

min_dm=1
if [[ -n "$5" && "$5" != "-" ]]; then
   min_dm=$5
fi

max_cutouts=-1
if [[ -n "$6" && "$6" != "-" ]]; then
   max_cutouts=$6
fi

min_timesteps=1000
if [[ -n "$7" && "$7" != "-" ]]; then
   min_timesteps=$7
fi


tmpfile=${merged_cand}.tmp

cat ${merged_cand} | awk '{if($1!="#"){print $1" "int(substr($6,2))" "int(substr($8,1,length($8)-1))" "$4;}}' > ${tmpfile}

cutout_file=${merged_cand}.cutouts.txt
rm -f ${cutout_file}

counter=0
mkdir ${outdir}
while read line # example 
do
   evt=`echo $line | awk '{print $1;}'`       
   start=`echo $line | awk '{print $2-1000;}'`
   end=`echo $line | awk '{print $3+1000;}'`
   dm=`echo $line | awk '{print $4;}'`
   is_dm_ok=`echo $dm" "$min_dm | awk '{if($1>$2){print 1;}else{print 0;}}'`
   outfits=${outdir}/cand_${evt}.fits   
   do_cutout=1
   
   if [[ $(($end-$start)) -lt $min_timesteps ]]; then
      echo "WARNING : end=$end , start=$start -> less then minimum $min_timesteps -> changing end to $start+$min_timesteps"
      end=$(($start+$min_timesteps))
   fi
   
   if [[ $max_cutouts -gt 0 && $counter -gt $max_cutouts ]]; then
      echo "WARNING : maximum number of cut-outs $counter exceeds limit -f $max_cutouts -> exiting loop"
      break
   fi
   
   if [[ $is_dm_ok -gt 0 ]]; then
      if [[ -n $evt_required ]]; then
         if [[ $evt != $evt_required ]]; then
            do_cutout=0
         fi
      fi
   
      if [[ $do_cutout -gt 0 ]]; then
         echo "${start} ${end} 1 ${evt}" >> ${cutout_file}
         counter=$(($counter+1))
      else
         echo "Event $evt skipped"
      fi
   else
      echo "WARNING : evt = $evt, dm = $dm lower than min_dm = $min_dm -> skipped"
   fi
done < ${tmpfile}

echo "cutimages $fits -i ${cutout_file} -f merged -c -o candidates_fits/"
cutimages $fits -i ${cutout_file} -f merged -c -o candidates_fits/


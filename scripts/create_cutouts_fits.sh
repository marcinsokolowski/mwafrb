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


tmpfile=${merged_cand}.tmp

cat ${merged_cand} | awk '{if($1!="#"){print $1" "int(substr($6,2))" "int(substr($8,1,length($8)-1));}}' > ${tmpfile}

doit_file=${merged_cand}.cutouts.sh
rm -f ${doit_file}

while read line # example 
do
   evt=`echo $line | awk '{print $1;}'`       
   start=`echo $line | awk '{print $2;}'`
   end=`echo $line | awk '{print $3;}'`
   outfits=${outdir}/cand_${evt}.fits
   do_cutout=1
   
   if [[ -n $evt_required ]]; then
      if [[ $evt != $evt_required ]]; then
         do_cutout=0
      fi
   fi
   
   if [[ $do_cutout -gt 0 ]]; then
      echo "cutimage $fits -s ${start} -e ${end} -c -f ${outfits}"
      echo "cutimage $fits -s ${start} -e ${end} -c -f ${outfits}" >> ${doit_file}
   else
      echo "Event $evt skipped"
   fi
done < ${tmpfile}

chmod +x ${doit_file}
cat ${doit_file}
./${doit_file}



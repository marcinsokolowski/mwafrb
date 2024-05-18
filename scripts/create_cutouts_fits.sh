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


tmpfile=${merged_cand}.tmp

cat ${merged_cand} | awk '{if($1!="#"){print $1" "int(substr($6,2))" "int(substr($8,1,length($8)-1))" "$4;}}' > ${tmpfile}

doit_file=${merged_cand}.cutouts.sh
echo "echo \"Starting cut-out file at:\"" > ${doit_file}
echo "date" >> ${doit_file}
echo "echo \"Finished cut-out file at:\"" > ${doit_file}
echo "date" >> ${doit_file}

counter=0
mkdir ${outdir}
while read line # example 
do
   evt=`echo $line | awk '{print $1;}'`       
   start=`echo $line | awk '{print $2;}'`
   end=`echo $line | awk '{print $3;}'`
   dm=`echo $line | awk '{print $4;}'`
   is_dm_ok=`echo $dm" "$min_dm | awk '{if($1>$2){print 1;}else{print 0;}}'`
   outfits=${outdir}/cand_${evt}.fits   
   do_cutout=1
   
   if [[ $counter -gt $max_cutouts ]]; then
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
         echo "cutimage $fits -s ${start} -e ${end} -c -f ${outfits}"
         echo "cutimage $fits -s ${start} -e ${end} -c -f ${outfits}" >> ${doit_file}
         
         counter=$(($counter+1))
      else
         echo "Event $evt skipped"
      fi
   else
      echo "WARNING : evt = $evt, dm = $dm lower than min_dm = $min_dm -> skipped"
   fi
done < ${tmpfile}

chmod +x ${doit_file}
cat ${doit_file}
./${doit_file}




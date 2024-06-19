#!/bin/bash

#SBATCH --account=director2183
#SBATCH --time=4:00:00
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=220gb
#SBATCH --output=./create_cutouts_fits.o%j
#SBATCH --error=./create_cutouts_fits.e%j
#SBATCH --export=NONE

# use old software stack:
# module unload gcc
# module swap pawseyenv/2024.05 pawseyenv/2023.08
# module load gcc/12.2.0
module load msfitslib/devel


fits=merged_channels_1715803113.613304_out_t.fits
if [[ -n "$1" && "$1" != "-" ]]; then
   fits=$1
fi


merged_cand=`ls -tr *.cand_merged | tail -1`
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

min_dm=0
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

extra=500
if [[ -n "$8" && "$8" != "-" ]]; then
   extra=$8
fi


tmpfile=${merged_cand}.tmp

cat ${merged_cand} | awk '{if($1!="#"){print $1" "int(substr($6,2))" "int(substr($8,1,length($8)-1))" "$4" "$13" "$5;}}' > ${tmpfile}

cutout_file=${merged_cand}.cutouts.txt
rm -f ${cutout_file}

counter=0
mkdir ${outdir}
while read line # example 
do
   evt=`echo $line | awk '{print $1;}'`    
   max_idt=`echo $line | awk '{print int($5);}'`   
   start_auto=`echo $line | awk -v max_idt=${max_idt} '{print int($6)-max_idt;}'`
   end_auto=`echo $line | awk -v max_idt=${max_idt} '{print int($6)+max_idt;}'`
   start=`echo $line | awk -v extra=${extra} -v start_auto=${start_auto} '{if(($2-extra)<(start_auto-extra)){print $2-extra;}else{print start_auto-extra;};}'`
   # end=`echo $line | awk -v extra=${extra} -v end_auto=${end_auto} '{if(($2+extra)>(start_auto+extra)){print $2+extra;}else{print start_auto+extra;};}'`
   end=`echo $line | awk -v extra=${extra} '{print $3+extra;}' # do not do the same at the end for now
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

echo "cutimages $fits -i ${cutout_file} -f merged -c -o ${outdir}"
cutimages $fits -i ${cutout_file} -f merged -c -o ${outdir}


#!/bin/bash

threshold_snr=5
if [[ -n "$1" && "$1" != "-" ]]; then
   threshold_snr=$1
fi

template="*.fil"
if [[ -n "$2" && "$2" != "-" ]]; then
   template=$2
fi

min_dm=1
if [[ -n "$3" && "$3" != "-" ]]; then
   min_dm=$3
fi

step_file=""
if [[ -n "$4" && "$4" != "-" ]]; then
   step_file=$4
fi


# run FREDDA on .fil files, for now sperataly, so an FRB or a pulse can be missed if it happens on the boundery of 2 FITS (i.e. .fil) files:
path_new=/home/msok/askap/craft/fredda/craft/cuda-fdmt/cudafdmt/fredda_current/craft/cuda-fdmt/cudafdmt/src

echo "#!/bin/bash" > fredda.sh
chmod +x fredda.sh

ux_start=`date +%s`
for fil_file in `ls ${template}`
do
   cand_file=${fil_file%%fil}cand
   merged_cand_file=${fil_file%%fil}cand_merged

   echo "${path_new}/cudafdmt $fil_file  -t 512 -d 2048 -S 0 -r 1 -s 1 -m ${min_dm} -x ${threshold_snr} -o $cand_file" >> fredda.sh 

   echo "${path_new}/cudafdmt $fil_file  -t 512 -d 2048 -S 0 -r 1 -s 1 -m ${min_dm} -x ${threshold_snr} -o $cand_file"
   ${path_new}/cudafdmt $fil_file  -t 512 -d 2048 -S 0 -r 1 -s 1 -m ${min_dm} -x ${threshold_snr} -o $cand_file
   
   # simple grouping to have reasonable number of candidates to inspect :
   # --step_file=total_power_fil_RunningMedian5_median.steps_vs_timeindex 
   echo "python $BIGHORNS/software/analysis/scripts/python/my_friends_of_friends.py $cand_file --outfile=${merged_cand_file} --step_file=${step_file}"
   python $BIGHORNS/software/analysis/scripts/python/my_friends_of_friends.py $cand_file --outfile=${merged_cand_file} --step_file=${step_file}
   
   ux_end=`date +%s`
   diff=$(($ux_end-$ux_start))
   echo "PROGRESS : FREDDA on all filterbank files took $diff seconds (end = $ux_end - start = $ux_start)"
done
ux_end=`date +%s`
diff=$(($ux_end-$ux_start))
echo "Profiler : FREDDA on all filterbank files took $diff seconds (end = $ux_end - start = $ux_start)"

awk '{if($1>50){print $0;}}' i_00000.cand > i_00000_snr50.cand
mkdir -p images/
root -b -q -l "histofile.C(\"i_00000.cand,1,0,0,1000000000,10000\")"

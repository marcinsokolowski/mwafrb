#!/bin/bash

template=1469203416_ch%d_beam00.fil
if [[ -n "$1" && "$1" != "-" ]]; then
   template=$1
fi

start_channel=133
if [[ -n "$2" && "$2" != "-" ]]; then
   start_channel=$2
fi
channel_count=24

out_template=$template
if [[ -n "$3" && "$3" != "-" ]]; then
   out_template=$3
fi

threshold_snr=10
if [[ -n "$4" && "$4" != "-" ]]; then
   threshold_snr=$4
fi

min_dm=0 # was 20 but changed to 0 for the time of testing to see all the candidates !
if [[ -n "$5" && "$5" != "-" ]]; then
   min_dm=$5
fi

string=`echo "$template $start_channel" | awk -v nch=${channel_count} '{format=$1",";for(i=0;i<nch;i++){printf(format,($2+i));}printf("\n");}'`
out_file=`echo "$out_template" | awk '{gsub("ch%d","ALL");print $1;}'`
if [[ -n "$6" && "$6" != "-" ]]; then
   out_file=$6
fi


norm_file=${out_file%%.fil}_norm.fil
cand_file=${out_file%%fil}cand
snr_file=${out_file%%fil}snr_vs_timeidx
total_power_file=${out_file%%fil}total_power
avg_file=${out_file%%fil}avg_spectrum

root_path=`which root`

echo "###########################################################"
echo "PARAMETERS :"
echo "###########################################################"
echo "string   = $string"
echo "out_file = $out_file"
echo "norm_file = $norm_file"
echo "cand_file = $cand_file"
echo "snr_file  = $snr_file"
echo "total_power_file = $total_power_file"
echo "avg_file  = $avg_file" 
echo "root_path = $root_path"
echo "###########################################################"

############################################################# ANALYSIS ON ORIGINAL FIL FILE #################################################################
if [[ -s ${out_file} ]]; then
   echo "INFO : merged .fil file ${out_file} already exists -> no need to merge again"
else
   echo "merge_coarse_channels $string $out_file"
   merge_coarse_channels $string $out_file
   
   echo "cp ${avg_file} ${avg_file}_MERGED"
   cp ${avg_file} ${avg_file}_MERGED
fi

exit;

echo "dumpfilfile_float $out_file -S 0 > dump.out 2>&1"
dumpfilfile_float $out_file -S 0 > dump.out 2>&1

mkdir -p images
if [[ -n "$root_path" ]]; then
   root -b -q -l "plotspec_2integrations_freq.C(\"${avg_file}\",\"${avg_file}_MERGED\")"
else
   echo "WARNING : CERN ROOT package not available to make the plots"
fi   

# 
echo "Finished here for now"
exit;

# just to check :
# echo "cutimage 1313388760_20210819061222_ch145_146_147_01_out.fits -s 0 -e 20000 -f 1313388760_20210819061222_ch145_146_147_01_out_0-20000.fits"
# cutimage 1313388760_20210819061222_ch145_146_147_01_out.fits -s 0 -e 20000 -f 1313388760_20210819061222_ch145_146_147_01_out_0-20000.fits

echo "running_median ${total_power_file} total_power_fil_RunningMedian5 -n 5 -D -1"
running_median ${total_power_file} total_power_fil_RunningMedian5 -n 5 -D -1
awk '{if( $1!="#" ){print $1" "$6;}}' total_power_fil_RunningMedian5_median.txt > total_power_fil_RunningMedian5_median.steps
awk '{if( $1!="#" && $6>0 ){print $1" "$2;}}' total_power_fil_RunningMedian5_median.txt > total_power_fil_RunningMedian5_median.steps_vs_timeindex

if [[ -n "$root_path" ]]; then
   root -b -q -l "plot_totalpower_vs_timeindex.C(\"${total_power_file}\",\"total_power_fil_RunningMedian5_median.steps_vs_timeindex\")"
else
   echo "WARNING : CERN ROOT package not available to make the plots"
fi   

# FREDDA before normalisation seems to work better :
echo "frb_search_fredda_eda2_many_files.sh $threshold_snr $out_file ${min_dm} total_power_fil_RunningMedian5_median.steps"
frb_search_fredda_eda2_many_files.sh $threshold_snr $out_file ${min_dm} total_power_fil_RunningMedian5_median.steps


############################################################# ANALYSIS ON NORMALISED FIL FILE #################################################################
mkdir FREDDA
cd FREDDA
ln -s ../${out_file}
echo "dumpfilfile_float ${out_file} -n ../${avg_file} -T 255 > dump.out 2>&1"
dumpfilfile_float ${out_file} -n ../${avg_file} -T 255 > dump.out 2>&1

mkdir -p images
if [[ -n "$root_path" ]]; then
   root -b -q "plotspec_2integrations_freq.C(\"${avg_file}\")"
else
   echo "WARNING : CERN ROOT package not available to make the plots"
fi   

echo "running_median ${total_power_file} total_power_fil_RunningMedian5 -n 5 -D -1"
running_median ${total_power_file} total_power_fil_RunningMedian5 -n 5 -D -1
awk '{if( $1!="#" ){print $1" "$6;}}' total_power_fil_RunningMedian5_median.txt > total_power_fil_RunningMedian5_median.steps
awk '{if( $1!="#" && $6>0 ){print $1" "$2;}}' total_power_fil_RunningMedian5_median.txt > total_power_fil_RunningMedian5_median.steps_vs_timeindex

if [[ -n "$root_path" ]]; then
   root -b -q -l "plot_totalpower_vs_timeindex.C(\"${total_power_file}\",\"total_power_fil_RunningMedian5_median.steps_vs_timeindex\")"
else
   echo "WARNING : CERN ROOT package not available to make the plots"
fi

# just to check :
# echo "cutimage 1313388760_20210819061222_ch145_146_147_01_out.fits -s 0 -e 20000 -f 1313388760_20210819061222_ch145_146_147_01_out_0-20000.fits"
# cutimage 1313388760_20210819061222_ch145_146_147_01_out.fits -s 0 -e 20000 -f 1313388760_20210819061222_ch145_146_147_01_out_0-20000.fits

# FREDDA :
echo "frb_search_fredda_eda2_many_files.sh ${threshold_snr} ${norm_file} ${min_dm} total_power_fil_RunningMedian5_median.steps"
frb_search_fredda_eda2_many_files.sh ${threshold_snr} ${norm_file} ${min_dm} total_power_fil_RunningMedian5_median.steps

cd ..

###############################################################################################################################################################

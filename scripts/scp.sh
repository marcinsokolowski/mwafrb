#!/bin/bash

path=/data/2024_04_22_pulsars/J0835-4510_flagants_90ch_ch230/230/filterbank_msok_64ch/

rsync -avP aavs2:${path}/*.cand* .
rsync -avP aavs2:${path}/total_power.txt .
rsync -avP aavs2:${path}/fredda_totalpower_4sec.out .

~/github/mwafrb/scripts/plot_median_of_median_check.sh fredda_totalpower_4sec.out

candmerged_file=`ls *.cand_merged | tail -1`
/home/msok/github/mwafrb/scripts/overplot_candidates_and_totalpower.sh ${candmerged_file}

cand_file=`ls *.cand | tail -1`
/home/msok/github/mwafrb/scripts/overplot_candidates_and_totalpower.sh ${cand_file}

# plots of total power for all merged candidates:
~/github/mwafrb/scripts/plot_total_power_for_merged.sh

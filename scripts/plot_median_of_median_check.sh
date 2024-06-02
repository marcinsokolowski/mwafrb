#!/bin/bash

outfile=fredda_totalpower_4sec.out
if [[ -n "$1" && "$1" != "-" ]]; then
   outfile="$1"
fi

grep "median of MEDIANS" ${outfile} | awk '{print NR" "$13;}' > median_of_medians.txt
grep "median of MEDIANS" ${outfile} | awk '{print NR" "$18;}' > local_median.txt

ls median_of_medians.txt > list_medians
ls local_median.txt >> list_medians

cp ~/github/mwafrb/scripts/root/plot_medians.C .
root -l "plot_medians.C+(\"list_medians\")"


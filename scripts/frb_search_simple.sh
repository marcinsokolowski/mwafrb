#!/bin/bash

fits=1358703336_20230125173518_ch120_02_out_t.fits
if [[ -n "$1" && "$1" != "-" ]]; then
   fits=$1
fi

dts=${fits%%.fits}_dts.fits
dts_minus_one=${fits%%.fits}_dts_minus_one.fits
dts_snr=${fits%%.fits}_dts_minus_one_div_rmsiqr.fits

echo "dynaspec_search ${fits} ${dts} -l 10 -m 500 -s 1 -t 29600 -n 10 -o 1358703336 -a 0 -d 20 -A -w 0 -S 0 > dynaspec_search.out"
dynaspec_search ${fits} ${dts} -l 10 -m 500 -s 1 -t 29600 -n 10 -o 1358703336 -a 0 -d 20 -A -w 0 -S 0 > dynaspec_search.out

x_size=`fitshdr ${dts} | grep NAXIS1 | awk '{print $3;}'`
y_size=`fitshdr ${dts} | grep NAXIS2 | awk '{print $3;}'`

echo "create_fits ${x_size} ${y_size}"
create_fits ${x_size} ${y_size}

echo "calcfits_bg ${dts} - one.fits ${dts_minus_one}"
calcfits_bg ${dts} - one.fits ${dts_minus_one}

echo "noise_mapper ${dts_minus_one} -r 5 -i"
noise_mapper ${dts_minus_one} -r 5 -i


echo "calcfits_bg ${dts_minus_one} / noise_map_rmsiqr.fits ${dts_snr}"
calcfits_bg ${dts_minus_one} / noise_map_rmsiqr.fits ${dts_snr}

# find sources in DTS :
echo "find_sources_fits ${dts_snr}"
find_sources_fits ${dts_snr}

echo "sort -n series.txt > series_sorted.txt"
sort -n series.txt > series_sorted.txt

echo "python $MWA_FRB/scripts/my_friends_of_friends.py series_sorted.txt --outfile=series_merged.txt --frbsearch_input --group_radius=100"
python $MWA_FRB/scripts/my_friends_of_friends.py series_sorted.txt --outfile=series_merged.txt --frbsearch_input --group_radius=100

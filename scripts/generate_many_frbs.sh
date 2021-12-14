#!/bin/bash

filfile=1322310160_20211130122222_ch143_02.fil
if [[ -n "$1" && "$1" != "-" ]]; then
   filfile=$1
fi

# dm_list="200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000"
dm_list="200 500 1000 1500 2000"
if [[ -n "$2" && "$2" != "-" ]]; then
   dm_list="$2"
fi

snr_list="50"
if [[ -n "$3" && "$3" != "-" ]]; then
   snr_list="$3"
fi

obsid=`echo $filfile | cut -b 1-10`
if [[ -n "$4" && "$4" != "-" ]]; then
   obsid=$4
fi


out_file=${filfile%%.fits}_many.fits

echo "##################################################"
echo "PARAMETERS"
echo "##################################################"
echo "dm_list     = $dm_list"
echo "snr_list   = $snr_list"
echo "filfile    = $filfile"
echo "out_file     = $out_file" 
echo "obsid        = $obsid"
echo "##################################################"


if [[ ! -s ${filfile} ]]; then
   echo "ERROR : image to inject is required and ${filfile} not found"
   exit
fi

# if [[ $obsid -lt 0 ]]; then
#   echo "ERROR : $obsid not specified -> cannot continue"
#   exit
# fi

# url="http://ws.mwatelescope.org/metadata/fits?obs_id="
# echo "wget ${url}${obsid} -O ${obsid}.metafits"
# wget ${url}${obsid} -O ${obsid}.metafits
# if [[ ! -s ${obsid}.metafits ]]; then
#   echo "ERROR : ${obsid}.metafits could not be downloaded"
#   exit
# fi

# inject_frb ${filfile} 0512_0512_many.fits -n 1000 > out 2>&1

# convert fil to fits file:
fits_file=${filfile%%.fil}_out_t.fits
fits_simul_file=${filfile%%.fil}_out_t_simulfrbs.fits
avg_file=${filfile%%fil}avg_spectrum
echo "dumpfilfile_float ${filfile}  -S 0 > dump.out 2>&1"
dumpfilfile_float ${filfile}  -S 0 > dump.out 2>&1

mkdir -p images/
root -b -q "plotspec_ch.C(\"${avg_file}\")"

# normalise by average spectrum
mkdir -p normalised/
cd normalised
echo "ln -s ../${filfile}"
ln -s ../${filfile}

echo "dumpfilfile_float ${filfile} -n ../${avg_file} > dump.out 2>&1"
dumpfilfile_float ${filfile} -n ../${avg_file} > dump.out 2>&1

mkdir -p images/
root -b -q "plotspec_ch.C(\"${avg_file}\")"
############### end of normalisation part ###############

echo "mkdir -p simulated_frbs/"
mkdir -p simulated_frbs/

for dm in `echo ${dm_list}`
do
   dm_str=`echo $dm | awk '{printf("%04d",$1);}'`
   mkdir -p dm_${dm_str}
   cd dm_${dm_str}
   
   for snr in `echo ${snr_list}`
   do
      snr_str=`echo ${snr} | awk '{printf("%05d",$1);}'`
      dir=snr_${snr_str}
      
      if [[ -s $dir ]]; then
         echo "WARNING : snr $snr already processed (directory $dir exists) -> skipped"
      else
         echo "INFO : snr $snr not processed yet -> creating directory $dir and generating ..."
         mkdir -p snr_${snr_str}
         cd snr_${snr_str}
         
         echo "ln -s ../../${fits_file}"
         ln -s ../../${fits_file}
         
#         echo "inject_frb ${filfile} ${out_file} -n 100 -d ${dm} -f ${snr} -o ${obsid} > out 2>&1"      
#         inject_frb ${filfile} ${out_file} -n 100 -d ${dm} -f ${snr} -o ${obsid} > out 2>&1
         echo "inject_frb ${fits_file} ${fits_simul_file} -n 100 -d ${dm} -f 10000 -o ${obsid} -A -V -V -V -S 50 >> simul.out 2>&1"
         inject_frb ${fits_file} ${fits_simul_file} -n 100 -d ${dm} -f 10000 -o ${obsid} -A -V -V -V -S 50 >> simul.out 2>&1
         cd ..
      fi
   done
   cd ..
done

#!/bin/bash

template="ch???_16ch"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

name=J0953+0755
if [[ -n "$2" && "$2" != "-" ]]; then
   name=$2
fi

psrcat_path=`which psrcat`
if [[ -n ${psrcat_path} ]]; then
   ra_deg=`psrcat -e ${name} | grep RAJ | awk '{ra=$2;gsub(":","",ra);if(substr(ra,1,1)=="0"){ra=substr(ra,2);}print ra;}'`
   dec_deg=`psrcat -e ${name} | grep DECJ | awk '{dec=$2;gsub(":","",dec);if(substr(dec,1,1)=="0"){dec=substr(dec,2);}print dec;}'`
   psrcat -e ${name}
   echo "DEBUG : PSR name = $name -> ( RA , DEC ) from psrcat = ( $ra_deg , $dec_deg )"
else
   ra_deg=95309.3097
   dec_deg=75535.75
fi

if [[ -n "$3" && "$3" != "-" ]]; then
   ra_deg=$3
fi

if [[ -n "$4" && "$4" != "-" ]]; then
   dec_deg=$4
fi

for dir in `ls -d ${template}`
do
   cd $dir
   filfile=`ls *.fil | tail -1`

   echo "presto_fold.sh ${filfile} ${name} ${ra_deg} ${dec_deg}"
   presto_fold.sh ${filfile} ${name} ${ra_deg} ${dec_deg}
   
   cd ../
done

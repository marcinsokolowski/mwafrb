#!/bin/bash

filfile=1359219936_20230131170518_ch120_03.fil
if [[ -n "$1" && "$1" != "-" ]]; then
   filfile=$1
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

nsub=16
if [[ -n "$5" && "$5" != "-" ]]; then
   nsub=$5
fi

start=0.08
if [[ -n "$6" && "$6" != "-" ]]; then
   start=$6
fi

options=""
if [[ -n "$7" && "$7" != "-" ]]; then
   options="$7"
fi


echo "########################################"
echo "PARAMETERS:"
echo "########################################"
echo "filfile = $filfile"
echo "name    = $name"
echo "(RA,DEC) = ($ra_deg,$dec_deg) [RA,DEC string -> DECIMAL value]"
echo "nsub = $nsub"
echo "start = $start"
echo "options = $options"
echo "########################################"


export PATH=~/github/presto/bin/:~/github/mwafrb/scripts:$PATH

b=${filfile%%.fil}

mkdir ${b}
cd ${b}
echo "ln -s ../${filfile}"
ln -s ../${filfile}

# set telescope_id := 'u' for MWA :
# J0953+0755 : RAJ 09:53:09.3097 , DECJ +07:55:35.75 -> 95309.3097 , 75535.75 
# NEW PRESTO CODE MWA = 30 (was 117 ???) see sigproc_fb.c case 30: strcpy(s->telescope, "MWA");
echo "update_fil_header ${filfile} -t 30 -r $ra_deg -d $dec_deg"
update_fil_header ${filfile} -t 30 -r $ra_deg -d $dec_deg

echo "readfile updated.fil"
readfile updated.fil

if [[ -n ${psrcat_path} ]]; then
   echo "${psrcat_path} -e ${name} > ${name}.eph"
   ${psrcat_path} -e ${name} > ${name}.eph
   
   echo "prepfold -timing ${name}.eph -debug -n 32 -npart 40 -nsub 16 ${options} updated.fil" >> history_prepfold.txt
   echo "prepfold -timing ${name}.eph -debug -n 32 -npart 40 -nsub 16 ${options} updated.fil"
   prepfold -timing ${name}.eph -debug -n 32 -npart 40 -nsub 16 ${options} updated.fil
else
   # to use all the data use -npart 40 (or different value !) see /home/msok/Desktop/FRBs/FRB_ASKAP_RW/MWA_FRB_searches_real-time/20230613_PRESTO_HELP_FROM_SAM.odt
   # Explanations of options :
   # -start 0.02 : skip the 1st 6sec/300sec = 0.02 of observation (this is assuming 300sec observations) - because the start is always some power drop (due to lost packets !?)
   echo "prepfold -psr ${name} -debug -nopsearch -nodmsearch -nosearch -n 32 -npart 40 -start ${start} -nsub $nsub ${options} updated.fil" >> history_prepfold.txt 
   echo "prepfold -psr ${name} -debug -nopsearch -nodmsearch -nosearch -n 32 -npart 40 -start ${start} -nsub $nsub ${options} updated.fil"
   prepfold -psr ${name} -debug -nopsearch -nodmsearch -nosearch -n 32 -npart 40 -start ${start} -nsub $nsub ${options} updated.fil 
fi   

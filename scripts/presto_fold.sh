#!/bin/bash

filfile=1359219936_20230131170518_ch120_03.fil
if [[ -n "$1" && "$1" != "-" ]]; then
   filfile=$1
fi

name=J0953+0755
if [[ -n "$2" && "$2" != "-" ]]; then
   name=$2
fi

ra_deg=95309.3097
if [[ -n "$3" && "$3" != "-" ]]; then
   ra_deg=$3
fi

dec_deg=75535.75
if [[ -n "$4" && "$4" != "-" ]]; then
   dec_deg=$4
fi


echo "########################################"
echo "PARAMETERS:"
echo "########################################"
echo "filfile = $filfile"
echo "name    = $name"
echo "(RA,DEC) = ($ra_deg,$dec_deg) [RA,DEC string -> DECIMAL value]"
echo "########################################"


export PATH=~/github/presto/bin/:~/github/mwafrb/scripts:$PATH

b=${filfile%%.fil}

mkdir ${b}
cd ${b}
echo "ln -s ../${filfile}"
ln -s ../${filfile}

# set telescope_id := 'u' for MWA :
# J0953+0755 : RAJ 09:53:09.3097 , DECJ +07:55:35.75 -> 95309.3097 , 75535.75 
echo "update_fil_header ${filfile} -t 117 -r $ra_deg -d $dec_deg"
update_fil_header ${filfile} -t 117 -r $ra_deg -d $dec_deg

echo "readfile updated.fil"
readfile updated.fil

# to use all the data use -npart 40 (or different value !) see /home/msok/Desktop/FRBs/FRB_ASKAP_RW/MWA_FRB_searches_real-time/20230613_PRESTO_HELP_FROM_SAM.odt
# Explanations of options :
# -start 0.02 : skip the 1st 6sec/300sec = 0.02 of observation (this is assuming 300sec observations) - because the start is always some power drop (due to lost packets !?)
echo "prepfold -psr ${name} -debug -nopsearch -nodmsearch -nosearch -n 32 -npart 40 -start 0.08 -nsub 16 updated.fil" >> history_prepfold.txt 
echo "prepfold -psr ${name} -debug -nopsearch -nodmsearch -nosearch -n 32 -npart 40 -start 0.08 -nsub 16 updated.fil"
prepfold -psr ${name} -debug -nopsearch -nodmsearch -nosearch -n 32 -npart 40 -start 0.08 -nsub 16 updated.fil

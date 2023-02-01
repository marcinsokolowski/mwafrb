#!/bin/bash

filfile=1359219936_20230131170518_ch120_03.fil
if [[ -n "$1" && "$1" != "-" ]]; then
   filfile=$1
fi

echo "########################################"
echo "PARAMETERS:"
echo "########################################"
echo "filfile = $filfile"
echo "########################################"


export PATH=~/github/presto/bin/:~/github/mwafrb/scripts:$PATH

b=${filfile%%.fil}

mkdir ${b}
cd ${b}
echo "ln -s ../${filfile}"
ln -s ../${filfile}

# set telescope_id := 'u' for MWA :
echo "update_fil_header ${filfile} -t 117 -r 148.28879042 -d 7.92659722"
update_fil_header ${filfile} -t 117 -r 148.28879042 -d 7.92659722

echo "readfile updated.fil"
readfile updated.fil

echo "prepfold -psr J0953+0755 -debug -nopsearch -nodmsearch -nosearch -n 32 updated.fil"
prepfold -psr J0953+0755 -debug -nopsearch -nodmsearch -nosearch -n 32 updated.fil
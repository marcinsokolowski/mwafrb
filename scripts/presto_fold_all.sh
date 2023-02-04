#!/bin/bash

template="*_02.fil"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
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

for filfile in `ls ${template}`
do
   echo "presto_fold.sh ${filfile} ${name} ${ra_deg} ${dec_deg}"
   presto_fold.sh ${filfile} ${name} ${ra_deg} ${dec_deg}
done

#!/bin/bash

template="*_02.fil"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi


for filfile in `ls ${template}`
do
   echo "presto_fold.sh ${filfile}"
   presto_fold.sh ${filfile}
done

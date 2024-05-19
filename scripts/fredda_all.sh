#!/bin/bash

for chdir in `ls -ald ch???`
do
   cd ${chdir}
   
   if [[ -s dynspec_avg7_i.fil ]]; then
      echo "./fredda! dynspec_avg7_i.fil"
      ./fredda! dynspec_avg7_i.fil
      
      echo "wc -l dynspec_avg7_i.cand"
      wc -l dynspec_avg7_i.cand
      
      sleep 5
   else
      echo "$chdir : no dynspec_avg7_i.fil file to process -> skipped"
   fi
   cd ..
done

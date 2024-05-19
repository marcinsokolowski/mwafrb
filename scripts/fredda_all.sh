#!/bin/bash

for chdir in `ls -d ch???`
do
   cd ${chdir}   
   if [[ -s dynspec_avg7_i.fil ]]; then
      echo "~/github/mwafrb/scripts/fredda.sh dynspec_avg7_i.fil"
      ~/github/mwafrb/scripts/fredda.sh dynspec_avg7_i.fil
      
      echo "wc -l dynspec_avg7_i.cand"
      wc -l dynspec_avg7_i.cand
      ls -al *.cand
      
      sleep 5
   else
      echo "$chdir : no dynspec_avg7_i.fil file to process -> skipped"
   fi
   cd ..
done

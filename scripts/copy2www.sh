#!/bin/bash

datadir=/media/msok/625ace3c-584e-49a0-b773-856d6fb8526f/mwa/frb/real_time/blc00/frb
if [[ -n "$1" && "$1" != "-" ]]; then
   datadir=$1
fi

remote_host="bighorns@bighorns2"
if [[ -n "$2" && "$2" != "-" ]]; then
   remote_host="$2"
fi

remote_dir="/var/www/html/mwa/frb"
if [[ -n "$3" && "$3" != "-" ]]; then
   remote_dir="$3"
fi

sleep_time=5

curr_dir=`pwd`

if [[ -d ${datadir} ]]; then
   cd ${datadir}
   
   for subdir in `ls -d *_02`
   do
      png_count=`ls ${subdir}/FREDDA/*.png 2>/dev/null | wc -l`
   
      if [[ $png_count -gt 0 ]]; then 
         echo "DEBUG : .png file found in $subdir and copied to remote WWW server"
         gps=`echo $subdir | cut -b 1-10`
         ux=$(($gps+315964783))
         dt=`date -u -d "1970-01-01 UTC $ux seconds" +"%Y%m%d"`
       
         echo "ssh ${remote_host} \"mkdir -p ${remote_dir}/${dt}\""
         ssh ${remote_host} "mkdir -p ${remote_dir}/${dt}"
      
         echo "rsync -avP ${subdir}/FREDDA/*.png ${remote_host}:${remote_dir}/${dt}/"
         rsync -avP ${subdir}/FREDDA/*.png ${remote_host}:${remote_dir}/${dt}/
         
         sleep $sleep_time
      else
         echo "INFO : no .png files in $subdir -> no copying required"
      fi
   done
   cd ${curr_dir}
else
   echo "ERROR : data directory ${datadir} does not exist"
fi   

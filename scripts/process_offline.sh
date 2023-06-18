#!/bin/bash

export PATH=~/github/mwafrb/scripts/:$PATH

create_links() {
   obslist_file=$1
   
   for obsid in `cat ${obslist_file}`
   do
      filfile=`ls ../${obsid}*02.fil | tail -1`
      echo "ln -sf ${filfile} ."
      ln -sf ${filfile} . 
   done
}


# VELA : object=J0835-4510  83520.61149 -451034.8751
# J1453 : J1453-6413 J1453_ch121.obsid 145332.665 -641316
object=B0950+08
ra=95309.3097
dec=75535.75
if [[ -n "$1" && "$1" != "-" ]]; then
   object=$1
fi

obslist_file=B0950.obsid
if [[ -n "$2" && "$2" != "-" ]]; then
   obslist_file=$2
fi

if [[ -n "$3" && "$3" != "-" ]]; then
   ra=$3
fi

if [[ -n "$4" && "$4" != "-" ]]; then
   dec=$4
fi

remote_dir=/datax2/mwa_fil/
if [[ -n "$5" && "$5" != "-" ]]; then
   remote_dir=$5
fi   

getdata=1
if [[ -n "$6" && "$6" != "-" ]]; then
   getdata=$6
fi

outdir=${obslist_file%%.obsid}
if [[ -n "$7" && "$7" != "-" ]]; then
   outdir=$7
fi


echo "##########################################"
echo "PARAMETERS:"
echo "##########################################"
echo "object = $object"
echo "outdir = $outdir"
echo "(ra,dec) = ($ra,$dec)"
echo "obslist_file = $obslist_file"
echo "remote_dir   = $remote_dir"
echo "getdata      = $getdata"
echo "##########################################"


if [[ $getdata -gt 0 ]]; then
   for obsid in `cat ${obslist_file}`
   do
     echo "rsync -avP mwa@blc00:${remote_dir}/${obsid}*_02.fil ."
     rsync -avP mwa@blc00:${remote_dir}/${obsid}*_02.fil .
     
     cnt=`ls ${obsid}*_02.fil | wc -l`
     if [[ $cnt -gt 0 ]]; then
        echo "OK : $cnt files ${obsid}*_02.fil transferred"
        ls ${obsid}*_02.fil >> ok_files.txt
     else
        echo "ERROR : missing files ${obsid}*_02.fil"
        echo "${obsid}*_02.fil" >> missing_files.txt
     fi
   done
else
   echo "INFO : getting data is not required"
fi   

cnt_ok=`cat ok_files.txt | wc -l`
if [[ $cnt_ok -le 0 ]]; then
   echo "WARNING / ERROR : no filterbank files found on server -> nothing to process"
   exit -1
fi

mkdir -p ${outdir}
echo "cp ${obslist_file} ${outdir}/"
cp ${obslist_file} ${outdir}/
cd ${outdir}
create_links "${obslist_file}"

mkdir -p Folding Fredda

# folding test:
cd Folding
echo "cp ../${obslist_file} ."
cp ../${obslist_file} .
create_links "${obslist_file}"

echo "presto_fold_all.sh \"*_02.fil\" ${object} ${ra} ${dec}"
presto_fold_all.sh "*_02.fil" ${object} ${ra} ${dec}


# FREDDA :
cd ../Fredda/
echo "cp ../${obslist_file} ."
cp ../${obslist_file} .
create_links "${obslist_file}"

path=`pwd`
echo "process_new_fil_files.sh 1 $path $path"
process_new_fil_files.sh 1 $path $path

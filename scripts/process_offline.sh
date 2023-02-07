#!/bin/bash

create_links() {
   obslist_file=$1
   
   for obsid in `cat ${obslist_file}`
   do
      filfile=`ls ../${obsid}*02.fil | tail -1`
      echo "ln -s ${filfile} ."
      ln -s ${filfile} . 
   done
}


# object=J0835-4510
object=B0950+08
ra=95309.3097
dec=75535.75
if [[ -n "$1" && "$1" != "-" ]]; then
   object=$1
fi

obslist_file=B0950.obsid
if [[ -n "$2" && "$2" != "-" ]]; then
   obslist=$2
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



echo "##########################################"
echo "PARAMETERS:"
echo "##########################################"
echo "object = $object"
echo "(ra,dec) = ($ra,$dec)"
echo "obslist_file = $obslist_file"
echo "remote_dir   = $remote_dir"
echo "##########################################"


mkdir -p ${object}
cp ${obslist_file} ${object}/
cd ${object}

mkdir -p Folding Fredda
create_links "${obslist_file}"

# folding test:
cd Folding
create_links "${obslist_file}"

echo "presto_fold_all.sh \"*_02.fil\" ${object} ${ra} ${dec}"
presto_fold_all.sh "*_02.fil" ${object} ${ra} ${dec}


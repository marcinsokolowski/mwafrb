#!/bin/bash

# list of processed fil files : processed_fil_files.txt
off_line_processing=0
if [[ -n "$1" && "$1" != "-" ]]; then
   off_line_processing=$1
fi

data_dir=/datax2/mwa_fil/
if [[ -n "$2" && "$2" != "-" ]]; then
   data_dir=$2
fi

working_dir=${data_dir}/frb/
if [[ -n "$3" && "$3" != "-" ]]; then
   working_dir=$3
fi

template="*_02.fil"
if [[ -n "$4" && "$4" != "-" ]]; then
   template=$4
fi

min_dm=0 # was 20 but changed to 0 for the time of testing to see all the candidates !
if [[ -n "$5" && "$5" != "-" ]]; then
   min_dm=$5
fi

mkdir -p ${working_dir}

cd ${working_dir}
pwd
touch processed_fil_files.txt

for filfile in `ls ${data_dir}/${template}`
do
   echo
   echo "#######################################################################################"
   echo "Processing $filfile ..."
   pwd
   date
   base_filfile=`basename $filfile`
   subdir_name=${base_filfile%%.fil}
#   pwd
   cnt_processed=`grep $base_filfile processed_fil_files.txt | wc -l`
   cand_file=${base_filfile%%fil}cand   
      
   if [[ $cnt_processed -gt 0 || -s ${cand_file} || -d ${subdir_name} ]]; then
      echo "INFO : fil file $filfile already processed -> skipped"
   else
      echo "INFO : fil file $filfile not processed yet -> processing now"
      
      if [[ $off_line_processing -gt 0 ]]; then
         echo "INFO : off-line processing is performed on .fil file $filfile"
         
         ch=`echo ${base_filfile} | awk '{i=index($1,"_ch");s=substr($1,i+1);i2=index(s,"_");print substr(s,3,(i2-3));}'`
         echo "INFO : .fil file $filfile -> ch = $ch"
         
         mkdir -p ${subdir_name}
         cd ${subdir_name}         
         echo "ln -s ${filfile}"
         ln -s ${filfile}
         pwd
         echo "process_fil_file.sh ${base_filfile} ${ch} - - ${min_dm} - 1"
         process_fil_file.sh ${base_filfile} ${ch} - - ${min_dm} - 1 
         cd ../
         pwd
      else      
         echo "INFO : real-time processing path"
         
         echo "ln -s ${filfile}"
         ln -s ${filfile}
         pwd
         echo "cudafdmt ${base_filfile}  -t 512 -d 2048 -S 0 -r 1 -s 1 -m ${min_dm} -x 10 -o ${cand_file}"
         cudafdmt ${base_filfile}  -t 512 -d 2048 -S 0 -r 1 -s 1 -m ${min_dm} -x 10 -o ${cand_file}   
   
         echo ${base_filfile} >> processed_fil_files.txt
 
         cnt_cand=`cat ${cand_file} | wc -l`
         echo "Fil file $filfile -> $cnt_cand candidates"
      fi
   fi
   echo "Processing of $filfile finished."
   date
   pwd
   echo "#######################################################################################"   
done

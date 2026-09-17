#!/bin/bash
#SBATCH --cpus-per-task=4
#SBATCH --mem=100GB
#SBATCH --job-name=process_tied_array_fil_file
#SBATCH --output=submit_process_tied_array_fil_file_%x-%j.out
#SBATCH --error=submit_process_tied_array_fil_file_%x-%j.err
#SBATCH --partition=highmem
#SBATCH --time=24:00:00

obsid=1469204016
if [[ -n "$1" && "$1" != "-" ]]; then
   obsid=$1
fi

jobid=1073848
if [[ -n "$2" && "$2" != "-" ]]; then
   jobid=$2
fi

start_ch=133
if [[ -n "$3" && "$3" != "-" ]]; then
   start_ch=$3
fi


if [[ -s ${obsid}_${jobid}_MWAX_BEAMFORMER.tar ]]; then
   echo "File ${obsid}_${jobid}_MWAX_BEAMFORMER.tar exists -> no need to copy"
else
   echo "cp /scratch/mwavcs/asvo/${jobid}/${obsid}_${jobid}_MWAX_BEAMFORMER.tar ."
   cp /scratch/mwavcs/asvo/${jobid}/${obsid}_${jobid}_MWAX_BEAMFORMER.tar .
   
   echo "tar xvf ${obsid}_${jobid}_MWAX_BEAMFORMER.tar"
   tar xvf ${obsid}_${jobid}_MWAX_BEAMFORMER.tar
fi

module load msfitslib/master-ittkjmq
module load cfitsio/3.49
module load libnova/0.15.0-iwh6cpn
module load fftw/3.3.10

export PATH=/software/projects/pawsey1154/msok/github/mwafrb/src:$PATH

dumpfilfile_float -h
merge_coarse_channels -h 

echo "/software/projects/pawsey1154/msok/github/mwafrb/scripts/process_tied_array_fil_file.sh \"${obsid}_ch%d_beam00.fil\" ${start_ch}"
/software/projects/pawsey1154/msok/github/mwafrb/scripts/process_tied_array_fil_file.sh "${obsid}_ch%d_beam00.fil" ${start_ch}

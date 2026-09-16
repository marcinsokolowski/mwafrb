#!/bin/bash
#SBATCH --cpus-per-task=4
#SBATCH --mem=100GB
#SBATCH --job-name=process_tied_array_fil_file
#SBATCH --output=submit_process_tied_array_fil_file_%x-%j.out
#SBATCH --error=submit_process_tied_array_fil_file__%x-%j.err
#SBATCH --partition=highmem
#SBATCH --time=24:00:00

echo "cp /scratch/mwavcs/asvo/1073849/1469203416_1073849_MWAX_BEAMFORMER.tar ."
cp /scratch/mwavcs/asvo/1073849/1469203416_1073849_MWAX_BEAMFORMER.tar .

echo "tar xvf 1469203416_1073849_MWAX_BEAMFORMER.tar"
tar xvf 1469203416_1073849_MWAX_BEAMFORMER.tar

module load msfitslib/master-ittkjmq
module load cfitsio/3.49
module load libnova/0.15.0-iwh6cpn
module load fftw/3.3.10

dumpfilfile_float -h
merge_coarse_channels -h 

echo "/software/projects/pawsey1154/msok/github/mwafrb/scripts/process_tied_array_fil_file.sh \"1469203416_ch%d_beam00.fil\" 133"
/software/projects/pawsey1154/msok/github/mwafrb/scripts/process_tied_array_fil_file.sh "1469203416_ch%d_beam00.fil" 133
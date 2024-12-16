#!/bin/bash

#SBATCH --account=director2183
#SBATCH --time=4:00:00
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=220gb
#SBATCH --output=./create_cutouts_fits.o%j
#SBATCH --error=./create_cutouts_fits.e%j
#SBATCH --export=NONE

# use old software stack:
# module unload gcc
# module swap pawseyenv/2024.05 pawseyenv/2023.08
# module load gcc/12.2.0
module load msfitslib/devel


time_cand=2633
if [[ -n "$1" && "$1" != "-" ]]; then
   time_cand=$1
fi

duration=3
if [[ -n "$2" && "$2" != "-" ]]; then
   duration=$2
fi

fil=updated.fil
if [[ -n "$3" && "$3" != "-" ]]; then
   fil=$3
fi




source /home/aavs/msok_python38_env/bin/activate

echo "python ~/github/presto/python/presto/waterfaller.py --show-ts --show-spec --colour-map='viridis' -d 0 -T ${time_cand} -t ${duration} --scaleindep ${fil}"
python ~/github/presto/python/presto/waterfaller.py --show-ts --show-spec --colour-map='viridis' -d 0 -T ${time_cand} -t ${duration} --scaleindep ${fil}


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


fits=merged_channels_1715803113.613304_out_t.fits
if [[ -n "$1" && "$1" != "-" ]]; then
   fits=$1
fi

centre_x=1896996
if [[ -n "$2" && "$2" != "-" ]]; then
   centre_x=$2
fi

outdir=fits/
if [[ -n "$3" && "$3" != "-" ]]; then
   outdir=$3
fi

radius=1000
if [[ -n "$4" && "$4" != "-" ]]; then
   radius=$4
fi

tag=cutout00000
if [[ -n "$5" && "$5" != "-" ]]; then
   tar=$5
fi


start=$(($centre_x-$radius))
end=$(($centre_x+$radius))


echo "${start} ${end} 1 ${tag}" > cutouts.txt

echo "cutimages $fits -i cutouts.txt -f merged -c -o ${outdir}"
cutimages $fits -i cutouts.txt -f merged -c -o ${outdir}

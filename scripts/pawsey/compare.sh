#!/bin/bash

#SBATCH --account=pawsey1154
#SBATCH --time=23:59:00
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --mem=128gb
#SBATCH --output=./compare.o%j
#SBATCH --error=./compare.e%j
#SBATCH --export=NONE

# module load msfitslib/devel

module load cpe/25.03 
module swap pawseyenv/2026.11 pawseyenv/2025.08 
module load gcc-native/14.2

module load msfitslib/master-ittkjmq

date

for fits in `ls *.fits`
do
   echo
   date
   echo "calcfits_bg ${fits} = ../${fits}"
   calcfits_bg ${fits} = ../${fits}
   echo
done

echo "Finished at:"
date

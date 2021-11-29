#!/bin/bash

b=`date +%Y%m%d`_generated
n_coarse_ch=3 # 24 all MWA 

# export PATH=/home/msok/bighorns/software/analysis/craft/tests:$PATH

echo "generate_fil_files ${b} $n_coarse_ch -m -1 -s 10 > fil_simulator.out 2>&1 &"
nohup generate_fil_files ${b} $n_coarse_ch -m -1 -s 10 > fil_simulator.out 2>&1 &


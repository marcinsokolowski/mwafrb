#!/bin/bash

filfile=merged_channels_1714558263.211720.fil
if [[ -n "$1" && "$1" != "-" ]]; then
   filfile="$1"
fi

# cudafdmt dynaspec_00092_00090.fil  -t 512 -d 2048 -S 0 -r 1 -s 1 -m 1 -x 5 -o dynaspec_00092_00090_test.cand

candfile=${filfile%%fil}cand
echo "/usr/local/bin//cudafdmt ${filfile} -t 512 -d 2048 -S 0 -r 1 -s 1 -m 100 -x 10 -o ${candfile}"
/usr/local/bin//cudafdmt ${filfile} -t 512 -d 2048 -S 0 -r 1 -s 1 -m 100 -x 10 -o ${candfile}



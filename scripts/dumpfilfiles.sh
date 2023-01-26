#!/bin/bash

for filfile in `ls *.fil`
do
   dumpfile=${filfile%fil}dmp
   
   echo "dumpfilfile $filfile > $dumpfile"
   dumpfilfile $filfile > $dumpfile   
done

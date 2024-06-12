#!/bin/bash

mkdir -p images/
for fitsfile in `ls *.fits`; 
do 
   png=${fitsfile%%fits}png     
   ds9 -scale zscale -geometry 1800x1000  $fitsfile -saveimage images/${png}; 
done


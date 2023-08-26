#!/bin/bash

awk -v n_bins=32 -v offset=26 '{if($1!="#"){newbin=$1;if($1<offset){newbin=n_bins+$1;}print (newbin-offset)/n_bins" "$2;}}' updated_0.00-0.90_PSR_j0835-4510.pfd.bestprof | sort -n > new.txt


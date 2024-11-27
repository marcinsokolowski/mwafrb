#!/bin/bash

candfile=`ls -tr *.cand_merged | tail -1`
if [[ -n "$1" && "$1" != "-" ]]; then
   candfile="$1"
fi
is_merged=`echo $candfile | awk '{print index($1,".cand_merged");}'`
cp $candfile ${candfile}.tmp

extra=1000
if [[ -n "$2" && "$2" != "-" ]]; then
   extra="$2"
fi

root_options="-l -q -b"
if [[ -n "$3" && "$3" != "-" ]]; then
   root_options="$3"
fi

show_ds9=1
if [[ -n "$4" && "$4" != "-" ]]; then
   show_ds9=$4
fi

echo "###########################################"
echo "PARAMETERS:"
echo "###########################################"
echo "candfile   = $candfile (is_merged = $is_merged)"
echo "extra      = $extra"
echo "root options = $root_options"
echo "show_ds9   = $show_ds9"
echo "###########################################"

rm -f plot_total_power_for_merged.doit
touch plot_total_power_for_merged.doit 

while read line # example 
do
   first_char=`echo $line | awk '{print $1;}'`

   if [[ $first_char != "#" ]]; then
# awk '{if($1!="#"){print $1" "int(substr($6,2))" "int(substr($8,1,length($8)-1))" "$4" "$13" "$5;}}'
      start_time=`echo $line | awk '{if($1!="#"){print int(substr($6,2))-$13;}}'`
      end_time=`echo $line | awk '{if($1!="#"){print int(substr($8,1,length($8)-1));}}'`      
      candname=`echo $line | awk '{if($1!="#"){print $1;}}'`
      
      echo "plot_total_power_range.sh $start_time $end_time $extra ${candfile} \"${root_options}\" \"Total power for candidate : ${candname}\"" $candname $show_ds9 >> plot_total_power_for_merged.doit
#      plot_total_power_range.sh $start_time $end_time $extra ${candfile}
#      exit
   else
      echo "DEBUG : comment line |$line| skipped"
   fi
done < $candfile


chmod +x plot_total_power_for_merged.doit
./plot_total_power_for_merged.doit

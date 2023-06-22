#!/bin/bash

export PATH=~/github/mwafrb/scripts/:$PATH

create_links() {
   obslist_file=$1
   
   for obsid in `cat ${obslist_file}`
   do
      filfile=`ls ../${obsid}*02.fil | tail -1`
      echo "ln -sf ${filfile} ."
      ln -sf ${filfile} . 
   done
}


# VELA : object=J0835-4510  83520.61149 -451034.8751
# J1453 : J1453-6413 J1453_ch121.obsid 145332.665 -641316
object=B0950+08
ra=95309.3097
dec=75535.75
if [[ -n "$1" && "$1" != "-" ]]; then
   object=$1
fi

obslist_file=B0950.obsid
if [[ -n "$2" && "$2" != "-" ]]; then
   obslist_file=$2
fi

if [[ -n "$3" && "$3" != "-" ]]; then
   ra=$3
fi

if [[ -n "$4" && "$4" != "-" ]]; then
   dec=$4
fi

remote_dir=/datax2/mwa_fil/
if [[ -n "$5" && "$5" != "-" ]]; then
   remote_dir=$5
fi   

getdata=1
if [[ -n "$6" && "$6" != "-" ]]; then
   getdata=$6
fi

outdir=${obslist_file%%.obsid}
if [[ -n "$7" && "$7" != "-" ]]; then
   outdir=$7
fi

calc_expected=1
if [[ -n "$8" && "$8" != "-" ]]; then
   calc_expected=$8
fi

observing_time_sec=296
if [[ -n "$9" && "$9" != "-" ]]; then
   observing_time_sec=$9
fi

inttime_sec=0.001
if [[ -n "${10}" && "${10}" != "-" ]]; then
   inttime_sec=${10}
fi

freq_channel=169
if [[ -n "${11}" && "${11}" != "-" ]]; then
   freq_channel=${11}
fi
freq_mhz=`echo $freq_channel | awk '{print $1*1.28;}'`

external_pulse_width_ms=-1
if [[ -n "${12}" && "${12}" != "-" ]]; then
   external_pulse_width_ms=${12}
fi

external_flux_density=-1
if [[ -n "${13}" && "${13}" != "-" ]]; then
   external_flux_density=${13}
fi

n_bins=32

echo "##########################################"
echo "PARAMETERS:"
echo "##########################################"
echo "object = $object"
echo "outdir = $outdir"
echo "(ra,dec) = ($ra,$dec)"
echo "obslist_file = $obslist_file"
echo "remote_dir   = $remote_dir"
echo "getdata      = $getdata"
echo "calc_expected = $calc_expected"
echo "   observing_time_sec = $observing_time_sec"
echo "   inttime_sec        = $inttime_sec"
echo "   freq_channel       = $freq_channel ($freq_mhz MHz)"
echo "   external_pulse_width = $external_pulse_width_ms [ms]"
echo "   external_flux_density = $external_flux_density"
echo "   n_bins                = $n_bins"
echo "##########################################"


if [[ $getdata -gt 0 ]]; then
   for obsid in `cat ${obslist_file}`
   do
     echo "rsync -avP mwa@blc00:${remote_dir}/${obsid}*_02.fil ."
     rsync -avP mwa@blc00:${remote_dir}/${obsid}*_02.fil .
     
     cnt=`ls ${obsid}*_02.fil | wc -l`
     if [[ $cnt -gt 0 ]]; then
        echo "OK : $cnt files ${obsid}*_02.fil transferred"
        ls ${obsid}*_02.fil >> ok_files.txt
     else
        echo "ERROR : missing files ${obsid}*_02.fil"
        echo "${obsid}*_02.fil" >> missing_files.txt
     fi
   done
else
   echo "INFO : getting data is not required"
fi   

cnt_ok=`cat ok_files.txt | wc -l`
if [[ $cnt_ok -le 0 ]]; then
   echo "WARNING / ERROR : no filterbank files found on server -> nothing to process"
   exit -1
fi

mkdir -p ${outdir}
echo "cp ${obslist_file} ${outdir}/"
cp ${obslist_file} ${outdir}/
cd ${outdir}
create_links "${obslist_file}"

mkdir -p Folding Fredda

# folding test:
cd Folding
echo "cp ../${obslist_file} ."
cp ../${obslist_file} .
create_links "${obslist_file}"

echo "presto_fold_all.sh \"*_02.fil\" ${object} ${ra} ${dec}"
presto_fold_all.sh "*_02.fil" ${object} ${ra} ${dec}


# FREDDA :
cd ../Fredda/
echo "cp ../${obslist_file} ."
cp ../${obslist_file} .
create_links "${obslist_file}"

path=`pwd`
echo "process_new_fil_files.sh 1 $path $path"
process_new_fil_files.sh 1 $path $path

# calculated expected sensitivity 
if [[ $calc_expected -gt 0 ]]; then
   cd ../
   pwd
   half_cnt=`cat $obslist_file | wc -l | awk '{print $1/2;}'`
   
   middle_obsid=`cat $obslist_file | head --lines=${half_cnt} | tail -1`
   echo "Calculating expected sensitivity and SNRs for middle obsID = $middle_obsid"
   
   echo "getmeta! ${middle_obsid}"
   getmeta! ${middle_obsid}
   
   echo "psrcat -e2 ${object} > ${object}.eph"
   psrcat -e2 ${object} > ${object}.eph
   
   ra_hms=`grep RAJ ${object}.eph | head -1 | awk '{print $2;}'`
   dec_dms=`grep DECJ ${object}.eph | head -1 | awk '{print $2;}'`
   ra_deg=`radec2deg.sh $ra_hms $dec_dms | grep ra_deg | awk '{print $3;}'`
   dec_deg=`radec2deg.sh $ra_hms $dec_dms | grep dec_deg | awk '{print $3;}'`   
   
   period=`grep P0 ${object}.eph | awk '{print $2;}'`
   pulse_width_ms=`grep W10 ${object}.eph | awk '{print $2;}'`

   ok=`echo $external_pulse_width_ms | awk '{if($1>0){print 1;}else{print 0;}}'`   
   if [[ $ok -gt 0 ]]; then
      echo "Using externally provided pulse width = $external_pulse_width_ms [ms]"
      pulse_width_ms=$external_pulse_width_ms 
   fi
   pulse_width_sec=`echo $pulse_width_ms | awk '{print $1/1000.00;}'`
   
   flux_density_line=`cat ${object}.eph | awk -v obs_freq_mhz=${freq_mhz} -v min_diff=1000 '{if(substr($1,1,1)=="S"){freq_mhz=strtonum(substr($1,2));diff=obs_freq_mhz-freq_mhz;if(diff<0){diff=-diff;}if(freq_mhz>0 && diff<min_diff){best_freq_mhz=freq_mhz;best_flux=$2;min_diff=diff;}}}END{print best_freq_mhz" "best_flux;}'`
   echo "Freq [MHz] and Flux [Jy] = $flux_density_line"
   flux_density_mjy=`echo $flux_density_line | awk '{print $2;}'`    # in mJy 
   flux_density=`echo $flux_density_line | awk '{print $2/1000.00;}'` # in Jy 

   ok=`echo $external_flux_density | awk '{if($1>0){print 1;}else{print 0;}}'`   
   if [[ $external_flux_density -gt 0 ]]; then
      flux_density=$external_flux_density
      echo "Using external flux density = $flux_density [Jy]"
   fi
   
   echo "python ~/github/mwa_pb/scripts/mwa_sensitivity.py -c ${freq_channel} -m full_EE -g ${middle_obsid} --pointing_ra_deg=${ra_deg} --pointing_dec_deg=${dec_deg} --metafits=${middle_obsid}.metafits --inttime=${inttime_sec} --antnum=128 --total_observing_time=${inttime_sec} --incoherent --pulsar_observing_time=${observing_time_sec} --n_phase_bins=${n_bins} --psr_mean_flux=${flux_density} --psr_period=${period} --psr_pulse_width=${pulse_width_sec} --show_snr --bandwidth=1280000 > ${object}_ch${freq_channel}_expected.out 2>&1"
   python ~/github/mwa_pb/scripts/mwa_sensitivity.py -c ${freq_channel} -m full_EE -g ${middle_obsid} --pointing_ra_deg=${ra_deg} --pointing_dec_deg=${dec_deg} --metafits=${middle_obsid}.metafits --inttime=${inttime_sec} --antnum=128 --total_observing_time=${inttime_sec} --incoherent --pulsar_observing_time=${observing_time_sec} --n_phase_bins=${n_bins} --psr_mean_flux=${flux_density} --psr_period=${period} --psr_pulse_width=${pulse_width_sec} --show_snr --bandwidth=1280000 > ${object}_ch${freq_channel}_expected.out 2>&1
   # python ~/github/mwa_pb/scripts/mwa_sensitivity.py -c ${ch} -m full_EE -g ${middle_obsid} --pointing_az_deg=0.00 --pointing_za_deg=0.00 --delays 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 --inttime=${inttime}  --antnum=128 --total_observing_time=${inttime} --incoherent --pulsar_observing_time=296.00 --n_phase_bins=32 --psr_mean_flux=2.37 --psr_period=0.2530651649482 --psr_pulse_width=0.0206 --show_snr --bandwidth=${bw_hz}
fi


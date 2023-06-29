#!/bin/bash

obsid=1371192616
if [[ -n "$1" && "$1" != "-" ]]; then
   obsid=$1
fi

object="J0835-4510"
if [[ -n "$2" && "$2" != "-" ]]; then
   object="$2"
fi

freq_channel=169
inttime_sec=0.001
observing_time_sec=190
n_bins=32

external_pulse_width_ms=40
external_flux_density=5.9

echo "Calculating expected sensitivity and SNRs for middle obsID = $obsid"
   
echo "getmeta! ${obsid}"
getmeta! ${obsid}
   
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
if [[ $ok -gt 0 ]]; then
    flux_density=$external_flux_density
    echo "Using external flux density = $flux_density [Jy]"
fi
   
echo "python ~/github/mwa_pb/scripts/mwa_sensitivity.py -c ${freq_channel} -m full_EE -g ${obsid} --pointing_ra_deg=${ra_deg} --pointing_dec_deg=${dec_deg} --metafits=${obsid}.metafits --inttime=${inttime_sec} --antnum=128 --total_observing_time=${inttime_sec} --incoherent --pulsar_observing_time=${observing_time_sec} --n_phase_bins=${n_bins} --psr_mean_flux=${flux_density} --psr_period=${period} --psr_pulse_width=${pulse_width_sec} --show_snr --bandwidth=1280000 > ${object}_ch${freq_channel}_expected.out 2>&1"
python ~/github/mwa_pb/scripts/mwa_sensitivity.py -c ${freq_channel} -m full_EE -g ${obsid} --pointing_ra_deg=${ra_deg} --pointing_dec_deg=${dec_deg} --metafits=${obsid}.metafits --inttime=${inttime_sec} --antnum=128 --total_observing_time=${inttime_sec} --incoherent --pulsar_observing_time=${observing_time_sec} --n_phase_bins=${n_bins} --psr_mean_flux=${flux_density} --psr_period=${period} --psr_pulse_width=${pulse_width_sec} --show_snr --bandwidth=1280000 > ${object}_ch${freq_channel}_expected.out 2>&1

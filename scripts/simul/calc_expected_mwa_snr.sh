#!/bin/bash

obsid=1360078208
inttime=0.10
total_time=240

getmeta! ${obsid}

python ~/github/mwa_pb/scripts/mwa_sensitivity.py -c 169 -m full_EE -g ${obsid} --pointing_ra_deg=128.83588121 --pointing_dec_deg=-45.17635419 --metafits=${obsid}.metafits \
      --inttime=${inttime} --antnum=128 --total_observing_time=${inttime} --incoherent --pulsar_observing_time=${total_time} --n_phase_bins=32 --psr_mean_flux=5.9 --pulsar_peak_flux=20.00 \
      --psr_period=0.089328385024 --psr_pulse_width=0.05 --show_snr --bandwidth=1280000 > J0835-4510_ch169_expected_PeakFlux20Jy.out 2>&1

python ~/github/mwa_pb/scripts/mwa_sensitivity.py -c 169 -m full_EE -g ${obsid} --pointing_ra_deg=128.83588121 --pointing_dec_deg=-45.17635419 --metafits=${obsid}.metafits \
      --inttime=${inttime} --antnum=128 --total_observing_time=${inttime} --incoherent --pulsar_observing_time=${total_time} --n_phase_bins=32 --psr_mean_flux=7.8 --pulsar_peak_flux=31.00 \
      --psr_period=0.089328385024 --psr_pulse_width=0.05 --show_snr --bandwidth=1280000 > J0835-4510_ch169_expected_MWA-flux.out 2>&1



python ~/github/mwa_pb/scripts/mwa_sensitivity.py -c 169 -m full_EE -g ${obsid} --pointing_ra_deg=128.83588121 --pointing_dec_deg=-45.17635419 --metafits=${obsid}.metafits \
      --inttime=${inttime} --antnum=128 --total_observing_time=${inttime} --incoherent --pulsar_observing_time=${total_time} --n_phase_bins=32 --psr_mean_flux=5.9 --pulsar_peak_flux=10.00 \
      --psr_period=0.089328385024 --psr_pulse_width=0.05 --show_snr --bandwidth=1280000 > J0835-4510_ch169_expected_PeakFlux10Jy.out 2>&1 


echo "Do not use last noise Stokes I - this is for ${inttime} seconds - but must use for a single phase bin (total_time/n_bins) or total_time"




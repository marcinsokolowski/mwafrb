import pdb
import numpy as np
import astropy.wcs as wcs
import astropy.io.fits as pf
from astropy import units as au
from astropy.coordinates import SkyCoord
from astropy.coordinates import Angle
from astropy.table import Table
import ephem
import os
import glob
import datetime
import pytz
import sys
import scipy.linalg
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from astropy import units as u
import math
from optparse import OptionParser,OptionGroup

try:
    import astropy.io.fits as pyfits
    import astropy.wcs as pywcs
    _useastropy=True
except ImportError:
    import pywcs,pyfits
    _useastropy=False


# Example of checking a source in the image in the GLEAM catalog :
# import find_gleam
# (RA,Dec,gleam_sources,gleam_fluxes,radius) = find_gleam.get_gleam_sources( "avg_I.fits" )
# s=find_gleam.find_source(gleam_sources,gleam_fluxes,97.2166,-28.9391) 
#
# Example of matching the sources from the text file list to GLEAM catalog :
# find_gleam.calibrate_image("avg_I_without.fits","avg_I_without.txt")
# avg_I_without.txt - format : X Y RA[deg] DEC[deg] Peak_Flux[Jy/Beam]

# Constants
C = 2.99792458e8
r2h = 12.0/np.pi
r2d = 180.0/np.pi
h2d = 360.0/24.0
d2h = 24.0/360.0
ARCMIN2RAD = np.pi / (60.0 * 180.0)
DEG2RAD = ( math.pi / 180.00)
RAD2DEG = ( 180.00 / math.pi )
RAD2ARCSEC = ( ( 180.00 / math.pi )*3600. )

gleam_default_file="/home/msok/MWA_Tools/catalogues/GLEAM_EGC.fits"
g_debug_level=0

class Source :
   index      = -1000
   fits_file  = ""
   fluence    = -1000
   flux       = -1000
   dm         = -1000
   start_time = -1000
   x          = -1000
   y          = -1000
   snr        = -1000   

   def __init__(self, index, fits_file, dm, fluence, flux, start_time, x=-1000, y=-1000, snr=-1000, end_time=-1000 ):
        self.index     = index
        self.fits_file = fits_file
        self.dm        = dm
        self.fluence   = fluence
        self.flux      = flux
        self.start_time = start_time
        self.x          = x
        self.y          = y
        self.snr        = snr
        self.end_time   = end_time 


def read_candidate_file( filename ) :
   sources=[]

   if os.path.exists(filename) and os.stat(filename).st_size > 0 :
      file=open(filename,'r')
      data=file.readlines()
      index=0
      for line in data :
         words = line.split(' ')
         if line[0] == '#' :
            continue
         
         if line[0] != "#" :
            x    = float(words[0+0])
            y    = float(words[1+0])
            snr  = float(words[2+0])
         
            sources.append( Source( index, None, -1000, -1000, -1000, -1000, x, y, snr ) )
            index += 1
      file.close()
      
   else :
      print "WARNING : empty or non-existing file %s" % (filename)      

   
   return sources

def read_fredda_candidates( filename ) :
   sources=[]

   if os.path.exists(filename) and os.stat(filename).st_size > 0 :
      file=open(filename,'r')
      data=file.readlines()
      index=0
      for line in data :
         # S/N, sampno, secs from file start, boxcar, idt, dm, beamno, mjd
         # 10.08 6760 67.6000 21 13 75.04 0 59548.516314815
         words = line.split(' ')
         if line[0] == '#' :
            continue
         
         if line[0] != "#" :
            snr  = float(words[0+0])
            sampno = int(words[1+0])
            secs_from_start = float(words[2+0])
            boxcar = int(words[3+0])
            idt = int(words[4+0])
            dm = float(words[5+0])
            beamno = int(words[6+0])
            mjd = float(words[7+0])
            # print("DEBUG : %d %.4f %.4f" % (index,snr,dm))
            
            # index, fits_file, dm, fluence, flux, start_time, x=-1000, y=-1000, snr=-1000 ):
            sources.append( Source( index, filename, dm, -1000, -1000, sampno, x=-1000, y=-1000, snr=snr ) )
            index += 1
      file.close()
      print("DEBUG : read %d candidates from candidate file %s" % (index,filename))
      
   else :
      print "WARNING : empty or non-existing file %s" % (filename)      

   
   return sources

   
def read_text_file( filename ) :
   sources=[]

   if os.path.exists(filename) and os.stat(filename).st_size > 0 :
      file=open(filename,'r')
      data=file.readlines()
      index=0
      for line in data :
         words = line.split(' ')
         if line[0] == '#' :
            continue
         
         if line[0] != "#" :
            # # FITS DM Fluence[Jy ms] Flux[Jy] START-TIMEINDEX END-TIMEINDEX
            # 1322310160_20211130122222_ch143_02_out_t_genfrb0000.fits 200.00 4.45 20.00 6727 6761
            fits_file  = (words[0+0])
            dm         = float(words[1+0])
            fluence    = float(words[2+0])
            flux       = float(words[3+0])
            start_time = float(words[4+0])
            end_time = float(words[5+0])
         
            sources.append( Source( index, fits_file, dm, fluence, flux, start_time, end_time=end_time ) )
            index += 1
      file.close()
      
   else :
      print "WARNING : empty or non-existing file %s" % (filename)      

   
   return sources

def check_fredda_detection( simul_frb, radius=10 , snr_threshold = 5.00, min_count=1, options=None ):
   margin = 10 
   base_fits_file = simul_frb.fits_file.replace(".fits","")
   cand_file = simul_frb.fits_file.replace(".fits",".cand")
   fredda_cand_file = ("%s/%s/%s" % (options.fredda_subdir,base_fits_file, cand_file))
   
   print("Checking candidate %d in fredda candidate file %s" % (simul_frb.index,fredda_cand_file))
   
   candidates = read_fredda_candidates( fredda_cand_file )
   print "Checking %d candidates from file %s" % (len(candidates),fredda_cand_file)   
   count_close=0
   
   found = False
   for cand in candidates :
      print("Candidate %d dm = %.4f" % (cand.index,cand.dm))
      
      if cand.start_time >= (simul_frb.start_time-margin) and cand.start_time <= ( simul_frb.end_time + margin ) :
         found = True
         break
   
   return found

def check_detection( simul_frb, radius=10 , snr_threshold = 5.00, min_count=1, backup_series_fits="../series_template.fits" ):
   global g_debug_level

   print 
   print "#################################### FRB %d ####################################" % (simul_frb.index)

   fits_series = simul_frb.fits_file.replace(".fits","_series.fits")
   cand_file=fits_series.replace(".fits",".txt")
   if not os.path.exists( fits_series ) :
      print "WARNING : fits file %s not found trying %s" % (fits_series,backup_series_fits)
      fits_series = backup_series_fits
   
   

   # cd /media/msok/6e241a3a-81a7-4239-b652-829eb527faa4/data/GRB/Inject_Systematic/NEW/dm_1000/fluence_01000$ fitshdr 0512_0512_genfrb0042_series.fits
   cdelt1 = 0.1  # CDELT1
   crval1 = -77  # CRVAL1
   cdelt2 = 1.00 # CDELT2
   crval2 = 100. # CRVAL2   
   
   try :
      fits = pyfits.open( fits_series )
      x_size=fits[0].header['NAXIS1']
      y_size=fits[0].header['NAXIS2']         
      cdelt1=float( fits[0].header['CDELT1'] )
      crval1=float( fits[0].header['CRVAL1'] )
      cdelt2=float( fits[0].header['CDELT2'] )
      crval2=float( fits[0].header['CRVAL2'] )
   except :
      print "WARNING : could not read fits file %s -> using default values for CDELT1, CRVAL1, CDELT2, CRVAL2 which might not be correct" % (fits_series)
   
   print "%s : crval1,cdelt1 = (%.2f,%.2f) , crval2,cdelt2 = (%.2f,%.2f)" % (simul_frb.fits_file,crval1,cdelt1,crval2,cdelt2)
         
   inttime = 0.5 # 0.5 seconds integrations 
   frb_time = simul_frb.start_time * inttime
   frb_dm   = simul_frb.dm
   print "Checking %d frb at timeindex = %d -> frb_time = %.2f seconds , dm = %.2f" % (simul_frb.index,simul_frb.start_time,frb_time,frb_dm)
   
   candidates = read_candidate_file( cand_file )
   print "Checking %d candidates from file %s" % (len(candidates),cand_file)   
   count_close=0
   for cand in candidates :
      cand_time = cand.x*cdelt1 + crval1
      cand_dm   = cand.y*cdelt2 + crval2 
      
      dist = math.sqrt( (cand_time-frb_time)**2 + (cand_dm-frb_dm)**2 )
      
      if g_debug_level > 0 or True :
         print "\t%d : cand_time = %.2f [sec] (x=%.1f), cand_dm = %.2f (y=%.0f), cand_snr = %.2f (threshold = %.2f), distance = %.2f (radius = %.2f)" % (cand.index,cand_time,cand.x,cand_dm,cand.y,cand.snr,snr_threshold,dist,radius)
      
      if dist < radius and cand.snr > snr_threshold :
         count_close += 1
   
   print "Number of candidates exceeding snr = %.2f and closer than %.2f pixels is %d" % (snr_threshold,radius,count_close)

   detected = False      
   if count_close > min_count :
      detected = True

   return count_close
   
      
   
def parse_options():
   usage="Usage: %prog [options]\n"
   usage+='\tCalculates efficiency of FRB findining algorithm\n'
   parser = OptionParser(usage=usage,version=1.00)
   parser.add_option('-o','--out','--outfile','--outf',dest="outfile",default="verified.txt", help="Output file with verification of generated events vs. candidates [default %default]",metavar="STRING")
#   parser.add_option('-r','--regfile',dest="regfile",default="avg_I_without.reg", help="Reg file to save GLEAM sources to [default %default]",metavar="STRING")
   parser.add_option('--fredda',action="store_true",dest="fredda_cand_files",default=False, help="FREDDA format of candidate files [default %s]")
   parser.add_option('--fredda_subdir','--subdir',dest="fredda_subdir",default="frb/", help="FREDDA subdirectory [default %s]",metavar="STRING")
   parser.add_option('--tag','--label',dest="label",default="UNKNOWN", help="Label to be printed in front of resulting efficiency [default %s]",metavar="STRING")
   parser.add_option('--snr_threshold','--threshold_snr','--snr',dest="snr_threshold",default=5.00,help="Threshold above which we consider detections (should depend on number of expected events from the noise) [default %s]",type="float")
   (options, args) = parser.parse_args()

   return (options, args)
   
      

if __name__ == '__main__':
   simul_log="generated.txt"
   if len(sys.argv) > 1:   
      simul_log = sys.argv[1]


   (options, args) = parse_options()     
   
   print "#################################################################"
   print "PARAMETERS :"
   print "#################################################################"
   print "Generator log = %s" % (simul_log)
   print "Label         = %s" % (options.label)
   print "snr_threshold = %.2f" % (options.snr_threshold)
   print "Fredda candidate files = %s" % (options.fredda_cand_files)
   print "Fredda subdir          = %s" % (options.fredda_subdir)
   print "#################################################################"
   
   generated_frbs = read_text_file( simul_log )   
   
   out_f = open( options.outfile, "w" )
   out_line = "# INDEX FITS DM Fluence[Jy ms] Flux[Jy] START-TIMEINDEX CLOSE_CANDIDATES"
   
   detected = 0
   ok_count = 0
   for simul_frb in generated_frbs :
      print
      try : 
         found = False
         if options.fredda_cand_files :
            found = check_fredda_detection( simul_frb , snr_threshold=options.snr_threshold, options=options )
         else :
            found = check_detection( simul_frb , snr_threshold=options.snr_threshold )
         
         print "Checking file %s (fluence=%.2f,start_time=%.2f,dm=%.2f) -> detected = %d" % (simul_frb.fits_file,simul_frb.fluence,simul_frb.start_time,simul_frb.dm,found)
         ok_count += 1
         if found > 0 :
            detected = detected + 1

         out_line = "%d %s %.2f %.2f %.2f %.1f %d" % (simul_frb.index,simul_frb.fits_file,simul_frb.dm,simul_frb.fluence,simul_frb.flux,simul_frb.start_time,found)
         out_f.write( out_line + "\n" )
         
      except :
         print "ERROR : could not check simulated FRB %d" % (simul_frb.index)
         continue
         
   out_f.close()

   eff = 0.00
   if ok_count != 0 :   
      eff = float(detected) / float(ok_count)
   print "%s : efficiency = %d / %d = %.2f %%" % (options.label,detected,ok_count,eff*100.00)
   
   
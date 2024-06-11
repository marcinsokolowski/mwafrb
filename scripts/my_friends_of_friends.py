# import pdb

import copy
import math
import sys
import os
import errno
from optparse import OptionParser,OptionGroup
import errno
import getopt
import optparse
import numpy

from datetime import datetime
import time
from astropy.time import Time


def parse_options(idx):
   parser=optparse.OptionParser()
   parser.set_usage("""parse_pulsars.py""")
   parser.add_option("--group_radius","--group_radius_timesteps",dest="group_radius_timesteps",default=1000,help="Group radius in timesteps [default: %default]",type="int")
   parser.add_option("--outfile","--out_script","--outf","-o",dest="outfile",default="merged_fredda.cand",help="Name of output file [default: %default]")
   parser.add_option("--stepfile","--step_file",dest="stepfile",default=None,help="File with steps vs. timeindex  [default: %default]")
   parser.add_option("--step_radius","--step_radius_timesteps","--stepradius",dest="step_radius_timesteps",default=10000,help="Step radius in timesteps [default: %default]",type="int")
   parser.add_option('--frbsearch_format','--frbsearch_input',action="store_true",dest="frbsearch_input",default=False, help="Format of input files, True-frb_search, False-FREDDA [default %default]")
   parser.add_option('-d','--debug','--verbose',action="store_true",dest="debug",default=False, help="Debug mode [default %default]")
   parser.add_option('--print_max_snr_range',action="store_false",dest="print_as_is",default=True, help="For each merged candidate print time range of top 10 SNR candidates in this range [default %default]")
   parser.add_option("--pulse_list_file","--pulse_list",'-p',dest="pulse_list_file",default=None,help="File with list of known pulses to check for FREDDA false-positives [default: %default]")
   parser.add_option("--time_res","--time_res_sec","--timeres_sec",dest="timeres_sec",default=0.001,help="Time resolution in seconds  [default: %default]",type="float")
#   parser.add_option("--station","-s",dest="station",default="aavs2",help="Station name [default: %default]")
#   parser.add_option("--max_sun_elev","--max_sun","--sun_max",dest="max_sun_elev",default=20,help="Max Sun elevation [default: %default]",type="float")
   (options,args)=parser.parse_args(sys.argv[idx:])

   return (options, args)



class cFreddaCandidate :
    def __init__(self, _timestep=0, _snr=0.00, _dm=-1.00, _idt=-1, _max_total_power=-1):
       self.timestep   = _timestep
       self.min_timestep = _timestep
       self.max_timestep = _timestep
       self.snr        = _snr 
       self.dm         = _dm
       self.idt        = _idt 
       self.timestep_sum = 0 # only for what is on the self.cand_list
       self.count      = 0   # only for what is on the self.cand_list
       self.added      = False
       self.cand_list  = [] # list of candidates this one is a mean of 
       self.bad_data   = False
       self.max_total_power = _max_total_power;

    def __repr__(self) :
       out_str =  ("Candidate timestamp = %d, snr = %.2f , dm = %.2f (idt = %d), count = %d, added = %s" % (self.timestep,self.snr,self.dm,self.idt,self.count,self.added))
       return out_str

    def __str__(self) :
       return "member of cFreddaCandidate"

    def belongs( self, check_cand, t_radius=1000 ) :
       if check_cand.timestep >= ( self.min_timestep-t_radius) and check_cand.timestep <= ( self.max_timestep+t_radius) :
          return True
       
       return False
   
#    def is_added( self, check_cand ) :
#       for cand in self.cand_list :
#          if cand.timestep == check_cand.timestep :
#             return cand;
#       
#       return None
       
#    def belongs( self, check_cand, t_radius=1000 ) :
#       cand_added = self.is_added( check_cand )
#       if cand_added is None :
#          if check_cand.timestep >= ( self.min_timestep-t_radius) and check_cand.timestep <= ( self.max_timestep+t_radius) :
#             # in the range enlarged by t_radius
#             return True                    
#       else :
#          if check_cand.snr != cand_added.snr or check_cand.dm != cand_added.dm :
#             # if different dm or snr for the same timestamp
#             return True
#          
#       return False
       
    def add( self, new_cand ) :
       if not new_cand.added : # if not already added :
          self.timestep_sum += new_cand.timestep
          
          # update min and max timestamp range :
          updated = False
          if new_cand.timestep < self.min_timestep :
             self.min_timestep = new_cand.timestep
             updated = True
             
          if new_cand.timestep > self.max_timestep :
             self.max_timestep = new_cand.timestep
             updated = True 
             
          self.count += 1
          self.timestep = self.timestep_sum / self.count
          if new_cand.snr > self.snr :
             self.snr = new_cand.snr
             self.dm  = new_cand.dm
             self.idt = new_cand.idt
             self.max_total_power = new_cand.max_total_power
             
          self.cand_list.append( copy.copy(new_cand) )   
          new_cand.added = True
          
          len_test=len(self.cand_list)
          
          print("DEBUG : candidate time range updated to %.1f - %.1f , added candidate (SNR=%.2f, DM=%.4f vs. added SNR=%.2f, DM=%.4f) at time = %.1f" % (self.min_timestep,self.max_timestep,new_cand.snr,new_cand.dm,self.cand_list[len_test-1].snr,self.cand_list[len_test-1].dm,new_cand.timestep))
          
          return True
          
       return False
      
    def get_snrs( self ) :
       snr_list = []
       i = 0 
       for cand in self.cand_list :
          # print("DEBUG : %d : get_snrs adding %.4f" % (i,cand.snr))
          snr_list.append( cand.snr )
          i = i + 1
          
       return numpy.array(snr_list)
       
       
       
    def get_maxsnr_range( self , top_n=-1 ) : # was top_n=10
       snr_list = self.get_snrs()
       snr_sorted = numpy.argsort( snr_list ) # WARNING : sorted from MIN to MAX !!!
       # snr_sorted = numpy.sort( snr_list )
       
       min_time = 1e20
       max_time = -1e20
       max_snr  = -1
       max_dm   = -1
       min_dm   = 1000000.00
       max_idt  = -1
       min_idt  = 10000000
       max_total_power = -1       
       
       max_i = len(snr_sorted)
       if top_n > 0 :
          if max_i > top_n :
             max_i = top_n
          
       print("DEBUG : get_maxsnr_range : %.4f (at %d) - %.4f (at %d), snr = %.4f" % (self.cand_list[snr_sorted[0]].snr,snr_sorted[0],self.cand_list[snr_sorted[len(snr_sorted)-1]].snr,snr_sorted[len(snr_sorted)-1],self.snr))   
          
       lowest_index = len(snr_sorted)-1-max_i
       if lowest_index < 0 :
          lowest_index = 0   
   
# 2024-05-19 : do not do this top 10 sorted by SNR because the highest SNR candidates have ZERO DM and this is not super informative !   
#       i = len(snr_sorted)-1
#       while i >= lowest_index :
       i = 0
       while i < len(snr_sorted) :
          idx = snr_sorted[i]
          cand = self.cand_list[ idx ]
          # cand = snr_sorted[i]
          print("\tDEBUG snr = %.4f ( index = %d ), dm = %.4f" % (cand.snr,idx,cand.dm))
          
          if cand.timestep < min_time :
             min_time = cand.timestep
       
          if cand.timestep > max_time :
             max_time = cand.timestep
             
          if cand.snr > max_snr :
             max_snr = cand.snr
             
          if cand.dm > max_dm :
             max_dm = cand.dm

          if cand.dm < min_dm :
             min_dm = cand.dm

          if cand.idt > max_idt :
             max_idt = cand.idt
             
          if cand.idt < min_idt :
             min_idt = cand.idt
             
          if cand.max_total_power > max_total_power :
             max_total_power = cand.max_total_power
             
          # i = i - 1   
          i = i + 1

       print("\treturn (%.4f,%.4f,%.4f,%.4f,%.4f,%d,%d,%.4f)" % (min_time,max_time,max_snr,min_dm,max_dm,min_idt,max_idt,max_total_power))             
       return (min_time,max_time,max_snr,min_dm,max_dm,min_idt,max_idt,max_total_power)
       


def read_file(file,verb=False,frbsearch_input=False) :
   cand_list = []

   f = open(file)
   data=f.readlines()

   for line in data : 
      words = line.split(' ')
      if verb :
         print("%d : %s %s" % (len(words),line,words[0+0]))

      if words[0+0] == "#" :
         continue
         
      t = 0
      snr = 0
      dm = 0
      idt = 0 
         
      if frbsearch_input : # input as from frb_search package 
         # TIME  DM  SNR  N_PIX
         # 201.3 10.0 11.4423 0 20277.0 0.0
         # t = int( float( words[0+0] ) ) why it was int() ??? use float !
         t = float( words[0+0] ) # 2023-07-14 - changed to float so that I can read time in seconds too 
         snr = float( words[2+0] )
         dm  = float( words[1+0] )
      else : # normal FREDDA output :
         # S/N, sampno, secs from file start, boxcar, idt, dm, beamno, mjd
         # 10.56 642 6.4200 3 0 0.00 0 40587.694168436 
         t   = int( float(words[1+0]) )
         snr = float(words[0+0])
         dm  = float(words[5+0])
         idt = int( float(words[4+0]) )
         max_total_power = float( words[10+0] )

      # print("DEBUG : DM = %.4f" % (dm))         
      cand = cFreddaCandidate( _timestep=t, _snr=snr, _dm=dm, _idt=idt, _max_total_power=max_total_power )

      cand_list.append( copy.copy(cand) )

   f.close()

   print("Read %d candidates from file %s" % (len(cand_list),file))

   return (cand_list)
   
def read_step_file(file,verb=False,step_radius=-1) :
   timeidx_list = []
   step_list = []

   f = open(file)
   data=f.readlines()

   for line in data : 
      words = line.split(' ')
      if verb :
         print("%d : %s %s" % (len(words),line,words[0+0]))

      if words[0+0] == "#" :
         continue

      t    = float( words[0+0] )
      step = int(words[1+0])

      timeidx_list.append( t )
      step_list.append( step )

   f.close()

   print("Read %d steps from file %s" % (len(timeidx_list),file))
   
   if step_radius > 0 :
      l = len( timeidx_list )
      i = 0 
      while i < l :
         if step_list[i] > 0 :
            for k in range(i-10,i+step_radius) :
               if k >= 0 and k < l :
                  step_list[k] = step_list[i]
            
            i = i + step_radius
         else :
            i += 1

   return (numpy.array(timeidx_list),numpy.array(step_list))
   

def add( list , new_cand ) :
   list.append( copy.copy(new_cand) )
   l = len(list)
   list[l-1].add( new_cand )
   

def friends_of_friends( cand_list, radius=1000, debug=False ) :
   print("INFO : friends_of_friends , radius = %.1f" % (radius))
   print_modulo=100
   if debug :
      print_modulo=1
      
   iter = 0
   out_list = []
   i=0
   l = len(cand_list)

   # add first candidate to the list, which is ok (not bad_data):
   while i < l :
      first_cand = cand_list[i]
      if not first_cand.bad_data :
         print("DEBUG : added first candidate i = %d , timeindex = %d" % (i,first_cand.timestep))
         add( out_list, first_cand )   
         break
         
      i += 1
   
   while i<len(cand_list) : # end when i is at the last candidate on the list 
      print("\tprogress iter=%d , i = %d / %d , number of output candidates %d" % (iter,i,len(cand_list),len(out_list)))
      # group_radius_timesteps

      j=0
      added = 0 # anything added in this iteration       
      try_again = True # means that we should try again to add to the current list 

      # iterate over and try to cluster to the current candidate until nothing is added in an iteration      
      while try_again :
         j=0
         added = 0 # anything added in this iteration       
               
         # ignore bad data :
#         if False : # cand_list[i].bad_data :
#            print("DEBUG : bad candidate at i = %d , timeindex = %d skipped" % (i,cand_list[i].timestep))
#            i += 1
#            continue
      
         while j < len(cand_list) :
            cand = cand_list[j]
#            if False : # cand.bad_data:
#               print("DEBUG : bad candidate at j = %d , timeindex = %d skipped" % (j,cand_list[j].timestep))
#               j += 1
#               continue         
         
#            if j == 252 :
#               print("odo")
            
            if not cand.added : 
               for out_cand in out_list :
                  if out_cand.belongs( cand , t_radius=radius ) :
                     out_cand.add( cand )
                     added += 1
            
            if ( j %  print_modulo ) == 0 and debug : # was 100
               print("DEBUG : i = %d (t = %d) , j = %d , added = %d, len(out_list) = %d" % (i,cand_list[i].timestep,j,added,len(out_list)))
            j = j + 1
   
         iter += 1
      
         print("PROGRESS (iter = %d) : i = %d -> added %d , range [%d - %d] , len(out_list) = %d" % (iter,i,added,out_cand.min_timestep,out_cand.max_timestep,len(out_list)))
         
         try_again = False
         if added > 0 :
            try_again = True
      
      if added <= 0 :
         # means that we've added all candidates belonging to i-th candidate and we can go to next one 
         while i<len(cand_list) and cand_list[i].added :
            # skip all added 
            i += 1
            
         # adding new ROOT candidate (if possible) :   
         if i<len(cand_list) :
            add( out_list, cand_list[i] )
      # else : 
      # we do not change i until all candidates belonging to it are added
      
   print("Final list has %d merged candidates" % (len(out_list)))         

   return (out_list)   
   
if __name__ == '__main__':   
   file="i_00000.cand"
   if len(sys.argv) > 1:
      file = sys.argv[1]

   (options, args) = parse_options(1)

   (cand_list) = read_file( file , frbsearch_input=options.frbsearch_input )
   
   step_times = []
   step_steps = []
   if options.stepfile is not None and len(options.stepfile)>0 :
      (step_times,step_steps) = read_step_file( options.stepfile, step_radius = options.step_radius_timesteps )
      print("PROGRESS : read %d/%d elements from file %s" % (len(step_times),len(step_steps),options.stepfile))
      
      if len(step_steps) > 0 :
         for i in range(0,len(cand_list)) :
            cand = cand_list[i]
            if cand.timestep < len(step_steps) :
               if step_steps[cand.timestep] > 0 :
                  print("DEBUG : candidate at time index = %d flagged due to step" % (cand.timestep))
                  cand.bad_data = True
            else :
               print("ERROR : timestep = %d outside range %d" % (cand.timestep,len(step_steps)))
         
         print("DEBUG : before flagging : %d candidates" % (len(cand_list)))      
         new_cand_list = []
         for i in range(0,len(cand_list)) :
            cand = cand_list[i]
            if not cand.bad_data :
               new_cand_list.append( copy.copy(cand) )
         
         cand_list = new_cand_list
         print("DEBUG : after flagging : %d candidates" % (len(cand_list))) 
         
            
   
   print("PROGRESS : finding Friends-of-Friends:")
   (out_list)  = friends_of_friends( cand_list, radius=options.group_radius_timesteps, debug=options.debug )

   # TODO : output center , time range etc 
   out_f = open( options.outfile , "w" )   
   line = "# INDEX  SNR   DM   TIMESTAMP   |TIMESTAMP_RANGE|    FILFILE  MIN_DM MAX_DM MIN_IDT MAX_IDT MAX_TOTAL_POWER"
   out_f.write( line + "\n" )
#   for cand in out_list :
   for i in range(0,len(out_list)) :   
      cand = out_list[i]
      
      # get time range from top 10 SNR candidates :
      min_dm = -1
      max_dm = -1.00
      min_idt = -1
      max_idt = -1
      max_snr = -1.00
      max_total_power = -1
      if options.print_as_is :
         min_time = cand.min_timestep
         max_time = cand.max_timestep
         (a,b,max_snr,min_dm,max_dm,min_idt,max_idt,max_total_power) = cand.get_maxsnr_range()
      else :
         (min_time,max_time,max_snr,min_dm,max_dm,min_idt,max_idt,max_total_power) = cand.get_maxsnr_range()   
#      line = ("%05d : %06.2f %08.2f %012.4f  |%012.4f - %012.4f|   %s" % (i,cand.snr,cand.dm,(cand.max_timestep+cand.min_timestep)/2.00,cand.min_timestep,cand.max_timestep,file))
      line = ("%05d : %06.2f %08.2f %012.4f  |%012.4f - %012.4f|   %s %08.2f %08.2f %04d %04d %.4f" % (i,max_snr,max_dm,(cand.max_timestep+cand.min_timestep)/2.00,min_time,max_time,file,min_dm,max_dm,min_idt,max_idt,max_total_power))
      print("%s" % (line))
      out_f.write( line + "\n" )
            
   out_f.close()
   
   # check for false positives 
   if options.pulse_list_file is not None :
      # def read_file(file,verb=False,frbsearch_input=False) :
      real_pulse_list = read_file( options.pulse_list_file,verb=True,frbsearch_input=True)      
      
      true_pulses = 0
      false_positives = 0
      
      for i in range(0,len(out_list)) :   
         cand = out_list[i]
         
         cand_start_time = cand.min_timestep*options.timeres_sec - 0.1 
         cand_end_time = cand.max_timestep*options.timeres_sec  + 0.1 
         
         print("Checking candidates at %.6f - %.6f" % (cand_start_time,cand_end_time))

         found=False
         for real_pulse in real_pulse_list :            
            # print("\tcompare with %.6f [sec]" % (real_pulse.timestep))
            if cand_start_time <= real_pulse.timestep and real_pulse.timestep <= cand_end_time :
               # event caused by a real pulse 
               print("\tFound true pulse at %.6f [sec]" % (real_pulse.timestep))
               found=True
               break
        
         if found :
            true_pulses += 1
         else :
            false_positives += 1 
                       

      print("Fredda found %d real pulses (confirmed on the list)" % (true_pulses))
      print("Fredda found %d false-positives" % (false_positives))
         



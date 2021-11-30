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
#   parser.add_option("--station","-s",dest="station",default="aavs2",help="Station name [default: %default]")
#   parser.add_option("--max_sun_elev","--max_sun","--sun_max",dest="max_sun_elev",default=20,help="Max Sun elevation [default: %default]",type="float")
   (options,args)=parser.parse_args(sys.argv[idx:])

   return (options, args)



class cFreddaCandidate :
    def __init__(self, _timestep=0, _snr=0.00, _dm=-1.00):
       self.timestep   = _timestep
       self.min_timestep = _timestep
       self.max_timestep = _timestep
       self.snr        = _snr 
       self.dm         = _dm
       self.timestep_sum = 0 # only for what is on the self.cand_list
       self.count      = 0   # only for what is on the self.cand_list
       self.added      = False
       self.cand_list  = [] # list of candidates this one is a mean of 
       self.bad_data   = False

    def __repr__(self) :
       out_str =  ("Candidate timestamp = %d, snr = %.2f , dm = %.2f, count = %d, added = %s" % (self.timestep,self.snr,self.dm,self.count,self.added))
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
          if new_cand.timestep < self.min_timestep :
             self.min_timestep = new_cand.timestep
          if new_cand.timestep > self.max_timestep :
             self.max_timestep = new_cand.timestep
             
          self.count += 1
          self.timestep = self.timestep_sum / self.count
          if new_cand.snr > self.snr :
             self.snr = new_cand.snr
             self.dm  = new_cand.dm
          self.cand_list.append( copy.copy(new_cand) )   
          new_cand.added = True
          
          return True
          
       return False
      
    def get_snrs( self ) :
       snr_list = []
       for cand in self.cand_list :
          snr_list.append( cand.snr )
          
       return numpy.array(snr_list)
       
       
       
    def get_maxsnr_range( self , top_n=10 ) :
       snr_list = self.get_snrs()
       snr_sorted = numpy.argsort( snr_list )
       
       min_time = 1e20
       max_time = -1e20
       max_snr  = -1
       max_dm   = -1
       
       max_i = len(snr_sorted)
       if max_i > top_n :
          max_i = top_n
       for i in range(0,max_i) :
          idx = snr_sorted[i]
          cand = self.cand_list[ idx ]
          
          if cand.timestep < min_time :
             min_time = cand.timestep
       
          if cand.timestep > max_time :
             max_time = cand.timestep
             
          if cand.snr > max_snr :
             max_snr = cand.snr
             
          if cand.dm > max_dm :
             max_dm = cand.dm
             
       return (min_time,max_time,max_snr,max_dm)
       


def read_file(file,verb=False) :
   cand_list = []

   f = open(file)
   data=f.readlines()

   for line in data : 
      words = line.split(' ')
      if verb :
         print("%d : %s %s" % (len(words),line,words[0+0]))

      if words[0+0] == "#" :
         continue

      t   = int( words[1+0] )
      snr = float(words[0+0])
      dm  = float(words[5+0])
      cand = cFreddaCandidate( _timestep=t, _snr=snr, _dm=dm )

      cand_list.append( copy.copy(cand) )

   f.close()

   print "Read %d candidates from file %s" % (len(cand_list),file)

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

   print "Read %d steps from file %s" % (len(timeidx_list),file)
   
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
   iter = 0
   out_list = []
   i=0
   l = len(cand_list)
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
      
      # ignore bad data :
#      if False : # cand_list[i].bad_data :
#         print("DEBUG : bad candidate at i = %d , timeindex = %d skipped" % (i,cand_list[i].timestep))
#         i += 1
#         continue
      
      while j < len(cand_list) :
         cand = cand_list[j]
#         if False : # cand.bad_data:
#            print("DEBUG : bad candidate at j = %d , timeindex = %d skipped" % (j,cand_list[j].timestep))
#            j += 1
#            continue         
         
#         if j == 252 :
#            print("odo")
            
         if not cand.added : 
            for out_cand in out_list :
               if out_cand.belongs( cand , t_radius=radius ) :
                  out_cand.add( cand )
                  added += 1
            
         if ( j % 100 ) == 0 and debug :
            print("DEBUG : i = %d (t = %d) , j = %d , added = %d, len(out_list) = %d" % (i,cand_list[i].timestep,j,added,len(out_list)))
         j = j + 1
   
      iter += 1
      
      print("PROGRESS : i = %d -> added %d" % (i,added))
      
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

   (cand_list) = read_file( file )
   
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
         
            
   
   print("PROGRESS : finding Friends-of-Friends")
   (out_list)  = friends_of_friends( cand_list, radius=options.group_radius_timesteps )

   # TODO : output center , time range etc 
   out_f = open( options.outfile , "w" )   
   line = "# INDEX  SNR   DM   TIMESTAMP   |TIMESTAMP_RANGE|    FILFILE"
   out_f.write( line + "\n" )
#   for cand in out_list :
   for i in range(0,len(out_list)) :   
      cand = out_list[i]
      
      # get time range from top 10 SNR candidates :
      (min_time,max_time,max_snr,max_dm) = cand.get_maxsnr_range(top_n=10)
   
#      line = ("%05d : %06.2f %08.2f %012.4f  |%012.4f - %012.4f|   %s" % (i,cand.snr,cand.dm,(cand.max_timestep+cand.min_timestep)/2.00,cand.min_timestep,cand.max_timestep,file))
      line = ("%05d : %06.2f %08.2f %012.4f  |%012.4f - %012.4f|   %s" % (i,max_snr,max_dm,(cand.max_timestep+cand.min_timestep)/2.00,min_time,max_time,file))
      print("%s" % (line))
      out_f.write( line + "\n" )
      
      
   out_f.close()


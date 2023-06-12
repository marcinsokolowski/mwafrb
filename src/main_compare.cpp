#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "SigprocFile.h"

string gOutDir="spectra/";
int    gSaveSpectra = 0;

void usage()
{
   printf("dumpfilfile_float test.fil\n");   
}

int main(int argc,char* argv[])
{
   if( argc<2 || (argc==2 && strcmp(argv[1],"-h")==0) ){
      usage();
      exit(0);
   }

   string filfilename1 = argv[1];
   string filfilename2 = argv[2];
   string outfile = "avg_spectrum.txt";


   SigprocFile filfile1( filfilename1.c_str() );
   SigprocFile filfile2( filfilename2.c_str() );
   
   printf("File1 : %s\n", filfile1.name() );
   printf("File2 : %s\n", filfile2.name() );
   printf("-------------------------------------------\n");
/*   printf("nifs   = %d\n",filfile.nifs());
   printf("nbits  = %d\n",filfile.nbits());
   printf("nbeams = %d\n",filfile.nbeams());
   printf("nchans = %d\n",filfile.nchans());
   printf("npols  = %d\n",filfile.npols());
   printf("nants  = %d\n",filfile.nants());
   printf("samples_read = %d\n",filfile.samples_read());
   printf("current_sample = %d\n",filfile.current_sample());
   printf("fch1 = %.4f\n",filfile.fch1());
   printf("foff = %.4f\n",filfile.foff());
   printf("tstart = %.4f\n",filfile.tstart());
   printf("tsamp  = %.6f\n",filfile.tsamp());*/

   if( filfile1.nifs() != filfile2.nifs() ){
      printf("nifs() differ : %d != %d\n",filfile1.nifs(),filfile2.nifs() );
   }
   if( filfile1.nbits() != filfile2.nbits() ){
      printf("nbits() differ : %d != %d\n",filfile1.nbits(),filfile2.nbits() );
   }
   if( filfile1.nbeams() != filfile2.nbeams() ){
      printf("nbeams() differ : %d != %d\n",filfile1.nbeams(),filfile2.nbeams() );
   }
   if( filfile1.nchans() != filfile2.nchans() ){
      printf("nchans() differ : %d != %d\n",filfile1.nchans(),filfile2.nchans() );
      printf("WARNING : different number of channels -> cannot compare data !\n");
      exit(-1);
   }
   
   int headers_diffs = 0;
   
   if( filfile1.npols() != filfile2.npols() ){
      printf("npols() differ : %d != %d\n",filfile1.npols(),filfile2.npols() );
      headers_diffs++;
   }

   if( filfile1.nants() != filfile2.nants() ){
      printf("nants() differ : %d != %d\n",filfile1.nants(),filfile2.nants() );
      headers_diffs++;
   }
   if( filfile1.samples_read() != filfile2.samples_read() ){
      printf("samples_read() differ : %d != %d\n",int(filfile1.samples_read()),int(filfile2.samples_read()) );
      headers_diffs++;
   }
   if( filfile1.current_sample() != filfile2.current_sample() ){
      printf("current_sample() differ : %d != %d\n",int(filfile1.current_sample()),int(filfile2.current_sample()) );
      headers_diffs++;
   }
   if( filfile1.fch1() != filfile2.fch1() ){
      printf("fch1() differ : %.8f != %.8f\n",filfile1.fch1(),filfile2.fch1() );
      headers_diffs++;
   }
   if( filfile1.foff() != filfile2.foff() ){
      printf("foff() differ : %.8f != %.8f\n",filfile1.foff(),filfile2.foff() );
      headers_diffs++;
   }
   if( filfile1.tstart() != filfile2.tstart() ){
      printf("tstart() differ : %.8f != %.8f\n",filfile1.tstart(),filfile2.tstart() );
      headers_diffs++;
   }
   if( fabs( filfile1.tsamp() - filfile2.tsamp() ) > 0.000000001 ){
      printf("tsamp() differ : %.8f != %.8f\n",filfile1.tsamp(),filfile2.tsamp() );
      headers_diffs++;
   }
  
   printf("Number of differences in headers = %d\n",headers_diffs);
   
   if( headers_diffs <= 0 ){
      printf("Headers the same :\n");
      printf("\tnifs   = %d\n",filfile1.nifs());
      printf("\tnbits  = %d\n",filfile1.nbits());
      printf("\tnbeams = %d\n",filfile1.nbeams());
      printf("\tnchans = %d\n",filfile1.nchans());
      printf("\tnpols  = %d\n",filfile1.npols());
      printf("\tnants  = %d\n",filfile1.nants());
      printf("\tsamples_read = %d\n",int(filfile1.samples_read()));
      printf("\tcurrent_sample = %d\n",int(filfile1.current_sample()));
      printf("\tfch1 = %.8f (%.8f)\n",filfile1.fch1(),filfile2.fch1());
      printf("\tfoff = %.4f\n",filfile1.foff());
      printf("\ttstart = %.4f\n",filfile1.tstart());
      printf("\ttsamp  = %.6f\n",filfile1.tsamp());
   }

   int n_sample_size = filfile1.nchans();
   float* buffer1 = new float[n_sample_size];
   float* buffer2 = new float[n_sample_size];

   int samples_read1 = 0;
   int samples_read2 = 0;
   int diff_count    = 0;
   int iteration     = 0;

   while( (samples_read1 = filfile1.read_samples_uint8( 1, (uint8_t*)buffer1 )) > 0  && (samples_read2 = filfile2.read_samples_uint8( 1, (uint8_t*)buffer2 )) >0 ){
       printf("Iteration %d : read %d and %d samples from both files\n",iteration,samples_read1,samples_read2);
       
       for(int i=0;i<n_sample_size;i++){
          double val1 = buffer1[i];
          double val2 = buffer2[i];
          
          if( fabs(val1-val2) > 0.000000001 ){
              printf("\tDIFFERENCE : at channel = %d %.8f != %.8f\n",i,val1,val2);
              diff_count++;
          }
       }
       
       iteration++;
   }
   printf("------------------------------------------------\n");
   printf("STATISTICS :\n");
   printf("------------------------------------------------\n");
   printf("Diff count = %d\n",diff_count);
   printf("------------------------------------------------\n");

   delete [] buffer1;
   delete [] buffer2;
}

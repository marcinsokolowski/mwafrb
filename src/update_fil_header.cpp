#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <string>
#include<bg_fits.h>
#include <bg_globals.h>


#include "SigprocFile.h"


string gOutDir="spectra/";
int    gSaveSpectra = 0;
string gOutFile="updated.fil";

int gWriteFlipped=0;
int gFlipData=0;
int gFlipHeader=0;

void usage()
{
   printf("update_fil_header test.fil KEY VALUE\n");   
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "hfFHn:o:";
   int opt,opt_param,i;

   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'o':
            if( optarg ){   
               gOutFile = optarg;
            }
            break;

         default:
            fprintf(stderr,"Unknown option %c\n",opt);
            usage();
      }
   }
}

void print_parameters( string& filfilename )
{
  printf("#####################################\n");
  printf("PARAMETERS :\n");
  printf("#####################################\n");
  printf("Input .fil file   = %s\n",filfilename.c_str());
  printf("Out file          = %s\n",gOutFile.c_str());
  printf("#####################################\n");
  fflush(stdout);
}


int main(int argc,char* argv[])
{
   if( argc<2 || (argc==2 && strcmp(argv[1],"-h")==0) ){
      usage();
      exit(0);
   }

   string filfilename = argv[1];
   
   // parsing and printing paramters :
   parse_cmdline(argc-1,argv+1);
   print_parameters( filfilename );


   SigprocFile filfile( filfilename.c_str() );
   
   printf("File : %s\n", filfile.name() );
   printf("-------------------------------------------\n");
   printf("nifs   = %d\n",filfile.nifs());
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
   printf("tsamp  = %.6f\n",filfile.tsamp());

   // SetHeaderValue( const char* keyword, double value )
   filfile.SetHeaderValue( "tstart", 58323.2563+(8000.00+1)*0.001000 );
      
   if( filfile.Write( gOutFile.c_str() ) ){
      printf("ERROR : could not write header file -> exiting now !\n");
      exit(-1);
   }
}


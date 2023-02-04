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

int gNewTelescopeID = -1; // (int)'u' -> MWA (117 or 85 ASCII)

double gRA_degrees  = 95309.3097;
string gRA_string;
double gDEC_degrees = 75535.75;
string gDEC_string;

void usage()
{
   printf("update_fil_header test.fil KEY VALUE\n");   
   printf("-t TELESCOPE_ID [default %d]\n",gNewTelescopeID);
   printf("-r RA[deg decimal]\n");
   printf("-d DEC[deg decimal]\n");
   printf("-R RA_string  [e.g. 09:53:09.3097]\n");
   printf("-D DEC_string [e.g. +07:55:35.75 ]\n");
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "hfFHn:o:t:r:d:R:D:";
   int opt,opt_param,i;

   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'o':
            if( optarg ){   
               gOutFile = optarg;
            }
            break;

         case 't':
            if( optarg ){   
               gNewTelescopeID = atol( optarg );
            }
            break;

         case 'r':
            if( optarg ){   
               gRA_degrees = atof( optarg );
            }
            break;

         case 'd':
            if( optarg ){   
               gDEC_degrees = atof( optarg );
            }
            break;

         case 'R':
            if( optarg ){   
               gRA_string = optarg;
            }
            break;

         case 'D':
            if( optarg ){   
               gDEC_string = optarg;
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
  printf("Telescope ID      = %d\n",gNewTelescopeID);
  printf("(RA,DEC)          = (%.6f,%.6f)\n",gRA_degrees,gDEC_degrees);
  printf("(RA,DEC) - string = (%s,%s)\n",gRA_string.c_str(),gDEC_string.c_str());
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
   printf("samples_read = %d\n",int(filfile.samples_read()));
   printf("current_sample = %d\n",int(filfile.current_sample()));
   printf("fch1 = %.4f\n",filfile.fch1());
   printf("foff = %.4f\n",filfile.foff());
   printf("tstart = %.4f\n",filfile.tstart());
   printf("tsamp  = %.6f\n",filfile.tsamp());

   // SetHeaderValue( const char* keyword, double value )
//   filfile.SetHeaderValue( "tstart", 58323.2563+(8000.00+1)*0.001000 );
   if( gNewTelescopeID >= 0 ){
      filfile.SetHeaderValue( "telescope_id", gNewTelescopeID ); // "MWA" );
   }
   
   if( fabs(gRA_degrees) > 0.0001 ){ // or isnan
      filfile.SetHeaderValue( "src_raj" , gRA_degrees );
      // filfile.SetHeaderValue( "src_raj" , 95309.3097 ); // see sgiproc.pdf : src raj (double): right ascension (J2000) of source (hhmmss.s)
   }

   if( fabs(gDEC_degrees) > 0.0001 ){ // or isnan
      filfile.SetHeaderValue( "src_dej" , gDEC_degrees );
      // filfile.SetHeaderValue( "src_dej" ,  75535.75 ); // src dej (double): declination (J2000) of source (ddmmss.s)
   }
      
   if( filfile.Write( gOutFile.c_str() ) ){
      printf("ERROR : could not write header file -> exiting now !\n");
      exit(-1);
   }
}


#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <string>
#include <bg_fits.h>
#include <bg_globals.h>


#include "SigprocFile.h"

string gFitsFile="test.fits";
string gOutDir="spectra/";
int    gSaveSpectra = -1;

std::string gNormFile;
vector<cValue> normalisation_spectrum;
string gOutFilFile="out.fil";
string gOutFitsFileName;
string gOutFitsTransposedFileName;
string gOutFitsAvgTransposedFileName;
string gOutTotalPowerFile="total_power_fil.txt";

int gWriteFlipped=0;
int gFlipData=0;
int gFlipHeader=0;

long int gMaxSpectraCount=-1;

int gFloat2UChar=0;

int gVerb=0;

void usage()
{
   printf("fits2fil test.fits test.fil out.fil\n");   
/*   printf("\t-n normalisation_file : file with mean spectrum to use for normalisation [default not set]\n");
   printf("\t-o output normalised fil file [default not required]\n");
   printf("\t-f save flipped file\n");   
   printf("\t-F flip data [default %d]\n",gFlipData);
   printf("\t-H flip header [default %d]\n",gFlipHeader);
   printf("\t-T 255 : truncate to 0 - 255 range and saved 8bits filterbank file\n");   
   printf("\t-S Save_spectrum_index : save spectrum index = %d [only if >= 0]\n",gSaveSpectra);
   printf("\t-L MAX_SPECTRA_COUNT : only save maximum this number of spectra\n");*/
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "hfFHn:o:T:S:L:";
   int opt,opt_param,i;

   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'n':
            if( optarg ){   
               gNormFile = optarg;
            }
            break;

         case 'o':
            if( optarg ){   
               gOutFilFile = optarg;
            }
            break;

         case 'f':
            gWriteFlipped = 1;
            break;

         case 'H':
            gFlipHeader = 1;
            break;
      
         case 'F':
            gFlipData = 1;
            break;

         case 'L':
            gMaxSpectraCount = atol( optarg );
            break;

         case 'S':
            gSaveSpectra = atol( optarg );
            break;

         case 'T':
            gFloat2UChar = atol( optarg );
            break;
      
         default:
            fprintf(stderr,"Unknown option %c\n",opt);
            usage();
      }
   }

   if( strlen(gNormFile.c_str()) > 0 ){
      read_file( gNormFile.c_str() , normalisation_spectrum );
      printf("Read %d points from normalisation file %s\n",int(normalisation_spectrum.size()),gNormFile.c_str());
   }
}

void print_parameters( string& filfilename, string& outfile )
{
  printf("#####################################\n");
  printf("PARAMETERS :\n");
  printf("#####################################\n");
  printf("FITS file          = %s\n",gFitsFile.c_str());
  printf("Input .fil file (just to use the same header)  = %s\n",filfilename.c_str());
  printf("Output .fil file = %s\n",gOutFilFile.c_str());
/*  printf("Norm file          = %s\n",gNormFile.c_str());
  printf("Write flipped      = %d (flip data = %d, flip header = %d)\n",gWriteFlipped,gFlipData,gFlipHeader);
  printf("Float2Uchar        = %d\n",gFloat2UChar);
  printf("Save spectrum      = %d\n",gSaveSpectra);
  printf("Max. spectra count = %d\n",gMaxSpectraCount); 
  printf("Output FITS files :\n");
  printf("\t\t%s\n",gOutFitsFileName.c_str());
  printf("\t\t%s\n",gOutFitsTransposedFileName.c_str());
  printf("\t\t%s\n",gOutFitsAvgTransposedFileName.c_str());*/
  printf("#####################################\n");
  fflush(stdout);
}


int main(int argc,char* argv[])
{
   if( argc<2 || (argc==2 && strcmp(argv[1],"-h")==0) ){
      usage();
      exit(0);
   }

   string fitsfilename = argv[1];
   string filfilename  = argv[2];
   string outfile = argv[3];
   change_ext( filfilename.c_str() , "total_power" ,gOutTotalPowerFile, false );
      
   // parsing and printing paramters :
   parse_cmdline(argc-1,argv+1);
   print_parameters( filfilename, outfile );
   mkdir( gOutDir.c_str() );

   SigprocFile filfile( filfilename.c_str() );
   SigprocFile filfile_out( filfile.nbits(), filfile.nifs(), filfile.nchans(), filfile.fch1(), filfile.foff(), filfile.tstart(), filfile.tsamp() );   

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
   printf("fch1 = %.8f\n",filfile.fch1());
   printf("foff = %.8f\n",filfile.foff());
   printf("tstart = %.4f\n",filfile.tstart());
   printf("tsamp  = %.6f\n",filfile.tsamp());

   if( filfile_out.Write( gOutFilFile.c_str(), filfile, 0, true ) ){
      printf("ERROR : could not write header file -> exiting now !\n");
      exit(-1);
   }
   
   
   CBgFits fits( fitsfilename.c_str() );
   if( fits.ReadFits( fitsfilename.c_str() ) ){
      printf("ERROR : could not read FITS file %s\n",fitsfilename.c_str());
      exit(-1);
   }
   
   int fits_n_channels = fits.GetYSize();
   int fits_n_timesteps = fits.GetXSize();
   float* buffer = new float[fits_n_channels];
   for(int t=0;t<fits_n_timesteps;t++){
      for(int ch=0;ch<fits_n_channels;ch++){
         buffer[ch] = fits.getXY(t,ch);
      }      
      
      filfile_out.WriteData( buffer , fits_n_channels );
   }
   delete [] buffer;

}

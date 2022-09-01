#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <string>
#include <bg_fits.h>
#include <bg_globals.h>


#include "SigprocFile.h"

string gFitsFile="test.fits";
string gOutDir="spectra/";
int    gFreqOffsetSign = -1;

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
string gSourceName;

long int gMaxSpectraCount=-1;

int gFloat2UChar=0;

// int gVerb=0;

// FIL header filling 
bool gUseFitsHeader=false;

void usage()
{
   printf("fits2fil test.fits out.fil\n");   
   printf("\t-u : use FITS file header to fil .fil file header [default %d]\n",gUseFitsHeader);
   printf("\t-S OFFSET_SIGN : frequency offset sign +/-1 to specify if freq. start from low and goes up (+1) or from upper to lower (-1) [default %d]\n",gFreqOffsetSign);
   printf("\t-s SOURCE_NAME [default not set]\n");
   
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
   char optstring[] = "hfFHn:o:T:S:L:us:";
   int opt,opt_param,i;

   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'u':
            gUseFitsHeader = true;
            break;
            
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
            gFreqOffsetSign = atol( optarg );
            break;

         case 's':
            gSourceName = optarg;
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
  printf("Use FITS header  = %d\n",gUseFitsHeader);
  printf("Frequency offset sign = %d\n",gFreqOffsetSign);
  printf("Source name        = %s\n",gSourceName.c_str());
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

   gFitsFile = argv[1];
   gOutFilFile = argv[2];
   string outfile = "out.fil";
   
//   change_ext( filfilename.c_str() , "total_power" ,gOutTotalPowerFile, false );
      
   // parsing and printing paramters :
   parse_cmdline(argc-1,argv+1);
   print_parameters( gFitsFile, outfile );
   mkdir( gOutDir.c_str() );

   // read FITS file earlier to be able to use header information to fill filterbank file header :   
   CBgFits fits( gFitsFile.c_str() );
   if( fits.ReadFits( gFitsFile.c_str() ) ){
      printf("ERROR : could not read FITS file %s\n",gFitsFile.c_str());
      exit(-1);
   }
   printf("DEBUG : FITS file read OK\n");


//   SigprocFile filfile( filfilename.c_str() );
   int nbeams = 1;
   int nants = 1;
   int nbits = 8*sizeof(float); // 8bits/bytes x number of bytes
   int nifs = 1;
   int nchans = 24;
   int npols = 1;
   double fch1 = 109*1.28+0.64; // was 132 but did not match the fact that the 1st channel is at the low freq. end (y=0 is channel = 0 )
   double foff = 1.28;
   double tstart = 0.00;
   double tsamp = 0.01;
//   SigprocFile filfile_out( filfile.nbits(), filfile.nifs(), filfile.nchans(), filfile.fch1(), filfile.foff(), filfile.tstart(), filfile.tsamp() );   

   if( gUseFitsHeader ){
      nchans = fits.GetXSize();      
      double fits_end_freq = fits.start_freq + fits.delta_freq * (nchans-1) + fits.delta_freq/2.00; // very upper band of frequency (not center THE END INDEED)
      printf("INFO : calculated frequency end = %.6f MHz as fits.start_freq + fits.delta_freq * (nchans-1) + fits.delta_freq/2.00 :\n",fits_end_freq);
      printf("\t\t %.6f + %.6f*(%d-1) + %.6f/2.00\n",fits.start_freq,fits.delta_freq,nchans,fits.delta_freq);      
      if( gFreqOffsetSign > 0 ){
         fits_end_freq = fits.start_freq - fits.delta_freq/2.00;
      }
      fch1 = fits_end_freq; // fits.start_freq - fits.delta_freq/2.00; // assuming start frequency 
      foff = gFreqOffsetSign*fits.delta_freq;
      tsamp = fits.inttime;
      tstart = fits.dtime_fs + fits.dtime_fu/1000000.00;
      
      printf("FITS header information:\n");
      printf("\tStart freq = %.8f MHz\n",fits.start_freq);
      printf("\tN channels = %d\n",nchans);
      printf("\tInttime    = %.6f [sec]\n",tsamp);
      printf("\tTstart     = %.6f [uxtime]\n",tstart);
   }

   SigprocFile filfile_out( nbits, nifs, nchans, fch1, foff, tstart, tsamp );
   if( strlen(gSourceName.c_str()) ){
      filfile_out.sourcename( gSourceName.c_str() );
   }

   printf("File : %s\n", gFitsFile.c_str() );
   printf("-------------------------------------------\n");
   printf("nifs   = %d\n",nifs);
   printf("nbits  = %d\n",nbits);
   printf("nbeams = %d\n",nbeams);
   printf("nchans = %d\n",nchans);
   printf("npols  = %d\n",npols);
   printf("nants  = %d\n",nants);
//   printf("samples_read = %d\n",int(samples_read()));
//   printf("current_sample = %d\n",int(current_sample()));
   printf("fch1 = %.8f\n",fch1);
   printf("foff = %.8f\n",foff);
   printf("tstart = %.4f\n",tstart);
   printf("tsamp  = %.6f\n",tsamp);
   printf("source name = %s\n",filfile_out.sourcename());

   printf("DEBUG : writting header ...\n");
   filfile_out.FillHeader();
   int ret = filfile_out.WriteHeader( gOutFilFile.c_str() , false, true );
   printf("DEBUG : header written OK ( %d bytes written )\n",ret);
   
   
   int fits_n_channels = fits.GetYSize();
   int fits_n_timesteps = fits.GetXSize();
   float* buffer = new float[fits_n_channels];
   for(int t=0;t<fits_n_timesteps;t++){
      for(int ch=0;ch<fits_n_channels;ch++){
         buffer[ch] = fits.getXY(t,ch);
      }      
      
      filfile_out.WriteData( buffer , fits_n_channels );
   }
   printf("DEBUG : output .fil file written OK\n");
   filfile_out.Close();
   
   delete [] buffer;

}

// PROGAM WRITTERN BY MARCIN SOKOLOWSKI (May 2019) , marcin.sokolowski@curtin.edu.au
//  It converts raw files in psrdada format into .fil (filterbank) file format 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>

#include <bg_fits.h>
#include "filfile.h"

// #define DADA_HEADER_SIZE 4096

#define NATIVE_N_CHANNELS 1280
#define NATIVE_TIMESTEPS  1000


char gInputFile[1024];
char gOutSpecFile[1024];
char gOutFilFile[1024];

/*
// see /home/msok/Desktop/ASKAP/logbook/fredda_on_J0953.odt
double gMinFITS_Value = -1; // Minimum value = 1401931.37500000
double gMaxFITS_Value = -1; // Maximum value = 5383969.00000000

int gAvgNChannels=0;
double gTimeResInSec = 0.001;
*/


int gNChannels=128;
int gTimeSteps=20970;

// int gVerb=0;

void usage()
{
   printf("fits2fil fits_file out_filfile -s out_specfile -x scaling_factor\n");
   printf("\tdada_file         : input FITS file [default = %s]\n",gInputFile);
   printf("\tout_filfile       : output FIL file [default = %s]\n",gOutFilFile);
   printf("\t-s out_specfile\n : output file with average spectrum [default = %s]\n",gOutSpecFile);
   printf("\t-m min_value in FITS for scaling [default = %.1f\n",CFilFile::gMinFITS_Value);
   printf("\t-x max_value in FITS for scaling [default = %.1f\n",CFilFile::gMaxFITS_Value);
   printf("\t-a avg_n_channels [default %d]\n",gAvgNChannels);   
   printf("\t-c n_channels in .dada file [default %d]\n",gNChannels);
   printf("\t-t N_TIMESTEPS [default %d]\n",gTimeSteps);         
   printf("\t-r TIME_RESOLUTION [default %.2f]\n",gTimeResInSec);
   
   exit(0);
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "dhs:x:m:a:c:t:r:";
   int opt,opt_param,i;
      
   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'c':
            if( optarg ){   
                gNChannels = atol(optarg);
            }
            break;

         case 's':
            if( optarg ){   
                strcpy(gOutSpecFile,optarg);
            }
            break;

         case 'a':
            if( optarg ){   
                gAvgNChannels = atol(optarg);
            }
            break;

         case 'x':
            if( optarg ){   
                CFilFile::gMaxFITS_Value = atof(optarg);
            }
            break;

         case 'm':
            if( optarg ){   
                CFilFile::gMinFITS_Value = atof(optarg);
            }
            break;

         case 'd':
            gVerb++;
            break;
            
         case 'h':   
            usage();
            break;

         case 't':
            if( optarg ){   
               gTimeSteps = atol(optarg);
            }
            break;

         case 'r':
            if( optarg ){   
               gTimeResInSec = atof(optarg);
            }
            break;

            
         default:
            fprintf(stderr,"Unknown option %c\n",opt);
            usage();                                          
      }
   }
}                                                                           

void printf_parameters()
{
  printf("#####################################\n");
  printf("PARAMETERS :\n");
  printf("#####################################\n");
  printf("Input FITS file   = %s\n",gInputFile);
  printf("N channels in input .dada file = %d\n",gNChannels);
  printf("Output .fil file   = %s\n",gOutFilFile);
  printf("Output avg. spectrum file  = %s\n",gOutSpecFile);
  printf("MIN FITS value     = %.8f (for scaling)\n",CFilFile::gMinFITS_Value);
  printf("MAX FITS value     = %.8f (for scaling)\n",CFilFile::gMaxFITS_Value);
  printf("Verb level         = %d\n",gVerb);
  printf("Avg N. channels    = %d\n",gAvgNChannels);
  printf("N timesteps        = %d\n",gTimeSteps);
  printf("Time resolution    = %.6f [seconds]\n",gTimeResInSec);
  printf("#####################################\n");       
}

void fits2fil( const char* filename, const char* szOutFilFile, const char* szOutSpecFile, int n_channels  = 1280, int n_timesteps = 1000  );

int main(int argc,char* argv[])
{
  strcpy(gInputFile,"incoherent_beam_1kHz_1ms.dada");
  strcpy(gOutSpecFile,"incoherent_beam_1kHz_1ms.spectrum");
  strcpy(gOutFilFile,"newfilfile.fil" );

  if( argc<2 || (argc==2 && strncmp(argv[1],"-h",2)==0) ){
    usage();
  }

  if( argc>=2 ){
    strcpy(gInputFile,argv[1]);
  }

  if( argc>=3 ){
    strcpy(gOutFilFile,argv[2]);
  }
  
  // parsing and printing paramters :
  parse_cmdline(argc-2,argv+2);
  printf_parameters();

  CFilFile::fits2fil( gInputFile , gOutFilFile, gOutSpecFile, gNChannels, gTimeSteps ); 
}

/*
void fits2fil( const char* filename, const char* szOutFilFile, const char* szOutSpecFile, int n_channels, int n_timesteps )  
{
  if( gVerb ){
    printf("Reading file %s\n",filename);  
  }
  FILE* f = fopen(filename, "rb");  
  
  FILE* outf=NULL;
  if( strlen(szOutSpecFile) ){
     outf = fopen(szOutSpecFile,"w");
  }

  if( f )
  {  
      int i=0,count=0;
      
      int string_len=4;      
      
      CBgFits infits( filename );
      infits.ReadFits( filename );


      // convert FITS header to FIL header and save :
      cFilFileHeader filheader;      
      n_channels = infits.GetYSize();      
      int final_channels = n_channels;
      
      if( gAvgNChannels > 1 ){
          final_channels = n_channels / gAvgNChannels;
      }
      filheader.nchans = final_channels;
      filheader.tsamp  = gTimeResInSec;

// HARDCODE Greg'S values : 
//      filheader.fch1   = 142.0760; // TEST ONLY !!!
//      filheader.foff   = -0.0040;
//      filheader.tstart = 58323.2563;
      
      CFilFile out_filfile( szOutFilFile );
      out_filfile.ParseHeader( infits , filheader );
      out_filfile.WriteHeader( filheader );

//      int nchans=-1;
      double* power = new double[n_channels];      
      memset(power,'\0',n_channels*sizeof(double));      
//      int block_size = NATIVE_N_CHANNELS*NATIVE_TIMESTEPS*4*sizeof(float);
//      int block_size = n_channels*n_timesteps*4*sizeof(float); // channels x timesteps x 4 blocks (only 1 filled with data)
//      int block_size = NATIVE_N_CHANNELS*n_timesteps*4*sizeof(float);
//      float* buffer_data = new float[block_size];
//      memset( buffer_data, '\0', block_size );
//      printf("Block size = %d floats ( = %d bytes )\n",int(block_size/sizeof(float)),block_size);
//      float* out_buffer = new float[ n_channels ];
      unsigned char* out_buffer = new unsigned char[final_channels];
      
      int block_count = 0;
      int n_total_timesteps = 0;
      
      double min_value = 1e20, max_value = -1e20;
      double min_value_scaled = 1e20, max_value_scaled = -1e20;

      int n_timesteps = infits.GetXSize();
      
      for(int t=0;t<n_timesteps;t++){      
         for(int ch=0;ch<n_channels;ch++){
            double val = infits.getXY(t,ch);
            float scaled_value = val;
                  
            if( scaled_value > max_value ){
                max_value = scaled_value;
            }
            if( scaled_value < min_value ){
                min_value = scaled_value;
            }
                  
            if( gMinFITS_Value >= 0 && gMaxFITS_Value >= 0 ){
                scaled_value = 255.00*(scaled_value - gMinFITS_Value)/(gMaxFITS_Value-gMinFITS_Value);
              
                if( scaled_value > max_value_scaled ){
                   max_value_scaled = scaled_value;
                }
                if( scaled_value < min_value_scaled ){
                   min_value_scaled = scaled_value;
                }
            }
            
            if( gMaxFITS_Value > 0 ){
               if( scaled_value > gMaxFITS_Value ){
                  scaled_value = gMaxFITS_Value;
               }
            }
                  
            power[ch] += val;
            out_buffer[ch] = (unsigned char)scaled_value;
         }             
              
         if( gAvgNChannels >= 1 ){
            double* avg_channels = new double[final_channels];
                  
            double sum=0;
            int    count=0, avg_channel = 0;
            for(int ch=0;ch<n_channels;ch++){
                sum += (out_buffer[ch]);
                count++;
                     
                if( count == gAvgNChannels ){
                   avg_channels[avg_channel] = ( sum / count );
                        
                   sum = 0.00;
                   count = 0;
                   avg_channel++;
                }
            }
                  
            if( avg_channel != final_channels ){
               printf("ERROR in channel averaging code !!! %d != %d\n",avg_channel,final_channels);
            }
                  
            for(int ch=0;ch<final_channels;ch++){                  
               out_buffer[ch] = avg_channels[ch];
            }
                  
            delete [] avg_channels;
        }

        int out_floats = out_filfile.WriteData( out_buffer, final_channels );
        if( out_floats != final_channels ){
            printf("ERROR : expected to write %d channels whilst written %d -> exiting !\n",final_channels,out_floats);
            break;
        }
              
        n_total_timesteps += 1;
      }      
      printf("%d spectra written new .fil file %s\n",n_total_timesteps,szOutFilFile);
      
      FILE* out_f = fopen(szOutSpecFile,"w");
      for(int ch=0;ch<n_channels;ch++){
          power[ch] = power[ch] / n_total_timesteps;
          
          fprintf(out_f,"%d %.8f\n",ch,power[ch]);          
      }
      fclose(out_f);

      printf("################################################\n");
      printf("STATISTICS :\n");
      printf("################################################\n");
      printf("min_value = %.4f\n",min_value);
      printf("max_value = %.4f\n",max_value);
      if( gMinFITS_Value > 0 && gMaxFITS_Value > 0 ){
          printf("scaled_min_value = %.4f\n",min_value_scaled);
          printf("scaled_max_value = %.4f\n",max_value_scaled);

      }
      printf("################################################\n");
      printf("Average spectrum written to file %s\n",szOutSpecFile);
      
      delete [] power;
      delete [] out_buffer;

  }else{
     printf("ERROR : could not open FITS file %s\n",filename);
  }
  fclose(f);
  
  if( outf ){
     fclose(outf);
  }
}
*/
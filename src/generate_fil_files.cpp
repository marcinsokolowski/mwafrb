#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <string>
#include<bg_fits.h>
#include <bg_globals.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// #include "random.h"
#include <random>

#include "SigprocFile.h"

/*
   cd /home/msok/askap/craft/data/20190705/flip_test/flip_header_only/FINAL_FLIP_TESTS/pipe_test
   mkfifo input_filfiles_list
   mkfifo fredda_pipe
   
*/

string gOutputFilBaseName="20190928_generated";
string gOutputFilList="fil_list.txt";
int    gNCoarseChannels=3;
int    gTimeStep = 10; // in seconds 
int    gMaxSteps = 10;
int    gSleepTime = -1;

// testing :
double gNoiseGainFactor = 0.1;

void usage()
{
   printf("generate_fil_files outfile_basename N_COARSE_CHANNELS -t TIME_STEP -m MAX_STEPS\n");   
   printf("DEFAULT values:\n");
   printf("\toutfile_basename   = %s\n",gOutputFilBaseName.c_str());
   printf("\tN_COARSE_CHANNELS  = %d\n",gNCoarseChannels);
   printf("\t-t TIME_STEP : default %d seconds\n",gTimeStep);
   printf("\t-m MAX_STEPS : default %d\n",gMaxSteps);
   printf("\t-s SLEEP_TIME : default %d\n",gSleepTime);
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "hm:s:";
   int opt,opt_param,i;

   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'm':
            if( optarg ){   
               gMaxSteps = atol( optarg );
            }
            break;

         case 's':
            if( optarg ){   
               gSleepTime = atol( optarg );
            }
            break;

         default:
            fprintf(stderr,"Unknown option %c\n",opt);
            usage();
      }
   }

   // bg_globals.h:int ParseCommaList( char* szList, vector<string>& out_list, const char * sep="," );
//   char szInputFilFiles[1024];
//   strcpy(szInputFilFiles , gInputFilFiles.c_str() );
//   ParseCommaList( szInputFilFiles, input_fil_files );
}

void print_parameters()
{
  printf("#####################################\n");
  printf("PARAMETERS :\n");
  printf("#####################################\n");
  printf("Output fil file base = %s\n",gOutputFilBaseName.c_str());
  printf("N_COARSE_CHANNELS    = %d\n",gNCoarseChannels);
  printf("Max time steps       = %d\n",gMaxSteps);
  printf("Sleep time           = %d\n",gSleepTime);
  printf("#####################################\n");
  fflush(stdout);
}

/*bool DoesFileExist(const char* fname)
{
        bool bRet=FALSE;
        if(fname && fname[0] ){
                // mystring szFile=fname;
                // szFile.env2str();

                if( access( fname, F_OK ) == 0 ) {
                        bRet = TRUE;
                }
        }
        return bRet;
}


int GetFileSize( const char* filename )
{
        if( !DoesFileExist( filename ) ){
                return -1;
        }

        struct stat buf;
        stat( filename, &buf );
        return (int)buf.st_size;
}*/


int main(int argc,char* argv[])
{
   if( argc==2 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--h")==0) ){
      usage();
      exit(0);
   }


   if( argc >= 2 ){
      gOutputFilBaseName = argv[1];
   }
   if( argc >= 3 ){
      gNCoarseChannels = atol( argv[2] );
   }
   
   // parsing and printing paramters :
   parse_cmdline(argc-2,argv+2);
   print_parameters();
  
   // int  SigprocFile::MergeCoarseChannels( std::vector<string>& fil_file_list, const char* out_file )
   // SigprocFile::MergeCoarseChannels( input_fil_files , gOutputFilFile.c_str() );   
   
   // SigprocFile( int nbits, int nifs, int nchans, double fch1, double foff, double tstart, double tsamp );
   std::vector< SigprocFile* > out_channels;
   out_channels.assign( gNCoarseChannels , NULL );
   
   double min_freq = 142.7200;
   double delta_freq = 0.0100; // not sure why (-) is required for fredda - should not be !
   double tstart0 = 0.00;
   double tsamp  = 0.001;
   int nchans    = 128;
   int nbits  = 32;
   int n_spectra = 1000;
   
   double spec_index = -2.00;
   double freq0 = 150.00;
   double power0 = 1000.00;
   double noise_mean = 0.00;
   double noise_sigma = 100.00;
   
   float* data_buffer = new float[nchans*n_spectra];
   

   FILE* out_fil_list = fopen( gOutputFilList.c_str(), "w" );
   fclose( out_fil_list );

   // http://www.cplusplus.com/reference/random/normal_distribution/
   std::default_random_engine generator;   
   
   int steps=0;
   while( steps < gMaxSteps || gMaxSteps<0 ){
      printf("Generating %d-th .fil file\n",steps);
      string szFilFiles;
      
      for(int cc=0;cc<gNCoarseChannels;cc++){
         if( out_channels[cc] ){
            delete (out_channels[cc]);
         }
      
         double fch1 = min_freq + cc*nchans*delta_freq;
         double tstart = tstart0 + steps*n_spectra*tsamp;
         out_channels[cc] = new SigprocFile( nbits, 1, nchans, fch1, delta_freq, tstart, tsamp );
         
         for( int t = 0 ; t < n_spectra ; t ++){
            for( int fine_ch = 0; fine_ch < nchans ; fine_ch ++ ){
               double fine_freq = fch1 + fine_ch*delta_freq;
               double power = power0*pow( (fine_freq/freq0) , spec_index );
               
               noise_sigma = ( power/sqrt( tsamp*fabs(delta_freq)*1e6 )) * gNoiseGainFactor;
               std::normal_distribution<double> distribution( 0.00, noise_sigma ) ; // 0.00, noise_sigma ); 
//               std::normal_distribution<double> distribution(  0.00, 20.00 ); 
//               double noise = CRandom::GetGauss( noise_sigma, noise_mean );
               double noise = distribution(generator);
//               printf("noise = %.4f ( sigma = %.4f ) \n",noise,noise_sigma);
//               power = 1.00;
               data_buffer[ t*nchans + fine_ch ] = power + noise;
            }
         }
         
         char szFileName[1024];
         sprintf(szFileName,"%s_time%.4fsec_cc%d.fil",gOutputFilBaseName.c_str(),tstart,cc);
         (out_channels[cc])->FillHeader();
         (out_channels[cc])->WriteHeader( szFileName, false, true );
         (out_channels[cc])->WriteData( data_buffer, nchans*n_spectra ); // to write all data 
         printf("Saved .fil file %s\n",szFileName);
         
         szFilFiles += szFileName;
         if( cc < (gNCoarseChannels-1) ){
            szFilFiles += ",";
         }         
      }
      
      FILE* out_fil_list = fopen( gOutputFilList.c_str(), "a+" );
      fprintf(out_fil_list,"%s\n",szFilFiles.c_str());
      fclose( out_fil_list );


      if( gSleepTime > 0 ){
         sleep( gSleepTime );
      }
      
      steps++;
   }   
      
}
 
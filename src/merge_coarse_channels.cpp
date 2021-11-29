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

#include "SigprocFile.h"

/*
   cd /home/msok/askap/craft/data/20190705/flip_test/flip_header_only/FINAL_FLIP_TESTS/pipe_test
   mkfifo input_filfiles_list
   mkfifo fredda_pipe
   
*/

string gInputFilFiles="in1.fil,in2.fil,in3.fil";
string gAvgSpectrumFile="avg_spectrum.txt";
vector<string> input_fil_files;
string gOutputFilFile="out.fil";

void usage()
{
   printf("merge_coarse_channels in1.fil,in2.fil,in3.fil output_filfile\n");   
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "h";
   int opt,opt_param,i;

   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
/*         case 'n':
            if( optarg ){   
               gNormFile = optarg;
            }
            break;*/

         default:
            fprintf(stderr,"Unknown option %c\n",opt);
            usage();
      }
   }

   // bg_globals.h:int ParseCommaList( char* szList, vector<string>& out_list, const char * sep="," );
   char szInputFilFiles[1024];
   strcpy(szInputFilFiles , gInputFilFiles.c_str() );
   ParseCommaList( szInputFilFiles, input_fil_files );
}

void print_parameters()
{
  printf("#####################################\n");
  printf("PARAMETERS :\n");
  printf("#####################################\n");
  printf("Input  fil files   = %s\n",gInputFilFiles.c_str());
  for(int i=0;i<input_fil_files.size();i++){
     printf("\t%s\n",input_fil_files[i].c_str());
  }
  printf("Output fil file  = %s\n",gOutputFilFile.c_str());
  printf("Average spectrum output file = %s\n",gAvgSpectrumFile.c_str());
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
   if( argc<2 || (argc==2 && strcmp(argv[1],"-h")==0) ){
      usage();
      exit(0);
   }


   if( argc >= 2 ){
      gInputFilFiles = argv[1];
   }
   if( argc >= 3 ){
      gOutputFilFile = argv[2];
   }
   change_ext( gOutputFilFile.c_str() , "avg_spectrum" , gAvgSpectrumFile, false );
   
   // parsing and printing paramters :
   parse_cmdline(argc-2,argv+2);
   print_parameters();

   double* avg_spectrum = NULL;  
   // int  SigprocFile::MergeCoarseChannels( std::vector<string>& fil_file_list, const char* out_file )
   int out_channels = SigprocFile::MergeCoarseChannels( input_fil_files , gOutputFilFile.c_str(), avg_spectrum );   
   
   FILE* out_f = fopen( gAvgSpectrumFile.c_str(), "w" );
   for(int ch=0;ch<out_channels;ch++){
      fprintf(out_f,"%d %.4f\n",ch,avg_spectrum[ch]);
   }
   fclose(out_f);   
   
   if( avg_spectrum ){
      delete [] avg_spectrum;
   }      
}

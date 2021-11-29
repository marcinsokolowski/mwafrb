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

string gInputFilFile="in.fil";
string gOutputFilFile="out.fil";

void usage()
{
   printf("avg_channels input_filfile output_filfile n_avg_channels\n");   
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

}

void print_parameters()
{
  printf("#####################################\n");
  printf("PARAMETERS :\n");
  printf("#####################################\n");
  printf("Input fil file   = %s\n",gInputFilFile.c_str());
  printf("Output fil file  = %s\n",gOutputFilFile.c_str());
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
      gInputFilFile = argv[1];
   }
   if( argc >= 3 ){
      gOutputFilFile = argv[2];
   }
   
   int n_avg_channels = 3;
   if( argc >= 4 ){
      n_avg_channels = atol( argv[3] );
   }
   
   
   // parsing and printing paramters :
   parse_cmdline(argc-2,argv+2);
   print_parameters();

   SigprocFile infile( gInputFilFile.c_str() );   
   infile.WriteAveragedChannels( gOutputFilFile.c_str(), n_avg_channels );
}

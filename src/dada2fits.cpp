#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>

#include <bg_fits.h>


int gVerb=0;
int gSnapshotIdx=-1;
int n_samples=8192;
double gWithTimeStamp=1;
double gTimeStampRadius=1800;
int gDumpChannel=0;
char filename[1024];
char gOutFile[1024];
int gShowTimeStamp=0;

void usage()
{
   printf("read_data dada_file\n");
   
   exit(0);
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "sudc:f:i:t:r:";
   int opt,opt_param,i;
      
   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'c':
            if( optarg ){   
               if( atol(optarg)>=0 && atol(optarg)<=7 ){
                 gDumpChannel = atol(optarg);
               }else{
                 printf("ERROR : wrong number of samples %d , not in range [0-7]!\n",(int)atol(optarg));
               }
            }
            break;

         case 'f':
            if( optarg ){   
               strcpy(gOutFile,optarg);
            }
            break;

         case 'i':
            if( optarg ){   
               gSnapshotIdx = atol(optarg);
            }
            break;
            
         case 'd':
            gVerb++;
            break;
            
         case 'u':
            gWithTimeStamp = 1;
            break;

         case 't':
            if( optarg ){
               gWithTimeStamp = atof(optarg);
            }
            break;

         case 'r':
            if( optarg ){
               gTimeStampRadius = atof(optarg);
            }
            break;

         case 's':
            gShowTimeStamp = 1;
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
  printf("Input _r file      = %s\n",filename);
  if( strlen(gOutFile) ){
     printf("Output file        = %s\n",gOutFile);
  }else{
     printf("Output file        = -\n");
  }
  printf("Dump channeld idx  = %d\n",gDumpChannel);
  printf("Integration idx    = %d\n",gSnapshotIdx);
  if( gWithTimeStamp > 100 ){
     printf("Dumping voltages around unixtime = %.2f +/- %.2f seconds\n",gWithTimeStamp,gTimeStampRadius);
  }else{
     printf("Time stamp enabled = %.2f\n",gWithTimeStamp);
  }
  printf("Time radius around = %.2f\n",gTimeStampRadius);
  printf("Show time stamp    = %d\n",gShowTimeStamp);
  printf("Verb level         = %d\n",gVerb);
  printf("#####################################\n");       
}

int main(int argc,char* argv[])
{
  FILE *f=NULL;  
  FILE* outf=NULL;
  int n;
  strcpy(filename,"incoherent_beam_1kHz_1ms.dada");
  strcpy(gOutFile,"incoherent_beam_1kHz_1ms.spectrum");

  if( argc<2 || (argc==2 && strncmp(argv[1],"-h",2)==0) ){
    usage();
  }

  if( argc>=2 ){
    strcpy(filename,argv[1]);
  }
  
  // parsing and printing paramters :
  parse_cmdline(argc-1,argv+1);
  printf_parameters();

  
  if( gVerb ){
    printf("Reading file %s\n",filename);  
  }
  f = fopen(filename, "rb");  
  if( strlen(gOutFile) ){
     outf = fopen(gOutFile,"w");
  }
  if (f)
  {  
      int i=0,count=0;
      
      int string_len=4;      
      char buffer[128],header_buffer[4096];
      memset(buffer,'\0',128);     
      memset(header_buffer,'\0',128);
      
      // read header :
      int header_size = 4096;
      printf("Reading header %d...\n",header_size);

      
      int n = fread(header_buffer,1,header_size,f);
      if( n == header_size ){
         printf("HEADER:\n");
         printf("------------------------------------------------------------------------------------\n");
         printf("%s\n",header_buffer);
         printf("------------------------------------------------------------------------------------\n");
      }else{
         printf("ERROR : expected %d and read only %d bytes : %s!\n",header_size,n,header_buffer);
      }

      int nchans=-1;

      int n_channels  = 1280;
      int n_timesteps = 1000;
      double power[1280];      
      memset(power,'\0',n_channels*sizeof(double));
      
      CBgFits out_fits( n_channels, n_timesteps*8 );
      
      int block_size = n_channels*n_timesteps*4*sizeof(float); // channels x timesteps x 4 blocks (only 1 filled with data)
//      float* buffer_data = new float[block_size];
      float* buffer_data = (float*)malloc(block_size);
      memset( buffer_data, '\0', block_size );
      printf("Block size = %d floats ( = %d bytes )\n",(block_size/sizeof(float)),block_size);
      
      int block_count = 0;
      int n_total_timesteps = 0;
      while( (n = fread(buffer_data, 1, block_size, f)) > 0 ){
          printf("%d : %.4f %.4f\n",block_count,buffer_data[0],buffer_data[1]);
          
          
          for(int t=0;t<n_timesteps;t++){
              for(int ch=0;ch<n_channels;ch++){
//                  int index = ch*n_timesteps + t;
                  int index = ch + t * n_channels;
                  
                  power[ch] += buffer_data[index];
                  
//                  out_fits.setXY( ch, t + block_count*n_timesteps, buffer_data[index] );
                  out_fits.setXY( ch, n_total_timesteps, buffer_data[index] );
              }             

              n_total_timesteps += 1;
          }
          printf("\tProcessed %d x %d = %d floats out of total blocksize = %d\n",n_timesteps,n_channels,(n_channels*n_timesteps),(block_size/sizeof(float)));
          
          block_count++;
      }      
      free( buffer_data );      
      printf("Read %d blocks of 1 second of data\n",block_count);
      
      FILE* out_f = fopen(gOutFile,"w");
      for(int ch=0;ch<n_channels;ch++){
          power[ch] = power[ch] / n_total_timesteps;
          
          fprintf(out_f,"%d %.8f\n",ch,power[ch]);          
      }
      fclose(out_f);
      
      out_fits.WriteFits( "out.fits" );
      printf("Saved output fits !\n");
      
      if( gVerb ){
        printf("Last read returned n=%d bytes, errno=%d, strerror=%s\n",n,errno,strerror(errno));
      }
  }else{
     printf("ERROR : could not open file %s\n",filename);
  }

  fclose(f);
  
  if( outf ){
     fclose(outf);
  }
}

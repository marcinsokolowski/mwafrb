#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>


int gVerb=0;
int gSnapshotIdx=-1;
int n_samples=8192;
double gWithTimeStamp=1;
double gTimeStampRadius=1800;
int gDumpChannel=0;
char filename[1024];
char gOutFile[1024];
char gOutFolded[1024];
int gShowTimeStamp=0;

enum eTimeResolution { eOneMilisecond=0, e250us=1 };
eTimeResolution gTimeResol = eOneMilisecond;

#define NATIVE_N_CHANNELS 1280
#define NATIVE_TIMESTEPS  1000

int n_channels  = 1280;
int n_timesteps = 1000;
double psr_period_in_milisec = 253.0651649482;

int mkdir( const char* dir )
{
   char szCommand[1024];
   sprintf(szCommand,"mkdir -p %s", dir );
   int ret = system(szCommand);   
   
   return ret;
}

int gDumpToTxt=0;
int gSaveSpectra=1;

void usage()
{
   printf("read_data dada_file\n");
   printf("-c number of channels in .dada file [default %d]\n",n_channels);
   printf("-s to save to .txt file\n");
   printf("-t N_TIMESTEPS [default %d]\n",n_timesteps);
   
   exit(0);
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "sudc:f:i:t:r:t:";
   int opt,opt_param,i;
      
   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'c':
            if( optarg ){   
               n_channels = atol(optarg);
            }
            break;

         case 'f':
            if( optarg ){   
               strcpy(gOutFile,optarg);
            }
            break;

         case 't':
            if( optarg ){   
               n_timesteps = atol(optarg);
            }
            break;
            
         case 'd':
            gVerb++;
            break;
            
         case 's':
            gDumpToTxt = 1;
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
  printf("N channels  = %d\n",n_channels);
  printf("N timesteps = %d\n",n_timesteps);
  printf("gDumpToTxt = %d (save to dump.txt file)\n",gDumpToTxt);
  printf("#####################################\n");       
}

int main(int argc,char* argv[])
{
  FILE *f=NULL;  
  FILE* outf=NULL;
  int n;
  strcpy(filename,"incoherent_beam_1kHz_1ms.dada");
  strcpy(gOutFile,"incoherent_beam_1kHz_1ms.spectrum");
  strcpy(gOutFolded,"J0953-0755_folded.txt");

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
      double* power = new double[n_channels];      
      int psr_period_in_milisec_int = int(psr_period_in_milisec);
      double* folded = new double[psr_period_in_milisec_int];
      int*    folded_count = new int[psr_period_in_milisec_int];
      memset(power,'\0',n_channels*sizeof(double));
      memset(folded,'\0',psr_period_in_milisec_int*sizeof(double));
      memset(folded_count,'\0',psr_period_in_milisec_int*sizeof(int));
  
      // NATIVE_N_CHANNELS    
      int block_size  = NATIVE_N_CHANNELS*NATIVE_TIMESTEPS*4*sizeof(float);
//      int block_size = n_channels*n_timesteps*4*sizeof(float); // channels x timesteps x 4 blocks (only 1 filled with data)
//      int block_size  = NATIVE_N_CHANNELS*n_timesteps*4*sizeof(float); 
//     int block_size  = NATIVE_N_CHANNELS*NATIVE_TIMESTEPS*4*sizeof(float);
//      float* buffer_data = new float[block_size];
      float* buffer_data = (float*)malloc(block_size);
      memset( buffer_data, '\0', block_size );
      printf("Block size = %d floats ( = %d bytes )\n",int(block_size/sizeof(float)),block_size);
      
      FILE* out_totpow_f = fopen("total_power.txt","w");
      FILE* out_dump_f   = NULL;
      
      if( gDumpToTxt > 0 ){
         out_dump_f = fopen("dump.txt","w");
      }
      
      if( gSaveSpectra > 0 ){
         mkdir( "spectra/" );
      }
      
      
      int block_count = 0;
      int n_total_timesteps = 0;
      double max_value = -1e6, min_value = +1e20;
      while( (n = fread(buffer_data, 1, block_size, f)) > 0 ){
          printf("%d : %.4f %.4f\n",block_count,buffer_data[0],buffer_data[1]);
          
                    
          for(int t=0;t<n_timesteps;t++){
              FILE* out_spectra_f = NULL;
              if( gSaveSpectra > 0 ){
                  char szSpectraFile[1024];
                  sprintf(szSpectraFile,"spectra/spectrum_%08d.txt",n_total_timesteps);
         
                  out_spectra_f = fopen( szSpectraFile , "w" );
              }

          
          
              double total_power = 0.00;
              for(int ch=0;ch<n_channels;ch++){
//                  int index = ch*n_timesteps + t;
                  int index = ch + t * n_channels;
                  float p = buffer_data[index];
                  
                  power[ch] += p;                  
                  total_power += p;
                  
                  if ( p > max_value ){
                     max_value = p;
                  }
                  if ( p < min_value ){
                     min_value = p;
                  }
                  
                  if( n_total_timesteps == 0 ){
                     printf("DEBUG : %d %.2f\n",ch,p);                     
                  }
                  if( out_dump_f ){
                     fprintf(out_dump_f,"%.8f\n",p);                     
                  }
                  if( out_spectra_f ){
                      fprintf(out_spectra_f,"%d %.8f\n",ch,p);
                  }
              }             
//              if( (total_power/n_channels) > 3000000 ){
//                  printf("DEBUG : total_power = %.4f > 3000000 at timestamp = %d in block %d\n",(total_power/n_channels),t,block_count);
//              }
              fprintf(out_totpow_f,"%d %.8f\n",n_total_timesteps,(total_power/n_channels));
              
              int folded_index = (n_total_timesteps % psr_period_in_milisec_int);
              // for 250usec :
              if( gTimeResol == e250us ){
                  folded_index = ( (n_total_timesteps/4) % psr_period_in_milisec_int);
              }
              
              folded[folded_index] += (total_power); // /n_channels
              folded_count[folded_index] += 1;
              printf("DEBUG : folded[%d] += %.4f = %.4f / %d = %.4f\n",folded_index,total_power,folded[folded_index],folded_count[folded_index],(folded[folded_index]/folded_count[folded_index]));
              fclose( out_spectra_f );

              n_total_timesteps += 1;
          }
          printf("\tProcessed %d x %d = %d floats\n",n_timesteps,n_channels,(n_channels*n_timesteps));
          
          block_count++;
      }      
      fclose(out_totpow_f);
      free( buffer_data );      
      printf("Read %d blocks of 1 second of data\n",block_count);
      
      FILE* out_f = fopen(gOutFile,"w");
      for(int ch=0;ch<n_channels;ch++){
          power[ch] = power[ch] / n_total_timesteps;
          
          fprintf(out_f,"%d %.8f\n",ch,power[ch]);          
      }
      fclose(out_f);
      
      if( out_dump_f ){
         fclose( out_dump_f );
      }

      // find maximum to normalise by it :
      double max_folded = -1e6;
      for(int t=0;t<psr_period_in_milisec_int;t++){
         if( (folded[t]/folded_count[t]) > max_folded ){
            max_folded = (folded[t]/folded_count[t]);
         }
      }
      
      out_f = fopen( gOutFolded , "w" );
      for(int t=0;t<psr_period_in_milisec_int;t++){
          double folded_norm = ( folded[t]/folded_count[t] );
          double folded_norm_max = folded_norm / max_folded;
      
          fprintf(out_f,"%d %.8f %.8f %d %.8f\n",t,folded[t]/folded_count[t],folded[t],folded_count[t],folded_norm_max);
      }
      fclose(out_f);
      delete [] folded;
      delete [] folded_count;
      
      if( gVerb ){
        printf("Last read returned n=%d bytes, errno=%d, strerror=%s\n",n,errno,strerror(errno));
      }
      printf("----------------------------------------------------\n");
      printf("STATISTICS :\n");
      printf("----------------------------------------------------\n");
      printf("Minimum value = %.8f\n",min_value);
      printf("Maximum value = %.8f\n",max_value);      
      printf("----------------------------------------------------\n");
      
  }else{
     printf("ERROR : could not open file %s\n",filename);
  }

  fclose(f);
  
  if( outf ){
     fclose(outf);
  }
}

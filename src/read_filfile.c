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
int gShowTimeStamp=0;

/* header keywords from : https://github.com/scottransom/presto/blob/master/lib/python/sigproc.py

  "HEADER_START": 'flag',
    "telescope_id": 'i',
    "machine_id": 'i',
    "data_type": 'i', 
    "rawdatafile": 'str',
    "source_name": 'str', 
    "barycentric": 'i', 
    "pulsarcentric": 'i', 
    "az_start": 'd',  
    "za_start": 'd',  
    "src_raj": 'd',  
    "src_dej": 'd',  
    "tstart": 'd',  
    "tsamp": 'd',  
    "nbits": 'i', 
    "signed": 'b', 
    "nsamples": 'i', 
    "nbeams": "i",
    "ibeam": "i",
    "fch1": 'd',  
    "foff": 'd',
    "FREQUENCY_START": 'flag',
    "fchannel": 'd',  
    "FREQUENCY_END": 'flag',
    "nchans": 'i', 
    "nifs": 'i', 
    "refdm": 'd',  
    "period": 'd',  
    "npuls": 'q',
    "nbins": 'i', 
"HEADER_END": 'flag'}
*/

char get_keyword_type( const char* keyword )
{
// f - flag
// i - integer
// s - string
// d - double
// u - unknown 

    if( strcmp(keyword,"HEADER_START") == 0 ){ return  'f'; }
    if( strcmp(keyword,"telescope_id") == 0 ){ return  'i'; }
    if( strcmp(keyword,"machine_id") == 0 ){ return  'i'; }
    if( strcmp(keyword,"data_type") == 0 ){ return  'i'; }
    if( strcmp(keyword,"rawdatafile") == 0 ){ return  's'; }
    if( strcmp(keyword,"source_name") == 0 ){ return  's'; }
    if( strcmp(keyword,"barycentric") == 0 ){ return  'i'; }
    if( strcmp(keyword,"pulsarcentric") == 0 ){ return  'i'; }
    if( strcmp(keyword,"az_start") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"za_start") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"src_raj") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"src_dej") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"tstart") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"tsamp") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"nbits") == 0 ){ return  'i'; }
    if( strcmp(keyword,"signed") == 0 ){ return  'b'; }
    if( strcmp(keyword,"nsamples") == 0 ){ return  'i'; }
    if( strcmp(keyword,"nbeams") == 0 ){ return  'i'; }
    if( strcmp(keyword,"ibeam") == 0 ){ return  'i'; }
    if( strcmp(keyword,"fch1") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"foff") == 0 ){ return  'd'; }
    if( strcmp(keyword,"FREQUENCY_START") == 0 ){ return  'f'; }
    if( strcmp(keyword,"fchannel") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"FREQUENCY_END") == 0 ){ return  'f'; }
    if( strcmp(keyword,"nchans") == 0 ){ return  'i'; }
    if( strcmp(keyword,"nifs") == 0 ){ return  'i'; }
    if( strcmp(keyword,"refdm") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"period") == 0 ){ return  'd'; } 
    if( strcmp(keyword,"npuls") == 0 ){ return  'q'; }
    if( strcmp(keyword,"nbins") == 0 ){ return  'i'; }
    if( strcmp(keyword,"HEADER_END") == 0 ){ return  'f'; }

    printf("ERROR : unknown type of keyword = %s\n",keyword);
 
    return 'u';
}



void usage()
{
   printf("read_binary test.fil -d -u -c -f outfile -i integration_index -t UNIX_TIME -r TIME_RADIUS -s \n");
   printf("\n");
   printf("Options :\n");
   printf("\t-u - assumes that UNIXTIME timestamp is added to line as extra record of adcv_t structure\n");
   printf("\t-t UNIX_TIME : dumps integrations around specified unix time\n");
   printf("\t-r TIME_RADIUS : specify time in seconds around which integrations are dumped\n");
   printf("\t-d - increases debug level\n");
   printf("\t-c CHANNEL : specifies channel to be dumped [default 0]\n");      
   printf("\t-f OUTFILENAME : name of file to which data is dumped\n");
   printf("\t-i INTEGRATION_INDEX : index of integration (= adcv dump) to be dumped, by default all are dumped\n");   
   printf("\t-s : only dumps timestamp of all integrations\n");
   
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
  strcpy(filename,"test.fil");
  gOutFile[0] = '\0';

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
      char buffer[128];
      memset(buffer,'\0',128);     
      
      // read header :
      printf("Reading header sizeof(int)=%d...\n",sizeof(int));
//      int header_start_len = strlen("HEADER_START");
//      n = fread(buffer, header_start_len, 1, f);
//      printf("First word = %s\n",buffer);

      int nchans=-1;

      
      while( (n = fread(buffer, string_len, 1, f)) > 0 ){
        char keyword[64],value[128];
        printf("\n");              
//        printf("Buffer with len = %c\n",buffer[0]);
        int string_size = *((int*)(buffer));                
        int string_bytes = fread(buffer, string_size, 1, f);
        
        char keyword_type = get_keyword_type( buffer );
        printf("Keyowrd %s is of type %c\n",buffer,keyword_type);
        strcpy(keyword,buffer);
        
        int val_size=0, val_read=0;
        int val_int = -1000;
        long long val_longlong = 0;
        char val_char = -100;
        double val_double = -1000;
        char val_string[128];
        int end_header=0;
        
        switch ( keyword_type ){
           case 'i' :
              val_size = sizeof(int);
              val_read = fread(buffer, val_size, 1, f);
              strcpy(value,buffer);
              val_int  = *((int*)(buffer)); 
              printf("Keyword %s = %d (type = %c , val_size = %d)\n",keyword,val_int,keyword_type,val_size);              
              if( strcmp(keyword,"nchans")==0){
                 nchans = val_int;
              }
              break;

           case 'q' :
              val_size = sizeof(long long);
              val_read = fread(buffer, val_size, 1, f);
              strcpy(value,buffer);
              val_longlong  = *((long long*)(buffer)); 
              printf("Keyword %s = %d (type = %c , val_size = %d)\n",keyword,val_longlong,keyword_type,val_size);              
              break;

           case 'b' :
              val_size = sizeof(char);
              val_read = fread(buffer, val_size, 1, f);
              strcpy(value,buffer);
              val_char  = *((char*)(buffer)); 
              printf("Keyword %s = %d (type = %c , val_size = %d), val_read = %d\n",keyword,val_char,keyword_type,val_size,val_read);              
              break;

           case 'd' :
              val_size = sizeof(double);
              val_read = fread(buffer, val_size, 1, f);
              strcpy(value,buffer);
              val_double  = *((double*)(buffer)); 
              printf("Keyword %s = %.4f (type = %c , val_size = %d)\n",keyword,val_double,keyword_type,val_size);         
              break;
                              
           case 's' :
              val_read = fread(buffer, string_len, 1, f);
              val_size = *((int*)(buffer));
              printf("Keyword %s is string of length = %d\n",keyword,val_size);
              memset(buffer,'\0',128);
              string_bytes = fread(buffer, val_size, 1, f);
              strcpy(val_string,buffer);
              printf("Keyword %s = %s (type = %c , val_size = %d)\n",keyword,val_string,keyword_type,val_size);
              
              break;
              
           case 'f' :
              printf("Flag %s -> ignored\n",keyword);    
              if(strcmp(keyword,"HEADER_END") == 0 ){
                 printf("End of header -> reading data\n");
                 end_header=1;
              }
              
              break;
              
           default :
              printf("ERROR : unknown type %c\n",keyword_type);               
        }
        
        
        memset(buffer,'\0',128);
        
        if( end_header > 0 ){
           break;
        }
      
        count++;
        
//        if ( count > 20 ){
//           break;
//        }
      }
      
      // 
      printf("HEADER VALUES:\n");
      printf("nchans = %d\n",nchans);
      
      // just read first spectrum :
      uint8_t data_buffer[336];
      n = fread(data_buffer, nchans, 1, f);
      
      for(int i=0;i<nchans;i++){
         printf("%d = %d\n",i,(int)(data_buffer[i]));
      }
      
      
      
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

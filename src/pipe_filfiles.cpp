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

string gInputFilFileList="input_filfiles_list";
string gOutputFilFilePipe="fredda_pipe";

void usage()
{
   printf("pipe_filfiles input_filfile_list[file or pipe] output_filfile_list[pipe]\n");   
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
  printf("Input fil file list  = %s\n",gInputFilFileList.c_str());
  printf("Output fil file list = %s\n",gOutputFilFilePipe.c_str());
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


int ReadWriteFilFile( const char* filfilename, FILE* outpipe, int fil_file_index )
{
    if( !SigprocFile::DoesFileExist( filfilename ) ){
       printf("WARNING : fil file %s does not exist -> skipped\n",filfilename);
       return -1;
    }

    SigprocFile filfile( filfilename ); // , 472 ); // standard header size for MWA files is 472 bytes 
    printf("Read header from file %s , header size = %d bytes\n",filfilename,filfile.gethdrbytes());
    
    
    // size_t fwrite(const void *ptr, size_t size, size_t nmemb,
    //                 FILE *stream);
    
    int written_header = 0;
    if( fil_file_index == 0 ){
       char* header = new char[MAX_HDR_SIZE];
       memset(header,'\0',MAX_HDR_SIZE);
       memcpy(header,filfile.gethdr(), filfile.gethdrbytes() );
    
       written_header = fwrite( header, 1, MAX_HDR_SIZE, outpipe );
       
       delete [] header;
    }else{
       printf("WARNING : header not written for fil file %d\n",filfilename);
    }
    
    int n_sample_size = filfile.nchans();
//    int nt = 512;
//    float* buffer = new float[n_sample_size*nt]; 
    int data_size = filfile.getdatabytes(); // SigprocFile::GetFileSize( filfilename ) - filfile.gethdrbytes();
    float* buffer = new float[data_size];
    int galp_size = data_size;
//    galp_size = 512*n_sample_size;
//    int samples_read = 0;
    int written_data = 0;
//    while( (samples_read = filfile.read_samples_uint8( nt, (uint8_t*)buffer )) > 0 ){
//       int write_ret = fwrite( buffer, sizeof(uint8_t), samples_read, outpipe );
//       written_data += write_ret;
//    }
    int bytes_read = filfile.read( buffer, galp_size ); // fread( buffer, 1, file_size,  filfile.
    if( bytes_read == galp_size ){
       printf("Read %d bytes from file %s\n",bytes_read,filfilename);
       
       int write_ret = fwrite( buffer, 1, bytes_read, outpipe );
       
       printf("Written %d bytes to output file / pipe\n",write_ret);
    }else{
       printf("ERROR : read %d bytes, expected %d -> nothing written to output file / pipe\n",bytes_read,galp_size);       
    }
     
    printf("Written %d header and %d data bytes ( %d bytes total ) to output file\n",written_header,written_data,(written_header+written_data));
    
    delete [] buffer;
    
    return (written_header+written_data*sizeof(float)); // return number of written bytes 
}

void Run()
{
   char buffer[1024];   
   int fil_file_index=0;
   
   FILE* outpipe = fopen( gOutputFilFilePipe.c_str() , "w" );
//   int fd_out = fileno( outpipe );

   while( 1 ){   
       // this while( 1 ) loop is because I could not force fgets to working in LOCKING mode :
       FILE* infile = fopen( gInputFilFileList.c_str() , "r" );
       int fd = fileno( infile );
       int flags = fcntl(fd, F_GETFL, 0);
       flags = flags & ~O_NONBLOCK;
       fcntl(fd, F_SETFL, flags);
   
       while( fgets(buffer,1024,infile) ){
           int buff_len=strlen(buffer);
           if( buff_len>0 ){ // otherwise already it is : m_pBuffer[0]='\0' 
              if ( buffer[buff_len-1]=='\n'){
                  buffer[buff_len-1]='\0';
              }
           }
           if( buffer[0] == '\n' ){
             printf("WARNING : skipped empty line\n");
           }
   
           printf("Reading %d filfile %s\n",fil_file_index,buffer);
      
           int written = ReadWriteFilFile( buffer, outpipe, fil_file_index );
           printf("Written %d bytes to output file %s\n",written,gOutputFilFilePipe.c_str());
           fil_file_index++;           
           fflush(outpipe);
           
       }
       printf("Exited the main loop\n");
                 
       fclose(infile);
   }
   
   fclose(outpipe);
}

int main(int argc,char* argv[])
{
   if( argc<2 || (argc==2 && strcmp(argv[1],"-h")==0) ){
      usage();
      exit(0);
   }


   if( argc >= 2 ){
      gInputFilFileList = argv[1];
   }
   if( argc >= 3 ){
      gOutputFilFilePipe = argv[2];
   }
   
   // parsing and printing paramters :
   parse_cmdline(argc-2,argv+2);
   print_parameters();
   
   Run();      
}
/*   


   SigprocFile filfile( filfilename.c_str() );
   SigprocFile filfile_norm;
   SigprocFile filfile_flipped;
   
   printf("File : %s\n", filfile.name() );
   printf("-------------------------------------------\n");
   printf("nifs   = %d\n",filfile.nifs());
   printf("nbits  = %d\n",filfile.nbits());
   printf("nbeams = %d\n",filfile.nbeams());
   printf("nchans = %d\n",filfile.nchans());
   printf("npols  = %d\n",filfile.npols());
   printf("nants  = %d\n",filfile.nants());
   printf("samples_read = %d\n",filfile.samples_read());
   printf("current_sample = %d\n",filfile.current_sample());
   printf("fch1 = %.4f\n",filfile.fch1());
   printf("foff = %.4f\n",filfile.foff());
   printf("tstart = %.4f\n",filfile.tstart());
   printf("tsamp  = %.6f\n",filfile.tsamp());

   if( strlen( gOutNormFilFile.c_str() ) > 0 && normalisation_spectrum.size() > 0 ){
      printf("Saving of normalised file (%s) is required , normalisation spectrum size = %d channels\n",gOutNormFilFile.c_str(),normalisation_spectrum.size());
      if( normalisation_spectrum.size() != filfile.nchans() ){
         printf("ERROR : wrong normalisation spectrum provided in file %s -> cannot normalise -> exiting now !\n",gNormFile.c_str());
         exit(-1);
      }      
      
      if( filfile_norm.Write( gOutNormFilFile.c_str(), filfile ) ){
         printf("ERROR : could not write header file -> exiting now !\n");
         exit(-1);
      }      
   }
   if( gWriteFlipped > 0 ){
      if( filfile_flipped.Write( "flipped.fil", filfile, gFlipHeader ) ){
          printf("ERROR : could not write flipped.fil file\n");
          exit(-1);
      }
   }


   CBgFits out_fits( filfile.nchans(), 1000 );


   // buffer for 1 spectrum :
//   int bytes_per_spectrum = filfile.nbeams()*filfile.nchans()*filfile.npols()*filfile.nants();
//   int n_sample_size  = filfile.nbeams()*filfile.npols()*filfile.nants();
   int n_sample_size = filfile.nchans();
   float* buffer = new float[n_sample_size];
   float* flip_buffer = NULL;

   double* avg_power = new double[n_sample_size];
   memset(avg_power,'\0',sizeof(double)*n_sample_size);
 
   double* avg_power_reim = new double[n_sample_size];
   memset(avg_power_reim,'\0',sizeof(double)*n_sample_size);

   FILE* out_total_power_f = fopen("total_power_fil.txt","w"); 
   double max_value=-1e20,min_value=+1e20;     
   int samples_read=0 , spectra_count=0;
   while( (samples_read = filfile.read_samples_uint8( 1, (uint8_t*)buffer )) > 0  ){
//       if( samples_read != bytes_per_spectrum ){
//          printf("WARNING : samples_read = %d != requested %d at %d spectra\n",samples_read,bytes_per_spectrum,spectra_count);
//          break;
//       }
       printf("Sample %d : read %d elements (samples = %d)\n",spectra_count,samples_read,n_sample_size);          

       // if required subtract mean spectra in the 1st step :
       if( normalisation_spectrum.size() > 0 ){
           if( n_sample_size != normalisation_spectrum.size() ){ 
              printf("ERROR : normalisation spectrum has %d channels != .fil file spectrum %d with %d channels\n",normalisation_spectrum.size(),spectra_count,n_sample_size);
              exit(-1);
           }
       
           for(int i=0;i<n_sample_size;i++){
              buffer[i] = buffer[i] - normalisation_spectrum[i].y;
           }
           filfile_norm.WriteData( buffer , n_sample_size );               
       }
       
       if( gWriteFlipped > 0 ){
           if( !flip_buffer ){
              flip_buffer = new float[n_sample_size];
           }
           for(int i=0;i<n_sample_size;i++){
              flip_buffer[i] = buffer[n_sample_size-1-i];
           }
           
           if( gFlipData > 0 ){
              printf("Flipping data ...\n");
              filfile_flipped.WriteData( flip_buffer , n_sample_size );
           }else{
              printf("NOT-Flipping data ...\n");
              filfile_flipped.WriteData( buffer , n_sample_size );
           }
       }
       

       
       out_fits.add_line( buffer, filfile.nchans() );
       
       char szOutFile[1024];
       FILE* spectra_f = NULL;
       if( gSaveSpectra > 0 ){
          sprintf(szOutFile,"%s/spectrum_%08d.txt",gOutDir.c_str(),spectra_count);
          printf("Opening file %s\n",szOutFile);
          spectra_f = fopen( szOutFile , "w" );
       }

       printf("DEBUG test samples = %d / %d / %d\n",buffer[0],buffer[160],buffer[319]);       
       double total_power = 0.00;
       for(int i=0;i<n_sample_size;i++){
          avg_power[i] += buffer[i];
          total_power += buffer[i];
          
          if( spectra_count == 0 ){
             printf("\t%d %u\n",(int)(buffer[i]),buffer[i]);
          }
          
          if( buffer[i] > max_value ){
             max_value = buffer[i];
          }
          if( buffer[i] < min_value ){
             min_value = buffer[i];
          }
 
          if( spectra_f ){          
             fprintf( spectra_f , "%d %d\n",i,buffer[i]);
          }
       }
       if( spectra_f ){
          fclose( spectra_f );
       }
       fprintf(out_total_power_f,"%d %.4f\n",spectra_count,(total_power/n_sample_size));    
       
       // test if not RE/IM :
       for(int i=0;i<n_sample_size;i+=2){
          // RE/IM VERSION :
          // uint8_t re = buffer[i];
          // uint8_t im = buffer[i+1];          
          // double power = re*re + im*im;
          // avg_power_reim[i/2] += power;
          
          // DETECTED POWER VERSION :
          double power = buffer[i];
          avg_power_reim[i] += power;
       }
       
       spectra_count++;
   }
   fclose( out_total_power_f );
   
   out_fits.set_ysize();
   // PrepareBigHornsHeader( double ux_start, double _inttime, double freq_start, double delta_freq_mhz )
   double freq_start = filfile.fch1() - filfile.nchans()*fabs(filfile.foff());
//   double freq_start = 110.00*1.28 - 0.64;
   out_fits.PrepareBigHornsHeader( get_dttm(), filfile.tsamp(), freq_start, fabs(filfile.foff()) );
   out_fits.WriteFits( "out.fits" );

   if( 1 ){
      // write transposed :
      CBgFits out_fits_t( out_fits.GetYSize(), filfile.nchans() );
      for(int ch=0;ch<out_fits.GetXSize();ch++){
         for(int t=0;t<out_fits.GetYSize();t++){
            double val = out_fits.getXY( ch, t );
            out_fits_t.setXY( t, ch, val );
         }
      }
      out_fits_t.PrepareBigHornsHeaderTransposed( get_dttm(), filfile.tsamp(), freq_start, fabs(filfile.foff()) );
      out_fits_t.WriteFits( "out_t.fits" );
      
      // write transposed and averaged in time / frequency :
      int n_time_avg=2;
      CBgFits out_fits_t_avg( out_fits_t.GetXSize()/n_time_avg, out_fits_t.GetYSize() );
      for(int t=0;t<out_fits_t.GetXSize();t+=n_time_avg){         
         for(int ch=0;ch<out_fits_t.GetYSize();ch++){
            
            double sum = 0.00;
            for(int k=0;k<n_time_avg;k++){
               sum += out_fits_t.getXY( t + k , ch );
            }       
            
            int t_avg = t / n_time_avg;
            out_fits_t_avg.setXY( t_avg, ch, (sum / n_time_avg) );
            
            if( t == 0 ){
               printf("DEBUG : t = %d -> t_avg = %d , ch = %d, value = %.4f\n",t,t_avg,ch, (sum / n_time_avg) );
            }
         }
      }
      out_fits_t_avg.PrepareBigHornsHeaderTransposed( get_dttm(), filfile.tsamp()*2.00, freq_start, fabs(filfile.foff()) );
      out_fits_t_avg.WriteFits( "out_t_avgt.fits" );

   }
   
   FILE* out_f = fopen( outfile.c_str(), "w" );
   for(int i=0;i<n_sample_size;i++){
      avg_power[i] = avg_power[i]  / spectra_count;
      
      
      fprintf(out_f,"%d %.8f\n",i,avg_power[i]);     
//      printf("%d %.8f\n",i,avg_power[i]);
   }
   fclose(out_f);  

   printf("------------------------------------------------\n");
   printf("STATISTICS :\n");
   printf("------------------------------------------------\n");
   printf("Spectra count = %d\n",spectra_count);
   printf("min value = %.2f\n",min_value);
   printf("max value = %.2f\n",max_value);
   printf("------------------------------------------------\n");
   printf("Average detected power written to file %s\n",outfile.c_str());
   
   
   out_f = fopen( "reim_test.txt", "w" );
   for(int i=0;i<n_sample_size;i+=2){
       fprintf(out_f,"%d %.8f\n",i,avg_power_reim[i/2]);
   }
   fclose(out_f);

   delete [] avg_power;
   delete [] avg_power_reim;
   delete [] buffer;
   if( flip_buffer ){ delete [] flip_buffer; }
}
*/
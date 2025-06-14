#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <string>
#include <bg_fits.h>
#include <bg_globals.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>

// to check maximum stack limits:
#include <sys/time.h>
#include <sys/resource.h>

#include "SigprocFile.h"

string gOutDir="spectra/";
int    gSaveSpectra = -1;

std::string gNormFile;
vector<cValue> normalisation_spectrum;
string gOutNormFilFile;
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

int gFoffSign=+1;

double gUnixTime=-1;

static time_t get_gmtime_from_string_localfunc( const char* szGmTime , const char* format="%Y%m%d_%H%M%S")
{
   struct tm gmtime_tm;
   memset(&gmtime_tm, '\0', sizeof(struct tm));
   // sscanf( szGmTime, "%.4u%.2u%.2u_%.2u%.2u%.2u", &gmtime_tm.tm_year,&gmtime_tm.tm_mon,
   // &gmtime_tm.tm_mday,&gmtime_tm.tm_hour,&gmtime_tm.tm_min,&gmtime_tm.tm_sec);

   strptime( szGmTime, format, &gmtime_tm );

   // gmtime_tm.tm_year -= 1900;
   // gmtime_tm.tm_mon--;
   time_t ret = timegm( &gmtime_tm );
        // time_t ret = timegm( &gmtime_tm );
   return ret;
}


time_t get_filfile_uxtime( const char* filfile )
{
   // 1359220824_20230131172006_ch120_02.fil
   long int gps;
   int ch,beam;

   char gps_str[64];
   char dtm[64],dt[16],tm[16],dtm2[64];;
   memset(gps_str,'\0',64);
   memset(dtm,'\0',64);
   memset(dt,'\0',16);
   memset(tm,'\0',16);
   
   strncpy(gps_str,filfile,10);
   strncpy(dtm,filfile+11,14);
   strncpy(dt,dtm,8);
   strncpy(tm,dtm+8,6);   
   sprintf(dtm2,"%s_%s",dt,tm);
   
   gps = atol(gps_str);
   time_t ux = get_gmtime_from_string_localfunc( dtm2 );
   time_t ux_gps = gps+315964782;
   
   printf("DEBUG : parsed filfile |%s| -> gps = %ld (from %s), dtm = %s (%s/%s) -> ux = %ld (vs. ux_gps = %ld) \n",filfile,gps,gps_str,dtm,dt,tm,ux,ux_gps);
  
   return ux;   
}

void usage()
{
   printf("dumpfilfile_float test.fil\n");   
   printf("\t-n normalisation_file : file with mean spectrum to use for normalisation [default not set]\n");
   printf("\t-o output normalised fil file [default not required]\n");
   printf("\t-f save flipped file\n");   
   printf("\t-F flip data [default %d]\n",gFlipData);
   printf("\t-H flip header [default %d]\n",gFlipHeader);
   printf("\t-T 255 : truncate to 0 - 255 range and saved 8bits filterbank file\n");   
   printf("\t-S Save_spectrum_index : save spectrum index = %d [only if >= 0]\n",gSaveSpectra);
   printf("\t-L MAX_SPECTRA_COUNT : only save maximum this number of spectra\n");
   printf("\t-s SIGN : sign of FOFF in .fil file (should be correct but may need a tweak) [default %d]\n",gFoffSign);
   printf("\t-U UNIXTIME : start unix time of filterbank data\n");
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "hfFHn:o:T:S:L:s:U:";
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
               gOutNormFilFile = optarg;
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

         case 's':
            if( optarg ){
               gFoffSign = atol( optarg );
            }
            break;

         case 'S':
            gSaveSpectra = atol( optarg );
            break;

         case 'T':
            gFloat2UChar = atol( optarg );
            break;

         case 'U':
            if( optarg ){
               gUnixTime = atof( optarg );
            }
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
  printf("Input .fil file   = %s\n",filfilename.c_str());
  printf("Output normalised .fil file = %s\n",gOutNormFilFile.c_str());
  printf("Norm file          = %s\n",gNormFile.c_str());
  printf("Write flipped      = %d (flip data = %d, flip header = %d)\n",gWriteFlipped,gFlipData,gFlipHeader);
  printf("Float2Uchar        = %d\n",gFloat2UChar);
  printf("Save spectrum      = %d\n",gSaveSpectra);
  printf("Max. spectra count = %d\n",int(gMaxSpectraCount)); 
  printf("Foff sign          = %d\n",gFoffSign);
  printf("Unixtime           = %.8f\n",gUnixTime);
  printf("Output FITS files :\n");
  printf("\t\t%s\n",gOutFitsFileName.c_str());
  printf("\t\t%s\n",gOutFitsTransposedFileName.c_str());
  printf("\t\t%s\n",gOutFitsAvgTransposedFileName.c_str());
  printf("#####################################\n");
  fflush(stdout);
}


int main(int argc,char* argv[])
{
   if( argc<2 || (argc==2 && strcmp(argv[1],"-h")==0) ){
      usage();
      exit(0);
   }

   string filfilename = argv[1];
   string outfile = "avg_spectrum.txt";
   change_ext( filfilename.c_str() , "avg_spectrum" , outfile , false );
   change_ext( filfilename.c_str() , "_norm.fil" , gOutNormFilFile, true );
   change_ext( filfilename.c_str() , "_out.fits" , gOutFitsFileName, true );   
   change_ext( filfilename.c_str() , "_out_t.fits" , gOutFitsTransposedFileName, true );   
   change_ext( filfilename.c_str() , "_out_t_avgt.fits" , gOutFitsAvgTransposedFileName, true );   
   change_ext( filfilename.c_str() , "total_power" ,gOutTotalPowerFile, false );
      
   // parsing and printing paramters :
   parse_cmdline(argc-1,argv+1);
   print_parameters( filfilename, outfile );
   mkdir( gOutDir.c_str() );

   if( normalisation_spectrum.size() <=0 && gFloat2UChar <= 0 ){
      gOutNormFilFile = "";
      printf("WARNING : normalisation spectrum is not provided and neither truncation from Float to UChar -> setting gOutNormFilFile to NULL -> no normalisation file writting\n");      
   }

   long int filfile_size = get_file_size( filfilename.c_str() );
   SigprocFile filfile( filfilename.c_str() );
   // SigprocFile( int nbits, int nifs, int nchans, double fch1, double foff, double tstart, double tsamp );
   SigprocFile filfile_norm( filfile.nbits(), filfile.nifs(), filfile.nchans(), filfile.fch1(), filfile.foff(), filfile.tstart(), filfile.tsamp() );   
//   SigprocFile filfile_norm;
   SigprocFile filfile_flipped;
   
   int nbytes = sizeof(float);
   if( gFloat2UChar > 0 ){
      filfile_norm.nbits( 8 );
      nbytes = 1;
   }
   long int initial_fits_size = (long int)( filfile_size/(filfile.nchans()*nbytes) ) + 10;
   if( gMaxSpectraCount > 0 ){
      initial_fits_size = gMaxSpectraCount;
   }
   
   struct rlimit rlim;
   getrlimit( RLIMIT_STACK, &rlim);
   printf("SYSTEM INFO : maximum stack size = %ld bytes (soft limit) and %ld bytes of hard limit\n",rlim.rlim_cur,rlim.rlim_max);
/*   if( initial_fits_size >= 4000 || filfile.nchans()>= 8000 ){
      rlim.rlim_cur = 33554432*2;
      setrlimit( RLIMIT_STACK, &rlim);
      printf("SYSTEM INFO : maximum stack size set to %ld bytes\n",rlim.rlim_cur);
   }*/
   
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
   printf("Source name = %s\n",filfile.sourcename());
   printf("(RA,DEC) = (%.4f, %.4f)\n",filfile.src_raj(),filfile.src_dej());
   printf("tsamp  = %.6f\n",filfile.tsamp());
   printf("file size = %ld\n",filfile_size);
   printf("Initial out FITS size = %ld\n",initial_fits_size);   

   if( strlen( gOutNormFilFile.c_str() ) > 0 ){
      filfile_norm.name( gOutNormFilFile.c_str() );
   
      if( normalisation_spectrum.size() > 0 ){
         printf("Saving of normalised file (%s) is required , normalisation spectrum size = %d channels\n",gOutNormFilFile.c_str(),int(normalisation_spectrum.size()));
         if( normalisation_spectrum.size() != filfile.nchans() ){
            printf("ERROR : wrong normalisation spectrum provided in file %s -> cannot normalise -> exiting now !\n",gNormFile.c_str());
            exit(-1);
         }      
      }

      if( gFloat2UChar > 0 ){
         filfile_norm.FillHeader();
         filfile_norm.WriteHeader( gOutNormFilFile.c_str() , false, true );
      }else{      
         if( filfile_norm.Write( gOutNormFilFile.c_str(), filfile, 0, true ) ){
            printf("ERROR : could not write header file -> exiting now !\n");
            exit(-1);
         }               
      }
   }
   if( gWriteFlipped > 0 ){
      if( filfile_flipped.Write( "flipped.fil", filfile, gFlipHeader ) ){
          printf("ERROR : could not write flipped.fil file\n");
          exit(-1);
      }
   }


   // Create vertical FITS (channels on X axis and time on Y axis):
   int filfile_nchans = filfile.nchans();
   CBgFits out_fits( filfile.nchans(), initial_fits_size );


   // buffer for 1 spectrum :
//   int bytes_per_spectrum = filfile.nbeams()*filfile.nchans()*filfile.npols()*filfile.nants();
//   int n_sample_size  = filfile.nbeams()*filfile.npols()*filfile.nants();
   int n_sample_size = filfile.nchans();
   printf("DEBUG : n_sample_size = %d\n",n_sample_size);
   float* buffer = new float[n_sample_size];
   float* flip_buffer = NULL;

   double* avg_power = new double[n_sample_size];
   memset(avg_power,'\0',sizeof(double)*n_sample_size);
 
   double* avg_power_reim = new double[n_sample_size];
   memset(avg_power_reim,'\0',sizeof(double)*n_sample_size);
   
   unsigned char* buffer_8bits = (unsigned char*)(new char[n_sample_size]);

   FILE* out_total_power_f = fopen( gOutTotalPowerFile.c_str() ,"w"); 
   double max_value=-1e20,min_value=+1e20;     
   int samples_read=0 , spectra_count=0;
   
   if( gFloat2UChar ){
      while( (samples_read = filfile.read_samples_uint8( 1, (uint8_t*)buffer , false ) ) > 0  ){
         for(int i=0;i<n_sample_size;i++){
            if( buffer[i] > max_value ){
               max_value = buffer[i];
            }
            if( buffer[i] < min_value ){
               min_value = buffer[i];
            }             
         }
      }    
      printf("INFO : min_value = %.8f , max_value = %.8f\n",min_value,max_value);
      filfile.rewind();
   }
     
   
   samples_read=0;
   spectra_count=0;
   while( (samples_read = filfile.read_samples_uint8( 1, (uint8_t*)buffer , false) ) > 0  && (gMaxSpectraCount<=0 || spectra_count<gMaxSpectraCount) ){
//       if( samples_read != bytes_per_spectrum ){
//          printf("WARNING : samples_read = %d != requested %d at %d spectra\n",samples_read,bytes_per_spectrum,spectra_count);
//          break;
//       }
       if( gVerb > 0 || 0 ){
          printf("Sample %d : read %d elements (samples = %d)\n",spectra_count,samples_read,n_sample_size);          
       }

       // if required subtract mean spectra in the 1st step :
       if( normalisation_spectrum.size() > 0 ){
           if( n_sample_size != normalisation_spectrum.size() ){ 
              printf("ERROR : normalisation spectrum has %d channels != .fil file spectrum %d with %d channels\n",int(normalisation_spectrum.size()),spectra_count,n_sample_size);
              exit(-1);
           }

           // Normalised and re-calculate MIN/MAX values after normalisation for truncation 
           min_value = +1e20;
           max_value = -1e20;       
           for(int i=0;i<n_sample_size;i++){
//              buffer[i] = buffer[i] - normalisation_spectrum[i].y;
              buffer[i] = buffer[i] / normalisation_spectrum[i].y;
              
              if( buffer[i] < min_value ){
                 min_value = buffer[i];
              }
              if( buffer[i] > max_value ){
                 max_value = buffer[i];
              }
           }
           if( strlen( gOutNormFilFile.c_str() ) > 0 ){
              if( normalisation_spectrum.size() > 0 || gFloat2UChar > 0 ){
                 if( gFloat2UChar > 0 ){
                     for(int i=0;i<n_sample_size;i++){
                         // FITS is not truncated as fil file, but can be for debug :
                         if( 0 ){ // change to 1 for a test :
                            buffer[i] = ( ( buffer[i] - min_value ) / ( max_value - min_value) ) * gFloat2UChar;
                         }
                         buffer_8bits[i] = int( ( ( buffer[i] - min_value ) / ( max_value - min_value) ) * gFloat2UChar );
                     }
                     filfile_norm.WriteData( buffer_8bits , n_sample_size );
                 }else{              
                     filfile_norm.WriteData( buffer , n_sample_size );               
                 }
              }else{
                 if( gVerb>=5 ){
                    printf("WARNING : normalisation spectrum not provided -> norm .fil file not written\n");
                 }
              }
//           }else{ 
//              printf("ERROR : no name for output normalised .fil file provided (use option -o)\n");              
           }
       }else{
          if( gFloat2UChar > 0 ){
             for(int i=0;i<n_sample_size;i++){
                buffer[i] = ( ( buffer[i] - min_value ) / ( max_value - min_value) ) * gFloat2UChar;
             }
          }
          
          if( strlen( gOutNormFilFile.c_str() ) > 0 ){
             if( gFloat2UChar > 0 ){
                for(int i=0;i<n_sample_size;i++){
                   buffer_8bits[i] = buffer[i];                   
                }
                filfile_norm.WriteData( buffer_8bits , n_sample_size );
                
//                static int n_norm_spectra=1;
//                printf("DEBUG : written %d normalised spectra\n",n_norm_spectra++);                
             }else{
                if( normalisation_spectrum.size() > 0 ){
                   filfile_norm.WriteData( buffer , n_sample_size );
                }else{
                   if( gVerb>=5 ){
                      printf("WARNING : normalisation spectrum not provided -> norm .fil file not written\n");
                   }
                }
             }
             if( gVerb > 0 ){
                printf("DEBUG : normalised file written to %s\n",gOutNormFilFile.c_str());
             }
//          }else{
//             printf("ERROR : no name for output normalised .fil file provided (use option -o)\n");
          }
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
       

       if( spectra_count < 10 ){
          for(int ch=0;ch<filfile.nchans();ch++){
             printf("%.2f ",buffer[ch]);
          }
          printf("\n");          
       }
       out_fits.add_line( buffer, filfile.nchans() );
       
       char szOutFile[1024];
       FILE* spectra_f = NULL;
       if( spectra_count == gSaveSpectra ){
          sprintf(szOutFile,"%s/spectrum_%08d.txt",gOutDir.c_str(),spectra_count);
          printf("Opening file %s\n",szOutFile);
          spectra_f = fopen( szOutFile , "w" );
       }

       if( gVerb > 0 ){
          printf("DEBUG test samples = %.2f / %.2f / %.2f\n",buffer[0],buffer[160],buffer[319]);       
       }
       
       double total_power = 0.00;
       for(int i=0;i<n_sample_size;i++){
          avg_power[i] += buffer[i];
          total_power += buffer[i];
          
          if( spectra_count == gSaveSpectra ){
             printf("\t%d %.4f\n",(int)(buffer[i]),buffer[i]);
          }
          
          if( buffer[i] > max_value ){
             max_value = buffer[i];
          }
          if( buffer[i] < min_value ){
             min_value = buffer[i];
          }
 
          if( spectra_f ){          
             fprintf( spectra_f , "%d %.4f\n",i,buffer[i]);
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

   printf("DEBUG : out_fits.get_lines_counter() = %d, out_fits.GetXSize() = %ld\n",out_fits.get_lines_counter(),out_fits.GetYSize());
   out_fits.set_ysize();
   // PrepareBigHornsHeader( double ux_start, double _inttime, double freq_start, double delta_freq_mhz )
   // double freq_start = filfile.fch1();
   double freq_start = filfile.fch1() + gFoffSign*filfile.nchans()*fabs(filfile.foff()); // 2023-01-26 - same as on blc00 (on laptop was just freq_start = filfile.fch1() )
   printf("DEBUG : freq_start = %.4f [MHz]\n",freq_start);
//   double freq_start = filfile.fch1() + filfile.nchans()*filfile.foff(); // ( was - filfile.nchans()*fabs( filfile.foff() ); 
//   double freq_start = 110.00*1.28 - 0.64;
   time_t filfile_uxtime = 0;
   if( gUnixTime > 0 ){
      filfile_uxtime = (time_t)gUnixTime;
   }else{
      filfile_uxtime = get_filfile_uxtime( filfile.name() );
   }
   out_fits.PrepareBigHornsHeader( filfile_uxtime, filfile.tsamp(), freq_start, fabs(filfile.foff()) );
   out_fits.WriteFits( gOutFitsFileName.c_str() );
   printf("INFO : wrote output FITS file to %s\n",gOutFitsFileName.c_str());

   if( true ){
      // write transposed :
      printf("DEBUG : creating transposed FITS file with dimensions (%ld,%d) from original FITS file (%d,%ld)\n",out_fits.GetYSize(), filfile.nchans() , filfile_nchans , out_fits.GetYSize() );      
      CBgFits out_fits_t( out_fits.GetYSize(), out_fits.GetXSize() ); // create horizontal fits (X axis time , Y axis channels)
                                                                   // based on vertical FITS (channels on X axis and time on Y axis): 
                                                                   // CBgFits out_fits( filfile.nchans(), initial_fits_size );

      printf("DEBUG : out_fits size %ld x %ld -> transposing to %ld x %ld\n",out_fits.GetXSize(),out_fits.GetYSize(),out_fits_t.GetXSize(),out_fits_t.GetYSize());
      for(int ch=0;ch<out_fits.GetXSize();ch++){
         for(int t=0;t<out_fits.GetYSize();t++){
            double val = out_fits.getXY( ch, t );
            out_fits_t.setXY( t, ch, val );
         }
      }
      printf("DEBUG : transposition finished\n");fflush(stdout);
      out_fits_t.PrepareBigHornsHeaderTransposed( filfile_uxtime, filfile.tsamp(), freq_start, fabs(filfile.foff()) );
      printf("DEBUG : output FITS header prepared\n");
      out_fits_t.WriteFits( gOutFitsTransposedFileName.c_str() );
      printf("DEBUG : wrote FITS file %s\n",gOutFitsTransposedFileName.c_str() );
      
      // write transposed and averaged in time / frequency :
      int n_time_avg=1000; // 1ms -> 1second (assuming orginally 1ms integrations)
      CBgFits out_fits_t_avg( out_fits_t.GetXSize()/n_time_avg, out_fits_t.GetYSize() ); // create averaged by n_time_avg along the X axis on transposed file
      printf("DEBUG : creating averated dynamic spectrum from vertical file (%ld,%ld) -> (%ld,%ld)\n",out_fits_t.GetXSize(), out_fits_t.GetYSize(),out_fits_t.GetXSize()/n_time_avg, out_fits_t.GetYSize());     
      for(int t=0;t<out_fits_t.GetXSize();t+=n_time_avg){ // loop over start times t along the horizontal axis of out_fits_t (Time)
         for(int ch=0;ch<out_fits_t.GetYSize();ch++){ // for each frequency channel, average n_time_avg - see next loop
            
            double sum = 0.00;
            int n_time_avg_real=0;
            for(int k=0;k<n_time_avg;k++){
               if( (t+k) < out_fits_t.GetXSize() ){
                  sum += out_fits_t.getXY( t + k , ch );
                  n_time_avg_real++;
               }
            }       
            
            int t_avg = int( t / n_time_avg_real );
            out_fits_t_avg.setXY( t_avg, ch, (sum / n_time_avg_real) );
            
            if( t == 0 ){
               float avg_val = float( sum / double(n_time_avg_real) );
               printf("DEBUG : t = %d -> t_avg = %d , ch = %d, value = %.4f\n",t,t_avg,ch,avg_val);
            }
         }
      }
      out_fits_t_avg.PrepareBigHornsHeaderTransposed( filfile_uxtime, filfile.tsamp()*2.00, freq_start, fabs(filfile.foff()) );
      out_fits_t_avg.WriteFits( gOutFitsAvgTransposedFileName.c_str() );
      printf("DEBUG : saved averaged dynamic spectrum to file %s\n",gOutFitsAvgTransposedFileName.c_str() );      
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
   delete [] buffer_8bits;
   if( flip_buffer ){ delete [] flip_buffer; }
}

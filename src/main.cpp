#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <bg_fits.h>

#include "SigprocFile.h"

string gOutDir="spectra/";
int    gSaveSpectra = 0;
string gOutFits;
int gVerb=0;
int gSpectraToProcess=-1;
bool gASKAPData=false;

void usage()
{
   printf("dumpfilfile test.fil [test.fits - specify to save FITS file]\n");   
   printf("\t-n SPECTRA : number of spectra to process [default ALL]\n");
   printf("\t-a : ASKAP .fil files - require some flip\n");
}

void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "n:a";
   int opt,opt_param,i;

   while ((opt = getopt(argc, argv, optstring)) != -1) {
      switch (opt) {
         case 'n':
            if( optarg ){
               gSpectraToProcess = atol(optarg);
            }
            
            break;
            
         case 'a':
            gASKAPData=true;
            break;


         case 'h':
            usage();
            break;



         default:  
            fprintf(stderr,"Unknown option %c\n",opt);
            usage();
      }
   }
}


int main(int argc,char* argv[])
{
   if( argc<2 || (argc==2 && strcmp(argv[1],"-h")==0) ){
      usage();
      exit(0);
   }

   string filfilename = argv[1];
   string outfile = "avg_spectrum.txt";
   if( argc >= 3 ){
      gOutFits = argv[2];
   }
   
   parse_cmdline(argc-1,argv+1);
   
   printf("#########################################################\n");
   printf("PARAMETERS :\n");
   printf("#########################################################\n");
   printf("Spectra to process = %d\n",gSpectraToProcess);
   printf("#########################################################\n");

   // const char* change_ext(const char* name,const char* new_ext,string& out, bool bChangeFileName=false);
   string gOutFitsTransposed;
   change_ext( gOutFits.c_str(), "_transposed.fits", gOutFitsTransposed, true );
   
   CBgFits* pOutFits = NULL;

   SigprocFile filfile( filfilename.c_str() );
   
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
   printf("fch1 = %.4f\n",filfile.fch1());
   printf("foff = %.8f [MHz]\n",filfile.foff());
   printf("tstart = %.4f\n",filfile.tstart());
   printf("tsamp  = %.6f\n",filfile.tsamp());
   printf("outfits = %s ( transposed %s )\n",gOutFits.c_str(),gOutFitsTransposed.c_str());  

   if( strlen(gOutFits.c_str()) > 0 ){
      pOutFits = new CBgFits( filfile.nchans(), 1000 );
   }


   // buffer for 1 spectrum :
//   int bytes_per_spectrum = filfile.nbeams()*filfile.nchans()*filfile.npols()*filfile.nants();
//   int n_sample_size  = filfile.nbeams()*filfile.npols()*filfile.nants();
   int n_sample_size = filfile.nchans();
   uint8_t* buffer = new uint8_t[n_sample_size];
   float* buffer_float = NULL;

   double* avg_power = new double[n_sample_size];
   memset(avg_power,'\0',sizeof(double)*n_sample_size);
 
   double* avg_power_reim = new double[n_sample_size];
   memset(avg_power_reim,'\0',sizeof(double)*n_sample_size);

   FILE* out_total_power_f = fopen("total_power_fil.txt","w"); 
   double max_value=-1e20,min_value=+1e20;     
   int samples_read=0 , spectra_count=0;

   if( gVerb > 0 ){
      printf("DEBUG : filfile.nchans = %d , n_sample_size = %d\n",filfile.nchans(),n_sample_size);   
   }
   
   int line=0;
   while( (samples_read = filfile.read_samples_uint8( 1, buffer, false ) ) > 0  ){
//       if( samples_read != bytes_per_spectrum ){
//          printf("WARNING : samples_read = %d != requested %d at %d spectra\n",samples_read,bytes_per_spectrum,spectra_count);
//          break;
//       }
       if( gVerb > 0 ){
          printf("Sample %d : read %d elements (samples = %d)\n",spectra_count,samples_read,n_sample_size);          
       }
       
       if( gSpectraToProcess > 0 ){
          if( spectra_count >= gSpectraToProcess ){
             break;
          }
       }

       if( pOutFits ){
          if( !buffer_float ){
             buffer_float = new float[n_sample_size];
          }
          for(int i=0;i<n_sample_size;i++){
          
             if( gASKAPData ){
                buffer_float[n_sample_size-1-i] = buffer[i];
             }else{
                buffer_float[i] = buffer[i];
             }
          }
//          pOutFits->set_line( line, buffer_float );
          pOutFits->add_line( buffer_float, n_sample_size );
          line++;
       }

       
       char szOutFile[1024];
       FILE* spectra_f = NULL;
       if( gSaveSpectra > 0 ){
          sprintf(szOutFile,"%s/spectrum_%08d.txt",gOutDir.c_str(),spectra_count);
          printf("Opening file %s\n",szOutFile);
          spectra_f = fopen( szOutFile , "w" );
       }

       if( gVerb > 0 ){
          printf("DEBUG test samples = %d / %d / %d\n",buffer[0],buffer[160],buffer[319]);       
       }
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
          uint8_t re = buffer[i];
          uint8_t im = buffer[i+1];          
          double power = re*re + im*im;
          avg_power_reim[i/2] += power;
          
          // DETECTED POWER VERSION :
          // double power = buffer[i];
          // avg_power_reim[i] += power;
       }
       
       spectra_count++;
   }
   fclose( out_total_power_f );
   if( pOutFits ){
      pOutFits->set_ysize( line );
      
//      double freq_start = filfile.fch1() - filfile.nchans()*fabs(filfile.foff());
      double freq_start = filfile.fch1();
      printf("DEBUG : freq_start = %.4f [MHz]\n",freq_start);
      pOutFits->PrepareBigHornsHeader( get_dttm(), filfile.tsamp(), freq_start, fabs(filfile.foff()) );
      pOutFits->WriteFits( gOutFits.c_str() );
      
      // save transposed too :
      CBgFits out_fits_t( pOutFits->GetYSize(), filfile.nchans() );
      pOutFits->Transpose( out_fits_t );
      out_fits_t.PrepareBigHornsHeaderTransposed( get_dttm(), filfile.tsamp(), freq_start, fabs(filfile.foff()) );
      out_fits_t.WriteFits( gOutFitsTransposed.c_str() );
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
   if( pOutFits ){
      delete pOutFits;
   }
   if( buffer_float ){
      delete [] buffer_float;
   }
}

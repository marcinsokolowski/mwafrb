// PROGAM WRITTERN BY MARCIN SOKOLOWSKI (May 2019) , marcin.sokolowski@curtin.edu.au
//  It converts raw files in psrdada format into .fil (filterbank) file format 
//  filfile.h(cpp) - implementation of class to create .fil (filterbank) files from psrdada files

#include "filfile.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include <bg_fits.h>
#include <libnova_interface.h>


// see /home/msok/Desktop/ASKAP/logbook/fredda_on_J0953.odt
#define VALUE_NOT_SET -1e20
double CFilFile::gMinFITS_Value = VALUE_NOT_SET; // Minimum value = 1401931.37500000
double CFilFile::gMaxFITS_Value = VALUE_NOT_SET; // Maximum value = 5383969.00000000
int gAvgNChannels=0;
double gTimeResInSec = 0.001;
int gVerb = 0;

CFilFile::CFilFile( const char* szFileName )
: m_File(NULL)
{
   if( szFileName && szFileName[0] ){
      m_szFileName = szFileName;
   } 
}

CFilFile::~CFilFile()
{
   Close();
}


void CFilFile::Close()
{
   fclose( m_File );
   m_File = NULL;
}

int CFilFile::Open( const char* szFileName )
{
   if( !m_File ){
       if( szFileName && strcmp(szFileName,m_szFileName.c_str()) ){
           m_szFileName = szFileName;
       }
       m_File = fopen( m_szFileName.c_str(), "wb" );
       
       if( !m_File ){
          printf("ERROR : could not open file %s due to error %s -> cannot continue\n",m_szFileName.c_str(),strerror(errno));
          exit(-1);
       }
   }
   
   return 1;
}

void CFilFile::CheckFile()
{
   if( !m_File ){
      printf("ERROR : file has not been open (requested name = %s) -> exiting !\n",m_szFileName.c_str());
      exit(-1);
   }
}

int CFilFile::ParseHeader( CBgFits& fits )
{
   return ParseHeader( fits , m_Header );
}

int CFilFile::ParseHeader( CBgFits& fits, cFilFileHeader& filHeader, bool bTransposed )
{
//   printf("WARNING : not implemented yet !!!\n");
   
   filHeader.tsamp  = fits.inttime;
   filHeader.nbits  = 8; // WARNING : fil file is by definition a single byte ! sizeof(BG_FITS_DATA_TYPE)*8;   
   filHeader.nbeams = 1;
   filHeader.nsamples = fits.GetXSize(); // timesamples ?
   filHeader.fch1 = fits.start_freq - fits.delta_freq/2.00 + fits.GetYSize()*fits.delta_freq; // was fits.start_freq; // for FREDDA : Should be lowest frequency /home/msok/Desktop/ASKAP/logbook/20190806_flip_of_header.odt
   filHeader.foff = -fits.delta_freq; // in MHz , for FREDDA - lowest freq and positive value here , if stop_freq is used this one should be negative 
   filHeader.fchannel = fits.delta_freq;
   filHeader.nchans = fits.GetYSize();
   
   double mjd = ux2mjd( fits.dtime_fs , fits.dtime_fu );
   filHeader.tstart = mjd;
   
   if( fits.GetKeyword( "RA" ) ){
      filHeader.src_raj = atof( fits.GetKeyword( "RA" )->Value.c_str() );
   }
   if( fits.GetKeyword( "DEC" ) ){
      filHeader.src_dej = atof( fits.GetKeyword( "DEC" )->Value.c_str() );
   }
   if( fits.GetKeyword( "OBJECT" ) ){
      filHeader.source_name = fits.GetKeyword( "OBJECT" )->Value.c_str();
   }
   
   printf("DEBUG : CFilFile::ParseHeader : fch1 = %.6f MHz , foff = %.6f MHz\n",filHeader.fch1,filHeader.foff);
   
   return 0;
}

int CFilFile::WriteHeader( const cFilFileHeader& filHeader )
{
   bool b_tstart_utc = false;

   double mjd = ux2mjd( int(filHeader.tstart), (filHeader.tstart-int(filHeader.tstart))*1000000.00 ); 
//   void get_ymd_hms_ut( time_t ut_time, int& year, int& month, int& day,
//                  int& hour, int& minute, double& sec )
   int year, month, day, hour, minute;
   double sec;
   get_ymd_hms_ut( filHeader.tstart, year, month, day, hour, minute, sec );
   char szUTC[128]; // 2019-07-18T14:53:13.920
   sprintf(szUTC,"%04d-%02d-%02dT%02d:%02d:%.3f",year, month, day, hour, minute, sec );   
   printf("DEBUG : CFilFile::WriteHeader ux = %d -> mjd = %.8f and %s\n",int(filHeader.tstart),mjd,szUTC);
   

   Open();
   
   WriteKeyword( "HEADER_START"  );
   
   // awk '{print "   WriteKeyword( \""$2"\" , filHeader."$2" );";}' keys.tmp > keys.txt
   WriteKeyword( "telescope_id" , filHeader.telescope_id );
   WriteKeyword( "machine_id" , filHeader.machine_id );
   WriteKeyword( "data_type" , filHeader.data_type );
   WriteKeyword( "rawdatafile" , filHeader.rawdatafile.c_str() );
   WriteKeyword( "source_name" , filHeader.source_name.c_str() );
   WriteKeyword( "barycentric" , filHeader.barycentric );
   WriteKeyword( "pulsarcentric" , filHeader.pulsarcentric );
   WriteKeyword( "az_start" , filHeader.az_start );
   WriteKeyword( "za_start" , filHeader.za_start );
   WriteKeyword( "src_raj" , filHeader.src_raj );
   WriteKeyword( "src_dej" , filHeader.src_dej );
   WriteKeyword( "ra_deg" , filHeader.src_raj );
   WriteKeyword( "dec_deg" , filHeader.src_dej );
   WriteKeyword( "gb" , 0.00 );
   WriteKeyword( "gl" , 0.00 );
   WriteKeyword( "tstart" , mjd );
   if( b_tstart_utc ){
      WriteKeyword( "tstart_utc" , szUTC );
   }
   WriteKeyword( "tsamp" , filHeader.tsamp );
   WriteKeyword( "nbits" , filHeader.nbits );
   WriteKeywordChar( "signed" , filHeader.signed_ );
// WriteKeyword( "signed" , filHeader.signed_ );
   WriteKeyword( "nsamples" , filHeader.nsamples );
   WriteKeyword( "nbeams" , filHeader.nbeams );
   WriteKeyword( "ibeam" , filHeader.ibeam );
   WriteKeyword( "fch1" , filHeader.fch1 );
   WriteKeyword( "foff" , filHeader.foff );
   WriteKeyword( "fchannel" , filHeader.fchannel );
   WriteKeyword( "nchans" , filHeader.nchans );
   WriteKeyword( "nifs" , filHeader.nifs );
   WriteKeyword( "refdm" , filHeader.refdm );
   WriteKeyword( "period" , filHeader.period );
   WriteKeyword( "npuls" , filHeader.npuls );
   WriteKeyword( "nbins" , filHeader.nbins );

      
   WriteKeyword( "HEADER_END"  );   
   
   return 0;
}

int CFilFile::WriteString( const char* keyname )
{
    CheckFile();

    // write length of keyword name 
    int len = strlen( keyname );
    int ret = fwrite( &len, 1, sizeof(len), m_File );
    
    ret += fwrite( keyname, 1, len, m_File );
    
    if( ret != (strlen(keyname)+sizeof(int)) ){
       printf("ERROR : error in code did not write keyword name %s correctly (written %d bytes, expected %d bytes)\n",keyname,ret,int(strlen(keyname)+sizeof(int)) );
       exit(-1);
    }
    
    return ret;
}

int CFilFile::WriteKeyword( const char* keyname, int iValue)
{
   int ret = WriteString( keyname );
   
   ret += fwrite( &iValue, 1, sizeof(iValue), m_File );
   
   return ret;
}

int CFilFile::WriteKeyword( const char* keyname, long long llValue)
{
   int ret = WriteString( keyname );
   ret += fwrite( &llValue, 1, sizeof(llValue), m_File );
   
   return ret;
}

int CFilFile::WriteKeyword( const char* keyname, double dValue)
{
   int ret = WriteString( keyname );
   ret += fwrite( &dValue, 1, sizeof(dValue), m_File );

   return ret;
}

int CFilFile::WriteKeyword( const char* keyname, const char* szValue)
{
   int ret = WriteString( keyname );
   
   if( szValue && strlen(szValue) ){
      ret += WriteString( szValue );       
   }
   
   return ret;
}

int CFilFile::WriteKeywordChar( const char* keyname, char cValue)
{
   int ret = WriteString( keyname );
   ret += fwrite( &cValue, 1, sizeof(cValue), m_File );
   
   return ret;
}

int  CFilFile::WriteData( unsigned char* data_uchar, int n_channels )
{
    int ret = fwrite( data_uchar, sizeof(unsigned char), n_channels, m_File );
    
    return ret;
}

int CFilFile::WriteData( float* data_float, int n_channels )
{
    int ret = fwrite( data_float, sizeof(float), n_channels, m_File );
    
    return ret;
}


int CFilFile::ParseDadaHeader( const char* header_buffer, int header_size, cFilFileHeader& filHeader )
{
   // TO BE IMPLEMENTED !
   return 0;
}

void CFilFile::fits2fil( const char* filename, const char* szOutFilFile, const char* szOutSpecFile, int n_channels, int n_timesteps, bool bReScale , bool bTransposed )
{
   if( gVerb ){
     printf("Reading file %s\n",filename);  
   }

   CBgFits infits( filename );
   infits.ReadFits( filename );
   
   CFilFile::fits2fil( infits, szOutFilFile, szOutSpecFile, n_channels, n_timesteps, bReScale, bTransposed );
}

void CFilFile::fits2fil( CBgFits& infits, const char* szOutFilFile, const char* szOutSpecFile, int n_channels, int n_timesteps, bool bReScale, bool bTransposed )
{
  if( !bReScale || (gMinFITS_Value <= VALUE_NOT_SET && gMaxFITS_Value <= VALUE_NOT_SET) ){
     printf("WARNING : no rescaling required in CFilFile::fits2fil\n");
  }

  FILE* outf=NULL;
  if( strlen(szOutSpecFile) ){
     outf = fopen(szOutSpecFile,"w");
  }

  if( 1 )
  {  
      int i=0,count=0;
      
      int string_len=4;      
      
//      CBgFits infits( filename );
//      infits.ReadFits( filename );


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
/*      filheader.fch1   = f_start; // TEST ONLY !!!
      filheader.foff   = foff;
      filheader.tstart = tstart;*/
      
      CFilFile out_filfile( szOutFilFile );
      out_filfile.ParseHeader( infits , filheader, bTransposed );
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
      
      if( gMinFITS_Value > VALUE_NOT_SET && gMaxFITS_Value > VALUE_NOT_SET ){
         printf("INFO(CFilFile : re-scaling range %.8f - %.8f to 0 - 255\n",gMinFITS_Value,gMaxFITS_Value);
      }
      
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
                  
            if( bReScale ){
               if( gMinFITS_Value > VALUE_NOT_SET && gMaxFITS_Value > VALUE_NOT_SET ){
                   scaled_value = 255.00*(scaled_value - gMinFITS_Value)/(gMaxFITS_Value-gMinFITS_Value);
              
                   if( scaled_value > max_value_scaled ){
                      max_value_scaled = scaled_value;
                   }
                   if( scaled_value < min_value_scaled ){
                      min_value_scaled = scaled_value;
                   }
               }               

               if( fabs(gMaxFITS_Value) > 1.00 ){            
                  if( gMaxFITS_Value > VALUE_NOT_SET ){
                     if( scaled_value > gMaxFITS_Value ){
                        scaled_value = gMaxFITS_Value;
                     }
                  }
               }
            }
                  
            power[ch] += val;
            out_buffer[ch] = (unsigned char)scaled_value;
            
            if( gVerb > 1 ){
               if( ch == 0 ){
                  printf("t = %d : %.8f -> %d\n",t,scaled_value,out_buffer[ch]);
               }
            }
         }             
              
         if( gAvgNChannels > 1 ){
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
        if( gVerb > 1 ){
           printf("DEBUG2 : t = %d , value = %d\n",t,out_buffer[0]);
        }
        if( out_floats != final_channels ){
            printf("ERROR : expected to write %d channels whilst written %d -> exiting !\n",final_channels,out_floats);
            break;
        }
              
        n_total_timesteps += 1;
      }      
      printf("%d spectra written new .fil file %s\n",n_total_timesteps,szOutFilFile);
      
      if( 1 ) {
         FILE* out_f = fopen(szOutSpecFile,"w");
         for(int ch=0;ch<n_channels;ch++){
             power[ch] = power[ch] / n_total_timesteps;
          
             fprintf(out_f,"%d %.8f\n",ch,power[ch]);          
         }
         fclose(out_f);
      }

      printf("################################################\n");
      printf("STATISTICS :\n");
      printf("################################################\n");
      printf("min_value = %.4f\n",min_value);
      printf("max_value = %.4f\n",max_value);
      if( gMinFITS_Value > VALUE_NOT_SET && gMaxFITS_Value > VALUE_NOT_SET ){
          printf("scaled_min_value = %.4f\n",min_value_scaled);
          printf("scaled_max_value = %.4f\n",max_value_scaled);

      }
      printf("################################################\n");
      printf("Average spectrum written to file %s\n",szOutSpecFile);
      
      delete [] power;
      delete [] out_buffer;

   }
//  }else{
//     printf("ERROR : could not open FITS file %s\n",filename);
//  }
//  fclose(f);
  
  if( outf ){
     fclose(outf);
  }
}

// PROGAM WRITTERN BY MARCIN SOKOLOWSKI (May 2019) , marcin.sokolowski@curtin.edu.au
//  It converts raw files in psrdada format into .fil (filterbank) file format 
//  filfile.h(cpp) - implementation of class to create .fil (filterbank) files from psrdada files

#ifndef _FILFILE_H__
#define _FILFILE_H__

#include <string>

class CBgFits;

extern int gVerb;
extern int gAvgNChannels;
//extern double gMinFITS_Value;
//extern double gMaxFITS_Value;
extern double gTimeResInSec;

// grep strcmp ../read_filfile.c  | awk '{ ind=index($0,"\"");line=substr($0,ind+1);end=index(line,"\"");key=substr(line,0,end-1);type=substr($7,2,1);type_enum="eFilHdrUnknown";if(type=="f"){type_enum="eFilHdrFlag";}if(type=="i"){type_enum="eFilHdrInt";} if(type=="s"){type_enum="eFilHdrStr";} if(type=="d"){type_enum="eFilHdrDouble";}  if(type=="b"){type_enum="eFilHdrBool";}  print "   "type_enum" "key";";}'
// see also : https://github.com/scottransom/presto/blob/master/lib/python/sigproc.py for some variables 
//      and   https://docs.python.org/3/library/struct.html for types q = C long long etc ...
class cFilFileHeader
{
public :

   int telescope_id;
   int machine_id;
   int data_type;
   std::string rawdatafile;
   std::string source_name;
   int barycentric;
   int pulsarcentric;
   double az_start;
   double za_start;
   double src_raj;
   double src_dej;
   double tstart;
   double tsamp;
   int nbits;
   char signed_; // TODO verify this one -> signed is a restricted keyword in C/C++ -> added _
   int nsamples;
   int nbeams;
   int ibeam;
   double fch1;
   double foff;
   double fchannel;
   int nchans;
   int nifs;
   double refdm;
   double period;
   long long npuls; // ??? number of pulses in the .fil file ???
   int nbins;
   
   cFilFileHeader() 
   : telescope_id(0),
     machine_id(0),
     data_type(0),
     rawdatafile(""),
     source_name(""),
     barycentric(0),
     pulsarcentric(0),
     az_start(0.00),
     za_start(0.00),
     src_raj(0.00),
     src_dej(0.00),
     tstart(0.00),
     tsamp(0.001),
     nbits(8),
     signed_(1), // signed is a restricted keyword in C/C++ -> added _
     nsamples(0),
     nbeams(1),
     ibeam(0),
     fch1(150.00),
     foff(-0.001),
     fchannel(0.001),
     nchans(1280),
     nifs(1),
     refdm(0),
     period(0.00),
     npuls(0),
     nbins(0)
     {}
};

class CFilFile
{
protected :

public :
   std::string m_szFileName;
   FILE*       m_File;
   cFilFileHeader m_Header; // not really used at the moment 

   CFilFile( const char* szFileName );    
   ~CFilFile();
        

   int Open( const char* szFileName=NULL );   
   void Close();
   void CheckFile();

   // parser :
   // return number of correctly recognised DADA keywords which can be translated to FIL keywords :
   int ParseDadaHeader( const char* header_buffer, int header_size, cFilFileHeader& filHeader );
   
   int ParseHeader( CBgFits& fits, cFilFileHeader& filHeader, bool bTransposed=false );
   int ParseHeader( CBgFits& fits );
   
   
   // HEADER :
   int WriteHeader( const cFilFileHeader& filHeader );
   int WriteString( const char* keyname );
   int WriteKeyword( const char* keyname, int iValue);
   int WriteKeyword( const char* keyname, double dValue);
   int WriteKeyword( const char* keyname, long long llValue);
   int WriteKeyword( const char* keyname, const char* szValue=NULL );
   int WriteKeywordChar( const char* keyname, char cValue);
   
   // DATA :
   int WriteData( float* data_float, int n_channels );
   int WriteData( unsigned char* data_uchar, int n_channels );
   
   // conversion :
   static void fits2fil( CBgFits& infits, const char* szOutFilFile, const char* szOutSpecFile, int n_channels, int n_timesteps, bool bReScale=true, bool bTransposed=false );
//                         double f_start=142.0760, double foff=-0.0040, double tstart=58323.2563, double bw=1.28 );
                         
   static void fits2fil( const char* filename, const char* szOutFilFile, const char* szOutSpecFile, int n_channels, int n_timesteps, bool bReScale=true, bool bTransposed=false );
   
   // common globals 
   static double gMinFITS_Value;
   static double gMaxFITS_Value;
};

#endif

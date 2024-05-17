#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

bool is_prime( int number )
{
  bool is_prime = true;
  
  for(int i=2;i<=(number-1);i++){
     if( (number % i) == 0 ){
        return false;
     }
  }
  
  return true;
}

int find_dividers( int number, std::vector<int>& out_dividers )
{
   out_dividers.clear();

   for(int i=2;i<=(number-1);i++){
      if( is_prime(i) ){
         if( (number % i) == 0 ){
            out_dividers.push_back(i);
         }
      }
   }

   return out_dividers.size();
}

int main(int argc,char* argv[])
{
   if( argc==2 && strcmp(argv[1],"-h")==0){
      printf("n_fine_channels N_COARSE_CH N_FINE_CH CALC_SUBBANDS\n");
      exit(0);
   }

   int n_coarse_ch=6;
   if( argc>=2 ){
      n_coarse_ch = atol(argv[1]);
   }
   
   int n_fine_ch = 16;
   if( argc>=3 ){
      n_fine_ch = atol(argv[2]);
   }
   
   bool b_calc_subbands = false;
   if( argc>=4 ){
      b_calc_subbands = (atol(argv[3]) > 0);
   }

   
   double alpha = (400.00/512.00);
   double beta  = (32.00/27.00);
   double nc = ( n_fine_ch*(beta-1)/beta ); // number of fine channels in the overlapping region
   
   int n_edge_channels = (n_fine_ch-int(nc/2))*2;
   int n_centre_channels = (n_coarse_ch-2)*(n_fine_ch-int(nc/2)-(int(nc/2)+1));
   int n_out_channels =  n_edge_channels + n_centre_channels; // number of final fine channels after excluding overlapping regions

   printf("Number of fine channels = %d (%d edge and %d centre)\n",n_out_channels,n_edge_channels,n_centre_channels);
   
   if( b_calc_subbands ){
      if( is_prime( n_out_channels ) ){
         printf("Prime number of fine channels -> nsubbands = %d\n",n_out_channels);
      }else{
         // find maximum reasonable divider ?
      }   
   }
}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char* argv[])
{
   if( argc==2 && strcmp(argv[1],"-h")==0){
      printf("n_fine_channels N_COARSE_CH N_FINE_CH\n");
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
   
   double alpha = (400.00/512.00);
   double beta  = (32.00/27.00);
   double nc = ( n_fine_ch*(beta-1)/beta ); // number of fine channels in the overlapping region
   
   int n_edge_channels = (n_fine_ch-int(nc/2))*2;
   int n_centre_channels = (n_coarse_ch-2)*(n_fine_ch-int(nc/2)-(int(nc/2)+1));
   int n_out_channels =  n_edge_channels + n_centre_channels; // number of final fine channels after excluding overlapping regions

   printf("Number of fine channels = %d (%d edge and %d centre)\n",n_out_channels,n_edge_channels,n_centre_channels);
}
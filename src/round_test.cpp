#include <stdio.h>
#include <stdlib.h>


int main()
{
   int n_coarse_ch=6;
   int n_fine_ch = 16;
   double alpha = (400.00/512.00);
   double beta  = (32.00/27.00);

   double nc = ( double(n_fine_ch)*(beta-1.00)/beta );
   printf("DEBUG : int(nc/2) = %d , nc = %.4f\n",int(nc/2),nc);
   
   int n_edge_channels = (n_fine_ch-int(nc/2))*2;
   int n_centre_channels = (n_coarse_ch-2)*(n_fine_ch-int(nc/2)-(int(nc/2)+1));
   int n_out_channels =  n_edge_channels + n_centre_channels; // number of final fine channels after excluding overlapping regions
   printf("DEBUG : n_fine_ch-int(nc/2) = %d , n_fine_ch-int(nc/2)-(int(nc/2)+1) = %d\n",(n_fine_ch-int(nc/2)),(n_fine_ch-int(nc/2)-(int(nc/2)+1)));

}
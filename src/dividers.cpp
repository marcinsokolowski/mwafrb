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

int find_dividers( int number, std::vector<int>& out_dividers, bool all=true )
{
   out_dividers.clear();

   for(int i=2;i<=(number-1);i++){
      if( is_prime(i) || all ){
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
      printf("dividers NUMBER\n");
      exit(0);
   }

   int n_coarse_ch=6;
   if( argc>=2 ){
      n_coarse_ch = atol(argv[1]);
   }
   
   printf("is_prime(%d) = %d\n",n_coarse_ch,is_prime(n_coarse_ch));
   std::vector<int> dividers;
   find_dividers(n_coarse_ch,dividers);
   if( dividers.size() > 0 ){
      printf("Dividers:\n");
      for(int i=0;i<dividers.size();i++){
         printf("%d\n",dividers[i]);
      }
   }else{ 
      if( is_prime(n_coarse_ch) ){
         printf("%d is a prime number -> no dividers\n",n_coarse_ch);
      }else{
         printf("ERROR in code : %d has no dividers, but is not prime !!!???\n",n_coarse_ch);
      }
   }
   exit(0);   
}

#include <stdio.h>
#include <stdlib.h>
#include "random.h"
// #include "mydate.h"
// #include "mycmnglobals.h"
#include <math.h>
#include <time.h>
// #include <myfits.h>

bool gChangeSeed=false;
bool gExternalSeed=false;
bool CRandom::m_bInitialized=false;
unsigned long CRandom::m_seed=DEFAULT_SEED;

time_t get_dttm()
{
        long gm_time;
        time( &gm_time );
        return gm_time;
}


CRandom::CRandom()
{
	Initialize();
}

CRandom::~CRandom()
{
}

void CRandom::Initialize(unsigned long seed)
{
	if (!m_bInitialized){
		m_seed = seed+get_dttm();
		if( gChangeSeed ){
			srand(m_seed);
		}else{
			if( gExternalSeed>0 ){
            srand( gExternalSeed );
         }else{
				srand(seed); // just temporary to make things more predictable ... :-)
			}
		}
		m_bInitialized = true;
	}
}

double  CRandom::GetRandom()
{
	Initialize();
	double r = double(rand())/double(RAND_MAX+1.0); 
	return r;
}

ULONG_T CRandom::GetRandomInteger(ULONG_T lowerbound,ULONG_T upperbound)
{
	Initialize();
	double r = GetRandom(); 
	ULONG_T ret = lowerbound+(ULONG_T)((upperbound - lowerbound)*r);
	return ret;
}

double CRandom::GetGauss( double sigma, double mean )
{
	Initialize();
	
	double mean50 = mean*50;
	double norm=1.0/(sqrt(2*M_PI)*sigma);
	double sigma2 = sigma*sigma;
	int i=0;
	int max_iter=10000000;
	while( i<=max_iter || 1  ){
		double x = (GetRandom()-0.5)*10*sigma; // ( GetRandom()-0.5 )*mean50;
		double y = GetRandom()*norm;
		double gauss_value = norm*exp( - ( ((x-mean)*(x-mean))/(2.00*sigma2) ) );

		if( y <= gauss_value ){
			return x;
		}

		i++;
	}

	printf("ERROR : value from gauss distribution could not be obtained !\n");
	return -1000;	
}

double CRandom::GetFastGauss( double sigma, double mean )
{
//	return CMyFit::GetGaussFast( sigma, mean );
}
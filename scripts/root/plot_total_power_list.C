// always verify line :
// gROOT->Reset();
#include <TROOT.h>
#include <TF1.h>
#include <TH1F.h>
#include <TPaveText.h>
#include <TGraph.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TMath.h>
#include <TComplex.h>
#include <TFile.h>


// always verify line :
#include <vector>
#include <string>
using namespace std;

#include <unistd.h>
#include <time.h>


// gROOT->Reset();

struct cFileDesc2
{
   char fname[512];
   char comment[512];
   double value_at_100MHz;
};

int gLog=0;
int gVerb=0;
int gDoNormalize=0;
int gDb=0;

// BEDLAM calibration constants :
double gBEDLAM_CALIB_P0 = 0.00;
double gBEDLAM_CALIB_P1 = 3493593612288.00; // @ 170MHz see tone_test_vs_ORBCOMM.odt - 2014-03-10 , 3281797513216.00 at 100MHz (see tone_sweep_100MHz_-136_to_-80_dBm_60s.odt )
                       // 3281797513216.00 - very simlar value 

// BEDLAM Quality Limit :
double gBEDLAM_QualityLimit_dBm = -100 + 63; // dBm + Gain_DB
double gBEDLAM_QualityLimit_dBm_Safe = -110 + 63; // dBm + Gain_DB

double gBandwidthStart=40.00; // MHz 
double gBandwidthEnd=350; // MHz

double gCheckMaxFreq=80.00;

// frequency range :
double gMinFreq=-100.00;
double gMaxFreq=1e20;
double gMaxReasonableValue=10000;

#define N_PAR_COUNT 10
Double_t spectrum_model( Double_t freq )
{
	Double_t par[50];
	for(int i=0;i<50;i++){
		par[i] = 0.00;
	}

/*	par[0]                        = -3.52364e+12   ; // +/-    1.041e+12   
	par[1]                        =  4.13935e+11   ; // +/-    6.51364e+10 
	par[2]                        = -1.02092e+10   ; // +/-    1.7242e+09  
	par[3]                        =  1.23619e+08   ; // +/-    2.53996e+07 
	par[4]                        =      -775448   ; // +/-    230140      
	par[5]                        =      2199.82   ; // +/-    1334.28     
	par[6]                        =     0.427662   ; // +/-    4.96585     
	par[7]                        =   -0.0193973   ; // +/-    0.0114757   
	par[8]                        =  4.62558e-05   ; // +/-    1.49857e-05 
	par[9]                        = -3.58946e-08   ; // +/-    8.44799e-09 */

// cd /media/BighornsData/labtest/tone_test/20140311/ZVL_comparison_100MHz_60sec/ZVL_100mhz_avg10000
// modelspec_poly acc32000_ch0_20140311_023648_000005.ZVL_CALIB_FIT
// MORE DETAILS in tone_test__vs_ORBCOMM.odt 
// this was done by :
//    measuring power with BEDLAM  -> P_bedlam(freq) 
//    measuring power with the ZBL -> P_zvl(freq) -> P_zvl_mW(freq) 
//  and -> calib(freq) = P_bedlam(freq) / P_zvl_mW(freq)
//  later polynomial (as below) was fited to calib(freq) and later is used to calibrated any BEDLAM power to dBm by : 
//  P_bedlam_dBm(freq) = P_bedlam(freq)/calib(freq) 
// this means the method should be robust - no matter how the system is changed it should always relay only on the P_bedlam !!!
par[0] = 4722515362706.85351562500000000000000000000000000000000000000000;
par[1] = -95027966093.02354431152343750000000000000000000000000000000000;
par[2] = 2858485415.97554826736450195312500000000000000000000000000000;
par[3] = -60083923.19348197430372238159179687500000000000000000000000;
par[4] = 788743.02736626658588647842407226562500000000000000000000;
par[5] = -6199.39397118529541330644860863685607910156250000000000;
par[6] = 28.97252980372121555774356238543987274169921875000000;
par[7] = -0.07877571963827538492619595444921287707984447479248;
par[8] = 0.00011499439127208088760705162467701256900909356773;
par[9] = -0.00000006966022305926903079173408313284898696338132;


	Double_t ret=0.00;
   int n = N_PAR_COUNT;
 
   double pow=1.00;
   for(int i=0;i<n;i++){
      double next = pow*par[i];
      
      ret += next;
      pow = pow*freq;
   }

   return ret;
}

int gBedlam2DBM = 0;

#define MAX_ROWS 10000000

#define REPLACE_ELEMS( tab, pos1, pos2 ) { tmp=tab[pos1]; tab[pos1]=tab[pos2]; tab[pos2]=tmp; }
double find_max_val( Double_t* x_values, Double_t* y_values, int count, double freq_min=75, double freq_max=100 );

double ch2freq(int ch)
{
   return (480.00/4096)*ch;
}

Double_t HorizontalLine( Double_t* x, Double_t* y )
{
   return y[0];
}

Double_t Line( Double_t* x, Double_t* y )
{
   Double_t a = y[0];
   Double_t b = y[1];

   //a = (483.269-33121.78)/(68.97-841.87);
   //b = 483.269 - a*68.97;

   return (x[0]*a+b);
   // return (x[0]*y[0]);
}

TGraph* DrawGraph( Double_t* x_values, Double_t* y_values, int numVal, 
         long q, TPad* pPad, int ColorNum = kBlack, int LineStyle=10, int LineWidth=2, const char* szOpt="pA",
         const char* fit_func_name=NULL, 
         double min_y=-10000, double max_y=-10000,
         const char* szStarName="", const char* fname="default",
         int bLog=0, const char* szDescX=NULL, const char* szDescY=NULL,
         double fit_min_x=-100000, double fit_max_x=-100000 )
{
    int MarkerType = 20;

    Double_t z,sigma_z,const_part;
    TF1 *line = NULL;
    TF1 *line_draw = NULL;
   TF1 *part1 = NULL;
   TF1 *part2 = NULL;
    Double_t maxX=-100000,maxY=-100000;
    Double_t minX=100000,minY=100000;

   printf("Drawing %d points in color = %d\n",numVal,ColorNum);

    TGraph* pGraph = new TGraph(q);
    for(int i=0;i<numVal;i++){
       if( gVerb && 0 ){
           printf("DrawGraph : %d : %f %f\n",i, x_values[i], y_values[i] );
        }

        pGraph->SetPoint( i, x_values[i], y_values[i] );

        if(x_values[i]>maxX)
            maxX = x_values[i];
        if(y_values[i]>maxY)
            maxY = y_values[i];
      
        if(x_values[i]<minX)
            minX = x_values[i];
        if(y_values[i]<minY)
            minY = y_values[i];
    }
    printf("Found min_x=%.2f , max_x=%.2f\n",minX,maxX);
    printf("Found min_y=%.2f , max_y=%.2f\n",minY,maxY);
    Double_t stepX = (maxX-minX)/10.00;
    Double_t stepY = (maxY-minY)/10.00;
    printf("Found  stepX=%.2f , stepY=%.2f\n",stepX,stepY);


   double r=1.00;
   double norm = 1;
   double mean = 1;
   double sigma = 1;

   pGraph->SetMarkerStyle(MarkerType);
   pGraph->SetLineStyle(LineStyle);
   pGraph->SetMarkerSize(1);
   pGraph->SetMarkerColor(ColorNum);
   pGraph->SetLineColor(ColorNum);
   pGraph->SetLineWidth(LineWidth);
    if( min_y>-10000 && max_y>-10000 ){
       pGraph->SetMinimum( 2.0*max_y );
       pGraph->SetMaximum( 0.5*max_y );
       printf("Set Min/Max = (%.2f,%.2f)\n",min_y,max_y);
    }else{      
       double minVal = minY-stepY;
       if( bLog ){
          if( minVal<0 ){
             minVal = 1;
          }
       }

//       pGraph->SetMinimum( 0 );
//       pGraph->SetMaximum( 80 );
       printf("Set Min/Max = (%.2f,%.2f)\n",minVal,(maxY+stepY));
    }
//    pGraph->SetMinimum( -90 );
//    pGraph->SetMaximum( 90 );
    pGraph->SetTitle( szStarName );
    pGraph->Draw( szOpt );

   if( fit_min_x<=-100000 ){
      fit_min_x = minX;
   }
   if( fit_max_x<=-100000 ){
      fit_max_x = maxX;
   }

   printf("fitting in x range (%.2f,%.2f)\n",fit_min_x,fit_max_x);

   int local_func=0;
   if( fit_func_name && strlen(fit_func_name) ){
      if( strcmp( fit_func_name, "horline" )==0 || fit_func_name[0]=='h' ){
         printf("Fitting horizontal line\n");
         line = new TF1("fit_func",HorizontalLine,fit_min_x,fit_max_x,1);
         line_draw = new TF1("fit_func2",HorizontalLine,minX,maxX,1);
         local_func=1;
      }
      if( strcmp( fit_func_name, "line" )==0 || fit_func_name[0]=='l' ){
         printf("Fitting straight line\n");
         line = new TF1("fit_func",Line,fit_min_x,fit_max_x,2);
         line_draw = new TF1("fit_func2",Line,minX,maxX,2);
         local_func=1;
      }
   }


   Double_t par[4];
   par[0] = 0.1;
   par[1] = 1.00;
   par[2] = 0.0;
   par[3] = 0.0;


   if( local_func ){
      line->SetParameters(par);
   }
   if( fit_func_name && strlen(fit_func_name) ){
      printf("fitting function : %s",fit_func_name);

      if( strcmp(fit_func_name,"gaus")==0 || fit_func_name[0]=='g' || fit_func_name[0]=='G' ){
         pGraph->Fit("gaus");
      }
               if( strcmp(fit_func_name,"pol1")==0  ){
                   pGraph->Fit("pol1");
//              par[0]=pol1->GetParameter(0);
//         par[1]=pol1->GetParameter(1);   
                }

              if( strcmp(fit_func_name,"pol2")==0  ){
                   pGraph->Fit("pol2");
//                   par[0]=pol2->GetParameter(0);
//                   par[1]=pol2->GetParameter(1);
//                   par[2]=pol2->GetParameter(2);

                }

      if( strstr(fit_func_name,"line") || fit_func_name[0]=='l' || fit_func_name[0]=='L'
          || fit_func_name[0]=='h' || fit_func_name[0]=='H' ){
         pGraph->Fit("fit_func","R");
      }

      if( local_func ){
         line->GetParameters(par);
//         line->Draw("same");
         line_draw->SetParameters(par);
         line_draw->Draw("same");
      }


      TLatex lat;
      lat.SetTextAlign(23);
      lat.SetTextSize(0.05);

      TString szA,szB,szConst,szDesc1,szDesc2;

      printf("minX=%.2f maxX=%.2f minY=%.2f maxY=%.2f\n",minX,maxX,minY,maxY);
//      if( strcmp(fit_func_name,"line")==0 || fit_func_name[0]=='l' || fit_func_name[0]=='L'){



      szDesc1 = "a = ";
      szDesc1 += par[0];

      if( strcmp(fit_func_name,"pol2")==0  ){
             szDesc1 = "y = ";
            szDesc1 += par[0];
            szDesc1 += " + (";
            szDesc1 += par[1];
            szDesc1 += " *m) + (";
            szDesc1 += par[2];
            szDesc1 += " *m^2) i=";
//            szDesc1 +=  i;                 
      }
      if( strcmp(fit_func_name,"pol1")==0  ){
            szDesc1 = "y = ";
            szDesc1 += par[0];
            szDesc1 += " + ("; 
            szDesc1 += par[1];
            szDesc1 += " *m) i=";
//            szDesc1 += i;
      } 

        lat.DrawLatex(minX+(maxX-minX)*0.2,minY+(maxY-minY)*0.7,szDesc1.Data());

      szDesc2 = "b = ";
      szDesc2 += par[1];
        lat.DrawLatex(minX+(maxX-minX)*0.2,minY+(maxY-minY)*0.6,szDesc2.Data());

//      }
   }


   if( szDescX && szDescX[0] ){
      pGraph->GetHistogram()->SetXTitle( szDescX );
   }else{
      pGraph->GetHistogram()->SetXTitle("Number of field observations");
   }
   if( szDescY && szDescY[0] ){
       pGraph->GetHistogram()->SetYTitle( szDescY );
   }else{
      pGraph->GetHistogram()->SetYTitle("Number of new stars");
   }
   // pGraph->GetHistogram()->SetYTitle("sigmaG1_homeo/SigmaG1");

	pGraph->GetXaxis()->SetTitleOffset(1.00);
   pGraph->GetYaxis()->SetTitleOffset(1.00);
   pGraph->GetXaxis()->SetTitleSize(0.05);
   pGraph->GetXaxis()->SetLabelSize(0.05);
   pGraph->GetYaxis()->SetTitleSize(0.05);
   pGraph->GetYaxis()->SetLabelSize(0.05);


   TLatex lat;
   lat.SetTextAlign(23);
   lat.SetTextSize(0.05);
                                                                                
   char szDesc[255];
   sprintf(szDesc,"sigma/mean = %.8f\n",r);
//   lat.DrawLatex( (minX+maxX)/2 , minY+(maxY-minY)/5 , szDesc);

   if( numVal>=10 ){
      TString szOutFile=fname;
      szOutFile += "_sigma.out";
      FILE* out_file = fopen( szOutFile.Data(),"w");
      fprintf(out_file,"%.8f %.8f %.8f %.8f\n",norm,mean,sigma,r);
      fclose(out_file);
   }

   
    return pGraph;
}

void my_sort_float( double* ftab, int cnt )
{
   double divider = ftab[0];
   
   int beg = 1;
   int end = cnt-1;
   double tmp;

   if(cnt){
      while(beg<=end){
         if(ftab[beg]>divider){
            if(ftab[end]<=divider){
               REPLACE_ELEMS( ftab, beg, end )
               beg++;
               end--;
            }else{   
               end--;
            }
         }else{
            beg++;
            if(ftab[end]>divider)
               end--;
         }
      }   
      if(end!=0){
         REPLACE_ELEMS( ftab, end, 0)
      }

      my_sort_float( ftab, end );
      my_sort_float( &(ftab[beg]), cnt-beg );
   }

}


double find_max_val( Double_t* x_values, Double_t* y_values, int count, double freq_min, double freq_max )
{
   double max_val=-1;
   int max_pos=-1;

   for(int i=0;i<count;i++){
      if( freq_min<=x_values[i] && x_values[i]<=freq_max ){
         if( y_values[i] > max_val ){
            max_val=y_values[i];
            max_pos=i;
         }
      }
   }

   if( max_pos >= 0 ){
      double values[100];
      int count=0;
      for(int i=(max_pos-10);i<=(max_pos+10);i++){
         values[count] = y_values[i];
         count++;
      }
      my_sort_float( values, count );

      double median = values[count/2];
      return median;      
   }

   return -1;
}

int ReadResultsFile( const char* fname, Double_t* x_values, Double_t* y_values, int maxRows=MAX_ROWS,
                     int CondCol=-1, int CondValue=-1, int x_col=0, int y_col=0 )
{
   const int lSize=1000;
   char buff[1000];
   
   FILE *fcd = fopen(fname,"r");
   if(!fcd){
       printf("Could not open file : %s\n",fname);
       return -1;
   }

   printf("x_col=%d y_col=%d\n",x_col,y_col);

   Int_t all = 0;
   Double_t fval1,fval2,fval3,fval4,fval5,fval6,mag,x,y;
   long lval1,lval2,lval3,lval4;

   double sum20mhz=0.00;
   double total_sum=0.00;

   all=0;
   int ncols=-1;
   while (1) {
      if( all >= maxRows ){
        break;
      }

      if(fgets(buff,lSize,fcd)==0)
         break;
      if(buff[0]=='#')
         continue;      
      if(strstr(buff,"nan"))
         continue;

      //  9.39  377.000000  8.000000 328.757587  77.088256   298.312992 65.223146   44.506926
      // ncols = sscanf(buff,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",&mag,&x,&y,&fval1,&fval2,&fval3,&fval4,&fval5);
      Double_t x_val;
      Double_t y_val;
//      ncols = sscanf(buff,"%f\t%f\n",&x_val,&y_val);

      int ncols=0;
      char* ptr=NULL;
      char* search_ptr=buff;
      int col=0;
      while( ptr = strtok(search_ptr," \t") ){
         search_ptr = NULL;
         if( gVerb ){
              printf("ptr = %s\n",ptr);
                     }

         if( col == x_col ){
            x_val = atof( ptr );
            ncols++;
         }
         if( col == y_col ){
            y_val = atof( ptr );
            ncols++;

            if( strstr(ptr,"inf") ){
               y_val=0;
            }
         }   

         col++;
      }


      if( ncols < 2 ){
         // changed due to stupid plots , single column rows are skiped
         printf("ERROR - less then 2 columns, line |%s| skiped !!!\n",buff);
         printf("%s\n",buff);
         continue;
         // y_val = x_val;
         // x_val = all;
      }

      if (ncols < 0) break;
     
     if(CondCol>=0){
          if(lval1!=CondValue)
            continue;                                        
     }
     // printf("%s\n",buff);

//     if( x_val < gMinFreq || x_val > gMaxFreq )
//        continue;
	     
     x_values[all] = x_val;
     if( gDb > 0 ){
        y_values[all] = 10.00*TMath::Log10(y_val);
     }else{
        y_values[all] = y_val;
     }

     if( 0 ){
        printf("values : %f %f\n",x_val,y_val);
     }

     all++;
   }
   fclose(fcd);

   printf("Integral up to 20MHz = %e , vs total = %e\n",sum20mhz,total_sum);

   // normalization:
/*   if( gDoNormalize > 0 ){
      double norm_val = find_max_val(x_values,y_values,all,75,100);
      for(int i=0;i<all;i++){
         y_values[i] = y_values[i] / norm_val;
       } 
   }*/

//   exit(0);   
   return all;
}  

int ReadListFile( const char* fname , cFileDesc2* file_list )
{
   const int lSize=1000;
   char buff[1000];
   
   FILE *fcd = fopen(fname,"r");
   if(!fcd){
       printf("Could not open file : %s\n",fname);
       return -1;
   }

   Int_t all = 0;
   Double_t fval1,fval2,fval3,fval4,fval5,fval6,mag,x,y;
   long lval1,lval2,lval3,lval4;

   double sum20mhz=0.00;
   double total_sum=0.00;

   int ncols=-1;
   while (1) {
      if(fgets(buff,lSize,fcd)==0)
         break;
      if(buff[0]=='#')
         continue;      
      if(strstr(buff,"nan"))
         continue;

      if( buff[strlen(buff)-1] == '\n' ){
           buff[strlen(buff)-1] = '\0';
      }

      cFileDesc2 file_desc;
      strcpy(file_desc.comment,buff);
      char* search_ptr = buff;
      const char* ptr = strtok(search_ptr," \t");
      strcpy( file_desc.fname, ptr );
 
//      printf("File %d = \n",all,file_desc.fname);
   
//      ptr = strtok(search_ptr," \t");
//      if( ptr ){
//      if( strlen(buff+strlen(ptr)) > 0 ){
//         strcpy(file_desc.comment,buff);
//      }

      file_list[all] = file_desc;
      all++;
   }
   fclose(fcd);

   return all;
}


void plot_total_power_list( const char* basename="list.txt", 
                            int maxRows=MAX_ROWS,
               const char* fit_func_name=NULL, double min_y=-10000, 
               double max_y=-10000 , int bLog=0,
      const char* szDescX="Time sample index",const char* szDescY="Power [?]", const char* szTitle="Total Power in Stokes I",
      double fit_min_x=-100000, double fit_max_x=-100000,
      int x_col=0, int y_col=1,
      double min_x=-100, double max_x=1e20,
		const char* szImagesDir="images/" )
{
   double bDb=0;
   if( !szTitle){
      szTitle = basename;
   }
   gLog = bLog;
   gDb = bDb;
	gMinFreq = min_x;
   gMaxFreq = max_x;


   if( gDb > 0 && strcmp(szDescY,"Intensity [A.U.]")==0 ){
      szDescY = "Intensity [dB]";
   }
   
   // gROOT->Reset();
   // const char* basename = "s_vs_sigma_g_sqr";

   TCanvas* c1 = new TCanvas("c1","xxxx",200,10,700,500);
   c1->SetGridx();
   c1->SetGridy();
   c1->SetFillColor(0);
   c1->SetFillStyle(0);
   if( bLog ){
      c1->SetLogy(1);
   }
   
   gStyle->SetPadTopMargin(0.03);
   gStyle->SetPadBottomMargin(1.2);
   gStyle->SetPadLeftMargin(0.15);
   gStyle->SetPadRightMargin(0.04);
   gStyle->SetTitleXOffset(1.12);
   gStyle->SetTitleYOffset(1.2);
   gStyle->SetFillColor(0);
   gStyle->SetFillStyle(0);


   Double_t* x_value1 = new Double_t[MAX_ROWS]; // x_value2[MAX_ROWS],x_value3[MAX_ROWS],x_value4[MAX_ROWS],x_value5[MAX_ROWS],x_value6[MAX_ROWS];
   Double_t* y_value1 = new Double_t[MAX_ROWS]; // y_value2[MAX_ROWS],y_value3[MAX_ROWS],y_value4[MAX_ROWS],y_value5[MAX_ROWS],y_value6[MAX_ROWS];
   Double_t maxX=-100000,maxY=-100000;   
   Double_t minX=100000,minY=100000;

   Int_t ncols;
   Int_t lq1=0,lq2=0,lq3=0,lq5=0,lq9=0,lq25=0;

   cFileDesc2 list[1000];
   int file_count = ReadListFile( basename, list );

   TString szTitleFinal = szTitle;
   TMultiGraph *mg = new TMultiGraph();
   int g=0;
   TGraph* graphs[1000];
// mg->SetMinimum(1);  
   if( min_y > -10000 ){
      mg->SetMinimum( min_y );
   }
   if( max_y > -10000 ){
      mg->SetMaximum( max_y );
   }

   TLegend *legend = new TLegend(.45,0.8,0.95,0.95);
   legend->SetTextFont(72);
   legend->SetTextSize(0.02);
   legend->SetNColumns(1);
   legend->SetFillStyle(1001);

   // kBlack - fit to all data
   // kRed - first night
   // kGreen - last night
   int basic_kolors[] = {kBlack,kRed,kBlue,kGreen,5,6,7,8,9,11,12,20,28,29,30,36,38,39,40,41,42,43,44,45,46,47,48,49,10,13,14,15,16,17,18,19,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,8,9,1,2,3,4,5,6,7,8};

   double* sum_tab = new double[MAX_ROWS];
   double* sum2_tab = new double[MAX_ROWS];
   double* rms_tab = new double[MAX_ROWS];
   double* mean_tab = new double[MAX_ROWS];
   int curve_count=0;
   double max_value_100MHz = -1000000; // max value at 100 MHz / channel 853 
   int max_value_100MHz_fileidx=-1;
   int ch100mhz=(int)(gCheckMaxFreq/(480.00/4096.00));
   printf("Channel number at %.2f [MHz] = %d\n",gCheckMaxFreq,ch100mhz);

   for(int i=0;i<file_count;i++){
      cFileDesc2& file_desc = list[i];
      const char* filename = file_desc.fname;
      const char* comment  = file_desc.comment;
      printf("------------------------------------------------------------------- %s -------------------------------------------------------------------\n",filename);

      lq1 = ReadResultsFile( filename, x_value1, y_value1, maxRows, -1, -1, x_col, y_col );

      // for RMS :
      for(int k=0;k<lq1;k++){
         sum_tab[k] += y_value1[k];
         sum2_tab[k] += (y_value1[k] * y_value1[k]);
      }

      TGraph* pNewGraph = NULL;
      TString szOpt="L,same";
      if( i==0 ){
         szOpt="LA";
      }else{
         if( i==1 ){
            szOpt="P,same"; 
         }else{
            szOpt="L,same";
         }
      }

      int kolor = basic_kolors[i];

      int line_style=1;
      int line_width=2;
      if( strstr(filename,"non-optimal") ){
          line_style=2;
      }else{
          if( strstr(filename,"real") && !strstr(filename,"single") ){
              line_style=9;
          }else{
             if( strstr(filename,"single") ){
                 if( strstr(filename,"root") ){
                     line_style=1;
                     line_width=4;
                     kolor = kRed;
                 }else{
                     line_style=1;
                     line_width=4;
                     kolor = kBlack; 
                 }
             }
          }
      }

      pNewGraph = DrawGraph( x_value1, y_value1, lq1, 1, NULL, kolor, line_style, line_width, szOpt.Data(),
                             fit_func_name, min_y, max_y, szTitleFinal.Data(), filename, bLog, szDescX, szDescY, fit_min_x, fit_max_x );
         
      TString szLegend=comment;
      mg->Add(pNewGraph);
      if( i == 0 || i>=2 ){
         legend->AddEntry(pNewGraph,szLegend.Data(),"L");// basename 
      }else{
         legend->AddEntry(pNewGraph,szLegend.Data(),"P");
      }

      szTitleFinal = filename;
      kolor+=2;
      curve_count++;
   }

   legend->Draw();

   FILE* out_f = fopen("rms_and_mean.txt","w");
   // calc mean curve and rms curve :
   for(int k=0;k<lq1;k++){
      double avg = sum_tab[k]/curve_count;
      double mean2 = (sum2_tab[k]/curve_count);

      rms_tab[k] = sqrt( mean2 - avg*avg );
      mean_tab[k] = avg;

      fprintf(out_f,"%.2f %.8f %.8f\n",x_value1[k],mean_tab[k],rms_tab[k]);
   }
   fclose(out_f);
   printf("RMS and MEAN curves saved to file rms_and_mean.txt\n");


   c1->Update();

   TString szEpsName1=szImagesDir;
	szEpsName1 += basename;
   szEpsName1 += ".eps";
   c1->Print(szEpsName1.Data());

   TString szPngName1=szImagesDir;
	szPngName1 += basename;
   szPngName1 += ".png";
   c1->Print(szPngName1.Data());
}

#include "xvt.h"
#include "noddy.h"
#include <math.h>

#define DEBUG(X)  

#define PROFILE_X_OFFSET   20
#define PROFILE_Y_OFFSET   20

#if XVT_CC_PROTO
int DrawCurvesBackground ();
int DrawCurves();
int profile_finished (double [2][11], OBJECTS);
int MoreSymbol(OBJECTS);
int TempSymbol(int, int, OBJECTS);
int closest10(int);
int GetGoodMouse(int *, int *, int, int, int, OBJECTS);
int ProcessPoints(double [2][11], OBJECTS);
int PlotCurve2z(OBJECTS);
int StoreFourierCoeffs(double [2][11]);
int RecallFourierCoeffs (double [2][11]);
int DisplayProfile(double [2][11], OBJECTS);
void four1 ();
#else
int DrawCurvesBackground ();
int DrawCurves();
int profile_finished ();
int MoreSymbol();
int TempSymbol();
int closest10();
int GetGoodMouse();
int ProcessPoints();
int PlotCurve2z();
int StoreFourierCoeffs();
int RecallFourierCoeffs ();
int DisplayProfile();
void four1 ();
#endif
                 /* ********************************** */
                 /* External Globals used in this file */

                 /* ************************* */
                 /* Globals used in this file */
static int x, y;  /* the previous point */
static int x1, y10, x2, y2;
static int polyPointsh[60], polyPointsv[60];
static int secondPointh, secondPointv, lastPointh, lastPointv;
static int noPts = 0;
static float dpts[68];  /* the points of the profile being drawn */


/************************************************************************
*                          *
*  void DrawCurvesBackground()               *
*                          *
*                          *
*  DrawCurvesBackground function draws background for curves window*
*                          *
*  DrawCurvesBackground() takes no arguments       *
*  DrawCurvesBackground() returns no value            *
*                          *
************************************************************************/
int DrawCurvesBackground ()
{
   register int j;
   
                          /* Draw horizontal lines */
   SetColor (COLOR_DKGRAY);
   for(j = 0; j <= 20; j++)
   {
      if(j == 10)
         penSize(2,2);
      else
         penSize(1,1);

      moveTo(PROFILE_X_OFFSET + (-160+160), PROFILE_Y_OFFSET + (j*10));
      lineTo(PROFILE_X_OFFSET + ( 160+160), PROFILE_Y_OFFSET + (j*10));
   }

                          /* Draw vertical lines (some as red spacers */
   for(j=-16+16;j<=16+16;j++)
   {
      if (j%8==0)
      {
         SetColor (COLOR_RED); 
      }
      else
      {
         SetColor (COLOR_DKGRAY);
      }

		moveTo(PROFILE_X_OFFSET + (j*10), PROFILE_Y_OFFSET);
      lineTo(PROFILE_X_OFFSET + (j*10), PROFILE_Y_OFFSET + 200);
   }
   SetColor(COLOR_BLACK);
   moveTo(-140+160,240);
   return (TRUE);
}

/************************************************************************
*                          *
*  void DrawCurves()                *
*                          *
*                          *
*  DrawCurves function mini event loop for curve drawing    *
*                          *
*  DrawCurves() takes no arguments              *
*  DrawCurves() returns no value             *
*                          *
************************************************************************/
int DrawCurves()
{
   int firstPointh,firstPointv;

   noPts=0;  /* reset for next time */
   firstPointh = 0;
   firstPointv = 100;
                              /*ForeColor(redColor);*/
   SetColor (COLOR_RED);
   x=closest10(firstPointh);
   y=closest10(firstPointv);
   x2=x;
   y2=y;
   polyPointsh[noPts]=x;
   polyPointsv[noPts]=y;
   noPts++;
   penSize(3,3);
   return (TRUE);
}

int
#if XVT_CC_PROTO
profile_finished (double fourierCoeff[2][11], OBJECTS type)
#else
profile_finished (fourierCoeff, type)
double fourierCoeff[2][11];
OBJECTS type;
#endif
{
          /* get the fourier coefficients to use now the profile finished */
   ProcessPoints(fourierCoeff, type);  /* store the profile drawn */
   SetColor (COLOR_BLACK);
   noPts=0;  /* reset for next time */
   return (TRUE);
}

/************************************************************************
*                          *
*  BOOLEAN MoreSymbol()                *
*                          *
*                          *
*  MoreSymbol function handles one point on curve selection *
*                          *
*  MoreSymbol() takes no arguments              *
*  MoreSymbol() returns true if # of pts >32 or final point of *
*  curve is selected                *
*                          *
************************************************************************/
int
#if XVT_CC_PROTO
MoreSymbol(OBJECTS type)
#else
MoreSymbol(type)
OBJECTS type;
#endif
{
   int i, last = FALSE;
   int basePos;

   moveTo(PROFILE_X_OFFSET + x, PROFILE_Y_OFFSET + y);
   lineTo(PROFILE_X_OFFSET + x1, PROFILE_Y_OFFSET + y10);
   penSize(1, 1);

   polyPointsh[noPts] = x1;
   polyPointsv[noPts] = y10;
   noPts++;

   switch (type)
   {
      case (FOLD):
         if ((x1==320 && y10==100) || (noPts == 32))
            last = TRUE;
         break;
      case (FAULT): case (SHEAR_ZONE) :
         if ((x1 >= 320) || (noPts == 16))
         {
              /* Mirror the selected points so
              ** that the function in periodic */
            basePos = polyPointsh[noPts-1];
            for (i = noPts-2; i >= 0; i--)
            {
               polyPointsh[noPts] = basePos
                                    + (basePos - polyPointsh[i]);
               polyPointsv[noPts] = polyPointsv[i];
               noPts++;
            }
            last = TRUE;
         }
         break;
      default:
         x2 = x1;
         y2 = y10;
   }
   return (last);
}

/************************************************************************
*                          *
*  void TempSymbol()                *
*                          *
*                          *
*  TempSymbol function draws moving line from last point to cursor   *
*                          *
*  TempSymbol() takes no arguments              *
*  TempSymbol() returns no value             *
*                          *
************************************************************************/
int
#if XVT_CC_PROTO
TempSymbol(int aPointh, int aPointv, OBJECTS type)
#else
TempSymbol(aPointh,aPointv, type)
int aPointh,aPointv;
OBJECTS type;
#endif
{
   penSize(3,3);

   aPointh -= PROFILE_X_OFFSET;
   aPointv -= PROFILE_Y_OFFSET;

   x=polyPointsh[noPts-1];
   y=polyPointsv[noPts-1];

   moveTo(PROFILE_X_OFFSET + x, PROFILE_Y_OFFSET + y);
   lineTo(PROFILE_X_OFFSET + x2, PROFILE_Y_OFFSET + y2);

   GetGoodMouse(&secondPointh,&secondPointv,aPointh,aPointv,x, type);      

   x1=closest10(secondPointh);
   y10=closest10(secondPointv);

   moveTo(PROFILE_X_OFFSET + x, PROFILE_Y_OFFSET + y);
   lineTo(PROFILE_X_OFFSET + x1, PROFILE_Y_OFFSET + y10);

   x2=x1;
   y2=y10;     
   
   return (TRUE);
}

/************************************************************************
*                          *
*  int closest10(i)                 *
*                          *
*  int i; number to be rounded               *
*                          *
*  closest10 function rounds an integer to the nearest decade  *
*                          *
*  closest10() takes no arguments               *
*  closest10() returns rounded number           *
*                          *
************************************************************************/
int
#if XVT_CC_PROTO
closest10(int i)
#else
closest10(i)
int i;
#endif
{
   int jj;
   
   jj=i+5;

   jj/=10;
   jj*=10;

   return(jj);
}

/************************************************************************
*                          *
*  void GetGoodMouse(goodPointh,goodPointv,h)         *
*                          *
*  Point    *goodPoint; pointer to location where mouse was clicked  *
*  int      h; horizontal value oflast good point     *
*                          *
*  GetGoodMouse function locates mouse click and checks for validity*
*                          *
*  GetGoodMouse() returns no value              *
*                          *
************************************************************************/
int
#if XVT_CC_PROTO
GetGoodMouse(int *goodPointh, int *goodPointv,
             int aPointh, int aPointv, int h, OBJECTS type)
#else
GetGoodMouse(goodPointh,goodPointv,aPointh,aPointv,h, type)
int *goodPointh,*goodPointv,aPointh,aPointv;
int h;
OBJECTS type;
#endif
{
   if (type == FOLD)
   {
      if (aPointh >= 160+160)
      {
         *goodPointh = 160+160;
         *goodPointv = 100;
         return (TRUE);
      }
   }
   
   if(aPointv>200 || aPointv<0)
   {
      *goodPointh=h;
      *goodPointv=100;
   }
   else if(aPointh<h)
   {
      *goodPointh=h;
      *goodPointv=aPointv;
   }
   else
   {
      *goodPointh=aPointh;
      *goodPointv=aPointv;
   }
   return (TRUE);
}     

int
#if XVT_CC_PROTO
ProcessPoints(double fourierCoeff[2][11], OBJECTS type)
#else
ProcessPoints(fourierCoeff, type)
double fourierCoeff[2][11];
OBJECTS type;
#endif
{
   int i,j;
   BOOLEAN quitit;
   double dx,dy,ratx;
   int scale;
   
   if (type == FOLD)
      scale = 10;
   else if ((type == FAULT) || (type == SHEAR_ZONE))
      scale = 20;
   else
      scale = 10;  /* default */

   dpts[1]= (float) 100.0;  /* Where the first point always is */
   dpts[2]= (float) 0.0;

   for(i=1;i<=32;i++)
   {
      quitit=FALSE;
      j=0;
      do
      {
         if (polyPointsh[j]<((i*scale) ) && polyPointsh[j+1]>=((i*scale)))
            quitit=TRUE;
         else
            j++;
      } while(!quitit);

      dpts[((i+1)*2)] = (float) 0.0;

      dx=(float) (polyPointsh[j+1]-polyPointsh[j]);
      dy=(float) (polyPointsv[j+1]-polyPointsv[j]);
      ratx=((i*scale)-polyPointsh[j])/dx;
      dpts[(i*2)+1] = (float) (polyPointsv[j] + (ratx*dy));
   }

   for (i=1;i<=64;i+=2)
      dpts[i] = (float) ((dpts[i]-100.0)/100.0);

   four1(dpts,32,1);
   
   PlotCurve2z(type);
   /*while(!Button()); ?????????*/
   StoreFourierCoeffs(fourierCoeff);   
   return (TRUE);
}

int
#if XVT_CC_PROTO
PlotCurve2z(OBJECTS type)
#else
PlotCurve2z(type)
OBJECTS type;
#endif
{
   int i,xh,yv;
   double ans, dx;
   int numTermsToPlot;

   SetColor (COLOR_BLACK); 
   moveTo(PROFILE_X_OFFSET, PROFILE_Y_OFFSET + 100);
   
   if (type == FOLD)
      numTermsToPlot = 128;
   else
      numTermsToPlot = 64;

   for(i = 0; i <= numTermsToPlot; i++)
   {
      dx=2.0*(i/128.0)*3.1415927;
      ans=fourtoo(dpts, dx)/16.0;
      
      if (type == FOLD)
         xh=(int)(i*2.5);
      else
         xh=(int)(i*5.0);
      yv=(int)(ans*100)+100;

      lineTo(PROFILE_X_OFFSET + xh, PROFILE_Y_OFFSET + yv);
   }
   return (TRUE);
}

int
#if XVT_CC_PROTO
StoreFourierCoeffs(double fourierCoeff[2][11])
#else
StoreFourierCoeffs(fourierCoeff)
double fourierCoeff[2][11];
#endif
{
   int i,j,k=1;

   for (j = 0; j < 11; j++) 
   {
      for (i = 0; i < 2; i++) 
      {
         fourierCoeff[i][j] = (double) -dpts[k++]/16.0;
      }
   }
   return (TRUE);
}     

int
#if XVT_CC_PROTO
RecallFourierCoeffs (double fourierCoeff[2][11])
#else
RecallFourierCoeffs (fourierCoeff)
double fourierCoeff[2][11];
#endif
{
   int i,j,k=1;

   for (j = 0; j < 11; j++) 
   {
      for(i = 0; i < 2; i++) 
      {
         dpts[k++]= (float) (-fourierCoeff[i][j]*16.0);
      }
   }
   return (TRUE);
}     
         
int
#if XVT_CC_PROTO
DisplayProfile(double fourierCoeff[2][11], OBJECTS type)
#else
DisplayProfile(fourierCoeff, type)
double fourierCoeff[2][11];
OBJECTS type;
#endif
{
   DrawCurvesBackground();
   DrawCurves();

   RecallFourierCoeffs(fourierCoeff);
   penSize(2,2);
   PlotCurve2z(type);

   SetColor (COLOR_BLACK);
   return (TRUE);
}

int
#if XVT_CC_PROTO
OldProfile(double fourierCoeff[2][11])
#else
OldProfile(fourierCoeff)
double fourierCoeff[2][11];
#endif
{
   RecallFourierCoeffs(fourierCoeff);
   penSize(2,2);
   PlotCurve2z(FOLD);
   return (TRUE);
}





/************************************************************************
*                          *
*  void four1(data,nn,isign)              *
*  float data[];  array of real & complex data        * 
*  int nn,isign;  nn is size of array of complex data    *
*        isign is forward (+1) or reverse (-1) fft flag  *
*                          *
*  four1 function performs FFT               *
*     from numerical recipes in C            * 
*                          *
*  four1() takes 3 arguments              *
*  four1() returns no value               *
*                          *
************************************************************************/
void
#if XVT_CC_PROTO
four1(float data[], int nn, int isign)
#else
four1(data,nn,isign)
float data[];
int nn,isign;
#endif
{
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
   int n,mmax,m,j,istep,i;
   double wtemp,wr,wpr,wpi,wi,theta;
   float tempr,tempi;

   n=nn << 1;
   j=1;
   for (i=1;i<n;i+=2) {
      if (j > i) {
         SWAP(data[j],data[i]);
         SWAP(data[j+1],data[i+1]);
      }
      m=n >> 1;
      while (m >= 2 && j > m) {
         j -= m;
         m >>= 1;
      }
      j += m;
   }
   mmax=2;
   while (n > mmax) {
      istep=2*mmax;
      theta=6.28318530717959/(isign*mmax);
      wtemp=sin(0.5*theta);
      wpr = -2.0*wtemp*wtemp;
      wpi=sin(theta);
      wr=1.0;
      wi=0.0;
      for (m=1;m<mmax;m+=2) {
         for (i=m;i<=n;i+=istep) {
            j=i+mmax;
            tempr = (float) (wr*data[j]-wi*data[j+1]);
            tempi = (float) (wr*data[j+1]+wi*data[j]);
            data[j]=data[i]-tempr;
            data[j+1]=data[i+1]-tempi;
            data[i] += tempr;
            data[i+1] += tempi;
         }
         wr=(wtemp=wr)*wpr-wi*wpi+wr;
         wi=wi*wpr+wtemp*wpi+wi;
      }
      mmax=istep;
   }
#undef SWAP
}


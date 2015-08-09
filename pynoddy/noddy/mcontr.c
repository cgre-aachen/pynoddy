#include <math.h>
#include "xvt.h"
#include "noddy.h"

#define DEBUG(X)  
                 /* ********************************** */
                 /* External Globals used in this file */
#if XVT_CC_PROTO
extern int lastdiff (unsigned char *, unsigned char *);
#else
extern int lastdiff ();
#endif
                 /* ************************* */
                 /* Globals used in this file */
#define MAX_CONTOURS 55

/************************************************************************
*                          *
*                          *
*                          *
*  mcontr(zmap,jmax,kmax,conval,ncon,cutoff,sc,xo,yo,flag)     *  
*                          *
*  double **zmap  array of pointers to start of each row of data matrix*
*  int jmax    vertical dimension of data array *
*  int kmax    horizontal dimension of data array  *
*  double conval[20] array of contour intervals to be plotted*
*  int ncon    number of contour intervals      *
*  double cutoff  value above which data point will be ignored *
*  double scale   scaling factor for contour plot     *
*  double xo      x offset for contour plot     *
*  double yo      y offset for contour plot     *
*  BOOLEAN flag   force single colour & width scaling *
*                          *
*  mcontr function for countouring any orthogonally gridded data set*
*                          *
*  this function assumes a coordinate space such that zmap[0][0]  *
*  is the origin, which is in the botton left corner of the area  *
*  to be contoured, and up and right are positive in both      *
*  coordinate and indexing space but for this version x & y are   *
*  swapped!!!!!!                    *
*                          *
*  the cutoff term allows you to contour data which is on a *
*  regular grid but is notavailable everywhere        *
*                          *
*************************************************************************/
int
#if XVT_CC_PROTO
mcontr (double **zmap, int jmax, int kmax, double *conval,
        int ncon, double cutoff, double sc, double xo, double yo,
        BOOLEAN flag, unsigned int flavor, int iign,
        COLOR *layerColors)
#else
mcontr (zmap, jmax, kmax, conval, ncon, cutoff, sc, xo, yo, flag,
        flavor, iign, layerColors)
double **zmap, sc;
double cutoff, xo, yo;
double *conval;
int jmax, kmax, ncon;   
BOOLEAN flag;
unsigned int flavor;
int iign; /* event index if the event is a Dyke or a Plug */
COLOR *layerColors;
#endif
{
   COLOR color;
   int tj1, tk1, tj2, tk2;
   int pnts[13][3];
   int icon[MAX_CONTOURS];
   double xscal, yscal, conx[MAX_CONTOURS][2], cony[MAX_CONTOURS][2];
   register int j, k, l, m, inc, itri, iside;
   double x1, y1, z1, x2, y2, z2, delcon;
   
DEBUG(printf("\nMCONTR: cutoff = %f, xo = %f, yo = %f, jmax = %d, kmax = %d, ncon = %d, flag = %d, flavor = %d, iign = %d",cutoff, xo, yo, jmax, kmax, ncon, flag, flavor, iign);)
   SetColor (COLOR_BLACK);

   xscal=sc;
   yscal=-sc;
        
   pnts[1][1]=1;
   pnts[1][2]=0;
   pnts[2][1]=0;
   pnts[2][2]=1;
   
   pnts[3][1]=0;
   pnts[3][2]=0;
   pnts[4][1]=0;
   pnts[4][2]=1;
   
   pnts[5][1]=0;
   pnts[5][2]=0;
   pnts[6][1]=1;
   pnts[6][2]=0;
   
   pnts[7][1]=1;
   pnts[7][2]=0;
   pnts[8][1]=0;
   pnts[8][2]=1;
     
   pnts[9][1]=0;
   pnts[9][2]=1;
   pnts[10][1]=1;
   pnts[10][2]=1;
     
   pnts[11][1]=1;
   pnts[11][2]=0;
   pnts[12][1]=1;
   pnts[12][2]=1;
   
   for (j = 1; j <= jmax; j++)
   {
      for (k = 1; k <= kmax; k++)
      {
         for (itri = 1, inc = 1; itri <= 2; itri++)
         {
            for (m=0; m < ncon; m++)
            {
               icon[m]=0;
            }

            for (iside = 1; iside <= 3; iside++)
            {
               tj1= j+pnts[inc][1];
               tk1= k+pnts[inc][2];
               z1 = zmap[tj1][tk1];
               tj2= j+pnts[inc+1][1];
               tk2= k+pnts[inc+1][2];
               z2 = zmap[tj2][tk2];
               y1 = yo-(tj1*yscal);
               x1 = xo+(tk1*xscal);
               y2 = yo-(tj2*yscal);
               x2 = xo+(tk2*xscal);
               inc= inc+2;
        
               if ((j==1 && k==1)  && iside==1)
               {
                  mplot(yo-.125,xo,3);
                  mplot(yo+.125,xo,2);
                  mplot(yo,xo-.125,3);
                  mplot(yo,xo+.125,2);
               }           
               else if ((j==jmax && k==kmax)  && iside==1)
               {
                  mplot(yo-((jmax+1)*yscal)-.125,xo+((kmax+1)*xscal),3);
                  mplot(yo-((jmax+1)*yscal)+.125,xo+((kmax+1)*xscal),2);
                  mplot(yo-((jmax+1)*yscal),xo+((kmax+1)*xscal)-.125,3);
                  mplot(yo-((jmax+1)*yscal),xo+((kmax+1)*xscal)+.125,2);
               }           

               for (l=0;l<ncon;l++)
               {
                  if (z1 < cutoff && z2 < cutoff)
                  {
                     if ((z1 < conval[l+1] && z2 >= conval[l+1])
                      || (z1 >= conval[l+1] && z2 < conval[l+1]))
                     {
                        icon[l]=icon[l]+1;
                        delcon=(conval[l+1]-z1)/(z2-z1);
                        if (iside==1) 
                        {
                           conx[l][icon[l]-1]=((x2-x1)*delcon)+x1;
                           cony[l][icon[l]-1]=y1-((y1-y2)*delcon);
                        }
                        else
                        {
                           conx[l][icon[l]-1]=((x2-x1)*delcon)+x1;
                           cony[l][icon[l]-1]=((y2-y1)*delcon)+y1;
                        }
                     }
                  }
               }
            }

            for (l = 0; l < ncon; l++)
            {
               if(icon[l] == 2)
               {
                  if (flag)
                  {
                     SetColor (COLOR_GREEN);
                     penSize(2,2);
                  }
                  else
                  {
                     if(!iign)
                     {
                        if (l < ncon-1)
                        {
                           if (layerColors)
                              color = layerColors[l+1];
                           else
                              color = COLOR_GREEN;
                        }
                        else
                        {
                           if (flavor == UNC_STRAT)
                              if (layerColors)
                                 color = layerColors[0];
                              else
                                 color = COLOR_GREEN;
                              else
                                 color = COLOR_RED; /* Red for IGNEOS */
                        }
                     }
                     else
                        color = COLOR_RED; /* Red for IGNEOS */

                     SetColor (color);
                  }  
                  mplot(cony[l][0]-0.05, conx[l][0]-0.17, 3);
                  mplot(cony[l][1]-0.05, conx[l][1]-0.17, 2);
               }
            }
         }   
      }
   }
   penSize(1,1);
   SetColor (COLOR_BLACK);
DEBUG(printf("\nMCONTR: finished");)
   return (TRUE);
}  


/************************************************************************
  coutourImage(zmap,jmax,kmax,conval,ncon,cutoff,sc,xo,yo,flag)

  unsigned char **zmap  array of pointers to start of each row of data matrix
  int jmax    vertical dimension of data array 
  int kmax    horizontal dimension of data array  
  double conval[20] array of contour intervals to be plotted
  int ncon    number of contour intervals      
  double cutoff  value above which data point will be ignored 
  double scale   scaling factor for contour plot     
  double xo      x offset for contour plot     
  double yo      y offset for contour plot     
  BOOLEAN flag   force single colour & width scaling 
                          
  mcontr function for countouring any orthogonally gridded data set

  this function assumes a coordinate space such that zmap[0][0]  
  is the origin, which is in the botton left corner of the area  
  to be contoured, and up and right are positive in both      
  coordinate and indexing space but for this version x & y are   
  swapped!!!!!!                    
                          
  the cutoff term allows you to contour data which is on a 
  regular grid but is notavailable everywhere        

*************************************************************************/
int
#if XVT_CC_PROTO
coutourImage (XVT_PIXMAP pixmap, unsigned char **zmap,
              int jmax, int kmax, double *conval, int ncon,
              double sc, int xo, int yo, COLOR *layerColors)
#else
coutourImage (pixmap, zmap, jmax, kmax, conval, ncon, sc, xo, yo, layerColors)
XVT_PIXMAP pixmap;
unsigned char **zmap;
double sc;
int xo, yo;
double *conval;
int jmax, kmax, ncon;   
COLOR *layerColors;
#endif
{
   int tj1, tk1, tj2, tk2;
   int pnts[13][3];
   int icon[MAX_CONTOURS];
   double xscal, yscal;
   int conx[MAX_CONTOURS][2], cony[MAX_CONTOURS][2];
   register int j, k, l, inc, itri, iside, index;
   double x1, y1, z1, x2, y2, z2, delcon;
   
   setCurrentDrawingWindow (pixmap);
   penSize(0, 0);

   SetColor (COLOR_BLACK);

   xscal=sc;
   yscal=-sc;
        
   pnts[1][1]=1;
   pnts[1][2]=0;
   pnts[2][1]=0;
   pnts[2][2]=1;
   
   pnts[3][1]=0;
   pnts[3][2]=0;
   pnts[4][1]=0;
   pnts[4][2]=1;
   
   pnts[5][1]=0;
   pnts[5][2]=0;
   pnts[6][1]=1;
   pnts[6][2]=0;
   
   pnts[7][1]=1;
   pnts[7][2]=0;
   pnts[8][1]=0;
   pnts[8][2]=1;
     
   pnts[9][1]=0;
   pnts[9][2]=1;
   pnts[10][1]=1;
   pnts[10][2]=1;
     
   pnts[11][1]=1;
   pnts[11][2]=0;
   pnts[12][1]=1;
   pnts[12][2]=1;
   
   for (j = 0; j <= jmax; j++)
   {
      for (k = 0; k <= kmax; k++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (itri = 0, inc = 1; itri < 2; itri++)
         {                          /* Zero Memory */
            memset (icon, 0, sizeof(int)*ncon);
            memset (conx, 0, sizeof(int)*ncon*2);
            memset (cony, 0, sizeof(int)*ncon*2);

            for (iside = 0; iside < 3; iside++)
            {
               tj1 = j + pnts[inc][1];
               tk1 = k + pnts[inc][2];
               tj2 = j + pnts[inc+1][1];
               tk2 = k + pnts[inc+1][2];
               z1  = (double) zmap[tk1][tj1];
               z2  = (double) zmap[tk2][tj2];
               y1  = yo - (tj1*yscal);
               x1  = xo + (tk1*xscal);
               y2  = yo - (tj2*yscal);
               x2  = xo + (tk2*xscal);
               inc += 2;

               for (l = 0; l < ncon; l++)
               {
                  if ((z1 <  conval[l] && z2 >= conval[l]) ||
                      (z1 >= conval[l] && z2 <  conval[l]))
                  {
                     delcon = (conval[l]-z1)/(z2-z1);
                     index = icon[l];   
                     if (iside == 0) 
                     {
                        conx[l][index] = (int) floor((x2-x1)*delcon + x1 + 0.5);
                        cony[l][index] = (int) floor(y1 - (y1-y2)*delcon + 0.5);
                     }
                     else
                     {
                        conx[l][index] = (int) floor((x2-x1)*delcon + x1 + 0.5);
                        cony[l][index] = (int) floor((y2-y1)*delcon + y1 + 0.5);
                     }
                     icon[l] = index + 1; 
                     if (index > 1) /* 2 is the maximum so dont check any more */
                        break;
                  }
               }
            }

            for (l = 0; l < ncon; l++)
            {
               if ((conx[l][0] != conx[l][1]) || (cony[l][0] != cony[l][1]))
               {
                  if (layerColors)
                     SetColor(layerColors[l]);
                  else
                     SetColor(COLOR_GREEN);

                  moveTo (cony[l][0], conx[l][0]); /* mplot(cony[l][1]-0.05, conx[l][1]-0.17, 3); */
                  lineTo (cony[l][1], conx[l][1]); /* mplot(cony[l][2]-0.05, conx[l][2]-0.17, 2); */
               }
            }
         }   
      }
   }

   return (TRUE);
}  


int
#if XVT_CC_PROTO
coutourImageFromDoubleData (XVT_PIXMAP pixmap, double **zmap,
              int jmax, int kmax, int jStart, int kStart, int flipXY, int reverseJ, int reverseK,
              double *conval, int ncon, double sc, int xo, int yo,
              COLOR *layerColors)
#else
coutourImageFromDoubleData (pixmap, zmap, jmax, kmax, jStart, kStart, flipXY, reverseJ, reverseK,
									 conval, ncon, sc, xo, yo, layerColors)
XVT_PIXMAP pixmap;
double **zmap;
double sc;
int xo, yo, jStart, kStart, flipXY, reverseJ, reverseK;
double *conval;
int jmax, kmax, ncon;   
COLOR *layerColors;
#endif
{
   int tj1, tk1, tj2, tk2;
	int index11, index12, index21, index22;
   int pnts[13][3];
   int icon[MAX_CONTOURS];
   double xscal, yscal;
   int conx[MAX_CONTOURS][2], cony[MAX_CONTOURS][2];
   register int j, k, l, inc, itri, iside, index;
   double x1, y1, z1, x2, y2, z2, delcon;
   
   setCurrentDrawingWindow (pixmap);
   penSize(0, 0);

   SetColor (COLOR_BLACK);

   xscal=sc;
   yscal=-sc;
        
   pnts[1][1]=1;
   pnts[1][2]=0;
   pnts[2][1]=0;
   pnts[2][2]=1;
   
   pnts[3][1]=0;
   pnts[3][2]=0;
   pnts[4][1]=0;
   pnts[4][2]=1;
   
   pnts[5][1]=0;
   pnts[5][2]=0;
   pnts[6][1]=1;
   pnts[6][2]=0;
   
   pnts[7][1]=1;
   pnts[7][2]=0;
   pnts[8][1]=0;
   pnts[8][2]=1;
     
   pnts[9][1]=0;
   pnts[9][2]=1;
   pnts[10][1]=1;
   pnts[10][2]=1;
     
   pnts[11][1]=1;
   pnts[11][2]=0;
   pnts[12][1]=1;
   pnts[12][2]=1;
   
   for (j = 0; j <= jmax; j++)
   {
      for (k = 0; k <= kmax; k++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (itri = 0, inc = 1; itri < 2; itri++)
         {                          /* Zero Memory */
            memset (icon, 0, sizeof(int)*ncon);
            memset (conx, 0, sizeof(int)*ncon*2);
            memset (cony, 0, sizeof(int)*ncon*2);

            for (iside = 0; iside < 3; iside++)
            {
               tj1 = j + pnts[inc][1];
               tk1 = k + pnts[inc][2];
               tj2 = j + pnts[inc+1][1];
               tk2 = k + pnts[inc+1][2];
					if (flipXY)
					{
						if (reverseJ)
						{
							index11 = (jmax-tj1)+jStart;
							index21 = (jmax-tj2)+jStart;
						}
						else
						{
							index11 = tj1+jStart;
							index21 = tj2+jStart;
						}

						if (reverseK)
						{
							index12 = (kmax-tk1)+kStart;
							index22 = (kmax-tk2)+kStart;
						}
						else
						{
							index12 = tk1+kStart;
							index22 = tk2+kStart;
						}

						z1  = (double) zmap[index11][index12];
						z2  = (double) zmap[index21][index22];
					}
					else
					{
						if (reverseJ)
						{
							index12 = (jmax-tj1)+jStart;
							index22 = (jmax-tj2)+jStart;
						}
						else
						{
							index12 = tj1+jStart;
							index22 = tj2+jStart;
						}

						if (reverseK)
						{
							index11 = (kmax-tk1)+kStart;
							index21 = (kmax-tk2)+kStart;
						}
						else
						{
							index11 = tk1+kStart;
							index21 = tk2+kStart;
						}

						z1  = (double) zmap[index11][index12];
						z2  = (double) zmap[index21][index22];
					}
               y1  = yo - (tj1*yscal);
               x1  = xo + (tk1*xscal);
               y2  = yo - (tj2*yscal);
               x2  = xo + (tk2*xscal);
               inc += 2;

               for (l = 0; l < ncon; l++)
               {
                  if ((z1 <  conval[l] && z2 >= conval[l]) ||
                      (z1 >= conval[l] && z2 <  conval[l]))
                  {
                     delcon = (conval[l]-z1)/(z2-z1);
                     index = icon[l];   
                     if (iside == 0) 
                     {
                        conx[l][index] = (int) floor((x2-x1)*delcon + x1 + 0.5);
                        cony[l][index] = (int) floor(y1 - (y1-y2)*delcon + 0.5);
                     }
                     else
                     {
                        conx[l][index] = (int) floor((x2-x1)*delcon + x1 + 0.5);
                        cony[l][index] = (int) floor((y2-y1)*delcon + y1 + 0.5);
                     }
                     icon[l] = index + 1; 
                     if (index > 1) /* 2 is the maximum so dont check any more */
                        break;
                  }
               }
            }

            for (l = 0; l < ncon; l++)
            {
               if ((conx[l][0] != conx[l][1]) || (cony[l][0] != cony[l][1]))
               {
                  if (layerColors)
                     SetColor(layerColors[l]);
                  else
                     SetColor(COLOR_GREEN);

						moveTo (cony[l][0], conx[l][0]); /* mplot(cony[l][1]-0.05, conx[l][1]-0.17, 3); */
                  lineTo (cony[l][1], conx[l][1]); /* mplot(cony[l][2]-0.05, conx[l][2]-0.17, 2); */
                  pointAt(cony[l][1], conx[l][1]);
               }
            }
         }   
      }
   }

   return (TRUE);
}  

int
#if XVT_CC_PROTO
coutourImageFromIntData (XVT_PIXMAP pixmap, int **zmap,
              int jmax, int kmax, int jStart, int kStart, int flipXY,
              double *conval, int ncon, double sc, int xo, int yo,
              COLOR *layerColors)
#else
coutourImageFromIntData (pixmap, zmap, jmax, kmax, jStart, kStart, flipXY,
								 conval, ncon, sc, xo, yo, layerColors)
XVT_PIXMAP pixmap;
int **zmap;
double sc;
int xo, yo, jStart, kStart, flipXY;
double *conval;
int jmax, kmax, ncon;   
COLOR *layerColors;
#endif
{
   int tj1, tk1, tj2, tk2;
   int pnts[13][3];
   int icon[MAX_CONTOURS];
   double xscal, yscal;
   int conx[MAX_CONTOURS][2], cony[MAX_CONTOURS][2];
   register int j, k, l, inc, itri, iside, index;
   double x1, y1, z1, x2, y2, z2, delcon;
   
   setCurrentDrawingWindow (pixmap);
   penSize(0, 0);

   SetColor (COLOR_BLACK);

   xscal=sc;
   yscal=-sc;
        
   pnts[1][1]=1;
   pnts[1][2]=0;
   pnts[2][1]=0;
   pnts[2][2]=1;
   
   pnts[3][1]=0;
   pnts[3][2]=0;
   pnts[4][1]=0;
   pnts[4][2]=1;
   
   pnts[5][1]=0;
   pnts[5][2]=0;
   pnts[6][1]=1;
   pnts[6][2]=0;
   
   pnts[7][1]=1;
   pnts[7][2]=0;
   pnts[8][1]=0;
   pnts[8][2]=1;
     
   pnts[9][1]=0;
   pnts[9][2]=1;
   pnts[10][1]=1;
   pnts[10][2]=1;
     
   pnts[11][1]=1;
   pnts[11][2]=0;
   pnts[12][1]=1;
   pnts[12][2]=1;
   
   for (j = 0; j <= jmax; j++)
   {
      for (k = 0; k <= kmax; k++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (itri = 0, inc = 1; itri < 2; itri++)
         {                          /* Zero Memory */
            memset (icon, 0, sizeof(int)*ncon);
            memset (conx, 0, sizeof(int)*ncon*2);
            memset (cony, 0, sizeof(int)*ncon*2);

            for (iside = 0; iside < 3; iside++)
            {
               tj1 = j + pnts[inc][1];
               tk1 = k + pnts[inc][2];
               tj2 = j + pnts[inc+1][1];
               tk2 = k + pnts[inc+1][2];
					if (flipXY)
					{
						z1  = (double) (zmap[tj1+jStart][tk1+kStart]);
						z2  = (double) (zmap[tj2+jStart][tk2+kStart]);
					}
					else
					{
						z1  = (double) (zmap[tk1+kStart][tj1+jStart]);
						z2  = (double) (zmap[tk2+kStart][tj2+jStart]);
					}
               y1  = yo - (tj1*yscal);
               x1  = xo + (tk1*xscal);
               y2  = yo - (tj2*yscal);
               x2  = xo + (tk2*xscal);
               inc += 2;

               for (l = 0; l < ncon; l++)
               {
                  if ((z1 <  conval[l] && z2 >= conval[l]) ||
                      (z1 >= conval[l] && z2 <  conval[l]))
                  {
                     delcon = (conval[l]-z1)/(z2-z1);
                     index = icon[l];   
                     if (iside == 0) 
                     {
                        conx[l][index] = (int) floor((x2-x1)*delcon + x1 + 0.5);
                        cony[l][index] = (int) floor(y1 - (y1-y2)*delcon + 0.5);
                     }
                     else
                     {
                        conx[l][index] = (int) floor((x2-x1)*delcon + x1 + 0.5);
                        cony[l][index] = (int) floor((y2-y1)*delcon + y1 + 0.5);
                     }
                     icon[l] = index + 1; 
                     if (index > 1) /* 2 is the maximum so dont check any more */
                        break;
                  }
               }
            }

            for (l = 0; l < ncon; l++)
            {
               if ((conx[l][0] != conx[l][1]) || (cony[l][0] != cony[l][1]))
               {
                  if (layerColors)
                     SetColor(layerColors[l]);
                  else
                     SetColor(COLOR_GREEN);

                  moveTo (cony[l][0], conx[l][0]); /* mplot(cony[l][1]-0.05, conx[l][1]-0.17, 3); */
                  lineTo (cony[l][1], conx[l][1]); /* mplot(cony[l][2]-0.05, conx[l][2]-0.17, 2); */
               }
            }
         }   
      }
   }

   return (TRUE);
}  


/************************************************************************
*                          *
*                          *
*                          *
*  mcontrlabel(zmap,jmax,kmax,conval,ncon,cutoff,sc,xo,yo,flag)   *  
*                          *
*  double   **zmap   array of pointers to start of each row of data matrix*
*  int   jmax  vertical dimension of data array    *
*  int   kmax  horizontal dimension of data array     *
*  double   conval[20]  array of contour intervals to be plotted  *
*  int   ncon  number of contour intervals         *
*  double   cutoff   value above which data point will be ignored *
*  double   scale scaling factor for contour plot        *
*  double   xo x offset for contour plot        *
*  double   yo y offset for contour plot        *
*  BOOLEAN flagforce single colour & width scaling       *
*                          *
*  mcontrlabel function for labelling contoured data set    *
*                          *
*  this function assumes a coordinate space such that zmap[0][0]  *
*  is the origin, which is in the botton left corner of the area  *
*  to be contoured, and up and right are positive in both      *
*  coordinate and indexing space but for this version x & y are   *
*  swapped!!!!!!                    *
*                          *
*  the cutoff term allows you to contour data which is on a *
*  regular grid but is not available everywhere       *
*                          *
*************************************************************************/
int mcontrlabel(zmap,jmax,kmax,conval,ncon,cutoff,sc,xo,yo)
double   **zmap,sc;
double   cutoff,xo,yo;
double   conval[20];
int      jmax,kmax,ncon;   
{
   double xscal, yscal;
   register int   jj, kk, l;
   double x1, y1, z1, z2;
   int quarter;

   quarter=jmax/4;
   xscal=sc;
   yscal=-sc;

   SetColor (COLOR_BLACK);

   for(jj=0;jj<3;jj++)
   {
      for(kk=5;kk<kmax-5;kk++)
      {
    z1=zmap[quarter][kk-1];
    z2=zmap[quarter][kk];
    for(l=0;l<ncon;l++)
    {
       if(z1 < cutoff && z2 < cutoff)
       {
          if((z1 < conval[l+1] && z2 >= conval[l+1])
                     || (z1 >= conval[l+1] && z2 < conval[l+1]))
          {
             y1=yo-(kk*yscal);
        x1=xo+(quarter*xscal);
        number(x1-0.25,y1,0.15,conval[l+1],0.0,-1);
          }
       }
    }
      }
      quarter+=quarter;
   }        
   SetColor (COLOR_BLACK);
   penSize(1,1);
   return (TRUE);
}

void contour_boundaries(histoire, ro, co, xo, yo)
STORY **histoire;
int ro, co;
double xo, yo;
{
#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   int cont_bndy[15]={
#else
   static int cont_bndy[15]={
#endif
            1111, 1211, 1121, 1112, 1222,
            1221, 1122, 1212, 1231, 1123,
            1223, 1233, 1213, 1232, 1234};
                  
#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   int cont_bndy_pts[15][4][2]={
#else
   static int cont_bndy_pts[15][4][2]={
#endif
            0, 0, 0, 0, 0, 0, 0, 0,
            1, 2, 0, 0, 0, 0, 0, 0,
            2, 3, 0, 0, 0, 0, 0, 0,
            3, 4, 0, 0, 0, 0, 0, 0,
            1, 4, 0, 0, 0, 0, 0, 0,
            1, 3, 0, 0, 0, 0, 0, 0,
            2, 4, 0, 0, 0, 0, 0, 0,
            1, 4, 2, 3, 0, 0, 0, 0,
            1, 5, 2, 5, 3, 5, 0, 0,
            2, 5, 3, 5, 4, 5, 0, 0,
            1, 5, 3, 5, 4, 5, 0, 0,
            1, 5, 2, 5, 4, 5, 0, 0,
            1, 2, 3, 4, 0, 0, 0, 0,
            1, 4, 2, 3, 0, 0, 0, 0,
            1, 5, 2, 5, 3, 5, 4, 5 };

#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   double pt_codes[6][2]={
#else
   static double pt_codes[6][2]={
#endif
            0.0, 0.0,
            0.5, 0.0,
            1.0, 0.5,
            0.5, 1.0,
            0.0, 0.5,
            0.5, 0.5 };
                  
   register int r, c, code, pt_seq, pt_index;
   unsigned char h1[ARRAY_LENGTH_OF_STRAT_CODE],
                 h2[ARRAY_LENGTH_OF_STRAT_CODE],
                 h3[ARRAY_LENGTH_OF_STRAT_CODE],
                 h4[ARRAY_LENGTH_OF_STRAT_CODE];
   double x1,y1,tx,ty;
   int clrs[5];
   OBJECT *object;
   NODDY_COLOUR *colorStruct;
   COLOR color;
      
   for(r=1;r<ro-1;r++)
   {
      for(c=1;c<co-1;c++)
      {
         code = 1000;
         iequal((unsigned char *) &h1[0],(unsigned char *) &(histoire[r][c].sequence[0]));
         iequal((unsigned char *) &h2[0],(unsigned char *) &(histoire[r][c+1].sequence[0]));
         iequal((unsigned char *) &h3[0],(unsigned char *) &(histoire[r+1][c+1].sequence[0]));
         iequal((unsigned char *) &h4[0],(unsigned char *) &(histoire[r+1][c].sequence[0]));
       
         clrs[1]=(int) lastdiff((unsigned char *) &h1[0],(unsigned char *) &h2[0]);      
         clrs[2]=(int) lastdiff((unsigned char *) &h2[0],(unsigned char *) &h3[0]);      
         clrs[3]=(int) lastdiff((unsigned char *) &h3[0],(unsigned char *) &h4[0]);      
         clrs[4]=(int) lastdiff((unsigned char *) &h1[0],(unsigned char *) &h4[0]);
            
         if (lineCoincide(h1,h2))
            code += 100;
         else
            code += 200;
            
         if (lineCoincide(h1,h3))
            code += 10;
         else if(lineCoincide(h2,h3) || (code==1100 && !lineCoincide(h2,h3)))
            code += 20;
         else
            code += 30;
         
         if (lineCoincide(h1,h4))
            code+=1;
         else if(lineCoincide(h2,h4) || (code==1120 && lineCoincide(h3,h4))|| code==1110)
            code+=2;
         else if (lineCoincide(h3,h4) || ((code==1120 || code==1220 || code==1210) && !lineCoincide(h3,h4)))
            code+=3;
         else
            code+=4;
            
         for(pt_index=0;pt_index<15;pt_index++)
            if(cont_bndy[pt_index]==code) break;
            
         for (pt_seq = 0; pt_seq < 4; pt_seq++)
         {
            if (cont_bndy_pts[pt_index][pt_seq][0]!=0)
            {
               tx = c+pt_codes[cont_bndy_pts[pt_index][pt_seq][0]][0];
               ty = r+pt_codes[cont_bndy_pts[pt_index][pt_seq][0]][1];
               x1 = xo + (tx*0.2);    /* 1.3 */
               y1 = yo - (ty*(-0.2)); /* 1.0 */
                                                 
               /* color according to the last event that divides
               **   break_code = lastdiff(start1, start2); */
               object = (OBJECT *) nthObject(NULL_WIN, clrs[cont_bndy_pts[pt_index][pt_seq][0]]);
               if (object)
               {
                  if (object->shape == UNCONFORMITY)
                  {
                     object->generalData = 1;
                  }
                  colorStruct = getNoddyColourStructure (object);
                  color = XVT_MAKE_COLOR (colorStruct->red, colorStruct->green, colorStruct->blue);
                  SetColor (color);
               }
               mplot(y1, x1, 3);

               tx = c + pt_codes[cont_bndy_pts[pt_index][pt_seq][1]][0];
               ty = r + pt_codes[cont_bndy_pts[pt_index][pt_seq][1]][1];
               x1 = xo + (tx*0.2);    /* 1.3 */
               y1 = yo - (ty*(-0.2)); /* 1.0 */
               
               mplot(y1, x1, 2);
            } 
         }
      }
   }
}


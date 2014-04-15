#include "xvt.h"
#include "noddy.h"
#include "scales.h"
#include <math.h>


#define DEBUG(X)

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern double iscale; /* scaling factor for geology */

                 /* ************************* */
                 /* Globals used in this file */


/************************************************************************
*                                            *
*    void ldotmp(maptype,cypher,cyphno,nx,ny)               *
*                                            *
*    int maptype;        type of map    (99=ermapper,98=3D) *
*    double cypher[];    array of previous sub area codes   *
*    int *cyphno;        number of sub areas           *
*    int nx,ny           size of array to contour *
*                                            *
*    ldotmp function calculates structure of top surface         *
*          and number of sub areas                *
*                                            *
*    ldotmp() takes 5 arguments                        *
*    ldotmp() returns no value                         *
*                                            *
************************************************************************/
int ldotmp(dots, histoire, maptype, cypher, cyphno, nx, ny)
double ***dots;
struct story **histoire;
unsigned char cypher[200][ARRAY_LENGTH_OF_STRAT_CODE];
int *cyphno,maptype;
int nx,ny;
{
   register int m, n, lno;
   double dx, dy, dz, length, declination = 90.0;
   BOOLEAN some;
   int bar=0, j, k, num;
   int numEvents = (int) countObjects(NULL_WIN);
   double SectionDec = geologyOptions.welllogDeclination;
   double gblock, msize;
   double gx1, gy1, gz1, gx2, gy2, gz2;
   double totalScale;
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;
   gblock = viewOptions->geophysicsCubeSize;
   msize = floor(geophysicsOptions.calculationRange/gblock);

   gx1 = viewOptions->originX;
   gy1 = viewOptions->originY;
   gz1 = viewOptions->originZ - viewOptions->lengthZ;
   gx2 = viewOptions->originX + viewOptions->lengthX;
   gy2 = viewOptions->originY + viewOptions->lengthY;
   gz2 = viewOptions->originZ;

   totalScale = iscale;
                           /*unsigned int xtemp;*/
   j=ny;
   k=nx;

   if (maptype == SECTION)
   {
      length=1000*totalScale;
      dx=length*sin(SectionDec*3.14159/180.0)/71.0;
      dy=length*cos(SectionDec*3.14159/180.0)/71.0;
      dz=500.0*totalScale/36.0;

      for (m = 1; m <= nx; m++)
      {                                                             
         incrementLongJob (INCREMENT_JOB);
         for (n=1;n<=ny;n++)
         {                                                          
            dots[n][m][1] = ((n-1)*dx) + absx;                       
            dots[n][m][2] = ((n-1)*dy) + absy;                      
            dots[n][m][3] = ((m-1)*dz) + absz;                      
            histoire[n][m].again=1;

            izero(histoire[n][m].sequence);
         }
      }
   }
   else if (maptype == 99)  /* ERMapper */
   {                                              
      for (m = 1; m <= nx; m++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (n = 1; n <= ny; n++)
         {                                                            
            dots[n][m][1] = gx1 + ((m-1)*gblock);
            dots[n][m][2] = gy2 - ((n-1)*gblock);
            dots[n][m][3] = gz2;

            histoire[n][m].again=1;
            izero(histoire[n][m].sequence);
         }
      }
      j=ny;
      k=nx;
   }
/*   else if (maptype == LINE)   .* ADDED BY PAUL *.
   {                                              
      for (m=1;m<=nx;m++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (n=1;n<=ny;n++)
         {
         dots[n][m][1]=((n-1)*14.285714*iscale)+absx;
         dots[n][m][2]=(((m-1))*14.0*iscale)+absy;
         dots[n][m][3]=500*iscale+absz;

            histoire[n][m].again=1;
            izero(histoire[n][m].sequence);
         }
      }
   } */
   else           /* linemap etc */
   {                                              
      for (m = 1; m <= nx; m++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (n = 1; n <= ny; n++)
         {
            dots[n][m][1] = ((n-1)*14.285714*totalScale)+absx;
            dots[n][m][2] = (((m-1))*14.0*totalScale)+absy;
            dots[n][m][3] = 500*totalScale+absz;

            histoire[n][m].again=1;
            izero(histoire[n][m].sequence);
         }
      }
   }

   num = numEvents - 1;
   if(num > 0)
   {                        /* ************************** */
                            /* Perform the inverse events */
      reverseEvents (dots, histoire, j, k);
   }                                                    

   iequal(cypher[1],histoire[1][1].sequence);

   for (m = 1; m <= nx; m++) 
   {                                                             
      for (n = 1; n <= ny; n++) 
      {                                                          
         some = FALSE;
         for (lno = 1; lno <= *cyphno; lno++) 
         {                                                  
/*          Wasn't drawing inside of ellip faults 
            if (lineCoincide(histoire[n][m].sequence, &(cypher[lno][0])))
*/
            if (coincide(histoire[n][m].sequence, &(cypher[lno][0])))
               some = TRUE;
         }
         if (!some)
         {
            *cyphno = *cyphno+1;

            iequal(&(cypher[*cyphno][0]), histoire[n][m].sequence);
         }
      }
   }

   return (TRUE);
}

/************************************************************************
*                                            *
*    void onedotmp(xcol,yrow)                     *
*                                            *
*    int xcol; row of array to calculate               *
*    int yrow; col of array to calculate               *
*                                            *
*                                            *
*    onedotmp function calculates structure of one point xcol,yrow    *
*                                            *
*    onedotmp() takes 2 arguments                      *
*    onedotmp() returns no value                       *
*                                            *
************************************************************************/
int onedotmp(dots, histoire, xcol, yrow)
double ***dots;
struct story **histoire;
int xcol,  yrow;
{
   int j, k;
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;
                                                   
   j=1;
   k=1;
   
   dots[1][1][1] = ((xcol-1)*14.285714*iscale)+absx;
   dots[1][1][2] = (((yrow-1))*14.0*iscale)+absy;
   dots[1][1][3] = 500*iscale*absz;

   ScaleOneTopo(dots,xcol,yrow);
   histoire[1][1].again=1;

   izero(histoire[1][1].sequence);

                            /* ************************** */
                            /* Perform the inverse events */
   reverseEvents (dots, histoire, j, k);
   return (TRUE);
}

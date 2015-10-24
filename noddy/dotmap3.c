#include "xvt.h"
#include "noddy.h"

#define DEBUG(X)    

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern double iscale; /* scaling factor for geology */

#if XVT_CC_PROTO
extern int block(double ***, struct story **, double, int, int, int,
                                       double, double, double, int);
#else
extern int block();
#endif
                 /* ************************* */
                 /* Globals used in this file */

/************************************************************************
*                                            *
*    void dotmap()                                *
*                                            *
*                                            *
*    dotmap function generates coordinate arrays for block            *
*   diagrams and calls inverse function routines                      *
*   in reverse chronological order.                                   *       
*                                            *
*    dotmap() takes no arguments                       *
*    dotmap() returns no arguments                     *
*                                            *
************************************************************************/
void dotmap(dots, histoire, BSize, xchair, ychair, zchair, ichair)
double ***dots;
struct story **histoire;
double BSize;
double xchair,ychair,zchair;
int ichair;
{
/*
c
c   dotmap generates coordinate arrays for block
c   diagrams and calls inverse function routines
c   in reverse chronological lh[15][3],order.
*/
   register int m,n;
   double xx,yy,zz;
   int iface;
   int j, k;  /* position indexs */
   int stopFound = FALSE;
   int mmax, nmax;
   double totalScale;
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;

   totalScale = BSize * iscale;
   
   DEBUG(printf("\nDOTMAP - begining");)

   iface=4;
   do
   {
      if (abortLongJob ())
         return;
         
      iface=iface-1;  

      if (iface == 1)         /* Top X-Y Face */
      {                                                       
         if (ichair)
         {
            /* mmax=(xchair*iscale/10)/(40*BSize*iscale); */
            mmax = (int) ((xchair-absx)/(40*totalScale));
            nmax = (int) ((ychair-absy)/(41.17647*totalScale));
            zz = zchair;
         }
         else
         {
             mmax = (int) (25/BSize);
             nmax = (int) (17/BSize);
             zz = BLOCK_DIAGRAM_SIZE_Z+absz;
         }

         j=mmax;
         k=nmax;

         for (m = 1; m <= mmax; m++)
         {                                                            
            incrementLongJob (INCREMENT_JOB);
            for (n = 1; n <= nmax; n++)
            {                                                            
               dots[m][n][1] = ((m-1)*40.0*totalScale) + absx;
               dots[m][n][2] = ((n-1)*41.17647*totalScale) + absy;
               /* dots[m][n][3]=(500.0*iscale)+absz;  */                                     
               dots[m][n][3] = zz;

               histoire[m][n].again = 1;

               izero(histoire[m][n].sequence);
            }
         }      
      }
      else if(iface == 2)
      {                
         if (ichair)
         {    
            mmax = (int) ((xchair-absx)/(40*totalScale)); 
            nmax = (int) ((absz+500*iscale-zchair)/(45.45454545*totalScale)); 
            yy = ychair;
            zz = zchair;
         } 
         else
         {
            mmax = (int) (25/BSize);
            nmax = (int) (11.3/BSize);
            yy = absy;
            zz = absz;
         }

         j = mmax;
         k = nmax;                         

         for (m = 1; m <= mmax; m++)
         {                                                            
            incrementLongJob (INCREMENT_JOB);
            for(n = 1; n <= nmax; n++)
            {
               dots[m][n][1] = ((m-1)*40.0*totalScale)+absx;
               dots[m][n][2] = yy;
               dots[m][n][3] = ((n-1)*45.45454545*totalScale)+zz;
               histoire[m][n].again = 1;

               izero(histoire[m][n].sequence);
            }
         }      
      }
      else if (iface == 3)
      {        
         if(ichair) 
         {     
            mmax = (int) ((ychair-absy)/(41.17647*totalScale));  
            nmax = (int) ((absz +500*iscale-zchair)/(45.45454545*totalScale));
            xx = xchair;
            zz = zchair;
         }  
         else
         { 
            mmax = (int) (18/BSize);
            nmax = (int) (11.3/BSize);
            xx = absx;
            zz = absz;
         } 

         j=mmax; 
         k=nmax;

         for (m = 1; m <= mmax; m++)
         {
            incrementLongJob (INCREMENT_JOB);
            for (n = 1; n <= nmax; n++)
            {
               dots[m][n][1]=xx;
               dots[m][n][2]=((m-1)*41.17647*totalScale)+absy;
               dots[m][n][3]=((n-1)*45.45454545*totalScale)+zz;
               histoire[m][n].again=1; 

               izero(histoire[m][n].sequence);
            }
         }      
      }       

      DEBUG(printf("\nDOTMAP: Performing Inverse Events");)

      if (abortLongJob ())
         return;
                            /* ************************** */
                            /* Perform the inverse events */
      reverseEvents (dots, histoire, j, k);

      if (abortLongJob ())
         return;
                            /* ********************** */
                            /* draw the block diagram */
      block (dots, histoire, BSize, iface, mmax, nmax,
             xchair, ychair, zchair, ichair);

   } while (iface > 1);      

   DEBUG(printf("\nDOTMAP - ending");)
}

#include "xvt.h"
#include "noddy.h"

#define DEBUG(X)    
#define DEBUG1(X)   

                 /* ********************************** */
                 /* External Globals used in this file */
extern BLOCK_VIEW_OPTIONS *blockViewOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern double iscale; /* scaling factor for geology */

#if XVT_CC_PROTO
extern void cliprect (double, double, double, double, BOOLEAN);
extern void plotpoint(double, double);
#else
extern void cliprect ();
extern void plotpoint();
#endif
                 /* ************************* */
                 /* Globals used in this file */

/************************************************************************
*                                            *
*    void block()                                 *
*                                            *
*                                            *
*    block function displays the block diagram of the present    *
*    geology in dotmap form for three faces of block        *
*                                            *
*    block() takes no arguments                        *
*    block() returns no value                     *
*                                            *
************************************************************************/
int block(dots, histoire, BSize, iface, mmax, nmax, xchair, ychair, zchair, ichair)
double ***dots;
struct story **histoire;
double BSize;
int iface; /* which face is being drawn */
int mmax, nmax;
double xchair,ychair,zchair;
int ichair;
{                                                         
/*
c   block displays the block diagram of the present                             
c   geology in dotmap form.                                                     
*/
   WINDOW currentDrawingWindow = getCurrentDrawingWindow ();
   OBJECT *object = NULL;
   NODDY_COLOUR *colorOptions = NULL;
   double a,b,c,d,e,f,xcord,ycord;
   double clipX1, clipX2, clipY1, clipY2;
   int m, n, index;                       
   unsigned int flavor;
   int j, k, numEvents = (int) countObjects(NULL_WIN);
   COLOR color;
   double absx; /* = geologyOptions.origin_X; */
   double absy; /* = geologyOptions.origin_Y; */
   double absz; /* = geologyOptions.origin_Z; */
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;
   DEBUG(printf("\nBLOCK: iface = %d",iface);)

              /* set a,b,c,d variables to define plane that face is on */
   if (iface == 1)        /* TOP X-Y Face */
   {                                                       
      a=0.4724409;
      b=-0.2779064;
      c=0.0;
      d=0.2779064;

      if (!ichair)
      {
         f=5.906;
         e=4.725;
      }
      else
      {
         e=4.725;
         f=5.906*(zchair-absz)/(500.0*iscale);
      }
      m=mmax;
      n=nmax;
   }
   else if (iface == 2)     /* FROUNT X-Z Face */
   {                                                   
      a=0.4724409;
      b=0.0;
      c=0.0;
      d=0.5368647;
      m=mmax;
      n=nmax;

      if (!ichair)
      {
         e=4.725;
         f=0.0;
      }
      else
      {
         e=-0.2779064*BSize*((ychair-absy)/(41.17647*iscale*BSize)-1) + 4.725;
         f= 0.2779064*BSize*((ychair-absy)/(41.17647*iscale*BSize)-1) +0.07+
            0.536847*BSize*((zchair-absz)/(45.45454545*iscale*BSize)-1);
      }
   }
   else                      /* SIDE Y-Z Face */
   {                                                                      
      a=-0.2779064;
      b=0.0;
      c=0.2779064;
      d=0.5368647;
      m=mmax;
      n=nmax;


      if (!ichair)
      {
         e=4.725;                                  
         f=0.0;
      }
      else
      {
         e= 0.4725*BSize*((xchair-absx)/(40*iscale*BSize) -1) + 4.725 ;
         f= 0.5906*BSize*((zchair-absz)/(45.45454545*iscale*BSize) -1)+0.07;
      }
   }

          /* set up the cliping rectangle for the face so the dot size
          ** does not show past the border */
/*  didn't draw anything with this in 
*/
   clipX1 = 0.0; clipX2 = 16.5;
   clipY1 = 0.0; clipY2 = 10.63;
   cliprect (clipX1, clipY1, clipX2, clipY2, TRUE);
   DEBUG(printf("\nBLOCK: calculating %d x %d points", mmax, nmax);)

   for (j = 1; j <= mmax; j++)
   {
      if (incrementLongJob (INCREMENT_JOB) == ABORT_JOB)
         return (FALSE);
         
      for (k = 1; k <= nmax; k++)
      {
         xcord=(a*BSize*(j-1))+(b*BSize*(k-1))+e;
         ycord=(c*BSize*(j-1))+(d*BSize*(k-1))+f;


         taste(numEvents, histoire[j][k].sequence, &flavor, &index);

                          /* all one color rock (eg plug, dyke etc) */
         if (flavor == IGNEOUS_STRAT)
         {
            if (!(object = (OBJECT *) nthObject (NULL_WIN, index)))
            {
               object = (OBJECT *) nthObject (NULL_WIN, 0);
            }
            if (object && (colorOptions = (NODDY_COLOUR *)
                                          getNoddyColourStructure (object)))
            {
               color = XVT_MAKE_COLOR (colorOptions->red, colorOptions->green,
                                   colorOptions->blue);
            }
            else
            {
               color = COLOR_BLACK;
            }  
            
            SetColor (color);

            plotpoint (xcord, ycord);
         }
         else
         {
            which(xcord,ycord,index,dots[j][k][1],dots[j][k][2],dots[j][k][3],flavor);
         }
      }
   }
   cliprect (clipX1, clipY1, clipX2, clipY2, FALSE);
   DEBUG(printf("\nBLOCK: Finished");)
   
   return (TRUE);
}

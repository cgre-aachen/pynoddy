#include <math.h>
#include "xvt.h"
#include "noddy.h"
#include "scales.h"

#define DEBUG(X)    

                 /* ********************************** */
                 /* External Globals used in this file */

                 /* ************************* */
                 /* Globals used in this file */


/************************************************************************
*                                            *
*    void lblock()                                *
*                                            *
*                                            *
*    block function displays the Linemap of the present          *
*    geology                                 *
*                                            *
*    block() takes no arguments                        *
*    block() returns no value                     *
*                                            *
************************************************************************/
int lblock(dots, histoire, BSize, mmax, nmax, maptype)
double ***dots;
struct story **histoire;
double BSize;
int mmax, nmax;
int maptype; 
{                                                         
/*
c   lblock displays the Linemap of the present
c   geology in dotmap form.
*/
   WINDOW currentDrawingWindow = getCurrentDrawingWindow ();
   OBJECT *object = NULL;
   NODDY_COLOUR *colorOptions = NULL;
   COLOR color;
   double a, d, e, f, xcord, ycord;
   int m, n, index, j, k;
   unsigned int flavor;
   int numEvents = (int) countObjects(NULL_WIN);
      
   DEBUG(printf("\nLBLOCK: Bsize = %f",BSize);)

   a=14/(25-BSize);
   d=10/(17-BSize);                                                          
   f=1.4;
   e=1.1;

   m=mmax;
   n=nmax;

   for(j=1;j<=mmax;j++)
   {
      incrementLongJob (INCREMENT_JOB);
      for(k=1;k<=nmax;k++)
      {
         xcord=a*BSize*(j-1)+e;
         ycord=d*BSize*(k-1)+f;

         taste(numEvents, histoire[j][k].sequence, &flavor, &index);

         if(flavor == IGNEOUS_STRAT)
         {  
                               /*define colour for dyke & plug*/
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
            lwhich(xcord, ycord, index, dots[j][k][1], dots[j][k][2],
                                        dots[j][k][3], flavor, maptype);
      }
   }
   return (TRUE);
}

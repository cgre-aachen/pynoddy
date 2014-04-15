/* ======================================================================

                      ** PLEASE READ AND NOTE **

     This file is used for both the sequential and parallel versions
     of Noddy, MAKE SURE any changes that are made are compatible
     with both version of Noddy.

     You may use the _MPL defined symbol to conditionally compile
     code that is only associated with a particular version of Noddy.

     _MPL is defined only for the MASPAR Parallel verion of Noddy.

                      THANK-YOU FOR YOUR ATTENTION

====================================================================== */
#ifndef _MPL
#include "xvt.h"
#endif
#include "noddy.h"

static int ind;
static BOOLEAN BitsSplat;

/************************************************************************
*                          *
*  void f01aaf(mata,matb)                 *
*                          *
*  double mata[4][4];         matrix to invert  *
*  double matb[4][4];         inverted matrix      *
*  f01aaf function performs matrix inversion(orig from NAG library)*
*                          *
*  f01aaf() takes 2 arguments             *
*  f01aaf() returns no value              *
*                          *
************************************************************************/

int f01aaf(mata,matb)
double mata[4][4],matb[4][4];
{
   float **a,**b,d,y[4];
   int i,j, *indx;
   
   a=(float **)matrix(0,3,0,3); /*initialize new arrays */
   b=(float **)matrix(0,3,0,3);
   indx=(int *)ivector(0,3);
   
   for(i=1;i<=3;i++) /* copy array to float pointer form */
   {
      for(j=1;j<=3;j++)
      {
         a[i][j]=(float) mata[i][j];
      }
   }

   for(i=1;i<=3;i++) /* make identity array */
   {
      for(j=1;j<=3;j++)
      {
         b[i][j] = (float) 0.0;
      }
      b[i][i] = (float) 1.0;
   }

   if (!ludcmp(a, (int) 3,indx,&d)) /* decompose matrix once */
      return (FALSE);
   
   for(i=1;i<=3;i++) /* find inverse by columns */
   {
      for(j=1;j<=3;j++)
      {
         y[j]=b[i][j];
      }
      lubksb(a,3,indx,y);
      for(j=1;j<=3;j++)
      {
         b[i][j]=y[j];
      }
   }
   
   
   for(i=1;i<=3;i++) /* copy back to double array form and transpose*/
   {
      for(j=1;j<=3;j++)
      {
         matb[i][j]=(double) b[j][i];
      }
   }
   
   free_matrix(a,0,3,0,3); /* free memory */
   free_matrix(b,0,3,0,3);
   free_ivector(indx,0,3);
   return(1);
}




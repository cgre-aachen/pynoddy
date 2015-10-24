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
#include <math.h>
#ifndef _MPL
#include "xvt.h"
#endif
#include "noddy.h"

#define DEBUG(X)  

void NewtonRaphson ();

void NewtonRaphson(x, Xaxis, F1, xprime)
double x, F1, *xprime;
double Xaxis;
{
   register double ng, g, error, xratio;
   register int i=0;
   
   g = x/Xaxis;
   xratio = x/Xaxis;

   do
   {
      ng = g-((xratio - g - F1 + (2*F1*g*g) - (F1*g*g*g*g))  /
              (-1 + (4*F1*x/(Xaxis*Xaxis)) - (4*F1*x*x*x/(Xaxis*Xaxis*Xaxis*Xaxis)))); /* mwj_fix */
/*              (-1 + (4*F1*g) - (4*F1*g*g*g))); */
      error = ng-g;

      g = ng;
      i++;
   } while (fabs(error) > 1e-5 && i <100);
   
   *xprime = g*Xaxis;
}
                  

/* Functions for use with UnFault */
void ProfilePosition(x, z, prof_rot, prof_dim, pos_vect)
double x, z; 
double prof_rot, prof_dim, pos_vect[2];
{
   double rot_mat[2][2];
   
   rot_mat[0][0] = sin(prof_rot*3.1415927/180.0);
   rot_mat[0][1] = -cos(prof_rot*3.1415927/180.0);
   rot_mat[1][0] = -rot_mat[0][1];
   rot_mat[1][1] = rot_mat[0][0];
   
   pos_vect[0] = (rot_mat[0][0]*x) + (rot_mat[0][1]*z);
   pos_vect[1] = (rot_mat[1][0]*x) + (rot_mat[1][1]*z);
   pos_vect[0] = pos_vect[0] + (prof_dim);

}

/* Functions for use with UnFault */
double OneYdisplace(pos_vect, profileArray,
                    prof_dim, prof_amp, cyl_index)
double pos_vect[2];
float *profileArray;
double prof_dim, prof_amp, cyl_index;
{
   double Yd, scale_xpos;
/*
   double PHASE_SHIFT=0.0, SCALE_IT=0.5;
   
   scale_xpos= (pos_vect[0]*3.1415927/(prof_dim*SCALE_IT))-PHASE_SHIFT;
*/
   scale_xpos= pos_vect[0]*(3.1415927/prof_dim)/* *2.0 */;
   
   if(pos_vect[0] >= 0.0 && pos_vect[0] < prof_dim*2.0)
      Yd=(calcProfile(profileArray, scale_xpos)*
             prof_amp*exp(-(pos_vect[1]*pos_vect[1])*cyl_index));
    else
      Yd=0.0;
       
    return (Yd);
}

/* Functions for use with UnFault */
double CalcYDisplace(YPos, newXpos, oldXpos, ZPos, xax, yax, zax,
                     sign, ProfRot, ProfDim, profileArray, ProfAmp,
                     CylIndex)
double   YPos, newXpos, oldXpos, ZPos, xax, yax, zax;
double sign, ProfRot, ProfDim;
float *profileArray;
double ProfAmp, CylIndex;
{
   double oldposvec[2], newposvec[2];
   double oldY, newY;
   double Ydis;
   
   ProfilePosition(oldXpos, ZPos, ProfRot, ProfDim, oldposvec);
   ProfilePosition(newXpos, ZPos, ProfRot, ProfDim, newposvec);
   oldY = OneYdisplace(oldposvec, profileArray, ProfDim, ProfAmp, CylIndex);
   newY = OneYdisplace(newposvec, profileArray, ProfDim, ProfAmp, CylIndex);
   
   Ydis = YPos - (newY-oldY);   /* pow((yax-newY)/yax,2.0); */
   return(Ydis);
}

void mat_copy(mata, matb, dimens)
double mata[10][4], matb[10][4];
int dimens;
{
   int ii, jj;
   
   for (ii = 1; ii < dimens+1; ii++)
      for (jj = 1; jj < 4; jj++)
         mata[ii][jj] = matb[ii][jj];
}


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
#include <math.h>

#define DEBUG(X)  
#define TOLERANCE 0.01

                 /* ********************************** */
                 /* External Globals used in this file */

                 /* ************************* */
                 /* Globals used in this file */
static double pole[2][4];  /* temporary line direction cosines */
static double axis[2][4];  /* temporary line direction cosines */
static double conrot[4][4];   /* temporary forward rotation matrix */
static double revcon[4][4];   /* temporary reverse rotation matrix */

void exchng(dots,m,n,point,lfor)
#ifdef _MPL
plural double ***dots;
plural int m,n;
plural double point[2][4];
#else
double ***dots;
int m,n;
double point[2][4];
#endif
BOOLEAN lfor;
{                                         
/*
c                                                                               
c   exchng creates a local array containing the                                 
c   coordinates of the given point.                                             
c                                                                               
*/
   register int l;
      
   if (lfor)
   {                                                             
      for (l=1;l<=3;l++)
         point[1][l]=dots[m][n][l];
   }
   else
   {                                                                      
      for (l=1;l<=3;l++)
         dots[m][n][l]=point[1][l];
   }
}

void convrt(dip,dipdir,pitch,lpitch)
double dip,dipdir,pitch;
BOOLEAN lpitch;
{
/*                                
c 
c   convert converts (dip/dip direction) data to the 
c   direction cosines of the pole to the plane, and
c   (pitch) data to the direction cosines of the 
c   coplanar lineation.
c 
*/
   double matc[4][4],matd[2][4],rad,halfpi;

   rad=atan(1.0)/45.0;
   halfpi=rad*90.0;
   dip=dip*rad;
   if (lpitch)
      pitch=pitch*rad;
   dipdir=dipdir*rad;
   pole[1][1]=sin(dipdir)*sin(dip);
   pole[1][2]=cos(dipdir)*sin(dip);
   pole[1][3]=cos(dip);
   if(lpitch)
   {                                                           
       matd[1][1]=sin(dipdir-halfpi);
       matd[1][2]=cos(dipdir-halfpi);
       matd[1][3]=0.0;
       irotate(matc,pole[1][1],pole[1][2],pole[1][3],-pitch);
       matmlt(matc,matd,axis,1);
   }                                                                     
}                                                                      


double fourtoo(data,ex)
float data[];
double ex;
{
/*
c
c   fourtoo calculates the fourier series function
c   from array data at point 'ex' returning                              
c   the value as foursm.
c
*/

   register int k;
   double fivesm,sixsm;
                                                      
   sixsm=data[1]/2.0;
   fivesm=0.0;

   for(k=1;k<11;k++) 
   {                                                              
      sixsm+=(double)(data[(k*2)+1]*cos((double)k*ex));
      fivesm+=(double)(data[(k*2)+2]*sin((double)k*ex));
   }                                                                  
   sixsm=sixsm+fivesm;
   return (sixsm);                                                      
}

#ifdef _MPL
plural double fourier (data, ex)
double data[2][11];
plural double ex;
#else
double fourier (data, ex)
double data[2][11];
double ex;
#endif
{
/*                                                  
c
c   calculates the fourier series function
c   from array data at point 'ex' returning
c   the value as foursm. (the index of data array starts from 0)
c
*/
   register int k;
#ifdef _MPL
   plural double fivesm, sixsm;
#else
   double fivesm, sixsm;
#endif
                                                      
   sixsm = data[0][0]/2.0;
   fivesm= 0.0;

   for (k = 1; k < 11; k++)
   {
#ifdef _MPL
      sixsm += (plural double) (data[0][k] * p_cos((double) k * ex));
      fivesm+= (plural double) (data[1][k] * p_sin((double) k * ex));
#else
      sixsm += (double) (data[0][k] * cos((double) k * ex));
      fivesm+= (double) (data[1][k] * sin((double) k * ex));
#endif
   }
   sixsm=sixsm+fivesm;

   return (sixsm);
}                                                                     

double fourierHalf (data, ex)
double data[2][11];
double ex;
{
/*                                                  
   calculates the fourier series function using only the first half
   of the function
   from array data at point 'ex' returning
   the value as foursm. (the index of data array starts from 0)
*/
   register int k;
   double fivesm, sixsm;
                                                      
   sixsm = data[0][0]/2.0;
   fivesm= 0.0;

   for (k = 1; k < 6; k++)
   {
      sixsm += (double) (data[0][k] * cos((double) k * ex));
      fivesm+= (double) (data[1][k] * sin((double) k * ex));
   }
   sixsm=sixsm+fivesm;

   return (sixsm);
}                                                                     



void local4x4From3x3(input, output)
double input[3][3];
double output[4][4];
{
/*                                                
c                                                                               
c   creates a local 4x4 matrix with the first row, column empty from a 3x3
c   matrix
c                                                                               
*/
   register int j,k;
      
   for (j = 0; j < 3; j++)
   {
      for (k = 0; k < 3; k++)
      {
         output[j+1][k+1] = input[j][k];
      }
   }
}

#ifdef _MPL
void p_matmlt(matp, matq, matr, nby3)
double matp[4][4];
plural double matq[10][4], matr[10][4];
int nby3;
{
/*

    matmlt multiplies a three by three matrix 'matp' by an
    'nby3' by three matrix 'matq' with the product
    returned as 'matr'.

*/
   register int l, m, j;

   for (l = 1; l <= nby3; l++) 
   {
      for (m = 1; m <= 3; m++) 
      {
         matr[l][m] = 0.0;
         for(j = 1; j <= 3; j++) 
         {
            matr[l][m] = matr[l][m] + (matq[l][j] * matp[j][m]);
         }
      }
   }
}
#endif


void matmlt(matp, matq, matr, nby3)
double matp[4][4];
double matq[10][4], matr[10][4];
int nby3;
{
/*

    matmlt multiplies a three by three matrix 'matp' by an
    'nby3' by three matrix 'matq' with the product
    returned as 'matr'.

*/
   register int l, m, j;

   for (l = 1; l <= nby3; l++) 
   {
      for (m = 1; m <= 3; m++) 
      {
         matr[l][m] = 0.0;
         for(j = 1; j <= 3; j++) 
         {
            matr[l][m] = matr[l][m] + (matq[l][j] * matp[j][m]);
         }
      }
   }
}

BOOLEAN more(digit)
double digit;
{
/*                                                      
c
c   more checks to see if a given point is within
c   an igneous body or above an unconformity.
c
*/
   BOOLEAN test;
      
   if (digit >= 0.0)
   {
      test=TRUE;                                                             
   }
   else
   {
      test=FALSE;                                                            
   }                                                                     
   return (test);
}                                                                      


void irotate(mata,x,y,z,angle)
double mata[4][4],x,y,z,angle;
{
/*                                       
c                                                                               
c   rotate generates a rotation matrix 'mata' which rotates                     
c   points around an axis which passes through the origin                       
c   and a point '(x,y,z)', through an angle of 'angle'                          
c   radians.                                                                    
c                                                                               
*/
      double absval,x1,y1,z1,cosang,sinang;
      
      absval=sqrt((x*x)+(y*y)+(z*z));                                        
      x1=x/absval;                                                               
      y1=y/absval;                                                               
      z1=z/absval;                                                               
      cosang=cos(angle);                                                         
      sinang=sin(angle);                                                         
      mata[1][1]=(x1*x1)+((1-(x1*x1))*cosang);                                    
      mata[1][2]=(x1*y1*(1-cosang))+(z1*sinang);                                  
      mata[1][3]=(x1*z1*(1-cosang))-(y1*sinang);                                  
      mata[2][1]=(x1*y1*(1-cosang))-(z1*sinang);                                  
      mata[2][2]=(y1*y1)+((1-(y1*y1))*cosang);                                    
      mata[2][3]=(y1*z1*(1-cosang))+(x1*sinang);                                  
      mata[3][1]=(x1*z1*(1-cosang))+(y1*sinang);                                  
      mata[3][2]=(y1*z1*(1-cosang))-(x1*sinang);                                  
      mata[3][3]=(z1*z1)+((1-(z1*z1))*cosang);                                    
}                                                                    


void rotset(dip, dipdir, pitch, forward, reverse)
double dip,dipdir,pitch;
double forward[3][3];
double reverse[3][3];
{
/*                                       
c                                                                               
c   rotset generates the forwrd and revers concatenated                         
c   rotation matrices, 'conrot' and 'revcon', that are                          
c   used to produce deformations with the desired attitude.                     
c                                                                               
*/
   double rot1[4][4], rot2[4][4], rot3[4][4], rot4[4][4],rot5[4][4];
   double rot6[4][4],newfa2[2][4],newpo2[2][4],temprt[4][4];
   double newpo[2][4], newfa[2][4],rad,rad_deg,temp1,temp2,temp3;
   int i, j;
 
   rad=3.1415926535/2.0;
   rad_deg=atan(1.0)/45.0;
   dip=dip*rad_deg;                
   pitch=pitch*rad_deg;
   dipdir=dipdir*rad_deg;
   if (fabs(cos(pitch)) < TOLERANCE)
      temp1 = dipdir;
   else
   {
      temp1 = sin(pitch)*cos(dip)/cos(pitch);
      /* temp1=atan(temp1)+(dipdir-rad); OLD */
      if (pitch > rad)
         temp1 = atan(temp1)+3.1415926535 + (dipdir-rad); /* new*/
      else
         temp1 = atan(temp1)+ (dipdir-rad); /* new*/
   } 
   irotate(rot1,0.0,0.0,1.0,temp1);
   irotate(rot6,0.0,0.0,1.0,-temp1);
   matmlt(rot1,axis,newfa,1);
   matmlt(rot1,pole,newpo,1);
   if (newfa[1][2] > 1.0)
      newfa[1][2] = 1.0;
   else if (newfa[1][2] < -1.0)
      newfa[1][2] = -1.0;
   temp2=acos(newfa[1][2]);
   irotate(rot2,1.0,0.0,0.0,temp2);
   irotate(rot5,1.0,0.0,0.0,-temp2);
   matmlt(rot2,newfa,newfa2,1);
   matmlt(rot2,newpo,newpo2,1);
   if (newpo2[1][1] > 1.0)
      newpo2[1][1] = 1.0;
   else if (newfa[1][1] < -1.0)
      newpo2[1][1] = -1.0;
   temp3=acos(newpo2[1][1]);
   irotate(rot3,0.0,1.0,0.0,temp3);
   irotate(rot4,0.0,1.0,0.0,-temp3);
   matmlt(rot2,rot1,temprt,3);
   matmlt(rot3,temprt,conrot,3);
   matmlt(rot5,rot4,temprt,3);
   matmlt(rot6,temprt,revcon,3);

              /* copy the matricies into the passed array */
   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         forward[i][j] = conrot[i+1][j+1];
         reverse[i][j] = revcon[i+1][j+1];
      }
   }
}

void matadd(vector,x,y,z)
#ifdef _MPL
plural double vector[2][4];
#else
double vector[2][4];
#endif
double x,y,z;
{
/*                                           
c                                                                               
c   matadd adds vector(x,y,z) to vector 'vector'.                               
c                                                                               
*/
      vector[1][1]+=x;                                                 
      vector[1][2]+=y;                                                 
      vector[1][3]+=z;                                                 
}                                                                     


void paxis(theta,phi,vector)
double theta,phi,vector[2][4];
{
/*                                        
c                                                                               
c   paxis calculates the direction cosines of a line from                       
c   it's plunge and plunge direction.                                           
c                                                                               
*/
      double rad;
                                                                
      rad=atan(1.0)/45.0;                                                        
      theta=theta*rad;                                                           
      phi=phi*rad;                                                               
      vector[1][1]=cos(theta)*sin(phi);                                           
      vector[1][2]=cos(theta)*cos(phi);                                           
      vector[1][3]=-sin(theta);                                                   
}                                                                      

void polcar(x,y,r,th)
#ifdef _MPL
plural double x, y;
plural double *r, *th;
#else
double x, y;
double *r, *th;
#endif
{

#ifdef _MPL
   *r= (plural double) p_pow(( (x)* (x)) + ( (y)* (y)),0.5);
#else
   *r= (double) pow(( (x)* (x)) + ( (y)* (y)),0.5);
#endif
   if (*r==0.0 || *r==-0.0)
      *th=0.0;
   else if (*r==-y)
      *th=(double) (-asin((double)1.0))*180.0/3.14159;
   else  if ( y<0 && *r<=-y )
      *th=(double) (-asin((double)1.0))*180.0/3.14159; 
   else if ( *r<=y )
      *th=(double) (asin((double)1.0))*180.0/3.14159;
   else
   {
#ifdef _MPL
      *th = (plural double) (p_asin( y/ *r))*180.0/3.14159;
#else
      *th = (double) (asin((double) y/(double)*r))*180.0/3.14159;
#endif
      if (x == 0)
         *th = 90.0;
   }
   if (x < 0) 
      *th=180.0-(*th);
   if (x > 0) 
      *th=360.0+(*th);
   if (x == 0 && y < 0) 
      *th=270.0;
   if (x>0 && y>0)
      *th-=360.0;

   *th=(*th)*3.14159/180.0;
}


void oldpolcar(x,y,pr,pth)
double x,y,*pr,*pth;
{
/*
c 
c   polcar returns the polar coordinates (r,th)
c   from the given cartesian coordinates (x,y). 
c 
*/
      double r,th,pi;
      
      pi=atan(1.0)*2.0;                                                          
      r=sqrt((x*x)+(y*y));                                                 
      if(x < 0.0 && y < 0.0) {                                            
        th=pi+atan(y/x);                                                         
      }
      else if(x < 0.0 && y >= 0.0) {                                        
        th=pi+atan(y/x);                                                         
      }
      else if(x >= 0.0 && y >= 0.0) {                                        
        if(x > 0.0) {                                                       
          th=atan(y/x);                                                          
        }
         else {                                                                    
          th=pi/2.0;                                                             
        }                                                                   
      }
      else {                                                                      
        if(x > 0.0) {                                                       
          th=atan(y/x)+(2.0*pi);                                                 
        }
      else {                                                                    
          th=1.5*pi;                                                             
        }                                                                   
      }
      *pr=r;
      *pth=th;                                                                     
}
    
    
double rover(y2,y)
double y2,y;
{
/*                                                      
c                                                                               
c   rover calculates the modulus 'y2' position of                               
c   point 'y' relative to the two defining planes.                              
c                                                                               
*/
      double temp,xrover;
      int m;
      
      temp=y/y2;                                                                 
      m=(int)temp;                                                              
      xrover=(fabs((m*y2)-y))/y2;                                                  
      return (xrover);                                                                     
}

void getAlphaBetaGamma (alpha, beta, gamma)
double *alpha, *beta, *gamma;
{
   *alpha = (double) axis[1][1];
   *beta  = (double) axis[1][2];
   *gamma = (double) axis[1][3];
}
                                                                            
void plane(pa,pb,pc,pd,x,y,z)
double *pa,*pb,*pc,*pd,x,y,z;
{
/*                                           
c                                                                               
c   plane calculates the constant term of the equation of the                   
c   plane defined by the direction cosines of the plane normal                  
c   and one point on the plane                                                  
c                                                                               
*/
      *pa=pole[1][1];                                                               
      *pb=pole[1][2];                                                               
      *pc=pole[1][3];                                                               
      *pd=-((*pa*x)+(*pb*y)+(*pc*z));                                                    
}                                                                       


void ninead(vector, sign, x, y, z)
double vector[10][4], sign;
double x, y, z;
{
   register int m;

   for (m = 1; m <= 9; m++)
   {
      vector[m][1] += (sign * x);
      vector[m][2] += (sign * y);
      vector[m][3] += (sign * z);
   }
}

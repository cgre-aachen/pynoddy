/* *******************************************************************

GENERAL DESCRIPTION

   Unevents take arrays of xyz positions and go through the history in
   reverse order to see where a particular xyz location was before the
   current event happened.
   
   eg. a history of
       STRAT, FOLD, TILT, FAULT
       
   is reversed as
       UNfault, UNtilt, UNfold, (the STRAT is the creation of the rock)
   
   When the creation of the rock is reached (eg the base stratigraphy)
   the location is compared to the divisions in the rock stratigraphy
   and and a rock type can then be assigned according to which layer
   the position falls within.

********************************************************************** */
#include <math.h>
#include "xvt.h"
#include "noddy.h"

#define DEBUG(X)    
#define DEBUG1(X)   

#define TOLERANCE   0.001

                 /* ************************************ */
                 /* External Functions used in this file */

double OneYdisplace();
double CalcYdisplace();
extern double calcProfile ();

                 /* ********************************** */
                 /* External Globals used in this file */

                 /* ************************* */
                 /* Globals used in this file */
int unfold (dots, histoire, xmax, ymax, eventOptions)
double ***dots;
STORY **histoire;
int xmax, ymax;
FOLD_OPTIONS *eventOptions;
{
/*                                                         
c
c   unfold calculates the coordinates of the points
c   prior to the folding episode.
c
*/
   double tcyl;
   register int mIndex, nIndex;
   double point[2][4], newpt[2][4];
   double temp2;
   double forwrd[4][4], revers[4][4];
   double waveLength = eventOptions->wavelength;
   double cyl = eventOptions->cycle;
   double amp2 = eventOptions->amplitude;
   int singleFold = eventOptions->singleFold;
   float *profileArray = eventOptions->profile.array;
   
   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);

   if (cyl > TOLERANCE)
      tcyl=0.693147/(cyl*cyl);
   else
      tcyl=0.0;

   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {                                                             
         if (histoire[mIndex][nIndex].again)
         {                                            
            exchng(dots,mIndex,nIndex,point,TRUE);
            matadd(point, -eventOptions->positionX, -eventOptions->positionY,
                                                    -eventOptions->positionZ);
            matmlt(forwrd,point,newpt,1);
            if (fabs(waveLength) > TOLERANCE)
            {
               temp2 = newpt[1][1] * 6.2831853 / waveLength;
               
               if (!singleFold || (newpt[1][1] >= 0.0 && newpt[1][1] < waveLength))
               {
                  newpt[1][3] = newpt[1][3]
                                - (calcProfile(profileArray,temp2) *
                                amp2 * exp(-(newpt[1][2]*newpt[1][2])*tcyl));
               }
            }
            matmlt(revers,newpt,point,1);                                   
            matadd(point, eventOptions->positionX, eventOptions->positionY,
                                                   eventOptions->positionZ);
            exchng(dots,mIndex,nIndex,point,FALSE);
         }                                                                 
      }
   }
   
   return (TRUE);
}

int unplan(dots, histoire, xmax, ymax, eventOptions, num)
double ***dots;
STORY **histoire;
int xmax, ymax;
UNCONFORMITY_OPTIONS *eventOptions;
int num;  /* item count */
{                                                         
   register int mIndex, nIndex;
   double sub;
   double a = eventOptions->plane.a;
   double b = eventOptions->plane.b;
   double c = eventOptions->plane.c;
   double d = eventOptions->plane.d;
   double point[2][4], newpt[2][4];
   double forwrd[4][4];
   int dxfResult, surfaceType;

   if (eventOptions->surfaceType == DXF_SURFACE)
   {                       /* local reference frame for dxf surface */
      local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
      if ((fabs(eventOptions->surfaceXDim) < TOLERANCE)
                || (fabs(eventOptions->surfaceYDim) < TOLERANCE)
                || (fabs(eventOptions->surfaceYDim) < TOLERANCE))
         surfaceType = FLAT_SURFACE;
      else
         surfaceType = eventOptions->surfaceType;
   }
   else
      surfaceType = FLAT_SURFACE;


   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {    
         if (histoire[mIndex][nIndex].again)                                    
         {                                            
            if (surfaceType == DXF_SURFACE)
            {
                ftype cutValue;
                double posScaleX, posScaleY, posScaleZ;
                          /* transform into local reference frame so dxf file is flat */
                exchng(dots, mIndex, nIndex, point, TRUE);
                matadd(point,-eventOptions->positionX, -eventOptions->positionY,
                                                       -eventOptions->positionZ);
                matmlt(forwrd,point,newpt,1);

                posScaleX = eventOptions->surfaceXDim / 1000.0;
                posScaleY = eventOptions->surfaceYDim / 1000.0;
                posScaleZ = eventOptions->surfaceZDim / 1000.0;
                if (dxfResult = DXFsurfHeight(&(eventOptions->dxfData), 0,
                              (ftype) (newpt[1][3]/posScaleY),
                              (ftype) (newpt[1][2]/posScaleX),
                              (ftype) (newpt[1][1]/posScaleZ), &cutValue))
                {
                   sub = (double) -cutValue*posScaleZ;
                }
                else
                   sub = 0.0;
            }

            if ((surfaceType != DXF_SURFACE) || (!dxfResult))
               sub=(a*dots[mIndex][nIndex][1])+(b*dots[mIndex][nIndex][2])+
                   (c*dots[mIndex][nIndex][3])+d;
                   
            if(sub >= 0.0)
            {                                                 
               S2Bits(histoire[mIndex][nIndex].sequence, num-1, UNC_STRAT);
               histoire[mIndex][nIndex].again = FALSE;
            }                                                               
         }                                                                 
      }
   }
   return (TRUE);
}                                                                       



int unrot (dots, histoire, xmax, ymax, object, num, x, y, z, rotation)
double ***dots;
STORY **histoire;
int xmax, ymax;
OBJECT *object;
int num;  /* item count */
double x, y, z;
double rotation[3][3];
{                                                          
   register int mIndex, nIndex;
   double point[2][4], newpt[2][4];
   double forwrd[4][4];

   local4x4From3x3(rotation, forwrd);

   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {                                            
         if(histoire[mIndex][nIndex].again)
         {                                            
            exchng (dots, mIndex, nIndex, point, TRUE);
            if(object->shape != STRAIN)
                matadd (point, -x, -y, -z);
            matmlt(forwrd, point, newpt, 1);
            if(object->shape != STRAIN)
                matadd (newpt, x, y, z);
            exchng (dots, mIndex, nIndex, newpt, FALSE);
         }                                                                 
      }
   }
   return (TRUE);
}

int unplug (dots, histoire, xmax, ymax, eventOptions,
                                              num, previousEventIgneous)
double ***dots;
STORY **histoire;
int xmax, ymax;
PLUG_OPTIONS *eventOptions;
int num;  /* item count */
BOOLEAN previousEventIgneous;
{
   register int mIndex, nIndex;
   double point[2][4], newpt[2][4];
   double wfactr, contac, r, th;
   double forwrd[4][4];
   double ta, tb, tc;
   int ibase;
   double apic = tan(eventOptions->apicalAngle*0.01745329278);
   double bval = eventOptions->BValue;
   double rad = eventOptions->radius;
   double ae = eventOptions->axisA;
   double be = eventOptions->axisB;
   double ce = eventOptions->axisC;
   int mbase = num - 1;
                                                     
   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);

   if (eventOptions->type == CONE_PLUG)
   {                                            
      for (mIndex = 1; mIndex <= xmax; mIndex++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (nIndex = 1; nIndex <= ymax; nIndex++)
         {                                 
            if(histoire[mIndex][nIndex].again)
            {                                          
               exchng(dots, mIndex, nIndex, point, TRUE);
               matadd(point,-eventOptions->positionX, -eventOptions->positionY,
                                                      -eventOptions->positionZ);
               matmlt(forwrd,point,newpt,1);
               wfactr=-newpt[1][1]*apic;
               polcar (newpt[1][2],newpt[1][3],&r,&th);
               contac= 1.0;
               contac*=wfactr;
               if (contac > r && newpt[1][1] < 0.0)
               {                        
                  if(!(eventOptions->mergeEvents && previousEventIgneous))
                  {                                              
                     S2Bits (histoire[mIndex][nIndex].sequence,num-1,IGNEOUS_STRAT);
                     histoire[mIndex][nIndex].again = FALSE;
                  }
                  else
                  {                                                            
                     ibase = mbase - 1;
                     S2Bits (histoire[mIndex][nIndex].sequence,ibase,IGNEOUS_STRAT);
                     histoire[mIndex][nIndex].again = FALSE;
                  }                                                           
               }
            }                                                               
         }
      }
   }
   else if (eventOptions->type == PARABOLIC_PLUG)
   {                                        
      for (mIndex = 1; mIndex <= xmax; mIndex++)
      {
        incrementLongJob (INCREMENT_JOB);
        for (nIndex = 1; nIndex <= ymax; nIndex++)
        {  
           if(histoire[mIndex][nIndex].again)                               
           {                                          
              exchng(dots,mIndex,nIndex,point,TRUE);
              matadd(point,-eventOptions->positionX, -eventOptions->positionY,
                                                     -eventOptions->positionZ);
              matmlt(forwrd,point,newpt,1);                                 
              wfactr=sqrt(fabs(newpt[1][1]*bval));
              polcar (newpt[1][2],newpt[1][3],&r,&th);
              contac=wfactr;
              if (contac > r && newpt[1][1] < 0.0)
              {                        
                 if(!(eventOptions->mergeEvents && previousEventIgneous))
                 {                                              
                    S2Bits (histoire[mIndex][nIndex].sequence,num-1,IGNEOUS_STRAT);
                    histoire[mIndex][nIndex].again = FALSE;
                 }
                 else
                 {                                                            
                    ibase = mbase - 1;
                    S2Bits (histoire[mIndex][nIndex].sequence,ibase,IGNEOUS_STRAT);
                    histoire[mIndex][nIndex].again = FALSE;
                 }                                                           
              }                                                             
           }                                                               
        }
      }
   }
   else if (eventOptions->type == CYLINDRICAL_PLUG)
   {                                                                      
      for (mIndex = 1; mIndex <= xmax; mIndex++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (nIndex = 1; nIndex <= ymax; nIndex++)
         {                                   
            if(histoire[mIndex][nIndex].again)
            {                                          
               exchng(dots,mIndex,nIndex,point,TRUE);
               matadd(point,-eventOptions->positionX, -eventOptions->positionY,
                                                      -eventOptions->positionZ);
               matmlt(forwrd,point,newpt,1) ;                                
               polcar(newpt[1][2],newpt[1][3],&r,&th);                           
               contac=rad;                    
               if (contac > r)
               {                                              
                  if(!(eventOptions->mergeEvents && previousEventIgneous))
                  {                                              
                     S2Bits (histoire[mIndex][nIndex].sequence,num-1,IGNEOUS_STRAT);
                     histoire[mIndex][nIndex].again = FALSE;
                  }
                    else
                  {   
                     ibase = mbase - 1;
                     S2Bits (histoire[mIndex][nIndex].sequence,ibase,IGNEOUS_STRAT);
                     histoire[mIndex][nIndex].again = FALSE;
                  }                                                           
               }
            }                                                               
         }
      }
   }                                                                     
   else if (eventOptions->type == ELLIPSOIDAL_PLUG)
   {                                        
       if(ae==0.0)
          ta=0.00000000001;
       else
          ta=ae*ae;
       if(be==0.0)
          tb=0.00000000001;
       else
          tb=be*be;
       if (ce==0.0)
          tc=0.00000000001;
       else
          tc=ce*ce;
       for (mIndex = 1; mIndex <= xmax; mIndex++)
       {        
          incrementLongJob (INCREMENT_JOB);
          for (nIndex = 1; nIndex <= ymax; nIndex++)
          {
             if (histoire[mIndex][nIndex].again)
             {
                                        
                exchng (dots, mIndex, nIndex, point, TRUE);
                                        
                matadd (point, -eventOptions->positionX, -eventOptions->positionY,
                                                         -eventOptions->positionZ);
                                        
                matmlt(forwrd,point,newpt,1) ;
                                        
                r=((newpt[1][1]*newpt[1][1])/ta)+
                  ((newpt[1][2]*newpt[1][2])/tc)+
                  ((newpt[1][3]*newpt[1][3])/tb);
                contac=1.0;
                if (contac > r)
                {
                   if(!(eventOptions->mergeEvents && previousEventIgneous))
                   { 
                      S2Bits (histoire[mIndex][nIndex].sequence,num-1,IGNEOUS_STRAT);
                      histoire[mIndex][nIndex].again = FALSE;
                   }
                   else
                   {
                      ibase = mbase - 1;
                      S2Bits (histoire[mIndex][nIndex].sequence,ibase,IGNEOUS_STRAT);
                      histoire[mIndex][nIndex].again = FALSE;
                   }                                                       
                }                                                        
             }                                                          
          }
       }
   }                                                                   
   return (TRUE);
}

int undyke (dots, histoire, xmax, ymax, eventOptions,
                                              num, previousEventIgneous)
double ***dots;
STORY **histoire;
int xmax, ymax;
DYKE_OPTIONS *eventOptions;
int num;  /* item count */
BOOLEAN previousEventIgneous;
{
   double deltaz,deltay,tempP,rad;
   int ibase;
   register int mIndex, nIndex;
   double point[2][4], newpt[2][4];
   double forwrd[4][4], revers[4][4];
   double slip = eventOptions->slipLength;
   double width = eventOptions->width;
   double pitch = eventOptions->slipPitch;
   int mbase = num - 1;

   if (eventOptions->type != DILATION_DYKE)
      pitch = 0.0;

   rad   = atan(1.0)/45.0;
   tempP = (-pitch) + 90.0;
   tempP*= rad;
   deltay= cos(tempP) * slip;
   deltaz= sin(tempP) * slip;

   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);

   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {                                             
         if(histoire[mIndex][nIndex].again)
         {                                            
            exchng(dots,mIndex,nIndex,point,TRUE);
            matadd(point,-eventOptions->positionX, -eventOptions->positionY,
                                                   -eventOptions->positionZ);
            matmlt(forwrd,point,newpt,1);                                   
            if((newpt[1][1] <= 0.0) && (eventOptions->type == DILATION_DYKE)) 
            {                                                   
               S2Bits(histoire[mIndex][nIndex].sequence,num-1,FAULT1_STRAT);
               newpt[1][1]=newpt[1][1]+width;
               newpt[1][2]=newpt[1][2]-deltay;
               newpt[1][3]=newpt[1][3]-deltaz;
               matmlt(revers,newpt,point,1);                                 
               matadd(point,eventOptions->positionX, eventOptions->positionY,
                                                     eventOptions->positionZ);
               exchng(dots,mIndex,nIndex,point,FALSE);
            }
            else if(newpt[1][1] > 0.0 && newpt[1][1] <= width)           
            {                                                           
               if(!(eventOptions->mergeEvents && previousEventIgneous))
               {
                               /* use the dyke stratigraphy */
                  S2Bits(histoire[mIndex][nIndex].sequence, num-1, IGNEOUS_STRAT);
                  histoire[mIndex][nIndex].again = FALSE;
               }
               else
               {                                                      
                               /* use the last events stratigraphy */
                  ibase = mbase - 1;  /* the event before the dyke */
                  S2Bits (histoire[mIndex][nIndex].sequence, ibase, IGNEOUS_STRAT);
                  histoire[mIndex][nIndex].again = FALSE;
               }                                                             
            }                                                               
         }                                                                 
      }
   }
   return (TRUE);
}

int unfault (dots, histoire, xmax, ymax, eventOptions, num)
double ***dots;
STORY **histoire;
int xmax, ymax;
FAULT_OPTIONS *eventOptions;
int num;  /* item count */
{
/*                                                         
**                                                                               
** unfault calculates the coordinates of the points                             
** prior to faulting, shear Zone or Kinks.                                               
**                                                                               
*/
   double Sfactor,Factor,xprime,Efactor,Nfactor;
   double deltaX, dist;
   int sign, m, n, dxfResult;
   double oldposvec[2],newposvec[2],Ycut,tcyl;
   double mat_pos[4][4],mat_neg[4][4],mat_pos_sz[4][4],mat_neg_sz[4][4];
   double rrot,concdist,Ytest,SZ_factor,scale_width;
   double point[2][4], newpt[2][4];
   double forwrd[4][4], revers[4][4];
   double sineCoeff[2][11];
   double slip = eventOptions->slip;
   double pitch = eventOptions->pitch;
   double xAxis = eventOptions->xAxis;
   double yAxis = eventOptions->yAxis;
   double zAxis = eventOptions->zAxis;
   double amp = eventOptions->amplitude;
   double rot = eventOptions->rotation;
   double width = eventOptions->width;  /* Shear Zone Only */
   double radius = eventOptions->radius; /* ? not yet defined xAxis/2 ? */
   double profilePitch = eventOptions->profilePitch;
   float *profileArray = eventOptions->profile.array;
   int surfaceType;

   if (eventOptions->surfaceType == DXF_SURFACE)
   {                       /* local reference frame for dxf surface */
      if ((fabs(eventOptions->surfaceXDim) < TOLERANCE)
                || (fabs(eventOptions->surfaceYDim) < TOLERANCE)
                || (fabs(eventOptions->surfaceYDim) < TOLERANCE))
         surfaceType = FLAT_SURFACE;
      else
         surfaceType = eventOptions->surfaceType;
   }
   else
      surfaceType = FLAT_SURFACE;

                   /* setup the four co-efficients for a sine wave */
   initSineFourierCoeff (sineCoeff);

          /* generate rotation matrices if rotational fault */
   if (eventOptions->geometry == ROTATION) 
   {
      if (eventOptions->movement == BOTH)
         rot = rot / 2.0;
      rrot=rot*3.1415927/180.0;
      irotate(mat_pos,1.0,0.0,0.0,-rrot);                                                 
      irotate(mat_neg,1.0,0.0,0.0,rrot);                                                 
   }      /* generate cylindricity index for curved faults */
   else if (eventOptions->geometry == CURVED) 
   {
      double cylIndex = eventOptions->cylindricalIndex;
       
      if (fabs(cylIndex) > TOLERANCE)
         tcyl=0.693147/(cylIndex*cylIndex);                            
      else
         tcyl=0.0;                                          
   }
   
        /* scaled slip factor */
   if ((eventOptions->geometry == CURVED) ||   
                         (eventOptions->geometry == ELLIPTICAL))
      if (eventOptions->movement == BOTH)                         
         Sfactor = slip/(2.0*eventOptions->xAxis);
      else
         Sfactor = slip/eventOptions->xAxis;
   else
      if (eventOptions->movement == BOTH)                         
         Sfactor = slip/2.0;
      else
         Sfactor = slip;
                                                
   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);
 
   for (m = 1; m <= xmax; m++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (n = 1; n <= ymax; n++)
      {
         if (histoire[m][n].again)
         {
            exchng(dots, m, n, point, TRUE);
            matadd(point,-eventOptions->positionX, -eventOptions->positionY,
                                                   -eventOptions->positionZ);
            matmlt(forwrd,point,newpt,1);

            /* calculate local fault plane position for curved faults */
            if (surfaceType == DXF_SURFACE)
            {
                ftype cutValue;
                double posScaleX, posScaleY, posScaleZ;
                
                posScaleX = eventOptions->surfaceXDim / 1000.0;
                posScaleY = eventOptions->surfaceYDim / 1000.0;
                posScaleZ = eventOptions->surfaceZDim / 1000.0;
                if (dxfResult = DXFsurfHeight(&(eventOptions->dxfData), 0,
                              (ftype) (newpt[1][3]/posScaleY),
                              (ftype) (newpt[1][2]/posScaleX),
                              (ftype) 0.0, &cutValue))
                {
                   Ycut = (double) cutValue*posScaleZ;
                }
                else
                   Ycut = 0.0;
            }

            if ((surfaceType != DXF_SURFACE) || (!dxfResult))
            {
               if (eventOptions->geometry == CURVED)
               {
                  oldposvec[0]=newpt[1][2];
                  oldposvec[1]=newpt[1][3];
                             /* replaced seperate FWAVE with XAXIS */
                  ProfilePosition(oldposvec[0], oldposvec[1], profilePitch, xAxis/**2.2*/, newposvec);
                            /* replaced seperate FWAVE with XAXIS */
                  Ycut= OneYdisplace(newposvec, profileArray, xAxis/**2.2*/, amp, tcyl);
               }
               /* calculate local fault plane position for ring fault */
               else if (eventOptions->geometry == RING)
               {
                  Ycut = radius;
                  concdist=sqrt((newpt[1][1]*newpt[1][1])
                               +(newpt[1][3]*newpt[1][3]));
               }
               else /* otherwise fault cuts origin */
                  Ycut=0.0;
            }
                           /* calculate position within ellipsoid */
            if ((eventOptions->geometry == CURVED)
                         || (eventOptions->geometry == ELLIPTICAL))
               Efactor=pow((newpt[1][2]) / xAxis,2.0)+
                       pow((newpt[1][1]) / yAxis,2.0)+
                       pow((newpt[1][3]) / zAxis,2.0);
            else
               Efactor=0.0;
            
                         /* discontinuity codes for faults */
            if (eventOptions->type == FAULT_EVENT)
            {                   /* above planar inifinite fault */
               if ((Efactor < 1.0) && (newpt[1][1] > Ycut)
                                   && (eventOptions->geometry != RING)
                                   && (eventOptions->geometry != CURVED)
                                   && (eventOptions->geometry != ELLIPTICAL))
               {
                  int nummmm;
               
                  nummmm = num-1;
             
                  S2Bits(histoire[m][n].sequence, nummmm, FAULT1_STRAT);
               }                    /* inside ring fault */
               else if (eventOptions->geometry == RING && (concdist < Ycut))
               {
                  int nummmm;
               
                  nummmm = num-1;
             
                  S2Bits(histoire[m][n].sequence, nummmm, FAULT1_STRAT);
               }                              /* above curved or elliptical fault, inside ellipsoid */
               else if ((Efactor < 1.0) && (newpt[1][1] > Ycut)
                                        && (eventOptions->geometry != RING)
                                        && ((eventOptions->geometry == CURVED) ||
                                            (eventOptions->geometry == ELLIPTICAL)))
               {
                  int nummmm;
               
                  nummmm = num-1;
             
                  S2Bits(histoire[m][n].sequence, nummmm, FAULT3_STRAT);
               }                              /* below fault, inside ellipsoid */
               else if ((Efactor < 1.0) && (newpt[1][1] <= Ycut)
                                        && ((eventOptions->geometry == CURVED) ||
                                            (eventOptions->geometry == ELLIPTICAL)))
               {
                  int nummmm;
               
                  nummmm = num-1;
             
                  S2Bits(histoire[m][n].sequence, nummmm, FAULT2_STRAT);
               }                 
            } 
                            
            if (Efactor < 1.0 ) /* needs to be faulted */
            {
               if ((eventOptions->type == SHEAR_ZONE_EVENT)
                       && (eventOptions->geometry != RING))
               {
                  if (newpt[1][1] > width/2 || newpt[1][1] < -width/2)
                     SZ_factor=1.0;
                  else
                  {
                     scale_width=(fabs(newpt[1][1])*3.1415927/width);                                    
                     SZ_factor=fourier(sineCoeff, scale_width);
                  }
               } 
               else if ((eventOptions->type == KINK_EVENT)
                            && (eventOptions->geometry != RING))
               {
                  if (newpt[1][1] > width/2 || newpt[1][1] < -width/2)
                     SZ_factor=1.0;
                  else
                     SZ_factor=2.0*fabs(newpt[1][1])/(width);
               }
               else if ((eventOptions->type == SHEAR_ZONE_EVENT)
                            && (eventOptions->geometry == RING))
               {
                  if (concdist < (radius-(width/2))
                             || concdist > (radius+(width/2)) )
                     SZ_factor=1.0;
                  else
                  {
                     scale_width=(fabs(radius-concdist)*3.1415927/(width));                                    
                     SZ_factor=fourier(sineCoeff,scale_width);
                  }
               } 
               else if ((eventOptions->type == KINK_EVENT)
                           && (eventOptions->geometry == RING))
               {
                  if (concdist < (radius-(width/2))
                             || concdist > (radius+(width/2)) )
                     SZ_factor=1.0;
                  else
                     SZ_factor=2.0*fabs(radius-concdist)/(width);
               }
               else
                  SZ_factor=1.0;
                     
               if (eventOptions->geometry == RING) /* cutoff test */
                  Ytest=concdist;
               else
                  Ytest=newpt[1][1];

                         /* Make movement opposite for hanging wall
                         ** and foot wall */           
               if (Ytest > Ycut) 
               {
                  if ((eventOptions->movement == HANGING_WALL)
                            || (eventOptions->movement == BOTH)) 
                     sign = -1;
                  else
                     sign = 0;
               }
               else
               {
                  if ((eventOptions->movement == FOOT_WALL)
                             || (eventOptions->movement == BOTH)) 
                     sign = 1;
                  else
                     sign = 0;
               }              
               
                         /* curved & ellipsoidal faults */
               if ((eventOptions->geometry == CURVED) 
                         || (eventOptions->geometry == ELLIPTICAL))
               {
                  Nfactor=(pow((yAxis-fabs(newpt[1][1]-Ycut)) / yAxis,2.0)*
                           pow((zAxis-fabs(newpt[1][3])) / zAxis,2.0));
                  Factor = Sfactor*-sign;
                  
                  NewtonRaphson(newpt[1][2],xAxis,Factor,&xprime);
                  deltaX = xprime - newpt[1][2];
                  
                  newpt[1][2] = newpt[1][2] + (deltaX*Nfactor*SZ_factor);
               }
               else if ((eventOptions->geometry == TRANSLATION) 
                            || (eventOptions->geometry == RING))
                  newpt[1][2]=newpt[1][2]+(SZ_factor*sign*Sfactor);
               else if (eventOptions->geometry == ROTATION)
               {
                  if (SZ_factor < 1.0)
                  {
                     rrot = SZ_factor*rot*3.1415927/180.0;
                     irotate (mat_pos_sz,1.0,0.0,0.0,-rrot);                                                 
                     irotate (mat_neg_sz,1.0,0.0,0.0,rrot);                                                 
                     if (sign == -1)
                        matmlt(mat_pos_sz,newpt,point,1);                                   
                     else if (sign == 1)
                        matmlt(mat_neg_sz,newpt,point,1);
                  }
                  else
                  {
                     if (sign == -1)
                        matmlt (mat_pos,newpt,point,1);                                   
                     else if (sign == 1)
                        matmlt (mat_neg,newpt,point,1);
                  }
                  if (sign != 0)
                      mat_copy(newpt,point,1);                                   
               }
                  /* add extra Y displacement for curved faults */                  
               if (eventOptions->geometry == CURVED)
                  newpt[1][1]=CalcYDisplace(newpt[1][1], oldposvec[0],
                            newpt[1][2], newpt[1][3], xAxis, yAxis, zAxis,
                            (double) sign, profilePitch, xAxis/**2.2*/,
                            profileArray, amp, tcyl); 
            }


                      /* calculate pos after slip so we can curve slip around surface */
            if (surfaceType == DXF_SURFACE)
            {
                int laterDxfResult;
                ftype cutValue;
                double posScaleX, posScaleY, posScaleZ;
                double laterYcut;
                      
                posScaleX = eventOptions->surfaceXDim / 1000.0;
                posScaleY = eventOptions->surfaceYDim / 1000.0;
                posScaleZ = eventOptions->surfaceZDim / 1000.0;
                if (laterDxfResult = DXFsurfHeight(&(eventOptions->dxfData), 0,
                              (ftype) (newpt[1][3]/posScaleY),
                              (ftype) (newpt[1][2]/posScaleX),
                              (ftype) 0.0, &cutValue))
                {
                   laterYcut = (double) cutValue*posScaleZ;
                }
                
                if (dxfResult || laterDxfResult)
                {
                   dist = fabs(newpt[1][1]);
                   newpt[1][1] -= (1.0/exp((dist/2000.0)*1.0)) * (laterYcut-Ycut);
                }
            }
            
            matmlt(revers,newpt,point,1);                                   
            matadd(point, eventOptions->positionX, eventOptions->positionY,
                                                   eventOptions->positionZ);
            exchng(dots,m,n,point,FALSE);                                      
         }                                                                 
      }
   }

   return (TRUE);
}


int unbiot (dots, histoire, xmax, ymax, eventOptions)
double ***dots;
STORY **histoire;
int xmax, ymax;
FOLD_OPTIONS *eventOptions;
{                                                         
                                                      
   int inc;
   double twave,tcyl,tamp,txi;
   register int mIndex, nIndex;
   double point[2][4], newpt[2][4];
   double x, y, z;
   double fun[4],jacob[4][4];                    
   double delta[4];
   double sinelx,coselx,sinxez,cosxez,epsiln,zcyl;
   double diff;
   double forwrd[4][4], revers[4][4];
   double wave = eventOptions->wavelength;
   double cyl = eventOptions->cycle;
   double xi = eventOptions->axisPitch;  /* ??? */
           
   delta[0] = delta[1] = delta[2] = delta[3] = 0.0;

   if (cyl!=0.0)
      tcyl=0.693147/(cyl*cyl);
   else
      tcyl=0.0;
   twave=2*3.1415927/wave;
   tamp=eventOptions->amplitude * 185;
   txi=xi;
   
   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);

   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {
         if (histoire[mIndex][nIndex].again)
         {
            inc=0;

            exchng(dots,mIndex,nIndex,point,TRUE);
            matadd(point,-eventOptions->positionX, -eventOptions->positionY,
                                                   -eventOptions->positionZ);
            matmlt(forwrd,point,newpt,1);

            if (!eventOptions->singleFold
                              || (newpt[1][1] >=0.0 && newpt[1][1]<wave))
            {
               x=newpt[1][1];
               y=newpt[1][2];
               z=newpt[1][3];

               do
               {
                  inc=inc+1;
                  sinelx=sin(twave*x);
                  coselx=cos(twave*x);

                  sinxez=sin(txi*twave*z);
                  cosxez=cos(txi*twave*z);
                  epsiln=exp(-tcyl*(y*y));

                  zcyl=2.0*y*tcyl;

                  fun[1]=-(x-(twave*tamp*txi*coselx*sinxez*epsiln)-newpt[1][1]);
                  fun[2] =-(y-newpt[1][2]);
                  fun[3]=-(z+(twave*tamp*sinelx*cosxez*epsiln)-newpt[1][3]);

                  jacob[1][1]=1.0+(tamp*(twave*twave)*txi*sinelx*sinxez*epsiln);
                  jacob[1][2]=zcyl*txi*twave*tamp*coselx*sinxez*epsiln;
                  jacob[1][3]=-tamp*(twave*twave)*(txi*txi)*coselx*cosxez*epsiln;
                  jacob[2][1]=0.0;
                  jacob[2][2]=1.0;
                  jacob[2][3]=0.0;

                  jacob[3][1]=tamp*twave*twave*coselx*cosxez*epsiln;
                  jacob[3][2]=-zcyl*tamp*twave*sinelx*cosxez*epsiln;
                  jacob[3][3]=1.0-(tamp*twave*twave*txi*sinelx*sinxez*epsiln);

                  /* f04atf(jacob,fun,delta); */
                  diff=fabs(delta[1])+fabs(delta[3]);

                  x=x+delta[1];
                  y=y+delta[2];
                  z=z+delta[3];
               } while(diff > 0.001 && inc < 20);

               newpt[1][1] = x;
               newpt[1][2] = y;
               newpt[1][3] = z;
            }
            matmlt(revers,newpt,point,1);
            matadd(point, eventOptions->positionX, eventOptions->positionY,
                                                   eventOptions->positionZ);
            exchng(dots,mIndex,nIndex,point,FALSE);
         }
      }
   }
   return (TRUE);
}
 

int unloft(dots, histoire, xmax, ymax, num, forward, reverse, fourierCoeff)
double ***dots;
STORY **histoire;
int xmax, ymax;
int num;  /* item count */
double forward[3][3], reverse[3][3];
double **fourierCoeff;
{                                                         
/* PAUL never seem to be called anyway
   double sub1,sub2;
   register int m, n, mIndex, nIndex;
   double point[2][4], newpt[2][4];
   double temp2, temp2, temp3;
   double forwrd[4][4], revers[4][4];
       
   local4x4From3x3(forward, forwrd);
   local4x4From3x3(reverse, revers);

   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {                                           
         if(histoire[mIndex][nIndex].again)
         {                                            
            exchng(dots,mIndex,nIndex,point,TRUE);
            matadd(point,-histry[num][5],-histry[num][6],-histry[num][7]);                                            
            matmlt(forwrd,point,newpt,1);
            temp1=(1.0+sin((rover(histry[num][12],newpt[1][2])
                                       * 3.14159)-1.57079))*0.5;
            temp2=newpt[1][1]*6.2831853/histry[num][9];
            temp3=newpt[1][1]*6.2831853/histry[num][11];
            sub1=foursm(histry[num][14],temp2)*histry[num][8];
            sub1=sub1*temp1;
            .* PAUL sub2=foursm(histry[num][18],temp3)*histry[num][10]; *.
            sub2=fourier(eventOptions->fourierCoeff,temp3)*histry[num][10];
            sub2=sub2*(1-temp1);
            newpt[1][3]=newpt[1][3]-(sub1+sub2);
            matmlt(revers,newpt,point,1);                                   
            matadd(point, histry[num][5], histry[num][6], histry[num][7]);
            exchng(dots,mIndex,nIndex,point,FALSE);
         }                                                                 
      }
   }
*/
   return (TRUE);
}                                                                       


int unbury (dots, histoire, xmax, ymax, num)
double ***dots;
STORY **histoire;
int xmax, ymax;
int num;  /* item count */
{                                                         
/* PAUL never seems to be called as kburie is never actually
** assigned to anything
   double tx,ty,ratx,raty,hight1,hight2,height;
   int ix,iy;
   register int m,n, mIndex, nIndex;


   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {                                         
         if(histoire[mIndex][nIndex].again)
         {                                            
            tx=dots[mIndex][nIndex][1]+500.0;                                                
            ty=dots[mIndex][nIndex][2]+350.0;                                                
            tx=rover(2000.0,tx)*12.0;                                            
            ty=rover(1400.0,ty)*8.0;                                             
            ix=(int)tx+1;                                                       
            iy=(int)ty+1;                                                       
            ratx=tx-(ix);                                                   
            raty=ty-(iy);                                                   
            if (ix != 13 && iy != 9)
            {                                       
               hight1=(bury[ix][iy]*(1-ratx))+(bury[ix+1][iy]*ratx);
               hight2=(bury[ix][iy+1]*(1-raty))+(bury[ix+1][iy+1]*raty);
               height=(hight1*(1-raty))+(hight2*raty);
            }
         else if (ix == 13 && iy == 9)
            {
               height = bury[ix][iy];
            }
         else if (ix == 13)
            {
               height=(bury[ix][iy]*(1-raty))+(bury[ix][iy+1]*raty);
            }
         else if (iy == 9)
            {
               height=(bury[ix][iy]*(1-ratx))+(bury[ix+1][iy]*ratx);
            }                                                               
            height=(height*histry[num][6])+histry[num][5];                         
            if (height < dots[mIndex][nIndex][3])
            {                                      
               S2Bits(histoire[mIndex][nIndex],num-1,UNC_STRAT);
               histoire[mIndex][nIndex].again = FALSE;
            }                                                               
         }                                                                 
      }
   }
*/
   return (TRUE);
}


int unImport(dots, histoire, xmax, ymax, eventOptions, num)
double ***dots;
STORY **histoire;
int xmax, ymax;
IMPORT_OPTIONS *eventOptions;
int num;  /* item count */
{                                                         
   register int mIndex, nIndex;
   int layerIndex;
   LAYER_PROPERTIES *layerProp;

   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {    
         if (histoire[mIndex][nIndex].again)                                    
         {
            layerProp = getClosestImportBlock (eventOptions,
                                      dots[mIndex][nIndex][1],
                                      dots[mIndex][nIndex][2],
                                      dots[mIndex][nIndex][3], &layerIndex);
            if (layerProp)
            {
               S2Bits(histoire[mIndex][nIndex].sequence, num-1, UNC_STRAT);
               histoire[mIndex][nIndex].again = FALSE;
            }
         }                                                                 
      }
   }
   return (TRUE);
}                                                                       

int ungeneric (dots, histoire, xmax, ymax, eventOptions)
double ***dots;
STORY **histoire;
int xmax, ymax;
GENERIC_OPTIONS *eventOptions;
{                                                         

   return (TRUE);
}


void reverseEvents (dots, histoire, xmax, ymax)
double ***dots;
STORY **histoire;
int xmax, ymax;
{
   OBJECT *p = NULL;
   WINDOW listWindow;
   register int num;
   BOOLEAN stopFound = FALSE;
   int shape;

   DEBUG(printf("\nREVERSE EVENTS");)
                 /* first count how many events we have */
   listWindow = (WINDOW) getEventDrawingWindow ();
   num = (int) countObjects (listWindow);

   do
   {
      p = (OBJECT *) nthObject (listWindow, num-1);
      if (!p) return;
      shape = (int) p->shape;

      switch (shape)
      {
         case FOLD:
              {
                 FOLD_OPTIONS *options=(FOLD_OPTIONS *) p->options;
/*
//                 if (options->type == BIOT_FOLD)
//                    unbiot (dots, histoire, xmax, ymax, options);
//                 else
*/
                 unfold (dots, histoire, xmax, ymax, options);
              }
              break;
         case FAULT:
              {
                 FAULT_OPTIONS *options=(FAULT_OPTIONS *) p->options;

                 unfault(dots, histoire, xmax, ymax, options, num);
              }
              break;
        case UNCONFORMITY:
              {
                 UNCONFORMITY_OPTIONS *options
                               = (UNCONFORMITY_OPTIONS *) p->options;

                 unplan(dots, histoire, xmax, ymax, options, num);
              }
              break;
         case SHEAR_ZONE:
              {
                 SHEAR_OPTIONS *options=(SHEAR_OPTIONS *) p->options;

                 unfault(dots, histoire, xmax, ymax, (FAULT_OPTIONS *) options, num);
              }
              break;
         case DYKE:
              {
                 DYKE_OPTIONS *options=(DYKE_OPTIONS *) p->options;
                 int previousEventIgneous;
                 OBJECT *previous_p;

                 previous_p = (OBJECT *) nthObject (listWindow, num-2);
                 if (previous_p && ((previous_p->shape == PLUG)
                                      || (previous_p->shape == DYKE)))
                    previousEventIgneous = TRUE;
                 else
                    previousEventIgneous = FALSE;

                 undyke(dots, histoire, xmax, ymax,
                                        options, num, previousEventIgneous);
              }
              break;
         case PLUG:
              {
                 PLUG_OPTIONS *options=(PLUG_OPTIONS *) p->options;
                 int previousEventIgneous;
                 OBJECT *previous_p;

                 previous_p = (OBJECT *) nthObject (listWindow, num-2);
                 if (previous_p && ((previous_p->shape == PLUG)
                                      || (previous_p->shape == DYKE)))
                    previousEventIgneous = TRUE;
                 else
                    previousEventIgneous = FALSE;

                 unplug(dots, histoire, xmax, ymax,
                                        options, num, previousEventIgneous);
              }
              break;
         case STRAIN:
              {
                 STRAIN_OPTIONS *options=(STRAIN_OPTIONS *) p->options;

                 unrot(dots, histoire, xmax, ymax, p, num,
                       0.0, 0.0, 0.0, options->inverseTensor);
              }
              break;
         case TILT:
              {
                 TILT_OPTIONS* options=(TILT_OPTIONS *) p->options;

                 unrot(dots, histoire, xmax, ymax, p, num,
                       options->positionX, options->positionY,
                       options->positionZ, options->rotationMatrix);
              }
              break;
         case FOLIATION:
              {
                 FOLIATION_OPTIONS *options
                            =(FOLIATION_OPTIONS *) p->options;

              }
              break;
         case LINEATION:
              {
                 LINEATION_OPTIONS *options
                            =(LINEATION_OPTIONS *) p->options;

              }
              break;
         case IMPORT:
              {
                 IMPORT_OPTIONS *options
                            =(IMPORT_OPTIONS *) p->options;

                 unImport(dots, histoire, xmax, ymax, options, num);
              }
              break;
         case GENERIC:
              {
                 GENERIC_OPTIONS *options
                            =(GENERIC_OPTIONS *) p->options;

                 ungeneric (dots, histoire, xmax, ymax, options);
              }
              break;
         case STOP:
              stopFound = TRUE;
              break;
      }
      
      num--;
   } while ((!stopFound) && (num > 0));
}                                              


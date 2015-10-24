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
#define DEBUG1(X)        

#define TOLERANCE       0.001

                 /* ************************************ */
                 /* External Functions used in this file */
double OneYdisplace();
double CalcYdisplace();
extern double calcProfile ();

                 /* ********************************** */
                 /* External Globals used in this file */



void fold (spots, eventOptions)
double spots[10][4];   /* array for orientation calculation */
FOLD_OPTIONS *eventOptions;
{                                                           
   double tcyl;
   int m;
   double forwrd[4][4], revers[4][4];
   double waveLength = eventOptions->wavelength;
   double cyl = eventOptions->cycle;
   double amp = eventOptions->amplitude;
   int singleFold = eventOptions->singleFold;
   float *profileArray = eventOptions->profile.array;
#ifdef _MPL
   plural double grots[10][4];
   plural double temp2;
#else
   double grots[10][4];
   double temp2;
#endif
   
   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);

   if (cyl > TOLERANCE)
      tcyl=0.693147/(cyl*cyl);                         
   else
      tcyl=0.0;                                          

   ninead(spots, -1.0, eventOptions->positionX,
                       eventOptions->positionY, eventOptions->positionZ);                                                   
   matmlt(forwrd, spots, grots, 9);

   for (m = 1; m <= 9; m++)
   {                                                               
      if (fabs(waveLength) > TOLERANCE)
      {
         temp2 = grots[m][1] * 6.2831853 / waveLength;
         if (!singleFold || (grots[m][1] >= 0.0 && grots[m][1] < waveLength))
         {
            grots[m][3] = grots[m][3]
                          + (calcProfile(profileArray,temp2) *
                          amp * exp(-(grots[m][2]*grots[m][2])*tcyl));
         }
      }
   }
   matmlt(revers,grots,spots,9);
   ninead(spots,  1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);
}

void biott (spots, eventOptions)
double spots[10][4];   /* array for orientation calculation */
FOLD_OPTIONS *eventOptions;
{                                                           
   double delx,delz;
   double tcyl,twave,tamp,txi;
   int m;
   double forwrd[4][4], revers[4][4];
   double wave = eventOptions->wavelength;
   double cyl = eventOptions->cycle;
   double xi = eventOptions->axisPitch;  /* ??? */
   double grots[10][4];
 
   if (cyl > TOLERANCE)
      tcyl=0.693147/(cyl*cyl);
   else
      tcyl=0.0;

   twave = 2*3.1415927/wave;
   tamp = eventOptions->amplitude * 185;
   txi = xi * wave / tamp;
 
   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);

   ninead(spots, -1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);
   matmlt(forwrd,spots,grots,9);

   for(m=1;m<=9;m++)
   {
      delx=-tamp*twave*txi*cos(twave*txi*grots[m][1])*
            sin(txi*twave*grots[m][3])*exp(-(grots[m][2]*grots[m][2])*tcyl);
      delz=tamp*twave*sin(twave*grots[m][1])*cos(txi*twave*grots[m][3])* 
            exp(-(grots[m][2]*grots[m][2])*tcyl);

      grots[m][1]=grots[m][1]+delx;
      grots[m][3]=grots[m][3]+delz;
   }

   matmlt(revers,grots,spots,9);
   ninead(spots,  1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);
}

/* PAUL
void loft (spots, forward, reverse)
double spots[10][4];   .* array for orientation calculation *.
double forward[3][3], reverse[3][3];
{
   double t1,t2,t3,sub1,sub2,sub3;
   int m;
   double forwrd[4][4], revers[4][4];
   double grots[10][4];
                                                       
   local4x4From3x3(forward, forwrd);
   local4x4From3x3(reverse, revers);

   ninead(spots, -1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);                                                   
   matmlt(forwrd,spots,grots,9);                                         
   for (m=1;m<=9;m++)
   {                                                               
      t1=(1.0+sin((rover(histry[num][12],grots[m][2])*3.14159)-1.57079))*0.5;
      t2=grots[m][1]*6.28318/histry[num][9];
      t3=grots[m][1]*6.28318/histry[num][11];
      sub1 = fourier (histry[num][14],t2)*histry[num][8];
      sub1 = sub1*t1;
      sub2 = fourier (histry[num][18],t3)*histry[num][10];
      sub2 = sub2*(1.0-t1);
      grots[m][3]=grots[m][3]+sub1+sub2;
   }
   matmlt(revers,grots,spots,9);
   ninead(spots,  1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);
}
*/

double fault (spots, eventOptions)
double spots[10][4];   /* array for orientation calculation */
FAULT_OPTIONS *eventOptions;
{                                                           
   double Sfactor, Factor, Efactor, Nfactor, distmt = 999999.9;
   int sign, m;
   double oldposvec[2], newposvec[2], Ycut, tcyl, dist;
   double mat_pos[4][4], mat_neg[4][4], mat_pos_sz[4][4], mat_neg_sz[4][4],
          rrot, concdist, Ytest, SZ_factor, scale_width;
   double grots[10][4];
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
   float *profileArray = eventOptions->profile.array;
   int dxfResult, surfaceType;

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

   if(eventOptions->geometry == ROTATION) 
   {
      if (eventOptions->movement == BOTH)
         rot = rot / 2.0;
      rrot=rot*3.1415927/180.0;
      irotate(mat_pos,1.0,0.0,0.0,rrot);
      irotate(mat_neg,1.0,0.0,0.0,-rrot);
   }
   else if (eventOptions->geometry == CURVED) 
   {
      double cylIndex = eventOptions->cylindricalIndex;
       
      if (fabs(cylIndex) > TOLERANCE)
         tcyl=0.693147/(cylIndex*cylIndex);                            
      else
         tcyl=0.0;                                          
   }
                                  /* ****************** */
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

   ninead(spots, -1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);                                                   
   matmlt(forwrd, spots, grots, 9);

   for (m = 1; m <= 9; m++) 
   {                                                               
          /* calculate local fault plane position for curved faults */
      if (surfaceType == DXF_SURFACE)
      {
          ftype cutValue;
          double posScaleX, posScaleY, posScaleZ;
                
          posScaleX = eventOptions->surfaceXDim / 1000.0;
          posScaleY = eventOptions->surfaceYDim / 1000.0;
          posScaleZ = eventOptions->surfaceZDim / 1000.0;
          if (dxfResult = DXFsurfHeight(&(eventOptions->dxfData), 0,
                        (ftype) (grots[1][3]/posScaleY),
                        (ftype) (grots[1][2]/posScaleX),
                        (ftype) 0.0, &cutValue))
          {
             Ycut = (double) cutValue*posScaleZ;
          }
          else
             Ycut = 0.0;
      }

      if ((surfaceType != DXF_SURFACE) || (!dxfResult))
      {
               /* calculate local fault plane position for curved faults */            
         if (eventOptions->geometry == CURVED)
         {
            oldposvec[0]=grots[m][2];
            oldposvec[1]=grots[m][3];
                                /* replaced seperate FWAVE with XAXIS */
            ProfilePosition(oldposvec[0],oldposvec[1],  pitch, xAxis/* *2.2*/,newposvec);
                                /* replaced seperate FWAVE with XAXIS */
            Ycut= OneYdisplace(newposvec, profileArray, xAxis/* *2.2*/, amp, tcyl);
         }
               /* calculate local fault plane position for ring fault */
         else if (eventOptions->geometry == RING)
         {
            Ycut = radius;
            concdist=sqrt((grots[m][1]*grots[m][1])
                         +(grots[m][3]*grots[m][3]));
         }
         else  /* otherwise fault cuts origin */
            Ycut=0.0;
      }
                           /* calculate position within ellipsoid */
      if ((eventOptions->geometry == CURVED)
                   || (eventOptions->geometry == ELLIPTICAL))
         Efactor = pow((grots[m][2]) / xAxis, 2.0) +
                   pow((grots[m][1]) / yAxis, 2.0) +
                   pow((grots[m][3]) / zAxis, 2.0);
      else
         Efactor = 0.0;
 
      if (Efactor < 1.0)  /* needs to be faulted */
      {                       
                             /* Work out SZ_factor if the event is a shear Zone or Kink */
                             /* Note for FAULT the SZ_factor comes to simply 1 */
         if ((eventOptions->type == SHEAR_ZONE_EVENT)
                 && (eventOptions->geometry != RING))
         {
            if (grots[m][1] > width/2 || grots[m][1] < -width/2)
               SZ_factor=1.0;
            else
            {
               scale_width=(fabs(grots[m][1])*3.1415927/width);                                    
               SZ_factor=fourier(sineCoeff, scale_width);
            }
         } 
         else if ((eventOptions->type == KINK_EVENT)
                      && (eventOptions->geometry != RING))
         {
            if (grots[m][1] > width/2 || grots[m][1] < -width/2)
               SZ_factor=1.0;
            else
               SZ_factor=2.0*fabs(grots[m][1])/(width);
         }
         else if ((eventOptions->type == SHEAR_ZONE_EVENT)
                     && (eventOptions->geometry == RING))
         {
            if (concdist < (radius-(width/2))
                           || concdist > (radius+(width/2)) )
               SZ_factor=1.0;
            else
            {
               scale_width = (fabs(radius-concdist)*3.1415927/(width));
               SZ_factor = fourier(sineCoeff,scale_width);
            }
         } 
         else if ((eventOptions->type == KINK_EVENT)
                      && (eventOptions->geometry == RING))
         {
            if (concdist < (radius-(width/2))
                                || concdist > (radius+(width/2)) )
               SZ_factor = 1.0;
            else
               SZ_factor = 2.0*fabs(radius-concdist)/width;
         }
         else
            SZ_factor=1.0;

         if (eventOptions->geometry == RING) /* cutoff test */
            Ytest = concdist;
         else
            Ytest = grots[m][1];
                  
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
         
                /* Store the distance to the contact point */
                /* Ytest is the offset from Zero, Ycut is the fault plane offset */
         if (m == 5)
            distmt = Ytest - Ycut;
                  
                         /* curved & ellipsoidal faults */
         if ((eventOptions->geometry == CURVED) 
                   || (eventOptions->geometry == ELLIPTICAL))
         {
            Nfactor = (pow((yAxis-fabs(grots[m][1]-Ycut)) / yAxis,2.0)*
                       pow((zAxis-fabs(grots[m][3])) / zAxis,2.0));
            Factor = Nfactor*Sfactor*-sign*SZ_factor;
            
            grots[m][2] = grots[m][2] + Factor*xAxis* 
                          pow(1-(pow(grots[m][2]/ xAxis, 2.0)), 2.0); /* mwj_fix */

            if (eventOptions->geometry == CURVED)
               grots[m][1] = CalcYDisplace (grots[m][1], oldposvec[0],
                                grots[m][2], grots[m][3], xAxis, yAxis, zAxis,
                                sign, pitch, xAxis/* *2.2 */, profileArray,
                                amp, tcyl); 
         }
         else if ((eventOptions->geometry == TRANSLATION) 
                   || (eventOptions->geometry == RING))
             grots[m][2] = grots[m][2] - (SZ_factor*sign*Sfactor);
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
                        (ftype) (grots[1][3]/posScaleY),
                        (ftype) (grots[1][2]/posScaleX),
                        (ftype) 0.0, &cutValue))
         {
            laterYcut = (double) cutValue*posScaleZ;
         }

         if (dxfResult || laterDxfResult)
         {
            dist = fabs(grots[m][1]);
            grots[m][1] += (1.0/exp((dist/2000.0)*1.0)) * (laterYcut-Ycut);
         }
      }
   } 
               /* **************************************** */
               /* The ROTATION does all the points at once */
   if (eventOptions->geometry == ROTATION)
   {
      if (SZ_factor < 1.0)
      {
         rrot=SZ_factor*rot*3.1415927/180.0;
         irotate(mat_pos_sz,1.0,0.0,0.0,rrot);                                                 
         irotate(mat_neg_sz,1.0,0.0,0.0,-rrot);                                                 
         if (sign == -1)
            matmlt(mat_pos_sz,grots,spots,m);                                   
         else if (sign == 1)
            matmlt(mat_neg_sz,grots,spots,m);
      }
      else
      {
         if (sign == -1)
            matmlt(mat_pos,grots,spots,m);                                   
         else if (sign == 1)
            matmlt(mat_neg,grots,spots,m);
      }
      if (sign != 0)
         mat_copy(grots,spots,m);                                   
   }
                                                                         
   matmlt(revers,grots,spots,9);                                         
   ninead(spots, 1.0, eventOptions->positionX, eventOptions->positionY,
                                               eventOptions->positionZ);
   return (distmt);
}


double dyke (spots, eventOptions)
double spots[10][4];   /* array for orientation calculation */
DYKE_OPTIONS *eventOptions;
{
   double distmt;
   int j;
   double deltaz,deltay,tempP,rad;
   double forwrd[4][4], revers[4][4];
   double grots[10][4];
   double slip = eventOptions->slipLength;
   double width = eventOptions->width;
   double pitch = eventOptions->slipPitch;

   rad = atan(1.0)/45.0;
   tempP = (-pitch) + 90.0;
   tempP*=rad;
   deltay = cos(tempP) * slip;
   deltaz = sin(tempP) * slip;

   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);

   ninead(spots, -1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);                                                   
   matmlt(forwrd,spots,grots,9);
   
               /* work out distance to edges */
   if (eventOptions->type == DILATION_DYKE)
      distmt = grots[5][1]-width;
   else  /* STOPE */
   {            
      if (grots[5][1] > width)
         distmt = grots[5][1] - width;
      else if (grots[5][1] < 0.0)
         distmt = grots[5][1];
      else
         distmt = 0.0;  /* inside the dyke */
   }
   
/*   distmt = fabs((width*0.5) - grots[5][1]) - (width*0.5); */
   if ((grots[5][1] <= 0.0) && (eventOptions->type == DILATION_DYKE))
   {
      for (j = 1; j <= 9; j++)
      {
         grots[j][1] = grots[j][1] - width; 
         grots[j][2] = grots[j][2] + deltay;
         grots[j][3] = grots[j][3] + deltaz;
      }
   }
   matmlt(revers, grots, spots, 9);
   ninead(spots, 1.0, eventOptions->positionX, eventOptions->positionY,
                                               eventOptions->positionZ);

   return (distmt);
}

void rot(spots, x, y, z, rotation, eventOptions)
double spots[10][4];   /* array for orientation calculation */
double x, y, z;
double rotation[3][3];
TILT_OPTIONS *eventOptions;  /* NULL if not a tilt event */
{
   double t1, t2;
   int j, k;
   double axis[2][4];
   double forwrd[4][4];
   double grots[10][4];

   if (eventOptions)    /* TILT */
   {                                            
      t1 = eventOptions->plunge;
      t2 = eventOptions->plungeDirection;
      paxis(t1,t2,axis);
      irotate(forwrd, axis[1][1], axis[1][2], axis[1][3],                       
                           -eventOptions->rotation * 0.0174532);
   }
   else
         local4x4From3x3(rotation, forwrd);

   if (eventOptions)  /* TILT */
      ninead(spots, -1.0, eventOptions->positionX, eventOptions->positionY,
                                                   eventOptions->positionZ);
      
   matmlt(forwrd, spots, grots, 9);                                         
      
   for (j = 1; j <= 9; j++) 
   {
      for (k = 1; k <= 3; k++) 
      {                                                          
         spots[j][k] = grots[j][k];
      }
   }
   if (eventOptions)  /* TILT */
      ninead(spots, 1.0, eventOptions->positionX, eventOptions->positionY,
                                                  eventOptions->positionZ);
}

void kink (spots, eventOptions)
double spots[10][4];   /* array for orientation calculation */
SHEAR_OPTIONS *eventOptions;
{                                                           
   int j;
   double deltaz,deltay,deltaoff,tempP,rad;  
   double forwrd[4][4], revers[4][4];
#ifdef _MPL
   plural double grots[10][4];
   plural double temp,concdist;
#else
   double grots[10][4];
   double temp,concdist;
#endif
   double width = eventOptions->width;
   double slip = -eventOptions->slip;

   rad=atan(1.0)/45.0;
   tempP=(-eventOptions->pitch) + 90.0;
   tempP*=rad;
   deltay=cos(tempP)*slip;
   deltaz=sin(tempP)*slip;
   deltaoff=2*width;

   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);

   ninead(spots, -1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);
   matmlt(forwrd,spots,grots,9);
   if (eventOptions->geometry == RING)
   {
      for(j=1;j<=9;j++)
      {
#ifdef _MPL
         concdist=p_sqrt((grots[j][2]*grots[j][2])+(grots[j][3]*grots[j][3]));
#else
         concdist=sqrt((grots[j][2]*grots[j][2])+(grots[j][3]*grots[j][3]));
#endif

         if(eventOptions->type == KINK_EVENT)
         {
            temp=concdist;
            if(concdist > width) temp=width;
            grots[j][1]=grots[j][1]-(slip*temp/width);
         }
         else
         {
            temp=concdist*6.283185307/width;
            if(concdist >= width)
               grots[j][1]=grots[j][1]-slip;
            else
            {
               if (eventOptions->type == SHEAR_ZONE_EVENT)
                  temp=(concdist*3.1415927/width)+(1.5*3.1415927);     
               grots[j][1]=grots[j][1]-(slip/2.)-
                     (slip*0.5*fourier(eventOptions->fourierCoeff,temp));
            }
         }
      }
   }
   else
   {
      if (grots[5][1] >= 0.0)
      {
         if (eventOptions->type == KINK_EVENT)
         {
            for (j=1;j<=9;j++)
            {
               temp=grots[j][1];
               if (grots[j][1] > width)
                  temp=width;

               grots[j][2] = grots[j][2]-(deltay*temp/width);       
               grots[j][3] = grots[j][3]-(deltaz*temp/width);       
            }
         }
         else
         {
            temp=grots[5][1]*6.28318/width;
            if (grots[5][1] >= width)
            {
               for (j = 1; j <= 9; j++)
               {
                  grots[j][2]=grots[j][2]-deltay;
                  grots[j][3]=grots[j][3]-deltaz;
               }
            }
            else
            {
               for (j = 1; j <= 9; j++)
               {
                  temp = grots[j][1]*6.28318/width;
                  if (eventOptions->type == SHEAR_ZONE_EVENT)
                     temp = (grots[j][1]*3.1415927/width)
                                    + (1.5*3.1415927);
                  grots[j][2] = grots[j][2]-(deltay/2.)
                    - (deltay*0.5*fourier(eventOptions->fourierCoeff,temp));
                  grots[j][3] = grots[j][3]-(deltaz/2.) 
                    - (deltaz*0.5*fourier(eventOptions->fourierCoeff,temp));
               }
            }
         }
      }
   }
   matmlt(revers,grots,spots,9);
   ninead(spots, 1.0, eventOptions->positionX, eventOptions->positionY,
                                               eventOptions->positionZ);
}


double plug (spots, eventOptions) /* ellipse not added yet */
double spots[10][4];   /* array for orientation calculation */
PLUG_OPTIONS *eventOptions;
{                                                           
   double contac,r,th,ta,tb,tc;
   double forwrd[4][4], revers[4][4];
   double grots[10][4];
   double distmt;     /* orthogonal distance from igneous contact-unused */
   double apic = tan(eventOptions->apicalAngle*0.01745329278);
   double bval = eventOptions->BValue;
   double rad = eventOptions->radius;
   double ae = eventOptions->axisA;
   double be = eventOptions->axisB;
   double ce = eventOptions->axisC;

   if (eventOptions->type == ELLIPSOIDAL_PLUG)
   {
      if (ae == 0.0)
         ta = 0.00000000001;
      else
         ta = ae * ae;
      if (be == 0.0)
         tb=0.00000000001;
      else
         tb = be * be;
      if (ce == 0.0)
         tc=0.00000000001;
      else
         tc = ce * ce;
   } 

   local4x4From3x3(eventOptions->rotationMatrix.forward, forwrd);
   local4x4From3x3(eventOptions->rotationMatrix.reverse, revers);

   ninead(spots, -1.0, eventOptions->positionX, eventOptions->positionY,
                                                eventOptions->positionZ);                                                   
   matmlt(forwrd,spots,grots,9);                                         

   switch (eventOptions->type)
   {
      case (CYLINDRICAL_PLUG):
         contac = rad;
         break;
      case (CONE_PLUG):
         contac = -grots[5][1]*apic;
         break;
      case (PARABOLIC_PLUG):
         contac = sqrt(fabs(grots[5][1]*bval));
         break;
      case (ELLIPSOIDAL_PLUG):
         contac = ((grots[5][1]*grots[5][1])/ta)
                + ((grots[5][2]*grots[5][2])/tc)
                + ((grots[5][3]*grots[5][3])/tb);  /* < 1.0 -> Inside, > 1.0 -> Outside */
         break;
   }

   polcar(grots[5][2], grots[5][3], &r, &th);                                   

   switch (eventOptions->type)
   {
      case (CYLINDRICAL_PLUG):
         r = (grots[5][2]*grots[5][2]) + (grots[5][3]*grots[5][3]);
         r = sqrt(r);
         distmt = r - contac;
         break;
      case (CONE_PLUG): case (PARABOLIC_PLUG):
         if (grots[5][1] <= 0.0)  /* Distance to surface of cone if next to it */
         {
            r = (grots[5][2]*grots[5][2]) + (grots[5][3]*grots[5][3]);
            r = sqrt(r);
                /* project to be perpendicular to the surface of the cone */
            if (eventOptions->type == CONE_PLUG)
               r = cos(eventOptions->apicalAngle*0.01745329278)*r;
            distmt = r - contac;
         }
         else
         {       /* Just the distance from the the point at the top of the cone */
            r = (grots[5][1]*grots[5][1]) + (grots[5][2]*grots[5][2])
                                          + (grots[5][3]*grots[5][3]);
            distmt = sqrt(r);
         }
         break;
      case (ELLIPSOIDAL_PLUG):
         r = (grots[5][1]*grots[5][1]) + (grots[5][2]*grots[5][2])
                                       + (grots[5][3]*grots[5][3]);
         r = sqrt(r);
         distmt = r - contac;
         break;
      default:
         distmt = 1000000.0;
   }

   matmlt(revers, grots, spots, 9);
   ninead(spots, 1.0, eventOptions->positionX, eventOptions->positionY,
                                               eventOptions->positionZ);
   return (distmt);
}


void import (spots, eventOptions)
double spots[10][4];   /* array for orientation calculation */
IMPORT_OPTIONS *eventOptions;
{                                                           

}

void generic (spots, eventOptions)
double spots[10][4];   /* array for orientation calculation */
GENERIC_OPTIONS *eventOptions;
{                                                           

}

void fore (spots, low, high)
double **spots;    /* array for orientation calculation */
int low, high;
{
   OBJECT *p = NULL;
   WINDOW listWindow;
   register int num;
   BOOLEAN stopFound = FALSE;


   DEBUG(printf("\nFORE: low = %d, high = %d",low,high);)
                 /* first count how many events we have */
   listWindow = (WINDOW) getEventDrawingWindow ();
   num = countObjects (listWindow);

   if (high > num)
      high = num;

   if (high <= 0)
      return;

   if (low < 1)
      low = 1;

   num=low;

   while ((!stopFound) && (num < high))
   {
      p = (OBJECT *) nthObject (listWindow, num);

      DEBUG(printf("\nFORE: objectl %d is %d",num,p->shape);)
      switch (p->shape)
      {
         case FOLD:
              {
                 FOLD_OPTIONS *options=(FOLD_OPTIONS *) p->options;

                 if (options->type == BIOT_FOLD)
                    biott (spots, options);
                 else
                    fold ((double **) spots, options);
              }
              break;
         case FAULT:
              {
                 FAULT_OPTIONS *options=(FAULT_OPTIONS *) p->options;

                 fault (spots, options);
              }
              break;
         case UNCONFORMITY:
              {
                 UNCONFORMITY_OPTIONS *options
                               = (UNCONFORMITY_OPTIONS *) p->options;
              }
              break;
         case SHEAR_ZONE:
              {
                 SHEAR_OPTIONS *options=(SHEAR_OPTIONS *) p->options;

                 fault (spots, (FAULT_OPTIONS *) options);
              }
              break;
         case DYKE:
              {
                 DYKE_OPTIONS *options=(DYKE_OPTIONS *) p->options;

                 dyke (spots, options);
              }
              break;
         case PLUG:
              {
                 PLUG_OPTIONS *options=(PLUG_OPTIONS *) p->options;

                 plug (spots, options);
              }
              break;
         case STRAIN:
              {
                 STRAIN_OPTIONS *options=(STRAIN_OPTIONS *) p->options;

                 rot (spots, 0.0, 0.0, 0.0, options->inverseTensor, NULL);
              }
              break;
         case TILT:
              {
                 TILT_OPTIONS* options=(TILT_OPTIONS *) p->options;

                 rot (spots, options->positionX, options->positionY,
                             options->positionZ, options->rotationMatrix,
                             options);
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
         case GENERIC:
              {
                 GENERIC_OPTIONS *options
                            =(GENERIC_OPTIONS *) p->options;

                 generic (spots, options);
              }
              break;
         case STOP:
              stopFound = TRUE;
              break;
      }
      num=num+1;
   }
}

int
#if XVT_CC_PROTO
forwardModel (int startEvent, int endEvent,
              double xLoc,     double yLoc,     double zLoc,
              double *newXLoc, double *newYLoc, double *newZLoc)
#else
forwardModel (startEvent, endEvent, xLoc, yLoc, zLoc, newXLoc, newYLoc, newZLoc)
int startEvent, endEvent;
double xLoc, yLoc, zLoc;
double *newXLoc, *newYLoc, *newZLoc;
#endif
{
   double points[10][4];   /* array for orientation calculation */
   int i, j;
   
   for (i = 0; i < 10; i++)
      for (j = 0; j < 4; j++)
         points[i][j] = 0.0;
         
   points[5][1] = xLoc;
   points[5][2] = yLoc;
   points[5][3] = zLoc;
   
   fore ((double **) points, startEvent, endEvent);
   
   *newXLoc = points[5][1];
   *newYLoc = points[5][2];
   *newZLoc = points[5][3];

   return (TRUE);
}


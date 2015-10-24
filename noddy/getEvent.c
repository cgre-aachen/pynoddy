#include <math.h>
#ifndef _MPL
#include "xvt.h"
#endif
#include "noddy.h"

#define DEBUG(X)  

#if XVT_CC_PROTO
int splitmods (char *, char *, char *);
int loadStratigraphy (FILE *, double, STRATIGRAPHY_OPTIONS *);
int loadProperties (FILE *, double, LAYER_PROPERTIES *);
int loadColor (FILE *, double, NODDY_COLOUR *);
#else
int splitmods ();
int loadStratigraphy ();
int loadProperties ();
int loadColor ();
#endif

int load_folds (in, INDEX, modifiers, version, options)
FILE *in;
int INDEX;
char *modifiers;
double version;
FOLD_OPTIONS *options;
{
   int i;
   double dum;
   char temp[225];

   if(equalstring2(modifiers,"(Sinusoidal)"))
   {
      options->type = SINE_FOLD;
      initSineFourierCoeff (options->fourierCoeff);
   }
   else
   {   
      options->type = FOURIER_FOLD;
   }

   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->dipDirection);

   loadit(in,"%lf",(void *) &options->axisPitch);
   loadit(in,"%lf",(void *) &options->amplitude);
   loadit(in,"%lf",(void *) &options->wavelength);
   loadit(in,"%lf",(void *) &options->cycle);

   if (version>=1.37)
   {
      loadit(in,"%s",(void *) temp);
      if(equalstring2((char *) temp, "true"))
         options->singleFold = TRUE;
      else
         options->singleFold = FALSE;
   }
   else
      options->singleFold = FALSE;

   if(options->type != SINE_FOLD)
   {
      loadit(in, "",(void *) &dum);
      for(i = 0; i < 11; i++)
      {
         loadit(in,"%lf",(void *) &(options->fourierCoeff[0][i]));
         loadit(in,"%lf",(void *) &(options->fourierCoeff[1][i]));
      }
   }
   convrt(options->dip, options->dipDirection, options->axisPitch, TRUE);
   rotset(options->dip, options->dipDirection, options->axisPitch,
                                           options->rotationMatrix.forward,
                                           options->rotationMatrix.reverse);
   return (TRUE);
}

int load_dykes(in, INDEX, modifiers, version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
DYKE_OPTIONS *options;
{
   int cindex;
   double value, mbase;
   double dipDirection;
        
   if(equalstring2(modifiers,"(Dilate)"))
      options->type = DILATION_DYKE;
   else
      options->type = STOPE_DYKE;
   
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->dipDirection);
   
   loadit(in,"%lf",(void *) &options->slipPitch);
   loadit(in,"%lf",(void *) &options->width);

   if (options->type == DILATION_DYKE)
      loadit(in,"%lf",(void *) &options->slipLength);
   
   copyit(in, (char *) "%s", (void *) options->properties.unitName);
   loadit(in,"%lf",(void *) &options->properties.density);
   loadit(in,"%le",(void *) &options->properties.sus_X);
   if (version >= 2.0)
   {
      loadit(in,"%le",(void *) &options->properties.sus_Y);
      loadit(in,"%le",(void *) &options->properties.sus_Z);
      loadit(in,"%lf",(void *) &value);
      options->properties.anisotropicField = (int) floor (value);

      loadit(in,"%lf",(void *) &value);
      options->properties.remanentMagnetization = (int) floor (value);
      loadit(in,"%le",(void *) &options->properties.strength);
      loadit(in,"%le",(void *) &value);
      loadit(in,"%le",(void *) &value);
      loadit(in,"%lf",(void *) &options->properties.inclination);
      loadit(in,"%lf",(void *) &options->properties.angleWithNorth);
   }
   else
   {  
      options->properties.sus_Y = 0.0;
      options->properties.sus_Z = 0.0;
      options->properties.anisotropicField = FALSE;

      options->properties.remanentMagnetization = FALSE;
      options->properties.strength = 0.0;
      options->properties.inclination = 0.0;
      options->properties.angleWithNorth = 0.0;
   }
   loadit(in,"%d",(void *) &cindex);
   loadit(in,"%lf",(void *) &mbase);

   if (options->dip == 90.0)
      dipDirection = options->dipDirection + 180.0;
   else
      dipDirection = options->dipDirection;
        
   if(options->type == STOPE_DYKE)
   {  
      options->slipPitch = 0;
   }
 
   convrt(options->dip, dipDirection, 90.0, TRUE);
   rotset(options->dip, dipDirection, 90.0, options->rotationMatrix.forward,
                                            options->rotationMatrix.reverse);
   return (TRUE);
}

int load_faults(in,INDEX,modifiers,version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
FAULT_OPTIONS *options;
{
   double pitch, dip;

   options->type = FAULT_EVENT;        
   if(equalstring2(modifiers,"(Translation)"))
   {
      options->geometry = TRANSLATION;
   }
   else if (equalstring2(modifiers,"(Rotation)"))
   {
      options->geometry = ROTATION;
   }
   else
   {
      options->geometry = ELLIPTICAL;
   }

   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->pitch);

   if (options->geometry == ROTATION)
   {
      loadit(in,"%lf",(void *) &options->rotation);
   }
   else
      loadit(in,"%lf",(void *) &options->slip);

   if (options->geometry == ELLIPTICAL)
   {
      loadit(in,"%lf",(void *) &options->xAxis);
      loadit(in,"%lf",(void *) &options->yAxis);
      loadit(in,"%lf",(void *) &options->zAxis);
   }

   if (options->geometry == ROTATION)
      pitch = 0.0;
   else
      pitch = options->pitch;

   if (options->dip == 90.0)
      dip = 89.999;
   else
      dip = options->dip;
      
   convrt(dip, options->dipDirection, pitch, TRUE);
   rotset(dip, options->dipDirection, pitch,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);

                   /* setup the four co-efficients for a sine wave */
   initSineFourierCoeff (options->fourierCoeff);
   return (TRUE);
}

int load_kinks (in, INDEX, modifiers, version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
SHEAR_OPTIONS *options;
{
   int i;
   double tSLIP;
   char mod1[255],mod2[255];
   double dip, pitch;
      
   for (i=0;i<255;i++)
   {
      mod1[i]=0;  
      mod2[i]=0;
   }

   splitmods(modifiers,(char *) mod1,(char *) mod2);
        
   if(equalstring3(mod1,"(Concentric)"))
      options->geometry = RING;
   else
      options->geometry = TRANSLATION;
        
   if(equalstring2(mod2,"(Shear_Zone)"))
      options->type = SHEAR_ZONE_EVENT;
   else
      options->type = KINK_EVENT;
        
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->pitch);
 
   loadit(in,"%lf",(void *) &options->width);
   loadit(in,"%lf",(void *) &tSLIP);
/*   options->slipLength = -tSLIP; .*  !? *.  Dont know what this was for */
   options->slip = tSLIP; 
    
   if (options->geometry == ROTATION)
      pitch = 0.0;
   else
      pitch = options->pitch;

   if (options->dip == 90.0)
      dip = 89.999;
   else
      dip = options->dip;
      
   convrt(dip, options->dipDirection, pitch, TRUE);
   rotset(dip, options->dipDirection, pitch,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);
   return (TRUE);
}

int splitmods (mod0, mod1, mod2)
char *mod0, *mod1, *mod2;
{
   int i=1,k=0;
 
   mod1[0] = mod0[0];
 
   while ((mod0[i] != '(') && (mod0[i] != '\n'))
   {
      mod1[i]=mod0[i];
      i++;
   }
   mod1[i+1]='\0';

   while ((mod0[i] != ')') && (mod0[i] != '\n'))
   {
      mod2[k++]=mod0[i++];
   }
   strcat(mod2,")");
   return (TRUE);
}

int load_lineations (in, INDEX, modifiers, version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
LINEATION_OPTIONS *options;
{
   loadit(in,"%lf",(void *) &options->plunge);
   loadit(in,"%lf",(void *) &options->plungeDirection);
 
   convrt(90.0-options->plunge, 180.0+options->plungeDirection, 0.0, TRUE);
   rotset(90.0-options->plunge, 180.0+options->plungeDirection, 0.0,
                                           options->rotationMatrix.forward,
                                           options->rotationMatrix.reverse);
   return (TRUE);
}

int load_planes (in, INDEX, modifiers, version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
FOLIATION_OPTIONS *options;
{
   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->dipDirection);
 
   convrt(options->dip, options->dipDirection, 0.0, TRUE);
   rotset(options->dip, options->dipDirection, 0.0,
                                           options->rotationMatrix.forward,
                                           options->rotationMatrix.reverse);
   return (TRUE);
}

int load_plugs (in, INDEX, modifiers, version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
PLUG_OPTIONS *options;
{
   double value, mbase;
   int cindex;
   double pitch;
   
   if (equalstring2(modifiers,"(Cone)"))
      options->type = CONE_PLUG;
   else if (equalstring2(modifiers,"(Cylinder)"))
      options->type = CYLINDRICAL_PLUG;
   else if (equalstring2(modifiers,"(Paraboloid)"))
      options->type = PARABOLIC_PLUG;
   else
      options->type = ELLIPSOIDAL_PLUG;

   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->axisPitch);

   if (options->type == CYLINDRICAL_PLUG)
   {
      loadit(in,"%lf",(void *) &options->radius);
   }
   else if (options->type == CONE_PLUG)
   {
      loadit(in,"%lf",(void *) &options->apicalAngle);
   }
   else if (options->type == PARABOLIC_PLUG)
   {
      loadit(in,"%lf",(void *) &options->BValue);
   }
   else
   {
      loadit(in,"%lf",(void *) &options->axisB);
      loadit(in,"%lf",(void *) &options->axisC);
      loadit(in,"%lf",(void *) &options->axisA);
   }               

   if (options->type != ELLIPSOIDAL_PLUG)
      pitch = 0.0;
   else
      pitch = options->axisPitch;

   copyit(in, (char *) "%s", (void *) options->properties.unitName);
   loadit(in,"%lf",(void *) &options->properties.density);
   loadit(in,"%le",(void *) &options->properties.sus_X);
   if(version>=2.0)
   {
      loadit(in,"%le",(void *) &options->properties.sus_Y);
      loadit(in,"%le",(void *) &options->properties.sus_Z);
      loadit(in,"%lf",(void *) &value);
      options->properties.anisotropicField = (int) floor (value);

      loadit(in,"%lf",(void *) &value);
      options->properties.remanentMagnetization = (int) floor (value);
      loadit(in,"%le",(void *) &options->properties.strength);
      loadit(in,"%le",(void *) &value);
      loadit(in,"%le",(void *) &value);
      loadit(in,"%lf",(void *) &options->properties.inclination);
      loadit(in,"%lf",(void *) &options->properties.angleWithNorth);
   }
   else
   {
       options->properties.sus_Y = 0.0;
       options->properties.sus_Z = 0.0;
       options->properties.anisotropicField = FALSE; 

       options->properties.remanentMagnetization = FALSE;
       options->properties.strength = 0.0;
       options->properties.inclination = 0.0;
       options->properties.angleWithNorth = 0.0;
   }
   loadit(in,"%d",(void *) &cindex);
   loadit(in,"%lf",(void *) &mbase);

   if (version < 6.12)  /* Added the +180 in calc so must take it off for old history files */
	{
		options->dip = options->dip + 90.0;  /* Added -90 so we need to add it for old histories */
	}

	convrt(options->dip-90.0, options->dipDirection, pitch, TRUE);
   rotset(options->dip-90.0, options->dipDirection, pitch,
                                       options->rotationMatrix.forward,
                                       options->rotationMatrix.reverse);
   return (TRUE);
}

int load_tilts (in, INDEX, modifiers, version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
TILT_OPTIONS *options;
{
   int i, j;
   double angle;
   double axis[2][4], mata[4][4];

        
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->plunge);
   loadit(in,"%lf",(void *) &options->plungeDirection);
           
   loadit(in,"%lf",(void *) &options->rotation);

   angle = options->rotation * 0.0174532;
   paxis (options->plunge, options->plungeDirection, axis);
   irotate(mata, axis[1][1], axis[1][2], axis[1][3], angle);

   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         options->rotationMatrix[i][j] = mata[i+1][j+1];
      }
   }
   return (TRUE);
}

int load_strains (in, INDEX, modifiers, version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
STRAIN_OPTIONS *options;
{
   int i, j;
   double tensor[4][4], inverseTensor[4][4];

   loadit(in,"%lf",(void *) &options->tensor[0][0]);
   loadit(in,"%lf",(void *) &options->tensor[1][0]);
   loadit(in,"%lf",(void *) &options->tensor[2][0]);
   loadit(in,"%lf",(void *) &options->tensor[0][1]);
   loadit(in,"%lf",(void *) &options->tensor[1][1]);
   loadit(in,"%lf",(void *) &options->tensor[2][1]);
   loadit(in,"%lf",(void *) &options->tensor[0][2]);
   loadit(in,"%lf",(void *) &options->tensor[1][2]);
   loadit(in,"%lf",(void *) &options->tensor[2][2]);
 
   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         tensor[j+1][i+1] = options->tensor[i][j];
      }
   }
   if (!f01aaf(tensor, inverseTensor)) /* matrix inversion */
      return (FALSE);
   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         options->inverseTensor[i][j] = inverseTensor[i+1][j+1];
      }
   }
   return (TRUE);
}      

int load_unconformities(in, INDEX, modifiers, version, options)
FILE *in;
int INDEX; 
char *modifiers; 
double version;
UNCONFORMITY_OPTIONS *options;
{
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->dipDirection);

   convrt(options->dip, options->dipDirection, 0.0, TRUE);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   rotset(options->dip, options->dipDirection, 0.0,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);
   return (TRUE);
}


int loadFold (in, version, options)
FILE *in;
double version;
FOLD_OPTIONS *options;
{
   int i;
   char temp[225];

   loadit(in,"%s",(void *) temp);
   if (strcmp (temp, "Sine") == 0)
      options->type = SINE_FOLD;
   else if (strcmp (temp, "Biot") == 0)
      options->type = BIOT_FOLD;
   else
      options->type = FOURIER_FOLD;

   loadit(in,"%s",(void *) temp);
   if (strcmp (temp, "TRUE") == 0)
      options->singleFold = TRUE;
   else
      options->singleFold = FALSE;

   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->axisPitch);

   loadit(in,"%lf",(void *) &options->wavelength);
   loadit(in,"%lf",(void *) &options->amplitude);
   loadit(in,"%lf",(void *) &options->cycle);

   loadit(in,"%s",(void *) temp);
   for (i = 0; i < 11; i++)
   {
      loadit(in,"%lf",(void *) &options->fourierCoeff[0][i]);
      loadit(in,"%lf",(void *) &options->fourierCoeff[1][i]);
   }
   
   if (version > 4.05)
   {
      if (!loadProfile (in, version, &(options->profile)))
         return (FALSE);
   }
   else  /* Convert fourier Coeff into approximation of profile in points */
   {
      if (!convertFourierToProfile (options->fourierCoeff, &(options->profile)))
         return (FALSE);
   }
   
   


   convrt(options->dip, options->dipDirection, options->axisPitch, TRUE);
   rotset(options->dip, options->dipDirection, options->axisPitch,
                                           options->rotationMatrix.forward,
                                           options->rotationMatrix.reverse);
   return (TRUE);
}

int loadDyke (in, version, options)
FILE *in;
double version;
DYKE_OPTIONS *options;
{
   double dipDirection;
   char temp[100], strVal[100];

   loadit(in,"%s",(void *) temp);
   if (strcmp (temp, "Dilate") == 0)
      options->type = DILATION_DYKE;
   else
      options->type = STOPE_DYKE;

   loadit(in,"%d",(void *) &options->mergeEvents);
        
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->slipPitch);

   loadit(in,"%lf",(void *) &options->slipLength);
   loadit(in,"%lf",(void *) &options->width);

   if (version > 4.05)
   {
      loadit(in,"%s",(void *) strVal);
      if (strstr (strVal, "NONE"))
         options->alterationZones = NONE_ALTERATION;
      else if (strstr (strVal, "TOP"))
         options->alterationZones = TOP_ALTERATION;
      else if (strstr (strVal, "BOTTOM"))
         options->alterationZones = BOTTOM_ALTERATION;
      else /* BOTH */
         options->alterationZones = BOTH_ALTERATION;
      if (!loadAlteration (in, version, &(options->alterationFuntions)))
         return (FALSE);
   }

   loadProperties(in, version, &(options->properties));
   
   if (options->dip == 90.0)
      dipDirection = options->dipDirection + 180.0;
   else
      dipDirection = options->dipDirection;
        
   convrt(options->dip, dipDirection, 90.0, TRUE);
   rotset(options->dip, dipDirection, 90.0, options->rotationMatrix.forward,
                                            options->rotationMatrix.reverse);
   return (TRUE);
}

int loadFault (in, version, options)
FILE *in;
double version;
FAULT_OPTIONS *options;
{
   int i;
   double pitch, dip;
   char temp[100], strVal[100];

   options->type = FAULT_EVENT;
   
   loadit(in,"%s",(void *) temp);
   if (strcmp (temp, "Translation") == 0)
      options->geometry = TRANSLATION;
   else if (strcmp (temp, "Rotation") == 0)
      options->geometry = ROTATION;
   else if (strcmp (temp, "Elliptical") == 0)
      options->geometry = ELLIPTICAL;
   else if (strcmp (temp, "Curved") == 0)
      options->geometry = CURVED;
   else
      options->geometry = RING;

   if (version > 3.4)
   {
      loadit(in,"%s",(void *) temp);
      if (strcmp (temp, "Hanging Wall") == 0)
         options->movement = HANGING_WALL;
      else if (strcmp (temp, "Foot Wall") == 0)
         options->movement = FOOT_WALL;
      else
         options->movement = BOTH;
   }
        
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->pitch);

   loadit(in,"%lf",(void *) &options->slip);
   loadit(in,"%lf",(void *) &options->rotation);
   if (version > 3.4)
   {
      loadit(in,"%lf",(void *) &options->amplitude);
      loadit(in,"%lf",(void *) &options->radius);
   }
   loadit(in,"%lf",(void *) &options->xAxis);
   loadit(in,"%lf",(void *) &options->yAxis);
   loadit(in,"%lf",(void *) &options->zAxis);
   if (version > 3.4)
   {
      loadit(in,"%lf",(void *) &options->cylindricalIndex);
      loadit(in,"%lf",(void *) &options->profilePitch);
   }

   loadColor (in, version, &(options->color));

   loadit(in,"%s",(void *) temp);
   for (i = 0; i < 11; i++)
   {
      loadit(in,"%lf",(void *) &options->fourierCoeff[0][i]);
      loadit(in,"%lf",(void *) &options->fourierCoeff[1][i]);
   }
   
   if (version > 4.05)
   {
      if (!loadProfile (in, version, &(options->profile)))
         return (FALSE);
         
      loadit(in,"%s",(void *) strVal);
      if (strstr (strVal, "NONE"))
         options->alterationZones = NONE_ALTERATION;
      else if (strstr (strVal, "TOP"))
         options->alterationZones = TOP_ALTERATION;
      else if (strstr (strVal, "BOTTOM"))
         options->alterationZones = BOTTOM_ALTERATION;
      else /* BOTH */
         options->alterationZones = BOTH_ALTERATION;
      if (!loadAlteration (in, version, &(options->alterationFuntions)))
         return (FALSE);
   }
   else  /* Convert fourier Coeff into approximation of profile in points */
   {
      if (!convertFourierToProfile (options->fourierCoeff, &(options->profile)))
         return (FALSE);
   }

   if (version > 5.00)
      if (!loadSurface (in, version, &(options->surfaceType), 
             &(options->surfaceFilename), &(options->surfaceXDim),
             &(options->surfaceYDim), &(options->surfaceZDim), &(options->dxfData)))
         return (FALSE);

   if (options->geometry == ROTATION)
      pitch = 0.0;
   else
      pitch = options->pitch;

   if (options->dip == 90.0)
      dip = 89.999;
   else
      dip = options->dip;
      
   convrt(dip, options->dipDirection, pitch, TRUE);
   rotset(dip, options->dipDirection, pitch,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);
   return (TRUE);
}

int loadShearZone (in, version, options)
FILE *in;
double version;
SHEAR_OPTIONS *options;
{
   int i;
   double dip, pitch;
   char temp[100], strVal[100];

   loadit(in,"%s",(void *) temp);
   if (strcmp (temp, "Shear Zone") == 0)
      options->type = SHEAR_ZONE_EVENT;
   else
      options->type = KINK_EVENT;

   if (version < 3.5)
   {
      int concentric;
      
      loadit(in,"%d",(void *) &concentric);
      if (concentric)
         options->geometry = RING;
      else
         options->geometry = TRANSLATION;
   }
   else
   {

      loadit(in,"%s",(void *) temp);
      if (strcmp (temp, "Translation") == 0)
         options->geometry = TRANSLATION;
      else if (strcmp (temp, "Rotation") == 0)
         options->geometry = ROTATION;
      else if (strcmp (temp, "Elliptical") == 0)
         options->geometry = ELLIPTICAL;
      else if (strcmp (temp, "Curved") == 0)
         options->geometry = CURVED;
      else
         options->geometry = RING;

      loadit(in,"%s",(void *) temp);
      if (strcmp (temp, "Hanging Wall") == 0)
         options->movement = HANGING_WALL;
      else if (strcmp (temp, "Foot Wall") == 0)
         options->movement = FOOT_WALL;
      else
         options->movement = BOTH;
   }
        
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->pitch);

   loadit(in,"%lf",(void *) &options->slip);
   if (version > 3.4)
   {
      loadit(in,"%lf",(void *) &options->rotation);
      loadit(in,"%lf",(void *) &options->amplitude);
   }
   loadit(in,"%lf",(void *) &options->width);
 
   if (version > 3.4)
   {
      loadit(in,"%lf",(void *) &options->radius);
      loadit(in,"%lf",(void *) &options->xAxis);
      loadit(in,"%lf",(void *) &options->yAxis);
      loadit(in,"%lf",(void *) &options->zAxis);
      loadit(in,"%lf",(void *) &options->cylindricalIndex);
      loadit(in,"%lf",(void *) &options->profilePitch);

      loadColor (in, version, &(options->color));
   }

   loadit(in,"%s",(void *) temp);
   for (i = 0; i < 11; i++)
   {
      loadit(in,"%lf",(void *) &options->fourierCoeff[0][i]);
      loadit(in,"%lf",(void *) &options->fourierCoeff[1][i]);
   }

   if (version > 4.05)
   {
      if (!loadProfile (in, version, &(options->profile)))
         return (FALSE);
         
      loadit(in,"%s",(void *) strVal);
      if (strstr (strVal, "NONE"))
         options->alterationZones = NONE_ALTERATION;
      else if (strstr (strVal, "TOP"))
         options->alterationZones = TOP_ALTERATION;
      else if (strstr (strVal, "BOTTOM"))
         options->alterationZones = BOTTOM_ALTERATION;
      else /* BOTH */
         options->alterationZones = BOTH_ALTERATION;
      if (!loadAlteration (in, version, &(options->alterationFuntions)))
         return (FALSE);
   }
   else  /* Convert fourier Coeff into approximation of profile in points */
   {
      if (!convertFourierToProfile (options->fourierCoeff, &(options->profile)))
         return (FALSE);
   }

   if (version > 5.00)
      if (!loadSurface (in, version, &(options->surfaceType), 
             &(options->surfaceFilename), &(options->surfaceXDim),
             &(options->surfaceYDim), &(options->surfaceZDim), &(options->dxfData)))
         return (FALSE);

   if (options->geometry == ROTATION)
      pitch = 0.0;
   else
      pitch = options->pitch;

   if (options->dip == 90.0)
      dip = 89.999;
   else
      dip = options->dip;
      
   convrt(dip, options->dipDirection, pitch, TRUE);
   rotset(dip, options->dipDirection, pitch,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);
   return (TRUE);
}


int loadLineation (in, version, options)
FILE *in;
double version;
LINEATION_OPTIONS *options;
{
   loadit(in,"%lf",(void *) &options->plungeDirection);
   loadit(in,"%lf",(void *) &options->plunge);
 
   convrt(90.0-options->plunge, 180.0+options->plungeDirection, 0.0, TRUE);
   rotset(90.0-options->plunge, 180.0+options->plungeDirection, 0.0,
                                           options->rotationMatrix.forward,
                                           options->rotationMatrix.reverse);
   return (TRUE);
}

int loadFoliation (in, version, options)
FILE *in;
double version;
FOLIATION_OPTIONS *options;
{
   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->dip);
 
   convrt(options->dip, options->dipDirection, 0.0, TRUE);
   rotset(options->dip, options->dipDirection, 0.0,
                                           options->rotationMatrix.forward,
                                           options->rotationMatrix.reverse);
   return (TRUE);
}

int loadPlug (in, version, options)
FILE *in;
double version;
PLUG_OPTIONS *options;
{
   double pitch;
   char temp[100], strVal[100];
   
   loadit(in,"%s",(void *) temp);
   if (strcmp (temp, "Cylindrical") == 0)
      options->type = CYLINDRICAL_PLUG;
   else if (strcmp (temp, "Cone") == 0)
      options->type = CONE_PLUG;
   else if (strcmp (temp, "Parabolic") == 0)
      options->type = PARABOLIC_PLUG;
   else
      options->type = ELLIPSOIDAL_PLUG;

   loadit(in,"%d",(void *) &options->mergeEvents);

   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->dip);
   loadit(in,"%lf",(void *) &options->axisPitch);

   loadit(in,"%lf",(void *) &options->radius);
   loadit(in,"%lf",(void *) &options->apicalAngle);
   loadit(in,"%lf",(void *) &options->BValue);
   loadit(in,"%lf",(void *) &options->axisA);
   loadit(in,"%lf",(void *) &options->axisB);
   loadit(in,"%lf",(void *) &options->axisC);

   if (options->type != ELLIPSOIDAL_PLUG)
      pitch = 0.0;
   else
      pitch = options->axisPitch;
      
   if (version > 4.05)
   {
      loadit(in,"%s",(void *) strVal);
      if (strstr (strVal, "NONE"))
         options->alterationZones = NONE_ALTERATION;
      else if (strstr (strVal, "TOP"))
         options->alterationZones = TOP_ALTERATION;
      else if (strstr (strVal, "BOTTOM"))
         options->alterationZones = BOTTOM_ALTERATION;
      else /* BOTH */
         options->alterationZones = BOTH_ALTERATION;
      if (!loadAlteration (in, version, &(options->alterationFuntions)))
         return (FALSE);
   }

   loadProperties (in, version, &(options->properties));

   if (version < 6.12)  /* Added the +180 in calc so must take it off for old history files */
	{
		options->dip = options->dip + 90.0;  /* Added -90 so we need to add it for old histories */
	}

   convrt(options->dip-90.0, options->dipDirection, pitch, TRUE);
   rotset(options->dip-90.0, options->dipDirection, pitch,
                                       options->rotationMatrix.forward,
                                       options->rotationMatrix.reverse);
   return (TRUE);
}

int loadTilt (in, version, options)
FILE *in;
double version;
TILT_OPTIONS *options;
{
   int i, j;
   double angle;
   double axis[2][4], mata[4][4];

        
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->rotation);
   loadit(in,"%lf",(void *) &options->plungeDirection);
   loadit(in,"%lf",(void *) &options->plunge);
           
   angle = options->rotation * 0.0174532;
   paxis (options->plunge, options->plungeDirection, axis);
   irotate(mata, axis[1][1], axis[1][2], axis[1][3], angle);

   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         options->rotationMatrix[i][j] = mata[i+1][j+1];
      }
   }
   return (TRUE);
}

int loadStrain (in, version, options)
FILE *in;
double version;
STRAIN_OPTIONS *options;
{
   int i, j;
   double tensor[4][4], inverseTensor[4][4];

   loadit(in,"%lf",(void *) &options->tensor[0][0]);
   loadit(in,"%lf",(void *) &options->tensor[1][0]);
   loadit(in,"%lf",(void *) &options->tensor[2][0]);
   loadit(in,"%lf",(void *) &options->tensor[0][1]);
   loadit(in,"%lf",(void *) &options->tensor[1][1]);
   loadit(in,"%lf",(void *) &options->tensor[2][1]);
   loadit(in,"%lf",(void *) &options->tensor[0][2]);
   loadit(in,"%lf",(void *) &options->tensor[1][2]);
   loadit(in,"%lf",(void *) &options->tensor[2][2]);
 
   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         tensor[j+1][i+1] = options->tensor[i][j];
      }
   }
   if (!f01aaf(tensor, inverseTensor)) /* matrix inversion */
      return (FALSE);
   for (i = 0; i < 3; i++)
   {
      for (j = 0; j < 3; j++)
      {
         options->inverseTensor[i][j] = inverseTensor[i+1][j+1];
      }
   }
   return (TRUE);
}      

int loadImport (in, version, options)
FILE *in;
double version;
IMPORT_OPTIONS *options;
{
   int i;
   char temp[225];
   
   loadit(in,"%s",(void *) temp);
   strcpy (options->importFile.name, temp);
   loadit(in,"%s",(void *) temp);
   xvt_fsys_convert_str_to_dir (temp, &(options->importFile.dir));
   if (findFile (&(options->importFile)))
      loadImportFile (options);
   else
      strcpy (options->importFile.name, "");
   
   
   loadit(in,"%s",(void *) temp);
   if (strcmp (temp, "Center") == 0)
      options->positionBy = POS_BY_CENTER;
   else
      options->positionBy = POS_BY_CORNER;
      
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->blockSize);

   loadit(in,"%s",(void *) temp);
   if (strcmp (temp, "Noddy") == 0)
      options->fileType = NODDY_BLOCK_MODEL;
   else
      options->fileType = NODDY_BLOCK_MODEL;

   loadit(in,"%d",(void *) &options->numProperties);
   if (options->properties)  /* Get rid of properties loaded with initial Block */
   {
      xvt_mem_free ((char *) options->properties);
      options->properties = NULL;
   }
   if ((options->blockData) &&
       (options->properties=(LAYER_PROPERTIES *) xvt_mem_alloc(options->numProperties
                                                         * sizeof(LAYER_PROPERTIES))))
   {
      for (i = 0; i < options->numProperties; i++)
      {
         loadProperties (in, version, &(options->properties[i]));
      }
   }
   else
   {              /* Just junk the properties we are reading */
      LAYER_PROPERTIES junkProperties;
      
      for (i = 0; i < options->numProperties; i++)
      {
         loadProperties (in, version, &junkProperties);
      }
      options->numProperties = 0;
   }

   return (TRUE);
}

int loadGeneric (in, version, options)
FILE *in;
double version;
GENERIC_OPTIONS *options;
{
   loadit(in,"%lf",(void *) &options->param1);
   loadit(in,"%lf",(void *) &options->param2);
   loadit(in,"%lf",(void *) &options->param3);
   loadit(in,"%lf",(void *) &options->param4);
   loadit(in,"%lf",(void *) &options->param5);
   loadit(in,"%lf",(void *) &options->param6);
   loadit(in,"%lf",(void *) &options->param7);
   loadit(in,"%lf",(void *) &options->param8);
   loadit(in,"%lf",(void *) &options->param9);
   loadit(in,"%lf",(void *) &options->param10);

   return (TRUE);
}

int loadUnconformity (in, version, options)
FILE *in;
double version;
UNCONFORMITY_OPTIONS *options;
{
   char strVal[100];
   
   loadit(in,"%lf",(void *) &options->positionX);
   loadit(in,"%lf",(void *) &options->positionY);
   loadit(in,"%lf",(void *) &options->positionZ);

   loadit(in,"%lf",(void *) &options->dipDirection);
   loadit(in,"%lf",(void *) &options->dip);

   if (version > 4.05)
   {
      loadit(in,"%s",(void *) strVal);
      if (strstr (strVal, "NONE"))
         options->alterationZones = NONE_ALTERATION;
      else if (strstr (strVal, "TOP"))
         options->alterationZones = TOP_ALTERATION;
      else if (strstr (strVal, "BOTTOM"))
         options->alterationZones = BOTTOM_ALTERATION;
      else /* BOTH */
         options->alterationZones = BOTH_ALTERATION;
      if (!loadAlteration (in, version, &(options->alterationFuntions)))
         return (FALSE);
   }

   if (version > 5.00)
      if (!loadSurface (in, version, &(options->surfaceType), 
             &(options->surfaceFilename), &(options->surfaceXDim),
             &(options->surfaceYDim), &(options->surfaceZDim), &(options->dxfData)))
         return (FALSE);

   loadStratigraphy (in, version, &(options->stratigraphyDetails));

   convrt(options->dip, options->dipDirection, 0.0, TRUE);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   rotset(options->dip, options->dipDirection, 0.0,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);

   return (TRUE);
}



int loadStratigraphy (in, version, options)
FILE *in;
double version;
STRATIGRAPHY_OPTIONS *options;
{
   int i;

   loadit(in,"%d",(void *) &options->numLayers);
   if (options->properties)
      xvt_mem_free ((char *) options->properties);
   if (!(options->properties = (LAYER_PROPERTIES *) xvt_mem_zalloc (options->numLayers
                                                          *sizeof(LAYER_PROPERTIES))))
      return (FALSE);
      
   for (i = 0; i < options->numLayers; i++)
   {
      loadProperties (in, version, &(options->properties[i]));
   }
   
   if (version < 5.0)
   {     /* Older Noddy versions always wrote out 7 layers - get rid of extra */
      LAYER_PROPERTIES dudLayer;
      if (options->numLayers < 7)
      {
         for (i = options->numLayers; i < 7; i++)
         {
            loadProperties (in, version, &dudLayer);
         }
      }
   }

   return (TRUE);
}


int loadProperties (in, version, options)
FILE *in;
double version;
LAYER_PROPERTIES *options;
{
   double value;
   char label[50];
   
   loadit(in,"%s",(void *) options->unitName);
   loadit(in,"%d",(void *) &options->height);
   if (version > 5.1)
   {
      loadit(in,"%s",(void *) label);
      if (strcmp (label, "OFF") == 0)
         options->applyAlterations = FALSE;
      else
         options->applyAlterations = TRUE;
   }
   else
      options->applyAlterations = TRUE;

   loadit(in,"%lf",(void *) &options->density);

   loadit(in,"%d",(void *) &options->anisotropicField);
   loadit(in,"%le",(void *) &options->sus_X);
   loadit(in,"%le",(void *) &options->sus_Y);
   loadit(in,"%le",(void *) &options->sus_Z);
   if (version > 3.5)
   {
      loadit(in,"%le",(void *) &options->sus_dip);
      loadit(in,"%le",(void *) &options->sus_dipDirection);
      loadit(in,"%le",(void *) &options->sus_pitch);
   }

   loadit(in,"%d",(void *) &options->remanentMagnetization);
   loadit(in,"%lf",(void *) &options->inclination);
   loadit(in,"%lf",(void *) &options->angleWithNorth);
   loadit(in,"%le",(void *) &options->strength);
   if (version < 3.6)
   {
      loadit(in,"%le",(void *) &value);  /* old kon_Y */
      loadit(in,"%le",(void *) &value);  /* old kon_Z */
   }

   loadColor (in, version, &(options->color));
   
                          /* calculate and store the forward and
                          ** reverse rotation matricies for remenants */
   convrt(90.0 - options->inclination, 180.0 + options->angleWithNorth,
                                       0.0, TRUE);
   rotset(90.0 - options->inclination, 180.0 + options->angleWithNorth,
                               0.0, options->remRotationMatrix.forward,
                                    options->remRotationMatrix.reverse);

   convrt(options->sus_dip, options->sus_dipDirection,
                            options->sus_pitch, TRUE);
   rotset(options->sus_dip, options->sus_dipDirection, options->sus_pitch,
                            options->aniRotationMatrix.forward,
                            options->aniRotationMatrix.reverse);
   return (TRUE);
}

int loadColor (in, version, options)
FILE *in;
double version;
NODDY_COLOUR *options;
{
   int red, green, blue;
   
   loadit(in,"%s",(void *) options->name);
   loadit(in,"%d",(void *) &red);
   loadit(in,"%d",(void *) &green);
   loadit(in,"%d",(void *) &blue);
   
   options->red = (unsigned char) red;
   options->green = (unsigned char) green;
   options->blue = (unsigned char) blue;
   
   return (TRUE);
}

int loadAlteration (in, version, options)
FILE *in;
double version;
PROFILE_OPTIONS **options;
{
   int numProfiles, profile;
   PROFILE_OPTIONS *profOptions = NULL,
                   *lastProfOptions = NULL,
                   *nextProfOptions = NULL;

   if (*options)  /* Free any alteration already loaded */
   {
      for (profOptions = (*options); profOptions; profOptions = nextProfOptions)
      {
         nextProfOptions = profOptions->next;
         profilePointsFree (profOptions->points);
         freeArrayForProfile (profOptions->array);
         xvt_mem_free ((char *) profOptions);
      }
   }
   
   loadit(in,"%d",(void *) &numProfiles);
   for (profile = 0; profile < numProfiles; profile++)
   {
      if (!(profOptions = (PROFILE_OPTIONS *) xvt_mem_zalloc (sizeof (PROFILE_OPTIONS))))
         return (FALSE);
      
      if (!loadProfile (in, version, profOptions))
         return (FALSE);
         
      if (lastProfOptions)
         lastProfOptions->next = profOptions;
      else
         *options = profOptions;
         
      lastProfOptions = profOptions;
   }

   return (TRUE);
}

int loadProfile (in, version, options)
FILE *in;
double version;
PROFILE_OPTIONS *options;
{
   PROFILE_POINT *pointPtr = NULL, *lastPointPtr = NULL;
   int numPoints, intVal, h, v, point;
   char strVal[100];

   loadit(in,"%s",(void *) options->name);
   loadit(in,"%d",(void *) &intVal);
   options->type = intVal;

   loadit(in,"%s",(void *) strVal);
   if (strstr (strVal, "LINES"))
      options->joinType = LINE_SEGMENTS;
   else if (strstr (strVal, "CURVES"))
      options->joinType = CURVE_SEGMENTS;
   else
      options->joinType = SQUARE_SEGMENTS;

   loadit(in,"%lf",(void *) &(options->graphLength));
   loadit(in,"%lf",(void *) &(options->minX));
   loadit(in,"%lf",(void *) &(options->maxX));
   if ((version < 4.11) && (version > 4.09))
   {
      loadit(in,"%lf",(void *) &(options->minYScale));
      loadit(in,"%lf",(void *) &(options->maxYScale));
   }
   else
   {
      loadit(in,"%lf",(void *) &(options->minYScale));
      loadit(in,"%lf",(void *) &(options->maxYScale));
      if (version > 5.00)
         loadit(in,"%lf",(void *) &(options->scaleOrigin));
      loadit(in,"%lf",(void *) &(options->minYReplace));
      loadit(in,"%lf",(void *) &(options->maxYReplace));
   }
      
   loadit(in,"%d",(void *) &numPoints);

                         /* Allocate enough points */
   profilePointsFree (options->points);
   options->points = NULL;
   for (point = 0; point < numPoints; point++)
   {
      if (!(pointPtr = (PROFILE_POINT *) xvt_mem_zalloc(sizeof(PROFILE_POINT))))
         return (FALSE);
         
      if (lastPointPtr)
         lastPointPtr->next = pointPtr; /* make list */
      else
         options->points = pointPtr; /* Store first */
         
      lastPointPtr = pointPtr;
   }

   for (pointPtr = options->points; pointPtr; pointPtr = pointPtr->next)
   {
      loadit(in,"%d",(void *) &h);
      loadit(in,"%d",(void *) &v);
      pointPtr->point.h = (short) h;
      pointPtr->point.v = (short) v;
   }

   options->array = getArrayForProfile (options);
   
   return (TRUE);
}

int
#if XVT_CC_PROTO
convertFourierToProfile (double fourierCoeff[2][11], PROFILE_OPTIONS *profOptions)
#else
convertFourierToProfile (fourierCoeff, profOptions)
double fourierCoeff[2][11];
PROFILE_OPTIONS *profOptions;
#endif
{
   PROFILE_POINT *pointPtr = NULL, *lastPointPtr = NULL;
   int point, numPoints = 20; 
   double value, loc, posInc;
   
   strcpy (profOptions->name, "Profile");
   profOptions->type = PLANE_PROFILE;
   profOptions->joinType = LINE_SEGMENTS;
   profOptions->graphLength = 200.0;
   profOptions->minX = 0.0;
   profOptions->maxX = 6.28;
   profOptions->minYScale = -1.0;
   profOptions->maxYScale = 1.0;
   profOptions->minYReplace = -1.0;
   profOptions->maxYReplace = 1.0;
   
                         /* Allocate enough points */
   profilePointsFree (profOptions->points);
   for (point = 0; point < numPoints; point++)
   {
      if (!(pointPtr = (PROFILE_POINT *) xvt_mem_zalloc(sizeof(PROFILE_POINT))))
         return (FALSE);
         
      if (lastPointPtr)
         lastPointPtr->next = pointPtr; /* make list */
      else
         profOptions->points = pointPtr; /* Store first */
         
      lastPointPtr = pointPtr;
   }
                           /* Same along the wavelength */
   posInc = (double) ((double) X_PROFILE_RESOLUTION)/((double) numPoints-1);  
   for (pointPtr = profOptions->points, point = 0;
                                pointPtr; pointPtr = pointPtr->next, point++)
   {
      loc = floor (point*posInc + 0.5);
      value = fourier (fourierCoeff, (double) (loc/100.0));
      pointPtr->point.h = (short) loc;
      pointPtr->point.v = (short) floor (value*((double) Y_PROFILE_RESOLUTION));
   }
   profOptions->array = getArrayForProfile (profOptions);

   return (TRUE);
}

int loadSurface (in, version, surfaceType, surfaceFilename,
                 surfaceXDim, surfaceYDim, surfaceZDim, dxfData)
FILE *in;
double version;
SURFACE_TYPE *surfaceType;
FILE_SPEC *surfaceFilename;
double *surfaceXDim, *surfaceYDim, *surfaceZDim;
DXFHANDLE *dxfData;
{
   char strVal[100];

   loadit(in,"%s",(void *) strVal);
   if (strcmp (strVal, "DXF_SURFACE") == 0)
      *surfaceType = DXF_SURFACE;
   else if (strcmp (strVal, "UNKNOWN_SURFACE") == 0)
      *surfaceType = UNKNOWN_SURFACE;
   else
      *surfaceType = FLAT_SURFACE;
   loadit(in,"%s",(void *) &strVal);
   strcpy (surfaceFilename->name, strVal);

   loadit(in,"%s",(void *) &strVal);
   xvt_fsys_convert_str_to_dir(strVal, &(surfaceFilename->dir));

   loadit(in,"%lf",(void *) surfaceXDim);
   loadit(in,"%lf",(void *) surfaceYDim);
   loadit(in,"%lf",(void *) surfaceZDim);

   if (*surfaceType == DXF_SURFACE)
   {
      if (!load3dSurface (surfaceFilename, (OBJECT *) NULL, dxfData))
      {
         xvt_dm_post_error ("Error, could not reload dxf File");
         return (FALSE);
      }
   }
   
   return (TRUE);
}




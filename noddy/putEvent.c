
#include "xvt.h"
#include "noddy.h"

                 /* ********************************** */
                 /* External Globals used in this file */

                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
int reportStratigraphy (FILE *, STRATIGRAPHY_OPTIONS *);
int reportProperties (FILE *, LAYER_PROPERTIES *);
int reportColor (FILE *, NODDY_COLOUR *);
#else
int reportStratigraphy ();
int reportProperties ();
int reportColor ();
#endif

int report_folds(stream, options)
FILE *stream;
FOLD_OPTIONS *options;
{
   int i;

   if (options->type == SINE_FOLD)
      fprintf(stream,"\tType\t= %s\n", "Sine");
   else if (options->type == BIOT_FOLD)
      fprintf(stream,"\tType\t= %s\n", "Biot");
   else
      fprintf(stream,"\tType\t= %s\n", "Fourier");

   if (options->singleFold)
      fprintf(stream,"\tSingle Fold\t= TRUE\n");
   else
      fprintf(stream,"\tSingle Fold\t= FALSE\n");

   fprintf(stream,"\tX\t= %6.2lf\n", options->positionX);
   fprintf(stream,"\tY\t= %6.2lf\n", options->positionY);
   fprintf(stream,"\tZ\t= %6.2lf\n", options->positionZ);

   fprintf(stream,"\tDip Direction\t= %6.2lf\n", options->dipDirection);
   fprintf(stream,"\tDip\t= %6.2lf\n", options->dip);
   fprintf(stream,"\tPitch\t= %6.2lf\n", options->axisPitch);

   fprintf(stream,"\tWavelength\t= %6.2lf\n", options->wavelength);
   fprintf(stream,"\tAmplitude\t= %6.2lf\n", options->amplitude);
   fprintf(stream,"\tCylindricity\t= %6.2lf\n", options->cycle);
        
   fprintf(stream,"\tFourier Series\n");
   for (i = 0; i < 11; i++)
   {
      fprintf(stream,"\t\tTerm A %d\t= %6.2lf\n",i,options->fourierCoeff[0][i]);
      fprintf(stream,"\t\tTerm B %d\t= %6.2lf\n",i,options->fourierCoeff[1][i]);
   }
   
   reportProfile (stream, &(options->profile));
   
   return (TRUE);
}



int report_dykes(stream, options)
FILE *stream; 
DYKE_OPTIONS *options;
{
   if (options->type == DILATION_DYKE)
      fprintf(stream,"\tType\t= %s\n", "Dilate");
   else
      fprintf(stream,"\tType\t= %s\n", "Stope");

   fprintf(stream,"\tMerge Events\t= %d\n", options->mergeEvents);

   fprintf(stream,"\tX\t= %6.2lf\n", options->positionX);
   fprintf(stream,"\tY\t= %6.2lf\n", options->positionY);
   fprintf(stream,"\tZ\t= %6.2lf\n", options->positionZ);

   fprintf(stream,"\tDip Direction\t= %6.2lf\n", options->dipDirection);
   fprintf(stream,"\tDip\t= %6.2lf\n", options->dip);
   fprintf(stream,"\tPitch\t = %6.2lf\n", options->slipPitch);

   fprintf(stream,"\tSlip\t = %6.2lf\n", options->slipLength);
   fprintf(stream,"\tWidth\t = %6.2lf\n", options->width);
   
   if (options->alterationZones == NONE_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= NONE\n");
   else if (options->alterationZones == TOP_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= TOP\n");
   else if (options->alterationZones == BOTTOM_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= BOTTOM\n");
   else /* BOTH_ALTERATION */
      fprintf(stream,"\tAlteration Type \t= BOTH\n");

   reportAlteration(stream, options->alterationFuntions);

   reportProperties (stream, &(options->properties));

   return (TRUE);
}



int report_faults(stream, options)
FILE *stream; 
FAULT_OPTIONS *options;
{
   int i;

   if (options->geometry == TRANSLATION)
      fprintf(stream,"\tGeometry\t= %s\n", "Translation");
   else if (options->geometry == ROTATION)
      fprintf(stream,"\tGeometry\t= %s\n", "Rotation");
   else if (options->geometry == ELLIPTICAL)
      fprintf(stream,"\tGeometry\t= %s\n", "Elliptical");
   else if (options->geometry == CURVED)
      fprintf(stream,"\tGeometry\t= %s\n", "Curved");
   else
      fprintf(stream,"\tGeometry\t= %s\n", "Ring");

   if (options->movement == HANGING_WALL)
      fprintf(stream,"\tMovement\t= %s\n", "Hanging Wall");
   else if (options->movement == FOOT_WALL)
      fprintf(stream,"\tMovement\t= %s\n", "Foot Wall");
   else
      fprintf(stream,"\tMovement\t= %s\n", "Both");

   fprintf(stream,"\tX\t= %6.2lf\n", options->positionX);
   fprintf(stream,"\tY\t= %6.2lf\n", options->positionY);
   fprintf(stream,"\tZ\t= %6.2lf\n", options->positionZ);

   fprintf(stream,"\tDip Direction\t= %6.2lf\n", options->dipDirection);
   fprintf(stream,"\tDip\t= %6.2lf\n", options->dip);

   fprintf(stream,"\tPitch\t= %6.2lf\n", options->pitch);
   fprintf(stream,"\tSlip\t= %6.2lf\n", options->slip);
   fprintf(stream,"\tRotation\t= %6.2lf\n", options->rotation);
   fprintf(stream,"\tAmplitude\t= %6.2lf\n", options->amplitude);
   fprintf(stream,"\tRadius\t= %6.2lf\n", options->radius);
   fprintf(stream,"\tXAxis\t= %6.2lf\n", options->xAxis);
   fprintf(stream,"\tYAxis\t= %6.2lf\n", options->yAxis);
   fprintf(stream,"\tZAxis\t= %6.2lf\n", options->zAxis);
   fprintf(stream,"\tCyl Index\t= %6.2lf\n", options->cylindricalIndex);
   fprintf(stream,"\tProfile Pitch\t= %6.2lf\n", options->profilePitch);

   reportColor (stream, &(options->color));

   fprintf(stream,"\tFourier Series\n");
   for (i = 0; i < 11; i++)
   {
      fprintf(stream,"\t\tTerm A %d\t= %6.2lf\n",i,options->fourierCoeff[0][i]);
      fprintf(stream,"\t\tTerm B %d\t= %6.2lf\n",i,options->fourierCoeff[1][i]);
   }
   
   reportProfile (stream, &(options->profile));

   if (options->alterationZones == NONE_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= NONE\n");
   else if (options->alterationZones == TOP_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= TOP\n");
   else if (options->alterationZones == BOTTOM_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= BOTTOM\n");
   else /* BOTH_ALTERATION */
      fprintf(stream,"\tAlteration Type \t= BOTH\n");

   reportAlteration(stream, options->alterationFuntions);

   reportSurface (stream, options->surfaceType, &(options->surfaceFilename),
         options->surfaceXDim, options->surfaceYDim, options->surfaceZDim);

   return (TRUE);
}



int report_shear_zone(stream, options)
FILE *stream; 
SHEAR_OPTIONS *options;
{
   int i;

   if (options->type == SHEAR_ZONE_EVENT)
      fprintf(stream,"\tType\t= %s\n", "Shear Zone");
   else
      fprintf(stream,"\tType\t= %s\n", "Kink");

   if (options->geometry == TRANSLATION)
      fprintf(stream,"\tGeometry\t= %s\n", "Translation");
   else if (options->geometry == ROTATION)
      fprintf(stream,"\tGeometry\t= %s\n", "Rotation");
   else if (options->geometry == ELLIPTICAL)
      fprintf(stream,"\tGeometry\t= %s\n", "Elliptical");
   else if (options->geometry == CURVED)
      fprintf(stream,"\tGeometry\t= %s\n", "Curved");
   else
      fprintf(stream,"\tGeometry\t= %s\n", "Ring");

   if (options->movement == HANGING_WALL)
      fprintf(stream,"\tMovement\t= %s\n", "Hanging Wall");
   else if (options->movement == FOOT_WALL)
      fprintf(stream,"\tMovement\t= %s\n", "Foot Wall");
   else
      fprintf(stream,"\tMovement\t= %s\n", "Both");

   fprintf(stream,"\tX\t= %6.2lf\n", options->positionX);
   fprintf(stream,"\tY\t= %6.2lf\n", options->positionY);
   fprintf(stream,"\tZ\t= %6.2lf\n", options->positionZ);

   fprintf(stream,"\tDip Direction\t= %6.2lf\n", options->dipDirection);
   fprintf(stream,"\tDip\t= %6.2lf\n", options->dip);

   fprintf(stream,"\tPitch\t= %6.2lf\n", options->pitch);
   fprintf(stream,"\tSlip\t= %6.2lf\n", options->slip);
   fprintf(stream,"\tRotation\t= %6.2lf\n", options->rotation);
   fprintf(stream,"\tAmplitude\t= %6.2lf\n", options->amplitude);
   fprintf(stream,"\tWidth\t= %6.2lf\n", options->width);
   fprintf(stream,"\tRadius\t= %6.2lf\n", options->radius);
   fprintf(stream,"\tXAxis\t= %6.2lf\n", options->xAxis);
   fprintf(stream,"\tYAxis\t= %6.2lf\n", options->yAxis);
   fprintf(stream,"\tZAxis\t= %6.2lf\n", options->zAxis);
   fprintf(stream,"\tCyl Index\t= %6.2lf\n", options->cylindricalIndex);
   fprintf(stream,"\tProfile Pitch\t= %6.2lf\n", options->profilePitch);

   reportColor (stream, &(options->color));

   fprintf(stream,"\tFourier Series\n");
   for (i = 0; i < 11; i++)
   {
      fprintf(stream,"\t\tTerm A %d\t= %6.2lf\n",i,options->fourierCoeff[0][i]);
      fprintf(stream,"\t\tTerm B %d\t= %6.2lf\n",i,options->fourierCoeff[1][i]);
   }

   reportProfile (stream, &(options->profile));

   if (options->alterationZones == NONE_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= NONE\n");
   else if (options->alterationZones == TOP_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= TOP\n");
   else if (options->alterationZones == BOTTOM_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= BOTTOM\n");
   else /* BOTH_ALTERATION */
      fprintf(stream,"\tAlteration Type \t= BOTH\n");

   reportAlteration(stream, options->alterationFuntions);

   reportSurface (stream, options->surfaceType, &(options->surfaceFilename),
         options->surfaceXDim, options->surfaceYDim, options->surfaceZDim);

   return (TRUE);
}



int report_lineations(stream, options)
FILE *stream; 
LINEATION_OPTIONS *options;
{
   fprintf(stream,"\tPlunge Direction\t = %6.2lf\n", options->plungeDirection);
   fprintf(stream,"\tPlunge\t = %6.2lf\n", options->plunge);
   return (TRUE);
}
 


int report_planes(stream, options)
FILE *stream; 
FOLIATION_OPTIONS *options;
{
   fprintf(stream,"\tDip Direction\t = %6.2lf\n", options->dipDirection);
   fprintf(stream,"\tDip\t = %6.2lf\n", options->dip);
   return (TRUE);
}



int report_plugs(stream, options)
FILE *stream; 
PLUG_OPTIONS *options;
{
   if (options->type == CYLINDRICAL_PLUG)
      fprintf(stream,"\tType\t= %s\n", "Cylindrical");
   else if (options->type == CONE_PLUG)
      fprintf(stream,"\tType\t= %s\n", "Cone");
   else if (options->type == PARABOLIC_PLUG)
      fprintf(stream,"\tType\t= %s\n", "Parabolic");
   else
      fprintf(stream,"\tType\t= %s\n", "Ellipsodial");

   fprintf(stream,"\tMerge Events\t= %d\n", options->mergeEvents);

   fprintf(stream,"\tX\t= %6.2lf\n", options->positionX);
   fprintf(stream,"\tY\t= %6.2lf\n", options->positionY);
   fprintf(stream,"\tZ\t= %6.2lf\n", options->positionZ);

   fprintf(stream,"\tDip Direction\t= %6.2lf\n", options->dipDirection);
   fprintf(stream,"\tDip\t= %6.2lf\n", options->dip);
   fprintf(stream,"\tPitch\t = %6.2lf\n", options->axisPitch);

   fprintf(stream,"\tRadius\t = %6.2lf\n", options->radius);
   fprintf(stream,"\tApicalAngle\t = %6.2lf\n", options->apicalAngle);
   fprintf(stream,"\tB-value\t = %6.2lf\n", options->BValue);
   fprintf(stream,"\tA-value\t = %6.2lf\n", options->axisA);
   fprintf(stream,"\tB-value\t = %6.2lf\n", options->axisB);
   fprintf(stream,"\tC-value\t = %6.2lf\n", options->axisC);
   
   if (options->alterationZones == NONE_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= NONE\n");
   else if (options->alterationZones == TOP_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= TOP\n");
   else if (options->alterationZones == BOTTOM_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= BOTTOM\n");
   else /* BOTH_ALTERATION */
      fprintf(stream,"\tAlteration Type \t= BOTH\n");

   reportAlteration(stream, options->alterationFuntions);

   reportProperties (stream, &(options->properties));

   return (TRUE);
}



int report_tilts(stream, options)
FILE *stream; 
TILT_OPTIONS *options;
{
   fprintf(stream,"\tX\t= %6.2lf\n", options->positionX);
   fprintf(stream,"\tY\t= %6.2lf\n", options->positionY);
   fprintf(stream,"\tZ\t= %6.2lf\n", options->positionZ);

   fprintf(stream,"\tRotation\t = %6.2lf\n", options->rotation);
   fprintf(stream,"\tPlunge Direction\t = %6.2lf\n", options->plungeDirection);
   fprintf(stream,"\tPlunge\t = %6.2lf\n", options->plunge);
   return (TRUE);
}



int report_strains(stream, options)
FILE *stream; 
STRAIN_OPTIONS *options;
{
   fprintf(stream,"\tA\t = %6.2lf\n", options->tensor[0][0]);
   fprintf(stream,"\tB\t = %6.2lf\n", options->tensor[1][0]);
   fprintf(stream,"\tC\t = %6.2lf\n", options->tensor[2][0]);
   fprintf(stream,"\tD\t = %6.2lf\n", options->tensor[0][1]);
   fprintf(stream,"\tE\t = %6.2lf\n", options->tensor[1][1]);
   fprintf(stream,"\tF\t = %6.2lf\n", options->tensor[2][1]);
   fprintf(stream,"\tG\t = %6.2lf\n", options->tensor[0][2]);
   fprintf(stream,"\tH\t = %6.2lf\n", options->tensor[1][2]);
   fprintf(stream,"\tI\t = %6.2lf\n", options->tensor[2][2]);
   return (TRUE);
}


int report_import(stream, options)
FILE *stream; 
IMPORT_OPTIONS *options;
{
   int i;
   char temp[255];
   
   fprintf(stream,"\tFilename\t = %s\n", options->importFile.name);
   xvt_fsys_convert_dir_to_str (&(options->importFile.dir), temp, 255);
   fprintf(stream,"\tDirectory\t = %s\n", temp);
   
   if (options->positionBy == POS_BY_CENTER)
      fprintf(stream,"\tPosition By\t = Center\n");
   else   
      fprintf(stream,"\tPosition By\t = Corner\n");
   
   fprintf(stream,"\tX\t= %6.2lf\n", options->positionX);
   fprintf(stream,"\tY\t= %6.2lf\n", options->positionY);
   fprintf(stream,"\tZ\t= %6.2lf\n", options->positionZ);

   fprintf(stream,"\tBlock Size\t= %6.2lf\n", options->blockSize);
   fprintf(stream,"\tFile Format\t= Noddy\n");

   fprintf(stream,"\tNum Properties\t = %d\n", options->numProperties);
   for (i = 0; i < options->numProperties; i++)
   {
      reportProperties (stream, &(options->properties[i]));
   }
   return (TRUE);
}

int report_generic(stream, options)
FILE *stream; 
GENERIC_OPTIONS *options;
{
   fprintf(stream,"\tParam1\t = %6.2lf\n", options->param1);
   fprintf(stream,"\tParam2\t = %6.2lf\n", options->param2);
   fprintf(stream,"\tParam3\t = %6.2lf\n", options->param3);
   fprintf(stream,"\tParam4\t = %6.2lf\n", options->param4);
   fprintf(stream,"\tParam5\t = %6.2lf\n", options->param5);
   fprintf(stream,"\tParam6\t = %6.2lf\n", options->param6);
   fprintf(stream,"\tParam7\t = %6.2lf\n", options->param7);
   fprintf(stream,"\tParam8\t = %6.2lf\n", options->param8);
   fprintf(stream,"\tParam9\t = %6.2lf\n", options->param9);
   fprintf(stream,"\tParam10\t = %6.2lf\n", options->param10);
   return (TRUE);
}


int report_unconformities(stream, options)
FILE *stream; 
UNCONFORMITY_OPTIONS *options;
{
   fprintf(stream,"\tX\t= %6.2lf\n", options->positionX);
   fprintf(stream,"\tY\t= %6.2lf\n", options->positionY);
   fprintf(stream,"\tZ\t= %6.2lf\n", options->positionZ);


   fprintf(stream,"\tDip Direction\t= %6.2lf\n", options->dipDirection);
   fprintf(stream,"\tDip\t= %6.2lf\n", options->dip);

   if (options->alterationZones == NONE_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= NONE\n");
   else if (options->alterationZones == TOP_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= TOP\n");
   else if (options->alterationZones == BOTTOM_ALTERATION)
      fprintf(stream,"\tAlteration Type \t= BOTTOM\n");
   else /* BOTH_ALTERATION */
      fprintf(stream,"\tAlteration Type \t= BOTH\n");

   reportAlteration(stream, options->alterationFuntions);

   reportSurface (stream, options->surfaceType, &(options->surfaceFilename),
         options->surfaceXDim, options->surfaceYDim, options->surfaceZDim);

   reportStratigraphy (stream, &(options->stratigraphyDetails));

   return (TRUE);
}


int reportStratigraphy (stream, options)
FILE *stream;
STRATIGRAPHY_OPTIONS *options;
{
   int i;

   fprintf(stream,"\tNum Layers\t= %d\n", options->numLayers);
   for (i = 0; i < options->numLayers; i++)
   {
      reportProperties (stream, &(options->properties[i]));
   }
   return (TRUE);
}


int reportProperties (stream, options)
FILE *stream;
LAYER_PROPERTIES *options;
{
   fprintf(stream,"\tUnit Name\t= %s\n", options->unitName);
   fprintf(stream,"\tHeight\t= %d\n", options->height);
   if (options->applyAlterations)
      fprintf(stream,"\tApply Alterations\t= ON\n");
   else
      fprintf(stream,"\tApply Alterations\t= OFF\n");

   fprintf(stream,"\tDensity\t= %6.2le\n", options->density);

   fprintf(stream,"\tAnisotropic Field\t= %d\n", options->anisotropicField);
   fprintf(stream,"\tMagSusX\t= %6.2le\n", options->sus_X);
   fprintf(stream,"\tMagSusY\t= %6.2le\n", options->sus_Y);
   fprintf(stream,"\tMagSusZ\t= %6.2le\n", options->sus_Z);
   fprintf(stream,"\tMagSus Dip\t= %6.2le\n", options->sus_dip);
   fprintf(stream,"\tMagSus DipDir\t= %6.2le\n", options->sus_dipDirection);
   fprintf(stream,"\tMagSus Pitch\t= %6.2le\n", options->sus_pitch);

   fprintf(stream,"\tRemanent Magnetization\t= %d\n",options->remanentMagnetization);
   fprintf(stream,"\tInclination\t= %6.2lf\n", options->inclination);
   fprintf(stream,"\tAngle with the Magn. North\t= %6.2lf\n",
                                                     options->angleWithNorth);
   fprintf(stream,"\tStrength\t= %6.2le\n", options->strength);

   reportColor (stream, &(options->color));
   return (TRUE);
}

int reportColor (stream, options)
FILE *stream;
NODDY_COLOUR *options;
{
   fprintf(stream,"\tColor Name\t= %s\n", options->name);
   fprintf(stream,"\tRed\t= %d\n", options->red);
   fprintf(stream,"\tGreen\t= %d\n", options->green);
   fprintf(stream,"\tBlue\t= %d\n", options->blue);
   return (TRUE);
}

int reportAlteration (stream, options)
FILE *stream;
PROFILE_OPTIONS *options;
{
   PROFILE_OPTIONS *profOptions;
   int numProfiles = 0;

   for (profOptions = options; profOptions; profOptions = profOptions->next)
      numProfiles++;
   
   fprintf(stream,"\tNum Profiles\t= %d\n", numProfiles);
   for (profOptions = options; profOptions; profOptions = profOptions->next)
   {
      reportProfile (stream, profOptions);
   }
   
   return (TRUE);
}

int reportProfile (stream, options)
FILE *stream;
PROFILE_OPTIONS *options;
{
   PROFILE_POINT *point;
   int numPoints = 0;
   
   fprintf(stream,"\tName\t= %s\n", options->name);
   fprintf(stream,"\tType\t= %d\n", options->type);
   if (options->joinType == LINE_SEGMENTS)
      fprintf(stream,"\tJoin Type \t= LINES\n");
   else if (options->joinType == CURVE_SEGMENTS)
      fprintf(stream,"\tJoin Type \t= CURVES\n");
   else
      fprintf(stream,"\tJoin Type \t= SQUARE\n");
   fprintf(stream,"\tGraph Length\t= %lf\n", options->graphLength);
   fprintf(stream,"\tMin X\t= %lf\n", options->minX);
   fprintf(stream,"\tMax X\t= %lf\n", options->maxX);
   fprintf(stream,"\tMin Y Scale\t= %lf\n", options->minYScale);
   fprintf(stream,"\tMax Y Scale\t= %lf\n", options->maxYScale);
   fprintf(stream,"\tScale Origin\t= %lf\n", options->scaleOrigin);
   fprintf(stream,"\tMin Y Replace\t= %lf\n", options->minYReplace);
   fprintf(stream,"\tMax Y Replace\t= %lf\n", options->maxYReplace);
                       /* Count the number of points we have */
   for (point = options->points; point; point = point->next)
      numPoints++;
      
   fprintf(stream,"\tNum Points\t= %d\n", numPoints);
   for (point = options->points; point; point = point->next)
   {
      fprintf(stream,"\t\tPoint X\t= %d\n", (int) point->point.h);
      fprintf(stream,"\t\tPoint Y\t= %d\n", (int) point->point.v);
   }

   return (TRUE);
}

int reportSurface (stream, surfaceType, surfaceFilename,
                   surfaceXDim, surfaceYDim, surfaceZDim)
FILE *stream;
SURFACE_TYPE surfaceType;
FILE_SPEC *surfaceFilename;
double surfaceXDim, surfaceYDim, surfaceZDim;
{
   char strVal[100];

   if (surfaceType == DXF_SURFACE)
      fprintf(stream,"\tSurface Type\t= DXF_SURFACE\n");
   else if (surfaceType == UNKNOWN_SURFACE)
      fprintf(stream,"\tSurface Type\t= UNKNOWN_SURFACE\n");
   else /* FLAT_SURFACE */
      fprintf(stream,"\tSurface Type\t= FLAT_SURFACE\n");
   fprintf(stream,"\tSurface Filename\t= %s\n", surfaceFilename->name);
   if (!xvt_fsys_convert_dir_to_str(&(surfaceFilename->dir), strVal, 100))
      strcpy (strVal, ".");
   fprintf(stream,"\tSurface Directory\t= %s\n", strVal);
   fprintf(stream,"\tSurface XDim\t= %lf\n", surfaceXDim);
   fprintf(stream,"\tSurface YDim\t= %lf\n", surfaceYDim);
   fprintf(stream,"\tSurface ZDim\t= %lf\n", surfaceZDim);

   return (TRUE);
}

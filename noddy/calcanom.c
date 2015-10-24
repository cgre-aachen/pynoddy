#include "noddy.h"
#include <math.h>
#include "time.h"

#define DEBUG(X)  
#define DEG_TO_RAD(X)   (((double) (X))*0.01745329)
#define RAD_TO_DEG(X)   (((double) (X))*57.2957795)
#define PI 3.1415927

#define TOLERANCE 0.01
#define HIGH_TOLERANCE 0.00001
#define GRAVITY_CONSTANT 6.672e-11;

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern double iscale; /* scaling factor for geology */
extern BOOLEAN skipMemManager;
extern double **topographyMap;
extern int TopoCol, TopoRow;
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE, minTopoValue, maxTopoValue;
extern int batchExecution;

#if XVT_CC_PROTO
extern int grab_line(FILE *, char *, int);
#else
extern int grab_line();
#endif


#if XVT_CC_PROTO
int getCubeToCalculate (int, int, int, int, int, int, double, double, double, double, short ***, 
				double *, double *, double *, double *, double *, double *, double, double **, int, int,
				short ***, LAYER_PROPERTIES **,  float ***, float ***, float ***, float ***, float ***,
				float ***, float ***, float ***, float ***, float ***, float ***);
int createAnomFilter(double **, double **, double **, double **, double **, double ***, double ***,
                     int, int, GEOPHYSICS_OPTIONS *, int, double, double, double, double);
int calcAnomAtPoints(double, double, double, double, double, double,
					  double, double, double, double, double, double, double,
					  Point3d *, int, double *, double *, double *, double *, double *,
					  int, double, double);
int getGravityVector(double *, int, int, int, short ***, LAYER_PROPERTIES **, float ***);
int getMagneticVectors(double *, double *, double *, double, double, double,
						   int, int, int, short ***, LAYER_PROPERTIES **, float ***,
							float ***, float ***, float ***, float ***, float ***,
							float ***, float ***, float ***, float ***);
int calcGravityLayer(double **, double **, int, GEOPHYSICS_OPTIONS *, int,
                     int, int, int, int, LAYER_PROPERTIES **, short ***, float ***);
int calcMagneticsLayer(double **, double **, double **, double **,
                       double **, double **, double **, double **, double ***, double ***, 
                       int, GEOPHYSICS_OPTIONS *, double, double, double, int, int, int,
                       int, int, int, int, int, LAYER_PROPERTIES **, short ***,
                       float ***, float ***,  float ***, float ***,
                       float ***, float ***,  float ***, float ***,
                       float ***, float ***);
int calcMagFromComponents(double **, double **, double **, double **,
								  double *, double *, double *, double *,
								  int, int, int, GEOPHYSICS_OPTIONS *, double, double, double);
int writeAnomFile (char *, double **, double *, int, int, int, int, int, BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *);
int writeXYZFile (char *, FILE_SPEC *, Point3d *, int, double *, double *, double *, double *, double *);
int calcAnomSlave(double ***, int, int, GEOPHYSICS_OPTIONS *, int, double);
int fillAnomImage (double **, double *, int, int, int, int, DOUBLE_2D_IMAGE *);
int oldCalcAnomLayer(double **, double **, double ***, int, GEOPHYSICS_OPTIONS *, int,
                  int, int, int, int,int, int, int, int,
                  int, int, LAYER_PROPERTIES **, short ***indexData,
                  float ***, float ***, float ***, float ***, float ***, float ***,
                  float ***, float ***, float ***, float ***, float ***);
#else
int getCubeToCalculate();
int createAnomFilter();
int calcAnomAtPoints ();
int getGravityVector();
int getMagneticVectors();
int calcGravityLayer();
int calcMagneticsLayer();
int writeAnomFile ();
int writeXYZFile ();
int calcAnomSlave();
int fillAnomImage ();
int oldCalcAnomLayer();
#endif
                 /* ************************* */
                 /* Globals used in this file */
enum { GRAVITY = 0, MAGNETICS, MAGNETICS_X, MAGNETICS_Y, MAGNETICS_Z };
enum { MAGNETICS_X_FIELD = 0, MAGNETICS_Y_FIELD,
       MAGNETICS_Z_FIELD, GRAVITY_FIELD,
       MAGNETICS_TOTAL_FIELD };

#if XVT_CC_PROTO
static short getIndex (short **, int, int);
static float getDensity (float **, int, int);
static float getMagSus (float **, int, int);
static float getRemSusDec (float **, int, int);
static float getRemSusAzi (float **, int, int);
static float getRemSusStr (float **, int, int);
static float getAniSusDip (float **, int, int);
static float getAniSusDDir (float **, int, int);
static float getAniSusPitch (float **, int, int);
static float getAniSusAxis1 (float **, int, int);
static float getAniSusAxis2 (float **, int, int);
static float getAniSusAxis3 (float **, int, int);
#else
static short getIndex ();
static float getDensity ();
static float getMagSus ();
static float getRemSusDec ();
static float getRemSusAzi ();
static float getRemSusStr ();
static float getAniSusDip ();
static float getAniSusDDir ();
static float getAniSusPitch ();
static float getAniSusAxis1 ();
static float getAniSusAxis2 ();
static float getAniSusAxis3 ();
#endif

int
#if XVT_CC_PROTO
getFieldAtLocation (GEOPHYSICS_OPTIONS *options, double xLoc, double yLoc, double zLoc,
						  double *inclination, double *declination, double *intensity)
#else
getFieldAtLocation (options, xLoc, yLoc, zLoc, inclination, declination, intensity)
GEOPHYSICS_OPTIONS *options;
double xLoc, yLoc, zLoc;
double *inclination, *declination, *intensity;
#endif
{
	int error = FALSE;

	if (options->fieldType == FIXED_FIELD)
	{
		*inclination = options->inclination;
		*declination = options->declination;
		*intensity = options->intensity;
	}
	else  /* Variable */
	{
		double dip, dipDir, coeffA, coeffB, coeffC, coeffD;

		if (fabs(options->inclinationChange) > HIGH_TOLERANCE) /* Inclination Plane */
		{
			dip = RAD_TO_DEG(atan(options->inclinationChange/1000.0));
			dipDir = options->inclinationOri;

			convrt(dip, 360.0-dipDir, 90.0, FALSE);
			plane(&coeffA, &coeffB, &coeffC, &coeffD, options->xPos, options->yPos, options->inclination);

			*inclination = -(xLoc*coeffA + yLoc*coeffB + coeffD)/coeffC;
		}
		else  /* Constant */
			*inclination = options->inclination;

		if (fabs(options->intensityChange) > HIGH_TOLERANCE) /* Intensity Plane */
		{
			dip = RAD_TO_DEG(atan(options->intensityChange/1000.0));
			dipDir = options->intensityOri;

			convrt(dip, 360.0-dipDir, 90.0, FALSE);
			plane(&coeffA, &coeffB, &coeffC, &coeffD, options->xPos, options->yPos, options->intensity);

			*intensity = -(xLoc*coeffA + yLoc*coeffB + coeffD)/coeffC;
		}
		else  /* Constant */
			*intensity = options->intensity;

		if (fabs(options->declinationChange) > HIGH_TOLERANCE) /* Declination Plane */
		{
			dip = RAD_TO_DEG(atan(options->declinationChange/1000.0));
			dipDir = options->declinationOri;

			convrt(dip, 360.0-dipDir, 90.0, FALSE);
			plane(&coeffA, &coeffB, &coeffC, &coeffD, options->xPos, options->yPos, options->declination);

			*declination = /*-*/(xLoc*coeffA + yLoc*coeffB + coeffD)/coeffC;
		}
		else  /* Constant */
			*declination = options->declination;
	}

	*declination *= -1;

	return (!error);
}

/* *****************************************************************
FUNCTION calcAnomalies

INPUT

OUTPUT

DESCRIPTION 
   calculates gravity and/or magnetic data from the blocks passed
   int if they are in memory or from appropriate disk files if 
   there was not enough memory.

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
calcAnomalies (char *outputFilename, char *blockName, int numLayers,
        int **layerDimensions, int numCubeSizes, int *cubeSizes,
        BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *options,
        int densityCalc, int susCalc, int remCalc, int aniCalc,
        int indexCalc, int numProps, LAYER_PROPERTIES **propArray,
        short ***indexData,        float ***densityData,
        float ***magSusData,       float ***remSusDecData,
        float ***remSusAziData,    float ***remSusStrData,
        float ***aniSusDipData,    float ***aniSusDDirData,
        float ***aniSusPitchData,  float ***aniSusAxis1Data,
        float ***aniSusAxis2Data,  float ***aniSusAxis3Data,
        DOUBLE_2D_IMAGE *magImage, DOUBLE_2D_IMAGE *grvImage)
#else
calcAnomalies (outputFilename, blockName, numLayers,
        layerDimensions, numCubeSizes, cubeSizes, blockView,
        options, densityCalc, susCalc, remCalc, aniCalc,
        indexCalc, numProps, propArray,   indexData,
        densityData,     magSusData,      remSusDecData,
        remSusAziData,   remSusStrData,   aniSusDipData,
        aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
        aniSusAxis2Data, aniSusAxis3Data, magImage, grvImage)
char *outputFilename, *blockName;
int numLayers, **layerDimensions, numCubeSizes, *cubeSizes;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
int densityCalc, susCalc, remCalc, aniCalc, indexCalc, numProps;
LAYER_PROPERTIES **propArray;
short ***indexData;
float ***densityData,    ***magSusData,      ***remSusDecData,
      ***remSusAziData,  ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData, ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data,***aniSusAxis3Data;
DOUBLE_2D_IMAGE *magImage, *grvImage;
#endif
{
   double ***anomFilter = NULL, **grvResult = NULL, **magResult = NULL;
   double **magXResult = NULL, **magYResult = NULL, **magZResult = NULL;
	double ***anomP1Filter = NULL, ***anomP2Filter = NULL;
   int filterSize, z, nx = 0, ny = 0, cubeSize, calcRangeInCubes;
	double inclination, declination, intensity;
   double zPos;

   cubeSize = cubeSizes[0];
   for (z = 0; z < numLayers; z++)
   {
      if (layerDimensions[z][0] > nx)
         nx = layerDimensions[z][0];
      if (layerDimensions[z][1] > ny)
         ny = layerDimensions[z][1];
      if (cubeSizes[z] < cubeSize) /* Finest cube Size */
         cubeSize = cubeSizes[z];
   }
   calcRangeInCubes = (int) floor ((double) (((double) options->calculationRange)/((double) cubeSize)));
   
                   /* *************************************** */                   
                   /* Alllocate the working calculation array */
   filterSize = calcRangeInCubes*2 + 1;
   if (!(anomFilter = (double ***) create3DArray (5, filterSize, filterSize,
                                             sizeof(double))))
      return (FALSE);

   if (!(anomP1Filter = (double ***) create3DArray (3, filterSize, filterSize,
                                             sizeof(double))))
      return (FALSE);
   if (!(anomP2Filter = (double ***) create3DArray (3, filterSize, filterSize,
                                             sizeof(double))))
      return (FALSE);

   if (!(grvResult = (double **) create2DArray(nx + filterSize,
                               ny + filterSize, sizeof(double))))
      return (FALSE);
   if (!(magResult = (double **) create2DArray(nx + filterSize,
                               ny + filterSize, sizeof(double))))
      return (FALSE);
      
   if (options->magneticVectorComponents || !options->projectVectorsOntoField)
   {
      if (!(magXResult = (double **) create2DArray(nx + filterSize,
                                  ny + filterSize, sizeof(double))))
         return (FALSE);
      if (!(magYResult = (double **) create2DArray(nx + filterSize,
                                  ny + filterSize, sizeof(double))))
         return (FALSE);
      if (!(magZResult = (double **) create2DArray(nx + filterSize,
                                  ny + filterSize, sizeof(double))))
         return (FALSE);
   }

                         /* ***************************** */                   
                         /* Calculate the Anomalies image */
   if (options->calculationAltitude == AIRBORNE)
      zPos = options->altitude;
   else
      zPos = 0.0;

                            /* Get the field location */
	getFieldAtLocation (options, 0.0, 0.0, 0.0, &inclination, &declination, &intensity);
							 /* Calculate from bottom layer up */
	for (z = numLayers-1; z >= 0; z--)
	{
		zPos = zPos + cubeSizes[z]/2.0;  /* add half a cube */

		createAnomFilter(anomFilter[MAGNETICS_X_FIELD], anomFilter[MAGNETICS_Y_FIELD],
							  anomFilter[MAGNETICS_Z_FIELD], anomFilter[MAGNETICS_TOTAL_FIELD],
							  anomFilter[GRAVITY_FIELD], anomP1Filter, anomP2Filter,
							  filterSize, filterSize, options, cubeSize, zPos,
							  inclination, declination, intensity);
		calcGravityLayer(grvResult, anomFilter[GRAVITY_FIELD], filterSize, options,
							  z, layerDimensions[z][0], layerDimensions[z][1], indexCalc,
							  numProps, propArray, indexData, densityData);
		calcMagneticsLayer(magXResult, magYResult, magZResult, magResult,
							  anomFilter[MAGNETICS_X_FIELD], anomFilter[MAGNETICS_Y_FIELD],
							  anomFilter[MAGNETICS_Z_FIELD], anomFilter[MAGNETICS_TOTAL_FIELD], 
							  anomP1Filter, anomP2Filter, filterSize, options, inclination,
							  declination, intensity, z, layerDimensions[z][0], layerDimensions[z][1],
							  susCalc, remCalc, aniCalc, indexCalc, numProps, propArray, indexData,
							  magSusData,      remSusDecData,   remSusAziData,   remSusStrData,
							  aniSusDipData,   aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
							  aniSusAxis2Data, aniSusAxis3Data);

		zPos = zPos + cubeSizes[z]/2.0;  /* add the other half */
   
		if (abortLongJob ())
			return (FALSE);
	}

	if (!options->projectVectorsOntoField)
	{                                /* Combine Vector Components into mag Image */
		calcMagFromComponents(magResult, magXResult, magYResult, magZResult,
									 (double *) NULL, (double *) NULL, (double *) NULL, (double *) NULL,
			                   nx+filterSize, ny+filterSize, filterSize, options,
									 inclination, declination, intensity);
		if (!options->magneticVectorComponents)
		{
			destroy2DArray ((char **) magXResult, nx + filterSize, ny + filterSize);
			destroy2DArray ((char **) magYResult, nx + filterSize, ny + filterSize);
			destroy2DArray ((char **) magZResult, nx + filterSize, ny + filterSize);
			magXResult = magYResult = magZResult = NULL;
		}
	}

                       /* ********************* */
                       /* Write out the Results */
   if (grvResult)
      writeAnomFile (outputFilename, grvResult, NULL, GRAVITY,
              nx, ny, numLayers, filterSize, blockView, options);

   if (magResult)
      writeAnomFile (outputFilename, magResult, NULL, MAGNETICS,
              nx, ny, numLayers, filterSize, blockView, options);

   if (magXResult)
      writeAnomFile (outputFilename, magXResult, NULL, MAGNETICS_X,
              nx, ny, numLayers, filterSize, blockView, options);

   if (magYResult)
      writeAnomFile (outputFilename, magYResult, NULL, MAGNETICS_Y,
              nx, ny, numLayers, filterSize, blockView, options);

   if (magZResult)
      writeAnomFile (outputFilename, magZResult, NULL, MAGNETICS_Z,
              nx, ny, numLayers, filterSize, blockView, options);

   if (magImage)
      fillAnomImage (magResult, NULL, MAGNETICS, nx, ny, filterSize, magImage);

   if (grvImage)
      fillAnomImage (grvResult, NULL, GRAVITY, nx, ny, filterSize, grvImage);

                       /* ******************************** */
                       /* Free all the memory that we used */
   if (grvResult)
      destroy2DArray ((char **) grvResult, nx + filterSize,
                                           ny + filterSize);
   if (magResult)
      destroy2DArray ((char **) magResult, nx + filterSize,
                                           ny + filterSize);
   if (magXResult)
      destroy2DArray ((char **) magXResult,nx + filterSize,
                                           ny + filterSize);
   if (magYResult)
      destroy2DArray ((char **) magYResult,nx + filterSize,
                                           ny + filterSize);
   if (magZResult)
      destroy2DArray ((char **) magZResult,nx + filterSize,
                                           ny + filterSize);

   if (anomP1Filter)
      destroy3DArray ((char ***) anomP1Filter, 3, filterSize, filterSize);
   if (anomP2Filter)
      destroy3DArray ((char ***) anomP2Filter, 3, filterSize, filterSize);

   if (anomFilter)
      destroy3DArray ((char ***) anomFilter, 5, filterSize, filterSize);
   
   return (TRUE);
}

/* *****************************************************************
FUNCTION calcCompleteAnomalies

INPUT

OUTPUT

DESCRIPTION 
   calculates gravity and/or magnetic data from the blocks passed
   int if they are in memory or from appropriate disk files if 
   there was not enough memory.

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
calcCompleteAnomalies (char *outputFilename, char *blockName, int numLayers,
        int **layerDimensions, int numCubeSizes, int *cubeSizes,
        BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *options,
		  int xyzCalc, double **pointPos, int numPoints, FILE_SPEC *xyzImportFile,
        int densityCalc, int susCalc, int remCalc, int aniCalc,
        int indexCalc, int numProps, LAYER_PROPERTIES **propArray,
        short ***indexData,        float ***densityData,
        float ***magSusData,       float ***remSusDecData,
        float ***remSusAziData,    float ***remSusStrData,
        float ***aniSusDipData,    float ***aniSusDDirData,
        float ***aniSusPitchData,  float ***aniSusAxis1Data,
        float ***aniSusAxis2Data,  float ***aniSusAxis3Data,
        DOUBLE_2D_IMAGE *magImage, DOUBLE_2D_IMAGE *grvImage)
#else
calcCompleteAnomalies (outputFilename, blockName, numLayers,
        layerDimensions, numCubeSizes, cubeSizes, blockView,
        options, xyzCalc, pointPos, numPoints, xyzImportFile,
		  densityCalc, susCalc, remCalc, aniCalc,
        indexCalc, numProps, propArray,   indexData,
        densityData,     magSusData,      remSusDecData,
        remSusAziData,   remSusStrData,   aniSusDipData,
        aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
        aniSusAxis2Data, aniSusAxis3Data, magImage, grvImage)
char *outputFilename, *blockName;
int numLayers, **layerDimensions, numCubeSizes, *cubeSizes;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
int xyzCalc;
double **pointPos;
int numPoints;
FILE_SPEC *xyzImportFile;
int densityCalc, susCalc, remCalc, aniCalc, indexCalc, numProps;
LAYER_PROPERTIES **propArray;
short ***indexData;
float ***densityData,    ***magSusData,      ***remSusDecData,
      ***remSusAziData,  ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData, ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data,***aniSusAxis3Data;
DOUBLE_2D_IMAGE *magImage, *grvImage;
#endif
{
	int x, y, z, nx = 0, ny = 0, imageNx, imageNy, cubeNum = 0, totalCubes;
	int skipPointGrv = FALSE, skipPointMag = FALSE, numPointsToMeasure = 0, calcRangeInCubes = 0;
	double xPos, zPos, surveyHeight;
	double cubeSize, halfCubeSize;
	Point3d *pointsToMeasure = NULL, *pointPtr;
   double blockSusX, blockSusY, blockSusZ, blockDensity;
   double *magneticResults = NULL, *magXResults = NULL, *magYResults = NULL, *magZResults = NULL;
	double *gravityResults = NULL;
	double minX, minY, minZ;
	double cubeRange, sphereRange;
	double inclination, declination, intensity;
	double cubeXMin, cubeYMin, cubeZMin, cubeXMax, cubeYMax, cubeZMax;
	double cubeXCenter, cubeYCenter, cubeZCenter, edgePadding;
	short ***cubeUsed = NULL, growCubes = FALSE;
	double **topoOverlay = NULL;
	char statusMessage[100];

	/* Create a padding region with the same value as the edge cubes */
	if ((options->padWithRealGeology != TRUE) && (options->padWithRealGeology != FALSE) &&
		 (options->padWithRealGeology > 0))
		edgePadding = options->padWithRealGeology;
	else
		edgePadding = -1.0;

   cubeSize = cubeSizes[0];
   for (z = 0; z < numLayers; z++)
   {
      if (layerDimensions[z][0] > nx)
         nx = layerDimensions[z][0];
      if (layerDimensions[z][1] > ny)
         ny = layerDimensions[z][1];
      if (cubeSizes[z] < cubeSize) /* Finest cube Size */
         cubeSize = cubeSizes[z];
   }
	nx; ny; /* -- !!*/
	halfCubeSize = cubeSize / 2.0;
	calcRangeInCubes = (int) floor ((double) (((double) options->calculationRange)/((double) cubeSize)));

	/* the real image size without the range */
	imageNx = nx - calcRangeInCubes*2;
	imageNy = ny - calcRangeInCubes*2;
   if (xyzCalc)
		numPointsToMeasure = numPoints;
	else
		numPointsToMeasure = imageNx * imageNy;

								/* ***************************** */                   
                        /* Calculate the Anomalies image */
   if (options->calculationAltitude == AIRBORNE)
      surveyHeight = options->altitude;
   else
      surveyHeight = 0.0;

								/* ******************************** */                   
                        /* Calculate the Range to calc over */
   if (options->allExact)
      cubeRange = -1.0;
   else
      cubeRange = (double) options->calculationRange;
	sphereRange = -1.0;

								 /* ***************************** */
                         /* Setup memory to store results */
	if (densityCalc)
	{
		if (!(gravityResults = (double *) create1DArray(numPointsToMeasure, sizeof(double))))
			return (FALSE);
	}
	if (TRUE)  /* Mag calc */
	{
		if (!(magneticResults = (double *) create1DArray(numPointsToMeasure, sizeof(double))))
		{
			destroy1DArray((char *) gravityResults);
			return (FALSE);
		}
	}
	if (options->magneticVectorComponents || !options->projectVectorsOntoField/* || xyzCalc*/)
	{
		if (!(magXResults = (double *) create1DArray(numPointsToMeasure, sizeof(double))))
		{
			destroy1DArray((char *) gravityResults);
			destroy1DArray((char *) magneticResults);
			return (FALSE);
		}
		if (!(magYResults = (double *) create1DArray(numPointsToMeasure, sizeof(double))))
		{
			destroy1DArray((char *) gravityResults);
			destroy1DArray((char *) magneticResults);
			destroy1DArray((char *) magXResults);
			return (FALSE);
		}
		if (!(magZResults = (double *) create1DArray(numPointsToMeasure, sizeof(double))))
		{
			destroy1DArray((char *) gravityResults);
			destroy1DArray((char *) magneticResults);
			destroy1DArray((char *) magXResults);
			destroy1DArray((char *) magYResults);
			return (FALSE);
		}
	}

								 /* ***************************************** */
                         /* Setup Memory that allows for cube growing */
	if (options->fieldType == FIXED_FIELD)
		growCubes = TRUE; 
	if (growCubes)
	{
		if (!(cubeUsed = (short ***) create3DArray(numLayers, nx, ny, sizeof(short))))
		{
			destroy1DArray((char *) gravityResults);
			destroy1DArray((char *) magneticResults);
			destroy1DArray((char *) magXResults);
			destroy1DArray((char *) magYResults);
			destroy1DArray((char *) magZResults);
			return (FALSE);
		}
	}

								 /* ******************************************** */
                         /* Setup the Image that we will be measuring at */
	if (!(pointsToMeasure = (Point3d *) create1DArray(numPointsToMeasure, sizeof(Point3d))))
		return (FALSE);
	pointPtr = pointsToMeasure;

								 /* ****************************************************************** */
                         /* Use the actuall topo data to get the correct height of the surface */
	if (geologyOptions.useTopography || options->drapedSurvey)
	{
		topoOverlay = (double **) create2DArray (imageNx+1, imageNy+1, sizeof(double));
		overlayTopoOnArray (topographyMap, TopoCol, TopoRow,
								  TopomapYW, TopomapYE, TopomapXW, TopomapXE, 
								  topoOverlay, imageNx, imageNy,
								  blockView->originY, blockView->originY + imageNy*cubeSize,
								  blockView->originX, blockView->originX + imageNx*cubeSize);
/*								  blockView->originY, blockView->originY + imageNy*cubeSize, */
/*								  blockView->originX, blockView->originX + imageNx*cubeSize); */
/*
								  blockView->originY + halfCubeSize,
								  blockView->originY + (imageNy-1)*cubeSize + halfCubeSize,
								  blockView->originX + halfCubeSize,
								  blockView->originX + (imageNx-1)*cubeSize + halfCubeSize);
*/
	}

	if (xyzCalc)
	{
		for (x = 0; x < numPointsToMeasure; x++)
		{
			pointPtr->x = pointPos[x][0];
			pointPtr->y = pointPos[x][1];
			pointPtr->z = pointPos[x][2];
			pointPtr++;
		}
	}
	else
	{
		if (options->drapedSurvey && topoOverlay)  /* Must have the topoOverlay to calculate */
		{
			for (x = 0; x < imageNx; x++)
			{
				for (y = 0; y < imageNy; y++)
				{
					pointPtr->x = blockView->originX + x*cubeSize + halfCubeSize;
					pointPtr->y = blockView->originY + (y*cubeSize + halfCubeSize);
					if ((pointPtr->x > TopomapXW) && (pointPtr->x < TopomapXE)
					 && (pointPtr->y > TopomapYW) && (pointPtr->y < TopomapYE))
						pointPtr->z = topoOverlay[x+1][imageNy-y/*y+1*/] + surveyHeight; /* Changed 11/3/98 (PM) */
					else
						pointPtr->z = blockView->originZ + surveyHeight;
/*
					topoHeight = getTopoValueAtLoc (topographyMap, TopoCol, TopoRow,
											  TopomapYW, TopomapYE, TopomapXW, TopomapXE,
											  pointPtr->x, pointPtr->y, &error);
					if (error)
						pointPtr->z = blockView->originZ + surveyHeight;
					else
						pointPtr->z = topoHeight + surveyHeight;
*/
					pointPtr++;
				}
			}
		}
		else  /* Just Flat */
		{
			for (x = 0; x < imageNx; x++)
			{
				for (y = 0; y < imageNy; y++)
				{
					pointPtr->x = blockView->originX + x*cubeSize + halfCubeSize;
					pointPtr->y = blockView->originY + (y*cubeSize + halfCubeSize);
					pointPtr->z = blockView->originZ + surveyHeight;
					pointPtr++;
				}
			}
		}
	}
										/* ****************************** */
										/* Calculate from bottom layer up */
	if (batchExecution)
		totalCubes = numLayers*nx*ny;
	minX = blockView->originX - calcRangeInCubes*cubeSize;
	minY = blockView->originY - calcRangeInCubes*cubeSize;
	minZ = blockView->originZ - cubeSize; /* -cubeSize so we get the min value */
	for (z = numLayers-1, zPos = minZ; z >= 0; z--, zPos -= cubeSize)
   {
		for (x = 0; x < nx; x++)
		{
			incrementLongJob(INCREMENT_JOB);

			xPos = minX + x*cubeSize;
			for (y = 0; y < ny; y++)
			{
				if (batchExecution)
				{
					cubeNum++;
					sprintf(statusMessage, "Calculating cube %d or %d\n", cubeNum, totalCubes);
					updateBatchStatus(statusMessage);
				}

				if (growCubes)
				{
					if (!getCubeToCalculate(x, y, z, nx, ny, numLayers, minX, minY, minZ, cubeSize, cubeUsed,
													&cubeXMin, &cubeYMin, &cubeZMin, &cubeXMax, &cubeYMax, &cubeZMax,
													edgePadding, topoOverlay, imageNx, imageNy,
													indexData, propArray, densityData, magSusData,
													remSusDecData,   remSusAziData,   remSusStrData,
													aniSusDipData,   aniSusDDirData,  aniSusPitchData,
													aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data))
						continue;
				}
				else
				{
					cubeXMin = xPos;
					cubeYMin = minY + ((ny-1)-y)*cubeSize /* minY + y*cubeSize CHANGE */;
					cubeZMin = zPos;
					cubeXMax = cubeXMin + cubeSize;
					cubeYMax = cubeYMin + cubeSize;
					cubeZMax = cubeZMin + cubeSize;
				}
				cubeXCenter = cubeXMin + ((cubeXMax - cubeXMin) / 2.0);
				cubeYCenter = cubeYMin + ((cubeYMax - cubeYMin) / 2.0);
				cubeZCenter = cubeZMin + ((cubeZMax - cubeZMin) / 2.0);

				if (cubeZMax <= cubeZMin)  /* Skip Zero size points */
				{
					skipPointMag = TRUE;
					skipPointGrv = TRUE;
				}
				else
				{									/* Calculate the Geophysics for this cube */
					getFieldAtLocation (options, cubeXCenter, cubeYCenter, cubeZCenter,
							                       &inclination, &declination, &intensity);
				}

				if ((!skipPointMag) && (magneticResults || magXResults || magYResults || magZResults))
				{
					if (getMagneticVectors(&blockSusX, &blockSusY, &blockSusZ,
										  intensity, inclination, declination,
										  x, y, z, indexData, propArray, magSusData,
										  remSusDecData,   remSusAziData,   remSusStrData,
										  aniSusDipData,   aniSusDDirData,  aniSusPitchData,
										  aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data))
						skipPointMag = FALSE;
					else
						skipPointMag = TRUE;
				}

				if ((!skipPointGrv) && (gravityResults))
				{
					if (getGravityVector(&blockDensity,
										  x, y, z, indexData, propArray, densityData))
						skipPointGrv = FALSE;
					else
						skipPointGrv = TRUE;
				}

				if (skipPointMag && skipPointGrv)  /* Points are skiped if the mag and gravity are zero */
				{
					skipPointMag = FALSE;  /* Only Skip this one not the next */
					skipPointGrv = FALSE;  /* Only Skip this one not the next */
				}
				else
				{
					calcAnomAtPoints(cubeXMin, cubeYMin, cubeZMin, cubeXMax, cubeYMax, cubeZMax,
							 blockSusX, blockSusY, blockSusZ, blockDensity, intensity,
							 inclination, declination, pointsToMeasure, numPointsToMeasure,
							 magneticResults, magXResults, magYResults, magZResults,
							 gravityResults, TRUE, cubeRange, sphereRange);
				}
			}
		}

      if (abortLongJob ())
         return (FALSE);
   }

						/* *********************************** */
						/* ****** Output of result data ****** */
   if (xyzCalc)
	{
		calcMagFromComponents((double **) NULL, (double **) NULL, (double **) NULL, (double **) NULL,
									 magneticResults, magXResults, magYResults, magZResults,
									 numPointsToMeasure, 1, 0, options,
									 inclination, declination, intensity);
		if (!options->magneticVectorComponents)
		{
			destroy1DArray ((char *) magXResults);
			destroy1DArray ((char *) magYResults);
			destroy1DArray ((char *) magZResults);
			magXResults = magYResults = magZResults = NULL;
		}

		writeXYZFile(outputFilename, xyzImportFile, pointsToMeasure, numPointsToMeasure,
			  gravityResults, magneticResults, magXResults, magYResults, magZResults);
	}
	else
	{
		if (!options->projectVectorsOntoField)
		{                                /* Combine Vector Components into mag Image */
			calcMagFromComponents((double **) NULL, (double **) NULL, (double **) NULL, (double **) NULL,
										 magneticResults, magXResults, magYResults, magZResults,
										 imageNx, imageNy, 0, options,
										 inclination, declination, intensity);
			if (!options->magneticVectorComponents)
			{
				destroy1DArray ((char *) magXResults);
				destroy1DArray ((char *) magYResults);
				destroy1DArray ((char *) magZResults);
				magXResults = magYResults = magZResults = NULL;
			}
		}

		if (topoOverlay)
			destroy2DArray ((char **) topoOverlay, imageNx+1, imageNy+1);

		if (gravityResults)
			writeAnomFile (outputFilename, NULL, gravityResults, GRAVITY,
					  imageNx, imageNy, numLayers, 0, blockView, options);

		if (magneticResults)
			writeAnomFile (outputFilename, NULL, magneticResults, MAGNETICS,
					  imageNx, imageNy, numLayers, 0, blockView, options);

		if (magXResults)
			writeAnomFile (outputFilename, NULL, magXResults, MAGNETICS_X,
					  imageNx, imageNy, numLayers, 0, blockView, options);

		if (magYResults)
			writeAnomFile (outputFilename, NULL, magYResults, MAGNETICS_Y,
					  imageNx, imageNy, numLayers, 0, blockView, options);

		if (magZResults)
			writeAnomFile (outputFilename, NULL, magZResults, MAGNETICS_Z,
					  imageNx, imageNy, numLayers, 0, blockView, options);

		if (grvImage)
			fillAnomImage (NULL, gravityResults, GRAVITY, imageNx, imageNy, 0, grvImage);

		if (magImage)
			fillAnomImage (NULL, magneticResults, MAGNETICS, imageNx, imageNy, 0, magImage);
	}
			                   /* ************************ */
			                   /* Free Memory we have used */
	if (pointsToMeasure)
		destroy1DArray((char *) pointsToMeasure);
	if (gravityResults)
		destroy1DArray((char *) gravityResults);
	if (magneticResults)
		destroy1DArray((char *) magneticResults);
	if (magXResults)
		destroy1DArray((char *) magXResults);
	if (magYResults)
		destroy1DArray((char *) magYResults);
	if (magZResults)
		destroy1DArray((char *) magZResults);
	if (cubeUsed)
		destroy3DArray((char ***) cubeUsed, numLayers, nx, ny);

   return (TRUE);
}

/* *****************************************************************
FUNCTION getMagneticVectors

INPUT

OUTPUT

DESCRIPTION 
   Get the magnetics field values needed to pass into 'calcAnomAtPoints'

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int getCubeToCalculate(x, y, z, nx, ny, nz, minX, minY, maxZ, cubeSize, cubeUsed,
				cubeXMin, cubeYMin, cubeZMin, cubeXMax, cubeYMax, cubeZMax,
				edgePadding, topoOverlay, topoNx, topoNy, 
				indexData, propArray, densityData, magSusData,
				remSusDecData,   remSusAziData,   remSusStrData,
				aniSusDipData,   aniSusDDirData,  aniSusPitchData,
				aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data)
int x, y, z, nx, ny, nz;
double minX, minY, maxZ, cubeSize;
short ***cubeUsed;
double *cubeXMin, *cubeYMin, *cubeZMin, *cubeXMax, *cubeYMax, *cubeZMax;
double edgePadding;
double **topoOverlay;
int topoNx, topoNy;
short ***indexData;
LAYER_PROPERTIES **propArray;
float ***densityData,    ***magSusData,
      ***remSusDecData,  ***remSusAziData,  ***remSusStrData,
		***aniSusDipData,  ***aniSusDDirData, ***aniSusPitchData,
		***aniSusAxis1Data,***aniSusAxis2Data,***aniSusAxis3Data;
{
	int result = FALSE;
	int xCheck, yCheck, zCheck;
	int xGroup, yGroup, zGroup;
	int xMatching, yMatching, zMatching;
	int value, variableBlocks;
	float densityValue,     magSusValue;
	float remSusDecValue,   remSusAziValue,   remSusStrValue;
	float aniSusDipValue,   aniSusDDirValue,  aniSusPitchValue;
	float aniSusAxis1Value, aniSusAxis2Value, aniSusAxis3Value;
	double cubeZMaxNoTopoCorrect;

					/* Note z = 0 -> block bottom     z = nz-1 -> top (highest) block */
	            /*      y = 0 -> min block			 y = ny-1 -> max block */
					/*		  x = 0 -> min block			 x = nx-1 -> max block */
	if (cubeUsed[z][x][y])
		return (FALSE);

	            /* Claim The initial cube as the block so far */
	zGroup = 1;
	yGroup = 1;
	xGroup = 1;
	cubeUsed[z][x][y] = TRUE;
	*cubeXMin = minX + x*cubeSize;
	*cubeYMin = /*minY + y*cubeSize; CHANGE */ minY + ((ny-1)-y)*cubeSize;
	*cubeZMin = maxZ - (nz-z-1)*cubeSize;
	*cubeXMax = *cubeXMin + cubeSize;
	*cubeYMax = *cubeYMin + cubeSize;
	*cubeZMax = *cubeZMin + cubeSize;

	cubeZMaxNoTopoCorrect = *cubeZMax;

	if (indexData)
		value = indexData[z][x][y];
	if (densityData)
		densityValue = densityData[z][x][y];
	if (magSusData)
		magSusValue = magSusData[z][x][y];
	if (remSusDecData)
		remSusDecValue = remSusDecData[z][x][y];
	if (remSusAziData)
		remSusAziValue = remSusAziData[z][x][y];
	if (remSusStrData)
		remSusStrValue = remSusStrData[z][x][y];
	if (aniSusDipData)
		aniSusDipValue = aniSusDipData[z][x][y];
	if (aniSusDDirData)
		aniSusDDirValue = aniSusDDirData[z][x][y];
	if (aniSusPitchData)
		aniSusPitchValue = aniSusPitchData[z][x][y];
	if (aniSusAxis1Data)
		aniSusAxis1Value = aniSusAxis1Data[z][x][y];
	if (aniSusAxis2Data)
		aniSusAxis2Value = aniSusAxis2Data[z][x][y];
	if (aniSusAxis3Data)
		aniSusAxis3Value = aniSusAxis3Data[z][x][y];
	
	if  (remSusDecData   && remSusAziData   && remSusStrData   &&
        aniSusDipData   && aniSusDDirData  && aniSusPitchData &&
        aniSusAxis1Data && aniSusAxis2Data && aniSusAxis3Data)
		variableBlocks = TRUE;
	else
		variableBlocks = FALSE;

	xMatching = TRUE;   /* Assume they will all match */
	yMatching = TRUE;   /* Assume they will all match */
	zMatching = TRUE;   /* Assume they will all match */


			/* ************************************************************************ */
			/* ******** Allow exact locations of topo Surface to be calculated ******** */
	if (topoOverlay)  /* Make sure the topography is exact */
	{
/*		if ((*cubeXMin >= TopomapXW) && (*cubeXMax <= TopomapXE) */
/*		 && (*cubeYMin >= TopomapYW) && (*cubeYMax <= TopomapYE)) */
		if ((*cubeXMin >= TopomapXW) && (*cubeXMax <= TopomapXE)
		 && (*cubeYMin >= TopomapYW) && (*cubeYMax <= TopomapYE))
		{
			double topoValue;
			int xTopoIndex, yTopoIndex;

			xTopoIndex = x - (nx-topoNx)/2 + 1;
			yTopoIndex = y - (ny-topoNy)/2 + 1;

			topoValue = topoOverlay[xTopoIndex][/*topoNy-(yTopoIndex-1)*/yTopoIndex];  /* Changed 11/3/98 (PM) */
			if (*cubeZMax > topoValue)
			{     /* Dont do coellecing in X or Y if in topo range */
				*cubeZMax = topoValue;
				xMatching = yMatching = FALSE;
			}
		}

		if ((*cubeZMax >= minTopoValue) && (*cubeZMax <= maxTopoValue))
		{     /* Dont do coellecing in X or Y if in topo range */
			xMatching = yMatching = FALSE;
		}

		if (*cubeZMax < *cubeZMin)  /* do no matching if the cube has been wiped */
		{
			zMatching = FALSE;
			return (FALSE);
		}
	}

	if (xMatching || yMatching || zMatching)
	{
		do   /* Try and Grow the cube in the X then Y then Z Directions until we cannot any more */
		{
												/* ****************************************************** */
												/* ***********        X Growth Check          *********** */
												/* ****************************************************** */
			if (xMatching)
			{
				xCheck = x+xGroup;  /* Check to see if we can grow in the x Direction */
				if (xCheck < nx)
				{							  /* Check the whole face in the X-Dir to see if it matches */
					for (zCheck = z; zCheck > (z-zGroup); zCheck--)
					{
						for (yCheck = y; yCheck < (y+yGroup); yCheck++)
						{
							if (cubeUsed[zCheck][xCheck][yCheck])
								xMatching = FALSE;
							else if  (indexData && (indexData[zCheck][xCheck][yCheck] != value))
								xMatching = FALSE;
							else if  (densityData && (densityData[zCheck][xCheck][yCheck] != densityValue))
								xMatching = FALSE;
							else if  (magSusData && (magSusData[zCheck][xCheck][yCheck] != magSusValue))
								xMatching = FALSE;
							else if  (variableBlocks &&
										 ((remSusDecData[zCheck][xCheck][yCheck] != remSusDecValue) ||
										  (remSusAziData[zCheck][xCheck][yCheck] != remSusAziValue) ||
										  (remSusStrData[zCheck][xCheck][yCheck] != remSusStrValue) ||
										  (aniSusDipData[zCheck][xCheck][yCheck] != aniSusDipValue) ||
										  (aniSusDDirData[zCheck][xCheck][yCheck] != aniSusDDirValue) ||
										  (aniSusPitchData[zCheck][xCheck][yCheck] != aniSusPitchValue) ||
										  (aniSusAxis1Data[zCheck][xCheck][yCheck] != aniSusAxis1Value) ||
										  (aniSusAxis2Data[zCheck][xCheck][yCheck] != aniSusAxis2Value) ||
										  (aniSusAxis3Data[zCheck][xCheck][yCheck] != aniSusAxis3Value)))
								xMatching = FALSE;
						}
					}
					if (xMatching)  /* If they all matched we can claim them and enlarge the block */
					{
						xGroup++;
						*cubeXMax = *cubeXMin + xGroup*cubeSize;
						for (zCheck = z; zCheck > (z-zGroup); zCheck--) /* Record that all the cubes on that face are used */
							for (yCheck = y; yCheck < (y+yGroup); yCheck++)
								cubeUsed[zCheck][xCheck][yCheck] = TRUE;
					}
				}
				else
					xMatching = FALSE;   /* We are at the Edge of the block so no more matches */
			}
												/* ****************************************************** */
												/* ***********        Y Growth Check          *********** */
												/* ****************************************************** */
			if (yMatching)
			{
				/* yCheck = y+yGroup; CHANGE */  /* Check to see if we can grow in the x Direction */
				yCheck = y-yGroup;
				if (yCheck < ny)
				{							  /* Check the whole face in the X-Dir to see if it matches */
					for (zCheck = z; zCheck > (z-zGroup); zCheck--)
					{
						for (xCheck = x; xCheck < (x+xGroup); xCheck++)
						{
							if (cubeUsed[zCheck][xCheck][yCheck])
								yMatching = FALSE;
							else if  (indexData && (indexData[zCheck][xCheck][yCheck] != value))
								yMatching = FALSE;
							else if  (densityData && (densityData[zCheck][xCheck][yCheck] != densityValue))
								yMatching = FALSE;
							else if  (magSusData && (magSusData[zCheck][xCheck][yCheck] != magSusValue))
								yMatching = FALSE;
							else if  (variableBlocks &&
										 ((remSusDecData[zCheck][xCheck][yCheck] != remSusDecValue) ||
										  (remSusAziData[zCheck][xCheck][yCheck] != remSusAziValue) ||
										  (remSusStrData[zCheck][xCheck][yCheck] != remSusStrValue) ||
										  (aniSusDipData[zCheck][xCheck][yCheck] != aniSusDipValue) ||
										  (aniSusDDirData[zCheck][xCheck][yCheck] != aniSusDDirValue) ||
										  (aniSusPitchData[zCheck][xCheck][yCheck] != aniSusPitchValue) ||
										  (aniSusAxis1Data[zCheck][xCheck][yCheck] != aniSusAxis1Value) ||
										  (aniSusAxis2Data[zCheck][xCheck][yCheck] != aniSusAxis2Value) ||
										  (aniSusAxis3Data[zCheck][xCheck][yCheck] != aniSusAxis3Value)))
								yMatching = FALSE;
						}
					}
					if (yMatching)  /* If they all matched we can claim them and enlarge the block */
					{
						yGroup++;
						/* *cubeYMax = *cubeYMin + yGroup*cubeSize; CHANGE */
						*cubeYMin = *cubeYMax - yGroup*cubeSize;
						for (zCheck = z; zCheck > (z-zGroup); zCheck--) /* Record that all the cubes on that face are used */
							for (xCheck = x; xCheck < (x+xGroup); xCheck++)
								cubeUsed[zCheck][xCheck][yCheck] = TRUE;
					}
				}
				else
					yMatching = FALSE;   /* We are at the Edge of the block so no more matches */
			}
												/* ****************************************************** */
												/* ***********        Z Growth Check          *********** */
												/* ****************************************************** */
			if (zMatching)
			{
				zCheck = z-zGroup;  /* Check to see if we can grow in the x Direction */
				if (zCheck >= 0)
				{							  /* Check the whole face in the X-Dir to see if it matches */
					for (yCheck = y; yCheck < (y+yGroup); yCheck++)
					{
						for (xCheck = x; xCheck < (x+xGroup); xCheck++)
						{
							if (cubeUsed[zCheck][xCheck][yCheck])
								zMatching = FALSE;
							else if  (indexData && (indexData[zCheck][xCheck][yCheck] != value))
								zMatching = FALSE;
							else if  (densityData && (densityData[zCheck][xCheck][yCheck] != densityValue))
								zMatching = FALSE;
							else if  (magSusData && (magSusData[zCheck][xCheck][yCheck] != magSusValue))
								zMatching = FALSE;
							else if  (variableBlocks &&
										 ((remSusDecData[zCheck][xCheck][yCheck] != remSusDecValue) ||
										  (remSusAziData[zCheck][xCheck][yCheck] != remSusAziValue) ||
										  (remSusStrData[zCheck][xCheck][yCheck] != remSusStrValue) ||
										  (aniSusDipData[zCheck][xCheck][yCheck] != aniSusDipValue) ||
										  (aniSusDDirData[zCheck][xCheck][yCheck] != aniSusDDirValue) ||
										  (aniSusPitchData[zCheck][xCheck][yCheck] != aniSusPitchValue) ||
										  (aniSusAxis1Data[zCheck][xCheck][yCheck] != aniSusAxis1Value) ||
										  (aniSusAxis2Data[zCheck][xCheck][yCheck] != aniSusAxis2Value) ||
										  (aniSusAxis3Data[zCheck][xCheck][yCheck] != aniSusAxis3Value)))
								zMatching = FALSE;
						}
					}
					if (zMatching)  /* If they all matched we can claim them and enlarge the block */
					{
						zGroup++;
						*cubeZMin = cubeZMaxNoTopoCorrect - zGroup*cubeSize;
						for (yCheck = y; yCheck < (y+yGroup); yCheck++) /* Record that all the cubes on that face are used */
							for (xCheck = x; xCheck < (x+xGroup); xCheck++)
								cubeUsed[zCheck][xCheck][yCheck] = TRUE;
					}
				}
				else
					zMatching = FALSE;   /* We are at the Edge of the block so no more matches */
			}
		} while (xMatching || yMatching || zMatching);
	}  /* endif (xMatching || yMatching || zMatching) */

	/* If this cube is an edge Cube then extend it by the amount of the padding */
	if (edgePadding > 0.0)
	{
		if (fabs(*cubeXMin-minX) < TOLERANCE)
			*cubeXMin -= edgePadding;
		if (fabs(*cubeXMax-(minX+nx*cubeSize)) < TOLERANCE)
			*cubeXMax += edgePadding;

		if (fabs(*cubeYMin-minY) < TOLERANCE)
			*cubeYMin -= edgePadding;
		if (fabs(*cubeYMax-(minY+ny*cubeSize)) < TOLERANCE)
			*cubeYMax += edgePadding;
	}

	return (TRUE);
}

/* *****************************************************************
FUNCTION createAnomFilter

INPUT

OUTPUT

DESCRIPTION 
   calculate the convolution Filter used in the calculation in the
   calculation of the anomalies images. Basically it is an array of
   the amount of influence the surrounding cells have on the
   center cell.

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int createAnomFilter(magXFilter, magYFilter, magZFilter, magTotalFilter,
							gravityFilter, magP1Filter, magP2Filter,
                     xmax, ymax, options, cubeSize, zPos,
							inclination, declination, intensity)
double **magXFilter;
double **magYFilter;
double **magZFilter;
double **magTotalFilter;
double **gravityFilter;
double ***magP1Filter;
double ***magP2Filter;
int xmax, ymax;
GEOPHYSICS_OPTIONS *options;
int cubeSize;
double zPos;
double inclination, declination, intensity;
{
#define NUM_X_STEPS 2
#define NUM_Y_STEPS 2
#define NUM_Z_STEPS 2
   double earthField[3];
   int gravityCalc, magneticsCalc;
   double sinInclination, cosInclination, sinDeclination, cosDeclination;
   double cosDecCosInc, sinDecCosInc;
   double fieldPointX, fieldPointY, fieldPointZ;
   double fieldPointMinX, fieldPointMinY, fieldPointMaxX, fieldPointMaxY;
   double xDist[NUM_X_STEPS], yDist[NUM_Y_STEPS], zDist[NUM_Z_STEPS], distance;
   register double sign, stepInc;
   register int i, j, k, step;
   register int q, p;
   double xLength, yLength, xLengthSoFar, yLengthSoFar, halfCubeSize;
   double to, phi1, phi3, phi4;
   double p1u, p1v, p1w, p2u, p2v, p2w;

   if (gravityFilter)  /* Check if gravity is to be calculated */
      gravityCalc = TRUE;
   else
      gravityCalc = FALSE;

                       /* Check if magnetics is to be calculated */
   if (magXFilter && magYFilter && magZFilter && magTotalFilter)
      magneticsCalc = TRUE;
   else
      magneticsCalc = FALSE;

	
                       /* Size of the convolution mask in meters */
   xLength = xmax*cubeSize;
   yLength = ymax*cubeSize;
   halfCubeSize = (float) (cubeSize/2.0);
   
     /* The position of the Point where the field will be measured from
     ** is directly above the center point of the convolution mask */
   fieldPointX = xLength/2.0;
   fieldPointY = yLength/2.0;
   fieldPointZ = zPos;
          /* define boundaries of fieldPoint cube */
   fieldPointMinX = fieldPointX - halfCubeSize;
   fieldPointMinY = fieldPointY - halfCubeSize;
   fieldPointMaxX = fieldPointX + halfCubeSize;
   fieldPointMaxY = fieldPointY + halfCubeSize;

               /* calculate useful constants we will be using */
   sinInclination = sin(DEG_TO_RAD(inclination));
   cosInclination = cos(DEG_TO_RAD(inclination));
   sinDeclination = sin(DEG_TO_RAD(declination));
   cosDeclination = cos(DEG_TO_RAD(declination));
   cosDecCosInc = cosDeclination*cosInclination;
   sinDecCosInc = sinDeclination*cosInclination;

               /* calculate components of geomagnetic field  */
   earthField[0] = intensity * cosInclination * sinDeclination;
   earthField[1] = intensity * cosInclination * cosDeclination;
   earthField[2] = intensity * sinInclination;

                               /* z corners of the cube that feild will be calculated for */
   for (step = 0, stepInc = (double) cubeSize/ (double) (NUM_Z_STEPS-1); step < NUM_Z_STEPS; step++)
   {
      zDist[step] = fieldPointZ + halfCubeSize   /* top of cube plus steps */
                                - stepInc*step;  /* down to the bottom */
      if (fabs(zDist[step]) < HIGH_TOLERANCE)    /* make sure we cant have any divide by zeros later */
         zDist[step] += HIGH_TOLERANCE;
   }
   
              /* solve volume integral for every cubeSize point */
   for (p = 0; p < xmax; p++)
   {
      incrementLongJob (INCREMENT_JOB);

      xLengthSoFar = cubeSize*p;
      for (step = 0, stepInc = (double) cubeSize/ (double) (NUM_X_STEPS-1); step < NUM_X_STEPS; step++)
      {
         xDist[step] = xLengthSoFar - fieldPointX    /* top of cube plus steps */
                                    + stepInc*step;  /* down to the bottom */
         if (fabs(xDist[step]) < HIGH_TOLERANCE)    /* make sure we cant have any divide by zeros later */
            xDist[step] += HIGH_TOLERANCE;
      }

      for (q = 0; q < ymax; q++)
      {
         yLengthSoFar = cubeSize*q;
         for (step = 0, stepInc = (double) cubeSize/ (double) (NUM_Y_STEPS-1); step < NUM_Y_STEPS; step++)
         {
            yDist[step] = yLengthSoFar - fieldPointY    /* top of cube plus steps */
                                       + stepInc*step;  /* down to the bottom */
            if (fabs(yDist[step]) < HIGH_TOLERANCE)    /* make sure we cant have any divide by zeros later */
               yDist[step] += HIGH_TOLERANCE;
         }
         
         p1u = p1v = p1w = 0.0;
         p2u = p2v = p2w = 0.0;
         to = 0.0;
         
         for (i = 0; i < NUM_X_STEPS; i++)
         {
            for (j = 0; j < NUM_Y_STEPS; j++)
            {
               for (k = 0; k < NUM_Z_STEPS; k++)
               {
                  sign = pow((double)(-1), (double)(i+1)+(j+1)+(k+1));
                  
                                               /* Intragral for gravity mask */
                  distance = (float) sqrt(xDist[i]*xDist[i]+yDist[j]*yDist[j]+zDist[k]*zDist[k]);
                  phi1 = sign * atan(yDist[j]*xDist[i] / (zDist[k]*distance));
                  phi3 = sign * log(yDist[j] + distance);
                  phi4 = sign * log(xDist[i] + distance);
                  to += -phi1*zDist[k] + phi3*xDist[i] + phi4*yDist[j];

                                               /* Intragral for magnetics mask */
                  p1u += sign * atan(yDist[j]*zDist[k] / (xDist[i]*distance));
                  p1v += sign * atan(-zDist[k]*xDist[i] / (yDist[j]*distance));
                  p1w += phi1;
                  p2u += phi4;
                  p2v += phi3;
                  p2w += sign * log(zDist[k] + distance);
               }
            }
         }

         if (magneticsCalc)
         {
            magXFilter[p][q] = (cosDecCosInc*p2w - sinDecCosInc*p1u
                                                 + sinInclination*p2v);
            magYFilter[p][q] = (cosDecCosInc*p1v + sinDecCosInc*p2w 
                                                 + sinInclination*p2u);
            magZFilter[p][q] = (cosDecCosInc*p2u + sinDecCosInc*p2v 
                                                 - sinInclination*p1w);
            magTotalFilter[p][q] = earthField[0]*magXFilter[p][q]
                                 + earthField[1]*magYFilter[p][q]
                                 + earthField[2]*magZFilter[p][q];
         }

			if (magP1Filter)
			{
            magP1Filter[0][p][q] = p1u;
            magP1Filter[1][p][q] = p1v;
            magP1Filter[2][p][q] = p1w;
			}
			if (magP2Filter)
			{
            magP2Filter[0][p][q] = p2u;
            magP2Filter[1][p][q] = p2v;
            magP2Filter[2][p][q] = p2w;
			}

         if (gravityCalc)
            gravityFilter[p][q] = to*GRAVITY_CONSTANT;
      }
   }

   return (TRUE);
}

/* *****************************************************************
FUNCTION calcAnomAtPoints

INPUT

OUTPUT

DESCRIPTION 
   calculate the magnetic or gravity for a block at specific XYZ locations

RETURN   # points processed - sucess
			FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
calcAnomAtPoints(double blockMinX, double blockMinY, double blockMinZ,
					  double blockMaxX, double blockMaxY, double blockMaxZ,
					  double blockSusX, double blockSusY, double blockSusZ,
					  double blockDensity, double bgFieldIntensity,
					  double bgFieldInclination, double bgFieldDeclination,
					  Point3d *pointsToMeasure, int numPointsToMeasure,
					  double *magneticResults,
					  double *magXResults, double *magYResults, double *magZResults,
					  double *gravityResults, int addToResult, double cubeRange, double sphereRange)
#else
calcAnomAtPoints(blockMinX, blockMinY, blockMinZ,
					  blockMaxX, blockMaxY, blockMaxZ,
					  blockSusX, blockSusY, blockSusZ,
					  blockDensity, bgFieldIntensity,
					  bgFieldInclination, bgFieldDeclination,
					  pointsToMeasure, numPointsToMeasure,
					  magneticResults, magXResults, magYResults, magZResults,
					  gravityResults, addToResult, cubeRange, sphereRange)
double blockMinX, blockMinY, blockMinZ;
double blockMaxX, blockMaxY, blockMaxZ;
double blockSusX, blockSusY, blockSusZ;
double blockDensity, bgFieldIntensity, bgFieldInclination, bgFieldDeclination;
Point3d *pointsToMeasure;
int numPointsToMeasure;
double *magneticResults, *magXResults, *magYResults, *magZResults, *gravityResults;
int addToResult;
double cubeRange, sphereRange;
#endif
{
#define ZERO 0.0000000001
   double bgField[3]; /* Components */
   int gravityCalc = FALSE, magneticsCalc = FALSE;
   double sinInclination, cosInclination, sinDeclination, cosDeclination;
   double cosDecCosInc, sinDecCosInc;
   double xDist[NUM_X_STEPS], yDist[NUM_Y_STEPS], zDist[NUM_Z_STEPS], distance;
   register double sign;
   register int i, j, k;
   int point, underSurface, forceAboveGround = FALSE;
	double aboveGroundHeight = 1.0;
   register double to, phi1, phi3, phi4;
   double p1u, p1v, p1w, p2u, p2v, p2w;
	double cubeSizeX, cubeSizeY, cubeSizeZ;
	double gravity, magneticsX, magneticsY, magneticsZ; /* magneticsTotal; */
	double *magResultPtr = NULL, *magXResultPtr = NULL, *magYResultPtr = NULL, *magZResultPtr = NULL;
	double *grvResultPtr = NULL, distanceToPoint;
	Point3d *pointPtr, cubeCentre, distanceVector;
	double nullValue = -999999.0;
	int nullPoint = FALSE;
	char *heightPtr;

	if (!pointsToMeasure || !numPointsToMeasure)
		return (FALSE);

	if (getenv ("NODDY_ABOVE_GROUND"))
		forceAboveGround = TRUE;
	if (heightPtr = getenv ("NODDY_ABOVE_HEIGHT"))
		sscanf(heightPtr, "%lf", &aboveGroundHeight);


   if (grvResultPtr = gravityResults)  /* Check if grv to be calculated */
      gravityCalc = TRUE;
   if (magResultPtr = magneticResults) /* Check if mag to be calculated */
      magneticsCalc = TRUE;
   if (magXResultPtr = magXResults) /* Check if magX to be calculated */
      magneticsCalc = TRUE;
   if (magYResultPtr = magYResults) /* Check if magY to be calculated */
      magneticsCalc = TRUE;
   if (magZResultPtr = magZResults) /* Check if magZ to be calculated */
      magneticsCalc = TRUE;

	if (!(gravityCalc || magneticsCalc))  /* Not Calculating anything */
		return (FALSE);

							                         /* Skip if block is zero */
	if ((!magneticsCalc &&  (fabs(blockDensity) < ZERO))  /* Only Gravity being calculated */
	 || (!gravityCalc   && ((fabs(blockSusX) < ZERO) && (fabs(blockSusY) < ZERO)   /* Only Magnetics */
	                                                 && (fabs(blockSusZ) < ZERO)))
    || ((fabs(blockDensity) < ZERO) && (fabs(blockSusX) < ZERO)
	     && (fabs(blockSusY) < ZERO) && (fabs(blockSusZ) < ZERO)))
	{
		if (!addToResult)  /* If adding then we are adding zero so skip it */
		{
			for (point = 0, pointPtr = pointsToMeasure;
								 point < numPointsToMeasure; point++, pointPtr++)
			{
				if (gravityCalc)
					*(grvResultPtr++) = 0.0;
				if (magneticsCalc)
					*(magResultPtr++) = 0.0;
			}
		}
		return (TRUE);
	}
                       /* Size of the cube we are calculating for */
	cubeSizeX = blockMaxX - blockMinX;
	cubeSizeY = blockMaxY - blockMinY;
	cubeSizeZ = blockMaxZ - blockMinZ;

	cubeCentre.x = blockMinX + cubeSizeX/2.0;
	cubeCentre.y = blockMinY + cubeSizeY/2.0;
	cubeCentre.z = blockMinZ + cubeSizeZ/2.0;

                       /* calculate useful constants we will be using */
   sinInclination = sin(DEG_TO_RAD(bgFieldInclination));
   cosInclination = cos(DEG_TO_RAD(bgFieldInclination));
   sinDeclination = sin(DEG_TO_RAD(180.0-bgFieldDeclination));
   cosDeclination = cos(DEG_TO_RAD(180.0-bgFieldDeclination));
   cosDecCosInc = cosDeclination*cosInclination;
   sinDecCosInc = sinDeclination*cosInclination;

                       /* calculate components of geomagnetic field  */
   bgField[0] = bgFieldIntensity * cosInclination * sinDeclination;
   bgField[1] = bgFieldIntensity * cosInclination * cosDeclination;
   bgField[2] = bgFieldIntensity * sinInclination;

                       /* solve volume integral for every cubeSize point */
	for (point = 0, pointPtr = pointsToMeasure;
	                point < numPointsToMeasure; point++, pointPtr++)
   {
		if (cubeRange >= 0.0)  /* Only calculate Points inside Range influence */
		{
			SubPt3d(pointPtr, &cubeCentre, &distanceVector);
			distanceToPoint = sqrt(distanceVector.x*distanceVector.x +  /* Cylinder */
			                       distanceVector.y*distanceVector.y);
		}
		else
			distanceToPoint = cubeRange - 1.0;  /* Less than cubeRange which is < 0.0 */

		if (distanceToPoint < cubeRange)
		{
						 /* Distance from Measure point to the corners of the block that has the field */
			xDist[0] = (pointPtr->x - blockMinX);  /* X Start */
			xDist[1] = (pointPtr->x - blockMaxX);  /* X End */
			yDist[0] = (pointPtr->y - blockMinY);  /* Y Start */
			yDist[1] = (pointPtr->y - blockMaxY);  /* Y End */
			zDist[0] = (pointPtr->z - blockMinZ);  /* Z Start */
			zDist[1] = (pointPtr->z - blockMaxZ);  /* Z End */
             /* make sure we cant have any divide by zeros later */
			if (fabs(xDist[0]) < HIGH_TOLERANCE) xDist[0] += HIGH_TOLERANCE;
			if (fabs(xDist[1]) < HIGH_TOLERANCE) xDist[1] += HIGH_TOLERANCE;
			if (fabs(yDist[0]) < HIGH_TOLERANCE) yDist[0] += HIGH_TOLERANCE;
			if (fabs(yDist[1]) < HIGH_TOLERANCE) yDist[1] += HIGH_TOLERANCE;
			if (fabs(zDist[0]) < HIGH_TOLERANCE) zDist[0] += HIGH_TOLERANCE;
			if (fabs(zDist[1]) < HIGH_TOLERANCE) zDist[1] += HIGH_TOLERANCE;

			                   /* Observation point is inside the block being calculated */
			if ((pointPtr->x >= blockMinX) && (pointPtr->x < blockMaxX) &&
				 (pointPtr->y >= blockMinY) && (pointPtr->y < blockMaxY) &&
				 (pointPtr->z >= blockMinZ) && (pointPtr->z < blockMaxZ))
			{
				if (forceAboveGround)
				{
					{
						FILE *fo;
						fo = fopen("./under.txt","a");
						fprintf(fo,"Under Surface At (%.1lf, %.1lf, %.1lf)   Block (%.1lf, %.1lf, %.1lf)-(%.1lf, %.1lf, %.1lf)\n", pointPtr->x, pointPtr->y, pointPtr->z, blockMinX, blockMinY, blockMinZ, blockMaxX, blockMaxY, blockMaxZ);
						fclose(fo);
					}
					underSurface = FALSE;
					pointPtr->z = blockMaxZ + aboveGroundHeight;
					nullPoint = TRUE;
				}
				else
					underSurface = TRUE;
			}
			else
				underSurface = FALSE;

			if (underSurface)
			{            /* It is inside so estimate cube using a sphere with a borehole (Elipsoid) through the centre */
				double volumeIntergral, n, m, dip, ddir, pitch;
				double ellipSusX, ellipSusY, ellipSusZ;
				double ellipSusXX, ellipSusYY, ellipSusZZ;
				Point3d *pointBefore, *pointAfter;

				volumeIntergral = -(4*PI/3);
													/* Gravity calculation for a sphere */
				SubPt3d(pointPtr, &cubeCentre, &distanceVector);
				distanceToPoint = sqrt(distanceVector.x*distanceVector.x + 
			                          distanceVector.y*distanceVector.y +
											  distanceVector.z*distanceVector.z);
				to = volumeIntergral*distanceToPoint;


						/* Magnetics calculation for a sphere*/
				magneticsX = volumeIntergral*blockSusX;
				magneticsY = volumeIntergral*blockSusY;
				magneticsZ = volumeIntergral*blockSusZ;
						/* Magnetics calculation for elipsoid borhole (a = infinity, b = c) */
						/* take the elipsoid magnitisation from the sphere magnetisation to simulate borehole */
				      /* First we need the slope of the borehole */
				if (point > 0)
				{
					pointBefore = pointPtr-1;
					pointAfter = pointPtr;
				}
				else 
				{
					pointBefore = pointPtr;
					if (point < numPointsToMeasure-1)
						pointAfter = pointPtr+1;
					else
						pointAfter = pointPtr;
				}

				if ((pointBefore->x == pointAfter->x) && (pointBefore->y == pointAfter->y))
				{
					dip = DEG_TO_RAD(90.0);
					ddir = DEG_TO_RAD(0.0);
				}
				else
				{
					if (distanceToPoint == 0.0)
               {
						if (pointBefore->z == pointAfter->z)
							dip = DEG_TO_RAD(0.0);
						else
							dip = DEG_TO_RAD(90.0);
					}
					else
						dip = acos((pointAfter->z - pointBefore->z)/distanceToPoint) - DEG_TO_RAD(90.0);
					if (pointBefore->x == pointAfter->x)
						ddir = 0.0;
					else
						ddir = atan((pointAfter->y - pointBefore->y)/(pointAfter->x - pointBefore->x)) + DEG_TO_RAD(90.0);;
				}
				pitch = DEG_TO_RAD(180.0);
					/* Now convert the Sus values we have to the borehole reference frame */
				n = blockSusY*cos(pitch) - blockSusX*sin(pitch);
				ellipSusZ = -(n*sin(dip) + blockSusZ*cos(dip));
				n = n*cos(dip) - blockSusZ*sin(dip);
				m = blockSusX*cos(pitch) + blockSusY*sin(pitch);
				ellipSusY = m*cos(ddir) - n*sin(ddir);
				ellipSusX = m*sin(ddir) + n*cos(ddir);
					/* Now we can do the calc for the borehole magnetisation */
				ellipSusX = 0;
				ellipSusY = -2*PI*ellipSusY;
				ellipSusZ = -2*PI*ellipSusZ;
					/* convert those values back to the model refererence frame */
				ddir = ddir + DEG_TO_RAD(180.0);
				dip *= -1.0;
				n = ellipSusY*cos(pitch) - ellipSusX*sin(pitch);
				ellipSusZZ = -(n*sin(dip) + ellipSusZ*cos(dip));
				n = n*cos(dip) - ellipSusZ*sin(dip);
				m = ellipSusX*cos(pitch) + ellipSusY*sin(pitch);
				ellipSusYY = m*cos(ddir) - n*sin(ddir);
				ellipSusXX = m*sin(ddir) + n*cos(ddir);
									/* Take the borehole away from the sphere's magnetisation */
				magneticsX = magneticsX - ellipSusXX;
				magneticsY = magneticsY - ellipSusYY;
				magneticsZ = magneticsZ - ellipSusZZ; /* direction along borehole */
				if (magXResultPtr)
				{
					if (addToResult)              /* Assign Y Results */
						*magXResultPtr += magneticsX;
					else
						*magXResultPtr =  magneticsX;
					magXResultPtr++;
				}
				
				if (magYResultPtr)
				{
					if (addToResult)              /* Assign Y Results */
						*magYResultPtr += magneticsY;
					else
						*magYResultPtr =  magneticsY;
					magYResultPtr++;
				}

				if (magZResultPtr)
				{
					if (addToResult)              /* Assign Y Results */
						*magZResultPtr += magneticsZ;
					else
						*magZResultPtr =  magneticsZ;
					magZResultPtr++;
				}
			}
			else   /* Above surface of strat */
			{
				p1u = p1v = p1w = to
					 = p2u = p2v = p2w = 0.000000;
      
				for (i = 0; i < NUM_X_STEPS; i++)
				{
					for (j = 0; j < NUM_Y_STEPS; j++)
					{
						for (k = 0; k < NUM_Z_STEPS; k++)
						{
							sign = pow((double)(-1), (double)(i+1)+(j+1)+(k+1));
               
																  /* Intragral for gravity mask */
							distance = (double) sqrt(xDist[i]*xDist[i]+yDist[j]*yDist[j]+zDist[k]*zDist[k]);
							phi1 = sign * atan(yDist[j]*xDist[i] / (zDist[k]*distance));
							phi3 = sign * log(yDist[j] + distance);
							phi4 = sign * log(xDist[i] + distance);
							to += -phi1*zDist[k] + phi3*xDist[i] + phi4*yDist[j];

																  /* Intragral for magnetics mask */
							p1u += sign * atan(yDist[j]*zDist[k] / (xDist[i]*distance));
							p1v += sign * atan(-zDist[k]*xDist[i] / (yDist[j]*distance));
							p1w += phi1;
							p2u += phi4;
							p2v += phi3;
							p2w += sign * log(zDist[k] + distance);
						}
					}
				}

				if (magneticsCalc)
				{
#ifdef PROJECTION
					magneticsX = -(cosDecCosInc*p2w - sinDecCosInc*p1u
															  + sinInclination*p2v);
					magneticsY = -(cosDecCosInc*p1v + sinDecCosInc*p2w 
															  + sinInclination*p2u);
					magneticsZ = -(cosDecCosInc*p2u + sinDecCosInc*p2v 
															  - sinInclination*p1w);
#else
					magneticsX = ((blockSusX*-p1u) + (blockSusY* p2w) + (blockSusZ* p2v));
					magneticsY =-((blockSusX* p2w) + (blockSusY* p1v) + (blockSusZ* p2u)); 
					magneticsZ = ((blockSusX* p2v) + (blockSusY* p2u) + (blockSusZ*-p1w));
#endif
				
					if (nullPoint || (fabs(*magResultPtr - nullValue) < ZERO))
					{
						if (nullPoint)
						{
							if (magResultPtr)
							{
								*magResultPtr = nullValue;
								magResultPtr++;
							}
							if (magXResultPtr)
							{
								*magXResultPtr = nullValue;
								magXResultPtr++;
							}
							if (magYResultPtr)
							{
								*magYResultPtr = nullValue;
								magYResultPtr++;
							}
							if (magZResultPtr)
							{
								*magZResultPtr = nullValue;
								magZResultPtr++;
							}
						}
						else
							nullPoint = TRUE;
					}
					else
					{
/* #define USE_SQUARE   1 */
						if (magResultPtr)
						{
#ifdef PROJECTION
							if (addToResult)              /* Assign Results */
								*magResultPtr += magneticsX*blockSusX + magneticsY*blockSusY + magneticsZ*blockSusZ;
							else
								*magResultPtr =  magneticsX*blockSusX + magneticsY*blockSusY + magneticsZ*blockSusZ;
#else
#ifdef USE_SQUARE
							if (addToResult)              /* Assign Results */
								*magResultPtr += sqrt(magneticsX*magneticsX + magneticsY*magneticsY + magneticsZ*magneticsZ)
															* ((magneticsZ>0)?1:-1);
							else
								*magResultPtr =  sqrt(magneticsX*magneticsX + magneticsY*magneticsY + magneticsZ*magneticsZ)
															* ((magneticsZ>0)?1:-1);
#else
							if (addToResult)              /* Assign Results */
								*magResultPtr += -(cosDecCosInc*p2w - sinDecCosInc*p1u + sinInclination*p2v)*blockSusX
													  -(cosDecCosInc*p1v + sinDecCosInc*p2w + sinInclination*p2u)*blockSusY
													  -(cosDecCosInc*p2u + sinDecCosInc*p2v - sinInclination*p1w)*blockSusZ;
							else
								*magResultPtr =  -(cosDecCosInc*p2w - sinDecCosInc*p1u + sinInclination*p2v)*blockSusX
													  -(cosDecCosInc*p1v + sinDecCosInc*p2w + sinInclination*p2u)*blockSusY
													  -(cosDecCosInc*p2u + sinDecCosInc*p2v - sinInclination*p1w)*blockSusZ;
#endif
#endif
							magResultPtr++;
						}
						if (magXResultPtr)
						{
#ifdef PROJECTION
							if (addToResult)              /* Assign X Results */
								*magXResultPtr += magneticsX*blockSusX;
							else
								*magXResultPtr =  magneticsX*blockSusX;
#else
							if (addToResult)              /* Assign X Results */
								*magXResultPtr += magneticsX;
							else
								*magXResultPtr =  magneticsX;
#endif
							magXResultPtr++;
						}
						if (magYResultPtr)
						{
#ifdef PROJECTION
							if (addToResult)              /* Assign Y Results */
								*magYResultPtr += magneticsY*blockSusY;
							else
								*magYResultPtr =  magneticsY*blockSusY;
#else
							if (addToResult)              /* Assign Y Results */
								*magYResultPtr += magneticsY;
							else
								*magYResultPtr =  magneticsY;
#endif
							magYResultPtr++;
						}
						if (magZResultPtr)
						{
#ifdef PROJECTION
							if (addToResult)              /* Assign Z Results */
								*magZResultPtr += magneticsZ*blockSusZ;
							else
								*magZResultPtr =  magneticsZ*blockSusZ;
#else
							if (addToResult)              /* Assign Z Results */
								*magZResultPtr += magneticsZ;
							else
								*magZResultPtr =  magneticsZ;
#endif
							magZResultPtr++;
						}
					}
				}
			}

			if (nullPoint)
			{
				if (grvResultPtr)
					*grvResultPtr = nullValue;
				nullPoint = FALSE;
			}
			else
			{
				if (gravityCalc)
				{
					gravity = to*GRAVITY_CONSTANT;
									  /* Assign Result */
					if (addToResult)
						*grvResultPtr += gravity*blockDensity;
					else
						*grvResultPtr = gravity*blockDensity;
					grvResultPtr++;
				}
			}
		}
		else
		{
			if (magResultPtr)
			{
				if (!addToResult)
					*magResultPtr = 0.0;
				magResultPtr++;
			}
			if (magXResultPtr)
			{
				if (!addToResult)
					*magXResultPtr = 0.0;
				magXResultPtr++;
			}
			if (magYResultPtr)
			{
				if (!addToResult)
					*magYResultPtr = 0.0;
				magYResultPtr++;
			}
			if (magZResultPtr)
			{
				if (!addToResult)
					*magZResultPtr = 0.0;
				magZResultPtr++;
			}

			if (grvResultPtr)
			{
				if (!addToResult)
					*grvResultPtr = 0.0;
				grvResultPtr++;
			}
		}
	}

   return (TRUE);
}  
  
/* *****************************************************************
FUNCTION getGravityVector

INPUT

OUTPUT

DESCRIPTION 
   Get the density value needed to pass into 'calcAnomAtPoints'

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int getGravityVector(blockDensity, x, y, z, indexData, propArray, densityData)
double *blockDensity;
int x, y, z;
short ***indexData;
LAYER_PROPERTIES **propArray;
float ***densityData;
{
	int index, result = TRUE;

   if (indexData && propArray)
	{
      if (index = indexData[z][x][y])
         *blockDensity = propArray[index-1]->density;
		else
			result = FALSE;
	}
	else
	{
      if (densityData)
			*blockDensity = (double) densityData[z][x][y];
		else
			result = FALSE;
	}

	if (result)   /* Correct two systems for calculation of density */
		if (*blockDensity < 50.0)
			*blockDensity *= 1000.0;

	return (result);
}

/* *****************************************************************
FUNCTION getMagneticVectors

INPUT

OUTPUT

DESCRIPTION 
   Get the magnetics field values needed to pass into 'calcAnomAtPoints'

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int getMagneticVectors(blockSusX, blockSusY, blockSusZ,
							bgFieldIntensity, bgFieldInclination, bgFieldDeclination,
						   x, y, z, indexData, propArray, magSusData,
							remSusDecData,   remSusAziData,   remSusStrData,
							aniSusDipData,   aniSusDDirData,  aniSusPitchData,
							aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data)
double *blockSusX, *blockSusY, *blockSusZ;
double bgFieldIntensity, bgFieldInclination, bgFieldDeclination;
int x, y, z;
short ***indexData;
LAYER_PROPERTIES **propArray;
float ***magSusData,
      ***remSusDecData, ***remSusAziData,  ***remSusStrData,
		***aniSusDipData,  ***aniSusDDirData, ***aniSusPitchData,
		***aniSusAxis1Data,***aniSusAxis2Data,***aniSusAxis3Data;
{
	int index, result = TRUE;
	int anisotropic = FALSE, remanent = FALSE;
	double remAngle, remIncl, remSusStr, remResult1, remResult2, remResult3;
	double susResult1, susResult2, susResult3;
	double sus1, sus2, sus3, susX, susY, susZ, aniDip, aniDDir, aniPitch;
	double cRf, sRf, sf, cf, sa, ca, cacf, sacf;

   sf = sin(DEG_TO_RAD(bgFieldInclination));
   cf = cos(DEG_TO_RAD(bgFieldInclination));

   sa = sin(DEG_TO_RAD(180.0-bgFieldDeclination));
   ca = cos(DEG_TO_RAD(180.0-bgFieldDeclination));
   cacf = ca*cf;
   sacf = sa*cf;

	if (indexData && propArray)
	{
      if (index = indexData[z][x][y])
		{
			index--;
			if (propArray[index]->anisotropicField)
			{
				anisotropic = TRUE;
				sus1 = (double) propArray[index]->sus_X;
				sus2 = (double) propArray[index]->sus_Y;
				sus3 = (double) propArray[index]->sus_Z;
            aniDip = (double) propArray[index]->sus_dip;
            aniDDir = (double) propArray[index]->sus_dipDirection;
            aniPitch = (double) propArray[index]->sus_pitch;
			}
			else
			{
				susX = susY = susZ =(double) propArray[index]->sus_X;
			}


			if (propArray[index]->remanentMagnetization)
			{
				remanent = TRUE;
			   remAngle = (double) propArray[index]->angleWithNorth;
            remIncl  = (double) propArray[index]->inclination;
            remSusStr= (double) propArray[index]->strength;
			}

		}
		else
			result = FALSE;
	}
	else
	{
		if (magSusData)
		{
			susX = susY = susZ = (double) magSusData[z][x][y];
		}
		else if (aniSusAxis1Data && aniSusAxis2Data && aniSusAxis3Data
			     && aniSusDipData && aniSusDDirData  && aniSusPitchData)
		{
			anisotropic = TRUE;
			sus1 = (double) aniSusAxis1Data[z][x][y];
			sus2 = (double) aniSusAxis2Data[z][x][y];
			sus3 = (double) aniSusAxis3Data[z][x][y];
         aniDip = (double) aniSusDipData[z][x][y];
         aniDDir = (double) aniSusDDirData[z][x][y];
         aniPitch = (double) aniSusPitchData[z][x][y];
		}
		else
			result = FALSE;

		if (remSusDecData && remSusAziData && remSusStrData)
		{
			remanent = TRUE;
         remAngle  = (double) remSusDecData[z][x][y];
         remIncl   = (double) remSusAziData[z][x][y];
         remSusStr = (double) remSusStrData[z][x][y];
		}
	}

	if (result)
	{
		if (anisotropic)
		{
			double n, m;
			double radAniPitch, radAniDip, radAniDDir;

			radAniPitch = DEG_TO_RAD(aniPitch);
			radAniDip = DEG_TO_RAD(aniDip);
			radAniDDir = DEG_TO_RAD(aniDDir);

         n = sus2*cos(radAniPitch) - sus1*sin(radAniPitch);
         susZ = -(n*sin(radAniDip) + sus3*cos(radAniDip));
         n = n*cos(radAniDip) - sus3*sin(radAniDip);
         m = sus1*cos(radAniPitch) + sus2*sin(radAniPitch);
         susY = m*cos(radAniDDir) - n*sin(radAniDDir);
         susX = m*sin(radAniDDir) + n*cos(radAniDDir);

               /* Because anisotropy does not have any dirrection associated
               ** with it the susX,Y,Z  sould always be +ve after this spining
               ** by the dip, dipdir and pitch (30/4/96 - Paul Manser) */
         susX = fabs(susX);
         susY = fabs(susY);
         susZ = fabs(susZ);
		}

		susResult1 = bgFieldIntensity * susX;
      susResult2 = bgFieldIntensity * susY;
      susResult3 = bgFieldIntensity * susZ;
		

		if (remanent)
		{
         cRf = cos(DEG_TO_RAD(remIncl));
         sRf = sin(DEG_TO_RAD(remIncl));

         remResult1 = remSusStr * cRf
                      * sin(DEG_TO_RAD(-remAngle));  /* old cos */ /* bgFieldDeclination- */
         remResult2 = remSusStr * cRf
                      * cos(DEG_TO_RAD(-remAngle));  /* old sin */ /* bgFieldDeclination- */
         remResult3 = remSusStr * sRf;
		}
		else
		{
         remResult1 = remResult2 = remResult3 = 0.0;
		}

	   *blockSusX = susResult1*sacf + remResult1;
      *blockSusY = susResult2*cacf + remResult2;
      *blockSusZ = susResult3*sf   + remResult3;
	}

	return (result);
}


/* *****************************************************************
FUNCTION calcGravityLayer

INPUT

OUTPUT

DESCRIPTION 
   calculate a gravity and/or magnetic  for a particular layer
   using the paramaters and the slave array

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int calcGravityLayer(grvResult, gravityFilter, filterSize, options, layer,
                     layerXMax, layerYMax, indexCalc, numProp, propArray,
                     indexData, densityData)
double **grvResult, **gravityFilter;
int filterSize;
GEOPHYSICS_OPTIONS *options;
int layer, layerXMax, layerYMax;
int indexCalc, numProp;
LAYER_PROPERTIES **propArray;
short ***indexData;
float ***densityData;
{
   register int x, y, i, j;
   register short index;
   double  density = 0.0;
                                             
   if (!grvResult)
      return (FALSE);
      
   for (y = 0; y < layerYMax; y++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (x = 0; x < layerXMax; x++)
      {
         if (indexData)
            index = getIndex(indexData[layer], x, y);

                       /* ********************************** */
                       /* Calculate Gravity from the density */
         if (indexData)
         {
            if (index < 0)
               density = 0.0;
            else
               density = propArray[index]->density;
                  
            if (density < 100.0)
               density = density*1000;
         }
         else
            density = (double) getDensity(densityData[layer], x, y);

         for (i = 0; i < filterSize; i++)
         {
            for (j = 0; j < filterSize; j++)
            {
               grvResult[x+i][y+j] += density*gravityFilter[i][j];
            }
         }
      }
   }
      
   return (TRUE);
}                 

/* *****************************************************************
FUNCTION calcMagneticsLayer

INPUT

OUTPUT

DESCRIPTION 
   calculate a gravity and/or magnetic  for a particular layer
   using the paramaters and the slave array

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int calcMagneticsLayer(magXResult, magYResult, magZResult, magResult,
                       magXFilter, magYFilter, magZFilter, magTotalFilter,
							  magP1Filter, magP2Filter, filterSize, options,
							  inclination, declination, intensity, layer, layerXMax, layerYMax,
                       susCalc, remCalc, aniCalc, indexCalc, numProp, propArray, indexData,
                       magSusData,      remSusDecData,   remSusAziData,   remSusStrData,
                       aniSusDipData,   aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
                       aniSusAxis2Data, aniSusAxis3Data)
double **magXResult, **magYResult, **magZResult, **magResult;
double **magXFilter;
double **magYFilter;
double **magZFilter;
double **magTotalFilter;
double ***magP1Filter;
double ***magP2Filter;
int filterSize;
GEOPHYSICS_OPTIONS *options;
double inclination, declination, intensity;
int layer, layerXMax, layerYMax;
int susCalc, remCalc, aniCalc, indexCalc, numProp;
LAYER_PROPERTIES **propArray;
short ***indexData;
float ***magSusData,     ***remSusDecData,
      ***remSusAziData,  ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData, ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data,***aniSusAxis3Data;
{
   register int x, y, i, j;
   float  magSus = (float) 0.0,      remSusDec = (float) 0.0,
          remSusAzi = (float) 0.0,   remSusStr = (float) 0.0,   aniSusDip = (float) 0.0,
          aniSusDDir = (float) 0.0,  aniSusPitch = (float) 0.0;
   double susResult1=0.0,    susResult2=0.0,    susResult3=0.0;
   double remantResult1=0.0, remantResult2=0.0, remantResult3=0.0;
   double remIncl, remAngle; /* declination, intensity; */
   double sus1, sus2, sus3, susX, susY, susZ;
   double sf, cf, sa, ca, cacf, sacf, cRf, sRf, m, n;
   double JR1, JR2, JR3, JR4 = 0.00000000000000;
   short index, magComponents;
   
   if (!magResult)
      return (FALSE);
           /* Calc the individual components as well as the total */
   if (magXResult && magYResult && magZResult)
      magComponents = TRUE;
   else
      magComponents = FALSE;
	
   sf = sin(DEG_TO_RAD(inclination));
   cf = cos(DEG_TO_RAD(inclination));

   sa = sin(DEG_TO_RAD(declination));
   ca = cos(DEG_TO_RAD(declination));
   cacf = ca*cf;
   sacf = sa*cf;

   for (y = 0; y < layerYMax; y++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (x = 0; x < layerXMax; x++)
      {
         JR1 = JR2 = JR3 = JR4 = 0.0;
         
         if (indexData)
            index = getIndex(indexData[layer], x, y);

                       /* ************************************************* */
                       /* Calculate Magnetics from all the axis information */
                                                      /* ************************** */
         if (susCalc)                                 /* Sus Only (Not Anisotropic) */
         {
            if (indexData)
               if (index < 0)
                  JR4 = 0.0;
               else
                  JR4 = propArray[index]->sus_X;
            else
               JR4 = getMagSus(magSusData[layer], x, y);
         }
                                                     /* ********************** */
         if (remCalc)                                /* Remanence Calculations */
         {
            if (indexData)
            {
               if ((index < 0) || (!propArray[index]->remanentMagnetization))
               {
                  remAngle = remIncl = remSusStr = (float) 0.0;
               }
               else
               {
                  remAngle = propArray[index]->angleWithNorth;
                  remIncl = propArray[index]->inclination;
                  remSusStr = (float) propArray[index]->strength;
               }
            }
            else
            {
               remAngle = getRemSusDec(remSusDecData[layer], x, y);
               remIncl = getRemSusAzi(remSusAziData[layer], x, y);
               remSusStr = getRemSusStr(remSusStrData[layer], x, y);
            }

            cRf = cos(DEG_TO_RAD(remIncl));
            sRf = sin(DEG_TO_RAD(remIncl));

            /* remantResult1 = horizontal component of remSusStr (from remIncl)
                               combined with the X axis (from declination-remAngle)*/
            /* remantResult2 = horizontal component of remSusStr (from remIncl)
                               combined with the Y axis (from declination-remAngle)*/
            /* remantResult3 = vertical component of remSusStr (from remIncl) */

            remantResult1 = remSusStr * cRf
                         * sin(DEG_TO_RAD(-remAngle));  /* old cos */ /* declination- */
            remantResult2 = remSusStr * cRf
                         * cos(DEG_TO_RAD(-remAngle));  /* old sin */ /* declination- */
            remantResult3 = remSusStr * sRf;
         }
            

                                                     /* *********************** */
         if (aniCalc)                                /* Anisotropy Calculations */
         {
            if (indexData)
            {
               if ((index < 0) || (!propArray[index]->anisotropicField))
               {
                  aniSusDip = aniSusDDir = aniSusPitch = (float) 0.0;
                  susResult1 = susResult2 = susResult3 = 0.0;
                            /* treat as through it was a normal sus calculation */
                  if (index >= 0)
                     JR4 = propArray[index]->sus_X;
               }
               else
               {
                  aniSusDip = (float) propArray[index]->sus_dip;
                  aniSusDDir = (float) propArray[index]->sus_dipDirection;
                  aniSusPitch = (float) propArray[index]->sus_pitch;

                  aniSusDip = (float) DEG_TO_RAD(aniSusDip); 
                  aniSusDDir = (float) DEG_TO_RAD(aniSusDDir);
                  aniSusPitch = (float) DEG_TO_RAD(aniSusPitch); 
                                          /* the initial axis */
                  sus1 = propArray[index]->sus_X;
                  sus2 = propArray[index]->sus_Y;
                  sus3 = propArray[index]->sus_Z;
                  
                  /* n = (final Y component of Y axis) - (final Y component of X axis)
                         final Y comonent of Y and X axis in aniSusPitch direction */
                  /* susX = (final X component (perp to Y component)) + (final X component of Z axis) */
                  /* n = x component in aniSusDip direction (including aniSusPitch orientation) */
                  /* m = x comp */
                                          /* work out components */
                  n = sus2*cos(aniSusPitch) - sus1*sin(aniSusPitch);
                  susZ = -(n*sin(aniSusDip) + sus3*cos(aniSusDip));
                  n = n*cos(aniSusDip) - sus3*sin(aniSusDip);
                  m = sus1*cos(aniSusPitch) + sus2*sin(aniSusPitch);
                  susY=m*cos(aniSusDDir)-n*sin(aniSusDDir);
                  susX=m*sin(aniSusDDir)+n*cos(aniSusDDir);
                  
                        /* Because anisotropy does not have any dirrection associated
                        ** with it the susX,Y,Z  sould always be zero after this spining
                        ** by the dip, dipdir and pitch (30/4/96 - Paul Manser) */
                  susX = fabs(susX);
                  susY = fabs(susY);
                  susZ = fabs(susZ);

/*
                  n = sus2*cos(aniSusPitch) - sus1*sin(aniSusPitch);
                  susX = -(n*sin(aniSusDip) + sus3*cos(aniSusDip));
                  n = n*cos(aniSusDip) - sus3*sin(aniSusDip);
                  m = sus1*cos(aniSusPitch) + sus2*sin(aniSusPitch);
                  susY=m*cos(aniSusDDir)-n*sin(aniSusDDir);
                  susZ=m*sin(aniSusDDir)+n*cos(aniSusDDir);
*/
                  susResult1 = intensity * susX;
                  susResult2 = intensity * susY;
                  susResult3 = intensity * susZ;
               }
            }
            else  /* indexData */
            {
               aniSusDip = getAniSusDip (aniSusDipData[layer], x, y);
               aniSusDDir = getAniSusDDir (aniSusDDirData[layer], x, y);
               aniSusPitch = getAniSusPitch (aniSusPitchData[layer], x, y);
                                          /* the initial axis */
               sus1 = getAniSusAxis1(aniSusAxis1Data[layer], x, y);
               sus2 = getAniSusAxis2(aniSusAxis2Data[layer], x, y);
               sus3 = getAniSusAxis3(aniSusAxis3Data[layer], x, y);
/*
               if ((fabs(aniSusDip) < TOLERANCE) &&
                   (fabs(aniSusDDir) < TOLERANCE) &&
                   (fabs(aniSusPitch) < TOLERANCE))
               {
                  susResult1 = susResult2 = susResult3 = 0.0;
                            .* treat as through it was a normal sus calculation *.
                  JR4 = sus1;
               }
               else
*/
               {
                  aniSusDip = (float) DEG_TO_RAD(aniSusDip); 
                  aniSusDDir = (float) DEG_TO_RAD(aniSusDDir);
                  aniSusPitch = (float) DEG_TO_RAD(aniSusPitch); 

                                          /* work out components */
                  n = sus2*cos(aniSusPitch) - sus1*sin(aniSusPitch);
                  sus3 = -(n*sin(aniSusDip) + sus3*cos(aniSusDip));
                  n = n*cos(aniSusDip) - sus3*sin(aniSusDip);
                  m = sus1*cos(aniSusPitch) + sus2*sin(aniSusPitch);
                  sus2=m*cos(aniSusDDir)-n*sin(aniSusDDir);
                  sus1=m*sin(aniSusDDir)+n*cos(aniSusDDir);

                  susResult1 = intensity * sus1;
                  susResult2 = intensity * sus2;
                  susResult3 = intensity * sus3;
               }
            } /* indexData */
         } /* Ani Calc */

                            /* Main calculations to move the convolution over
                            ** with the correct Values finally worked out */
         JR1 = susResult1*sacf + remantResult1;
         JR2 = susResult2*cacf + remantResult2;
         JR3 = susResult3*sf   + remantResult3;

         for (i = 0; i < filterSize; i++)
         {
            for (j = 0; j < filterSize; j++)
            {
					magResult[x+i][y+j] += -JR1*magXFilter[i][j]
						                    -JR2*magYFilter[i][j]
							                 -JR3*magZFilter[i][j]
								              -JR4*magTotalFilter[i][j];

               if (magComponents)
               {
						magXResult[x+i][y+j] +=  ((JR1*-magP1Filter[0][i][j])
						                        + (JR2* magP2Filter[2][i][j])
														+ (JR3* magP2Filter[1][i][j]));
						magYResult[x+i][y+j] += -((JR1* magP2Filter[2][i][j])
						                        + (JR2* magP1Filter[1][i][j])
														+ (JR3* magP2Filter[0][i][j]));
						magZResult[x+i][y+j] +=  ((JR1* magP2Filter[1][i][j])
						                        + (JR2* magP2Filter[0][i][j])
														+ (JR3*-magP1Filter[2][i][j]));
               }
            }
         }
      }
   }
      
   return (TRUE);
}                 


/* *****************************************************************
FUNCTION calcMagFromComponents

INPUT

OUTPUT

DESCRIPTION 
   calculate the magnetic image from the 3 components of the fields

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int calcMagFromComponents(magResult2D, magXResult2D, magYResult2D, magZResult2D,
								  magResult1D, magXResult1D, magYResult1D, magZResult1D,
								  nx, ny, filterSize, options, fieldInclination, fieldDeclination, fieldIntensity)
double **magXResult2D, **magYResult2D, **magZResult2D, **magResult2D;
double *magXResult1D, *magYResult1D, *magZResult1D, *magResult1D;
int nx, ny, filterSize;
GEOPHYSICS_OPTIONS *options;
double fieldInclination, fieldDeclination, fieldIntensity;
{
	int x, y, index;
	double fieldX, fieldY, fieldZ;
   double sinInclination, cosInclination, sinDeclination, cosDeclination;
	double cosDecCosInc, sinDecCosInc;
	double addX, addY, addZ, removeOffset;

	if (!magResult2D && !magResult1D)
		return (FALSE);
	if (!magXResult2D && !magXResult1D)
		return (FALSE);
	if (!magYResult2D && !magYResult1D)
		return (FALSE);
	if (!magZResult2D && !magZResult1D)
		return (FALSE);

   sinInclination = sin(DEG_TO_RAD(fieldInclination));
   cosInclination = cos(DEG_TO_RAD(fieldInclination));
   sinDeclination = sin(DEG_TO_RAD(180.0-fieldDeclination));
   cosDeclination = cos(DEG_TO_RAD(180.0-fieldDeclination));
	cosDecCosInc = cosDeclination*cosInclination;
	sinDecCosInc = sinDeclination*cosInclination;

               /* calculate components of geomagnetic field  */
   fieldX = fieldIntensity * cosInclination * sinDeclination;
   fieldY = fieldIntensity * cosInclination * cosDeclination;
   fieldZ = fieldIntensity * sinInclination;

	addX = fabs(fieldX);
	addY = fabs(fieldY);
	addZ = fabs(fieldZ);
	removeOffset = sqrt(addX*addX + addY*addY + addZ*addZ);

	if (magResult2D && magXResult2D && magYResult2D && magZResult2D)
	{
		for (x = 0; x < nx; x++)
		{
			for (y = 0; y < ny; y++)
			{
				magResult2D[x][y] = sqrt(pow(magXResult2D[x][y] + addX, 2.0)
					                    + pow(magYResult2D[x][y] + addY, 2.0)
					                    + pow(magZResult2D[x][y] + addZ, 2.0)) - removeOffset;
			}
		}
	}
	else if (magResult1D && magXResult1D && magYResult1D && magZResult1D)
	{
		for (index = 0; index < nx*ny; index++)
		{
			magResult1D[index] = sqrt(pow(magXResult1D[index] + addX, 2.0)
					                  + pow(magYResult1D[index] + addY, 2.0)
					                  + pow(magZResult1D[index] + addZ, 2.0)) - removeOffset;
		}
	}
	else
		return (FALSE);


	return (TRUE);
}


/* *****************************************************************
FUNCTION writeAnomFile

INPUT

OUTPUT

DESCRIPTION 
   write out a anomalies result image to a Noddy format anomalies 
   file

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
writeAnomFile (char *filename, double **result2D, double *result1D,
               int fileType, int nx, int ny, int nz, int filterSize,
               BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *options)
#else
writeAnomFile (filename, result2D, result1D, fileType, nx, ny, nz, filterSize, blockView, options)
char *filename;
double **result2D, *result1D;
int fileType;
int nx, ny, nz, filterSize;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
#endif
{
   FILE *fo;
   char identifier[10];
   double output;
   register int x, y;
   float airgap = (float) 0.0;
   double unitScale;
   int calcRangeInCubes;
   time_t longTime;
   struct tm *today;
	FILE_SPEC fileSpec;

   time(&longTime); /* current Time */
   today = localtime(&longTime);
   
   if (!filename || !(result2D || result1D))
      return (FALSE);

   calcRangeInCubes = (int) floor (options->calculationRange/blockView->geophysicsCubeSize);

   if (fileType == MAGNETICS)
   {
      addFileExtention (filename, MAGNETICS_FILE_EXT);
      strcpy (identifier, "333");
      unitScale = 1.00000;
   }
   else if (fileType == MAGNETICS_X)
   {
      addFileExtention (filename, ".mgx");
      strcpy (identifier, "333");
      unitScale = 1.00000;
   }
   else if (fileType == MAGNETICS_Y)
   {
      addFileExtention (filename, ".mgy");
      strcpy (identifier, "333");
      unitScale = 1.00000;
   }
   else if (fileType == MAGNETICS_Z)
   {
      addFileExtention (filename, ".mgz");
      strcpy (identifier, "333");
      unitScale = 1.00000;
   }
   else
   {
      addFileExtention (filename, GRAVITY_FILE_EXT);
      strcpy (identifier, "444");
      unitScale = 100000.00000;  /* fudge to get into Model Vision units */
   }
      
   if (!(fo = (FILE *) fopen (filename, "w")))
      return (FALSE);
      
   if (options->calculationAltitude == AIRBORNE)
      airgap = (float) options->altitude;
      
   fprintf(fo,"%s\n", identifier);

	getCurrentFileName(&fileSpec);
	fprintf(fo,"%s%s\n", ANOM_HISTORY_TAG, fileSpec.name);
	fprintf(fo,"%s%s", ANOM_DATETIME_TAG, asctime(today)); /* asctime has a CR in it */

	if (result1D)
		fprintf(fo,"%d\t%d\t%d\t%d\n", calcRangeInCubes,
			                            nx+2*calcRangeInCubes+1, ny+2*calcRangeInCubes+1, nz);
	else
		fprintf(fo,"%d\t%d\t%d\t%d\n", calcRangeInCubes,
			                            nx+1, ny+1, nz);  /* was nx+1, ny+1, nz+1 */
   fprintf(fo,"%f\t%f\t%f\n", (float) options->inclination,
                              (float) options->declination,
                              (float) options->intensity);  
   fprintf(fo,"%f\t%f\t%f\n", (float) blockView->originX + projectOptions.easting,
                              (float) blockView->originY + projectOptions.northing,
                              (float) blockView->originZ - blockView->lengthZ);
   fprintf(fo,"%f\t%f\t%f\n", (float) blockView->originX + blockView->lengthX + projectOptions.easting,
                              (float) blockView->originY + blockView->lengthY + projectOptions.northing,
                              (float) blockView->originZ);
   fprintf(fo,"%f\t%f\n", (float) blockView->geophysicsCubeSize, airgap);  

   if (result2D)
	{
		for (y = filterSize-1; y < ny/* -1 !! */; y++)  /* was filterSize-1 */
		{
			for (x = filterSize-1; x < nx/* -1 !! */; x++)  /* was filterSize-1 */
			{
				output = result2D[x][y]*unitScale;
				fprintf(fo, "%lf\t", output);
			}
			fprintf(fo,"\n");
		}
	}
	else if (result1D)
	{
		for (y = ny-1; y >= 0; y--)
		{
			for (x = 0; x < nx; x++)
			{
				output = result1D[x*ny + y]*unitScale;
				fprintf(fo, "%lf\t", output);
			}
			fprintf(fo,"\n");
		}
	}

   fclose (fo);

   return (TRUE);
}

/* *****************************************************************
FUNCTION fillAnomImage

INPUT

OUTPUT

DESCRIPTION 
   fill out structure that containst the result image

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
fillAnomImage (double **result2D, double *result1D, int fileType, int nx, int ny, int filterSize, DOUBLE_2D_IMAGE *anomImage)
#else     
fillAnomImage (result2D, result1D, fileType, nx, ny, filterSize, anomImage)
double **result2D, *result1D;
int fileType;
int nx, ny, filterSize;
DOUBLE_2D_IMAGE *anomImage;
#endif
{
   register int x, y, xIndex, yIndex;
   int xDim, yDim;
   double unitScale;
   
   if (!result2D && !result1D)
      return (FALSE);

   if (fileType == MAGNETICS)
   {
      unitScale = 1.00000;
   }
   else if (fileType == GRAVITY)
   {
      unitScale = 100000.00000;  /* fudge to get into Model Vision units */
   }
   else
      unitScale = 1.0;

   xDim = nx - (filterSize - 1); /* !! - filterSize; */
   yDim = ny - (filterSize - 1); /* !! - filterSize; */

   skipMemManager = TRUE;   /* Make sure this is not free on the way out of the geophysics calcs */
   if (!(anomImage->data = (double **) create2DArray(yDim, xDim, sizeof(double))))
      return (FALSE);
   skipMemManager = FALSE;
      
   anomImage->dim1 = yDim;
   anomImage->dim2 = xDim;

   if (result2D)
	{
		yIndex = 0;
		for (y = filterSize-1; y < ny/*-1*/; y++)  /* was filterSize-1 */
		{
			xIndex = 0;
			for (x = filterSize-1; x < nx/*-1*/; x++)  /* was filterSize-1 */
			{
				anomImage->data[yIndex][xIndex] = (double) (result2D[x][y]*unitScale); 
				xIndex++;
			}
			yIndex++;
		}
	}
	else if (result1D)
	{
		yIndex = 0;
		for (y = 0; y < ny; y++)
		{
			xIndex = 0;
			for (x = 0; x < nx; x++)
			{
				anomImage->data[yIndex][xIndex] = (double) (result1D[x*ny + y]*unitScale); 
				xIndex++;
			}
			yIndex++;
		}
	}

   return (TRUE);
}

int
#if XVT_CC_PROTO
writeStandardAnomFile (char *filename, double **result,
               int fileType, int nx, int ny, int nz,
               BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *options)
#else
writeStandardAnomFile (filename, result, fileType, nx, ny, nz, blockView, options)
char *filename;
double **result;
int fileType;
int nx, ny, nz;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
#endif
{
   FILE *fo;
   char identifier[10];
   double output;
   register int x, y;
   float airgap = (float) 0.0;
   double unitScale = 1.000000000000000;
   int calcRangeInCubes;
   time_t longTime;
   struct tm *today;
	FILE_SPEC fileSpec;
   
   if (!filename || !result)
      return (FALSE);

   time(&longTime); /* current Time */
   today = localtime(&longTime);

   calcRangeInCubes = (int) floor (options->calculationRange/blockView->geophysicsCubeSize);

   if (fileType == MAGNETICS)
   {
      addFileExtention (filename, MAGNETICS_FILE_EXT);
      strcpy (identifier, "333");
   }
   else if (fileType == MAGNETICS_X)
   {
      addFileExtention (filename, ".mgx");
      strcpy (identifier, "333");
   }
   else if (fileType == MAGNETICS_Y)
   {
      addFileExtention (filename, ".mgy");
      strcpy (identifier, "333");
   }
   else if (fileType == MAGNETICS_Z)
   {
      addFileExtention (filename, ".mgz");
      strcpy (identifier, "333");
   }
   else
   {
      addFileExtention (filename, GRAVITY_FILE_EXT);
      strcpy (identifier, "444");
   }
      
   if (!(fo = (FILE *) fopen (filename, "w")))
      return (FALSE);
      
   if (options->calculationAltitude == AIRBORNE)
      airgap = (float) options->altitude;
      
   fprintf(fo,"%s\n", identifier);
	
	getCurrentFileName(&fileSpec);
	fprintf(fo,"%s%s\n", ANOM_HISTORY_TAG, fileSpec.name);
	fprintf(fo,"%s%s", ANOM_DATETIME_TAG, asctime(today)); /* asctime has a CR in it */

   fprintf(fo,"%d\t%d\t%d\t%d\n",calcRangeInCubes,
                                 nx+2*calcRangeInCubes+1, ny+2*calcRangeInCubes+1, nz);  /* was nx+1, ny+1, nz+1 */
   fprintf(fo,"%f\t%f\t%f\n", (float) options->inclination,
                              (float) options->declination,
                              (float) options->intensity);  
   fprintf(fo,"%f\t%f\t%f\n", (float) blockView->originX,
                              (float) blockView->originY,
                              (float) blockView->originZ - blockView->lengthZ);
   fprintf(fo,"%f\t%f\t%f\n", (float) blockView->originX + blockView->lengthX,
                              (float) blockView->originY + blockView->lengthY,
                              (float) blockView->originZ);
   fprintf(fo,"%f\t%f\n", (float) blockView->geophysicsCubeSize, airgap);  

   for (y = 0; y < ny; y++)
   {
      for (x = 0; x < nx; x++)
      {
         output = result[y][x]*unitScale;
         fprintf(fo, "%lf\t", output);
      }
      fprintf(fo,"\n");
   }

   fclose (fo);

   return (TRUE);
}


/* *****************************************************************
FUNCTION writeAnomProfileFile

INPUT

OUTPUT

DESCRIPTION 
   write out a anomalies result image to a Noddy format anomalies 
   file

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
writeAnomProfileFile (char *filename, double *result,
               int fileType, int nx, BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *options)
#else
writeAnomProfileFile (filename, result, fileType, nx, blockView, options)
char *filename;
double *result;
int fileType;
int nx;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
#endif
{
   FILE *fo;
   char identifier[10];
   double output;
   register int x;
   float airgap = (float) 0.0;
   double unitScale;
   int calcRangeInCubes;
   
   if (!filename || !result)
      return (FALSE);

   calcRangeInCubes = (int) floor (options->calculationRange/blockView->geophysicsCubeSize);
   
   if (!filename || !result)
      return (FALSE);

   if (fileType == MAGNETICS)
   {
      addFileExtention (filename, MAGNETICS_FILE_EXT);
      strcpy (identifier, "555");
      unitScale = 1.00000;
   }
   else   /* gravity */
   {
      addFileExtention (filename, GRAVITY_FILE_EXT);
      strcpy (identifier, "666");
      unitScale = 1.00000;
   }
      
   if (!(fo = (FILE *) fopen (filename, "w")))
      return (FALSE);
      
   if (options->calculationAltitude == AIRBORNE)
      airgap = (float) options->altitude;
      
   fprintf(fo,"%s\n", identifier);
   fprintf(fo,"%d\t%d\t%d\t%d\n",calcRangeInCubes,
                                 1, nx+1, 1);
   fprintf(fo,"%f\t%f\t%f\n", (float) options->inclination,
                              (float) options->declination,
                              (float) options->intensity);  
   fprintf(fo,"%f\t%f\t%f\n", (float) blockView->originX,
                              (float) blockView->originY,
                              (float) blockView->originZ - blockView->lengthZ);
   fprintf(fo,"%f\t%f\t%f\n", (float) blockView->originX + blockView->lengthX,
                              (float) blockView->originY + blockView->lengthY,
                              (float) blockView->originZ);
   fprintf(fo,"%f\t%f\n", (float) blockView->geophysicsCubeSize, airgap);  

   for (x = 0; x < nx; x++)
   {
      output = result[x]*unitScale;
      fprintf(fo, "%lf\n", output);
   }
   fprintf(fo,"\n");

   fclose (fo);

   return (TRUE);
}

/* *****************************************************************
FUNCTION writeXYZFile

INPUT

OUTPUT

DESCRIPTION 
   write out a anomalies xyz file 
   file

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
writeXYZFile (char *outputFilename, FILE_SPEC *xyzImportFile, Point3d *pointsToMeasure,
				  int numPointsToMeasure, double *gravityResults, double *magneticResults,
				  double *magXResults, double *magYResults, double *magZResults)
#else
writeXYZFile (outputFilename, xyzImportFile, pointsToMeasure, numPointsToMeasure,
				  gravityResults, magneticResults, magXResults, magYResults, magZResults)
char *outputFilename;
FILE_SPEC *xyzImportFile;
Point3d *pointsToMeasure;
int numPointsToMeasure;
double *gravityResults, *magneticResults, *magXResults, *magYResults, *magZResults;
#endif
{
	FILE *pointFile, *xyzFile;
	char line[200], line2[200], word[40];
	int xyzLineFound = FALSE;
	int nextLine, point;

	if (xyzImportFile)
	{
		xvt_fsys_set_dir(&(xyzImportFile->dir));
		if (!(pointFile = fopen(xyzImportFile->name, "r")))
		{
         if (batchExecution)
            fprintf(stderr,"Can't open XYZ file: %s\n",xyzImportFile->name);
         else ;
			   //xvt_dm_post_error("Can't open XYZ file: %s\n",xyzImportFile->name);
		}
	}
	else
		pointFile = NULL;

	addFileExtention(outputFilename, ".geo");
	if (!(xyzFile = fopen(outputFilename, "w")))
	{
      if (batchExecution)
         fprintf(stderr,"Cannot write XYZ File: %s\n",outputFilename);
      else
		   //xvt_dm_post_error("Cannot write XYZ File: %s\n",outputFilename);
		return (FALSE);
	}

	if (pointFile)
	{  /* At end of this 'line2' has column headings and 'line' the first data values */
		int firstTime = TRUE;

		while (!xyzLineFound && fgets(line, 200, pointFile))
		{     /* Use the first line with a number at the start as a sample */
			strcpy (line2,"X\tY\tZ\n");  /* Init to something if there is no headings */
			sscanf(line, "%s", word);
			if (isdigit(word[0]))
			{
				xyzLineFound = TRUE;
				line[strlen(line)-1] = '\0';
			}
			else
			{
				if (!firstTime)
					fprintf(xyzFile,"%s", line2);
				firstTime = FALSE;
				strcpy (line2, line);   /* Keep line Before */
			}
		}
		line2[strlen(line2)-1] = '\0';  /* Removes \n  that  fgets leaves */
	}
	else
		strcpy (line2,"X\tY\tZ");

	            /* Assemble Column Headers */
	if (gravityResults)
		strcat(line2, "\tGRAVITY");
	if (magneticResults)
		strcat(line2, "\tMAGNETICS");
	if (magXResults)
		strcat(line2, "\tMAGX");
	if (magYResults)
		strcat(line2, "\tMAGY");
	if (magZResults)
		strcat(line2, "\tMAGZ");

	/* Add a Line field that is basically a counter */
	strcat(line2, "\tLINE");

	fprintf(xyzFile,"%s\n", line2);

	point = 0;
	nextLine = TRUE;
	do
	{
		if (!pointFile)
			fprintf(xyzFile, "%lf\t%lf\t%lf", pointsToMeasure[point].x,
			        pointsToMeasure[point].y, pointsToMeasure[point].z);
		else
			fprintf(xyzFile, "%s", line);

		if (point < numPointsToMeasure)
		{
			if (gravityResults)     /* fudge to get into Model Vision units */
				fprintf(xyzFile, "\t%e", gravityResults[point]*100000.0);
			if (magneticResults)
				fprintf(xyzFile, "\t%e", magneticResults[point]);
			if (magXResults)
				fprintf(xyzFile, "\t%e", magXResults[point]);
			if (magYResults)
				fprintf(xyzFile, "\t%e", magYResults[point]);
			if (magZResults)
				fprintf(xyzFile, "\t%e", magZResults[point]);
			/* Add a Line field that is basically a counter */
			fprintf(xyzFile, "\t%d", point+1);

			point++;
		}
		fprintf(xyzFile, "\n");

		if (pointFile)
			nextLine = grab_line(pointFile, line, 200);
      
		if (point >= numPointsToMeasure)
			nextLine = FALSE;
	} while (nextLine);

	if (pointFile)
		fclose(pointFile);
	fclose(xyzFile);

	return (TRUE);
}

/* *****************************************************************
FUNCTION getDensity, getMagSus, get....... etc

INPUT

OUTPUT

DESCRIPTION 
              
RETURN
   value of the data or 0.0 if the real data was not available
***************************************************************** */
static short   
#if XVT_CC_PROTO  
getIndex (short **indexData, int x, int y)
#else        
getIndex (indexData, x, y)
short **indexData;
int x, y;
#endif           
{
   if (indexData)
      return (indexData[x][y]-1);
   else   /* read from file */
   {
      return ((short) -1);
   }
}

static float
#if XVT_CC_PROTO
getDensity (float **densityData, int x, int y)
#else
getDensity (densityData, x, y)
float **densityData;
int x, y;
#endif
{
   if (densityData)
      return (densityData[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getMagSus (float **magSusData, int x, int y)
#else
getMagSus (magSusData, x, y)
float **magSusData;
int x, y;
#endif
{
   if (magSusData)
      return (magSusData[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getRemSusDec (float **remSusDecData, int x, int y)
#else
getRemSusDec (remSusDecData, x, y)
float **remSusDecData;
int x, y;
#endif
{
   if (remSusDecData)
      return (remSusDecData[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getRemSusAzi (float **remSusAziData, int x, int y)
#else
getRemSusAzi (remSusAziData, x, y)
float **remSusAziData;
int x, y;
#endif
{
   if (remSusAziData)
      return (remSusAziData[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getRemSusStr (float **remSusStrData, int x, int y)
#else
getRemSusStr (remSusStrData, x, y)
float **remSusStrData;
int x, y;
#endif
{
   if (remSusStrData)
      return (remSusStrData[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getAniSusDip (float **aniSusDipData, int x, int y)
#else
getAniSusDip (aniSusDipData, x, y)
float **aniSusDipData;
int x, y;
#endif
{
   if (aniSusDipData)
      return (aniSusDipData[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getAniSusDDir (float **aniSusDDirData, int x, int y)
#else
getAniSusDDir (aniSusDDirData, x, y)
float **aniSusDDirData;
int x, y;
#endif
{
   if (aniSusDDirData)
      return (aniSusDDirData[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getAniSusPitch (float **aniSusPitchData, int x, int y)
#else
getAniSusPitch (aniSusPitchData, x, y)
float **aniSusPitchData;
int x, y;
#endif
{
   if (aniSusPitchData)
      return (aniSusPitchData[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getAniSusAxis1 (float **aniSusAxis1Data, int x, int y)
#else
getAniSusAxis1 (aniSusAxis1Data, x, y)
float **aniSusAxis1Data;
int x, y;
#endif
{
   if (aniSusAxis1Data)
      return (aniSusAxis1Data[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getAniSusAxis2 (float **aniSusAxis2Data, int x, int y)
#else
getAniSusAxis2 (aniSusAxis2Data, x, y)
float **aniSusAxis2Data;
int x, y;
#endif
{
   if (aniSusAxis2Data)
      return (aniSusAxis2Data[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

static float
#if XVT_CC_PROTO
getAniSusAxis3 (float **aniSusAxis3Data, int x, int y)
#else
getAniSusAxis3 (aniSusAxis3Data, x, y)
float **aniSusAxis3Data;
int x, y;
#endif
{
   if (aniSusAxis3Data)
      return (aniSusAxis3Data[x][y]);
   else   /* read from file */
   {
      return ((float) 0.0);
   }
}

/* *****************************************************************
FUNCTION calcAnomalies

INPUT

OUTPUT

DESCRIPTION 
   calculates gravity and/or magnetic data from the blocks passed
   int if they are in memory or from appropriate disk files if 
   there was not enough memory.

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
oldCalcAnomalies (char *outputFilename, char *blockName, int numLayers,
        int **layerDimensions, int numCubeSizes, int *cubeSizes,
        BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *options,
        int densityCalc, int susCalc, int remCalc, int aniCalc,
        int indexCalc, int numProps, LAYER_PROPERTIES **propArray,
        short ***indexData,       float ***densityData,
        float ***magSusData,      float ***remSusDecData,
        float ***remSusAziData,   float ***remSusStrData,
        float ***aniSusDipData,   float ***aniSusDDirData,
        float ***aniSusPitchData, float ***aniSusAxis1Data,
        float ***aniSusAxis2Data, float ***aniSusAxis3Data)
#else
oldCalcAnomalies (outputFilename, blockName, numLayers,
        layerDimensions, numCubeSizes, cubeSizes, blockView,
        options, densityCalc, susCalc, remCalc, aniCalc,
        indexCalc, numProps, propArray,   indexData,
        densityData,     magSusData,      remSusDecData,
        remSusAziData,   remSusStrData,   aniSusDipData,
        aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
        aniSusAxis2Data, aniSusAxis3Data)
char *outputFilename, *blockName;
int numLayers, **layerDimensions, numCubeSizes, *cubeSizes;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
int densityCalc, susCalc, remCalc, aniCalc, indexCalc, numProps;
LAYER_PROPERTIES **propArray;
short ***indexData;
float ***densityData,    ***magSusData,      ***remSusDecData,
      ***remSusAziData,  ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData, ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data,***aniSusAxis3Data;
#endif
{
   double ***anomFilter, **grvResult, **magResult;
   int filterSize, z, nx = 0, ny = 0, cubeSize, calcRangeInCubes;
   double zPos;

   cubeSize = cubeSizes[0];
   for (z = 0; z < numLayers; z++)
   {
      if (layerDimensions[z][0] > nx)
         nx = layerDimensions[z][0];
      if (layerDimensions[z][1] > ny)
         ny = layerDimensions[z][1];
      if (cubeSizes[z] < cubeSize) /* Finest cube Size */
         cubeSize = cubeSizes[z];
   }
   calcRangeInCubes = (int) floor (options->calculationRange/cubeSize);
   
                   /* *************************************** */                   
                   /* Alllocate the working calculation array */
   filterSize = calcRangeInCubes*2 + 1;
   if (!(anomFilter = (double ***) create3DArray (filterSize, filterSize, 5,
                                             sizeof(double))))
      return (FALSE);
   if (!(grvResult = (double **) create2DArray(nx + filterSize,
                               ny + filterSize, sizeof(double))))
      return (FALSE);
   if (!(magResult = (double **) create2DArray(nx + filterSize,
                               ny + filterSize, sizeof(double))))
      return (FALSE);
      

                         /* ***************************** */                   
                         /* Calculate the Anomalies image */
   if (options->calculationAltitude == AIRBORNE)
      zPos = options->altitude;
   else
      zPos = 0.0;
   
                      /* Calculate from bottom layer up */
   for (z = numLayers-1; z >= 0; z--)
   {
      zPos = zPos + cubeSizes[z]/2.0;  /* add half a cube */

      calcAnomSlave(anomFilter, filterSize, filterSize, options, cubeSize, zPos);
      oldCalcAnomLayer(grvResult, magResult, anomFilter, filterSize, options, z,
                  layerDimensions[z][0], layerDimensions[z][1], nx, ny,
                  densityCalc, susCalc, remCalc, aniCalc,
                  indexCalc, numProps, propArray,   indexData,
                  densityData,     magSusData,      remSusDecData,
                  remSusAziData,   remSusStrData,   aniSusDipData,
                  aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
                  aniSusAxis2Data, aniSusAxis3Data);

      zPos = zPos + cubeSizes[z]/2.0;  /* add the other half */
      
      if (abortLongJob ())
         return (FALSE);
   }

                       /* ********************* */
                       /* Write out the Results */
   if (grvResult)
      writeAnomFile (outputFilename, grvResult, NULL, GRAVITY,
              nx, ny, numLayers, filterSize, blockView, options);

   if (magResult)
      writeAnomFile (outputFilename, magResult, NULL, MAGNETICS,
              nx, ny, numLayers, filterSize, blockView, options);

                       /* ******************************** */
                       /* Free all the memory that we used */
   if (grvResult)
      destroy2DArray ((char **) grvResult, nx + filterSize,
                                           ny + filterSize);
   if (magResult)
      destroy2DArray ((char **) magResult, nx + filterSize,
                                           ny + filterSize);
   if (anomFilter)
      destroy3DArray ((char ***) anomFilter, filterSize, filterSize, 5);
   
   return (TRUE);
}

/* *****************************************************************
FUNCTION calcAnomSlave

INPUT

OUTPUT

DESCRIPTION 
   calculate the slave array used in the calculation of an anomalies
   image for a given layer

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int calcAnomSlave(slave, xmax, ymax, options, cubeSize, zPos)
double ***slave;
int xmax, ymax;
GEOPHYSICS_OPTIONS *options;
int cubeSize;
double zPos;
{
   float sf, cf, sa, ca;
   float px, py, pz, JJ[4];
   float w[3];
   float g, minXMinusHalf, minYMinusHalf, minXPlusHalf, minYPlusHalf;
   double sign;
   double cacf,sacf;
   register int i,j,k;
   register int q, p;
   float u[3], v[3], rad;
   float xLength, yLength, xLengthSoFar, yLengthSoFar, halfCubeSize;
   double to, phi1, phi3, phi4;
   double p1u, p1v, p1w, p2u, p2v, p2w;

   g = (float) 6.672e-11;
   
   xLength = (float) (xmax*cubeSize);
   yLength = (float) (ymax*cubeSize);
   halfCubeSize = (float) (cubeSize/2.0);
   
   px = (float) (xLength/2.0);       /* define boundaries of cube */
   py = (float) (yLength/2.0);
   pz = (float) (zPos + 1.0);
   minXMinusHalf = px-halfCubeSize;
   minYMinusHalf = py-halfCubeSize;
   minXPlusHalf = px+halfCubeSize;
   minYPlusHalf = py+halfCubeSize;

   sf = (float) sin(DEG_TO_RAD(options->inclination));
   cf = (float) cos(DEG_TO_RAD(options->inclination));
   sa = (float) sin(DEG_TO_RAD(options->declination));
   ca = (float) cos(DEG_TO_RAD(options->declination));
   cacf = ca*cf;
   sacf = sa*cf;
               /* calculate comps of geomagnetic field  */
   JJ[1] = (float) options->intensity * cf * sa;
   JJ[2] = (float) options->intensity * cf * ca;
   JJ[3] = (float) options->intensity * sf;

   w[1] = pz + halfCubeSize;
   w[2] = pz - halfCubeSize;
              /* solve volume integral for every cubeSize point */
   for (p = 0; p < xmax; p++)
   {
      incrementLongJob (INCREMENT_JOB);

      xLengthSoFar = cubeSize*p + halfCubeSize;
      u[1] = xLengthSoFar - minXPlusHalf;
      u[2] = xLengthSoFar - minXMinusHalf;

      for (q = 0; q < ymax; q++)
      {
         yLengthSoFar = cubeSize*q + halfCubeSize;
         v[1] = yLengthSoFar - minYPlusHalf;
         v[2] = yLengthSoFar - minYMinusHalf;
         
         p1u = p1v = p1w = 0.0;
         p2u = p2v = p2w = 0.0;
         to = 0.0;
         
         for (i = 1; i <= 2; i++)
         {
            for (j = 1; j <= 2; j++)
            {
               for (k = 1; k <= 2; k++)
               {
                  sign = pow((double)(-1),(double)(i+j+k));
                  rad = (float) sqrt(u[i]*u[i]+v[j]*v[j]+w[k]*w[k]);
                  phi1 = sign * atan(v[j]*u[i]/(w[k]*rad));
                  phi3 = sign * log(v[j]+rad);
                  phi4 = sign * log(u[i]+rad);
                  to += -phi1*w[k] + phi3*u[i] + phi4*v[j];

                  p1u += sign * atan(v[j]*w[k]/(u[i]*rad));
                  p1v += sign * atan(-w[k]*u[i]/(v[j]*rad));
                  p1w += phi1;
                  p2u += phi4;
                  p2v += phi3;
                  p2w += sign * log(w[k] + rad);
               }
            }
         }

         slave[p][q][0] = (cacf*p2w - sacf*p1u + sf*p2v);
         slave[p][q][1] = (cacf*p1v + sacf*p2w + sf*p2u);
         slave[p][q][2] = (cacf*p2u + sacf*p2v - sf*p1w);

         slave[p][q][3] = to*g;
         slave[p][q][4] = JJ[1]*slave[p][q][0] + JJ[2]*slave[p][q][1]
                                               + JJ[3]*slave[p][q][2];
      }
   }
   return (TRUE);
}  

/* *****************************************************************
FUNCTION oldCalcAnomLayer

INPUT

OUTPUT

DESCRIPTION 
   calculate a gravity and/or magnetic  for a particular layer
   using the paramaters and the slave array

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int oldCalcAnomLayer(grvResult, magResult, slave, slaveSize, options, layer,
                  layerXMax, layerYMax, firstLayerXMax, firstLayerYMax,
                  densityCalc, susCalc, remCalc, aniCalc,
                  indexCalc, numProp, propArray,    indexData,
                  densityData,     magSusData,      remSusDecData,
                  remSusAziData,   remSusStrData,   aniSusDipData,
                  aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
                  aniSusAxis2Data, aniSusAxis3Data)
double **grvResult, **magResult, ***slave;
int slaveSize;
GEOPHYSICS_OPTIONS *options;
int layer, layerXMax, layerYMax, firstLayerXMax, firstLayerYMax;
int densityCalc, susCalc, remCalc, aniCalc, indexCalc, numProp;
LAYER_PROPERTIES **propArray;
short ***indexData;
float ***densityData,    ***magSusData,      ***remSusDecData,
      ***remSusAziData,  ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData, ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data,***aniSusAxis3Data;
{
   register int x, y, i, j, xPos, yPos;
   float  density = (float) 0.0,     magSus = (float) 0.0,      remSusDec = (float) 0.0,
          remSusAzi = (float) 0.0,   remSusStr = (float) 0.0,   aniSusDip = (float) 0.0,
          aniSusDDir = (float) 0.0,  aniSusPitch = (float) 0.0;
   double susResult1=0.0,    susResult2=0.0,    susResult3=0.0;
   double remantResult1=0.0, remantResult2=0.0, remantResult3=0.0;
   double remIncl, remAngle, declination, intensity;
   double sus1, sus2, sus3, susX, susY, susZ;
   double sf, cf, sa, ca, cacf, sacf, cRf, sRf, m, n;
   double JR1, JR2, JR3, JR4 = 0.00000000000000;
   short index;
   double layerWithOutSlaveX, layerWithOutSlaveY;
   double fLayerWithOutSlaveX, fLayerWithOutSlaveY;
   double ratioLayerToFirstX, ratioLayerToFirstY;
                                             
                    /* num of pixels in a block for variable cube size */
   layerWithOutSlaveX = layerXMax-slaveSize;
   layerWithOutSlaveY = layerYMax-slaveSize;
   fLayerWithOutSlaveX = firstLayerXMax-slaveSize;
   fLayerWithOutSlaveY = firstLayerYMax-slaveSize;
   ratioLayerToFirstX = fLayerWithOutSlaveX/layerWithOutSlaveX;
   ratioLayerToFirstY = fLayerWithOutSlaveY/layerWithOutSlaveY;
   
   declination = options->declination;
   intensity = options->intensity;

   sf = sin(DEG_TO_RAD(options->inclination));
   cf = cos(DEG_TO_RAD(options->inclination));

   sa = sin(DEG_TO_RAD(declination));
   ca = cos(DEG_TO_RAD(declination));
   cacf = ca*cf;
   sacf = sa*cf;

   for (y = 0; y < firstLayerYMax; y++)
   {
      yPos = (int) floor (((double) y)/ratioLayerToFirstY);
      incrementLongJob (INCREMENT_JOB);
      for (x = 0; x < firstLayerXMax; x++)
      {
         JR1 = JR2 = JR3 = JR4 = 0.0;
         
         xPos =(int) floor (((double) x)/ratioLayerToFirstX);

         if (indexData)
            index = getIndex(indexData[layer], xPos, yPos);

                       /* ********************************** */
                       /* Calculate Gravity from the density */
         if (grvResult)
         {
            if (indexData)
            {
               if (index < 0)
                  density = (float) 0.0;
               else
                  density = (float) propArray[index]->density;
                  
               if (density < 100.0)
                  density = density*1000;
            }
            else
               density = getDensity(densityData[layer], xPos, yPos);

            for (i = 0; i < slaveSize; i++)
            {
               for (j = 0; j < slaveSize; j++)
               {
                  grvResult[x+i][y+j] += density*slave[i][j][3];
               }
            }
         }
                       
                       /* ************************************************* */
                       /* Calculate Magnetics from all the axis information */
         if (magResult)
         {
                                                         /* ************************** */
            if (susCalc)                                 /* Sus Only (Not Anisotropic) */
            {
               if (indexData)
                  if (index < 0)
                     JR4 = 0.0;
                  else
                     JR4 = propArray[index]->sus_X;
               else
                  JR4 = getMagSus(magSusData[layer], xPos, yPos);

               if (remCalc) /* scale each axis independantly if need rem later */
               {
               /* this was for when remance was a percentage of sus which it is not now
                  susResult1 = susResult2
                             = susResult3 = intensity * JR4;
                  JR4 = 0.000000;
               */
               }
            }
            
                                                        /* ********************** */
            if (remCalc)                                /* Remanence Calculations */
            {
               if (indexData)
               {
                  if ((index < 0) || (!propArray[index]->remanentMagnetization))
                  {
                     remAngle = remIncl = remSusStr = (float) 0.0;
                  }
                  else
                  {
                     remAngle = propArray[index]->angleWithNorth;
                     remIncl = propArray[index]->inclination;
                     remSusStr = (float) propArray[index]->strength;
                  }
               }
               else
               {
                  remAngle = getRemSusDec(remSusDecData[layer], xPos, yPos);
                  remIncl = getRemSusAzi(remSusAziData[layer], xPos, yPos);
                  remSusStr = getRemSusStr(remSusStrData[layer], xPos, yPos);
               }

               cRf = cos(DEG_TO_RAD(remIncl));
               sRf = sin(DEG_TO_RAD(remIncl));

               remantResult1 = remSusStr * cRf
                            * sin(DEG_TO_RAD(-remAngle));  /* old cos */ /* declination- */
               remantResult2 = remSusStr * cRf
                            * cos(DEG_TO_RAD(-remAngle));  /* old sin */ /* declination- */
               remantResult3 = remSusStr * sRf;
            }
            

                                                        /* *********************** */
            if (aniCalc)                                /* Anisotropy Calculations */
            {
               if (indexData)
               {
                  if ((index < 0) || (!propArray[index]->anisotropicField))
                  {
                     aniSusDip = aniSusDDir = aniSusPitch = (float) 0.0;
                     susResult1 = susResult2 = susResult3 = 0.0;
                               /* treat as through it was a normal sus calculation */
                     if (index >= 0)
                        JR4 = propArray[index]->sus_X;
                  }
                  else
                  {
                     aniSusDip = (float) propArray[index]->sus_dip;
                     aniSusDDir = (float) propArray[index]->sus_dipDirection;
                     aniSusPitch = (float) propArray[index]->sus_pitch;

                     aniSusDip = (float) DEG_TO_RAD(aniSusDip); 
                     aniSusDDir = (float) DEG_TO_RAD(aniSusDDir);
                     aniSusPitch = (float) DEG_TO_RAD(aniSusPitch); 

                                             /* the initial axis */
                     sus1 = propArray[index]->sus_X;
                     sus2 = propArray[index]->sus_Y;
                     sus3 = propArray[index]->sus_Z;

                                             /* work out components */
                     n = sus2*cos(aniSusPitch) - sus1*sin(aniSusPitch);
                     susX = -(n*sin(aniSusDip) + sus3*cos(aniSusDip));
                     n = n*cos(aniSusDip) - sus3*sin(aniSusDip);
                     m = sus1*cos(aniSusPitch) + sus2*sin(aniSusPitch);
                     susY=m*cos(aniSusDDir)-n*sin(aniSusDDir);
                     susZ=m*sin(aniSusDDir)+n*cos(aniSusDDir);

                     susResult1 = intensity * susX;
                     susResult2 = intensity * susY;
                     susResult3 = intensity * susZ;
                  }
               }
               else  /* indexData */
               {
                  aniSusDip = getAniSusDip (aniSusDipData[layer], xPos, yPos);
                  aniSusDDir = getAniSusDDir (aniSusDDirData[layer], xPos, yPos);
                  aniSusPitch = getAniSusPitch (aniSusPitchData[layer], xPos, yPos);
                                             /* the initial axis */
                  sus1 = getAniSusAxis1(aniSusAxis1Data[layer], xPos, yPos);
                  sus2 = getAniSusAxis2(aniSusAxis2Data[layer], xPos, yPos);
                  sus3 = getAniSusAxis3(aniSusAxis3Data[layer], xPos, yPos);

                  if ((fabs(aniSusDip) < TOLERANCE) &&
                      (fabs(aniSusDDir) < TOLERANCE) &&
                      (fabs(aniSusPitch) < TOLERANCE))
                  {
                     susResult1 = susResult2 = susResult3 = 0.0;
                               /* treat as through it was a normal sus calculation */
                     JR4 = sus1;
                  }
                  else
                  {
                     aniSusDip = (float) DEG_TO_RAD(aniSusDip); 
                     aniSusDDir = (float) DEG_TO_RAD(aniSusDDir);
                     aniSusPitch = (float) DEG_TO_RAD(aniSusPitch); 

                                             /* work out components */
                     n = sus2*cos(aniSusPitch) - sus1*sin(aniSusPitch);
                     sus3 = -(n*sin(aniSusDip) + sus3*cos(aniSusDip));
                     n = n*cos(aniSusDip) - sus3*sin(aniSusDip);
                     m = sus1*cos(aniSusPitch) + sus2*sin(aniSusPitch);
                     sus2=m*cos(aniSusDDir)-n*sin(aniSusDDir);
                     sus1=m*sin(aniSusDDir)+n*cos(aniSusDDir);

                     susResult1 = intensity * sus1;
                     susResult2 = intensity * sus2;
                     susResult3 = intensity * sus3;
                  }
               } /* indexData */
            } /* Ani Calc */

                               /* Main calculations to move the convolution over
                               ** with the correct Values finally worked out */
            JR1 = susResult1*sacf + remantResult1;
            JR2 = susResult2*cacf + remantResult2;
            JR3 = susResult3*sf   + remantResult3;

            for (i = 0; i < slaveSize; i++)
            {
               for (j = 0; j < slaveSize; j++)
               {
                  magResult[x+i][y+j] += -JR1*slave[i][j][0]
                                         -JR2*slave[i][j][1]
                                         -JR3*slave[i][j][2]
                                         -JR4*slave[i][j][4];
               }
            }
         }
      }
   }
      
   return (TRUE);
}                 

/*  OLD GEOPHYSICS LOOP from calcCompleteAnomalies
				{
					.* yPos = minY + (ny-1-y)*cubeSize; *.
					yPos = minY + y*cubeSize;

					getFieldAtLocation (options, xPos+halfCubeSize, yPos+halfCubeSize,
											  zPos+halfCubeSize, &inclination, &declination, &intensity);

					if (magneticResults || magXResults || magYResults || magZResults)
						if (!getMagneticVectors(&blockSusX, &blockSusY, &blockSusZ,
											  intensity, inclination, declination,
											  x, y, z, indexData, propArray, magSusData,
											  remSusDecData,   remSusAziData,   remSusStrData,
											  aniSusDipData,   aniSusDDirData,  aniSusPitchData,
											  aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data))
							skipPoint = TRUE;

					if (gravityResults)
						if (!getGravityVector(&blockDensity,
											  x, y, z, indexData, propArray, densityData))
							skipPoint = TRUE;

					if (skipPoint)
						skipPoint = FALSE;  .* Only Skip this one not the next *.
					else
					{

						calcAnomAtPoints(xPos, yPos, zPos, xPos+cubeSize, yPos+cubeSize, zPos+cubeSize,
								 blockSusX, blockSusY, blockSusZ, blockDensity, intensity,
								 inclination, declination, pointsToMeasure, numPointsToMeasure,
								 magneticResults, magXResults, magYResults, magZResults,
								 gravityResults, TRUE, cubeRange, sphereRange);
					}
				}
*/




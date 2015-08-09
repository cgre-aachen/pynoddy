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
#else
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include "noddy.h"
#include <math.h>

#define DEBUG(X)    
#define DEBUGX(X)                       

#define VERSION_TOLERANCE     0.01
#define TOLERANCE             0.0001
#define STRAT_LIMIT           100

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern int TopoRow, TopoCol; 
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE;
extern double **topographyMap;
extern int batchExecution;

#if XVT_CC_PROTO
extern void Remfind(double *, int, LAYER_PROPERTIES *, int, int, double *, double *, double *, double *, double *);
extern int planeIntersect (double *, double *, double, double, double, double, double, double);
extern void Anifind(double *, int, LAYER_PROPERTIES *, int, int, double *, double *, double *, double *, double *);
extern int assignLayerInBlockModels (LAYER_PROPERTIES ***, int, int,
                   int, int, LAYER_PROPERTIES **, int , int ,
                   short ***, float ***, float ***, float ***,
                   float ***, float ***, float ***, float ***,
                   float ***, float ***, float ***, float ***);
extern double fault(double [10][4], FAULT_OPTIONS *);
extern double plug (double [10][4], PLUG_OPTIONS *);
extern double dyke (double [10][4], DYKE_OPTIONS *);
extern int forwardModel (int, int, double, double, double, double *, double *, double *);
#else
extern void Remfind();
extern int planeIntersect ();
extern void Anifind();
extern int assignLayerInBlockModels ();
extern double fault();
extern double plug ();
extern double dyke ();
extern int forwardModel ();
#endif

                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
COLOR ***convertFloatBlockDataToColors(float ***, int, int, int, COLOR *, int);
COLOR ***convertIntBlockDataToColors(int ***, int, int, int, COLOR *, int);
int readBlockHeader (char *, int *, int *, int *, int *, int *,
                     double *, double *, double *, double *, double *, double *,
                     double *, double *, double *);
int readBlockFile (char *, int ***, int *, int, int, int, float [121],
                   float [121][4], double [121][4], double [121],
                   double [121], int [121], int *, int *,
                   double *, double *, double *, double *, double *, double *,
                   BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *);
void calcBlockModel (int ***, double ***, STORY **, double **, double, double,
                   int *, int, int, int, float [121],
                   float [121][4], double [121][4], double [121],
                   double [121], int [121], int *, int *, char *);
int calcBlockLayer(LAYER_PROPERTIES ***, int, int, double,
             double, double, int, int, int, LAYER_PROPERTIES **,
             int, int, int, int, short ***, float ***,
             float ***, float ***, float ***, float ***,
             float ***, float ***, float ***, float ***,
             float ***,float ***);
COLOR ***calcBlockDiagramColors(double, double, double, int, int, int, double, int, LAYER_PROPERTIES **);
int calcReverseSection(double, double, double, double,
                       double, double, double, double ****, STORY ***, int *, int *);
COLOR **calcBlockSectionColors(double, double, double, double,
                        double, double, double, int *, int *);
int **calcBlockSectionIndexs(double, double, double, double,
                        double, double, double, int *, int *);
LAYER_PROPERTIES ***calcBlockSection(double, double, double, double,
                        double, double, double, int *, int *);
float ***calcBlockPropertiesData (PROPERTY_TYPE, double, double, double, double, int, int, int);
int calcDeformRemanence(LAYER_PROPERTIES ***, double ***,
                        STORY **, int, int, float **, float **);
int calcDeformAnisotropy(LAYER_PROPERTIES ***, double ***,
                        STORY **, int, int, float **, float **, float **);
int calcAlterationZone(LAYER_PROPERTIES ***, double ***, STORY **,
             int, int, int, int, LAYER_PROPERTIES **,    float ***,
             float ***, float ***, float ***, float ***, float ***,
             float ***, float ***, float ***, float ***, float ***);
int writeBlockFile(int ***, char *, int, int, int, int,
                   int, float [121], float [121][4], double [121][4],
                   double [121], double [121], int [121], int, int);
int writeBlockDiagramFile (int ***, char *, int, int, int, int);
int writeGeologyBlockFile (int ***, char *, int, int, int, int);
int readGeoBlockHeader (FILE *, int *, int *, int *, double *, char *);
int writeDicerBlockFile (int ***, char *, int, int, int, int);
void initBlockModelProfile (double ***, STORY **, int, int,
                            double, double, double, double, double);
void initBlockModelDicer (double ***, STORY **, int, int, int,
                                      double, double, double);
void initBlockModelAnomMap (double ***, STORY **, int, int, int,
                                        double, double, double);
LAYER_PROPERTIES *calcBlockPixel(double, double, double, int, int,
                         float *, float *, int, float *, float *, float *);

void calcAndSaveBlock (char *, char *, int, int, int);
void DoSaveBlockDicer (char *);

int writeBlockHeaderToFile (char *, int, int **, double, double, double,
         double, double, double, int, int *, int, double, double,
         double, int, int, int, int, int, int, LAYER_PROPERTIES **);
int write3DBlockToFile (char *, char ***, int, int, int, int);
int write3DIregBlockToFile (char *, char ***, int, int **, int);
int write2DBlockToFile (char *, char **, int, int, int);
int write2DBlock (FILE *, char **, int, int, int);
int writeLineThroughBlock (char *, double **, int);
int readBlockHeaderFromFile (char *, int *, int ***, double *, double *, double *,
         double *, double *, double *, int *, int **, int *, double *, double *,
         double *, int *, int *, int *, int *, int *, int*, LAYER_PROPERTIES **);
int read3DBlockFromFile (char *, char ***, int, int, int, int);
int read3DIregBlockFromFile (char *, char ***, int, int **, int);
int read2DBlockFromFile (char *, char **, int, int, int);
int read2DBlock (FILE *, char **, int, int, int);
int convertOldBlkToNewBlk (FILE_SPEC *, FILE_SPEC *, int, int, int);
int extractXYZFromFile (FILE_SPEC *, int, int, int, double, double, double,
                        double ***, int *);
int extractXYZHeader (FILE_SPEC *, int *);
double distanceToContact (double, double, double, OBJECT *);
int distanceToVector (double, OBJECT *, double *, double *, double *);
static int applyAlterations (double,
             PROFILE_OPTIONS *, int, int, int, float ***,
             float ***, float ***, float ***, float ***, float ***,
             float ***, float ***, float ***, float ***, float ***);
#else
COLOR ***convertFloatBlockDataToColors();
COLOR ***convertIntBlockDataToColors();
int readBlockHeader ();
int readBlockFile ();
void calcBlockModel ();
int calcBlockLayer();
COLOR ***calcBlockDiagramColors();
COLOR **calcBlockSectionColors();
int **calcBlockSectionIndexs();
LAYER_PROPERTIES ***calcBlockSection();
float ***calcBlockPropertiesData ();
int calcDeformRemanence();
int calcDeformAnisotropy();
int calcAlterationZone();
LAYER_PROPERTIES *calcBlockPixel();
int writeBlockFile();
int writeGeologyBlockFile();
int readGeoBlockHeader ();
int writeDicerBlockFile();
void initBlockModelProfile ();
void initBlockModelDicer ();
void initBlockModelAnomMap ();
void calcAndSaveBlock ();
void DoSaveBlockDicer ();

int writeBlockHeaderToFile ();
int write3DBlockToFile ();
int write3DIregBlockToFile ();
int write2DBlockToFile ();
int write2DBlock ();
int writeLineThroughBlock ();
int readBlockHeaderFromFile ();
int read3DBlockFromFile ();
int read3DIregBlockFromFile ();
int read2DBlockFromFile ();
int read2DBlock ();
int convertOldBlkToNewBlk ();
int extractXYZFromFile ();
int extractXYZHeader ();
double distanceToContact ();
int distanceToVector ();
static int applyAlterations ();
#endif


/* ======================================================================
FUNCTION        convertFloatBlockDataToColors

DESCRIPTION
        Convert a data format to colors
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
COLOR ***
#if XVT_CC_PROTO
convertFloatBlockDataToColors(float ***blockValueData, int nx, int ny, int nz,
                              COLOR *lut, int numColors)
#else
convertFloatBlockDataToColors(blockValueData, nx, ny, nz, lut, numColors)
float ***blockValueData;
int nx, ny, nz;
COLOR *lut;
int numColors;
#endif
{
   COLOR ***blockColorData = NULL;
   double minValue, maxValue; 
   double valueInc, value;
   int x, y, z, index;
    
   if (!blockValueData || !lut)
      return (blockColorData);
      
   if (!(blockColorData = (COLOR ***) create3DArray(nz, nx, ny, sizeof(COLOR))))
      return (blockColorData);
      
   minValue = maxValue = (double) blockValueData[0][0][0];
   for (z = 0; z < nz; z++)
   {
      for (x = 0; x < nx; x++)
      {
         for (y = 0; y < ny; y++)
         {
            value = (double) blockValueData[z][x][y];
            if (value < minValue)
               minValue = value;
            else if (value > maxValue)
               maxValue = value;
         }
      }
   }
   valueInc = (maxValue - minValue)/ ((double) numColors);
   
   for (z = 0; z < nz; z++)
   {
      for (x = 0; x < nx; x++)
      {
         for (y = 0; y < ny; y++)
         {
            index = (int) floor (((double) blockValueData[z][x][ny-y-1] - minValue)/valueInc);
            if (index < 0)
               index = 0;
            else if (index > numColors-1)
               index = numColors-1;
            blockColorData[z][x][y] = lut[index];
         }
      }
   }

   return (blockColorData);
}

/* ======================================================================
FUNCTION        convertIntBlockDataToColors

DESCRIPTION
        Convert a data format to colors
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
COLOR ***
#if XVT_CC_PROTO
convertIntBlockDataToColors(int ***blockValueData, int nx, int ny, int nz,
                              COLOR *lut, int numColors)
#else
convertIntBlockDataToColors(blockValueData, nx, ny, nz, lut, numColors)
int ***blockValueData;
int nx, ny, nz;
COLOR *lut;
int numColors;
#endif
{
   COLOR ***blockColorData = NULL;
   double minValue, maxValue; 
   double valueInc, value;
   int x, y, z, index;
   
   if (!blockValueData || !lut)
      return (blockColorData);
      
   if (!(blockColorData = (COLOR ***) create3DArray(nz, nx, ny, sizeof(COLOR))))
      return (blockColorData);
      
   minValue = maxValue = (double) blockValueData[0][0][0];
   for (z = 0; z < nz; z++)
   {
      for (x = 0; x < nx; x++)
      {
         for (y = 0; y < ny; y++)
         {
            value = (double) blockValueData[z][x][y];
            if (value < minValue)
               minValue = value;
            else if (value > maxValue)
               maxValue = value;
         }
      }
   }
   valueInc = (maxValue - minValue)/ ((double) numColors);
   
   for (z = 0; z < nz; z++)
   {
      for (x = 0; x < nx; x++)
      {
         for (y = 0; y < ny; y++)
         {
            index = (int) floor (((double) blockValueData[z][x][y] - minValue)/valueInc);
            if (index < 0) index = 0;
            else if (index > numColors-1) index = numColors-1;
            blockColorData[z][x][y] = lut[index];
         }
      }
   }

   return (blockColorData);
}


/* ======================================================================
FUNCTION        calcTopology

DESCRIPTION
        Calc the topology codes for a given layer height

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
calcTopology(LAYER_PROPERTIES ***blockLayer, int nx, int ny,
    double xLoc, double yLoc, double zLoc, int blockSize, int useTopo,
    int numProps, LAYER_PROPERTIES **rockProps, int deformRemanence,
    int deformAnisotropy, int alterationZones, int zIndex, short ***indexData,
    float ***densityData,    float ***magSusData,     float ***remSusDecData,
    float ***remSusAziData,  float ***remSusStrData,  float ***aniSusDipData,
    float ***aniSusDDirData, float ***aniSusPitchData,float ***aniSusAxis1Data,
    float ***aniSusAxis2Data,float ***aniSusAxis3Data, FILE *topo_out)
#else
calcTopology(blockLayer, nx, ny, xLoc, yLoc, zLoc, blockSize, useTopo,
               numProps, rockProps, deformRemanence, deformAnisotropy,
               alterationZones, zIndex, indexData, densityData, magSusData,
               remSusDecData, remSusAziData, remSusStrData,
               aniSusDipData, aniSusDDirData, aniSusPitchData,
               aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data, *topo_out)
LAYER_PROPERTIES ***blockLayer;
int nx, ny;
double xLoc, yLoc, zLoc;
int blockSize, useTopo, numProps;
LAYER_PROPERTIES **rockProps;
int deformRemanence, deformAnisotropy, alterationZones, zIndex;
short ***indexData;
float ***densityData,     ***magSusData,      ***remSusDecData,
      ***remSusAziData,   ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData,  ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data, ***aniSusAxis3Data;
FILE *topo_out;
#endif
{
   int numEvents = countObjects(NULL_WIN);
   unsigned int rockType;
   int eventIndex;
	double zLocBase;
   int x, y,codes;
   double ***xyzLoc = NULL, **topoOverlay = NULL;
   STORY **histoire = NULL;
   int tempcode;

	zLocBase = (double) zLoc - blockSize;

	 /* NOTE all the +1's on array dimensions and
    ** Index's is because the original reverseEvents
    ** call that is still used requires the xyzLoc
    ** and historie arrays to start at 1 (a hang over
    ** from when this code was in "Fortran" no less.
    ** Anyway the xyz Locations are also from index 1
    ** that is why xyzLoc has a third dim of 4 and not 3
    */

   xyzLoc = (double ***) create3DArray (nx+1, ny+1, 4, sizeof(double));
   histoire = (STORY **) create2DArray (nx+1, ny+1, sizeof(STORY));
   if (!xyzLoc || !histoire)
   {
      destroy3DArray ((char ***) xyzLoc, nx+1, ny+1, 4);
      destroy2DArray ((char **) histoire, nx+1, ny+1);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to calculate block Layer");
      else
         //xvt_dm_post_error ("Error, Not enough memory to calculate block Layer");
      return (FALSE);
   }

                       /* *********************************** */
                       /* Allocate Topography if it is needed */
   if (useTopo && geologyOptions.useTopography)
   {
      topoOverlay = (double **) create2DArray (nx+1, ny+1, sizeof(double));
      overlayTopoOnArray (topographyMap, TopoCol, TopoRow,
                          TopomapYW, TopomapYE, TopomapXW, TopomapXE,
                          topoOverlay, nx, ny,
                          yLoc-blockSize/2.0, (yLoc-blockSize/2.0) + (ny)*blockSize,  /* give full extents, not 1/2 blocksize in */
                          xLoc-blockSize/2.0, (xLoc-blockSize/2.0) + (nx)*blockSize);
/*
                          yLoc-blockSize, yLoc + (ny-1)*blockSize,
                          xLoc-blockSize, xLoc + (nx-1)*blockSize);
*/
	}

                                    /* **************** */
                                    /* Initialise Layer */
   for (x = 0; x < nx; x++)
   {
      for (y = 0; y < ny; y++)
      {
         xyzLoc[x+1][y+1][1] = (x*blockSize) + xLoc;
         xyzLoc[x+1][y+1][2] = ((ny-y-1)*blockSize) + yLoc;
         xyzLoc[x+1][y+1][3] = zLoc;
         histoire[x+1][y+1].again = TRUE;
         izero(histoire[x+1][y+1].sequence);
      }
   }

   if (abortLongJob ())
      return (FALSE);
                       /* ************************************ */
                       /* Do the forward modeling of the layer */
   reverseEvents (xyzLoc, histoire, nx, ny);
   for (y = 0; y < ny; y++)
    {
       for (x = 0; x < nx; x++)
       {
    	   for(codes=0;codes<numEvents;codes++)
    		   {
    		   	   if(G2Bits(histoire[x+1][y+1].sequence,codes) == BASE_STRAT)
    		   		   tempcode=0;
    		   	   else if(G2Bits(histoire[x+1][y+1].sequence,codes) == UNC_STRAT)
		   		   	   tempcode=3;
    		   	   else if(G2Bits(histoire[x+1][y+1].sequence,codes) == FAULT1_STRAT)
		   		   	   tempcode=2;
    		   	   else if(G2Bits(histoire[x+1][y+1].sequence,codes) == IGNEOUS_STRAT)
		   		   	   tempcode=5;
    		   	   else if(G2Bits(histoire[x+1][y+1].sequence,codes) == FAULT2_STRAT)
		   		   	   tempcode=7;
    		   	   else if(G2Bits(histoire[x+1][y+1].sequence,codes) == FAULT3_STRAT)
		   		   	   tempcode=8;
    		   	   else
		   		   	   tempcode=9;

    		   	   fprintf(topo_out,"%0d",tempcode);
    		   }

    		   fprintf(topo_out,"\t");
       }
	   fprintf(topo_out,"\n");
      }
   fprintf(topo_out,"\n");

                       /* ***************************** */
                       /* Assign the block layer values */
   for (y = 0; y < ny; y++)
   {
      for (x = 0; x < nx; x++)
      {
								/* Clear block if any of it is above topo */
         if (topoOverlay && (zLocBase > topoOverlay[x+1][y+1]))
            blockLayer[x][y] = (LAYER_PROPERTIES *) NULL;
         else
         {
            taste(numEvents, histoire[x+1][y+1].sequence,
                             &rockType, &eventIndex);
            blockLayer[x][y] = whichLayer (eventIndex, xyzLoc[x+1][y+1][1],
                                        xyzLoc[x+1][y+1][2], xyzLoc[x+1][y+1][3]);
         }
      }
   }

                       /* ************************************* */
                       /* Assign Deformable Remenance if needed */
   if (deformRemanence && remSusDecData && remSusAziData)
      calcDeformRemanence(blockLayer, xyzLoc, histoire, nx, ny,
                remSusDecData[zIndex], remSusAziData[zIndex]);

                       /* ************************************* */
                       /* Assign Deformable Remenance if needed */
   if (deformAnisotropy && aniSusDipData && aniSusDDirData && aniSusPitchData)
      calcDeformAnisotropy(blockLayer, xyzLoc, histoire, nx, ny,
                aniSusDipData[zIndex], aniSusDDirData[zIndex], aniSusPitchData[zIndex]);

                       /* ******************************************** */
                       /* Assign The storing arrays the correct values */
   assignLayerInBlockModels (blockLayer,
               zIndex, nx, ny, numProps, rockProps,
               deformRemanence,  deformAnisotropy,
               indexData, densityData, magSusData, remSusDecData,
               remSusAziData,   remSusStrData,   aniSusDipData,
               aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
               aniSusAxis2Data, aniSusAxis3Data);

                       /* ********************************* */
                       /* Assign Alteration Zones if needed */
   if (alterationZones)
      calcAlterationZone(blockLayer, xyzLoc, histoire,
               zIndex, nx, ny,  numProps,        rockProps,
               densityData,     magSusData,      remSusDecData,
               remSusAziData,   remSusStrData,   aniSusDipData,
               aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
               aniSusAxis2Data, aniSusAxis3Data);

                                 /* ************************ */
                                 /* Free any memory Alocated */
   if (xyzLoc)
      destroy3DArray ((char ***) xyzLoc, nx+1, ny+1, 4);
   if (histoire)
      destroy2DArray ((char **) histoire, nx+1, ny+1);
   if (topoOverlay)
      destroy2DArray ((char **) topoOverlay, nx, ny);

#ifdef _MPL
   printf(" Done."); fflush (stdout);
#endif

   return (TRUE);
}


/* ======================================================================
FUNCTION        calcBlockLayer

DESCRIPTION
        Calc the block a given layer height
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
calcBlockLayer(LAYER_PROPERTIES ***blockLayer, int nx, int ny,
    double xLoc, double yLoc, double zLoc, int blockSize, int useTopo,
    int numProps, LAYER_PROPERTIES **rockProps, int deformRemanence,
    int deformAnisotropy, int alterationZones, int zIndex, short ***indexData,
    float ***densityData,    float ***magSusData,     float ***remSusDecData,
    float ***remSusAziData,  float ***remSusStrData,  float ***aniSusDipData,
    float ***aniSusDDirData, float ***aniSusPitchData,float ***aniSusAxis1Data,
    float ***aniSusAxis2Data,float ***aniSusAxis3Data)
#else
calcBlockLayer(blockLayer, nx, ny, xLoc, yLoc, zLoc, blockSize, useTopo,
               numProps, rockProps, deformRemanence, deformAnisotropy,
               alterationZones, zIndex, indexData, densityData, magSusData,
               remSusDecData, remSusAziData, remSusStrData, 
               aniSusDipData, aniSusDDirData, aniSusPitchData,
               aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data)
LAYER_PROPERTIES ***blockLayer;
int nx, ny;
double xLoc, yLoc, zLoc;
int blockSize, useTopo, numProps;
LAYER_PROPERTIES **rockProps;
int deformRemanence, deformAnisotropy, alterationZones, zIndex;
short ***indexData;
float ***densityData,     ***magSusData,      ***remSusDecData,
      ***remSusAziData,   ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData,  ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data, ***aniSusAxis3Data;
#endif
{
   int numEvents = countObjects(NULL_WIN);
   unsigned int rockType;
   int eventIndex;
	double zLocBase;
   int x, y;
   double ***xyzLoc = NULL, **topoOverlay = NULL;
   STORY **histoire = NULL;


	zLocBase = (double) zLoc - blockSize;

	 /* NOTE all the +1's on array dimensions and
    ** Index's is because the original reverseEvents
    ** call that is still used requires the xyzLoc
    ** and historie arrays to start at 1 (a hang over
    ** from when this code was in "Fortran" no less.
    ** Anyway the xyz Locations are also from index 1
    ** that is why xyzLoc has a third dim of 4 and not 3
    */

   xyzLoc = (double ***) create3DArray (nx+1, ny+1, 4, sizeof(double));
   histoire = (STORY **) create2DArray (nx+1, ny+1, sizeof(STORY));
   if (!xyzLoc || !histoire)
   {
      destroy3DArray ((char ***) xyzLoc, nx+1, ny+1, 4);
      destroy2DArray ((char **) histoire, nx+1, ny+1);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to calculate block Layer");
      else
         //xvt_dm_post_error ("Error, Not enough memory to calculate block Layer");
      return (FALSE);
   }
      
                       /* *********************************** */
                       /* Allocate Topography if it is needed */
   if (useTopo && geologyOptions.useTopography)
   {
      topoOverlay = (double **) create2DArray (nx+1, ny+1, sizeof(double));
      overlayTopoOnArray (topographyMap, TopoCol, TopoRow,
                          TopomapYW, TopomapYE, TopomapXW, TopomapXE,
                          topoOverlay, nx, ny,
                          yLoc-blockSize/2.0, (yLoc-blockSize/2.0) + (ny)*blockSize,  /* give full extents, not 1/2 blocksize in */
                          xLoc-blockSize/2.0, (xLoc-blockSize/2.0) + (nx)*blockSize);
/*
                          yLoc-blockSize, yLoc + (ny-1)*blockSize,
                          xLoc-blockSize, xLoc + (nx-1)*blockSize);
*/
	}

                                    /* **************** */
                                    /* Initialise Layer */
   for (x = 0; x < nx; x++)
   {
      for (y = 0; y < ny; y++)
      {
         xyzLoc[x+1][y+1][1] = (x*blockSize) + xLoc;
         xyzLoc[x+1][y+1][2] = ((ny-y-1)*blockSize) + yLoc;
         xyzLoc[x+1][y+1][3] = zLoc;
         histoire[x+1][y+1].again = TRUE;
         izero(histoire[x+1][y+1].sequence);
      }
   }

   if (abortLongJob ())
      return (FALSE);
                       /* ************************************ */   
                       /* Do the forward modeling of the layer */
   reverseEvents (xyzLoc, histoire, nx, ny);
                       
                       /* ***************************** */
                       /* Assign the block layer values */
   for (y = 0; y < ny; y++)
   {
      for (x = 0; x < nx; x++)
      {
								/* Clear block if any of it is above topo */
         if (topoOverlay && (zLocBase > topoOverlay[x+1][y+1]))
            blockLayer[x][y] = (LAYER_PROPERTIES *) NULL;
         else
         {
            taste(numEvents, histoire[x+1][y+1].sequence,
                             &rockType, &eventIndex);
            blockLayer[x][y] = whichLayer (eventIndex, xyzLoc[x+1][y+1][1],
                                        xyzLoc[x+1][y+1][2], xyzLoc[x+1][y+1][3]);
         }
      }
   }

                       /* ************************************* */
                       /* Assign Deformable Remenance if needed */
   if (deformRemanence && remSusDecData && remSusAziData)
      calcDeformRemanence(blockLayer, xyzLoc, histoire, nx, ny,
                remSusDecData[zIndex], remSusAziData[zIndex]);

                       /* ************************************* */
                       /* Assign Deformable Remenance if needed */
   if (deformAnisotropy && aniSusDipData && aniSusDDirData && aniSusPitchData)
      calcDeformAnisotropy(blockLayer, xyzLoc, histoire, nx, ny,
                aniSusDipData[zIndex], aniSusDDirData[zIndex], aniSusPitchData[zIndex]);
   
                       /* ******************************************** */
                       /* Assign The storing arrays the correct values */
   assignLayerInBlockModels (blockLayer,
               zIndex, nx, ny, numProps, rockProps,
               deformRemanence,  deformAnisotropy,
               indexData, densityData, magSusData, remSusDecData,
               remSusAziData,   remSusStrData,   aniSusDipData,
               aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
               aniSusAxis2Data, aniSusAxis3Data);

                       /* ********************************* */
                       /* Assign Alteration Zones if needed */
   if (alterationZones)
      calcAlterationZone(blockLayer, xyzLoc, histoire,
               zIndex, nx, ny,  numProps,        rockProps,
               densityData,     magSusData,      remSusDecData,
               remSusAziData,   remSusStrData,   aniSusDipData,
               aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
               aniSusAxis2Data, aniSusAxis3Data);

                                 /* ************************ */
                                 /* Free any memory Alocated */
   if (xyzLoc)
      destroy3DArray ((char ***) xyzLoc, nx+1, ny+1, 4);
   if (histoire)
      destroy2DArray ((char **) histoire, nx+1, ny+1);
   if (topoOverlay)
      destroy2DArray ((char **) topoOverlay, nx, ny);

#ifdef _MPL
   printf(" Done."); fflush (stdout);
#endif

   return (TRUE);
}


/* ======================================================================
FUNCTION        calcBlockDiagram

DESCRIPTION
        Calc the block between two 3d locations
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
COLOR ***
#if XVT_CC_PROTO
calcBlockDiagramColors(double minX, double minY, double minZ,
                 int nx, int ny, int nz, double blockSize,
                 int numLayersToDraw, LAYER_PROPERTIES **drawLayers)
#else
calcBlockDiagramColors(minX, minY, minZ, nx, ny, nz, blockSize,
                       numLayersToDraw, drawLayers)
double minX, minY, minZ;
int nx, ny, nz;
double blockSize;
int numLayersToDraw;
LAYER_PROPERTIES **drawLayers;
#endif
{
   COLOR ***blockData = NULL;
   double ***xyzLoc = NULL;
   STORY **histoire = NULL;
   int numEvents = countObjects(NULL_WIN);
   LAYER_PROPERTIES *layerProp;
   unsigned int rockType;
   int eventIndex;
   register int x, y, z, layer;
   double zLoc, halfCube;
   

    /* NOTE all the +1's on array dimensions and
    ** Index's is because the original reverseEvents
    ** call that is still used requires the xyzLoc
    ** and historie arrays to start at 1 (a hang over
    ** from when this code was in "Fortran" no less.
    ** Anyway the xyz Locations are also from index 1
    ** that is why xyzLoc has a third dim of 4 and not 3
    */
   blockData = (COLOR ***) create3DArray (nz, nx, ny, sizeof(COLOR));
   xyzLoc = (double ***) create3DArray (nx+1, ny+1, 4, sizeof(double));
   histoire = (STORY **) create2DArray (nx+1, ny+1, sizeof(STORY));
   if (!blockData || !xyzLoc || !histoire)
   {
      destroy3DArray ((char ***) blockData, nz, nx, ny);
      destroy3DArray ((char ***) xyzLoc, nx+1, ny+1, 4);
      destroy2DArray ((char **) histoire, nx+1, ny+1);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to calculate block Diagram");
      else
         //xvt_dm_post_error ("Error, Not enough memory to calculate block Diagram");
      return ((COLOR ***) NULL);
   }

   halfCube = ((double) blockSize)/2.0;
   for (z = 0, zLoc = minZ + halfCube; z < nz; z++, zLoc += blockSize)
   {
      incrementLongJob (INCREMENT_JOB);
                                       /* **************** */
                                       /* Initialise Layer */
      for (x = 0; x < nx; x++)
      {
         for (y = 0; y < ny; y++)
         {
            xyzLoc[x+1][y+1][1] = (x*blockSize) + minX + halfCube;
            xyzLoc[x+1][y+1][2] = (y*blockSize) + minY + halfCube;
            xyzLoc[x+1][y+1][3] = zLoc;
            histoire[x+1][y+1].again = TRUE;
            izero(histoire[x+1][y+1].sequence);
         }
      }
   
      if (abortLongJob ())
         return (FALSE);
                          /* ************************************ */   
                          /* Do the forward modeling of the layer */
      reverseEvents (xyzLoc, histoire, nx, ny);
                          
   
                          /* ***************************** */
                          /* Assign the block layer values */
      for (y = 0; y < ny; y++)
      {
         for (x = 0; x < nx; x++)
         {
            taste(numEvents, histoire[x+1][y+1].sequence,
                             &rockType, &eventIndex);

            if (layerProp = whichLayer (eventIndex, xyzLoc[x+1][y+1][1],
                                                    xyzLoc[x+1][y+1][2],
                                                    xyzLoc[x+1][y+1][3]))
            {
               blockData[z][x][y] = COLOR_INVALID;
               for (layer = 0; layer < numLayersToDraw; layer++)
               {
                  if (drawLayers[layer] == layerProp)
                  {
                     blockData[z][x][y] = XVT_MAKE_COLOR(layerProp->color.red,
                                                         layerProp->color.green,
                                                         layerProp->color.blue);
                     break;
                  }
               }
            }
         }
      }
   }

                                 /* ************************ */
                                 /* Free any memory Alocated */
   if (xyzLoc)
      destroy3DArray ((char ***) xyzLoc, nx+1, ny+1, 4);
   if (histoire)
      destroy2DArray ((char **) histoire, nx+1, ny+1);

   return (blockData);
}

/* ======================================================================
FUNCTION        calcBlockSection

DESCRIPTION
        Calc the block a Position, dip direction, and pitch
        NOTE this only does a 2d slice between the start and end
             locations, not a 3d block.
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
calcReverseSection(double xLocStart, double yLocStart,   double zLocStart,
                   double xLocEnd,   double yLocEnd,     double zLocEnd,
                   double cubeSize,  double ****pXyzLoc, STORY ***pHistoire,
                   int *pDim1, int *pDim2)
#else
calcReverseSection(xLocStart, yLocStart, zLocStart,
                   xLocEnd,   yLocEnd,   zLocEnd,
                   cubeSize,  pXyzLoc,   pHistoire, pDim1, pDim2)
double xLocStart, yLocStart, zLocStart;
double xLocEnd,   yLocEnd,   zLocEnd;
double cubeSize;
double ****pXyzLoc;
STORY ***pHistoire;
int *pDim1, *pDim2;
#endif
{
   double xInc, yInc, zInc;
   double diffX, diffY, diffZ;
   double halfCube;
   int dim1, dim2;
   int i, j;
   double ***xyzLoc = NULL;
   STORY **histoire = NULL;

   halfCube = cubeSize/2.0;
   
   diffX = xLocEnd - xLocStart;
   diffY = yLocEnd - yLocStart;
   diffZ = zLocEnd - zLocStart;

   if (fabs(diffZ) < TOLERANCE) /* No Z height */
   {                      /* first dim is line joining corners */
      dim1 = (int) floor (diffX/cubeSize);
      dim2 = (int) floor (diffY/cubeSize); /* second dimension is the height */

      xInc = diffX/dim1;  yInc = diffY/dim2;  zInc = 0.0;
   }
   else
   {                      /* first dim is line joining corners */
      dim1 = (int) floor (sqrt(diffX*diffX + diffY*diffY)/cubeSize);
      dim2 = (int) floor (diffZ/cubeSize); /* second dimension is the height */

      xInc = diffX/dim1;  yInc = diffY/dim1;  zInc = diffZ/dim2;
   }
   
   xLocStart += xInc/2.0;
   yLocStart += yInc/2.0;
   zLocStart += zInc/2.0;

   /* NOTE all the +1's on array dimensions and
   ** Index's is because the original reverseEvents
   ** call that is still used requires the xyzLoc
   ** and historie arrays to start at 1 (a hang over
   ** from when this code was in "Fortran" no less.
   ** Anyway the xyz Locations are also from index 1
   ** that is why xyzLoc has a third dim of 4 and not 3
   */

   xyzLoc = (double ***) create3DArray (dim1+1, dim2+1, 4, sizeof(double));
   histoire = (STORY **) create2DArray (dim1+1, dim2+1, sizeof(STORY));
   if (!xyzLoc || !histoire)
   {
      destroy3DArray ((char ***) xyzLoc,  dim1+1, dim2+1, 4);
      destroy2DArray ((char **) histoire, dim1+1, dim2+1);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to calculate block Layer");
      else
         //xvt_dm_post_error ("Error, Not enough memory to calculate block Layer");
      return (FALSE);
   }
      
                                    /* **************** */
                                    /* Initialise Layer */
   if (fabs(diffZ) < TOLERANCE) /* No Z height */
   { 
      for (i = 0; i < dim1; i++)
      {
         for (j = 0; j < dim2; j++)
         {
            xyzLoc[i+1][j+1][1] = (i*xInc) + xLocStart;
            xyzLoc[i+1][j+1][2] = (j*yInc) + yLocStart;
            xyzLoc[i+1][j+1][3] = zLocStart;
            histoire[i+1][j+1].again = TRUE;
            izero(histoire[i+1][j+1].sequence);
         }
      }
   }
   else
   {
      for (i = 0; i < dim1; i++)
      {
         for (j = 0; j < dim2; j++)
         {
            xyzLoc[i+1][j+1][1] = (i*xInc) + xLocStart;
            xyzLoc[i+1][j+1][2] = (i*yInc) + yLocStart;
            xyzLoc[i+1][j+1][3] = (j*zInc) + zLocStart;
            histoire[i+1][j+1].again = TRUE;
            izero(histoire[i+1][j+1].sequence);
         }
      }
   }

   if (abortLongJob ())
      return (FALSE);
                       /* ************************************ */   
                       /* Do the forward modeling of the layer */
   reverseEvents (xyzLoc, histoire, dim1, dim2);
                       
   *pDim1 = dim1;
   *pDim2 = dim2;
   *pXyzLoc = xyzLoc;
   *pHistoire = histoire;

   return (TRUE);
}

/* ======================================================================
FUNCTION        calcBlockSection

DESCRIPTION
        Calc the block a Position, dip direction, and pitch
        NOTE this only does a 2d slice between the start and end
             locations, not a 3d block.
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
COLOR **
#if XVT_CC_PROTO
calcBlockSectionColors(double xLocStart, double yLocStart, double zLocStart,
                       double xLocEnd,   double yLocEnd,   double zLocEnd,
                       double cubeSize, int *pDim1, int *pDim2)
#else
calcBlockSectionColors(xLocStart, yLocStart, zLocStart,
                       xLocEnd,   yLocEnd,   zLocEnd,
                       cubeSize, pDim1, pDim2)
double xLocStart, yLocStart, zLocStart;
double xLocEnd,   yLocEnd,   zLocEnd;
double cubeSize;
int *pDim1, *pDim2;
#endif
{
   COLOR **blockLayer;
   LAYER_PROPERTIES *layerProp;
   int numEvents = countObjects(NULL_WIN);
   unsigned int rockType;
   int eventIndex, dim1, dim2;
   int i, j;
   double ***xyzLoc = NULL;
   STORY **histoire = NULL;

   if (batchExecution)
   {
      fprintf(stdout,"\nCalculating Block Section."); fflush (stdout);
   }
   
   if (!calcReverseSection(xLocStart, yLocStart, zLocStart,
                      xLocEnd,   yLocEnd,   zLocEnd,
                      cubeSize,  &xyzLoc,   &histoire, &dim1, &dim2))
      return (NULL);                      

   blockLayer = (COLOR **) create2DArray (dim1, dim2, sizeof(COLOR));
   if (!blockLayer || !xyzLoc || !histoire)
   {
      destroy2DArray ((char **) blockLayer, dim1, dim2);
      destroy3DArray ((char ***) xyzLoc,  dim1+1, dim2+1, 4);
      destroy2DArray ((char **) histoire, dim1+1, dim2+1);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to calculate block Layer");
      else
         //xvt_dm_post_error ("Error, Not enough memory to calculate block Layer");
      return (NULL);
   }

                       /* ***************************** */
                       /* Assign the block layer values */
   for (j = 0; j < dim2; j++)
   {                               
      for (i = 0; i < dim1; i++)
      {
         taste(numEvents, histoire[i+1][j+1].sequence,
                          &rockType, &eventIndex);
         if (layerProp = whichLayer (eventIndex, xyzLoc[i+1][j+1][1],
                                     xyzLoc[i+1][j+1][2], xyzLoc[i+1][j+1][3]))
            blockLayer[i][j] = XVT_MAKE_COLOR(layerProp->color.red,
                                              layerProp->color.green, 
                                              layerProp->color.blue);
         else
            blockLayer[i][j] = COLOR_INVALID;
      }
   }

                                 /* ************************ */
                                 /* Free any memory Alocated */
   if (xyzLoc)
      destroy3DArray ((char ***) xyzLoc,  dim1+1, dim2+1, 4);
   if (histoire)
      destroy2DArray ((char **) histoire, dim1+1, dim2+1);

   *pDim1 = dim1;
   *pDim2 = dim2;
   
   if (batchExecution)
   {
      fprintf(stdout," Done."); fflush (stdout);
   }

   return (blockLayer);
}

/* ======================================================================
FUNCTION        calcBlockSectionIndexs

DESCRIPTION
        Calc the block a Position, dip direction, and pitch
        NOTE this only does a 2d slice between the start and end
             locations, not a 3d block.
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int **
#if XVT_CC_PROTO
calcBlockSectionIndexs(double xLocStart, double yLocStart, double zLocStart,
                       double xLocEnd,   double yLocEnd,   double zLocEnd,
                       double cubeSize, int *pDim1, int *pDim2)
#else
calcBlockSectionIndexs(xLocStart, yLocStart, zLocStart,
                       xLocEnd,   yLocEnd,   zLocEnd,
                       cubeSize, pDim1, pDim2)
double xLocStart, yLocStart, zLocStart;
double xLocEnd,   yLocEnd,   zLocEnd;
double cubeSize;
int *pDim1, *pDim2;
#endif
{
   int **blockLayer;
   LAYER_PROPERTIES *layerProp, *properties[50];
   int numEvents = countObjects(NULL_WIN);
   unsigned int rockType;
   int eventIndex, dim1, dim2;
   int i, j, numProps, prop;
   double ***xyzLoc = NULL;
   STORY **histoire = NULL;

   if (batchExecution)
   {
      fprintf(stdout,"\nCalculating Block Section."); fflush (stdout);
   }

   if (!calcReverseSection(xLocStart, yLocStart, zLocStart,
                      xLocEnd,   yLocEnd,   zLocEnd,
                      cubeSize,  &xyzLoc,   &histoire, &dim1, &dim2))
      return (NULL);                      

   blockLayer = (int **) create2DArray (dim1, dim2, sizeof(int));
   if (!blockLayer || !xyzLoc || !histoire)
   {
      destroy2DArray ((char **) blockLayer, dim1, dim2);
      destroy3DArray ((char ***) xyzLoc,  dim1+1, dim2+1, 4);
      destroy2DArray ((char **) histoire, dim1+1, dim2+1);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to calculate block Layer");
      else
         //xvt_dm_post_error ("Error, Not enough memory to calculate block Layer");
      return (NULL);
   }

                       /* ***************************** */
                       /* Assign the block layer values */
   numProps = assignPropertiesForStratLayers (properties, 50);

   for (j = 0; j < dim2; j++)
   {                               
      for (i = 0; i < dim1; i++)
      {
         taste(numEvents, histoire[i+1][j+1].sequence,
                          &rockType, &eventIndex);
         layerProp = whichLayer (eventIndex, xyzLoc[i+1][j+1][1],
                        xyzLoc[i+1][j+1][2], xyzLoc[i+1][j+1][3]);
         blockLayer[i][j] = -1;  /* No layer */
         for (prop = 0; prop < numProps; prop++)
         {
            if (properties[prop] == layerProp)
            {
               blockLayer[i][j] = prop;
               break;
            }
         }
      }
   }

                                 /* ************************ */
                                 /* Free any memory Alocated */
   if (xyzLoc)
      destroy3DArray ((char ***) xyzLoc,  dim1+1, dim2+1, 4);
   if (histoire)
      destroy2DArray ((char **) histoire, dim1+1, dim2+1);

   *pDim1 = dim1;
   *pDim2 = dim2;
   
   if (batchExecution)
   {
      fprintf(stdout," Done."); fflush (stdout);
   }

   return (blockLayer);
}


/* ======================================================================
FUNCTION        calcBlockSection

DESCRIPTION
        Calc the block a Position, dip direction, and pitch
        NOTE this only does a 2d slice between the start and end
             locations, not a 3d block.
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
LAYER_PROPERTIES ***
#if XVT_CC_PROTO
calcBlockSection(double xLocStart, double yLocStart, double zLocStart,
                 double xLocEnd,   double yLocEnd,   double zLocEnd,
                 double cubeSize, int *pDim1, int *pDim2)
#else
calcBlockSection(xLocStart, yLocStart, zLocStart,
                 xLocEnd,   yLocEnd,   zLocEnd,
                 cubeSize, pDim1, pDim2)
double xLocStart, yLocStart, zLocStart;
double xLocEnd,   yLocEnd,   zLocEnd;
double cubeSize;
int *pDim1, *pDim2;
#endif
{
   LAYER_PROPERTIES ***blockLayer;
   int numEvents = countObjects(NULL_WIN);
   unsigned int rockType;
   int eventIndex, dim1, dim2;
   int i, j;
   double ***xyzLoc = NULL;
   STORY **histoire = NULL;

   if (batchExecution)
   {
      fprintf(stdout,"\nCalculating Block Section."); fflush (stdout);
   }

   if (!calcReverseSection(xLocStart, yLocStart, zLocStart,
                      xLocEnd,   yLocEnd,   zLocEnd,
                      cubeSize,  &xyzLoc,   &histoire, &dim1, &dim2))
      return (NULL);                      

   blockLayer = (LAYER_PROPERTIES ***) create2DArray (dim1, dim2, sizeof(LAYER_PROPERTIES *));
   if (!blockLayer || !xyzLoc || !histoire)
   {
      destroy2DArray ((char **) blockLayer, dim1, dim2);
      destroy3DArray ((char ***) xyzLoc,  dim1+1, dim2+1, 4);
      destroy2DArray ((char **) histoire, dim1+1, dim2+1);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to calculate block Layer");
      else
         //xvt_dm_post_error ("Error, Not enough memory to calculate block Layer");
      return (NULL);
   }

                       /* ***************************** */
                       /* Assign the block layer values */
   for (j = 0; j < dim2; j++)
   {                               
      for (i = 0; i < dim1; i++)
      {
         taste(numEvents, histoire[i+1][j+1].sequence,
                          &rockType, &eventIndex);
         blockLayer[i][j] = whichLayer (eventIndex, xyzLoc[i+1][j+1][1],
                                     xyzLoc[i+1][j+1][2], xyzLoc[i+1][j+1][3]);
      }
   }

                                 /* ************************ */
                                 /* Free any memory Alocated */
   if (xyzLoc)
      destroy3DArray ((char ***) xyzLoc,  dim1+1, dim2+1, 4);
   if (histoire)
      destroy2DArray ((char **) histoire, dim1+1, dim2+1);

   *pDim1 = dim1;
   *pDim2 = dim2;
   
   if (batchExecution)
   {
      fprintf(stdout," Done."); fflush (stdout);
   }

   return (blockLayer);
}


/* ======================================================================
FUNCTION        calcBlockImageValueData
DESCRIPTION
        calculate the valueData for the specific dataType
INPUT

OUTPUT  array for the given propty is [z][x][y] format

RETURNED
====================================================================== */
float ***
#if XVT_CC_PROTO
calcBlockPropertiesData (PROPERTY_TYPE dataType,
                         double originX, double originY, double originZ,
                         double cubeSize, int nx, int ny, int nz)
#else
calcBlockPropertiesData (dataType, originX, originY, originZ, cubeSize, nx, ny, nz)
PROPERTY_TYPE dataType;
double originX, originY, originZ, cubeSize;
int nx, ny, nz;
#endif
{
   short ***indexData = NULL;
   float ***densityData = NULL,    ***magSusData = NULL,     ***remSusDecData = NULL;
   float ***remSusAziData = NULL,  ***remSusStrData = NULL,  ***aniSusDipData = NULL;
   float ***aniSusDDirData = NULL, ***aniSusPitchData = NULL,***aniSusAxis1Data = NULL;
   float ***aniSusAxis2Data = NULL,***aniSusAxis3Data = NULL;
   float ***valueData = NULL;
   LAYER_PROPERTIES ***blockLayer = NULL;
   LAYER_PROPERTIES *rockProps[STRAT_LIMIT];
   int numProps, altZones, x, y, z;
   double halfBlockSize, zLoc;
   int calcValueData = TRUE;
   
   switch (dataType)
   {
      case (ROCK_PROP): /* Density */
         if (!(indexData = (short ***) create3DArray(nz, nx, ny, sizeof(short))))
            return (valueData);
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
         {
            destroy3DArray((char ***) indexData, nz, nx, ny);
            return (valueData);
         }
         break;
      case (DENSITY_PROP): /* Density */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         densityData = valueData;
         break;
      case (SUS_X_PROP): /* Sus X */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         aniSusAxis1Data = valueData;
         break;
      case (SUS_Y_PROP): /* Sus Y */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         aniSusAxis2Data = valueData;
         break;
      case (SUS_Z_PROP): /* Sus Z */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         aniSusAxis3Data = valueData;
         break;
      case (SUS_DIP_PROP): /* Sus Dip */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         aniSusDipData = valueData;
         if (geophysicsOptions.deformableAnisotropy && calcValueData)
         {
            aniSusDDirData = (float ***) create3DArray(nz, nx, ny, sizeof(float));
            aniSusPitchData = (float ***) create3DArray(nz, nx, ny, sizeof(float));
         }
         break;
      case (SUS_DDIR_PROP): /* Sus Dip Direction */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         aniSusDDirData = valueData;
         if (geophysicsOptions.deformableAnisotropy && calcValueData)
         {
            aniSusDipData = (float ***) create3DArray(nz, nx, ny, sizeof(float));
            aniSusPitchData = (float ***) create3DArray(nz, nx, ny, sizeof(float));
         }
         break;
      case (SUS_PITCH_PROP): /* Sus Pitch */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         aniSusPitchData = valueData;
         if (geophysicsOptions.deformableAnisotropy && calcValueData)
         {
            aniSusDipData = (float ***) create3DArray(nz, nx, ny, sizeof(float));
            aniSusDDirData = (float ***) create3DArray(nz, nx, ny, sizeof(float));
         }
         break;
      case (REM_STR_PROP): /* Rem Intensity */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         remSusStrData = valueData;
         break;
      case (REM_INC_PROP): /* Rem Inclination */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         remSusAziData = valueData;
         if (geophysicsOptions.deformableRemanence && calcValueData)
            remSusDecData = (float ***) create3DArray(nz, nx, ny, sizeof(float));
         break;
      case (REM_DEC_PROP): /* Rem Declination */
         if (!(valueData = (float ***) create3DArray(nz, nx, ny, sizeof(float))))
            return (valueData);
         remSusDecData = valueData;
         if (geophysicsOptions.deformableRemanence && calcValueData)
            remSusAziData = (float ***) create3DArray(nz, nx, ny, sizeof(float));
         break;
      default:
         return (valueData);
   }


   initLongJob (0, nz*nx, "Calculating Block Values...", NULL);

   halfBlockSize = cubeSize/2.0;
   altZones =  checkHistoryForAlterations ();
   numProps = assignPropertiesForStratLayers(rockProps, STRAT_LIMIT);
   if (blockLayer = (LAYER_PROPERTIES ***) create2DArray(nx, ny, sizeof (LAYER_PROPERTIES *)))
   {
      for (zLoc = originZ, z = nz-1; z >= 0; z--)
      {
         zLoc -= halfBlockSize;
         incrementLongJob(INCREMENT_JOB);
         calcBlockLayer(blockLayer, nx, ny,
             originX + halfBlockSize,
             originY + halfBlockSize, zLoc,
             (int) cubeSize, FALSE, numProps, rockProps,
             geophysicsOptions.deformableRemanence,
             geophysicsOptions.deformableAnisotropy, altZones, z, indexData,
             densityData,     magSusData,      remSusDecData,
             remSusAziData,   remSusStrData,   aniSusDipData,
             aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
             aniSusAxis2Data, aniSusAxis3Data);
         zLoc -= halfBlockSize;
      }
      destroy2DArray ((char **) blockLayer, nx, ny);
   }

   if (dataType == ROCK_PROP)
   {
      for (z = 0; z < nz; z++)
         for (x = 0; x < nx; x++)
            for (y = 0; y < ny; y++)
               valueData[z][x][y] = (float) indexData[z][x][y];
      destroy3DArray((char ***) indexData, nz, nx, ny);
   }
   
   if ((dataType == SUS_DIP_PROP) || (dataType == SUS_DDIR_PROP) || (dataType == SUS_PITCH_PROP))
   {
      if (aniSusDipData != valueData)
         destroy3DArray((char ***) aniSusDipData, nz, nx, ny);
      if (aniSusDDirData != valueData)
         destroy3DArray((char ***) aniSusDDirData, nz, nx, ny);
      if (aniSusPitchData != valueData)
         destroy3DArray((char ***) aniSusPitchData, nz, nx, ny);
   }
   if ((dataType == REM_INC_PROP) || (dataType == REM_DEC_PROP))
   {
      if (remSusAziData != valueData)
         destroy3DArray((char ***) remSusAziData, nz, nx, ny);
      if (remSusDecData != valueData)
         destroy3DArray((char ***) remSusDecData, nz, nx, ny);
   }
      
   finishLongJob ();

   return (valueData);
}


/* ======================================================================
FUNCTION        calcDeformRemanence

DESCRIPTION
   If the rock associated with a particular XYZ location has
   deformable remanence associated with it then the following
   happens.
   
   We have the XYZ location of the rock at creation (before the
   effects of any event that comes after it in the history 
   (see Unevents.c))
   
   We use two perpendicular planes to define a vector corresponding
   to the remanence vector that this rock has at creation. (remFind)
   
   We then use the forward modeling code to deform that vector
   according to the events that come after the creation of that
   rock. (done in remFind)
   
   The final orientation of the vector (the intersection of the two
   planes) is the deformed remanence vector for the location that
   is being calculated in the block. (done in px2)
        
RETURNED
   pointer to the LAYER_PROPERTIES
   NULL - error or over topography
====================================================================== */
int
#if XVT_CC_PROTO
calcDeformRemanence(LAYER_PROPERTIES ***blockLayer, double ***xyzLoc,
              STORY **histoire, int nx, int ny,
              float **remSusDecData, float **remSusAziData)
#else
calcDeformRemanence(blockLayer, xyzLoc, histoire, nx, ny,
                    remSusDecData, remSusAziData)
LAYER_PROPERTIES ***blockLayer;
double ***xyzLoc;
STORY **histoire;
int nx, ny;
float **remSusDecData, **remSusAziData;
#endif
{
   int numEvents = countObjects(NULL_WIN);
   unsigned int rockType;
   int eventIndex;
   int x, y;

   if (remSusDecData && remSusAziData)
   {
      double xAxis1, yAxis1, zAxis1;
      double xAxis2, yAxis2, zAxis2;
      double dip, dipDir;
      
      for (y = 0; y < ny; y++)
      {
         if (abortLongJob ())
            return (FALSE);

         for (x = 0; x < nx; x++)
         {
            if (blockLayer[x][y] && blockLayer[x][y]->remanentMagnetization)
            {
                               /* get the event num */
               taste(numEvents, histoire[x+1][y+1].sequence,
                                &rockType, &eventIndex);

                               /* deform 2 intersecting planes to
                               ** calculate the remanence vector */
               Remfind(xyzLoc[x+1][y+1],
                    eventIndex, blockLayer[x][y], eventIndex, 1,
                    &xAxis1, &yAxis1, &zAxis1, &dip, &dipDir);
               Remfind(xyzLoc[x+1][y+1],
                    eventIndex, blockLayer[x][y], eventIndex, 2,
                    &xAxis2, &yAxis2, &zAxis2, &dip, &dipDir);

                         /* Find intersection of two planes */
               planeIntersect (&dip, &dipDir,
                 xAxis1, xAxis2, yAxis1, yAxis2, zAxis1, zAxis2);
               remSusDecData[x][y] = (float) dipDir;
               remSusAziData[x][y] = (float) dip;
            }
            else
            {
               remSusDecData[x][y] = (float) 0.0;
               remSusAziData[x][y] = (float) 0.0;
            }
         }
      }
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        calcDeformAnisotropy

DESCRIPTION
   See the description of calcDeformRemanence. The only
   difference is that 3 planes are used as anistropy is 3
   ortogional axis which are deformed and not just a single 
   vector.
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
calcDeformAnisotropy(LAYER_PROPERTIES ***blockLayer, double ***xyzLoc,
                     STORY **histoire, int nx, int ny,
                     float **aniSusDipData, float **aniSusDDirData,
                     float **aniSusPitchData)
#else
calcDeformAnisotropy(blockLayer, xyzLoc, histoire, nx, ny,
                     aniSusDipData, aniSusDDirData, aniSusPitchData)
LAYER_PROPERTIES ***blockLayer;
double ***xyzLoc;
STORY **histoire;
int nx, ny;
float **aniSusDipData, **aniSusDDirData, **aniSusPitchData;
#endif
{
   int numEvents = countObjects(NULL_WIN);
   unsigned int rockType;
   int eventIndex;
   int x, y;

   if (aniSusDipData && aniSusDDirData && aniSusPitchData)
   {
      double xAxis1, yAxis1, zAxis1;
      double xAxis2, yAxis2, zAxis2;
      double xAxis3, yAxis3, zAxis3;
      double dip, dipDir;
      
      for (y = 0; y < ny; y++)
      {
         if (abortLongJob ())
            return (FALSE);

         for (x = 0; x < nx; x++)
         {
            if (blockLayer[x][y] && blockLayer[x][y]->anisotropicField)
            {
											/* get the event num */
               taste(numEvents, histoire[x+1][y+1].sequence,
                                &rockType, &eventIndex);

                               /* deform 3 intersecting planes to
                               ** calculate the anisotropy axis */
               Anifind(xyzLoc[x+1][y+1],
                    eventIndex, blockLayer[x][y], eventIndex, 1,
                    &xAxis1, &yAxis1, &zAxis1, &dip, &dipDir);
               Anifind(xyzLoc[x+1][y+1],
                    eventIndex, blockLayer[x][y], eventIndex, 2,
                    &xAxis2, &yAxis2, &zAxis2, &dip, &dipDir);
	if ((x == 13) && (y == 11))
	{
		dipDir = dipDir + 360;
		dipDir = dipDir - 360;
	}
               Anifind(xyzLoc[x+1][y+1],
                    eventIndex, blockLayer[x][y], eventIndex, 3,
                    &xAxis3, &yAxis3, &zAxis3, &dip, &dipDir);
	if (!(((dipDir > 115) && (dipDir < 125)) || ((dipDir  > 295) && (dipDir < 305))))
	{
		dipDir = dipDir + 360;
		dipDir = dipDir - 360;
	}

               aniSusDipData[x][y] = (float) dip;
               aniSusDDirData[x][y] = (float) dipDir;

                         /* Find intersection of two planes */
               planeIntersect (&dip, &dipDir,
                 xAxis1, xAxis2, yAxis1, yAxis2, zAxis1, zAxis2);
               aniSusPitchData[x][y] = (float) dip + (float) 90.0; 

/*
               px2 (&dip, &dipDir,
                 xAxis2, xAxis3, yAxis2, yAxis3, zAxis2, zAxis3);
               aniSusDipData[x][y] = (float) dip;
*/
            }
            else
            {
               aniSusDipData[x][y] = (float) 0.0;
               aniSusDDirData[x][y] = (float) 0.0;
               aniSusPitchData[x][y] = (float) 0.0;
            }
         }
      }
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        calcAlterationZone

DESCRIPTION
   If the rock associated with a particular XYZ location has
   an alteration zone associated with it then the following
   happens.
   
   We have the XYZ location of the rock at creation (before the
   effects of any event that comes after it in the history 
   (see Unevents.c))
   
   For each rock that we have that has an alteration zone associated
   with it. In order of creation of the rocks (earliest first) we
   do the following
   
      We loop through every other location in the block that is
      not of this rock type and change to rock properties of the 
      rock in that position according to the Alteration graphs
      of the rock we are doing the alteration zone for. The 
      distance is the the Distance to the nearest point of contact
      of the rock with the alteration zone.
   
RETURNED
   pointer to the LAYER_PROPERTIES
   NULL - error or over topography
====================================================================== */
int
#if XVT_CC_PROTO
calcAlterationZone(LAYER_PROPERTIES ***blockLayer, double ***xyzLoc,
      STORY **histoire, int z, int nx, int ny, int numProps, LAYER_PROPERTIES **rockProps,
      float ***densityData,    float ***magSusData,     float ***remSusDecData,
      float ***remSusAziData,  float ***remSusStrData,  float ***aniSusDipData,
      float ***aniSusDDirData, float ***aniSusPitchData,float ***aniSusAxis1Data,
      float ***aniSusAxis2Data,float ***aniSusAxis3Data)
#else
calcAlterationZone(blockLayer, xyzLoc, histoire,
              z, nx, ny, numProps, rockProps,
              densityData,    magSusData,     remSusDecData,
              remSusAziData,  remSusStrData,  aniSusDipData,
              aniSusDDirData, aniSusPitchData,aniSusAxis1Data,
              aniSusAxis2Data,aniSusAxis3Data)
LAYER_PROPERTIES ***blockLayer;
double ***xyzLoc;
STORY **histoire;
int z, nx, ny, numProps;
LAYER_PROPERTIES **rockProps;
float ***densityData,     ***magSusData,      ***remSusDecData,
      ***remSusAziData,   ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData,  ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data, ***aniSusAxis3Data;
#endif
{
   int numEvents = countObjects(NULL_WIN);
   OBJECT *objectPtr;
   int  x, y, lastEventNum, eventNum;
   ALTERATION_TYPE altType;
   PROFILE_OPTIONS *altFcns;
   LAYER_PROPERTIES *alterationRock = NULL;
   unsigned int rockType;
   int eventIndex;
   double distance;
   
   for (objectPtr = nthObject(NULL_WIN, 0), lastEventNum = 0, eventNum = 0; 
                     (objectPtr && (objectPtr->shape != STOP));
                                    objectPtr = objectPtr->next, eventNum++)
   {
      altType = FALSE;  /* check if event has alteration on it */
      switch (objectPtr->shape)
      {
         case (FAULT):
            {
               FAULT_OPTIONS *options = (FAULT_OPTIONS *) objectPtr->options;
               altType = options->alterationZones;
            }
            break;
         case (SHEAR_ZONE):
            {
               SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) objectPtr->options;
               altType = options->alterationZones;
            }
            break;
         case (UNCONFORMITY):
            {
               UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) objectPtr->options;
               altType = options->alterationZones;
            }
            break;
         case (PLUG):
            {
               PLUG_OPTIONS *options = (PLUG_OPTIONS *) objectPtr->options;
               altType = options->alterationZones;
            }
            break;
         case (DYKE):
            {
               DYKE_OPTIONS *options = (DYKE_OPTIONS *) objectPtr->options;
               altType = options->alterationZones;
            }
            break;
      }


      if (altType)  /* alteration associated with this event */
      {
                  /* setup things specific to this alteration */
         objectPtr->generalData = ALTERATION_PROFILE;
         altFcns = getProfileOptionsStructure(objectPtr);
         alterationRock = getLayerPropertiesStructure(objectPtr);

         for (y = 0; y < ny; y++)
         {
            if (abortLongJob ())
               return (FALSE);
   
            for (x = 0; x < nx; x++)
            {
               taste(numEvents, histoire[x+1][y+1].sequence, &rockType, &eventIndex);
               
                  /* Dont process the alt rock or rocks that are not created yet */
               if (blockLayer[x][y] && (blockLayer[x][y] != alterationRock)
						                  && (eventIndex < eventNum)
                                    && blockLayer[x][y]->applyAlterations)
               {
                  forwardModel (lastEventNum, eventNum,
                      xyzLoc[x+1][y+1][1],  xyzLoc[x+1][y+1][2],  xyzLoc[x+1][y+1][3],
                     &xyzLoc[x+1][y+1][1], &xyzLoc[x+1][y+1][2], &xyzLoc[x+1][y+1][3]);
                  distance = distanceToContact(xyzLoc[x+1][y+1][1],
                                               xyzLoc[x+1][y+1][2],
                                               xyzLoc[x+1][y+1][3],
                                               objectPtr);
                                               
                  if ((altType == BOTH_ALTERATION) 
                         || ((distance > 0.0) && (altType == TOP_ALTERATION))
                         || ((distance < 0.0) && (altType == BOTTOM_ALTERATION)))
                  {
                                /* each index in the array is 10 meters */
                     applyAlterations (distance, altFcns, z, x, y,
                             densityData,     magSusData,      remSusDecData,
                             remSusAziData,   remSusStrData,   aniSusDipData,
                             aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
                             aniSusAxis2Data, aniSusAxis3Data);
                  }
               }
            }
         }
         lastEventNum = eventNum;
      }
   }

   return (TRUE);
}
                                                              
                                                              
/* ======================================================================
FUNCTION        calcBlockPixel

DESCRIPTION
        Calc the block for a given xyz location
                                         
RETURNED
   pointer to the LAYER_PROPERTIES
   NULL - error or over topography
====================================================================== */
LAYER_PROPERTIES *calcBlockPixel(xLoc, yLoc, zLoc, useTopo,
             deformRemanence, remSusDec, remSusAzi,
             deformAnisotropy, aniSusDip, aniSusDDir, aniSusPitch)
double xLoc, yLoc, zLoc;
int useTopo, deformRemanence;
float *remSusDec, *remSusAzi;
int deformAnisotropy;
float *aniSusDip, *aniSusDDir, *aniSusPitch;
{
   LAYER_PROPERTIES **blockLayer;
   LAYER_PROPERTIES *resultLayer = NULL;
   float **pRemSusDec, **pRemSusAzi;
   float **pAniSusDip, **pAniSusDDir, **pAniSusPitch;
   float ***ppRemSusDec, ***ppRemSusAzi;
   float ***ppAniSusDip, ***ppAniSusDDir, ***ppAniSusPitch;
   int nx, ny;
   int altZones = FALSE;
         
   nx = ny = 1;
   blockLayer = &resultLayer;
	if (remSusDec)
	{
		pRemSusDec = &remSusDec;
		ppRemSusDec = &pRemSusDec;
	}
	else
		ppRemSusDec = NULL;

	if (remSusAzi)
	{
	   pRemSusAzi = &remSusAzi;
	   ppRemSusAzi = &pRemSusAzi;
	}
	else
	   ppRemSusAzi = NULL;
	
	if (aniSusDip)
	{
		pAniSusDip = &aniSusDip;
		ppAniSusDip = &pAniSusDip;
	}
	else
		ppAniSusDip = NULL;
	
	if (aniSusDDir)
	{
	   pAniSusDDir = &aniSusDDir;
		ppAniSusDDir = &pAniSusDDir;
	}
	else
	   ppAniSusDDir = NULL;
	
	if (aniSusPitch)
	{
		pAniSusPitch = &aniSusPitch;
		ppAniSusPitch = &pAniSusPitch;
	}
	else
		ppAniSusPitch = NULL;
   
   calcBlockLayer(&blockLayer, nx, ny, xLoc, yLoc, zLoc, 100, useTopo,
             0, (LAYER_PROPERTIES **) NULL, deformRemanence, deformAnisotropy,
             altZones, 0, (short ***) NULL, (float ***) NULL, (float ***) NULL,
             (float ***) ppRemSusDec, (float ***) ppRemSusAzi,  (float ***) NULL,
             (float ***) ppAniSusDip, (float ***) ppAniSusDDir, (float ***) ppAniSusPitch,
             (float ***) NULL,        (float ***) NULL,         (float ***) NULL);
   
   return (resultLayer);
}



/* ======================================================================
FUNCTION        writeBlockFile

DESCRIPTION
        write out a block to a file (Old format)
                                         
RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int writeBlockFile(blockData, blockName, nx, ny, nz, gblock,
                   nlitho, rho, sus, ken, ang, plu, num, piRem, piAni)
char *blockName;
#ifdef _MPL
plural int ***blockData;
#else
int ***blockData;
#endif
int nx, ny, nz, gblock;
int nlitho;
float rho[121],sus[121][4];
double ken[121][4],ang[121],plu[121];
int num[121];
int piRem,piAni;
{
   WINDOW eventWindow = (WINDOW) getEventDrawingWindow();
   WINDOW_INFO *wip;
   STRATIGRAPHY_OPTIONS *stratOptions;
   LAYER_PROPERTIES *propOptions;
   IMPORT_OPTIONS *importOptions;
   OBJECT *p;
   FILE *geophy_out;
   char outputFile[150], *ptr;
   int numEvents = (int) countObjects(NULL_WIN);
   int msize;
   double Inclination = geophysicsOptions.inclination;
   double Intensity = geophysicsOptions.intensity;
   double altitude = geophysicsOptions.altitude;
   int sum2, layer, index, value;
   double height, start;
   double sus_Y, sus_Z, kon_X, kon_Y, kon_Z, inclination, angleWithNorth;
   register int jj, kk;
   double gx1, gy1, gz1, gx2, gy2, gz2;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
#ifdef _MPL
   plural register int xIndex, yIndex;
#endif

   msize = (int) floor (geophysicsOptions.calculationRange/gblock);
   gx1 = viewOptions->originX;
   gy1 = viewOptions->originY;
   gz1 = viewOptions->originZ - viewOptions->lengthZ;
   gx2 = viewOptions->originX + viewOptions->lengthX;
   gy2 = viewOptions->originY + viewOptions->lengthY;
   gz2 = viewOptions->originZ;

#ifdef _MPL
   printf("\nWriting Block File."); fflush (stdout);
#endif
   wip = (WINDOW_INFO *) get_win_info(eventWindow);

   if (!blockName)
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for anomalies block output - No Name");
      else
         //xvt_dm_post_error("Can't open file for anomalies block output - No Name");
      return (FALSE);
   }
                        /* make sure the output file has a .blk extention */
   strcpy (outputFile, blockName);
   if (ptr = (char *) strchr (outputFile, '.'))
      strcpy (ptr, ".blk");
   else
      strcat (outputFile, ".blk");

   if (!(geophy_out = (FILE *) fopen (outputFile, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for anomalies block output");
      else
         //xvt_dm_post_error("Can't open file for anomalies block output");
      return (FALSE);
   }

                /* **************************************************** */
                /* Count the total number of stratigraphy layers (sum2) */
   for (p =  wip->head, sum2 = 0; p != NULL; p = p->next)
   {
      if ((p->shape == STRATIGRAPHY) || (p->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                            getStratigraphyOptionsStructure (p);
         sum2 += stratOptions->numLayers;
      }
      if (p->shape == IMPORT)
      {
         importOptions = (IMPORT_OPTIONS *) p->options;
         sum2 += importOptions->numProperties;
      }      
   }
   for (p =  wip->head, index = 0; p != NULL; p = p->next, index++)
   {
      if ((p->shape == DYKE) || (p->shape == PLUG))
         sum2++;
   }


                         /* ******************************************** */
                         /* Write out the actual file (first the header) */
   fprintf(geophy_out,"%s\n","7771");
/*
   if (ny == 1)  .* a profile rather than a map *.
      fprintf(geophy_out,"%lf\t%lf\t%lf\t%lf\n",Inclination,
      -(geophysicsOptions.orientation + 90.0 + geophysicsOptions.declination),
                                                         Intensity, altitude);
   else
*/
      fprintf(geophy_out,"%lf\t%lf\t%lf\t%lf\n",Inclination,
                      geophysicsOptions.declination, Intensity, altitude);
   
   fprintf(geophy_out,"%d\t%d\t%d\t%d\n%2f\n%d\n",
                                      nx,ny,nz,sum2,(float)gblock,msize);
/*
   if (ny == 1)  .* a profile rather than a map *.
   {
      fprintf(geophy_out,"%ld\t%ld\t%ld\n",(long) gx2, (long) gy2, (long) gz1);
      fprintf(geophy_out,"%ld\t%ld\t%ld\n",(long) geophysicsOptions.length,
                             (long) geophysicsOptions.orientation, (long) gz2);
   }
   else
*/
   {                         /* make sure the cordinated are right */
      gx1 = gx1 - (gblock*msize);
      gy1 = gy1 - (gblock*msize);
      gx2 = gx1 + (gblock*nx);
      gy2 = gy1 + (gblock*ny);
      fprintf(geophy_out,"%ld\t%ld\t%ld\n",(long) gx1, (long) gy1, (long) gz1);
      fprintf(geophy_out,"%ld\t%ld\t%ld\n",(long) gx2, (long) gy2, (long) gz2);
   }
   fprintf(geophy_out,"%d\t%2f\t%e\t%e\t%e\t%f\t%e\t%e\t%e\t%2f\t%2f\n",
                                 0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0);

   for (p =  wip->head, sum2 = 1; p != NULL; p = p->next)
   {
      if ((p->shape == STRATIGRAPHY) || (p->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                         getStratigraphyOptionsStructure (p);
         for (kk = 0; kk < stratOptions->numLayers; kk++, sum2++)
         {
            if (stratOptions->properties[kk].anisotropicField)
            {
               sus_Y = (double) stratOptions->properties[kk].sus_Y;
               sus_Z = (double) stratOptions->properties[kk].sus_Z;
               kon_Y = (double) 0.0;
               kon_Z = (double) 0.0;
            }
            else
            {
               sus_Y = 0.0; sus_Z = 0.0;
               kon_Y = 0.0; kon_Z = 0.0;
            }
            if (stratOptions->properties[kk].remanentMagnetization)
            {
               kon_X = (double) stratOptions->properties[kk].strength;
               inclination = (double)stratOptions->properties[kk].inclination;
               angleWithNorth = stratOptions->properties[kk].angleWithNorth;
            }
            else
            {
               kon_X = 0.0; inclination = 0.0; angleWithNorth = 0.0;
            }
            fprintf(geophy_out,
                       "%d\t%2f\t%e\t%e\t%e\t%f\t%e\t%e\t%e\t%2f\t%2f\n",
                       sum2,
                       (double) stratOptions->properties[kk].density,
                       (double) stratOptions->properties[kk].sus_X,
                       (double) sus_Y,
                       (double) sus_Z,
                       (double) stratOptions->properties[kk].anisotropicField,
                       (double) kon_X,
                       (double) kon_Y,
                       (double) kon_Z,
                       (double) inclination,
                       (double) angleWithNorth);
         }
      }
      if (p->shape == IMPORT)
      {
         importOptions = (IMPORT_OPTIONS *) (p->options);
         for (kk = 0; kk < importOptions->numProperties; kk++, sum2++)
         {
            if (importOptions->properties[kk].anisotropicField)
            {
               sus_Y = (double) importOptions->properties[kk].sus_Y;
               sus_Z = (double) importOptions->properties[kk].sus_Z;
               kon_Y = (double) 0.0;
               kon_Z = (double) 0.0;
            }
            else
            {
               sus_Y = 0.0; sus_Z = 0.0;
               kon_Y = 0.0; kon_Z = 0.0;
            }
            if (importOptions->properties[kk].remanentMagnetization)
            {
               kon_X = (double) importOptions->properties[kk].strength;
               inclination = (double)importOptions->properties[kk].inclination;
               angleWithNorth = importOptions->properties[kk].angleWithNorth;
            }
            else
            {
               kon_X = 0.0; inclination = 0.0; angleWithNorth = 0.0;
            }
            fprintf(geophy_out,
                       "%d\t%2f\t%e\t%e\t%e\t%f\t%e\t%e\t%e\t%2f\t%2f\n",
                       sum2,
                       (double) importOptions->properties[kk].density,
                       (double) importOptions->properties[kk].sus_X,
                       (double) sus_Y,
                       (double) sus_Z,
                       (double) importOptions->properties[kk].anisotropicField,
                       (double) kon_X,
                       (double) kon_Y,
                       (double) kon_Z,
                       (double) inclination,
                       (double) angleWithNorth);
         }
      }   
   }

   for (p =  wip->head; p != NULL; p = p->next)
   {
      if ((p->shape == DYKE) || (p->shape == PLUG))
      {
         propOptions = (LAYER_PROPERTIES *) getLayerPropertiesStructure (p);
         if (stratOptions->properties[kk].anisotropicField)
         {
            sus_Y = (double) propOptions->sus_Y;
            sus_Z = (double) propOptions->sus_Z;
            kon_Y = (double) 0.0;
            kon_Z = (double) 0.0;
         }
         else
         {
            sus_Y = 0.0; sus_Z = 0.0;
            kon_Y = 0.0; kon_Z = 0.0;
         }
         if (stratOptions->properties[kk].remanentMagnetization)
         {
            kon_X = (double) propOptions->strength;
            inclination =(double) propOptions->inclination;
            angleWithNorth = (double) propOptions->angleWithNorth;
         }
         else
         {
            kon_X = 0.0; inclination = 0.0; angleWithNorth = 0.0;
         }
         fprintf(geophy_out,
                    "%d\t%2f\t%e\t%e\t%e\t%f\t%e\t%e\t%e\t%2f\t%2f\n", sum2,
                    (double) propOptions->density,
                    (double) propOptions->sus_X,
                    (double) sus_Y,
                    (double) sus_Z,
                    (double) propOptions->anisotropicField,
                    (double) kon_X,
                    (double) kon_Y,
                    (double) kon_Z,
                    (double) inclination,
                    (double) angleWithNorth);
         sum2++;
      }
   }

                                                /* **************** */
                                                /* Write Block Data */
   start = gz2;   /* height to start writing from */
   for (height = start, layer = 0; height >= gz1;
                                              height -= gblock, layer++)
   {
      for (kk = 0; kk < ny; kk++)
      {
         for (jj = 0;jj < nx; jj++)
         {
#ifdef _MPL
                           /* only the processor with this element needs
                              to write it to the file */
            value = proc[kk%nyproc][jj%nxproc].
                                   blockData[layer][jj/nxproc][kk/nyproc];
            fprintf(geophy_out, "%d\t", value);
#else
            value = blockData[layer][jj+1][kk+1];
            fprintf(geophy_out, "%d\t", value);
#endif
         }
         fprintf(geophy_out,"\n");
      }
      if (ny != 1)  /* this line is not included in profile block output */
         fprintf(geophy_out,"\n");
   }
        
   fclose(geophy_out);

#if (XVTWS == MACWS)
   {
      FILE_SPEC fileSpec;

      xvt_fsys_get_dir (&(fileSpec.dir));
      strcpy (fileSpec.name, outputFile);
      xvt_fsys_set_file_attr(&fileSpec,XVT_FILE_ATTR_TYPESTR,(long) "TEXT");
      xvt_fsys_set_file_attr(&fileSpec,XVT_FILE_ATTR_CREATORSTR,(long)"Nody");
   }
#endif

#ifdef _MPL
   printf(" Done."); fflush (stdout);
#endif

   return (TRUE);
}


/* ======================================================================
FUNCTION        writeBlockDiagramFile
DESCRIPTION
        write out the blockData to a simple file
INPUT int ***blockData - the data to write out
        int xmax, ymax, zmax - dimensions of the data
        int header - flag TRUE if the output file needs a header

RETURNED
   TRUE on sucess, FALSE on error
====================================================================== */
int writeBlockDiagramFile(blockData, blockName, xmax, ymax, zmax, header)
char *blockName;
#ifdef _MPL
plural int ***blockData;
#else
int ***blockData;
#endif
int xmax, ymax, zmax;
int header;
{
   FILE *out;
   register int x, y, z, value;
   char outputFile[150], *ptr;
#ifdef _MPL
   char outStr[150];
   plural unsigned short data[3];
   plural int px, py, pz;
   char newLine[2];
#endif

#ifdef _MPL
   printf("\nWriting Block Diagram File."); fflush (stdout);
   strcpy(newLine,"\n");
#endif

   if (!blockName)
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for Block Diagram Output - No Name");
      else
         //xvt_dm_post_error("Can't open file for Block Diagram Output - No Name");
      return (FALSE);
   }
                        /* make sure the output file has a .blk extention */
   strcpy (outputFile, blockName);
   if (ptr = (char *) strchr (outputFile, '.'))
      strcpy (ptr, ".geo");
   else
      strcat (outputFile, ".geo");

#ifdef _MPL
   if (!(fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRWXG)))
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for Block Diagram Output");
      else
         //xvt_dm_post_error("Can't open file for Block Diagram Output");
      return (FALSE);
   }
#else
   if (!(out = (FILE *) fopen(outputFile, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for Block Diagram Output");
      else
         //xvt_dm_post_error("Can't open file for Block Diagram Output");
      return (FALSE);
   }
#endif

                                                /* ************* */
                                                /* Write XY FACE */
   if (header)
   {
#ifdef _MPL
      sprintf(outStr, "# Noddy %.2f - BINARY Block Diagram File (XY face)\n",
                                                              VERSION_NUMBER);
      write (fd, outStr, strlen(outStr));
      sprintf(outStr, "# x = %d, y = %d, z = 1\n", xmax, ymax);
      write (fd, outStr, strlen(outStr));
#else
      fprintf(out, "# Noddy %.2f - Block Diagram File (XY face)\n",
                                                              VERSION_NUMBER);
      fprintf(out, "# x = %d, y = %d, z = 1\n", xmax, ymax);
#endif
   }

#ifdef _MPL
   z = 0;
   for (py = iyproc; py < ymax; py+=nyproc)
   {
      for (px = ixproc; px < xmax; px+=nxproc)
      {
         data[0] = px;
         data[1] = py+1;
         data[2] = blockData[z][px/nxproc][py/nyproc];
         p_write (fd, data, 6);
      }
   }
   write(fd, newLine, 1);
#else
   z = 0;
   for (y = 0; y < ymax; y++)
   {
      for (x = 0; x < xmax; x++)
      {
         value = blockData[z][x+1][y+1];
         fprintf(out, "%d\t", value);
      }
      fprintf(out,"\n");
   }
#endif
        
                                                /* ************* */
                                                /* Write XZ FACE */
   if (header)
   {
#ifdef _MPL
      sprintf(outStr, "# Noddy %.2f - BINARY Block Diagram File (XY face)\n",
                                                              VERSION_NUMBER);
      write (fd, outStr, strlen(outStr));
      sprintf(outStr, "# x = %d, y = 1, z = %d\n", xmax, zmax);
      write (fd, outStr, strlen(outStr));
#else
      fprintf(out, "# Noddy %.2f - Block Diagram File (XY face)\n",
                                                              VERSION_NUMBER);
      fprintf(out, "# x = %d, y = 1, z = %d\n", xmax, zmax);
#endif
   }

   y = ymax - 1;
#ifdef _MPL
   for (z = 0; z < zmax; z++)
   {
      for (px = ixproc; px < xmax; px+=nxproc)
      {
         data[0] = px;
         data[1] = z+2;
         data[2] = blockData[z][px/nxproc][y/nyproc];
         if (iyproc == (y%nyproc))
            p_write (fd, data, 6);
      }
   }
   write(fd, newLine, 1);
#else
   for (z = 0; z < zmax; z++)
   {
      for (x = 0; x < xmax; x++)
      {
         value = blockData[z][x+1][y+1];
         fprintf(out, "%d\t", value);
      }
      fprintf(out,"\n");
   }
#endif
                                                /* ************* */
                                                /* Write YZ FACE */
   if (header)
   {
#ifdef _MPL
      sprintf(outStr, "# Noddy %.2f - BINARY Block Diagram File (XY face)\n",
                                                              VERSION_NUMBER);
      write (fd, outStr, strlen(outStr));
      sprintf(outStr, "# x = 1, y = %d, z = %d\n", ymax, zmax);
      write (fd, outStr, strlen(outStr));
#else
      fprintf(out, "# Noddy %.2f - Block Diagram File (XY face)\n",
                                                              VERSION_NUMBER);
      fprintf(out, "# x = 1, y = %d, z = %d\n", ymax, zmax);
#endif
   }

   x = 0;
#ifdef _MPL
   for (z = 0; z < zmax; z++)
   {
      for (py = iyproc; py < ymax; py+=nyproc)
      {
         data[0] = ymax - py;
         data[1] = z+2;
         data[2] = blockData[z][x][py/nyproc];
         if (ixproc == x)
            p_write (fd, data, 6);
      }
   }
   write(fd, newLine, 1);
   close(fd);
#else
   for (z = 0; z < zmax; z++)
   {
      for (y = ymax-1; y >= 0; y--)
      {
         value = blockData[z][x+1][y+1];
         fprintf(out, "%d\t", value);
      }
      fprintf(out,"\n");
   }
   fclose(out);
#endif

#ifdef _MPL
   printf(" Done."); fflush (stdout);
#endif

   return (TRUE);
}


/* ======================================================================
FUNCTION        writeGeologyBlockFile
DESCRIPTION
        write out the blockData to a simple file
INPUT int ***blockData - the data to write out
        int xmax, ymax, zmax - dimensions of the data
        int header - flag TRUE if the output file needs a header

RETURNED
   TRUE on sucess, FALSE on error
====================================================================== */
int writeGeologyBlockFile(blockData, blockName, xmax, ymax, zmax, header)
char *blockName;
#ifdef _MPL
plural int ***blockData;
#else
int ***blockData;
#endif
int xmax, ymax, zmax;
int header;
{
   FILE *out;
   register int x, y, z, value;
   char outputFile[150], *ptr;
#ifdef _MPL
   int fd;
   char outStr[150];
   plural unsigned short data[3];
   plural int px, py, pz;
   char newLine[2];
#endif

#ifdef _MPL
   printf("\nWriting Geology Block File."); fflush (stdout);
   strcpy(newLine,"\n");
#endif

   if (!blockName)
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for Geology block output - No Name");
      else
         //xvt_dm_post_error("Can't open file for Geology block output - No Name");
      return (FALSE);
   }
                        /* make sure the output file has a .blk extention */
   strcpy (outputFile, blockName);
   if (ptr = (char *) strchr (outputFile, '.'))
      strcpy (ptr, ".geo");
   else
      strcat (outputFile, ".geo");

#ifdef _MPL
   if (!(fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRWXG)))
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for geology block output");
      else
         //xvt_dm_post_error("Can't open file for geology block output");
      return (FALSE);
   }
#else
   if (!(out = (FILE *) fopen(outputFile, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for geology block output");
      else
         //xvt_dm_post_error("Can't open file for geology block output");
      return (FALSE);
   }
#endif

                                                /* ***************** */
                                                /* Write File Header */
   if (header)
   {
#ifdef _MPL
      sprintf(outStr, "# Noddy %.2f - BINARY Geology Block File\n",VERSION_NUMBER);
      write (fd, outStr, strlen(outStr));
      sprintf(outStr, "# x = %d, y = %d, z = %d\n", xmax, ymax, zmax);
      write (fd, outStr, strlen(outStr));
#else
      fprintf(out, "# Noddy %.2f - Geology Block File\n",VERSION_NUMBER);
      fprintf(out, "# x = %d, y = %d, z = %d\n", xmax, ymax, zmax);
#endif
   }

                                                /* **************** */
                                                /* Write Block Data */
#ifdef _MPL
   for (z = 0; z < zmax; z++)
   {
      for (py = iyproc; py < ymax; py+=nyproc)
      {
         for (px = ixproc; px < xmax; px+=nxproc)
         {
            data[0] = py;
            data[1] = px;
            data[2] = blockData[z][px/nxproc][py/nyproc];
            p_write (fd, data, 6);
         }
      }
   }
   write(fd, newLine, 1);
   close(fd);
#else
   for (z = 0; z < zmax; z++)
   {
      for (y = 0; y < ymax; y++)
      {
         for (x = 0; x < xmax; x++)
         {
            value = blockData[z][x+1][y+1];
            fprintf(out, "%d\t", value);
         }
         fprintf(out,"\n");
      }
   }
   fclose(out);
#endif

#ifdef _MPL
   printf(" Done."); fflush (stdout);
#endif

   return (TRUE);
}


/* ======================================================================
FUNCTION        readGeoBlockHeader
DESCRIPTION
        extract the information from the header of a geology block
        file
INPUT FILE *fi;      - the file to read header from

OUTPUT   int *nx, *ny, *nz;   - dimensions of data in file
   double *version;  - the version number of the file
RETURNED
   TRUE if it is a valid file
   FALSE on error
====================================================================== */
int 
#if XVT_CC_PROTO
readGeoBlockHeader (FILE *fi, int *nx, int *ny, int *nz,
                    double *version, char *comment)
#else
readGeoBlockHeader (fi, nx, ny, nz, version, comment)
FILE *fi;
int *nx, *ny, *nz;
double *version;
char *comment;
#endif
{
   char line[100], word[20];
   char *ptr;

   if (!fgets (line, 100, fi))        /* get first line */
      return (FALSE);

                           /* check to make sure this is the right file */
   sscanf(line, "# %s %lf", word, version);
   if (strcmp (word, "Noddy") != 0)
      return (FALSE);

   if (ptr = (char *) strchr (line, '-'))   /* pick the comment up after the '-' */
      strcpy (comment, ptr+2);     /* comments are ' - COMMENT' */
   else
      strcpy (comment, "");
   if (ptr = (char *) strchr (comment, '\n'))  /* get rid of CR in the comment */
      *ptr = '\0'; 

   if ((*version > VERSION_NUMBER+VERSION_TOLERANCE) || (*version < 1.0))
      return (FALSE);

   if (!fgets (line, 50, fi))        /* get second line */
      return (FALSE);
                            /* read in the size of the file */
   if (sscanf(line, "# x = %d, y = %d, z = %d", nx, ny, nz) != 3)
      return (FALSE);

   return (TRUE);
}

int writeDicerBlockFile(blockData, blockName, xmax, ymax, zmax, header)
char *blockName;
#ifdef _MPL
plural int ***blockData;
#else
int ***blockData;
#endif
int xmax, ymax, zmax;
int header;
{
   FILE *out;
   register int x, y, z, value;
   char outputFile[150], *ptr;

#ifdef _MPL
   printf("\nWriting Dicer Block File."); fflush (stdout);
#endif

   if (!blockName)
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for dicer block output - No Name");
      else
         //xvt_dm_post_error("Can't open file for dicer block output - No Name");
      return (FALSE);
   }
                        /* make sure the output file has a .blk extention */
   strcpy (outputFile, blockName);
   if (ptr = (char *) strchr (outputFile, '.'))
      strcpy (ptr, ".dic");
   else
      strcat (outputFile, ".dic");

   if (!(out = (FILE *) fopen(outputFile, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Can't open file for dicer block output");
      else
         //xvt_dm_post_error("Can't open file for dicer block output");
      return (FALSE);
   }

                                                /* ***************** */
                                                /* Write File Header */
   if (header)
   {
      fprintf(out, "# Noddy %.2f - Dicer Block File\n",VERSION_NUMBER);
      fprintf(out, "# x = %d, y = %d, z = %d\n", xmax, ymax, zmax);
   }

                                                /* **************** */
                                                /* Write Block Data */
   for (z = zmax-1; z >= 0; z--)
   {
      for (y = 0; y < ymax; y++)
      {
         for (x = 0; x < xmax; x++)
         {
#ifdef _MPL
                           /* only the processor with this element needs
                              to write it to the file */
            value = proc[y%nyproc][x%nxproc].blockData[z][x/nxproc][y/nyproc];
            fprintf(out, "%d\t", value);
#else
            value = blockData[z][x+1][y+1];
            fprintf(out, "%d\t", value);
#endif
         }
         fprintf(out,"\n");
      }
   }
        
   fclose(out);

#ifdef _MPL
   printf(" Done."); fflush (stdout);
#endif

   return (TRUE);
}



/* ======================================================================
FUNCTION        writeBlockHeaderToFile
DESCRIPTION
        write a block Header to a file 
INPUT
   char *filename; - base name for file
   int numLayers; - z dimension of block
   int **layerDimensions; - the x, y dimensions of each layer
   double topX, topY, topZ; - geological coordinates of volume start
   double botX, botY, botZ; - geological coordinates of volume end
   int numCubeSizes, *cubeSizes - cube sizes for layers
   ing calcRange; - 
   double inclination; - inclination of earths mag field
   double intensity; - intensity of earth mag field
   double declination; - declination of volume wrt mag North
   int densityCalc, susCalc, remCalc, aniCalc;
                       - flags showing which file are generated

RETURNED
   TRUE if it is a valid file
   FALSE on error
====================================================================== */
int 
#if XVT_CC_PROTO
writeBlockHeaderToFile (char *filename, int numLayers, int **layerDimensions,
               double topX, double topY, double topZ,
               double botX, double botY, double botZ,
               int numCubeSizes, int *cubeSizes,
               int calcRange, double inclination, double intensity,
               double declination, int densityCalc, int susCalc,
               int remCalc, int aniCalc, int indexCalc, int numProps,
               LAYER_PROPERTIES **layerProps)
#else
writeBlockHeaderToFile (filename, numLayers, layerDimensions,
               topX, topY, topZ, botX, botY, botZ,
               numCubeSizes, cubeSizes,
               calcRange, inclination, intensity, declination,
               densityCalc, susCalc, remCalc, aniCalc, indexCalc,
               numProps, layerProps)
char *filename;
int numLayers, **layerDimensions;
double topX, topY, topZ;
double botX, botY, botZ;
int numCubeSizes, *cubeSizes, calcRange;
double inclination, intensity, declination;
int densityCalc, susCalc, remCalc, aniCalc;
int indexCalc, numProps;
LAYER_PROPERTIES **layerProps;
#endif
{
   FILE *fo;
   char filePrefix[50], dateString[50], timeString[50];
   char *charPtr, *densityPtr, *susPtr, *remPtr, *aniPtr, *indexedPtr;
   char yesString[4], noString[3];
   double minX, minY, minZ, maxX, maxY, maxZ;
   int i;
   
   strcpy (yesString, "Yes"); strcpy (noString, "No");
   
   addFileExtention (filename, ".g00");
   if (!(fo = (FILE *) fopen(filename, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Writing.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Writing.", filename);
      return (FALSE);
   }
         /* extract file prefix */
   strcpy (filePrefix, filename);
   if (charPtr = (char *) strchr (filePrefix, '.'))
      *charPtr = '\0';  /* terminate the string before extention */

         /* build date string */
   strcpy (dateString, "01/01/90");
   
         /* build time string */
   strcpy (timeString, "12:00:00");

         /* Assign files that are calculated */
   if (densityCalc) densityPtr = yesString;
   else densityPtr = noString;

   if (susCalc) susPtr = yesString;
   else susPtr = noString;

   if (remCalc) remPtr = yesString;
   else remPtr = noString;

   if (aniCalc) aniPtr = yesString;
   else aniPtr = noString;

   if (indexCalc) indexedPtr = yesString;
   else indexedPtr = noString;

                      /* ************************** */
                      /* *** Write out the file *** */
   fprintf (fo, "VERSION = %.2lf\n", VERSION_NUMBER);
   fprintf (fo, "FILE PREFIX = %s\n", filePrefix);
   fprintf (fo, "DATE = %s\n", dateString);
   fprintf (fo, "TIME = %s\n", timeString);
   
   minX = MIN(topX, botX);  maxX = MAX(topX, botX);
   minY = MIN(topY, botY);  maxY = MAX(topY, botY);
   minZ = MIN(topZ, botZ);  maxZ = MAX(topZ, botZ);
   fprintf (fo, "UPPER SW CORNER (X Y Z) = %.1lf %.1lf %.1lf\n", minX, minY, maxZ);
   fprintf (fo, "LOWER NE CORNER (X Y Z) = %.1lf %.1lf %.1lf\n", maxX, maxY, minZ);
   fprintf (fo, "NUMBER OF LAYERS = %d\n", numLayers);
   for (i = 0; i < numLayers; i++)
   {
      fprintf (fo, "\tLAYER %d DIMENSIONS (X Y) = %d %d\n",
                   i+1, layerDimensions[i][0], layerDimensions[i][1]);
   }

   fprintf (fo, "NUMBER OF CUBE SIZES = %d\n", numCubeSizes);
   for (i = 0; i < numCubeSizes; i++)
   {
      fprintf (fo, "\tCUBE SIZE FOR LAYER %d = %d\n", i+1, cubeSizes[i]);
   }
   
   fprintf (fo, "CALCULATION RANGE = %d\n", calcRange);
   fprintf (fo, "INCLINATION OF EARTH MAG FIELD = %.2lf\n", inclination);
   fprintf (fo, "INTENSITY OF EARTH MAG FIELD = %.2lf\n", intensity);
   fprintf (fo, "DECLINATION OF VOL. WRT. MAG NORTH = %.2lf\n", declination);
   
   fprintf (fo, "DENSITY CALCULATED = %s\n", densityPtr);
   fprintf (fo, "SUSCEPTIBILITY CALCULATED = %s\n", susPtr);
   fprintf (fo, "REMANENCE CALCULATED = %s\n", remPtr);
   fprintf (fo, "ANISOTROPY CALCULATED = %s\n", aniPtr);
   
   fprintf (fo, "INDEXED DATA FORMAT = %s\n", indexedPtr);
   if (indexCalc)
   {
      fprintf (fo, "NUM ROCK TYPES = %d\n", numProps);
      for (i = 0; i < numProps; i++)
      {
         fprintf (fo, "ROCK DEFINITION %s = %d\n", layerProps[i]->unitName,i+1);
         if (densityCalc)
            fprintf (fo, "\tDensity = %lf\n", layerProps[i]->density);

         if (susCalc)
            fprintf (fo, "\tSus = %lf\n", layerProps[i]->sus_X);

         if (remCalc)
         {
            fprintf (fo, "\tDeclination = %lf\n", layerProps[i]->angleWithNorth);
            fprintf (fo, "\tAzimuth = %lf\n", layerProps[i]->inclination);
            fprintf (fo, "\tStrength = %lf\n", layerProps[i]->strength);
         }

         if (aniCalc)
         {
            fprintf (fo, "\tDip = %lf\n", layerProps[i]->sus_dip);
            fprintf (fo, "\tDip Direction = %lf\n", layerProps[i]->sus_dipDirection);
            fprintf (fo, "\tPitch = %lf\n", layerProps[i]->sus_pitch);
            fprintf (fo, "\tAxis 1 = %lf\n", layerProps[i]->sus_X);
            fprintf (fo, "\tAxis 2 = %lf\n", layerProps[i]->sus_Y);
            fprintf (fo, "\tAxis 3 = %lf\n", layerProps[i]->sus_Z);
         }
      }

   }

   fclose (fo);
   return (TRUE);
}

/* ======================================================================
FUNCTION        write3DBlockToFile
DESCRIPTION
        write a block array to a file 
OUTPUT
   char *filename;      - the file to write to
   int nx, ny, nz;      - dimensions of the block
   char ***data;         - the array with the information
   int dataSize;        - size of the data in the array

RETURNED
   TRUE if it is a valid file
   FALSE on error
====================================================================== */
int 
#if XVT_CC_PROTO
write3DBlockToFile (char *filename, char ***data,
                    int nx, int ny, int nz, int dataSize)
#else
write3DBlockToFile (filename, data, nx, ny, nz, dataSize)
char *filename;
char ***data;
int nx, ny, nz, dataSize;
#endif
{
   FILE *fo;
   int z;

   if (!data || !dataSize)
      return (FALSE);

   if (!(fo = (FILE *) fopen(filename, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Writing.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Writing.", filename);
      return (FALSE);
   }

                 /* Write out the array */
   for (z = nz-1; z >= 0; z--)
   {
      write2DBlock (fo, data[z], nx, ny, dataSize);
   }
   
   fclose (fo);
   return (TRUE);
}

int 
#if XVT_CC_PROTO
write3DIregBlockToFile (char *filename, char ***data,
                        int numLayers, int **layerDimensions, int dataSize)
#else
write3DIregBlockToFile (filename, data, numLayers, layerDimensions, dataSize)
char *filename;
char ***data;
int numLayers, **layerDimensions;
int dataSize;
#endif
{
   FILE *fo;
   int z;
   
   if (!data || !numLayers || !layerDimensions || !dataSize)
      return (FALSE);
   
   if (!(fo = (FILE *) fopen(filename, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Writing.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Writing.", filename);
      return (FALSE);
   }

                 /* Write out the array */
   for (z = numLayers-1; z >= 0; z--)
   {
      write2DBlock (fo, data[z], layerDimensions[z][1],
                                 layerDimensions[z][0], dataSize);
      fprintf(fo, "\n");
   }
   
   fclose (fo);
   return (TRUE);
}

int 
#if XVT_CC_PROTO
write2DBlockToFile (char *filename, char **data,
                    int nx, int ny, int dataSize)
#else
write2DBlockToFile (filename, data, nx, ny, dataSize)
char *filename;
char **data;
int nx, ny, dataSize;
#endif
{
   FILE *fo;

   if (!data || !dataSize)
      return (FALSE);

   if (!(fo = (FILE *) fopen(filename, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Writing.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Writing.", filename);
      return (FALSE);
   }

   write2DBlock (fo, data, nx, ny, dataSize);

   fclose (fo);

   return (TRUE);
}

int 
#if XVT_CC_PROTO
write2DBlock (FILE *fo, char **data,
                    int nx, int ny, int dataSize)
#else
write2DBlock (fo, data, nx, ny, dataSize)
FILE *fo;
char **data;
int nx, ny, dataSize;
#endif
{
   int x, y, dataIncrement;
   char *dataPtr;
   
   if (!data || !dataSize)
      return (FALSE);
   
         /* the num of char to move for one element */
   dataIncrement = dataSize;
   
                 /* Write out the array */
   for (y = 0; y < ny; y++)
   {                  /* point to a single line of data */
      dataPtr = (char *) data[y];

      for (x = 0; x < nx; x++, dataPtr+=dataIncrement)
      {
         if (dataSize == sizeof(double))
            fprintf (fo, "%lf\t", *((double *) dataPtr));
         else if (dataSize == sizeof(float))
            fprintf (fo, "%f\t", *((float *) dataPtr));
         else if (dataSize == sizeof(int))
            fprintf (fo, "%d\t", *((int *) dataPtr));
         else if (dataSize == sizeof(short))
            fprintf (fo, "%d\t", *((short *) dataPtr));
         else if (dataSize == sizeof(char))
            fprintf (fo, "%d\t", *((char *) dataPtr));
      }
      fprintf (fo, "\n");  /* seperate with newlines */
   }
   
   return (TRUE);
}



int 
#if XVT_CC_PROTO
writeLineThroughBlock (char *filename, double **pointPos,
                       int numPoints)
#else
writeLineThroughBlock (filename, pointPos, numPoints)
char *filename;
double **pointPos;
int numPoints;
#endif
{
   FILE *fo;
   LAYER_PROPERTIES *rockProp;
   float remSusDec, remSusAzi, aniSusDip, aniSusDDir, aniSusPitch;
   int i, j, point, sucess = TRUE;
   double grid;

   if (!(fo = (FILE *) fopen(filename, "w")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Writing.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Writing.", filename);
      return (FALSE);
   }
   
   if (geologyOptions.useTopography)
      grid = (TopomapXE - TopomapXW)/TopoCol;

                        /* Write Out the file header */
   fprintf (fo, "Noddy Geological Point Description\n");
   fprintf (fo, "VERSION=%.2f\tLINES=%d\tANICALC=%d\tREMCAL=%d\n",
                                                   VERSION_NUMBER, numPoints, TRUE, TRUE);
   fprintf (fo, "X\tY\tZ\tDENSITY\tSUSX\tSUSY\tSUSZ\tDECL\tAZI\tSTR\tDIP\tDDIR\tPITCH\tNAME\tLINE\n");
                   

   for (point = 0; point < numPoints; point++)
   {
      if (geologyOptions.useTopography)
      {
         i = (int) floor (((pointPos[point][0]-TopomapXW)/grid) + 0.5);
         j = (int) floor (((pointPos[point][1]-TopomapYW)/grid) + 0.5);
         if ((i >= 0) && (j >= 0) && (i < TopoRow) && (j < TopoCol))
         {
            pointPos[point][2] = topographyMap[j+1][i+1];
         }          
      }
      
      if (!(rockProp = calcBlockPixel(pointPos[point][0], pointPos[point][1], pointPos[point][2],
               (int) geologyOptions.useTopography, (int) geophysicsOptions.deformableRemanence,
               &remSusDec, &remSusAzi, (int) geophysicsOptions.deformableAnisotropy,
               &aniSusDip, &aniSusDDir, &aniSusPitch)))
      {
         sucess = FALSE;
      }
      else
      {
         fprintf(fo, "%.1f\t%.1f\t%.1f\t",
                       pointPos[point][0], pointPos[point][1], pointPos[point][2]);
         fprintf(fo, "%.1f\t", rockProp->density);
         fprintf(fo, "%.1f\t%.1f\t%.1f\t",
                                rockProp->sus_X, rockProp->sus_Y, rockProp->sus_Z);
         if (geophysicsOptions.deformableRemanence)
            fprintf(fo, "%.1f\t%.1f\t%.1f\t",
                                         remSusDec, remSusAzi, rockProp->strength);
         else
            fprintf(fo, "%.1f\t%.1f\t%.1f\t", rockProp->angleWithNorth,
                                        rockProp->inclination, rockProp->strength);
         if (geophysicsOptions.deformableAnisotropy)
            fprintf(fo, "%.1f\t%.1f\t%.1f\t",  aniSusDip, aniSusDDir, aniSusPitch);
         else
            fprintf(fo, "%.1f\t%.1f\t%.1f\t", rockProp->sus_dip,
                                  rockProp->sus_dipDirection, rockProp->sus_pitch);
         fprintf(fo, "%s\t%d", rockProp->unitName, point+1);
         fprintf(fo, "\n");
      }
   }

   fclose (fo);
   
   return (sucess);
}


/* ======================================================================
FUNCTION        readBlockHeaderFromFile
DESCRIPTION
        read a block Header from a file 
INPUT
   char *filename; - base name for file
   int *numLayers; - z dimension of block
   int **layerDimensions; - the x, y dimensions of each layer
   double *topX, *topY, *topZ; - geological coordinates of volume start
   double *botX, *botY, *botZ; - geological coordinates of volume end
   int *numCubeSizes, *cubeSizes - cube sizes for layers
   int *calcRange; - 
   double *inclination; - inclination of earths mag field
   double *intensity; - intensity of earth mag field
   double *declination; - declination of volume wrt mag North
   int *densityCalc, *susCalc, *remCalc, *aniCalc;
                       - flags showing which file are generated

RETURNED
   TRUE if it is a valid file
   FALSE on error
====================================================================== */
int 
#if XVT_CC_PROTO
readBlockHeaderFromFile (char *filename, int *numLayers, int ***layerDimensions,
               double *topX, double *topY, double *topZ,
               double *botX, double *botY, double *botZ,
               int *numCubeSizes, int **cubeSizes,
               int *calcRange, double *inclination, double *intensity,
               double *declination, int *densityCalc, int *susCalc,
               int *remCalc, int *aniCalc, int *indexCalc,
               int *numProps, LAYER_PROPERTIES **layerProps)
#else
readBlockHeaderFromFile (filename, numLayers, layerDimensions,
               topX, topY, topZ, botX, botY, botZ,
               numCubeSizes, cubeSizes,
               calcRange, inclination, intensity, declination,
               densityCalc, susCalc, remCalc, aniCalc,
               indexCalc, numProps, layerProps)
char *filename;
int *numLayers, ***layerDimensions;
double *topX, *topY, *topZ;
double *botX, *botY, *botZ;
int *numCubeSizes, **cubeSizes, *calcRange;
double *inclination, *intensity, *declination;
int *densityCalc, *susCalc, *remCalc, *aniCalc;
int *indexCalc, *numProps;
LAYER_PROPERTIES **layerProps;
#endif
{
   FILE *fi;
   double version;
   char filePrefix[50], dateString[50], timeString[50];
   char values[50];
   LAYER_PROPERTIES *allocedProps = NULL;
   int i, layer;
   
   char yesString[4], noString[3];
   
   strcpy (yesString, "Yes"); strcpy (noString, "No");
   
   if (!(fi = (FILE *) fopen(filename, "r")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Writing.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Writing.", filename);
      return (FALSE);
   }

                      /* ************************ */
                      /* *** Read in the file *** */
   loadit(fi,"%lf",(void *) &version);
   loadit(fi,"%s",(void *) filePrefix);
   loadit(fi,"%s",(void *) dateString);
   loadit(fi,"%s",(void *) timeString);
   loadit(fi,"%s",(void *) values);
   sscanf(values, "%lf %lf %lf", topX, topY, topZ);
   loadit(fi,"%s",(void *) values);
   sscanf(values, "%lf %lf %lf", botX, botY, botZ);

   if (*layerDimensions)
      destroy2DArray((char **) *layerDimensions, *numLayers, 2);
   
   loadit(fi,"%d",(void *) numLayers);
   
   if (!(*layerDimensions = (int**) create2DArray (*numLayers, 2, sizeof(int))))
      return (FALSE);
   
   for (i = 0; i < *numLayers; i++)
   {
      loadit(fi,"%s",(void *) values);
      sscanf(values, "%d %d", &((*layerDimensions)[i][0]),
                              &((*layerDimensions)[i][1]));
   }

   if (*cubeSizes)
      destroy1DArray((char *) *cubeSizes);

   loadit(fi,"%d",(void *) numCubeSizes);

   if (!(*cubeSizes = (int *) create1DArray (*numCubeSizes, sizeof(int))))
      return (FALSE);
   
   for (i = 0; i < *numCubeSizes; i++)
   {
      loadit(fi,"%s",(void *) values);
      sscanf(values, "%d", &((*cubeSizes)[i]));
   }
   
   loadit(fi,"%d",(void *) calcRange);
   loadit(fi,"%lf",(void *) inclination);
   loadit(fi,"%lf",(void *) intensity);
   loadit(fi,"%lf",(void *) declination);
   
   loadit(fi,"%s",(void *) values);
   if (strcmp (values, "Yes") == 0)
      *densityCalc = TRUE;
   else
      *densityCalc = FALSE;
      
   loadit(fi,"%s",(void *) values);
   if (strcmp (values, "Yes") == 0)
      *susCalc = TRUE;
   else
      *susCalc = FALSE;

   loadit(fi,"%s",(void *) values);
   if (strcmp (values, "Yes") == 0)
      *remCalc = TRUE;
   else
      *remCalc = FALSE;

   loadit(fi,"%s",(void *) values);
   if (strcmp (values, "Yes") == 0)
      *aniCalc = TRUE;
   else
      *aniCalc = FALSE;

   loadit(fi,"%s",(void *) values);
   if (strcmp (values, "Yes") == 0)
      *indexCalc = TRUE;
   else
      *indexCalc = FALSE;

   if (*indexCalc)
   {
      loadit(fi,"%d",(void *) numProps);
      
      if (!(allocedProps = (LAYER_PROPERTIES *) create1DArray ((*numProps),
                                                  sizeof (LAYER_PROPERTIES))))
      {
         *indexCalc = FALSE;
         fclose (fi);
         return (FALSE);
      }
      else
      {
         layerProps[0] = allocedProps;
         for (i = 1; i < *numProps; i++)
            layerProps[i] = &(allocedProps[i]);
      }
         
      for (i = 0; i < *numProps; i++)
      {
         loadit(fi,"%d",(void *) &layer); layer--;
         
         if (*densityCalc)
            loadit(fi,"%lf",(void *) &(layerProps[layer]->density));

         if (*susCalc)
            loadit(fi,"%lf",(void *) &(layerProps[layer]->sus_X));

         if (*remCalc)
         {
            loadit(fi,"%lf",(void *) &(layerProps[layer]->angleWithNorth));
            loadit(fi,"%lf",(void *) &(layerProps[layer]->inclination));
            loadit(fi,"%lf",(void *) &(layerProps[layer]->strength));
            layerProps[layer]->remanentMagnetization = TRUE;
         }

         if (*aniCalc)
         {
            loadit(fi,"%lf",(void *) &(layerProps[layer]->sus_dip));
            loadit(fi,"%lf",(void *) &(layerProps[layer]->sus_dipDirection));
            loadit(fi,"%lf",(void *) &(layerProps[layer]->sus_pitch));
            loadit(fi,"%lf",(void *) &(layerProps[layer]->sus_X));
            loadit(fi,"%lf",(void *) &(layerProps[layer]->sus_Y));
            loadit(fi,"%lf",(void *) &(layerProps[layer]->sus_Z));
            layerProps[layer]->anisotropicField = TRUE;
         }
      }
   }
   
   fclose (fi);
   return (TRUE);
}

/* ======================================================================
FUNCTION        read3DBlockFromFile
DESCRIPTION
        write a block array to a file 
OUTPUT
   char *filename;      - the file to write to
   int nx, ny, nz;      - dimensions of the block
   char ***data;         - the array with the information
   int dataSize;        - size of the data in the array

RETURNED
   TRUE if it is a valid file
   FALSE on error
====================================================================== */
int 
#if XVT_CC_PROTO
read3DBlockFromFile (char *filename, char ***data,
                     int nx, int ny, int nz, int dataSize)
#else
read3DBlockFromFile (filename, data, nx, ny, nz, dataSize)
char *filename;
char ***data;
int nx, ny, nz, dataSize;
#endif
{
   FILE *fi;
   int z;

   if (!data || !dataSize)
      return (FALSE);
  
   if (!(fi = (FILE *) fopen(filename, "r")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Reading.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Reading.", filename);
      return (FALSE);
   }

                 /* Read in the array */
   for (z = nz-1; z >= 0; z--)
   {
      read2DBlock (fi, data[z], nx, ny, dataSize);
   }
   
   fclose (fi);
   return (TRUE);
}

int 
#if XVT_CC_PROTO
read3DIregBlockFromFile (char *filename, char ***data,
                         int numLayers, int **layerDimensions, int dataSize)
#else
read3DIregBlockFromFile (filename, data, numLayers, layerDimensions, dataSize)
char *filename;
char ***data;
int numLayers, **layerDimensions;
int dataSize;
#endif
{
   FILE *fi;
   int z;
   
   if (!data || !numLayers || !layerDimensions || !dataSize)
      return (FALSE);

   if (!(fi = (FILE *) fopen(filename, "r")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Reading.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Reading.", filename);
      return (FALSE);
   }

                 /* Read in the array */
   for (z = numLayers-1; z >= 0; z--)
   {
      read2DBlock (fi, data[z], layerDimensions[z][1],
                                layerDimensions[z][0], dataSize);
   }
   
   fclose (fi);
   return (TRUE);
}

int 
#if XVT_CC_PROTO
read2DBlockFromFile (char *filename, char **data,
                     int nx, int ny, int dataSize)
#else
read2DBlockFromFile (filename, data, nx, ny, dataSize)
char *filename;
char **data;
int nx, ny, dataSize;
#endif
{
   FILE *fi;

   if (!data || !dataSize)
      return (FALSE);

   if (!(fi = (FILE *) fopen(filename, "r")))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Cannot Open %s for Reading.", filename);
      else
         //xvt_dm_post_error("Error, Cannot Open %s for Reading.", filename);
      return (FALSE);
   }

                 /* Read in the array */
   read2DBlock (fi, data, nx, ny, dataSize);
   
   fclose (fi);
   return (TRUE);
}

int 
#if XVT_CC_PROTO
read2DBlock (FILE *fi, char **data,
             int nx, int ny, int dataSize)
#else
read2DBlock (fi, data, nx, ny, dataSize)
FILE *fi;
char **data;
int nx, ny, dataSize;
#endif
{
   int x, y, dataIncrement;
   char *dataPtr;
   short *shortPtr;
   unsigned char *charPtr;
   int value;

   if (!data || !dataSize)
      return (FALSE);

         /* the num of char to move for one element */
   dataIncrement = dataSize;
   
                 /* Write out the array */
   for (y = 0; y < ny; y++)
   {                  /* point to a single line of data */
      dataPtr = (char *) data[y];

      for (x = 0; x < nx; x++, dataPtr+=dataIncrement)
      {
         if (dataSize == sizeof(double))
            fscanf (fi, "%lf", (double *) dataPtr);
         else if (dataSize == sizeof(float))
            fscanf (fi, "%f", (float *) dataPtr);
         else if (dataSize == sizeof(int))
            fscanf (fi, "%d", (int *) dataPtr);
         else if (dataSize == sizeof(short))
         {
            fscanf (fi, "%d", &value);
            shortPtr = (short *) dataPtr;
            *shortPtr = (short) value;
         }
         else if (dataSize == sizeof(char))
         {
            fscanf (fi, "%d", &value);
            charPtr = (unsigned char *) dataPtr;
            *charPtr = (unsigned char) value;
         }
      }
   }
   
   return (TRUE);
}
 

int
#if XVT_CC_PROTO
convertOldBlkToNewBlk (FILE_SPEC *oldBlock, FILE_SPEC *noddyBlock,
                       int flipX, int flipY, int flipZ)
#else
convertOldBlkToNewBlk (oldBlock, noddyBlock, flipX, flipY, flipZ)
FILE_SPEC *oldBlock, *noddyBlock;
int flipX, flipY, flipZ;
#endif
{
   GEOPHYSICS_OPTIONS options;
   int nx, ny, nz, x, y, z, xVal, yVal, zVal;
   double startX, startY, startZ;
   double endX, endY, endZ;
   int ***intBlockData = NULL;
   short ***shortBlockData = NULL;
   int numRock, i;
   float rho[121],sus[121][4];
   double ken[121][4],ang[121],plu[121];
   int num[121];
   int calcRem, calcAni, cubeSize;
   int **layerDimensions, *cubeSizes;
   LAYER_PROPERTIES *rockProps[50], *allocedProps;
   BLOCK_VIEW_OPTIONS viewOptions;
   int calcRangeInCubes;

                                       /* ******************************** */
                                       /* First read in the old Block File */
   xvt_fsys_set_dir (&(oldBlock->dir));
   if (!readBlockHeader (oldBlock->name, &nx, &ny, &nz, &cubeSize,
                    &calcRangeInCubes,
                    &startX, &startY, &startZ, &endX, &endY, &endZ,
                    &options.inclination, &options.intensity, &options.altitude))
   {
      if (batchExecution)
         fprintf (stderr,"Error, Could not read header of block File.");
      else
         //xvt_dm_post_error ("Error, Could not read header of block File.");
      return (FALSE);
   }
   options.calculationRange = calcRangeInCubes*cubeSize;
   viewOptions.geologyCubeSize = viewOptions.geophysicsCubeSize = (double) cubeSize;
   
   initLongJob (0, nz + 3, "Converting Old Block...", NULL);
      
                                       /* ********************************* */
                                       /* Allocate memory needed to hold it */
   if (!(intBlockData = (int ***) create3DArray (nz, nx, ny, sizeof(int))))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to read in block.");
      else
         //xvt_dm_post_error ("Error, Not enough memory to read in block.");
      finishLongJob ();
      return (FALSE);
   }
   if (!(shortBlockData = (short ***) create3DArray (nz, nx, ny, sizeof(int))))
   {
      destroy3DArray ((char ***) intBlockData, nz, nx, ny);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to read in block.");
      else
         //xvt_dm_post_error ("Error, Not enough memory to read in block.");
      finishLongJob ();
      return (FALSE);
   }


                                       /* ******************************** */
                                       /* First read in the old Block File */
   if (!readBlockFile (oldBlock->name, intBlockData, &numRock, nx, ny, nz,
                   rho, sus, ken, ang, plu, num, &calcRem, &calcAni,
                   (double *) NULL, (double *) NULL, (double *) NULL,
                   (double *) NULL, (double *) NULL, (double *) NULL,
                   &viewOptions, &options))
   {
      destroy3DArray ((char ***) intBlockData, nz, nx, ny);
      destroy3DArray ((char ***) shortBlockData, nz, nx, ny);
      if (batchExecution)
         fprintf (stderr, "Error, Could not read in Block File.");
      else
         //xvt_dm_post_error ("Error, Could not read in Block File.");
      finishLongJob ();
      return (FALSE);
   }

   incrementLongJob (INCREMENT_JOB);
      
   if (options.altitude > 0.1)
      options.calculationAltitude = AIRBORNE;
   else
      options.calculationAltitude = SURFACE;
   
                           /* ************************************************* */
                           /* Allocate some arrays needed to describe new block */
   layerDimensions = (int **) create2DArray(nz, 2, sizeof(int));
   cubeSizes = (int *) create1DArray(nz, sizeof(int));
   allocedProps = (LAYER_PROPERTIES *) create1DArray (numRock,
                                            sizeof (LAYER_PROPERTIES));
      
   for (i = 0; i < numRock; i++)
      rockProps[i] = &(allocedProps[i]);
   for (i = 0; i < nz; i++)
   {
      layerDimensions[i][0] = nx;
      layerDimensions[i][1] = ny;
      cubeSizes[i] = (int) viewOptions.geophysicsCubeSize;
   }
                           /* ***************************** */
                           /* Convert the block information */
   for (z = 0; z < nz; z++)
   {
      incrementLongJob (INCREMENT_JOB);
      if (flipZ)
         zVal = z;
      else
         zVal = nz-z-1;
         
      for (y = 0; y < ny; y++)
      {  
         if (flipY)
            yVal = ny - y - 1;
         else
            yVal = y;
            
         for (x = 0; x < nx; x++)
         {
            if (flipX)
               xVal = nx - x - 1;
            else
               xVal = x;
               
            shortBlockData[zVal][xVal][yVal] = (short) intBlockData[z][x][y];
         }
      }
   }
   
   for (i = 0; i < numRock; i++)
   {
      allocedProps[i].density = rho[i+1];
      allocedProps[i].angleWithNorth = ang[i+1];
      allocedProps[i].inclination = plu[i+1];
      allocedProps[i].anisotropicField = (int) sus[i+1][0];
      allocedProps[i].sus_X = sus[i+1][1];
      allocedProps[i].sus_Y = sus[i+1][2];
      allocedProps[i].sus_Z = sus[i+1][3];
      allocedProps[i].strength = ken[i+1][1];
      if (ken[i+1][1] > 0.01)
         allocedProps[i].remanentMagnetization = TRUE;
   }

                           /* *********************************** */
                           /* Write Out the New Block File format */
   xvt_fsys_set_dir (&(noddyBlock->dir));                        
   addFileExtention (noddyBlock->name, ANOM_HEADER_FILE_EXT);
   writeBlockHeaderToFile (noddyBlock->name, nz, layerDimensions,
                     startX, startY, endZ, endX, endY, startZ, nz,  cubeSizes,
                     (int) floor(options.calculationRange/cubeSize),
                     options.inclination, options.intensity, 0.0, TRUE,
                     TRUE, FALSE, FALSE, TRUE, numRock, rockProps);

   incrementLongJob (INCREMENT_JOB);

   addFileExtention (noddyBlock->name, ANOM_INDEX_FILE_EXT);
   write3DIregBlockToFile (noddyBlock->name, (char ***) shortBlockData,
                           nz, layerDimensions, sizeof(short));
   
                                /* Free memory we have alloced */
   destroy3DArray((char ***) intBlockData, nz, nx, ny);
   destroy3DArray((char ***) shortBlockData, nz, nx, ny);
   destroy2DArray((char **) layerDimensions, nz, 2);
   if (cubeSizes) destroy1DArray((char *) cubeSizes);
   if (allocedProps) destroy1DArray((char *) allocedProps);

   finishLongJob ();
   return (TRUE);
}

int
#if XVT_CC_PROTO
extractXYZFromFile (FILE_SPEC *xyzFile, int xColumn, int yColumn, int zColumn,
                    double xOffset, double yOffset, double zOffset,
                    double ***pointPos, int *numPoints)
#else           
extractXYZFromFile (xyzFile, xColumn, yColumn, zColumn, xOffset, yOffset, zOffset,
                    pointPos, numPoints)
FILE_SPEC *xyzFile;
int xColumn, yColumn, zColumn;
double xOffset, yOffset, zOffset;
double ***pointPos;
int *numPoints;
#endif
{
   FILE *fi;
   char lineIn[200], word[50];
   int xyzLineFound = FALSE, point;
   int wordCount, inWord, pos, lineLength;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   xvt_fsys_set_dir (&(xyzFile->dir));
   if (!(fi = fopen (xyzFile->name, "r")))
      return (FALSE);
      
                                          /* skip header */
   while (fgets(lineIn, 200, fi) && !xyzLineFound)
   {     /* Use the first line with a number at the start as a sample */
      sscanf(lineIn, "%s", word);
      if (isdigit(word[0]))
      {
         xyzLineFound = TRUE;
         break;
      }
   }
   *numPoints = 0;
   do      /* count the num of points */
   {
      (*numPoints)++;
   } while (fgets(lineIn, 200, fi));
                                        
                                        /* Allocate enough memory to store points */
   if (!(*pointPos = (double **) create2DArray (*numPoints, 3, sizeof(double))))
   {
      fclose (fi);
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory to import XYZ Line");
      else
         //xvt_dm_post_error ("Error, Not enough memory to import XYZ Line");
      return (FALSE);
   }
   rewind (fi);
                       /* skip header */
   xyzLineFound = FALSE;
   while (fgets(lineIn, 200, fi) && !xyzLineFound)
   {     /* Use the first line with a number at the start as a sample */
      sscanf(lineIn, "%s", word);
      if (isdigit(word[0]))
      {
         xyzLineFound = TRUE;
         break;
      }
   }
   point = 0;   
   do      /* count the num of points */
   {
      lineLength = strlen(lineIn) - 1;
      inWord = FALSE; wordCount = 0;
      for (pos = 0; pos < lineLength; pos++)
      {
         if (!isspace(lineIn[pos]))
         {
            if (!inWord)  /* Count the beginiing of each new word */
            {
               wordCount++;
               inWord = TRUE;
               if (wordCount == xColumn)  /* extract values if right columns */
                  sscanf (&(lineIn[pos]), "%lf", &((*pointPos)[point][0]));
               if (wordCount == yColumn)
                  sscanf (&(lineIn[pos]), "%lf", &((*pointPos)[point][1]));
               if (wordCount == zColumn)
                  sscanf (&(lineIn[pos]), "%lf", &((*pointPos)[point][2]));
               
            }
         }
         else
            inWord = FALSE;
      }
      (*pointPos)[point][0] += xOffset;
      (*pointPos)[point][1] += yOffset;
      if (zColumn)  /* Add offset if we have a column with values */
         (*pointPos)[point][2] += zOffset;
      else  /* No values so set to block top */
         (*pointPos)[point][2] = viewOptions->originZ;
      point++;
   } while (fgets(lineIn, 200, fi));

   fclose (fi);

   return (TRUE);
}

int
#if XVT_CC_PROTO
extractXYZHeader (FILE_SPEC *xyzFile, int *numParams)
#else           
extractXYZHeader (xyzFile, numParams)
FILE_SPEC *xyzFile;
int *numParams;
#endif
{
   FILE *fi;
   char lineIn[200], word[50];
   int foundSample = FALSE;
   int wordCount = 0, inWord, pos, lineLength;
   
   xvt_fsys_set_dir (&(xyzFile->dir));
   if (!(fi = fopen (xyzFile->name, "r")))
      return (FALSE);
      
   while (fgets(lineIn, 200, fi) && !foundSample)
   {     /* Use the first line with a number at the start as a sample */
      sscanf(lineIn, "%s", word);
      if (isdigit(word[0]))
         foundSample = TRUE;
   }
   fclose (fi);
   
   if (foundSample)  /* Count the columns */
   {
      lineLength = strlen(lineIn) - 1;
      inWord = FALSE;
      for (pos = 0; pos < lineLength; pos++)
      {
         if (!isspace(lineIn[pos]))
         {
            if (!inWord)  /* Count the beginiing of each new word */
            {
               wordCount++;
               inWord = TRUE;
            }
         }
         else
            inWord = FALSE;
      }
      *numParams = wordCount;
      return (TRUE);
   }
   else
      return (FALSE);
}


/* *****************************************************************
** *******************                        **********************
** ******************* Old Block Calculations **********************
** *******************                        **********************
** ************************************************************** */

int readBlockHeader (blockName, xmax, ymax, zmax, cubeSize, calcRange,
                                gx1, gy1, gz1, gx2, gy2, gz2,
                                inclination, intensity, altitude)
char *blockName;
int *xmax, *ymax, *zmax;
int *cubeSize, *calcRange;
double *gx1, *gy1, *gz1;
double *gx2, *gy2, *gz2;
double *inclination, *intensity, *altitude;
{
   FILE *in;
   int layers, test;
   float fdud;

   if (!(in = (FILE *) fopen (blockName, "r")))
      return (FALSE);

   fscanf(in,"%d",&test);

   if (test == 7777)
   {
       fclose(in);
       return (FALSE);
   }
   else if(test != 7771)
   {
      fclose(in);
      return (FALSE);
   }

   fscanf(in,"%lf %f %lf %lf",inclination, &fdud, intensity, altitude);
   fscanf(in,"%d %d %d %d",xmax, ymax, zmax, &layers);

   fscanf(in,"%f",&fdud); *cubeSize = (int) fdud;
   fscanf(in,"%d",calcRange);

   fscanf(in,"%lf %lf %lf", gx1, gy1, gz1);
   fscanf(in,"%lf %lf %lf", gx2, gy2, gz2);

   fclose (in); 
   
   return (TRUE);
}

int readBlockFile (blockName, blockData, nlitho, xmax, ymax, zmax,
                   rho, sus, ken, ang, plu, num, piRem, piAni,
                   lowerSouthWest_X, lowerSouthWest_Y, lowerSouthWest_Z,
                   upperNorthEast_X, upperNorthEast_Y, upperNorthEast_Z,
                   viewOptions, options)
char *blockName;
#ifdef _MPL
plural int ***blockData;
#else
int ***blockData;
#endif
int *nlitho;
int xmax, ymax, zmax;
float rho[121],sus[121][4];
double ken[121][4],ang[121],plu[121];
int num[121];
int *piRem,*piAni;
double *lowerSouthWest_X, *lowerSouthWest_Y, *lowerSouthWest_Z;
double *upperNorthEast_X, *upperNorthEast_Y, *upperNorthEast_Z;
BLOCK_VIEW_OPTIONS *viewOptions;
GEOPHYSICS_OPTIONS *options;
{
   FILE *in;
   int layers, test, iinum, blockXmax, blockYmax, blockZmax;
   float drho, dsus, dsus2, dsus3, dsus0,
         dken, dken2, dken3, dplu, dang;
   register int i, x, y, z;
   float fdud;
   double lswX, lswY, lswZ, uneX, uneY, uneZ;
   int calcRangeInCubes;

#ifdef _MPL
   printf("\nReading Block File."); fflush (stdout);
#endif
   if (!(in = (FILE *) fopen (blockName, "r")))
      return (FALSE);

   fscanf(in,"%d",&test);

   if (test == 7777)
   {
      fclose(in);
      if (batchExecution)
         fprintf (stderr, "Sorry, This is an old Block Model File");
      else
          //xvt_dm_post_error("Sorry, This is an old Block Model File");
      return (FALSE);
   }
   else if(test != 7771)
   {
      fclose(in);
      if (batchExecution)
         fprintf (stderr, "This is not a Block Model File");
      else
         //xvt_dm_post_error("This is not a Block Model File");
      return (FALSE);
   }

   *piRem=0;
   *piAni=0;

   fscanf(in,"%lf %f %lf %lf",&options->inclination, &fdud,
                            &options->intensity,
                            &options->altitude);
   fscanf(in,"%d %d %d %d",&blockXmax,&blockYmax,&blockZmax,&layers);
   if ((blockXmax != xmax) || (blockYmax != ymax) || (blockZmax != zmax))
   {
      fclose (in);
      if (batchExecution)
         fprintf (stderr, "Error, Block file is wrong size.");
      else
         //xvt_dm_post_error("Error, Block file is wrong size.");
      return (FALSE);
   }

   fscanf(in,"%f",&fdud);
   viewOptions->geologyCubeSize = viewOptions->geophysicsCubeSize = (double) fdud;
   fscanf(in,"%d",&calcRangeInCubes);
   options->calculationRange = (int) (calcRangeInCubes*viewOptions->geophysicsCubeSize);
   
   fscanf(in,"%lf %lf %lf", &lswX, &lswY, &lswZ);
   fscanf(in,"%lf %lf %lf", &uneX, &uneY, &uneZ);

   if (lowerSouthWest_X) *lowerSouthWest_X = lswX;
   if (lowerSouthWest_Y) *lowerSouthWest_Y = lswY;
   if (lowerSouthWest_Z) *lowerSouthWest_Z = lswZ;
   
   if (upperNorthEast_X) *upperNorthEast_X = uneX;
   if (upperNorthEast_Y) *upperNorthEast_Y = uneY;
   if (upperNorthEast_Z) *upperNorthEast_Z = uneZ;
   
   if (!lowerSouthWest_X || !lowerSouthWest_Y || !lowerSouthWest_Z
    || !upperNorthEast_X || !upperNorthEast_Y || !upperNorthEast_Z)
   {
      if (viewOptions)
      {
         viewOptions->originX = lswX;
         viewOptions->originY = lswY;
         viewOptions->originZ = lswZ + (uneZ - lswZ);
         viewOptions->lengthX = (uneX - lswX);
         viewOptions->lengthY = (uneY - lswY);
         viewOptions->lengthZ = (uneZ - lswZ);
      }
   }

   *nlitho = layers;

   for (i = 0; i <= layers; i++)
   {
      fscanf(in,"%d %f %f %f %f %f %f %f %f %f %f",
                      &iinum,&drho,&dsus,&dsus2,&dsus3,&dsus0,
                      &dken,&dken2,&dken3,&dplu,&dang);
      num[i]=iinum;
      rho[iinum] = 1000*drho;
      sus[iinum][1] = dsus;
      sus[iinum][2] = dsus2;
      sus[iinum][3] = dsus3;
      sus[iinum][0] = dsus0;
      ken[iinum][1] = dken;
      ken[iinum][2] = dken2;
      ken[iinum][3] = dken3;
      plu[iinum] = dplu;
      ang[iinum] = dang;

      if (dken != 0.0)
         *piRem = 1;
      if (dsus0 != 0.0)
         *piAni = 1;
   }

   for (z = 0; z < zmax; z++)
   {
      for (y = 0; y < ymax; y++)
      {
         for (x = 0; x < xmax; x++)
         { 
#ifdef _MPL
                           /* only the processor with this element needs
                              to read it into its array */
            fscanf (in, "%d", &value);
            proc[y%nyproc][x%nxproc].blockData[z][x/nxproc][y/nyproc] = value;
#else
            fscanf (in, "%d", &(blockData[z][x][y]));
#endif
         }
      }
   }

   fclose (in);
#ifdef _MPL
   printf(" Done."); fflush (stdout);
#endif
   return (TRUE);
}

void calcBlockModel(blockData, dots, histoire, maptopo, dx, dy,
                   nlitho, xmax, ymax, zmax, rho, sus, ken, ang, plu, num,
                   piRem, piAni, blockModelType)
int ***blockData;
double ***dots;
STORY **histoire;
double **maptopo;
double dx, dy;
int *nlitho;
int xmax, ymax, zmax;
float rho[121],sus[121][4];
double ken[121][4],ang[121],plu[121];
int num[121];
int *piRem,*piAni;
char *blockModelType;
{
   WINDOW eventWindow = (WINDOW) getEventDrawingWindow();
   WINDOW_INFO *wip;
   STRATIGRAPHY_OPTIONS *stratOptions;
   IMPORT_OPTIONS *importOptions;
   LAYER_PROPERTIES *propOptions;
   OBJECT *p;
   int numEvents = (int) countObjects(NULL_WIN);
   int gblock, msize;
   double Inclination = geophysicsOptions.inclination;
   double Intensity = geophysicsOptions.intensity;
   double altitude = geophysicsOptions.altitude;
   int sum2, layer, index, ignlut[65];
   double height, start, cubeScale;
   double blockOffsetX, blockOffsetY;
   enum {   ANOMALIES_MAP_BLOCK, DICER_BLOCK,   PROFILE_BLOCK,
      LINE_MAP_BLOCK,      SECTION_BLOCK, BLOCK_BLOCK
        } blockType;
   register int xIndex, yIndex;
   unsigned int flavor;
   int pindex, lutindex, iStrat, iLayer;
   int eventNumForStrat[50];
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   gblock = (int) viewOptions->geophysicsCubeSize;
   cubeScale = viewOptions->geologyCubeSize;
   msize = (int) floor(geophysicsOptions.calculationRange/gblock);

                  /* different calculation types have slightly
                  ** different initialisation for the block array */
   if (blockModelType && (strcmp (blockModelType, "DICER") == 0))
   {
      blockOffsetX = viewOptions->originX;
      blockOffsetY = viewOptions->originY;
      blockType = DICER_BLOCK;
                                      /* height to start writing from */
      start = viewOptions->originZ;
   }
   else if (blockModelType && (strcmp (blockModelType, "PROFILE") == 0))
   {
      blockOffsetX = -dx*msize*cubeScale;
      blockOffsetY = -dy*msize*cubeScale;
      blockType = PROFILE_BLOCK;
                                      /* height to start writing from */
      start = viewOptions->originZ;
   }
   else if (blockModelType && (strcmp (blockModelType, "SECTION") == 0))
   {
      gblock = (int) ((BLOCK_DIAGRAM_SIZE_X / xmax) * cubeScale);
      blockOffsetX = (BLOCK_DIAGRAM_SIZE_X / xmax) * cubeScale;
      blockOffsetY = viewOptions->originY;
      blockType = SECTION_BLOCK;
                                      /* height to start writing from */
      start = viewOptions->originZ - 2*gblock;
   }
   else if (blockModelType && (strcmp (blockModelType, "LINE_MAP") == 0))
   {
      gblock = (int) ((BLOCK_DIAGRAM_SIZE_X / xmax) * cubeScale);
      blockOffsetX = viewOptions->originX;
      blockOffsetY = viewOptions->originY;
      blockType = LINE_MAP_BLOCK;
                                      /* height to start writing from */
      start = viewOptions->originZ;
   }
   else if (blockModelType && (strcmp (blockModelType, "BLOCK") == 0))
   {
      gblock = (int) ((BLOCK_DIAGRAM_SIZE_X / xmax) * cubeScale);
      blockOffsetX =  viewOptions->originX;
      blockOffsetY = viewOptions->originY;
      blockType = BLOCK_BLOCK;
                                      /* height to start writing from */
      start = viewOptions->originZ;
   }
   else
   {
      blockOffsetX = viewOptions->originX-(gblock*msize);
      blockOffsetY = viewOptions->originY-(gblock*msize);
                                      /* height to start writing from */
      start = viewOptions->originZ;
      blockType = ANOMALIES_MAP_BLOCK;
   }

            /* assume that there is not remance etc unless we find some */
   *piRem = FALSE;
   *piAni = FALSE;

   wip = (WINDOW_INFO *) get_win_info(eventWindow);

   layer = 0;
   for (p =  wip->head, index = 0, sum2 = 1; p != NULL; p = p->next, index++)
   {
      if ((p->shape == STRATIGRAPHY) || (p->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                            getStratigraphyOptionsStructure (p);
         sum2 += stratOptions->numLayers;
         for (; layer < sum2; layer++) 
            eventNumForStrat[layer] = index;
      }
      if (p->shape == IMPORT)
      {
         importOptions = (IMPORT_OPTIONS *) p->options;
         sum2 += importOptions->numProperties;
         for (; layer < sum2; layer++) 
            eventNumForStrat[layer] = index;
      }      
   }

   for (p =  wip->head, index = 0; p != NULL; p = p->next, index++)
   {
      if ((p->shape == DYKE) || (p->shape == PLUG))
      {
         ignlut[index] = sum2++;
         for (; layer < sum2; layer++) 
            eventNumForStrat[layer] = index;
      }
   }

                          /* ************************************** */
                          /* build up arrays to describe the header */
   for (p =  wip->head, sum2 = 1; p != NULL; p = p->next)
   {
      if ((p->shape == STRATIGRAPHY) || (p->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                         getStratigraphyOptionsStructure (p);
         for (layer = 0; layer < stratOptions->numLayers; layer++, sum2++)
         {
            rho[sum2] = (float) (1000*stratOptions->properties[layer].density);

            sus[sum2][0] = (float) stratOptions->properties[layer].anisotropicField;
            sus[sum2][1] = (float) stratOptions->properties[layer].sus_X;
            if (stratOptions->properties[layer].anisotropicField)
            {
               sus[sum2][2] = (float) stratOptions->properties[layer].sus_Y;
               sus[sum2][3] = (float) stratOptions->properties[layer].sus_Z;
               ken[sum2][2] = 0.0;
               ken[sum2][3] = 0.0;
               *piAni = TRUE;
            }
            else
            {
               sus[sum2][2] = (float) 0.0;
               sus[sum2][3] = (float) 0.0;
               ken[sum2][2] =  0.0;
               ken[sum2][3] =  0.0;
            }
            if (stratOptions->properties[layer].remanentMagnetization)
            {
               ken[sum2][1] = stratOptions->properties[layer].strength;
               plu[sum2] = stratOptions->properties[layer].inclination;
               ang[sum2] = stratOptions->properties[layer].angleWithNorth;
               *piRem = TRUE;
            }
            else
            {
               ken[sum2][1] = 0.0;
               plu[sum2] = 0.0;
               ang[sum2] = 0.0;
            }
         }
      }
      if (p->shape == IMPORT)
      {
         importOptions = (IMPORT_OPTIONS *) (p->options);
         for (layer = 0; layer < importOptions->numProperties; layer++, sum2++)
         {
            rho[sum2] = (float) (1000*importOptions->properties[layer].density);

            sus[sum2][0] = (float) importOptions->properties[layer].anisotropicField;
            sus[sum2][1] = (float) importOptions->properties[layer].sus_X;
            if (importOptions->properties[layer].anisotropicField)
            {
               sus[sum2][2] = (float) importOptions->properties[layer].sus_Y;
               sus[sum2][3] = (float) importOptions->properties[layer].sus_Z;
               ken[sum2][2] = 0.0;
               ken[sum2][3] = 0.0;
               *piAni = TRUE;
            }
            else
            {
               sus[sum2][2] = (float) 0.0;
               sus[sum2][3] = (float) 0.0;
               ken[sum2][2] = 0.0;
               ken[sum2][3] = 0.0;
            }
            if (importOptions->properties[layer].remanentMagnetization)
            {
               ken[sum2][1] = (float) importOptions->properties[layer].strength;
               plu[sum2] = (float) importOptions->properties[layer].inclination;
               ang[sum2] = (float) importOptions->properties[layer].angleWithNorth;
               *piRem = TRUE;
            }
            else
            {
               ken[sum2][1] = (float) 0.0;
               plu[sum2] = (float) 0.0;
               ang[sum2] = (float) 0.0;
            }
         }
      }
   }

   for (p =  wip->head; p != NULL; p = p->next)
   {
      if ((p->shape == DYKE) || (p->shape == PLUG))
      {
         propOptions = (LAYER_PROPERTIES *) getLayerPropertiesStructure (p);
         rho[sum2] = (float) (1000*propOptions->density);

         sus[sum2][0] = (float) propOptions->anisotropicField;
         sus[sum2][1] = (float) propOptions->sus_X;
         if (propOptions->anisotropicField)
         {
            sus[sum2][2] = (float) propOptions->sus_Y;
            sus[sum2][3] = (float) propOptions->sus_Z;
            ken[sum2][2] = 0.0;
            ken[sum2][3] = 0.0;
            *piAni = TRUE;
         }
         else
         {
            sus[sum2][2] = (float) 0.0;
            sus[sum2][3] = (float) 0.0;
            ken[sum2][2] = 0.0;
            ken[sum2][3] = 0.0;
         }
         if (propOptions->remanentMagnetization)
         {
            ken[sum2][1] = propOptions->strength;
            plu[sum2] = propOptions->inclination;
            ang[sum2] = propOptions->angleWithNorth;
            *piRem = TRUE;
         }
         else
         {
            ken[sum2][1] = 0.0;
            plu[sum2] = 0.0;
            ang[sum2] = 0.0;
         }
         sum2++;
      }
   }

   if (geologyOptions.useTopography)
   {
      calculMaptopo(maptopo, xmax, ymax, 1.0, 1);
   }
                                                /* ******************** */
                                                /* Calculate Block Data */
   for (height = start, layer = 0; layer < zmax;
                                              height -= gblock, layer++)
   {
      switch (blockType)
      {
         case (ANOMALIES_MAP_BLOCK):
            initBlockModelAnomMap (dots, histoire, xmax, ymax,
                       gblock, height, blockOffsetX, blockOffsetY);
            break;
         case (DICER_BLOCK): case (LINE_MAP_BLOCK):
         case (SECTION_BLOCK): case (BLOCK_BLOCK):
            initBlockModelDicer (dots, histoire, xmax, ymax,
                       gblock, height, blockOffsetX, blockOffsetY);
            break;
         case (PROFILE_BLOCK):
            initBlockModelProfile (dots, histoire, xmax, ymax,
                       dx, dy, height, blockOffsetX, blockOffsetY);
            break;
      }

      if ((numEvents-1) > 0)
      {
         reverseEvents (dots, histoire, xmax, ymax);
      }

      if (geologyOptions.useTopography)
      {
         for (yIndex = ymax; yIndex >= 1; yIndex--)
         {
            for (xIndex = 1; xIndex <= xmax; xIndex++)
            {
               if (height > maptopo[xIndex][yIndex])
                  blockData[layer][xIndex][yIndex] = 0;
               else
               {
                  taste(numEvents, histoire[xIndex][yIndex].sequence, &flavor,
                                                                      &pindex);

                  if (flavor == IGNEOUS_STRAT)
                  {
                     blockData[layer][xIndex][yIndex] = ignlut[pindex];
                  }
                  else
                  {
                     for (index = 0; index < 50; index++)
                     {            /* Find the first strat that is from this event */
                        if (pindex == eventNumForStrat[index])
                        {         /* Now index will be the number of layers before it */
                           iLayer = index;
                          break;
                        }
                     }
                     which_prop(pindex, dots[xIndex][yIndex][1],
                                        dots[xIndex][yIndex][2],
                                        dots[xIndex][yIndex][3],
                                        flavor, &lutindex, &iStrat, &iLayer);
                     blockData[layer][xIndex][yIndex] = lutindex;
                  }
               } /* end height>maptopo[jj][kk] */
            }
         }
      } /* end if (geologyOptions.useTopography) */
      else
      {
         for (yIndex = ymax; yIndex >= 1; yIndex--)
         {
            for (xIndex = 1; xIndex <= xmax; xIndex++)
            {
               taste(numEvents, histoire[xIndex][yIndex].sequence, &flavor,
                                                                   &pindex);

               if (flavor == IGNEOUS_STRAT)
               {
                  blockData[layer][xIndex][yIndex] = ignlut[pindex];
               }
               else
               {
                  for (index = 0; index < 50; index++)
                  {            /* Find the first strat that is from this event */
                     if (pindex == eventNumForStrat[index])
                     {         /* Now index will be the number of layers before it */
                        iLayer = index;
                        break;
                     }
                  }
                        
                  which_prop(pindex, dots[xIndex][yIndex][1],
                                     dots[xIndex][yIndex][2],
                                     dots[xIndex][yIndex][3],
                                     flavor, &lutindex, &iStrat, &iLayer);
                  blockData[layer][xIndex][yIndex] = lutindex;
               }
            }
         }
      }          /* end else for if (geologyOptions.useTopography) */
   }
}


void calcAndSaveBlock (blockName, format, nx, ny, nz)
char *blockName;
char *format;
int nx, ny, nz;
{
#ifdef _MPL
   plural double ***dots;
   plural double **maptopo;
   plural STORY **histoire;
   plural int ***blockData;
#else
   double ***dots;
   double **maptopo;
   STORY **histoire;
   int ***blockData;
#endif
   int numEvents = (int) countObjects(NULL_WIN);
   int gblock, totalJob;
   int nlitho; /* Variables to describe block file representation (Header) */
   float rho[121],sus[121][4];
   double ken[121][4],ang[121],plu[121];
   int num[121], piRem, piAni;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   gblock = (int) viewOptions->geophysicsCubeSize;

#ifdef _MPL
   mapOverProcessors (nx, ny, &sizeX, &sizeY);
   if (!(blockData = (plural int ***) p_qitrimat(0,nz,0,sizeX,0,sizeY)))
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         //xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }
   if ((dots = (plural double ***) p_qdtrimat(0,sizeX,0,sizeY,0,3))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Change geophysics options or try closing some windows");
      else
         //xvt_dm_post_error("Change geophysics options or try closing some windows");
      p_free_qitrimat(blockData,0,nz,0,sizeX,0,sizeY);
      return;
   }
   if((maptopo = (plural double **) p_dmatrix(0,sizeX,0,sizeY))==0L)
   {
      p_freeqdtrimat(dots,0,sizeX,0,sizeY,0,3);
      p_free_qitrimat(blockData,0,nz,0,sizeX,0,sizeY);
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         //xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }
   if ((histoire = (plural STORY **) p_strstomat(0,sizeX,0,sizeY))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         //xvt_dm_post_error("Not enough memory, try closing some windows");
      p_free_dmatrix(maptopo,0,sizeX,0,sizeY);
      p_freeqdtrimat(dots,0,sizeX,0,sizeY,0,3);
      p_free_qitrimat(blockData,0,nz,0,sizeX,0,sizeY);
      return;
   }
#else
   if ((blockData = (int ***) itrimat(0,nz,0,nx,0,ny))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         //xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }
   if ((dots = (double ***) qdtrimat(0,nx,0,ny,0,3))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Change geophysics options or try closing some windows");
      else
         //xvt_dm_post_error("Change geophysics options or try closing some windows");
      free_itrimat(blockData,0,nz,0,nx,0,ny);
      return;
   }
   if((maptopo = (double **) dmatrix(0,nx,0,ny))==0L)
   {
      freeqdtrimat(dots,0,nx,0,ny,0,3);
      free_itrimat(blockData,0,nz,0,nx,0,ny);
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         //xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }
   if ((histoire = (struct story **) strstomat(0,nx,0,ny))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         //xvt_dm_post_error("Not enough memory, try closing some windows");
      free_itrimat(blockData,0,nz,0,nx,0,ny);
      freeqdtrimat(dots,0,nx,0,ny,0,3);
      free_dmatrix(maptopo,0,nx,0,ny);
      return;
   }
#endif

   totalJob = nz * nx * (numEvents-1);
   totalJob += nz;
   initLongJob (0, totalJob, "Saving Block Model...", NULL);

   if (strcmp (format, "DICER") == 0)
   {
      calcBlockModel (blockData, dots, histoire, maptopo, 0.0, 0.0,
                  &nlitho, nx, ny, nz, rho, sus, ken,
                  ang, plu, num, &piRem, &piAni, "DICER");
      writeDicerBlockFile (blockData, blockName, nx, ny, nz, FALSE);
   }
   else if (strcmp (format, "NODDY") == 0)
   {
      calcBlockModel (blockData, dots, histoire, maptopo, 0.0, 0.0,
                  &nlitho, nx, ny, nz, rho, sus, ken,
                  ang, plu, num, &piRem, &piAni, "ANOMALIES");
      writeBlockFile (blockData, blockName, nx, ny, nz, gblock,
                     nlitho, rho, sus, ken, ang, plu, num, piRem, piAni);
   }
   else if (strcmp (format, "LINE_MAP") == 0)
   {
      calcBlockModel (blockData, dots, histoire, maptopo, 0.0, 0.0,
                  &nlitho, nx, ny, nz, rho, sus, ken,
                  ang, plu, num, &piRem, &piAni, "LINE_MAP");
      writeGeologyBlockFile (blockData, blockName, nx, ny, nz, TRUE);
   }
   else if (strcmp (format, "SECTION") == 0)
   {
      calcBlockModel (blockData, dots, histoire, maptopo, 0.0, 0.0,
                  &nlitho, nx, ny, nz, rho, sus, ken,
                  ang, plu, num, &piRem, &piAni, "SECTION");
      writeGeologyBlockFile (blockData, blockName, nx, ny, nz, TRUE);
   }
   else if (strcmp (format, "BLOCK") == 0)
   {
      calcBlockModel (blockData, dots, histoire, maptopo, 0.0, 0.0,
                  &nlitho, nx, ny, nz, rho, sus, ken,
                  ang, plu, num, &piRem, &piAni, "BLOCK");
      writeBlockDiagramFile (blockData, blockName, nx, ny, nz, TRUE);
   }
   else
   {
      if (batchExecution)
         fprintf (stderr, "Unknown Block Format - Saving as Noddy Format");
      else
         //xvt_dm_post_error("Unknown Block Format - Saving as Noddy Format");
      calcBlockModel (blockData, dots, histoire, maptopo, 0.0, 0.0,
                  &nlitho, nx, ny, nz, rho, sus, ken,
                  ang, plu, num, &piRem, &piAni, "ANOMALIES");
      writeBlockFile (blockData, blockName, nx, ny, nz, gblock,
                     nlitho, rho, sus, ken, ang, plu, num, piRem, piAni);
   }

#ifdef _MPL
   p_free_qitrimat(blockData,0,nz,0,sizeX,0,sizeY);
   p_freeqdtrimat(dots,0,sizeX,0,sizeY,0,3);
   p_free_strstomat(histoire,0,sizeX,0,sizeY);
   p_free_dmatrix(maptopo,0,sizeX,0,sizeY);
#else
   freeqdtrimat(dots,0,nx,0,ny,0,3);
   free_strstomat(histoire,0,nx,0,ny);
   free_dmatrix(maptopo,0,nx,0,ny);
   free_itrimat(blockData,0,nz,0,nx,0,ny);
#endif
   
   finishLongJob();
}

void DoSaveBlockDicer (blockName)
char *blockName;
{
   int nx, ny, nz;
   double cubeSize;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   cubeSize = viewOptions->geophysicsCubeSize;
   nx = (int) (viewOptions->lengthX / cubeSize);
   ny = (int) (viewOptions->lengthY / cubeSize);
   nz = (int) (viewOptions->lengthZ / cubeSize + 1);
   calcAndSaveBlock (blockName, "DICER", nx, ny, nz);
}

void initBlockModelProfile (dots, histoire, xmax, ymax,
                                    dx, dy, height, offsetX, offsetY)
#ifdef _MPL
plural double ***dots;
plural STORY **histoire;
#else
double ***dots;
STORY **histoire;
#endif
int xmax, ymax;
double dx, dy, height, offsetX, offsetY;
{
#ifdef _MPL
   plural int mIndex, nIndex, m, n;
#else
   int mIndex, nIndex;
#endif
   double trx1, try1;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   trx1 = viewOptions->originX + viewOptions->lengthX;
   try1 = viewOptions->originY + viewOptions->lengthY;
   
#ifdef _MPL
   nIndex = 0;
   for (m = ixproc+1, mIndex = 0; m <= xmax; m += nxproc, mIndex++)
   {
      dots[mIndex][nIndex][1]=trx1+((m-1)*dx) + offsetX;
      dots[mIndex][nIndex][2]=try1+((m-1)*dy) + offsetY;
      dots[mIndex][nIndex][3]=height;
      histoire[mIndex][nIndex].again=1;
      p_izero(histoire[mIndex][nIndex].sequence);
   }
#else
   nIndex = 1;
   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      dots[mIndex][nIndex][1]=trx1+((mIndex-1)*dx) + offsetX;
      dots[mIndex][nIndex][2]=try1+((mIndex-1)*dy) + offsetY;
      dots[mIndex][nIndex][3]=height;
      histoire[mIndex][nIndex].again=1;
      izero(histoire[mIndex][nIndex].sequence);
   }
#endif
}

void initBlockModelDicer (dots, histoire, xmax, ymax, gblock,
                                          height, offsetX, offsetY)
#ifdef _MPL
plural double ***dots;
plural STORY **histoire;
#else
double ***dots;
STORY **histoire;
#endif
int xmax, ymax, gblock;
double height, offsetX, offsetY;
{
#ifdef _MPL
   plural int mIndex, nIndex, m, n;
#else
   int mIndex, nIndex;
#endif

#ifdef _MPL
   for (m = ixproc+1, mIndex = 0; m <= xmax; m += nxproc, mIndex++)
   {
      for (n = iyproc+1, nIndex = 0; n <= ymax; n += nyproc, nIndex++)
      {
         dots[mIndex][nIndex][1]=1.0e-5+((m-1)*gblock) + offsetX;
         dots[mIndex][nIndex][2]=1.0e-5+((ymax - n)*gblock) + offsetY;
         dots[mIndex][nIndex][3]=1.0e-5+height;
         histoire[mIndex][nIndex].again=1;
         p_izero(histoire[mIndex][nIndex].sequence);
      }
   }
#else
   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {
         dots[mIndex][nIndex][1]=1.0e-5+((mIndex-1)*gblock) + offsetX;
         dots[mIndex][nIndex][2]=1.0e-5+((ymax-nIndex)*gblock) + offsetY;
         dots[mIndex][nIndex][3]=1.0e-5+height;
         histoire[mIndex][nIndex].again=1;
         izero(histoire[mIndex][nIndex].sequence);
      }
   }
#endif
}

void initBlockModelAnomMap (dots, histoire, xmax, ymax, gblock,
                                            height, offsetX, offsetY)
#ifdef _MPL
plural double ***dots;
plural STORY **histoire;
#else
double ***dots;
STORY **histoire;
#endif
int xmax, ymax, gblock;
double height, offsetX, offsetY;
{
#ifdef _MPL
   plural int mIndex, nIndex, m, n;
#else
   int mIndex, nIndex;
#endif

#ifdef _MPL
   for (m = ixproc+1, mIndex = 0; m <= xmax; m += nxproc, mIndex++)
   {
      for (n = iyproc+1, nIndex = 0; n <= ymax; n += nyproc, nIndex++)
      {
         dots[mIndex][nIndex][1]=1.0e-5+((m-1)*gblock) + offsetX;
         dots[mIndex][nIndex][2]=1.0e-5+((ymax - n)*gblock) + offsetY;
         dots[mIndex][nIndex][3]=1.0e-5+height;
         histoire[mIndex][nIndex].again=1;
         p_izero(histoire[mIndex][nIndex].sequence);
      }
   }
#else
   for (mIndex = 1; mIndex <= xmax; mIndex++)
   {
      for (nIndex = 1; nIndex <= ymax; nIndex++)
      {
         dots[mIndex][nIndex][1]=1.0e-5+((mIndex-1)*gblock) + offsetX;
         dots[mIndex][nIndex][2]=1.0e-5+((ymax-nIndex)*gblock) + offsetY;
         dots[mIndex][nIndex][3]=1.0e-5+height;
         histoire[mIndex][nIndex].again=1;
         izero(histoire[mIndex][nIndex].sequence);
      }
   }
#endif
}

double
#if XVT_CC_PROTO
distanceToContact (double xLoc, double yLoc, double zLoc,
                   OBJECT *contactObject)
#else
distanceToContact (xLoc, yLoc, zLoc, contactObject)
double xLoc, yLoc, zLoc;
OBJECT *contactObject;
#endif
{
   double distance;
   double points[10][4];
   int point, axis;

                       /* Initialise Points */   
   for (point = 0; point < 10; point++)
      for (axis = 0; axis < 4; axis++)
         points[point][axis] = 0.0;
         
   points[5][1] = xLoc;
   points[5][2] = yLoc;
   points[5][3] = zLoc;

   switch (contactObject->shape)
   {
      case (FAULT):
         {
            FAULT_OPTIONS *options = (FAULT_OPTIONS *) contactObject->options;
            distance = fault (points, options);
         }
         break;
      case (SHEAR_ZONE):
         {
            SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) contactObject->options;
            distance = fault (points, options);
         }
         break;
      case (UNCONFORMITY):
         {
            UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) contactObject->options;
            distance = options->plane.a*xLoc
                                  + options->plane.b*yLoc
                                  + options->plane.c*zLoc
                                  + options->plane.d;
         }
         break;
      case (PLUG):
         {
            PLUG_OPTIONS *options = (PLUG_OPTIONS *) contactObject->options;
            distance = plug (points, options);
         }
         break;
      case (DYKE):
         {
            DYKE_OPTIONS *options = (DYKE_OPTIONS *) contactObject->options;
            distance = dyke (points, options);
         }
         break;
   }
   
   return (distance);
}

int
#if XVT_CC_PROTO
distanceToVector (double distance, OBJECT *contactObject,
                  double *xVec, double *yVec, double *zVec)
#else
distanceToVector (distance, contactObject, xVec, yVec, zVec)
double distance;
OBJECT *contactObject;
double *xVec, *yVec, *zVec;
#endif
{
   double forwrd[4][4];
   ROTATION_MATRICES *rotMatrix;
   int point, axis;
   double points[10][4];
   double vecPoints[10][4];

   if (!(rotMatrix = getRotationMatrices (contactObject)))
      return (FALSE);
      
   local4x4From3x3(rotMatrix->forward, forwrd);

                       /* Initialise Points */   
   for (point = 0; point < 10; point++)
      for (axis = 0; axis < 4; axis++)
         points[point][axis] = 0.0;
         
   points[3][1] = 1.0;
   points[4][2] = 1.0;
   points[5][3] = 1.0;

   matmlt(forwrd, points, vecPoints, 9);
         
   *xVec = -vecPoints[3][1]*distance;
   *yVec = -vecPoints[3][2]*distance;
   *zVec = -vecPoints[3][3]*distance;
   
   return (TRUE);
}

static int
#if XVT_CC_PROTO
applyAlterations (double distance, PROFILE_OPTIONS *altFcns, int z, int x, int y,
      float ***densityData,    float ***magSusData,     float ***remSusDecData,
      float ***remSusAziData,  float ***remSusStrData,  float ***aniSusDipData,
      float ***aniSusDDirData, float ***aniSusPitchData,float ***aniSusAxis1Data,
      float ***aniSusAxis2Data,float ***aniSusAxis3Data)
#else
applyAlterations (distance, altFcns, z, x, y,
                  densityData,     magSusData,      remSusDecData,
                  remSusAziData,   remSusStrData,   aniSusDipData,
                  aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
                  aniSusAxis2Data, aniSusAxis3Data)
double distance;
PROFILE_OPTIONS *altFcns;
int z, x, y;
float ***densityData,     ***magSusData,      ***remSusDecData,
      ***remSusAziData,   ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData,  ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data, ***aniSusAxis3Data;
#endif
{
   int distIndex;
   float *altArray;
   int replaceValues, altIndex, numAlterations;
   double scaledValue;
   PROFILE_OPTIONS *function,
                   *orderedAlterations[TOTAL_ALTERATIONS];
   int twoYProfile = 2*Y_PROFILE_RESOLUTION;

      /* Control the order that the alteratios are applyed in
      ** Rem and Ani being turned on and off must be last as these
      ** override the values that may have already been altered */
   memset ((char *) orderedAlterations, 0, sizeof(PROFILE_OPTIONS *)*TOTAL_ALTERATIONS);
   for (altIndex = TOTAL_ALTERATIONS-1, numAlterations = 0, function = altFcns;
                                        function; function = function->next)
   {                /* Place these at the end of the list to be performed last */
      if ((function->type == ANI_PROFILE) || (function->type == REM_PROFILE))
         orderedAlterations[altIndex--] = function;
      else
         orderedAlterations[numAlterations++] = function;
   }

   for (altIndex = 0; altIndex < TOTAL_ALTERATIONS; altIndex++)
   {
      function = orderedAlterations[altIndex];
      switch (function->type)
      {
         case (DENSITY_PROFILE):
            if (!densityData)
               continue;
            break;
         case (ANI_PROFILE):
            if (!aniSusAxis2Data || !aniSusAxis3Data || !aniSusDipData
                                 || !aniSusDDirData  || !aniSusPitchData)
               continue;
            break;
         case (SUS_X_PROFILE):
            if (!aniSusAxis1Data)
               continue;
            break;
         case (SUS_Y_PROFILE):
            if (!aniSusAxis2Data)
               continue;
            break;
         case (SUS_Z_PROFILE):
            if (!aniSusAxis3Data)
               continue;
            break;
         case (SUS_DIP_PROFILE):
            if (!aniSusDipData)
               continue;
            break;
         case (SUS_DDIR_PROFILE):
            if (!aniSusDDirData)
               continue;
            break;
         case (SUS_PITCH_PROFILE):
            if (!aniSusPitchData)
               continue;
            break;
         case (REM_PROFILE):
            if (!remSusStrData)
               continue;
            break;
         case (REM_DEC_PROFILE):
            if (!remSusDecData)
               continue;
            break;
         case (REM_INC_PROFILE):
            if (!remSusAziData)
               continue;
            break;
         case (REM_STR_PROFILE):
            if (!remSusStrData)
               continue;
            break;
      }
      altArray = function->array;
      if (function->joinType == SQUARE_SEGMENTS)
         replaceValues = TRUE;
      else
         replaceValues = FALSE;
      
      if (function->maxX < 1.0)  /* no distance on alteration, get out before divide by 0 */
         continue;
                      /* Convert distance into range of array (X_PROFILE_RESOLUTION) */
      distIndex = (int) floor (fabs(distance)/function->maxX*((double) X_PROFILE_RESOLUTION) + 0.5);
                     
      if (distIndex < X_PROFILE_RESOLUTION)  /* Leave values out of range unalted */
      {                /* make scaledValue (the level of alteration
                       ** at this point) in range from 0.0 - 1.0 */
         scaledValue = ((double) (altArray[distIndex]+Y_PROFILE_RESOLUTION))/twoYProfile;
         if (replaceValues)
            scaledValue = function->minYReplace +
                         scaledValue*(function->maxYReplace-function->minYReplace);
         else
            scaledValue = function->minYScale +
                          scaledValue*(function->maxYScale-function->minYScale);

         switch (function->type)
         {
            case (DENSITY_PROFILE):
               if (replaceValues)
                  densityData[z][x][y] = (float) (scaledValue*1000.0);
               else
                  densityData[z][x][y] *= (float) scaledValue;
               break;
            case (ANI_PROFILE):
               if (fabs((double) altArray[distIndex]) < 1.0)  /* Susceptibility */
               {

                  if (aniSusAxis2Data)
                     aniSusAxis2Data[z][x][y] = aniSusAxis1Data[z][x][y];
                  if (aniSusAxis3Data)
                     aniSusAxis3Data[z][x][y] = aniSusAxis1Data[z][x][y];

                  if (aniSusDipData)
                     aniSusDipData[z][x][y] = (float) NORMAL_MAG_FIELD_DIP;
                  if (aniSusDDirData)
                     aniSusDDirData[z][x][y] = (float) NORMAL_MAG_FIELD_DDIR;
                  if (aniSusPitchData)
                     aniSusPitchData[z][x][y] = (float) NORMAL_MAG_FIELD_PITCH;
               }
               break;
            case (SUS_X_PROFILE):
               if (replaceValues)
                  aniSusAxis1Data[z][x][y] = (float) (pow(10.0,scaledValue));
               else
                  aniSusAxis1Data[z][x][y] *= (float) pow(10.0,scaledValue);
               break;
            case (SUS_Y_PROFILE):
               if (replaceValues)
                  aniSusAxis2Data[z][x][y] = (float) (pow(10.0,scaledValue));
               else
                  aniSusAxis2Data[z][x][y] *= (float) pow(10.0,scaledValue);
               break;
            case (SUS_Z_PROFILE):
               if (replaceValues)
                  aniSusAxis3Data[z][x][y] = (float) (pow(10.0,scaledValue));
               else
                  aniSusAxis3Data[z][x][y] *= (float) pow(10.0,scaledValue);
               break;
            case (SUS_DIP_PROFILE):
               if (replaceValues)
                  aniSusDipData[z][x][y] = (float) (scaledValue);
               else
                  aniSusDipData[z][x][y] *= (float) scaledValue;
               break;
            case (SUS_DDIR_PROFILE):
               if (replaceValues)
                  aniSusDDirData[z][x][y] = (float) (scaledValue);
               else
                  aniSusDDirData[z][x][y] *= (float) scaledValue;
               break;
            case (SUS_PITCH_PROFILE):
               if (replaceValues)
                  aniSusPitchData[z][x][y] = (float) (scaledValue);
               else
                  aniSusPitchData[z][x][y] *= (float) scaledValue;
               break;
            case (REM_PROFILE):
               if (fabs((double) altArray[distIndex]) < 1.0)  /* Remanence */
               {
                  remSusStrData[z][x][y] = (float) 0.0000000000;
               }
               break;
            case (REM_DEC_PROFILE):
               if (replaceValues)
                  remSusDecData[z][x][y] = (float) (scaledValue);
               else
                  remSusDecData[z][x][y] *= (float) scaledValue;
               break;
            case (REM_INC_PROFILE):
               if (replaceValues)
                  remSusAziData[z][x][y] = (float) (scaledValue);
               else
                  remSusAziData[z][x][y] *= (float) scaledValue;
               break;
            case (REM_STR_PROFILE):
               if (replaceValues)
                  remSusStrData[z][x][y] = (float) (pow(10.0,scaledValue));
               else
                  remSusStrData[z][x][y] *= (float) pow(10.0,scaledValue);
               break;
         }
      }
   }

   return (TRUE);
}



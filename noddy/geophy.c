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

#define TOLERANCE 0.1
#define STRAT_LIMIT  200      /* number of indivdual strat layers */

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern int batchExecution;


                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
int allocateBlockModels (int, int **, int, int, int, int, int,
    short ****, float ****, float ****, float ****, float ****,
    float ****, float ****, float ****, float ****, float ****,
    float ****, float ****);
int freeBlockModels (int, int **,
    short ****, float ****, float ****, float ****, float ****,
    float ****, float ****, float ****, float ****, float ****,
    float ****, float ****);
int readInBlockModels (char *, int *, int ***, int *, int **,
    BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *, int *, int *,
    int *, int *, int *, int *, LAYER_PROPERTIES **, 
    short ****, float ****, float ****, float ****, float ****,
    float ****, float ****, float ****, float ****, float ****,
    float ****, float ****);
#else
#endif

int
#if XVT_CC_PROTO
allocateBlockModels (int numLayers, int **layerDimensions,
    int densityCalc, int susCalc, int remCalc, int aniCalc, int indexCalc,
    short ****indexData,
    float ****densityData,    float ****magSusData,     float ****remSusDecData, 
    float ****remSusAziData,  float ****remSusStrData,  float ****aniSusDipData,
    float ****aniSusDDirData, float ****aniSusPitchData,
    float ****aniSusAxis1Data,float ****aniSusAxis2Data,float ****aniSusAxis3Data)
#else
allocateBlockModels (numLayers, layerDimensions,
                     densityCalc, susCalc, remCalc, aniCalc, indexCalc,
                     indexData,
                     densityData,     magSusData,      remSusDecData, 
                     remSusAziData,   remSusStrData,   aniSusDipData,
                     aniSusDDirData,  aniSusPitchData,
                     aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data)
int numLayers, **layerDimensions;
int densityCalc, susCalc, remCalc, aniCalc, indexCalc;
short ****indexData;
float ****densityData,     ****magSusData,      ****remSusDecData, 
      ****remSusAziData,   ****remSusStrData,   ****aniSusDipData,
      ****aniSusDDirData,  ****aniSusPitchData, ****aniSusAxis1Data,
      ****aniSusAxis2Data, ****aniSusAxis3Data;
#endif
{
   //if (indexCalc) //save index even if deformable rem or alteration mwj_fix
   //{
      if (!(*indexData = (short ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(short))))
         return (FALSE);
   //}
  // else
   //{
      if (densityCalc)
      {
         if (!(*densityData = (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
      }
      if (susCalc)
      {
          if (!(*magSusData = (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
             return (FALSE);
      }   
      if (remCalc)
      {                             /* Remanent Susceptibility declination File */
         if (!(*remSusDecData = (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
                                 /* Remanent Susceptibility Azimuth File */
         if (!(*remSusAziData = (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
                                 /* Remanent Susceptibility Strength File */
         if (!(*remSusStrData = (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
      }
      if (aniCalc)
      {                             /* Anisotropy Susceptibility Dip File */
         if (!(*aniSusDipData  = (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
                                 /* Anisotropy Susceptibility Dip Direction File */
         if (!(*aniSusDDirData = (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
                                 /* Anisotropy Susceptibility Pitch File */
         if (!(*aniSusPitchData= (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
                         /* Anisotropy Susceptibility Primary Axis Strength File */
         if (!(*aniSusAxis1Data= (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
                         /* Anisotropy Susceptibility Secondary Axis Strength File */
         if (!(*aniSusAxis2Data= (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
                         /* Anisotropy Susceptibility Tertiary Axis Strength File */
         if (!(*aniSusAxis3Data= (float ***) create3DIregArray (numLayers,
                                  layerDimensions, sizeof(float))))
            return (FALSE);
      }
   //}
   return (TRUE);
}
 
int
#if XVT_CC_PROTO
freeBlockModels (int numLayers, int **layerDimensions,
    short ****indexData,
    float ****densityData,    float ****magSusData,     float ****remSusDecData, 
    float ****remSusAziData,  float ****remSusStrData,  float ****aniSusDipData,
    float ****aniSusDDirData, float ****aniSusPitchData,
    float ****aniSusAxis1Data,float ****aniSusAxis2Data,float ****aniSusAxis3Data)
#else
freeBlockModels (numLayers, layerDimensions, indexData,
                     densityData,     magSusData,      remSusDecData, 
                     remSusAziData,   remSusStrData,   aniSusDipData,
                     aniSusDDirData,  aniSusPitchData,
                     aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data)
int numLayers, **layerDimensions;
short ****indexData;
float ****densityData,     ****magSusData,      ****remSusDecData, 
      ****remSusAziData,   ****remSusStrData,   ****aniSusDipData,
      ****aniSusDDirData,  ****aniSusPitchData, ****aniSusAxis1Data,
      ****aniSusAxis2Data, ****aniSusAxis3Data;
#endif
{
   if (indexData)
      destroy3DIregArray ((char ***) *indexData, numLayers, layerDimensions);
   *indexData = NULL;

   if (densityData)
      destroy3DIregArray ((char ***) *densityData, numLayers, layerDimensions);
   *densityData = NULL;

   if (magSusData)
      destroy3DIregArray ((char ***) *magSusData, numLayers, layerDimensions);
   *magSusData = NULL;

   if (remSusDecData)
      destroy3DIregArray ((char ***) *remSusDecData, numLayers, layerDimensions);
   *remSusDecData = NULL;

   if (remSusAziData)
      destroy3DIregArray ((char ***) *remSusAziData, numLayers, layerDimensions);
   *remSusAziData = NULL;

   if (remSusStrData)
      destroy3DIregArray ((char ***) *remSusStrData, numLayers, layerDimensions);
   *remSusStrData = NULL;

   if (aniSusDipData)            
      destroy3DIregArray ((char ***) *aniSusDipData, numLayers, layerDimensions);
   *aniSusDipData = NULL;

   if (aniSusDDirData)
      destroy3DIregArray ((char ***) *aniSusDDirData, numLayers, layerDimensions);
   *aniSusDDirData = NULL;

   if (aniSusPitchData)
      destroy3DIregArray ((char ***) *aniSusPitchData, numLayers, layerDimensions);
   *aniSusPitchData = NULL;

   if (aniSusAxis1Data)
      destroy3DIregArray ((char ***) *aniSusAxis1Data, numLayers, layerDimensions);
   *aniSusAxis1Data = NULL;

   if (aniSusAxis2Data)
      destroy3DIregArray ((char ***) *aniSusAxis2Data, numLayers, layerDimensions);
   *aniSusAxis2Data = NULL;
                          
   if (aniSusAxis3Data)
      destroy3DIregArray ((char ***) *aniSusAxis3Data, numLayers, layerDimensions);
   *aniSusAxis3Data = NULL;

   return (TRUE);
}

 
int
#if XVT_CC_PROTO
readInBlockModels (char *blockName, int *numLayers, int ***layerDimensions,
    int *numCubeSizes, int **cubeSizes,
    BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *options,
    int *densityCalc, int *susCalc, int *remCalc, int *aniCalc, int *indexCalc,
    int *numProps, LAYER_PROPERTIES **layerProps, short ****indexData,
    float ****densityData,    float ****magSusData,     float ****remSusDecData, 
    float ****remSusAziData,  float ****remSusStrData,  float ****aniSusDipData,
    float ****aniSusDDirData, float ****aniSusPitchData,
    float ****aniSusAxis1Data,float ****aniSusAxis2Data,float ****aniSusAxis3Data)
#else
readInBlockModels (blockName, numLayers, layerDimensions,
                   numCubeSizes, cubeSizes, blockView, options,
                   densityCalc, susCalc, remCalc, aniCalc, indexCalc,
                   numProps, layerProps, indexData,
                   densityData,     magSusData,      remSusDecData, 
                   remSusAziData,   remSusStrData,   aniSusDipData,
                   aniSusDDirData,  aniSusPitchData,
                   aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data)
char *blockName;
int *numLayers, ***layerDimensions, *numCubeSizes, **cubeSizes;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
int *densityCalc, *susCalc, *remCalc, *aniCalc,
    *indexCalc, *numProps;
LAYER_PROPERTIES **layerProps;
short ****indexData;
float ****densityData,     ****magSusData,      ****remSusDecData, 
      ****remSusAziData,   ****remSusStrData,   ****aniSusDipData,
      ****aniSusDDirData,  ****aniSusPitchData, 
      ****aniSusAxis1Data, ****aniSusAxis2Data, ****aniSusAxis3Data;
#endif
{
   double lowerSouthWest_X, lowerSouthWest_Y, lowerSouthWest_Z;
   double upperNorthEast_X, upperNorthEast_Y, upperNorthEast_Z;
   int calcRangeInCubes;
   double minCubeSize;

   addFileExtention (blockName, ANOM_HEADER_FILE_EXT);
   readBlockHeaderFromFile (blockName, numLayers, layerDimensions,
               &lowerSouthWest_X, &lowerSouthWest_Y,
               &upperNorthEast_Z, &upperNorthEast_X,
               &upperNorthEast_Y, &lowerSouthWest_Z,
               numCubeSizes,   cubeSizes,  &calcRangeInCubes,
               &options->inclination,      &options->intensity,
               &options->declination, densityCalc, susCalc, remCalc,
               aniCalc, indexCalc, numProps, layerProps);

   minCubeSize = (double) MIN((*cubeSizes)[0],(*cubeSizes)[*numCubeSizes]);
   options->calculationRange = (int) (calcRangeInCubes*minCubeSize);
   
   blockView->lengthX = upperNorthEast_X - lowerSouthWest_X;
   blockView->lengthY = upperNorthEast_Y - lowerSouthWest_Y;
   blockView->lengthZ = upperNorthEast_Z - lowerSouthWest_Z;
   blockView->originX = lowerSouthWest_X;
   blockView->originY = lowerSouthWest_Y;
   blockView->originZ = lowerSouthWest_Z + blockView->lengthZ;
   blockView->geologyCubeSize = minCubeSize;
   blockView->geophysicsCubeSize = minCubeSize;
               
   allocateBlockModels (*numLayers, *layerDimensions,
                        *densityCalc, *susCalc, *remCalc, *aniCalc,
                        *indexCalc, indexData, 
                        densityData,    magSusData,     remSusDecData, 
                        remSusAziData,  remSusStrData,  aniSusDipData,
                        aniSusDDirData, aniSusPitchData,
                        aniSusAxis1Data,aniSusAxis2Data,aniSusAxis3Data);
   if (*indexData)
   {
      if (*indexData)
      {
         addFileExtention (blockName, ANOM_INDEX_FILE_EXT);
         read3DIregBlockFromFile (blockName, (char ***) *indexData,
                    *numLayers, *layerDimensions, sizeof(short));
      }
   }
   else
   {
      if (*densityCalc)
      {
         if (*densityData)
         {
            addFileExtention (blockName, ANOM_DENSITY_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *densityData,
                       *numLayers, *layerDimensions, sizeof(float));
         }
      }
      if (*susCalc)
      {
         if (*magSusData)
         {
            addFileExtention (blockName, ANOM_MAG_SUS_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *magSusData,
                       *numLayers, *layerDimensions, sizeof(float));
         }
      }
      if (*remCalc)
      {                             /* Remanent Susceptibility declination File */
         if (*remSusDecData)
         {
            addFileExtention (blockName, ANOM_REM_SUS_DEC_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *remSusDecData,
                       *numLayers, *layerDimensions, sizeof(float));
         }
                                 /* Remanent Susceptibility Azimuth File */
         if (*remSusAziData)
         {
            addFileExtention (blockName, ANOM_REM_SUS_AZI_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *remSusAziData,
                       *numLayers, *layerDimensions, sizeof(float));
         }
                                 /* Remanent Susceptibility Strength File */
         if (*remSusStrData)
         {
            addFileExtention (blockName, ANOM_REM_SUS_STR_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *remSusStrData,
                       *numLayers, *layerDimensions, sizeof(float));
         }
      }
      if (*aniCalc)
      {                             /* Anisotropy Susceptibility Dip File */
         if (*aniSusDipData)
         {
            addFileExtention (blockName, ANOM_ANI_SUS_DIP_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *aniSusDipData,
                       *numLayers, *layerDimensions, sizeof(float));
         }
                                 /* Anisotropy Susceptibility Dip Direction File */
         if (*aniSusDDirData)
         {
            addFileExtention (blockName, ANOM_ANI_SUS_DDIR_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *aniSusDDirData,
                       *numLayers, *layerDimensions, sizeof(float));
         }
                                 /* Anisotropy Susceptibility Pitch File */
         if (*aniSusPitchData)
         {
            addFileExtention (blockName, ANOM_ANI_SUS_PITCH_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *aniSusPitchData,
                       *numLayers, *layerDimensions, sizeof(float));
         }
                         /* Anisotropy Susceptibility Primary Axis Strength File */
         if (*aniSusAxis1Data)
         {
            addFileExtention (blockName, ANOM_ANI_SUS_AXIS1_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *aniSusAxis1Data,
                       *numLayers, *layerDimensions, sizeof(float));
         }
                         /* Anisotropy Susceptibility Secondary Axis Strength File */
         if (*aniSusAxis2Data)
         {
            addFileExtention (blockName, ANOM_ANI_SUS_AXIS2_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *aniSusAxis2Data,
                       *numLayers, *layerDimensions, sizeof(float));
         }
                         /* Anisotropy Susceptibility Tertiary Axis Strength File */
         if (*aniSusAxis3Data)
         {
            addFileExtention (blockName, ANOM_ANI_SUS_AXIS3_FILE_EXT);
            read3DIregBlockFromFile (blockName, (char ***) *aniSusAxis3Data,
                       *numLayers, *layerDimensions, sizeof(float));
         }
      }
   }
   
   return (TRUE);
} 
 
int
#if XVT_CC_PROTO
writeOutBlockModels (char *blockName, int numLayers, int **layerDimensions,
    int numCubeSizes, int *cubeSizes,
    BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *options,
    int densityCalc, int susCalc, int remCalc, int aniCalc, int indexCalc,
    int numProps, LAYER_PROPERTIES **layerProps, short ***indexData,
    float ***densityData,    float ***magSusData,     float ***remSusDecData, 
    float ***remSusAziData,  float ***remSusStrData,  float ***aniSusDipData,
    float ***aniSusDDirData, float ***aniSusPitchData,
    float ***aniSusAxis1Data,float ***aniSusAxis2Data,float ***aniSusAxis3Data)
#else
writeOutBlockModels (blockName, numLayers, layerDimensions,
                     numCubeSizes, cubeSizes, blockView, options,
                     densityCalc, susCalc, remCalc, aniCalc,
                     indexCalc, numProps, layerProps, indexData,
                     densityData,     magSusData,      remSusDecData, 
                     remSusAziData,   remSusStrData,   aniSusDipData,
                     aniSusDDirData,  aniSusPitchData,
                     aniSusAxis1Data, aniSusAxis2Data, aniSusAxis3Data)
char *blockName;
int numLayers, **layerDimensions, numCubeSizes, *cubeSizes;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
int densityCalc, susCalc, remCalc, aniCalc, indexCalc, numProps;
LAYER_PROPERTIES **layerProps;
short ***indexData;
float ***densityData,     ***magSusData,      ***remSusDecData, 
      ***remSusAziData,   ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData,  ***aniSusPitchData,
      ***aniSusAxis1Data, ***aniSusAxis2Data, ***aniSusAxis3Data;
#endif
{
   double xStart, yStart, zStart, xEnd, yEnd, zEnd;
   int nx = 0, ny = 0, cubeSize;
   int i, calcRangeInCubes;
   char dxfname[250];
   cubeSize = cubeSizes[0];
   for (i = 0; i < numLayers; i++)
   {
      if (layerDimensions[i][0] > nx)
        nx = layerDimensions[i][0];
      if (layerDimensions[i][1] > ny)
        ny = layerDimensions[i][1];
      if (cubeSizes[i] < cubeSize)
        cubeSize = cubeSizes[i];
   }
   
   calcRangeInCubes = (int) floor (options->calculationRange/cubeSize);
   
   xStart = blockView->originX - cubeSize*calcRangeInCubes;
   yStart = blockView->originY - cubeSize*calcRangeInCubes;
   zStart = blockView->originZ;
   xEnd = xStart + nx*cubeSize;
   yEnd = yStart + ny*cubeSize;
   for (i = 0, zEnd = zStart; i < numLayers; i++)
      zEnd = zEnd - cubeSizes[i];


   sprintf((char *) dxfname,"%s.dxf",blockName);

   addFileExtention (blockName, ANOM_HEADER_FILE_EXT);
   writeBlockHeaderToFile (blockName, numLayers, layerDimensions,
             xStart, yStart, zStart, xEnd, yEnd, zEnd,
             numCubeSizes,  cubeSizes,  calcRangeInCubes,
             options->inclination,      options->intensity,
             options->declination, densityCalc, susCalc, remCalc, aniCalc,
             indexCalc, numProps, layerProps);
             
  // if (indexCalc)//save index even if deformable rem or alteration mwj_fix
   //{
      addFileExtention (blockName, ANOM_INDEX_FILE_EXT);
      write3DIregBlockToFile (blockName, (char ***) indexData,
                       numLayers, layerDimensions, sizeof(short));
      //doTopology(blockName,(char ***) indexData);
      //do3dStratMap ((THREED_IMAGE_DATA *) NULL, dxfname); //comment out for for ipython

  // }
   //else
  // {
      if (densityCalc)
      {
         addFileExtention (blockName, ANOM_DENSITY_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) densityData,
                       numLayers, layerDimensions, sizeof(float));
      }
      if (susCalc)
      {
         addFileExtention (blockName, ANOM_MAG_SUS_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) magSusData,
                       numLayers, layerDimensions, sizeof(float));
      }
      if (remCalc)
      {                             /* Remanent Susceptibility declination File */
         addFileExtention (blockName, ANOM_REM_SUS_DEC_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) remSusDecData,
                       numLayers, layerDimensions, sizeof(float));
                                 /* Remanent Susceptibility Azimuth File */
         addFileExtention (blockName, ANOM_REM_SUS_AZI_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) remSusAziData,
                       numLayers, layerDimensions, sizeof(float));
                                 /* Remanent Susceptibility Strength File */
         addFileExtention (blockName, ANOM_REM_SUS_STR_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) remSusStrData,
                       numLayers, layerDimensions, sizeof(float));
      }
      if (aniCalc)
      {                              /* Anisotropy Susceptibility Dip File */
         addFileExtention (blockName, ANOM_ANI_SUS_DIP_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) aniSusDipData,
                       numLayers, layerDimensions, sizeof(float));
                                 /* Anisotropy Susceptibility Dip Direction File */
         addFileExtention (blockName, ANOM_ANI_SUS_DDIR_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) aniSusDDirData,
                       numLayers, layerDimensions, sizeof(float));
                                 /* Anisotropy Susceptibility Pitch File */
         addFileExtention (blockName, ANOM_ANI_SUS_PITCH_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) aniSusPitchData,
                       numLayers, layerDimensions, sizeof(float));
                         /* Anisotropy Susceptibility Primary Axis Strength File */
         addFileExtention (blockName, ANOM_ANI_SUS_AXIS1_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) aniSusAxis1Data,
                       numLayers, layerDimensions, sizeof(float));
                         /* Anisotropy Susceptibility Secondary Axis Strength File */
         addFileExtention (blockName, ANOM_ANI_SUS_AXIS2_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) aniSusAxis2Data,
                       numLayers, layerDimensions, sizeof(float));
                         /* Anisotropy Susceptibility Tertiary Axis Strength File */
         addFileExtention (blockName, ANOM_ANI_SUS_AXIS3_FILE_EXT);
         write3DIregBlockToFile (blockName, (char ***) aniSusAxis3Data,
                       numLayers, layerDimensions, sizeof(float));
      }
   //}

   return (TRUE);
}

int
#if XVT_CC_PROTO
assignLayerInBlockModels (LAYER_PROPERTIES ***blockLayer,
               int z, int nx, int ny, int numProps,
               LAYER_PROPERTIES **rockProps,
               int deformableRemanence, int deformableAnisotropy,
               short ***indexData, float ***densityData,
               float ***magSusData, float ***remSusDecData,
               float ***remSusAziData, float ***remSusStrData,
               float ***aniSusDipData, float ***aniSusDDirData,
               float ***aniSusPitchData, float ***aniSusAxis1Data,
               float ***aniSusAxis2Data, float ***aniSusAxis3Data)
#else
assignLayerInBlockModels (blockLayer, z, nx, ny, numProps, rockProps,
                  deformableRemanence, deformableAnisotropy, indexData,
                  densityData,    magSusData,     remSusDecData,
                  remSusAziData,  remSusStrData,  aniSusDipData,
                  aniSusDDirData, aniSusPitchData,aniSusAxis1Data,
                  aniSusAxis2Data,aniSusAxis3Data)
LAYER_PROPERTIES ***blockLayer;
int z, nx, ny, numProps, deformableRemanence, deformableAnisotropy;
LAYER_PROPERTIES **rockProps;
short ***indexData;
float ***densityData,     ***magSusData,      ***remSusDecData,
      ***remSusAziData,   ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData,  ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data, ***aniSusAxis3Data;
#endif
{
   register int x, y, i;
   
   if (indexData)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
               for (i = 0; i < numProps; i++)
                  if (rockProps[i] == blockLayer[x][y])
                  {
                     indexData[z][x][y] = i+1;
                     break;
                  }
   }

   if (densityData)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
               densityData[z][x][y] = (float) (1000.0*blockLayer[x][y]->density);
   }
   
   if (magSusData)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
               magSusData[z][x][y] = (float) blockLayer[x][y]->sus_X;
   }
   
   if (remSusDecData && !deformableRemanence)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y] && blockLayer[x][y]->remanentMagnetization)
               remSusDecData[z][x][y] = (float) blockLayer[x][y]->angleWithNorth;
            else
               remSusDecData[z][x][y] = (float) 0.0;
   }
   
   if (remSusAziData && !deformableRemanence)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y] && blockLayer[x][y]->remanentMagnetization)
               remSusAziData[z][x][y] = (float) blockLayer[x][y]->inclination;
            else
               remSusAziData[z][x][y] = (float) 0.0;
   }
   
   if (remSusStrData)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y] && blockLayer[x][y]->remanentMagnetization)
               remSusStrData[z][x][y] = (float) blockLayer[x][y]->strength;
            else
               remSusStrData[z][x][y] = (float) 0.0;
   }
   
   if (aniSusDipData && !deformableAnisotropy)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
            {
               if (blockLayer[x][y]->anisotropicField)
                  aniSusDipData[z][x][y] = (float) blockLayer[x][y]->sus_dip;
               else
                  aniSusDipData[z][x][y] = (float) NORMAL_MAG_FIELD_DIP;
            }
            else
               aniSusDipData[z][x][y] = (float) 0.0;
   }
   
   if (aniSusDDirData && !deformableAnisotropy)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
            {
               if (blockLayer[x][y]->anisotropicField)
                  aniSusDDirData[z][x][y] = (float) blockLayer[x][y]->sus_dipDirection;
               else
                  aniSusDDirData[z][x][y] = (float) NORMAL_MAG_FIELD_DDIR;
            }
            else
               aniSusDDirData[z][x][y] = (float) 0.0;
   }
   
   if (aniSusPitchData && !deformableAnisotropy)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
            {
               if (blockLayer[x][y]->anisotropicField)
                  aniSusPitchData[z][x][y] = (float) blockLayer[x][y]->sus_pitch;
               else
                  aniSusPitchData[z][x][y] = (float) NORMAL_MAG_FIELD_PITCH;
            }
            else
               aniSusPitchData[z][x][y] = (float) 0.0;
   }
   
   if (aniSusAxis1Data)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
               aniSusAxis1Data[z][x][y] = (float) blockLayer[x][y]->sus_X;
            else
               aniSusAxis1Data[z][x][y] = (float) 0.0;
   }

   if (aniSusAxis2Data)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
            {
               if (blockLayer[x][y]->anisotropicField)
                  aniSusAxis2Data[z][x][y] = (float) blockLayer[x][y]->sus_Y;
               else
                  aniSusAxis2Data[z][x][y] = (float) blockLayer[x][y]->sus_X;
            }
            else
               aniSusAxis2Data[z][x][y] = (float) 0.0;
   }

   if (aniSusAxis3Data)
   {
      for (x = 0; x < nx; x++)
         for (y = 0; y < ny; y++)
            if (blockLayer[x][y])
            {
               if (blockLayer[x][y]->anisotropicField)
                  aniSusAxis3Data[z][x][y] = (float) blockLayer[x][y]->sus_Z;
               else
                  aniSusAxis3Data[z][x][y] = (float) blockLayer[x][y]->sus_X;
            }
            else
               aniSusAxis3Data[z][x][y] = (float) 0.0;
   }

   return (TRUE);
}
 
int
#if XVT_CC_PROTO
createMagImage (char *outputFilename, char *blockName, int numLayers,
        int **layerDimensions, int numCubeSizes, int *cubeSizes,
        BLOCK_VIEW_OPTIONS *viewOptions, GEOPHYSICS_OPTIONS *options,
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
createMagImage (outputFilename, blockName, numLayers,
        layerDimensions, numCubeSizes, cubeSizes, viewOptions,
        options, xyzCalc, pointPos, numPoints, xyzImportFile,
        densityCalc, susCalc, remCalc, aniCalc,
        indexCalc, numProps, propArray,   indexData,
        densityData,     magSusData,      remSusDecData,
        remSusAziData,   remSusStrData,   aniSusDipData,
        aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
        aniSusAxis2Data, aniSusAxis3Data, magImage, grvImage)
char *outputFilename, *blockName;
int numLayers, **layerDimensions, numCubeSizes, *cubeSizes;
BLOCK_VIEW_OPTIONS *viewOptions;
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
	int result;

   if (options->calculationMethod == SPECTRAL)
   {
      result = magCalc (outputFilename, blockName, numLayers,
            layerDimensions, numCubeSizes, cubeSizes, viewOptions,
            options, xyzCalc, pointPos, numPoints, xyzImportFile,
            densityCalc, susCalc, remCalc, aniCalc,
            indexCalc, numProps, propArray, indexData,
            densityData, magSusData, remSusDecData, remSusAziData,
            remSusStrData, aniSusDipData, aniSusDDirData,
            aniSusPitchData, aniSusAxis1Data, aniSusAxis2Data,
            aniSusAxis3Data, magImage, grvImage);
   }
   else if (options->calculationMethod == SPATIAL) /* Spatial */
   {
      result = calcAnomalies (outputFilename, blockName, numLayers,
            layerDimensions, numCubeSizes, cubeSizes, viewOptions,
            options, densityCalc, susCalc, remCalc, aniCalc,
            indexCalc, numProps, propArray, indexData,
            densityData, magSusData, remSusDecData, remSusAziData,
            remSusStrData, aniSusDipData, aniSusDDirData,
            aniSusPitchData, aniSusAxis1Data, aniSusAxis2Data,
            aniSusAxis3Data, magImage, grvImage);
   }
	else /* Full */
	{
		result = calcCompleteAnomalies (outputFilename, blockName, numLayers,
            layerDimensions, numCubeSizes, cubeSizes, viewOptions,
            options, xyzCalc, pointPos, numPoints, xyzImportFile,
				densityCalc, susCalc, remCalc, aniCalc,
            indexCalc, numProps, propArray, indexData,
            densityData, magSusData, remSusDecData, remSusAziData,
            remSusStrData, aniSusDipData, aniSusDDirData,
            aniSusPitchData, aniSusAxis1Data, aniSusAxis2Data,
            aniSusAxis3Data, magImage, grvImage);
	}

	return (result);
}

int
#if XVT_CC_PROTO
matchImageResolutions (char *outputFilename,
                       char *fineFilename, char *coarseFilename)
#else
matchImageResolutions (outputFilename, fineFilename, coarseFilename)
char *outputFilename, *fineFilename, *coarseFilename;
#endif
{
   FILE *fi, *fo;
   double **fineImageData, **coarseImageData, **interpImageData;
   int fileId, icsize, xmax, ymax, zmax, binary;
   float dat[3], position[6], grid, airgap;
   int fineSizeX, fineSizeY, coarseSizeX, coarseSizeY;
   double coarseRatioX, coarseRatioY, output;
   int x, y, result = TRUE;
         
                   /* Load in the Two images */
                           /* second read the coarse image */
   if (fi = fopen (coarseFilename, "r"))
   {
      if (readGeophysHeader (fi, &fileId, &icsize, &xmax, &ymax, &zmax,
               dat, position, &grid, &airgap, &coarseSizeX, &coarseSizeY, &binary))
      {
         if (coarseImageData = (double **) create2DArray (coarseSizeY, coarseSizeX, sizeof(double)))
         {
            readGeophys (fi, (unsigned char **) NULL, (double **) coarseImageData,
                         coarseSizeX, coarseSizeY, NULL, NULL, FALSE, binary);
         }
         else
            result = FALSE;
      }
      else
         result = FALSE;
      fclose (fi);
   }
   else
      result = FALSE;

                           /* first read the coarse image */
   if (result && (fi = fopen (fineFilename, "r")))
   {
      if (readGeophysHeader (fi, &fileId, &icsize, &xmax, &ymax, &zmax,
               dat, position, &grid, &airgap, &fineSizeX, &fineSizeY, &binary))
      {
         if (fineImageData = (double **) create2DArray (fineSizeY, fineSizeX, sizeof(double)))
         {
            readGeophys (fi, (unsigned char **) NULL, (double **) fineImageData,
                         fineSizeX, fineSizeY, NULL, NULL, FALSE, binary);
         }
         else
            result = FALSE;
      }
      else
         result = FALSE;
      fclose (fi);
   }
   else
      result = FALSE;

   if (interpImageData = (double **) create2DArray (fineSizeY, fineSizeX, sizeof(double)))
      interpDblArray(coarseImageData, interpImageData,
                     coarseSizeY, coarseSizeX, fineSizeY, fineSizeX, 1.0, 0.0);
   else
      result = FALSE;
      
        /* Add the coarse image at the fine resolution to get the final images */
   if (result)
   {                   
      coarseRatioX = (double) ((double) coarseSizeX)/((double) fineSizeX);
      coarseRatioY = (double) ((double) coarseSizeY)/((double) fineSizeY);
      for (x = 0; x < fineSizeX; x++)
      {
/*
         cx = (int) floor(((double) x)*coarseRatioX);
         if (cx > coarseSizeX)
            cx = coarseSizeX;
         else if (cx < 0)
            cx = 0;
*/         
         for (y = 0; y < fineSizeY; y++)
         {
/*
            cy = (int) floor(((double) y)*coarseRatioY);
            if (cy > coarseSizeY)
               cy = coarseSizeY;
            else if (cy < 0)
               cy = 0;
            
            fineImageData[y+1][x+1] += coarseImageData[cy+1][cx+1];
*/
            fineImageData[y][x] += interpImageData[y][x];
         }
      }

                      /* Write out the result image */
      if (fo = fopen (outputFilename, "w"))
      {
         fprintf(fo,"%d\n", fileId);
         fprintf(fo,"%d\t%d\t%d\t%d\n", icsize, xmax, ymax, zmax);
         fprintf(fo,"%f\t%f\t%f\n", dat[0], dat[1], dat[2]);
         fprintf(fo,"%f\t%f\t%f\n", position[0], position[1], position[2]);
         fprintf(fo,"%f\t%f\t%f\n", position[3], position[4], position[5]);
         fprintf(fo,"%f\t%f\n", grid, airgap);  
      
         for (y = 0; y < fineSizeY; y++)
         {
            for (x = 0; x < fineSizeX; x++)
            {
               output = fineImageData[y][x];
               fprintf(fo, "%lf\t", output);
            }
            fprintf(fo,"\n");
         }
         fclose (fo);
      }
      else
         result = FALSE;
   }

                   /* Free the memory */
   destroy2DArray ((char **) interpImageData, fineSizeY,   fineSizeX);
   destroy2DArray ((char **) fineImageData,   fineSizeY,   fineSizeX);
   destroy2DArray ((char **) coarseImageData, coarseSizeY, coarseSizeX);

   return (result);
}


int
#if XVT_CC_PROTO
doGeophysics (int calcType, BLOCK_VIEW_OPTIONS *blockView,
              GEOPHYSICS_OPTIONS *options, char *outputFilename,
              char *blockName, double **pointPos, int numPoints,
              FILE_SPEC *xyzImportFile, DOUBLE_2D_IMAGE *magImage, DOUBLE_2D_IMAGE *grvImage)
#else
doGeophysics (calcType, blockView, options, outputFilename, blockName,
              pointPos, numPoints, xyzImportFile, magImage, grvImage)
int calcType;
BLOCK_VIEW_OPTIONS *blockView;
GEOPHYSICS_OPTIONS *options;
char *outputFilename;
char *blockName;
double **pointPos;
int numPoints;
FILE_SPEC *xyzImportFile;
DOUBLE_2D_IMAGE *magImage, *grvImage;
#endif
{
   float ***densityData = NULL,    ***magSusData = NULL,
         ***remSusDecData = NULL,  ***remSusAziData = NULL,
         ***remSusStrData = NULL,  ***aniSusDipData = NULL,
         ***aniSusDDirData = NULL, ***aniSusPitchData = NULL,
         ***aniSusAxis1Data = NULL,***aniSusAxis2Data = NULL,
         ***aniSusAxis3Data = NULL;
   float ***nullData = NULL;
   short ***nullShortData = NULL;
   short ***indexData = NULL;
   int **layerDimensions, *cubeSizes;
   char memLabel[10];
   LAYER_PROPERTIES *stratArray[STRAT_LIMIT];
   int numLayers, numCubeSizes, numStrat, numProps;
   int numFineLayers, numCoarseLayers;
   int nx, ny, nz, z, i, jobLength, totalX, totalY;
   int readBlocks, calcBlocks, writeBlocks, calcMag, calcGrav;
   int xyzCalc, densityCalc, susCalc, remCalc, aniCalc, indexCalc;
   int defRem, defAni, altZones;
   double xLoc, yLoc, zLoc;
   int calcRangeInCubes;
   FILE *outfile;
   char topofname[100];
   
   strcpy (memLabel, "Anom");
     
   if (!memManagerAddLabel (memLabel))
   {
      if (batchExecution)
         fprintf (stderr, "Error, Not Enough memory for block");
      else
         xvt_dm_post_error ("Error, Not Enough memory for block");
      return (FALSE);
   }

   defRem = options->deformableRemanence;
   defAni = options->deformableAnisotropy;
   if ((options->calculationMethod == SPATIAL) || (options->calculationMethod == SPATIAL_FULL))
      calcRangeInCubes = (int) floor (options->calculationRange/blockView->geophysicsCubeSize);
   else
      calcRangeInCubes = 0;

                    /* **************************************************** */
                    /* Work out dimensions of top most layer and num Layers */
   nx = (int) (blockView->lengthX/blockView->geophysicsCubeSize)
                         + (2*calcRangeInCubes); /* !! + 1 */
   ny = (int) (blockView->lengthY/blockView->geophysicsCubeSize)
                         + (2*calcRangeInCubes); /* !! + 1 */
   nz = (int) (blockView->lengthZ/blockView->geophysicsCubeSize);
   numCubeSizes = numLayers = nz;

                  /* ********************************************************** */
                  /* Allow for larger box sizes on lower layers (Clever Boxing) */
   if (!(layerDimensions = (int **) create2DArray (numLayers, 2, sizeof (int))))
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         xvt_dm_post_error("Not enough memory, try closing some windows");
      memManagerFreeAfterLabel (memLabel);
      return (FALSE);
   }
   if (!(cubeSizes = (int *) create1DArray (numCubeSizes, sizeof (int))))
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         xvt_dm_post_error("Not enough memory, try closing some windows");
      memManagerFreeAfterLabel (memLabel);
      return (FALSE);
   }
   
   numFineLayers = 0; numCoarseLayers = 0;
   for (z = numCubeSizes-1, zLoc = 0.0 + blockView->geophysicsCubeSize/2.0;
                            z >= 0; z--, zLoc += blockView->geophysicsCubeSize)
   {
      if ((zLoc < options->constantBoxDepth)
                  || (fabs(options->constantBoxDepth) < TOLERANCE))
      {
         cubeSizes[z] = (int) blockView->geophysicsCubeSize;  /* same constante size */
         numFineLayers++;
      }
      else
      {
         cubeSizes[z] = (int) (blockView->geophysicsCubeSize * options->cleverBoxRatio);
         numCoarseLayers++;
      }
   }

   for (z = 0; z < numLayers; z++)
   {
      layerDimensions[z][0] = (int) (blockView->lengthX/cubeSizes[z])
                                                  + (2*calcRangeInCubes); /* !! + 1 */
      layerDimensions[z][1] = (int) (blockView->lengthY/cubeSizes[z])
                                                  + (2*calcRangeInCubes); /* !! + 1 */

   }

                                     /* ********************************* */
                                     /* Every path will need the forward  */
                                     /* modeling so add that to the gauge */
   jobLength = totalX = totalY = 0;
   for (z = 0; z < numLayers; z++)
   {
      totalX += layerDimensions[z][0];
      totalY += layerDimensions[z][1];
   }
   jobLength = totalX * (countObjects(NULL_WIN)-1);

                                     /* ****************************** */
                                     /* Decide on steps for operation */
   switch (calcType)
   {
      case (ANOM): 
         readBlocks = FALSE; calcBlocks = TRUE;  writeBlocks = FALSE;
         calcMag    = TRUE;  calcGrav   = TRUE;  xyzCalc = FALSE;

         if (options->calculationMethod == SPECTRAL)
            jobLength = jobLength + nz*ny + ny*3;
         else if (options->calculationMethod == SPATIAL_FULL)
            jobLength = jobLength + totalX + (nz * 2 * calcRangeInCubes);
         else /* SPATIAL Convolution */
            jobLength = jobLength + totalY*2 + (nz * (2 * calcRangeInCubes + 1));
         initLongJob (0, jobLength, "Calculating Anomalies...", memLabel);
         break;
      case (XYZ_ANOM):
         if (!pointPos || !numPoints)
         {
            if (batchExecution)
               fprintf (stderr, "Error, No XYZ Locations to calculate at.");
            else
               xvt_dm_post_error ("Error, No XYZ Locations to calculate at.");
            return (FALSE);
         }
         readBlocks = FALSE; calcBlocks = TRUE;  writeBlocks = FALSE;
         calcMag    = TRUE;  calcGrav   = TRUE;  xyzCalc = TRUE;

         if (options->calculationMethod == SPECTRAL)
            jobLength = jobLength + nz*ny + ny*3;
         else if (options->calculationMethod == SPATIAL_FULL)
            jobLength = jobLength + totalX + (nz * 2 * calcRangeInCubes);
         else /* SPATIAL Convolution */
            jobLength = jobLength + totalY*2 + (nz * (2 * calcRangeInCubes + 1));
         initLongJob (0, jobLength, "Calculating Anomalies at XYZ's...", memLabel);
         break;
      case (ANOM_FROM_BLOCK):
         readBlocks = TRUE;  calcBlocks = FALSE; writeBlocks = FALSE;
         calcMag    = TRUE;  calcGrav   = TRUE;  xyzCalc = FALSE;
         
         if (!blockName)
         {
            memManagerFreeAfterLabel (memLabel);
            return (FALSE);
         }

         if (options->calculationMethod == SPECTRAL)
            jobLength = jobLength + nz*ny + ny*3;
         else
            jobLength = jobLength + totalY + (nz * 2 * calcRangeInCubes);
         initLongJob (0, jobLength, "Calculating Anom. From Block...", memLabel);
         break;
      case (BLOCK_AND_ANOM):
         readBlocks = FALSE; calcBlocks = TRUE;  writeBlocks = TRUE;
         calcMag    = TRUE;  calcGrav   = TRUE;  xyzCalc = FALSE;

         if (options->calculationMethod == SPECTRAL)
            jobLength = jobLength + nz*ny + ny*3;
         else
            jobLength = jobLength + totalY + (nz * 2 * calcRangeInCubes);
         initLongJob (0, jobLength, "Calculating Anom. and Block...", memLabel);
         break;
      case (BLOCK_ONLY):
         readBlocks = FALSE; calcBlocks = TRUE;  writeBlocks = TRUE;
         calcMag    = FALSE; calcGrav   = FALSE; xyzCalc = FALSE;

         jobLength = jobLength + totalY;
         initLongJob (0, jobLength, "Calculating Block...", memLabel);
         break;
      case (TOPOLOGY):
         readBlocks = FALSE; calcBlocks = TRUE;  writeBlocks = TRUE;
         calcMag    = FALSE; calcGrav   = FALSE; xyzCalc = FALSE;

         jobLength = jobLength + totalY;
         initLongJob (0, jobLength, "Calculating Block...", memLabel);
         break;
      default:
         memManagerFreeAfterLabel (memLabel);
         return (FALSE);
   }
   
                                     /* ****************** */
                                     /* Read Block File In */
   if (readBlocks)
   {
      readInBlockModels (blockName, &numLayers, &layerDimensions,
               &numCubeSizes, &cubeSizes, blockView, options,
               &densityCalc, &susCalc, &remCalc, &aniCalc,
               &indexCalc, &numProps, stratArray, &indexData,
               &densityData, &magSusData, &remSusDecData, &remSusAziData,
               &remSusStrData, &aniSusDipData, &aniSusDDirData,
               &aniSusPitchData, &aniSusAxis1Data, &aniSusAxis2Data,
               &aniSusAxis3Data);
   }
   else
   {
      numStrat = assignPropertiesForStratLayers(stratArray, STRAT_LIMIT);
      densityCalc = TRUE;  /* We always need density */
      susCalc = TRUE;   /* Assume we need sus calc */
      aniCalc = remCalc = altZones = FALSE;

          /* Only calc alteration Zones if we have an event with one */
      altZones =  checkHistoryForAlterations ();
          
          /* Only calc remanence or anisotropy if we have a strat with it used in */
      for (i = 0; i < numStrat; i++)
      {
         if (stratArray[i]->anisotropicField)
            aniCalc = TRUE;
         if (stratArray[i]->remanentMagnetization)
            remCalc = TRUE;
      }

      if ((remCalc && defRem) || (aniCalc && defAni) || altZones
                              || options->magneticVectorComponents
                              || !options->projectVectorsOntoField)
         indexCalc = FALSE;   /* cannot calculate by index as can vary at any loc */
      else
         indexCalc = TRUE;  /* simplify calculation by using indexed block */

      if (options->magneticVectorComponents || !options->projectVectorsOntoField)
         aniCalc = TRUE;

      if (aniCalc)
         susCalc = FALSE;
      if (altZones) /* Need full indexing for alt zones in case */
      {             /* the values a change in the zones */
          susCalc = FALSE;
          aniCalc = TRUE;
          remCalc = TRUE;
      }
         
      defRem = defRem && remCalc;  /* true if we actually will calc def Rem */
      defAni = defAni && aniCalc;  /* true if we actually will calc def Ani */

      if (indexCalc)
         numProps = numStrat;

      allocateBlockModels (numLayers, layerDimensions,
             densityCalc, susCalc, remCalc, aniCalc, indexCalc,
             &indexData,
             &densityData,    &magSusData,     &remSusDecData, 
             &remSusAziData,  &remSusStrData,  &aniSusDipData,
             &aniSusDDirData, &aniSusPitchData,
             &aniSusAxis1Data,&aniSusAxis2Data,&aniSusAxis3Data);
   }
   
                                     /* *************** */
                                     /* Calculate Block */

   if(calcType == TOPOLOGY)
   {
	   sprintf(topofname,"%s.g21",outputFilename);
	   outfile=fopen(topofname,"w");
   }
   if (calcBlocks)
   {
      LAYER_PROPERTIES ***blockLayer;
      
      for (z = numLayers-1, zLoc = blockView->originZ; z >= 0; z--)
      {
                           /* Calc at cube centers in x, y directions */
         xLoc = blockView->originX - cubeSizes[z]*calcRangeInCubes + cubeSizes[z]/2.0;
         yLoc = blockView->originY - cubeSizes[z]*calcRangeInCubes + cubeSizes[z]/2.0;
         if (blockLayer = (LAYER_PROPERTIES ***)
                              create2DArray(layerDimensions[z][0],
                                            layerDimensions[z][1],
                                            sizeof(LAYER_PROPERTIES *)))
         {
            zLoc = zLoc - cubeSizes[z]/2.0;

            if(calcType == TOPOLOGY)
            {
            	calcTopology(blockLayer,
                        layerDimensions[z][0], layerDimensions[z][1],
                        xLoc, yLoc, zLoc, cubeSizes[z], geologyOptions.useTopography,
                        numProps, stratArray, defRem, defAni, altZones, z, indexData,
                        densityData,     magSusData,      remSusDecData,
                        remSusAziData,   remSusStrData,   aniSusDipData,
                        aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
                        aniSusAxis2Data, aniSusAxis3Data, outfile);
            	write_rocks(layerDimensions[z][0], layerDimensions[z][1],numLayers, xLoc, yLoc, zLoc, blockView->geophysicsCubeSize, outputFilename);
            }
            else if (!calcBlockLayer (blockLayer,
               layerDimensions[z][0], layerDimensions[z][1],
               xLoc, yLoc, zLoc, cubeSizes[z], geologyOptions.useTopography,
               numProps, stratArray, defRem, defAni, altZones, z, indexData,
               densityData,     magSusData,      remSusDecData,
               remSusAziData,   remSusStrData,   aniSusDipData,
               aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
               aniSusAxis2Data, aniSusAxis3Data))
            {
               memManagerFreeAfterLabel (memLabel);
               return (FALSE);
            }
                       
            zLoc = zLoc - cubeSizes[z]/2.0;

            destroy2DArray((char **) blockLayer, layerDimensions[z][0],
                                                 layerDimensions[z][1]);
         }
      }
   }
   
   if(calcType == TOPOLOGY)
   {
	   fclose(outfile);
   }
                                     /* ******************** */
                                     /* Write Out Block File */
   if (writeBlocks)
   {
      writeOutBlockModels (blockName, numLayers, layerDimensions,
               numCubeSizes, cubeSizes, blockView, options,
               densityCalc, susCalc, remCalc, aniCalc,
               indexCalc, numProps, stratArray, indexData,
               densityData, magSusData, remSusDecData, remSusAziData,
               remSusStrData, aniSusDipData, aniSusDDirData,
               aniSusPitchData, aniSusAxis1Data, aniSusAxis2Data,
               aniSusAxis3Data);
   }
   

                                     /* ****************************** */
                                     /* Calculate Gravity or Magnetics */
   if (calcMag || calcGrav)
   {  
      if (numCoarseLayers == 0) /* Just a straight calculation of the block */
      {
         createMagImage (outputFilename, blockName, numLayers,
               layerDimensions, numCubeSizes, cubeSizes, blockView,
               options, xyzCalc, pointPos, numPoints, xyzImportFile,
               densityCalc, susCalc, remCalc, aniCalc,
               indexCalc, numProps, stratArray, indexData,
               densityData, magSusData, remSusDecData, remSusAziData,
               remSusStrData, aniSusDipData, aniSusDDirData,
               aniSusPitchData, aniSusAxis1Data, aniSusAxis2Data,
               aniSusAxis3Data, magImage, grvImage);
      }
      else  /* Do images of the different block sizes and then add */
      {
         char fineFilename[100], coarseFilename[100];
         FILE_SPEC removeFile;
         int calculationAltitude;
         double altitude;

         xvt_fsys_get_default_dir (&(removeFile.dir));
         strcpy(fineFilename, "~fine.grv");
         strcpy(coarseFilename, "~coarse.grv");

         createMagImage (fineFilename, blockName,
               numFineLayers, &(layerDimensions[numCoarseLayers]),
               numFineLayers, &(cubeSizes[numCoarseLayers]), blockView,
               options, xyzCalc, pointPos, numPoints, xyzImportFile,
               densityCalc, susCalc, remCalc, aniCalc,
               indexCalc, numProps, stratArray,
               ((indexData)      ?(&(indexData[numCoarseLayers]))      :(nullShortData)),
               ((densityData)    ?(&(densityData[numCoarseLayers]))    :(nullData)),
               ((magSusData)     ?(&(magSusData[numCoarseLayers]))     :(nullData)),
               ((remSusDecData)  ?(&(remSusDecData[numCoarseLayers]))  :(nullData)),
               ((remSusAziData)  ?(&(remSusAziData[numCoarseLayers]))  :(nullData)),
               ((remSusStrData)  ?(&(remSusStrData[numCoarseLayers]))  :(nullData)),
               ((aniSusDipData)  ?(&(aniSusDipData[numCoarseLayers]))  :(nullData)),
               ((aniSusDDirData) ?(&(aniSusDDirData[numCoarseLayers])) :(nullData)),
               ((aniSusPitchData)?(&(aniSusPitchData[numCoarseLayers])):(nullData)),
               ((aniSusAxis1Data)?(&(aniSusAxis1Data[numCoarseLayers])):(nullData)),
               ((aniSusAxis2Data)?(&(aniSusAxis2Data[numCoarseLayers])):(nullData)),
               ((aniSusAxis3Data)?(&(aniSusAxis3Data[numCoarseLayers])):(nullData)),
               magImage, grvImage);
                           /* save them so we can put them back later */
         calculationAltitude = options->calculationAltitude;
         altitude = options->altitude;
                           /* Make sure this is the right height */
         if (options->calculationAltitude == AIRBORNE)
            options->altitude += options->constantBoxDepth;
         else
            options->altitude = options->constantBoxDepth;
         options->calculationAltitude = AIRBORNE;
         createMagImage (coarseFilename, blockName, numCoarseLayers,
               layerDimensions, numCoarseLayers, cubeSizes, blockView,
               options, xyzCalc, pointPos, numPoints, xyzImportFile,
               densityCalc, susCalc, remCalc, aniCalc,
               indexCalc, numProps, stratArray, indexData,
               densityData, magSusData, remSusDecData, remSusAziData,
               remSusStrData, aniSusDipData, aniSusDDirData,
               aniSusPitchData, aniSusAxis1Data, aniSusAxis2Data,
               aniSusAxis3Data, magImage, grvImage);
                           /* restore values */
         options->calculationAltitude = calculationAltitude;
         options->altitude = altitude;

         addFileExtention(outputFilename,".grv");
         addFileExtention(fineFilename,".grv");
         addFileExtention(coarseFilename,".grv");
         matchImageResolutions (outputFilename, fineFilename, coarseFilename);
         addFileExtention(outputFilename,".mag");
         addFileExtention(fineFilename,".mag");
         addFileExtention(coarseFilename,".mag");
         matchImageResolutions (outputFilename, fineFilename, coarseFilename);
         
                     /* Remove the tempory files we have used */
         addFileExtention(fineFilename,".grv");
         addFileExtention(coarseFilename,".grv");
         strcpy (removeFile.name, fineFilename);
         xvt_fsys_rem_file (&removeFile);
         strcpy (removeFile.name, coarseFilename);
         xvt_fsys_rem_file (&removeFile);

         addFileExtention(fineFilename,".mag");
         addFileExtention(coarseFilename,".mag");
         strcpy (removeFile.name, fineFilename);
         xvt_fsys_rem_file (&removeFile);
         strcpy (removeFile.name, coarseFilename);
         xvt_fsys_rem_file (&removeFile);
      }
   }

                               /* ***************************** */
                               /* Free Memory that we have used */
   if (readBlocks && indexCalc && stratArray[0])
      xvt_mem_free ((char *) stratArray[0]);
   freeBlockModels (numLayers, layerDimensions, &indexData,
                    &densityData,     &magSusData,      &remSusDecData, 
                    &remSusAziData,   &remSusStrData,   &aniSusDipData,
                    &aniSusDDirData,  &aniSusPitchData,
                    &aniSusAxis1Data, &aniSusAxis2Data, &aniSusAxis3Data);
   destroy2DArray ((char **) layerDimensions, numLayers, 2);
   if (cubeSizes) xvt_mem_free ((char *) cubeSizes);

//   memManagerFreeAfterLabel (memLabel);

   finishLongJob ();
   
   return (TRUE);
}



#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include "dxfvox.h"

#define DEBUG(X)    

extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;


#if XVT_CC_PROTO
extern void setIdentity(ftype I[4][4]);
extern int writeOutBlockModels (char *, int, int **, int, int *, BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *,
                int, int, int, int, int, int, LAYER_PROPERTIES **, short ***, float ***, float ***, float ***, 
                float ***, float ***, float ***, float ***, float ***, float ***, float ***, float ***);
extern int writeStandardAnomFile (char *, double **, int, int, int, int, BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *);
#else
extern void setIdentity();
extern int writeOutBlockModels ();
extern int writeStandardAnomFile ();
#endif

#if XVT_CC_PROTO
int convertDXFBlock (FILE_SPEC *, FILE_SPEC *, int, int, int, double ,double, double, double);
int convertAsciiImage (FILE_SPEC *, FILE_SPEC *, int, int, int, int, double, double, double, double, double, double, double);
#else
int convertDXFBlock ();
int convertAsciiImage ();
#endif



/* ======================================================================
FUNCTION        convertAsciiImage
DESCRIPTION
     Load in the binary image for reference

INPUT  

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
convertDXFBlock(FILE_SPEC *inFile, FILE_SPEC *outFile, int nx, int ny, int nz,
                double originX, double originY, double originZ, double blockSizeIn)
#else
convertDXFBlock(inFile, outFile, nx, ny, nz,
                originX, originY, originZ, blockSizeIn)
FILE_SPEC *inFile;
FILE_SPEC *outFile;
int nx, ny, nz;
double originX, originY, originZ, blockSizeIn;
#endif
{
   DXFHANDLE dxfData;
   DXFVOXEL voxelModel;
   ftype matrix[4][4];
   double blockSize;
   int dimensions[3];
   int result = TRUE;
   int volume = TRUE;

   if (blockSizeIn < 0.0)
   {
      xvt_dm_post_error ("Error, Block size less than 0.0");
      return (FALSE);
   }

   if (blockSizeIn < 1.0)
   {
      xvt_dm_post_error ("Warning, Block size less than 1.0 will be scaled to 100.0 for Noddy use.");
      blockSize = 100.0;
   }
   else
      blockSize = blockSizeIn;

   xvt_fsys_set_dir(&(inFile->dir));
   DXFread(&dxfData, inFile->name);
   if (dxfData.resultFlags)
      return (FALSE);  /* Errors have occured */

   initLongJob (0, 4, "Importing DXF Block...", NULL);
   
   if (volume)
      DXFvolOrient(&dxfData);
   setIdentity(matrix);

   incrementLongJob (INCREMENT_JOB);

   if (volume)
      result = DXFvolConv(&dxfData, matrix, FALSE);
   else
      result = DXFsurfConv(&dxfData, aWCS, matrix, TRUE);

   incrementLongJob (INCREMENT_JOB);

   if (result);
   {
      
      dimensions[0] = nx; dimensions[1] = ny; dimensions[2] = nz;


                                /* Init the voxel model */
      if (result = VOXinit (&voxelModel, dxfData.min, dxfData.max, dimensions))
      {
         if (volume)
         {
            if (result = VOXblankEdges (&dxfData, &voxelModel))
               result = VOXvolRegion(&dxfData, &voxelModel);
         }
         else
         {
            if (result = VOXblankEdges (&dxfData, &voxelModel))
               result = VOXsurfRegion(&dxfData, &voxelModel);
         }

         incrementLongJob (INCREMENT_JOB);

         if (result)
         {
            int x, y, z, numProps, maxIndex, minIndex, index;
            int **layerDimensions, *cubeSizes;
            short ***indexData;
            BLOCK_VIEW_OPTIONS blockView;
            GEOPHYSICS_OPTIONS geophOptions;
            LAYER_PROPERTIES **stratArray;
            
            blockView.originX = originX;
            blockView.originY = originY;
            blockView.originZ = originZ;
            blockView.lengthX = originX + nx*blockSize;
            blockView.lengthY = originY + ny*blockSize;
            blockView.lengthZ = originZ + nz*blockSize;
            blockView.geophysicsCubeSize = blockSize;
            blockView.geologyCubeSize = blockSize;
            memcpy (&geophOptions, &geophysicsOptions, sizeof (GEOPHYSICS_OPTIONS));
            geophOptions.calculationRange = 0;
            
            cubeSizes = (int *) create1DArray(nz, sizeof(int));
            if ((layerDimensions = (int **) create2DArray(nz, 2, sizeof(int)))
                && cubeSizes)
            {
               for (z = 0; z < dimensions[2]; z++)
               {
                  cubeSizes[z] = (int) floor (blockSize);
                  layerDimensions[z][0] = dimensions[0];
                  layerDimensions[z][1] = dimensions[1];
               }
               if (indexData = (short ***) create3DIregArray (nz, layerDimensions, sizeof(short)))
               {        
                  maxIndex = minIndex = 0;
                  
                  for (z = 0; z < dimensions[2]; z++)
                  {
                     for (y = 0; y < dimensions[1]; y++)
                     {
                        for (x = 0; x < dimensions[0]; x++)
                        {
                           index = (short) VOXget (&voxelModel, x, y, z);
                           indexData[z][x][y] = index;
                           if (index < minIndex)
                              minIndex = index;
                           if (index > maxIndex)
                              maxIndex = index;
                        }
                     }
                  }
                  
                  numProps = maxIndex - minIndex;
                  if (stratArray = (LAYER_PROPERTIES **) create1DArray(numProps, sizeof(LAYER_PROPERTIES *)))
                  {
                     for (z = 0; z < numProps; z++)
                     {
                        if (stratArray[z] = (LAYER_PROPERTIES *) create1DArray(1, sizeof(LAYER_PROPERTIES)))
                        {
                           sprintf(stratArray[z]->unitName, "Layer %d", z+1);
                           stratArray[z]->height = -31000;
                           stratArray[z]->density = z+1;
                           stratArray[z]->sus_X = stratArray[z]->sus_Y = stratArray[z]->sus_Z =
                              stratArray[z]->strength = 1.0e-03 + z*0.1e-03;
                           stratArray[z]->applyAlterations = TRUE;
                           stratArray[z]->anisotropicField = FALSE;
                           stratArray[z]->remanentMagnetization = FALSE;
                           stratArray[z]->inclination = 30.0;
                           stratArray[z]->angleWithNorth = 30.0;
                           stratArray[z]->sus_dip = 90.0;
                           stratArray[z]->sus_dipDirection = 90.0;
                           stratArray[z]->sus_pitch = 0.0;
                        }
                     }
                     xvt_fsys_set_dir(&(outFile->dir));
                     writeOutBlockModels (outFile->name, nz, layerDimensions,
                        nz, cubeSizes, &blockView, &geophOptions,
                        TRUE, TRUE, FALSE, FALSE,
                        TRUE, numProps, stratArray, indexData,
                        (float ***) NULL, (float ***) NULL, (float ***) NULL,
                        (float ***) NULL, (float ***) NULL, (float ***) NULL,
                        (float ***) NULL, (float ***) NULL, (float ***) NULL,
                        (float ***) NULL, (float ***) NULL);
                     for (z = 0; z < numProps; z++)
                        destroy1DArray ((char *) stratArray[z]);
                     destroy1DArray((char *) stratArray);
                  }
               }
            }
            destroy3DIregArray((char ***) indexData, nz, layerDimensions);
            destroy2DArray((char **) layerDimensions, nz, 2);
            destroy1DArray((char *) cubeSizes);
         }
         VOXfree (&voxelModel);
      }
   }

   finishLongJob ();
               
   DXFfree(&dxfData);

   return (result);
}

/* ======================================================================
FUNCTION        convertAsciiImage
DESCRIPTION
     Load in the binary image for reference

INPUT  

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
convertAsciiImage(FILE_SPEC *inFile, FILE_SPEC *outFile,
                  int type, int nx, int ny, int flipXY, double cellSize,
                  double minEasting, double minNorthing, double inclination,
                  double declination, double intensity, double altitude)
#else
convertAsciiImage(inFile, outFile, type, nx, ny, flipXY, cellSize, minEasting, minNorthing,
                  inclination, declination, intensity, altitude)
FILE_SPEC *inFile, *outFile;
int type, nx, ny, flipXY;
double cellSize, minEasting, minNorthing;
double inclination, declination, intensity, altitude;
#endif
{
   FILE *refFile;
   DOUBLE_2D_IMAGE *referenceImage = NULL;
   BLOCK_VIEW_OPTIONS blockView;
   GEOPHYSICS_OPTIONS geophOptions;
   int x, y;

   xvt_fsys_set_dir (&(inFile->dir));
   if (!(refFile = fopen(inFile->name, "r")))
      return (FALSE);

   if (referenceImage = (DOUBLE_2D_IMAGE *) xvt_mem_zalloc(sizeof(DOUBLE_2D_IMAGE)))
   {
      if (!(referenceImage->data = (double **) create2DArray(ny, nx, sizeof(double))))
      {
         fclose (refFile);
         return (FALSE);
      }
      referenceImage->dim1 = ny;
      referenceImage->dim2 = nx;
   }
   else
   {
      fclose (refFile);
      return (FALSE);
   }   

	if (flipXY)  /* Read the image one way or another */
	{
		for (x = 0; x < nx; x++)
		{
			for (y = 0; y < ny; y++)
			{
				if (fscanf(refFile,"%lf",&(referenceImage->data[y][x])) != 1)
				{
					fclose (refFile);
					xvt_dm_post_error ("Error, End of File reached too early. Adjust dimensions");
					destroy2DArray((char **) referenceImage->data, referenceImage->dim1, referenceImage->dim2);
					xvt_mem_free ((char *) referenceImage);
					return (FALSE);
				}
			}
		}
	}
	else
	{
		for (y = 0; y < ny; y++)
		{
			for (x = 0; x < nx; x++)
			{
				if (fscanf(refFile,"%lf",&(referenceImage->data[y][x])) != 1)
				{
					fclose (refFile);
					xvt_dm_post_error ("Error, End of File reached too early. Adjust dimensions");
					destroy2DArray((char **) referenceImage->data, referenceImage->dim1, referenceImage->dim2);
					xvt_mem_free ((char *) referenceImage);
					return (FALSE);
				}
			}
		}
	}
   fclose (refFile);   

   memcpy (&blockView, getViewOptions(), sizeof (BLOCK_VIEW_OPTIONS));
   blockView.originX = minEasting;
   blockView.originY = minNorthing;
   blockView.originZ = 0.0;
   blockView.lengthX = nx*cellSize;
   blockView.lengthY = ny*cellSize;
   blockView.lengthZ = 1000.0;
   blockView.geophysicsCubeSize = cellSize;
   
   memcpy (&geophOptions, &geophysicsOptions, sizeof (GEOPHYSICS_OPTIONS));
   geophOptions.calculationAltitude = AIRBORNE;
   geophOptions.altitude = altitude;
   geophOptions.inclination = inclination;
   geophOptions.declination = declination;
   geophOptions.intensity = intensity;
   geophOptions.calculationRange = 0;
   
   xvt_fsys_set_dir (&(outFile->dir));
   writeStandardAnomFile (outFile->name, referenceImage->data, type,
        referenceImage->dim2, referenceImage->dim1, 1, &blockView, &geophOptions);
         
   destroy2DArray((char **) referenceImage->data, referenceImage->dim1, referenceImage->dim2);
   xvt_mem_free ((char *) referenceImage);

   return (FALSE);
}


/* ======================================================================
FUNCTION        convertAsciiImage
DESCRIPTION
     Load in the binary image for reference

INPUT  

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
importColumnFormatImage (FILE_SPEC *inFile, FILE_SPEC *outFile,
								 double minX, double minY, double minZ,
								 double maxX, double maxY, double maxZ, double cubeSize,
							    double **pointPos, int numPoints)
#else
importColumnFormatImage (inFile, outFile, minX, minY, minZ,
								 maxX, maxY, maxZ, cubeSize, pointPos, numPoints)
FILE_SPEC *inFile, *outFile;
double minX, minY, minZ, maxX, maxY, maxZ, cubeSize;
double **pointPos;
int numPoints;
#endif
{
	DOUBLE_2D_IMAGE *referenceImage;
   BLOCK_VIEW_OPTIONS blockView;
   GEOPHYSICS_OPTIONS geophOptions;
	int nx, ny, x, y, point;
	int type = 1;

	nx = (int) floor(((maxX - minX)/cubeSize)) + 1;
	ny = (int) floor(((maxY - minY)/cubeSize)) + 1;

	/* /////////////////////////////////////////////*/
	/* Allocate memory										*/
   if (referenceImage = (DOUBLE_2D_IMAGE *) xvt_mem_zalloc(sizeof(DOUBLE_2D_IMAGE)))
   {
      if (!(referenceImage->data = (double **) create2DArray(ny, nx, sizeof(double))))
      {
         return (FALSE);
      }
      referenceImage->dim1 = ny;
      referenceImage->dim2 = nx;

      for (point = 0; point < numPoints; point++)
      {
			x = (int) floor((pointPos[point][0]-minX)/cubeSize);
			y = (int) floor((pointPos[point][1]-minY)/cubeSize);
			referenceImage->data[ny-y-1][x] = pointPos[point][2];
      }
   }
   else
   {
      return (FALSE);
   }   

	/* /////////////////////////////////////////////*/
	/* write out the file  */
   memcpy (&blockView, getViewOptions(), sizeof (BLOCK_VIEW_OPTIONS));
   blockView.originX = minX;
   blockView.originY = minY;
   blockView.originZ = 0.0;
   blockView.lengthX = nx*cubeSize;
   blockView.lengthY = ny*cubeSize;
   blockView.lengthZ = 1000.0;
   blockView.geophysicsCubeSize = cubeSize;

   memcpy (&geophOptions, &geophysicsOptions, sizeof (GEOPHYSICS_OPTIONS));
   geophOptions.calculationRange = 0;
   
   xvt_fsys_set_dir (&(outFile->dir));
   writeStandardAnomFile (outFile->name, referenceImage->data, type,
        referenceImage->dim2, referenceImage->dim1, 1, &blockView, &geophOptions);
         
   destroy2DArray((char **) referenceImage->data, referenceImage->dim1, referenceImage->dim2);
   xvt_mem_free ((char *) referenceImage);

	return (TRUE);
}
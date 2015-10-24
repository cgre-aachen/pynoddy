#include "xvt.h"
#include "noddy.h"                            
#include <math.h>
#include "titles.h"

#define DEBUG(X)

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;

                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
int writeGeosoftAnomImage (char *, double **, int, int, int, int, double, double, double, GEOPHYSICS_OPTIONS *, int, int);
WINDOW displayGeosoftImage(char *);
int readGeosoftHeader(char *, int *, int *, double *, double *, double *);
int readGeosoftGeophys(char *, double **, int, int, double *, double *);
#else
int writeGeosoftAnomImage ();
WINDOW displayGeosoftImage();
int readGeosoftHeader();
int readGeosoftGeophys();
#endif



/* *****************************************************************
FUNCTION writeGeosoftAnomFile

INPUT

OUTPUT

DESCRIPTION 
   write out both gravity and magnetics to an ERMapper format
   output file

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
writeGeosoftAnomImage (char *filename, double **imageData,
      int nx, int ny, int nz, int slaveSize, double cubeSize,
      double easting, double northing,
      GEOPHYSICS_OPTIONS *options, int magImage, int grvImage)
#else
writeGeosoftAnomImage (filename, imageData, nx, ny, nz, slaveSize,
                   cubeSize, easting, northing, options, magImage, grvImage)
char *filename;
double **imageData;
int nx, ny, nz, slaveSize;
double cubeSize, easting, northing;
GEOPHYSICS_OPTIONS *options;
int magImage, grvImage;
#endif
{
   FILE *fo;
   register int x, y;
   float magBuf = (float) 0.0;
   char dataFilename[100];
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   int lineLength;
   char valueStr[80];

   if (!filename)
      return (FALSE);

   strcpy (dataFilename, filename);
   
   addFileExtention (dataFilename, ".gxf");
   
   if (!(fo = (FILE *) fopen (dataFilename, "w")))
      return (FALSE);


   if (magImage)
      fprintf(fo,"#TITLE\nNoddy Magnetics Image\n");
   else if (grvImage)
      fprintf(fo,"#TITLE\nNoddy Gravity Image\n");
   fprintf(fo,"#POINTS\n%d\n", nx-slaveSize);
   fprintf(fo,"#ROWS\n%d\n", ny);
   fprintf(fo,"#PTSEPARATION\n%d\n", (int) floor(cubeSize));
   fprintf(fo,"#RWSEPARATION\n%d\n", (int) floor(cubeSize));
   fprintf(fo,"#XORIGIN\n%d\n", (int) (floor(blockView->originX)+floor(easting)));
   fprintf(fo,"#YORIGIN\n%d\n", (int) (floor(blockView->originY)+floor(northing)));
   fprintf(fo,"#SENSE\n-2\n");
   fprintf(fo,"#ROTATION\n0\n");

   fprintf(fo,"#GRID\n");
        
   for (y = ny-1; y >= 0; y--)
   {
      lineLength = 0;
      for (x = slaveSize; x < nx; x++)
      {
         if (imageData)
         {
            sprintf(valueStr, "%lf\t", imageData[x][y]);
            lineLength += strlen(valueStr);
            if (lineLength > 80)
            {         /* Keep lines under 80 characters long */
               fprintf (fo, "\n");
               lineLength = strlen(valueStr);
            }
            fprintf(fo, "%s", valueStr);
         }
            
      }
      fprintf(fo, "\n");
   }

   fclose (fo);

   return (TRUE);
}

WINDOW
#if XVT_CC_PROTO
displayGeosoftImage(char *filename)
#else
displayGeosoftImage(filename)
char *filename;
#endif
{
   WINDOW imageWindow;
   ANOMIMAGE_DATA *anomImageData;
   double **imageData;
   double minValue, maxValue;
   double interpolate = projectOptions.imageScalingFactor;
	double cellSize, originX, originY;
	int nx, ny;
   char title[100];
   RCT rect;

   if (!filename)
      return (NULL_WIN);

									    /* Read in the data */
   if (!(anomImageData = (ANOMIMAGE_DATA *) xvt_mem_zalloc (sizeof(ANOMIMAGE_DATA))))
   {
      xvt_dm_post_error ("Error, Not enough memory to display Image");
      return (NULL_WIN);
   }

                      /* check to see if this is an anomily file */
   if (!readGeosoftHeader (filename, &nx, &ny, &cellSize, &originX, &originY))
   {
      xvt_dm_post_error("This is not an Valid Geosoft File");
      xvt_mem_free ((char *) anomImageData);
      return (NULL_WIN);
   }

                       /* init image Display options */
   memcpy (&(anomImageData->imageDisplay),
           &(projectOptions.magneticsDisplay), sizeof (IMAGE_DISPLAY_DATA));

   if (!(imageData = (double **) create2DArray (ny, nx,
                                             sizeof (double))))
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      xvt_mem_free ((char *) anomImageData);
      return (NULL_WIN);
   }

   initLongJob (0, (int) (ny*2 + nx), "Loading Anomalies Image...", NULL);

   readGeosoftGeophys (filename, imageData, nx, ny, &maxValue, &minValue);

	strcpy(anomImageData->historyFile, "Geosoft Import");
	strcpy(anomImageData->dateTime, "unknown");

   rect.top = (short) (0 * interpolate + 30); rect.left = (short) (0 * interpolate + 30);
   rect.bottom= (short) ((ny-1) * interpolate + 30);
   rect.right = (short) ((nx-1) * interpolate + 30);
   sprintf (title, "%s - %s", IMAGE_TITLE, filename);
   if (!(imageWindow = xvt_win_create (W_DOC, &rect, title,
                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, anomImageEventHandler, (long) anomImageData)))
   {
      finishLongJob ();
      xvt_dm_post_error("Cannot Create Image Display Window.");
      destroy2DArray ((char **) imageData, ny, nx);
      xvt_mem_free ((char *) anomImageData);
      setStratPalet (imageWindow);
      
      return (NULL_WIN);
   }

   rect.bottom -= rect.top; rect.right -= rect.left;   /* put back to 0 */
   rect.top = 0; rect.left = 0;
                           
   anomImageData->imageData = imageData;
   anomImageData->dataXSize = nx;
   anomImageData->dataYSize = ny;
   anomImageData->minValue = minValue;
   anomImageData->maxValue = maxValue;
   if (maxValue == minValue)
      anomImageData->scale = 1.0;
   else
      anomImageData->scale = (double) 255.0 / (maxValue - minValue);
   anomImageData->geoXStart = originX;
   anomImageData->geoXEnd = originX + nx*cellSize;
   anomImageData->geoYStart = originY;
   anomImageData->geoYEnd = originY + ny*cellSize;
	anomImageData->geoHeight = 0.0;
	anomImageData->fileBlockTop = (double) 0.0;
	anomImageData->fileBlockBottom = (double) 0.0;
	anomImageData->fileCubeSize = (double) cellSize;
   anomImageData->dataType = MAGNETICS_DATA;
   anomImageData->inclination = 0.0;
   anomImageData->declination = 0.0;
   anomImageData->intensity = 0.0;
   if (anomImageData->imageDisplay.clippingType == RELATIVE_CLIPPING)
   {
      double percentInc;  /* convert percent values into real image values */
      
      percentInc = (anomImageData->maxValue - anomImageData->minValue)/100.0;

      anomImageData->imageDisplay.minClip = anomImageData->minValue
                             + percentInc*anomImageData->imageDisplay.minClip;
      anomImageData->imageDisplay.maxClip = anomImageData->minValue
                             + percentInc*anomImageData->imageDisplay.maxClip;
   }
                               /* create a PIXMAP from the raster data */
                               /* rect is the size of the data array */
   updateAnomaliesImageDisplay(imageWindow);
   
   xvt_vobj_set_data (imageWindow, (long) anomImageData);

   finishLongJob ();

   xvt_dwin_invalidate_rect (imageWindow, NULL);

   xvt_vobj_set_visible (imageWindow, TRUE);
   bringWindowToFront(imageWindow);
   
   return (imageWindow);
}

int
#if XVT_CC_PROTO
readGeosoftHeader(char *filename, int *nx, int *ny, double *cellSize, double *originX, double *originY)
#else
readGeosoftHeader(filename, nx, ny, cellSize, originX, originY)
char *filename;
int *nx, *ny;
double *cellSize, *originX, *originY;
#endif
{
#define FIELD_LENGTH 100
   FILE *fi;
	char value[FIELD_LENGTH];
	int error = FALSE;
	double xCellDim, yCellDim;

   if (!(fi = fopen (filename, "r")))
      return (FALSE);

	*originX = 0.0;
	*originY = 0.0;
	*cellSize = 100.0;

	if (readGeophysComment (fi, "#POINTS", value, FIELD_LENGTH-1, TRUE))
		fscanf(fi, "%d", nx);
	else
		error = TRUE;

	if (readGeophysComment (fi, "#ROWS", value, FIELD_LENGTH-1, TRUE))
		fscanf(fi, "%d", ny);
	else
		error = TRUE;

		         /* The optional material */
	if (readGeophysComment (fi, "#XORIGIN", value, FIELD_LENGTH-1, TRUE))
		fscanf(fi, "%lf", originX);
	if (readGeophysComment (fi, "#YORIGIN", value, FIELD_LENGTH-1, TRUE))
		fscanf(fi, "%lf", originY);

   xCellDim = yCellDim = *cellSize;
	if (readGeophysComment (fi, "#RWSEPARATION", value, FIELD_LENGTH-1, TRUE))
		fscanf(fi, "%lf", &yCellDim);
	if (readGeophysComment (fi, "#PTSEPARATION", value, FIELD_LENGTH-1, TRUE))
		fscanf(fi, "%lf", &xCellDim);
	*cellSize = MIN(xCellDim, yCellDim);
	
	if (error)
	{
		fclose(fi);
		return (FALSE);
	}
	else
	{
		fclose (fi);
	}

	return (!error);
}


int
#if XVT_CC_PROTO
readGeosoftGeophys(char *filename, double **imageData,
					    int nx, int ny, double *maxValue, double *minValue)
#else
readGeosoftGeophys(filename, imageData, nx, ny, maxValue, minValue)
char *filename;
double **imageData;
int nx, ny;
double *maxValue, *minValue;
#endif
{
	FILE *fi;
	int x, y, dummy;
   char dud[50], dummyValue[50], strValue[50];
	double value;

   if (!filename || !imageData)
      return (FALSE);

	if (!(fi = fopen (filename, "r")))
      return (FALSE);


	if (readGeophysComment (fi, "#DUMMY", dud, 49, TRUE))
	{
		fscanf(fi, "%s", (char *) dummyValue);
		dummy = TRUE;
	}
	else
		dummy = FALSE;

	if (readGeophysComment (fi, "#GRID", dud, 49, TRUE))
	{
		for (y = 0; y < ny; y++)
		{
			for (x = 0; x < nx; x++)
			{
				if (dummy)
				{
					fscanf (fi, "%s", strValue);
					if (strcmp(strValue, dummyValue) == 0)
						value = 0.0;
					else
						sscanf(strValue, "%lf", &value);
				}
				else
				{
					fscanf (fi, "%lf", &value);
				}

				imageData[y][x] = value;

				if (minValue && maxValue)  /* Work out min and max if we need it */
				{
					if ((x == 0) && (y == 0))
					{
						*minValue = value;
						*maxValue = value;
					}
					else
					{
						if (value < *minValue)
							*minValue = value;
						else if (value > *maxValue)
							*maxValue = value;
					}
				}
			}
		}
	}
	fclose (fi);

	return (TRUE);
}

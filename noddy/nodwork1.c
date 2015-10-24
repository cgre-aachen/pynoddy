/*
*/
#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include "scales.h"
#include "titles.h"

#ifndef FCN_NODDY_ONLY
#if XVTWS == MACWS
#include "Quickdraw.h"
#include "Memory.h"
#endif
#endif

#define DEBUG(X)    
#define DEG_TO_RAD(X)   (((double) (X))*0.01745329)
#define RAD_TO_DEG(X)   (((double) (X))*57.2957795)
#define TOLERANCE 0.001

extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;
extern ROCK_DATABASE rockDatabase;
extern int batchExecution;
extern COLOR backgroundColor;

WINDOW lastActiveWindow = NULL_WIN;
WINDOW currentPreviewWindow = NULL_WIN;

WINDOW blockDiagramWindow = NULL_WIN;
WINDOW boreHoleWindow = NULL_WIN;
WINDOW boreHole3DWindow = NULL_WIN;
WINDOW sectionWindow = NULL_WIN;
WINDOW lineMapWindow = NULL_WIN;
WINDOW lineMapPlotWindow = NULL_WIN;
WINDOW stratColumnsWindow = NULL_WIN;
WINDOW topoWindow = NULL_WIN;
WINDOW topo3dWindow = NULL_WIN;
WINDOW strat3dWindow = NULL_WIN;
WINDOW gravityReferenceWindow = NULL_WIN;
WINDOW magneticsReferenceWindow = NULL_WIN;

FILE_SPEC topoFileSpec;
int TopoRow, TopoCol; 
double TopomapXW, TopomapYW, TopomapXE, TopomapYE, minTopoValue, maxTopoValue;
double **topographyMap = NULL;
  /* flag to stop job status window from appearing during calcs */
int noStatusWin = FALSE;
ANOMIMAGE_DATA *gravityReferenceData = NULL, *magneticsReferenceData = NULL;
char errorMessage[ERROR_MESSAGE_LENGTH] = "";
int errorSet = FALSE;
                                /* Functions external to this file */
extern void xyToLineMapCoords();
extern void lineMapCoordsToXY();

#if XVT_CC_PROTO
extern COLOR *initBlockImageColorLut (int);
extern int writeAnomProfileFile (char *, double *,
               int, int, BLOCK_VIEW_OPTIONS *blockView, GEOPHYSICS_OPTIONS *);
extern int refreshContents (double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS], int, int, int);
extern int Load_status(char *);
extern int report_status (char *);
extern int getSpacedColorRGB (int, int, NODDY_COLOUR *);
extern int do3dPreview (THREED_IMAGE_DATA *, BLOCK_VIEW_OPTIONS *, OBJECT *, double, double, double, double, double, double, int);
extern void SaveOrientations(char *, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS], int);
extern int DrawStereoNet (WINDOW, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS], int);
extern void DoLineMap_Line (WINDOW);
extern void DoSection_Line (WINDOW);
extern void setIdentity(float [4][4]);
extern void multiplyMatrix( float [4][4], float [4][4], float [4][4] );
extern void translateBy(float [4][4], float, float, float);
extern void scaleBy(float[4][4], float, float, float);
extern long traceEventHandler (WINDOW win, EVENT *ep);
extern int coutourImageFromDoubleData (XVT_PIXMAP, double **, int, int, int, int, int, int, int,
              double *, int, double, int, int, COLOR *);
extern int writeGeosoftAnomImage (char *, double **, int, int, int, int, double,
              double, double, GEOPHYSICS_OPTIONS *, int, int);
extern int Vertical_Derivative(double **,int ,int ,int ,double **);
extern int writeStandardAnomFile (char *, double **, int , int, int, int, BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *);
extern int refreshSectionSymbols ();
extern int updateMovieTitle (WINDOW);
#else
extern COLOR *initBlockImageColorLut ();
extern int writeAnomProfileFile ();
extern int refreshContents ();
extern int Load_status();
extern int report_status ();
extern int getSpacedColorRGB ();
extern int do3dPreview ();
extern void SaveOrientations();
extern int DrawStereoNet ();
extern void DoLineMap_Line ();
extern void DoSection_Line ();
extern void setIdentity();
extern void multiplyMatrix();
extern void translate();
extern void scaleBy();
extern long traceEventHandler ();
extern int coutourImageFromDoubleData ();
extern int writeGeosoftAnomImage ();
extern int Vertical_Derivative();
extern int writeStandardAnomFile ();
extern int refreshSectionSymbols ();
extern int updateMovieTitle ();
#endif
                                /* Functions in this file */
#if XVT_CC_PROTO
void saveLineOfPoints (char *, int, double, double, double, double, double, double, double, double);
void saveProfileData (WINDOW, char *);
void calculateAnomalies (char *, int, int);
WINDOW displayAnomaliesImage (char *, ANOMIMAGE_DATA *);
int updateAnomaliesImageDisplay (WINDOW);
void loadContourImage (char *);
void loadProfileImage (char *);
int updateProfileImage (ANOMIMAGE_DATA *, PNT, PNT, double, double);
int convertToERMImage (char *);
int convertToGeosoftImage (char *);
int calcImageDifference (ANOMIMAGE_DATA *, ANOMIMAGE_DATA *, ANOMIMAGE_DATA *);
int calcVertDerivImage (char *, int);
ANOMIMAGE_DATA *loadAnomImage (char *);
int setReferenceImage (ANOMIMAGE_DATA *);
int loadTraceImage (OBJECT *, char *);
int convertTraceData (TRACE_DATA *);
void loadPictureFile (char *);
void savePictureFile (WINDOW, char *);
int savePixmap (XVT_PIXMAP, char *);
XVT_PIXMAP loadPixmap (char *);
XVT_PIXMAP getWindowPixmap (WINDOW);
void printWindow (WINDOW);
void loadHistoryFile (char *);
void saveHistoryFile (char *);
int getImportFile (FILE_SPEC *);
int loadImportFile (IMPORT_OPTIONS *);
int loadTopographyFile (FILE_SPEC *);
void updateTopographyMap ();
void update3dTopographyMap ();
void update3dStratigraphy ();
void update3dPreview (WINDOW, OBJECT *);
int load3dSurface (FILE_SPEC *, OBJECT *, DXFHANDLE *);
int view3dSurface (WINDOW, OBJECT *);
void saveOrientationFile (WINDOW, char *);
void plotOrientations (WINDOW);
void updateStratColumns ();
void updateLineMap (int);
void invalidateCurrentLineMap ();
void updateSection (int, BLOCK_SURFACE_DATA *);
void updateWellLog ();
int importBoreholePoints (FILE_SPEC *, int);
double **generateBoreholePoints(double, double, double, double, double, double, double, int *);
int updateBorehole (double **, int, int);
void updateBlockDiagram (WINDOW);
void createMovie (FILE_SPEC *);
int loadMovie (FILE_SPEC *);
BOOLEAN winHasTaskMenu (WINDOW);
int newRockInDatabase (WINDOW);
int deleteRockInDatabase (WINDOW);
int editRockInDatabase (WINDOW);
int updateRocksInDatabase (WINDOW);
int initBlockDiagramData (WINDOW, BLOCK_DIAGRAM_DATA *, double);
int freeSurfaceData (BLOCK_SURFACE_DATA *);
BLOCK_DIAGRAM_DATA *freeBlockDiagramData (BLOCK_DIAGRAM_DATA *, int);
int addChairDiagramData (BLOCK_DIAGRAM_DATA *, double, double, double);
int addEmptyRectData (BLOCK_DIAGRAM_DATA *, double, double, double,
                                            double, double, double);
#else
void saveLineOfPoints ();
void saveProfileData ();
void calculateAnomalies ();
WINDOW displayAnomaliesImage ();
int updateAnomaliesImageDisplay ();
void loadContourImage ();
void loadProfileImage ();
int updateProfileImage ();
int convertToERMImage ();
int convertToGeosoftImage ();
int calcImageDifference ();
int calcVertDerivImage ();
ANOMIMAGE_DATA *loadAnomImage ();
int setReferenceImage ();
int loadTraceImage ();
int convertTraceData ();
void loadPictureFile ();
void savePictureFile ();
int savePixmap ();
XVT_PIXMAP loadPixmap ();
XVT_PIXMAP getWindowPixmap ();
void printWindow ();
void loadHistoryFile ();
void saveHistoryFile ();
int getImportFile ();
int loadImportFile ();
int loadTopographyFile ();
void updateTopographyMap ();
void update3dTopographyMap ();
void update3dStratigraphy ();
void update3dPreview ();
int load3dSurface ();
int view3dSurface ();
void saveOrientationFile ();
void plotOrientations ();
void updateStratColumns ();
void updateLineMap ();
void invalidateCurrentLineMap ();
void updateSection ();
void updateWellLog ();
int importBoreholePoints ();
double **generateBoreholePoints ();
int updateBorehole ();
void updateBlockDiagram ();
void createMovie ();
int loadMovie ();
BOOLEAN winHasTaskMenu ();
int newRockInDatabase ();
int deleteRockInDatabase ();
int editRockInDatabase ();
int updateRocksInDatabase ();
int initBlockDiagramData ();
int freeSurfaceData ();
BLOCK_DIAGRAM_DATA *freeBlockDiagramData ();
int addChairDiagramData ();
int addEmptyRectData ();
#endif


/* ======================================================================
FUNCTION        saveLineOfPoints
DESCRIPTION
     save a line of points (like a block but along a line)

INPUT 
     char *filename;     the output file for the line

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveLineOfPoints (char *filename, int fileType, double xPos, double yPos, double zPos,
                  double declination, double lineLength, double lineHeight,
                  double lineSpacing, double sampleSpacing)
#else
saveLineOfPoints (filename, fileType, xPos, yPos, zPos, declination, lineLength,
                                  lineHeight, lineSpacing, sampleSpacing)
int fileType;
char *filename;
double xPos, yPos, zPos, declination, lineLength, lineHeight, lineSpacing, sampleSpacing;
#endif
{
   int numPoints, point, pointOffset, pointIndex;
   int numLines, pointsPerLine, line;
   double angle, xInc, yInc, lineXInc, lineYInc;
   double **pointPos, pointX, pointY, pointZ;
   double minX, minY, minZ, maxX, maxY, maxZ;
   BLOCK_VIEW_OPTIONS view, *blockView = getViewOptions ();
   
   numLines = (int) (lineHeight / lineSpacing) + 1;
   pointsPerLine = (int) (lineLength/sampleSpacing) + 1;
   numPoints = pointsPerLine*numLines;
   if (!(pointPos = (double **) create2DArray (numPoints, 3, sizeof(double))))
   {
      //xvt_dm_post_error ("Error, Not enough memory to Generate Line");
      return;
   }
          /* Increment Needed To follow declination of line */
          /*
                     |             Angle x (the direction of the line) is
              2      |       1     found for each quater (x always being off
               \   y |     /|      the horiz-axis) y (at 90deg to the line
                 \  /|\  /  |      defined by x always has the same angle
                   \ | / x  |      as x. Line 1 give the increments for
            ---------+----------   line of sampling and Line 2 give the 
                     |             increment needed to get from 1 sample
                     |             line to the next.
          */
   if (declination < 0.0) declination = 360 + declination;
   if (declination > 360.0) declination -= 360;
   if (declination <= 90.0)
   {
      angle = DEG_TO_RAD(90.0-declination);
      xInc = cos(angle) * sampleSpacing;
      yInc = sin(angle) * sampleSpacing;
      lineXInc = -(sin(angle) * lineSpacing);
      lineYInc = cos(angle) * lineSpacing;
   }                         
   else if (declination <= 180.0)
   {
      angle = DEG_TO_RAD(declination-90.0);
      xInc = cos(angle) * sampleSpacing;
      yInc = -(sin(angle) * sampleSpacing);
      lineXInc = sin(angle) * lineSpacing;
      lineYInc = cos(angle) * lineSpacing;
   }
   else if (declination <= 270.0)
   {
      angle = DEG_TO_RAD(270.0-declination);
      xInc = -(cos(angle) * sampleSpacing);
      yInc = -(sin(angle) * sampleSpacing);
      lineXInc = -(sin(angle) * lineSpacing);
      lineYInc = cos(angle) * lineSpacing;
   }
   else
   {
      angle = DEG_TO_RAD(declination-270.0);
      xInc = -(cos(angle) * sampleSpacing);
      yInc = sin(angle) * sampleSpacing;
      lineXInc = sin(angle) * lineSpacing;
      lineYInc = cos(angle) * lineSpacing;
   }   
   
   pointX = xPos - xInc;  /* -xInc  and -yInc so that when it is first added */
   pointY = yPos - yInc;  /*  on it will be back in the start position */
   pointZ = zPos;
   minX = maxX = pointX + xInc;
   minY = maxY = pointY + yInc;
   minZ = maxZ = pointZ;
   for (line = 0; line < numLines;  line++)
   {
      pointOffset = line*pointsPerLine;
      for (point = 0; point < pointsPerLine; point++)
      {
         pointX += xInc;
         pointY += yInc;

         pointIndex = pointOffset + point;
         pointPos[pointIndex][0] = pointX;
         pointPos[pointIndex][1] = pointY;
         pointPos[pointIndex][2] = pointZ;
         
         if (pointX < minX) minX = pointX;
         if (pointY < minY) minY = pointY;
         if (pointZ < minZ) minZ = pointZ;
         if (pointX > maxX) maxX = pointX;
         if (pointY > maxY) maxY = pointY;
         if (pointZ > maxZ) maxZ = pointZ;
         
      }
      pointX += lineXInc + xInc;
      pointY += lineYInc + yInc;
      xInc = -xInc;   /* go up and down each line in  opposite directions */
      yInc = -yInc;   /* as a plane would fly */
   }
   
   if (fileType == 0)  /* Geological */
      writeLineThroughBlock (filename, pointPos, numPoints);
   else
   {
      GEOPHYSICS_OPTIONS options;

      memcpy(&view, blockView, sizeof(BLOCK_VIEW_OPTIONS));
      memcpy (&options, &geophysicsOptions, sizeof(GEOPHYSICS_OPTIONS));

              /* make sure the block covers the xyz locations */
      view.originX = minX - view.geophysicsCubeSize*2;
      view.originY = minY - view.geophysicsCubeSize*2;
      /* view.originZ = maxZ + view.geophysicsCubeSize*2; */
      view.lengthX = (maxX - minX) + view.geophysicsCubeSize*4;
      view.lengthY = (maxY - minY) + view.geophysicsCubeSize*4;
      /* view.lengthZ = (maxZ - minZ) + view.geophysicsCubeSize*4; */
      
      if (options.calculationMethod == SPATIAL) /* Only spatial cannot do xyz calcs (onlyt Spectral/Full) */
		{
			//xvt_dm_post_error("You were trying to calculate with a Spatial convolution. Only Spectral and Full Spatial method support this operation. You are now using a Spectral Calculation Method.");
		   options.calculationMethod = SPECTRAL; /* Default to spectral as it is quicker */ 
		}
      doGeophysics (XYZ_ANOM, &view, &options, filename, NULL, pointPos, numPoints, NULL, NULL, NULL);
      finishLongJob ();
   }

   destroy2DArray ((char **) pointPos, numPoints, 3);
}

/* ======================================================================
FUNCTION        saveProfileData
DESCRIPTION
     save the interactively created profile to a file

INPUT 
     char *filename;     the output file for the Profile

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveProfileData (WINDOW win, char *filename)
#else
saveProfileData (win, filename)
WINDOW win;
char *filename;
#endif
{
   char profileFile[100];
   char title[100];
   double *profileData;
   int numPoints, point;
   GEOPHYSICS_OPTIONS options;
   ANOMIMAGE_DATA *anomImageData;
   double rangeX, rangeY;
   BLOCK_VIEW_OPTIONS blockView;

   if (!win)
   {
      //xvt_dm_post_error ("Error, No Window Selected");
      return;
   }
   xvt_vobj_get_title (win, title, 100);
   if (!strstr(title, PROFILE_FROM_IMAGE_TITLE))
   {
      //xvt_dm_post_error ("Error, Profiles can only be saved from Profile Windows.");
      return;
   }

   if (!(anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win)))
   {
      //xvt_dm_post_error ("Error, Invalid Window. Try another Window.");
      return;
   }
   
   if (!filename)  /* get the file name if we dont have one */
   {
      FILE_SPEC fileSpec;
      
      if (anomImageData->dataType == GRAVITY_DATA)
      {
         strcpy (fileSpec.name, "untitled.grv");
         strcpy (fileSpec.type, "grv");
      }
      else
      {
         strcpy (fileSpec.name, "untitled.mag");
         strcpy (fileSpec.type, "mag");
      }
#if (XVTWS == MACWS)
         strcpy (fileSpec.type, "TEXT");
#endif

      getDefaultDirectory(&(fileSpec.dir));
      switch (xvt_dm_post_file_save(&fileSpec, "Save Profile As ..."))
      {
         case FL_OK:
            if (strlen(fileSpec.name) == 0)
            {
               //xvt_dm_post_error ("Error, No file Specified");
               return;
               break;
            }
            setDefaultDirectory(&(fileSpec.dir));
            break;
         case FL_BAD:
            //xvt_dm_post_error("Error getting file name.");
            return;
            break;
         case FL_CANCEL:
            return;
            break;
      }             

      xvt_fsys_set_dir(&fileSpec.dir);
      strcpy (profileFile, fileSpec.name);
   }
   else
      strcpy (profileFile, filename);
   
   numPoints = anomImageData->dataXSize;
   
   options.calculationRange = 0;
   options.inclination = anomImageData->inclination;
   options.declination = anomImageData->declination;
   options.intensity = anomImageData->intensity;

   blockView.originX = anomImageData->geoXStart;
   blockView.originY = anomImageData->geoYStart;
   blockView.originZ = anomImageData->geoHeight;
   blockView.lengthX = anomImageData->geoXEnd - anomImageData->geoXStart;
   blockView.lengthY = anomImageData->geoYEnd - anomImageData->geoYStart;
   blockView.lengthZ = 0.0;
   
   rangeX = blockView.lengthX;
   rangeY = blockView.lengthY;
   blockView.geologyCubeSize = blockView.geophysicsCubeSize = (sqrt(rangeX*rangeX + rangeY*rangeY)/numPoints);
   
   profileData = (double *) xvt_mem_zalloc (numPoints*sizeof(double));
                  /* need to skip first point as it is zero */
   for (point = 0; point < numPoints; point++)
      profileData[point] = (double) anomImageData->imageData[0][point];
/*
      profileData[point] = (double) (((double) anomImageData->imageData[0][point])
                                         / anomImageData->scale
                                         + anomImageData->minValue);
*/
   
   if (!writeAnomProfileFile (profileFile, profileData,
                  anomImageData->dataType, numPoints, &blockView, &options))
   {
      //xvt_dm_post_error ("Error, Profile not written.");
   }

   xvt_mem_free ((char *) profileData);
}

/* ======================================================================
FUNCTION        calculateAnomalies
DESCRIPTION
     read in a histroy file

INPUT 
     char *filename;     the output file for the anomily
     int type; which type of anomilty is being calculated
               ANOM
               ANOM_FROM_BLOCK
               BLOCK_AND_ANOM
               BLOCK_ONLY

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
calculateAnomalies (char *filename, int type, int loadResult)
#else
calculateAnomalies (filename, type, loadResult)
char *filename;
int type, loadResult;
#endif
{
   static char *defaultBlockName = "tempblk.g00";
   static FILE_SPEC blockFileSpec;
   char *blockName = NULL;
   
   blockName = defaultBlockName;

   if (type == ANOM_FROM_BLOCK)
   {
#if (XVTWS == MACWS)
      strcpy (blockFileSpec.type, "TEXT");
#else
      strcpy (blockFileSpec.type, "g00");
#endif
      getDefaultDirectory(&(blockFileSpec.dir));
      if (xvt_dm_post_file_open(&blockFileSpec, "Block To Calculate From ...") != FL_OK)
         return;

      setDefaultDirectory(&(blockFileSpec.dir));

      blockName = blockFileSpec.name;
   }

                   /* write the block file with same name */
   if ((type == BLOCK_AND_ANOM) || (type == BLOCK_ONLY))
      blockName = filename;
                                                 
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
   if (projectOptions.calculationType == REMOTE_JOB)
   {
      REMOTE_CMD_STATUS errorStatus;
      char inputFiles[100], outputFiles[100], remoteCommand[100];
      char outputBaseName[50], *ptr;

                   /* get the base name to use as output files */
      strcpy (outputBaseName, filename);
      if (ptr = strchr(outputBaseName,'.'))
         *ptr = '\0';

      sprintf(inputFiles, "/tmp/temp_%s.his",projectOptions.accountName);
      report_status (inputFiles);  /* save the current history */

      switch (type)
      {
         case (ANOM_FROM_BLOCK):
            strcat (inputFiles, ":");
            strcat (inputFiles, blockName);
            sprintf(outputFiles,"%s.grv:%s.mag",outputBaseName,outputBaseName);
            sprintf(remoteCommand, "%s %s -o %s -anomFromBlock -b %s",
               projectOptions.noddyPath, inputFiles, outputBaseName, blockName);
            break;
         case (BLOCK_AND_ANOM):
            sprintf(outputFiles,"%s.grv:%s.mag:%s.blk",outputBaseName,
                                   outputBaseName, outputBaseName);
            sprintf(remoteCommand, "%s %s -o %s -anomAndBlock -b %s.blk",
               projectOptions.noddyPath, inputFiles, outputBaseName,
                                                                outputBaseName);
            break;
         case (BLOCK_ONLY):
            strcpy (outputFiles, filename); /* overwrite .mag and .grv files */
            sprintf(remoteCommand, "%s %s -o %s -block",
                       projectOptions.noddyPath, inputFiles, outputFiles);
            break;
         default:
            sprintf(outputFiles,"%s.grv:%s.mag",outputBaseName,outputBaseName);
            sprintf(remoteCommand, "%s %s -o %s",
                       projectOptions.noddyPath, inputFiles, outputBaseName);
      }

      if (!runRemoteCommand (inputFiles, outputFiles,
                             projectOptions.internetAddress,
                             projectOptions.accountName,
                             remoteCommand, &errorStatus))
      {
         //xvt_dm_post_error("Error, Remote Command Failed");
      }
   }
   else
#endif   
   {
      int calcRangeStore, padStore = TRUE;
      
      if (geophysicsOptions.calculationMethod == SPECTRAL)
      {                              /* So we dont get the 6 pixel border */
         calcRangeStore = geophysicsOptions.calculationRange;
         geophysicsOptions.calculationRange = 0;
      }
      if ((geophysicsOptions.calculationMethod == SPATIAL_FULL) && (!geophysicsOptions.padWithRealGeology))
      {                              /* So we dont get the 6 pixel border */
         calcRangeStore = geophysicsOptions.calculationRange;
			padStore = geophysicsOptions.padWithRealGeology;
			if (calcRangeStore > 0)
			{
				geophysicsOptions.padWithRealGeology = geophysicsOptions.calculationRange;
				geophysicsOptions.calculationRange = 0;
			}
      }

      doGeophysics (type, getViewOptions(), &geophysicsOptions,
                    filename, blockName, NULL, 0, NULL, NULL, NULL);

      if ((geophysicsOptions.calculationMethod == SPATIAL_FULL) && !padStore)
      {                              /* So we dont get the 6 pixel border */
			if (calcRangeStore > 0)
			{
				geophysicsOptions.calculationRange = calcRangeStore;
				geophysicsOptions.padWithRealGeology = padStore;
			}
      }
      if (geophysicsOptions.calculationMethod == SPECTRAL)
      {
         geophysicsOptions.calculationRange = calcRangeStore;
      }
   }

   finishLongJob();

                    /* Display the result of the calc */
   if ((type == ANOM)  || (type == ANOM_FROM_BLOCK)
                       || (type == BLOCK_AND_ANOM))
   {               /* load images if the calc was not aborted */
      if (!abortLongJob () && loadResult)
      {
         if (geophysicsOptions.magneticVectorComponents)
         {
            addFileExtention (filename, ".mgx");
            displayAnomaliesImage (filename, NULL);

            addFileExtention (filename, ".mgy");
            displayAnomaliesImage (filename, NULL);

            addFileExtention (filename, ".mgz");
            displayAnomaliesImage (filename, NULL);
         }
         else
         {
            addFileExtention (filename, MAGNETICS_FILE_EXT);
            displayAnomaliesImage (filename, NULL);
      
            addFileExtention (filename, GRAVITY_FILE_EXT);
            displayAnomaliesImage (filename, NULL);
         }
      }
   }
}

/* ======================================================================
FUNCTION        displayAnomaliesImage
DESCRIPTION
     read in an anomilies Image

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
WINDOW
#if XVT_CC_PROTO
displayAnomaliesImage (char *filename, ANOMIMAGE_DATA *anomaliesImageData)
#else
displayAnomaliesImage (filename, anomaliesImageData)
char *filename;
ANOMIMAGE_DATA *anomaliesImageData;
#endif
{
   FILE *fi;
   WINDOW imageWindow;
   ANOMIMAGE_DATA *anomImageData;
   double **imageData;
   double minValue, maxValue;
   int fileId, csize, xmax, ymax, zmax, sizeX, sizeY;
   float dat[3], position[6], grid, airgap;
   double interpolate = projectOptions.imageScalingFactor;
   int binary;
   char title[100];
   RCT rect;

   if (!filename && !anomaliesImageData)
      return (NULL_WIN);

   if (!anomaliesImageData)    /* Read in the data */
   {
      if (!(anomImageData = (ANOMIMAGE_DATA *) xvt_mem_zalloc (sizeof(ANOMIMAGE_DATA))))
      {
         //xvt_dm_post_error ("Error, Not enough memory to display Image");
         return (NULL_WIN);
      }

      if (!(fi = fopen (filename, "r")))
      {
         //xvt_dm_post_error("Could not open image to read.");
         xvt_mem_free ((char *) anomImageData);
         return (NULL_WIN);
      }
                         /* check to see if this is an anomily file */
      if ((!readGeophysHeader (fi, &fileId, &csize, &xmax, &ymax, &zmax,
                dat, position, &grid, &airgap, &sizeX, &sizeY, &binary))
           || ((fileId != 444) && (fileId != 333)))
      {
         fclose (fi);
         //xvt_dm_post_error("This is not an Anomaly Data File");
         xvt_mem_free ((char *) anomImageData);
         return (NULL_WIN);
      }
   
                          /* init image Display options */
      if (fileId == 444)  /* Gravity */
         memcpy (&(anomImageData->imageDisplay),
                 &(projectOptions.gravityDisplay), sizeof (IMAGE_DISPLAY_DATA));
      else  /* 333 - Magnetics */
         memcpy (&(anomImageData->imageDisplay),
                 &(projectOptions.magneticsDisplay), sizeof (IMAGE_DISPLAY_DATA));
   
      if (!(imageData = (double **) create2DArray (sizeY, sizeX,
                                                sizeof (double))))
      {
         fclose (fi);
         //xvt_dm_post_error("Not enough memory, try closing some windows");
         xvt_mem_free ((char *) anomImageData);
         return (NULL_WIN);
      }
   
      initLongJob (0, (int) (sizeY*2 + sizeY), "Loading Anomalies Image...", NULL);
   
      readGeophys (fi, (unsigned char **) NULL, (double **) imageData, sizeX, sizeY,
                              &maxValue, &minValue, FALSE, binary);

		if (!readGeophysComment(fi, ANOM_HISTORY_TAG, anomImageData->historyFile, SZ_FNAME, TRUE))
			strcpy(anomImageData->historyFile, "unknown");
		if (!readGeophysComment(fi, ANOM_DATETIME_TAG, anomImageData->dateTime, 30, TRUE))
			strcpy(anomImageData->dateTime, "unknown");

      fclose (fi);
   }
   else
   {
      anomImageData = anomaliesImageData;
      sizeX = anomImageData->dataXSize;
      sizeY = anomImageData->dataYSize;
   }

   rect.top = (short) (0 * interpolate + 30); rect.left = (short) (0 * interpolate + 30);
   rect.bottom= (short) ((sizeY-1) * interpolate + 30);
   rect.right = (short) ((sizeX-1) * interpolate + 30);
   sprintf (title, "%s - %s", IMAGE_TITLE, filename);
   if (!(imageWindow = xvt_win_create (W_DOC, &rect, title,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, anomImageEventHandler, (long) anomImageData)))
   {
      finishLongJob ();
      //xvt_dm_post_error("Cannot Create Image Display Window.");
      if (!anomaliesImageData)
      {
         destroy2DArray ((char **) imageData, sizeY, sizeX);
         xvt_mem_free ((char *) anomImageData);
      }
      setStratPalet (imageWindow);
      
      return (NULL_WIN);
   }

   rect.bottom -= rect.top; rect.right -= rect.left;   /* put back to 0 */
   rect.top = 0; rect.left = 0;
                           
   if (!anomaliesImageData)           /* Fill in structure data */
   {
      anomImageData->imageData = imageData;
      anomImageData->dataXSize = sizeX;
      anomImageData->dataYSize = sizeY;
      anomImageData->minValue = minValue;
      anomImageData->maxValue = maxValue;
      if (maxValue == minValue)
         anomImageData->scale = 1.0;
      else
         anomImageData->scale = (double) 255.0 / (maxValue - minValue);
      anomImageData->geoXStart = MIN(position[0], position[3]);
      anomImageData->geoXEnd = MAX(position[0], position[3]);
      anomImageData->geoYStart = MIN(position[1], position[4]);
      anomImageData->geoYEnd = MAX(position[1], position[4]);
		anomImageData->geoHeight = airgap;
		anomImageData->fileBlockTop = (double) position[5];
		anomImageData->fileBlockBottom = (double) position[2];
		anomImageData->fileCubeSize = (double) grid;
      if (fileId == 333)
         anomImageData->dataType = MAGNETICS_DATA;
      else
         anomImageData->dataType = GRAVITY_DATA;
      anomImageData->inclination = dat[0];
      anomImageData->declination = dat[1];
      anomImageData->intensity = dat[2];
      if (anomImageData->imageDisplay.clippingType == RELATIVE_CLIPPING)
      {
         double percentInc;  /* convert percent values into real image values */
         
         percentInc = (anomImageData->maxValue - anomImageData->minValue)/100.0;
   
         anomImageData->imageDisplay.minClip = anomImageData->minValue
                                + percentInc*anomImageData->imageDisplay.minClip;
         anomImageData->imageDisplay.maxClip = anomImageData->minValue
                                + percentInc*anomImageData->imageDisplay.maxClip;
      }
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

/* ======================================================================
FUNCTION        updateAnomaliesImageDisplay
DESCRIPTION
     create a pixmap to reflect the display paramaters

INPUT  WINDOW win - that contains the anom image

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateAnomaliesImageDisplay (WINDOW win)
#else
updateAnomaliesImageDisplay (win)
WINDOW win;
#endif
{
   XVT_PIXMAP pixmap = NULL_PIXMAP;
   NODDY_COLOUR lut[260];
   double interpolate = projectOptions.imageScalingFactor;
   ANOMIMAGE_DATA *anomImageData;
   double range, maxValue, minValue;
   double clipRange, clipMaxValue, clipMinValue;
   double imageRange, imageMaxValue, imageMinValue, imageIncrement;
   COLOR layerColors[100], *clut;
   double contourLevel[100], *contourLevelPtr;
   int numContour, i, increment, fifthContour, colorInc, sucess = FALSE;
   RCT rect;
	EVENT ep;
   
   if (!win)
      return (FALSE);
      
   if (!(anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win)))
      return (FALSE);

	contourLevelPtr = &(contourLevel[0]);

   imageMinValue = anomImageData->minValue;
   imageMaxValue = anomImageData->maxValue;
   imageRange = imageMaxValue - imageMinValue;

   clipMinValue = anomImageData->imageDisplay.minClip;
   clipMaxValue = anomImageData->imageDisplay.maxClip;
   clipRange = clipMaxValue - clipMinValue;

	anomImageData->scalingAtCreation = interpolate;

   if (imageRange < TOLERANCE)
   {
      if (!(pixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                                   (short) (anomImageData->dataXSize*interpolate),
                                   (short) (anomImageData->dataYSize*interpolate), NULL)))
      {
         //xvt_dm_post_error("Cannot Create Raster Image.");
         return (FALSE);
      }
      xvt_dwin_clear (pixmap, COLOR_BLACK);  
   }
   else
   {
      if ((clipMinValue < imageMinValue) || (clipMaxValue > imageMaxValue))
      {     /* Clipping extends outside the image data we have */
         imageIncrement = clipRange/256.0;
         
         minValue = 0;   /* (clipMinValue - imageMinValue)/imageIncrement; */
         maxValue = 255; /* (clipMaxValue - imageMinValue)/imageIncrement; */
         range = maxValue - minValue;
      }
      else  /* Clipping is inside the image Data */
      {
         imageIncrement = imageRange/256.0;
         
         minValue = (clipMinValue - imageMinValue)/imageIncrement;
         maxValue = (clipMaxValue - imageMinValue)/imageIncrement;
         range = maxValue - minValue;
      }
       
      switch (anomImageData->imageDisplay.display)
      {
         case (PSEUDO_COLOR):
				if (anomImageData->imageDisplay.lut)  /* Keep a copy of the lut */
				{
					xvt_mem_free((char *) anomImageData->imageDisplay.lut);
					anomImageData->imageDisplay.lutSize = 0;
				}
				if (clut = (COLOR *) xvt_mem_zalloc(256*sizeof(COLOR)))
					anomImageData->imageDisplay.lutSize = 256;
            assignPseudoLut(lut, clut, 256, 36, (int) minValue, (int) maxValue, 0);
				anomImageData->imageDisplay.lut = clut;
            xvt_rect_set(&rect, (short) 0, (short) 0, (short) anomImageData->dataXSize,
																		(short) anomImageData->dataYSize);
            if (!(pixmap = createPixmapFromDoubleData(win, anomImageData->imageData,
                                 clipMinValue, clipMaxValue, rect, lut, interpolate)))
               return (FALSE);
            setStratPalet (pixmap);
            break;
         case (GREY_COLOR):
				if (anomImageData->imageDisplay.lut)  /* Keep a copy of the lut */
				{
					xvt_mem_free((char *) anomImageData->imageDisplay.lut);
					anomImageData->imageDisplay.lutSize = 0;
				}
				if (clut = (COLOR *) xvt_mem_zalloc(256*sizeof(COLOR)))
					anomImageData->imageDisplay.lutSize = 256;
            assignGreyLut(lut, clut, 256, 36, (int) minValue, (int) maxValue);
				anomImageData->imageDisplay.lut = clut;
            xvt_rect_set(&rect, (short) 0, (short) 0, (short) anomImageData->dataXSize,
																		(short) anomImageData->dataYSize);
            if (!(pixmap = createPixmapFromDoubleData(win, anomImageData->imageData,
                                 clipMinValue, clipMaxValue, rect, lut, interpolate)))
               return (FALSE);
            setStratPalet (pixmap);
            break;
         case (CONTOUR_IMAGE):
                     /* create the PIXMAP to describe the contour
                     ** and attach it to window we created as app_data */
            numContour = anomImageData->imageDisplay.numContours;
   /*
            minValue = anomImageData->imageDisplay.minClip/100.0*256;
            maxValue = anomImageData->imageDisplay.maxClip/100.0*256;
            range = maxValue - minValue;
   */
            fifthContour = numContour / 5;
            colorInc = 255/fifthContour;
            for(i = 0; i < numContour; i++)
            {
               contourLevel[i] = clipMinValue + ((i+1) * (clipRange/(numContour+1.0)));
         
                              /* Setup the colors to draw the contours */
               if (i < fifthContour*1)
               {
                  increment = fifthContour - (i+1);
                  layerColors[i] = XVT_MAKE_COLOR(colorInc*increment, 0, 255);
               }
               else if (i < fifthContour*2)
               {
                  increment = (i+1) - fifthContour;
                  layerColors[i] = XVT_MAKE_COLOR(0, colorInc*increment, 255);
               }
               else if (i < fifthContour*3)
               {
                  increment = fifthContour*3 - (i+1);
                  layerColors[i] = XVT_MAKE_COLOR(0, 255, colorInc*increment);
               }
               else if (i < fifthContour*4)
               {
                  increment = (i+1) - fifthContour*3;
                  layerColors[i] = XVT_MAKE_COLOR(colorInc*increment, 255, 0);
               }
               else if (i < fifthContour*5)
               {
                  increment = fifthContour*5 - (i+1);
                  layerColors[i] = XVT_MAKE_COLOR(255, colorInc*increment, 0);
               }
            }

				if (anomImageData->imageDisplay.lut)  /* Keep a copy of the lut */
				{
					xvt_mem_free((char *) anomImageData->imageDisplay.lut);
					anomImageData->imageDisplay.lutSize = 0;
				}
				if (clut = (COLOR *) xvt_mem_zalloc(numContour*sizeof(COLOR)))
				{
					memcpy (clut, layerColors, numContour*sizeof(COLOR));
					anomImageData->imageDisplay.lutSize = numContour;
					anomImageData->imageDisplay.lut = clut;
				}
            
            if (anomImageData->pixmap)
            {
               xvt_pmap_destroy (anomImageData->pixmap);
               anomImageData->pixmap == NULL_PIXMAP;
            }

            if (!(anomImageData->pixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                       (short) ((anomImageData->dataXSize-1)*interpolate),
                       (short) ((anomImageData->dataYSize-1)*interpolate), NULL)))
               return (FALSE);
            setStratPalet (anomImageData->pixmap);
   
            xvt_dwin_clear (anomImageData->pixmap, COLOR_WHITE);  
            initLongJob (0, anomImageData->dataXSize*anomImageData->dataYSize,
                         "Calculating Contour...", NULL);
            coutourImageFromDoubleData(anomImageData->pixmap, anomImageData->imageData,
                         (int) (anomImageData->dataXSize-2), (int) (anomImageData->dataYSize-2),
								 0, 0, FALSE, FALSE, FALSE,
								 (double *) contourLevelPtr, (int) numContour, interpolate,
								 (int) 0, (int) 0, (COLOR *) layerColors);
            finishLongJob ();
            xvt_dwin_invalidate_rect (win, NULL);  /* refresh the window */
            return (TRUE);
            break;
      }
   }
   if (!pixmap) /* Just make sure everything worked before getting rid of old one */
      return (FALSE);
      
   if (anomImageData->pixmap)  /* destroy old image if we had one */
      xvt_pmap_destroy (anomImageData->pixmap);

   anomImageData->pixmap = pixmap;  /* use the new one */

   /* xvt_dwin_invalidate_rect (win, NULL);  .* refresh the window */
	        /* refresh the size to make sure of aspect ratio */
   ep.type = E_SIZE;
   anomImageEventHandler (win, &ep);

   return (TRUE);
}

/* ======================================================================
FUNCTION        loadContourImage
DESCRIPTION
     read in an anomilies Image and show contours

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadContourImage (char *filename)
#else
loadContourImage (filename)
char *filename;
#endif
{
   FILE *fi;
   WINDOW contourWindow;
   XVT_PIXMAP pixmap;
   double **imageData, offsetX, offsetY;
   int fileId, icsize, xmax, ymax, zmax, i, sizeX, sizeY;
   long csize;
   float dat[3], position[6], grid, airgap;
   double maxValue, minValue, range, scale;
   double contourLevel[100];
   int numContour = 50;
   double interpolate = projectOptions.imageScalingFactor;
   COLOR layerColors[100];
   char title[100];
   int binary;
   RCT rect;


   if (!(fi = fopen (filename, "r")))
   {
      //xvt_dm_post_error("Could not open Contour to read.");
      return;
   }
                      /* check to see if this is an anomily file */
   if ((!readGeophysHeader (fi, &fileId, &icsize, &xmax, &ymax, &zmax,
                  dat, position, &grid, &airgap, &sizeX, &sizeY, &binary))
        || ((fileId != 444) && (fileId != 333)))
   {
      fclose (fi);
      //xvt_dm_post_error("This is not an Anomaly Data File");
      return;
   }
   csize = (long) icsize;


   if (!(imageData = (double **) dmatrix (0, sizeX, 0, sizeY)))
   {
      fclose (fi);
      //xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }

   initLongJob (0, (int) (sizeY*2), "Loading Anomalies Contour...", NULL);

   readGeophys (fi, (unsigned char **) NULL, (double **) imageData, sizeX, sizeY,
                                     &maxValue, &minValue, TRUE, binary);
   fclose (fi);

   rect.top = (short) (0 * interpolate + 30); rect.left = (short) (0 * interpolate + 30);
   rect.bottom= (short) ((sizeY-1) * interpolate + 30);
   rect.right = (short) ((sizeX-1) * interpolate + 220 + 30);
   sprintf (title, "%s - %s", CONTOUR_TITLE, filename);
   if (!(contourWindow = xvt_win_create (W_DOC, &rect, title,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, pictureEventHandler, 0L)))
   {
      finishLongJob ();
      //xvt_dm_post_error("Cannot Create Contour Display Window.");
      free_dmatrix (imageData, 0, sizeX, 0, sizeY);
      return;
   }


             /* capture the drawing of the contour in a PIXMAP */
   rect.bottom -= rect.top; rect.top = 0;
   rect.right -= rect.left; rect.left = 0;
   if (!(pixmap = xvt_pmap_create (contourWindow, XVT_PIXMAP_DEFAULT,
                                  rect.right, rect.bottom, NULL)))
   {
      finishLongJob ();
      //xvt_dm_post_error("Cannot Create Contour Representation.");
      free_dmatrix (imageData, 0, sizeX, 0, sizeY);
      return;
   }
   xvt_dwin_clear (pixmap, COLOR_WHITE);

   setCurrentDrawingWindow (pixmap);

                  /* create the PIXMAP to describe the contour
                  ** and attach it to window we created as app_data */
   range = maxValue - minValue;
   for(i = 0; i < numContour+1; i++)
      contourLevel[i] = -99999.0; /* initialise off the edge */
   for(i = 1; i <= numContour; i++)
   {
      contourLevel[i] = minValue + ((i) * (range/(numContour+1.0)));
      if (i < 11)
         layerColors[i] = XVT_MAKE_COLOR((int) ((255.0/10)*(10-i)),
                                         0, 255);

                     /* Setup the colors to draw the contours */
      else if (i < 21)
         layerColors[i] = XVT_MAKE_COLOR(0,
                                   (int) ((255.0/10)*(i-10)), 255);
      else if (i < 31)
         layerColors[i] = XVT_MAKE_COLOR(0, 255,
                                  (int) ((255.0/10)*(30-i)));
      else if (i < 41)
         layerColors[i] = XVT_MAKE_COLOR((int) ((255.0/10)*(i-30)),
                                         255, 0);
      else if (i < 51)
         layerColors[i] = XVT_MAKE_COLOR(255,
                                   (int) ((255.0/10)*(50-i)), 0);
      
   }

   scale = 0.343*(interpolate/10.0);
   offsetX = 0.0 - (sizeY*(interpolate/10.0)-35)*0.35;
   offsetY = -0.5;
   mcontr (imageData, sizeX-1, sizeY-2, contourLevel, numContour,
           1.0e6, scale, offsetX, offsetY, FALSE, BASE_STRAT, 0, layerColors);
   PlotLegend((int) (sizeX*interpolate), 20, csize, xmax, ymax, zmax, dat,
                   position, grid, airgap, fileId, minValue, range, filename);


   free_dmatrix (imageData, 0, sizeX, 0, sizeY);
   xvt_vobj_set_data (contourWindow, (long) pixmap);

   finishLongJob ();

   xvt_vobj_set_visible (contourWindow, TRUE);
   bringWindowToFront(contourWindow);
}
/* ======================================================================
FUNCTION        loadProfileImage
DESCRIPTION
     read in an anomilies Image and display the profile

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadProfileImage (char *filename)
#else
loadProfileImage (filename)
char *filename;
#endif
{
   FILE *fi;
   WINDOW profileWindow;
   XVT_PIXMAP pixmap;
   double **imageData;
   int fileId, csize, xmax, ymax, zmax, sizeX, sizeY;
   float dat[3], position[6], grid, airgap;
   double minoff, range;
   int binary;
   char title[100];
   RCT rect;

   if (!(fi = fopen (filename, "r")))
   {
      //xvt_dm_post_error("Could not Open Profile to Read.");
      return;
   }
                      /* check to see if this is an anomily file */
   if (!readGeophysHeader (fi, &fileId, &csize, &xmax, &ymax, &zmax,
                  dat, position, &grid, &airgap, &sizeX, &sizeY, &binary)
        || ((fileId != 555) && (fileId != 666)))
   {
      fclose (fi);
      //xvt_dm_post_error("This is not an Profile Data File");
      return;
   }



   if (!(imageData = (double **) create2DArray (sizeX+1, sizeY+1, sizeof(double))))
   {
      fclose (fi);
      //xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }

   initLongJob (0, sizeY, "Loading Profile Image...", NULL);

   readGeophys (fi, (unsigned char **) NULL, (double **) imageData, sizeX, sizeY,
                           (double *) NULL, (double *) NULL, TRUE, binary);

   fclose (fi);

   getWindowStartPosition(PROFILE_TITLE, &rect.left, &rect.top, &rect.right, &rect.bottom, NULL, NULL); 
   sprintf (title, "%s - %s", PROFILE_TITLE, filename);
   if (!(profileWindow = xvt_win_create (W_DOC, &rect, title,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, pictureEventHandler, 0L)))
   {
      //xvt_dm_post_error("Cannot Create Profile Display Window.");
      destroy2DArray ((char **) imageData, sizeX+1, sizeY+1);
      return;
   }
   setStratPalet (profileWindow);


             /* capture the drawing of the profile in a PIXMAP */
   rect.bottom -= rect.top; rect.top = 0;
   rect.right -= rect.left; rect.left = 0;
   if (!(pixmap = xvt_pmap_create (profileWindow, XVT_PIXMAP_DEFAULT,
                                  rect.right, rect.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Profile Representation.");
      destroy2DArray ((char **) imageData, sizeX+1, sizeY+1);
      return;
   }
   xvt_dwin_clear (pixmap, COLOR_WHITE);
   setCurrentDrawingWindow (pixmap);

                  /* create the PIXMAP to describe the profile
                  ** and attach it to window we created as app_data */
   DrawGraph (sizeY, &(imageData[1][0]), ((float) 5.0), ((fileId == 555) ? 1 : 0),
                                        0, zmax, &minoff, &range);
   PlotTravLegend ((int) 25, (int) 365, (long) csize,  xmax, ymax, zmax,
                   (float *) dat, (float *) position, (float) grid,
                   (float) airgap, (int) fileId, (double) minoff, (double) range, filename);

   destroy2DArray ((char **) imageData, sizeX+1, sizeY+1);
   xvt_vobj_set_data (profileWindow, (long) pixmap);

   finishLongJob ();

   xvt_vobj_set_visible (profileWindow, TRUE);
   bringWindowToFront(profileWindow);
}

int
#if XVT_CC_PROTO
updateProfileImage (ANOMIMAGE_DATA *anomImageData,
                    PNT startLine, PNT endLine, double scale, double scaleToData)
#else
updateProfileImage (anomImageData, startLine, endLine, scale, scaleToData)
ANOMIMAGE_DATA *anomImageData;
PNT startLine, endLine;
double scale, scaleToData;
#endif
{
   static int count = 0;
   ANOMIMAGE_DATA *profileAnomImageData = NULL;
	PNT startLineOrig, endLineOrig;
   WINDOW profWindow;
   XVT_PIXMAP pixmap;
   char title[100];
   RCT rect;
   double xDist, yDist, length;
#define PROFILE_DISPLAY_HEIGHT 290
#define PROFILE_BORDER_WIDTH   100
	startLineOrig = startLine;
	endLineOrig = endLine;
   startLine.h = (short) (startLine.h / scale);
   startLine.v = (short) (startLine.v / scale);
   endLine.h = (short) (endLine.h / scale);
   endLine.v = (short) (endLine.v / scale);
   xDist = (endLine.h - startLine.h);
   yDist = (endLine.v - startLine.v);
   length = sqrt((double) (xDist*xDist + yDist*yDist));

   getWindowStartPosition(PROFILE_FROM_IMAGE_TITLE, &rect.left, &rect.top, NULL, NULL, NULL, NULL);
   rect.bottom = rect.top + PROFILE_DISPLAY_HEIGHT;
   rect.right = (short) (rect.left + length + PROFILE_BORDER_WIDTH);
   sprintf (title, "%s (%d)", PROFILE_FROM_IMAGE_TITLE, count+1); count++;
   if (!(profWindow = xvt_win_create (W_DOC, &rect, title,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, anomImageEventHandler, 0L)))
   {
      //xvt_dm_post_error("Cannot Create Profile Display Window.");
      return (0);
   }


             /* capture the drawing of the profile in a PIXMAP */
   rect.bottom -= rect.top; rect.top = 0;
   rect.right -= rect.left; rect.left = 0;
   if (!(pixmap = xvt_pmap_create (profWindow, XVT_PIXMAP_DEFAULT,
                                   rect.right, rect.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Profile Representation.");
      return (0);
   }
                              /* Calculate the Profile */
   profileAnomImageData = (ANOMIMAGE_DATA *) plotAnomSection (pixmap,
                             anomImageData, startLine, endLine, scale,
								  	  startLineOrig, endLineOrig, scaleToData);
   
   xvt_vobj_set_data (profWindow, (long) profileAnomImageData);

   xvt_vobj_set_visible (profWindow, TRUE);
   bringWindowToFront(profWindow);
   
   return (count);
}

/* ======================================================================
FUNCTION        convertToERMImage
DESCRIPTION
     convert a mag or grv image to ERM format

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
convertToERMImage (char *filename)
#else
convertToERMImage (filename)
char *filename;
#endif
{
   FILE_SPEC fileSpec;
   FILE *fi;
   double **imageData;
   int fileId, icsize, xmax, ymax, zmax, sizeX, sizeY;
   long csize;
   float dat[3], position[6], grid, airgap;
   double maxValue, minValue;
   int binary, magImage = FALSE, grvImage = FALSE;
   GEOPHYSICS_OPTIONS options;
   int lineMapConversion;
   ASK_RESPONSE response;
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   
   memcpy (&options, &geophysicsOptions, sizeof(GEOPHYSICS_OPTIONS));
   if (strlen(filename) > 0)
      lineMapConversion = FALSE;
   else
      lineMapConversion = TRUE;

   if (lineMapConversion)
   {
         response = xvt_dm_post_ask ("No", "Yes", NULL,
                      "This will save the current Geology Linemap as vector Data. Continue ?");
         if (response == RESP_DEFAULT)
            return (FALSE);
   } 
   else  /* load the image to convert */
   {                          /* ******************* */
                              /* Load the input file */
      if (!(fi = fopen (filename, "r")))
      {
         //xvt_dm_post_error("Could not open Anomalies file to read.");
         return (FALSE);
      }
                         /* check to see if this is an anomily file */
      if ((!readGeophysHeader (fi, &fileId, &icsize, &xmax, &ymax, &zmax,
                     dat, position, &grid, &airgap, &sizeX, &sizeY, &binary))
           || ((fileId != 444) && (fileId != 333)))
      {
         fclose (fi);
         //xvt_dm_post_error("This is not an Anomaly Data File");
         return (FALSE);
      }
      csize = (long) icsize;
   
   
      /* if (!(imageData = (double **) dmatrix (0, sizeX, 0, sizeY))) */
	   if (!(imageData = (double **) create2DArray (sizeX+1, sizeY+1, sizeof(double))))
      {
         fclose (fi);
         //xvt_dm_post_error("Not enough memory, try closing some windows");
         return (FALSE);
      }
   
      initLongJob (0, (int) (sizeY*2), "Loading Image...", NULL);
   
      readGeophys (fi, (unsigned char **) NULL, (double **) imageData, sizeX, sizeY,
                                        &maxValue, &minValue, TRUE, binary);
      fclose (fi);
      finishLongJob ();
   }   

                      /* ************************************* */
                      /* First get the name of the output file */
   if (lineMapConversion)
      strcpy (fileSpec.name, "untitled.erv");
   else
      strcpy (fileSpec.name, "untitled.ers");
      
#if (XVTWS == MACWS)
   strcpy (fileSpec.type, "TEXT");
#else
   if (lineMapConversion)
      strcpy (fileSpec.type, "erv");
   else
      strcpy (fileSpec.type, "ers");
#endif
                      /* First get the name of the output file */
   getDefaultDirectory(&(fileSpec.dir));
   switch (xvt_dm_post_file_save(&fileSpec, "ERM Output File..."))
   {
      case FL_OK:
         if (strlen(fileSpec.name) == 0)
         {
            //xvt_dm_post_error ("Error, No file Specified");
            if (!lineMapConversion)
					destroy2DArray ((char **) imageData, sizeX+1, sizeY+1);
					/* free_dmatrix (imageData, 0, sizeX, 0, sizeY); */
            return (FALSE);
         }
         setDefaultDirectory(&(fileSpec.dir));
         break;
      case FL_BAD:
         //xvt_dm_post_error("Error getting file name.");
         if (!lineMapConversion)
				destroy2DArray ((char **) imageData, sizeX+1, sizeY+1);
            /* free_dmatrix (imageData, 0, sizeX, 0, sizeY); */
         return (FALSE);
         break;
      case FL_CANCEL:
         if (!lineMapConversion)
				destroy2DArray ((char **) imageData, sizeX+1, sizeY+1);
            /* free_dmatrix (imageData, 0, sizeX, 0, sizeY); */
         return (FALSE);
         break;
   }

   if (lineMapConversion)
   {
      DoERMapperLineMap((int) (blockView->lengthX / blockView->geophysicsCubeSize),
                        (int) (blockView->lengthY / blockView->geophysicsCubeSize),
                        fileSpec.name);
   }
   else
   {
      if (fileId == 333)
         magImage = TRUE;
      if (fileId == 444)
         grvImage = TRUE;
         
      initLongJob (0, 100, "Writing ERM File...", NULL);
      writeERMAnomImage (fileSpec.name, imageData, sizeX+1, sizeY,  /* Was sizeY (added +1) */
                         1, 0/*1*/, (double) grid,(double) position[0],
                         (double) position[1], &options, magImage, grvImage);
      finishLongJob ();
                                /* Free the memory we are using */
		destroy2DArray ((char **) imageData, sizeX+1, sizeY+1);
      /* free_dmatrix (imageData, 0, sizeX, 0, sizeY); */
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        convertToGeosoftImage
DESCRIPTION
     convert a mag or grv image to ERM format

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
convertToGeosoftImage (char *filename)
#else
convertToGeosoftImage (filename)
char *filename;
#endif
{
   FILE_SPEC fileSpec;
   FILE *fi;
   double **imageData;
   int fileId, icsize, xmax, ymax, zmax, sizeX, sizeY;
   long csize;
   float dat[3], position[6], grid, airgap;
   double maxValue, minValue;
   int binary, magImage = FALSE, grvImage = FALSE;
   GEOPHYSICS_OPTIONS options;
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   
   memcpy (&options, &geophysicsOptions, sizeof(GEOPHYSICS_OPTIONS));
   if (strlen(filename) == 0)
      return (FALSE);

                          /* ******************* */
                          /* Load the input file */
   if (!(fi = fopen (filename, "r")))
   {
      //xvt_dm_post_error("Could not open Anomalies file to read.");
      return (FALSE);
   }
                      /* check to see if this is an anomily file */
   if ((!readGeophysHeader (fi, &fileId, &icsize, &xmax, &ymax, &zmax,
                  dat, position, &grid, &airgap, &sizeX, &sizeY, &binary))
        || ((fileId != 444) && (fileId != 333)))
   {
      fclose (fi);
      //xvt_dm_post_error("This is not an Anomaly Data File");
      return (FALSE);
   }
   csize = (long) icsize;
   
   
   if (!(imageData = (double **) dmatrix (0, sizeX, 0, sizeY)))
   {
      fclose (fi);
      //xvt_dm_post_error("Not enough memory, try closing some windows");
      return (FALSE);
   }
   
   initLongJob (0, (int) (sizeY*2), "Loading Image...", NULL);
   
   readGeophys (fi, (unsigned char **) NULL, (double **) imageData, sizeX, sizeY,
                                     &maxValue, &minValue, TRUE, binary);
   fclose (fi);
   finishLongJob ();

                      /* ************************************* */
                      /* First get the name of the output file */
   strcpy (fileSpec.name, "untitled.gxf");
      
#if (XVTWS == MACWS)
   strcpy (fileSpec.type, "TEXT");
#else
   strcpy (fileSpec.type, "gxf");
#endif
                      /* First get the name of the output file */
   getDefaultDirectory(&(fileSpec.dir));
   switch (xvt_dm_post_file_save(&fileSpec, "Geosoft Output File..."))
   {
      case FL_OK:
         if (strlen(fileSpec.name) == 0)
         {
            //xvt_dm_post_error ("Error, No file Specified");
               free_dmatrix (imageData, 0, sizeX, 0, sizeY);
            return (FALSE);
         }
         setDefaultDirectory(&(fileSpec.dir));
         break;
      case FL_BAD:
         //xvt_dm_post_error("Error getting file name.");
         free_dmatrix (imageData, 0, sizeX, 0, sizeY);
         return (FALSE);
         break;
      case FL_CANCEL:
         free_dmatrix (imageData, 0, sizeX, 0, sizeY);
         return (FALSE);
         break;
   }

   if (fileId == 333)
      magImage = TRUE;
   if (fileId == 444)
      grvImage = TRUE;
         
   initLongJob (0, 100, "Writing Geosoft File...", NULL);
   writeGeosoftAnomImage (fileSpec.name, imageData, sizeX+1, sizeY,
                      1, 1, (double) grid, (double) position[0],
                      (double) position[1], &options, magImage, grvImage);
   finishLongJob ();
                             /* Free the memory we are using */
   free_dmatrix (imageData, 0, sizeX, 0, sizeY);

   return (TRUE);
}

/* ======================================================================
FUNCTION        calcImageDifference
DESCRIPTION
     take the values on one anom image from another and store result.

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
calcImageDifference (ANOMIMAGE_DATA *image1Data, ANOMIMAGE_DATA *image2Data, ANOMIMAGE_DATA *diffImage)
#else
calcImageDifference (image1Data, image2Data, diffImage)
ANOMIMAGE_DATA *image1Data, *image2Data, *diffImage;
#endif
{
   double **result, **image1, **image2, **tempImage = NULL;
   int resultX, resultY, x, y;
   double minValue, maxValue;
   
   if (!image1Data || !image2Data || !diffImage)
      return (FALSE);
   
   resultX = image1Data->dataXSize;
   resultY = image1Data->dataYSize;

   image1 = image1Data->imageData;
   image2 = image2Data->imageData;

   if (!image1 || !image2)
      return (FALSE);

   if ((resultX != image1Data->dataXSize) || (resultY != image2Data->dataYSize)
              || (floor(image1Data->geoXStart) != floor(image2Data->geoXStart))
              || (floor(image1Data->geoYStart) != floor(image2Data->geoYStart))
              ||   (floor(image1Data->geoXEnd) != floor(image2Data->geoXEnd))
              ||   (floor(image1Data->geoYEnd) != floor(image2Data->geoYEnd)) )
   {                                 /* Match Data Images to same size */
      if (tempImage = (double **) create2DArray (resultY, resultX, sizeof(double)))
      {      
         mapArrayToArray(image2, image2Data->dataXSize, image2Data->dataYSize,
                                 image2Data->geoYEnd, image2Data->geoYStart,
                                 image2Data->geoXStart, image2Data->geoXEnd,
                      tempImage, resultX, resultY,
                                 image1Data->geoYEnd, image1Data->geoYStart,
                                 image1Data->geoXStart, image1Data->geoXEnd, FALSE);
         image2 = tempImage;
      }
      else
         return (FALSE);
   }
   
   if (!(result = (double **) create2DArray(resultY, resultX, sizeof(double))))
   {
      if (tempImage)
         destroy2DArray ((char **) tempImage, resultY, resultX);
      return (FALSE);
   }
   
   for (x = 0; x < resultX; x++)
   {
      for (y = 0; y < resultY; y++)
      {
         result[y][x] = image1[y][x] - image2[y][x];
         if ((x == 0) && (y == 0))
         {
            minValue = maxValue = result[y][x];
         }
         else
         {
            if (minValue > result[y][x])
               minValue = result[y][x];
            if (maxValue < result[y][x])
               maxValue = result[y][x];
         }
      }
   }

   memcpy (diffImage, image1Data, sizeof(ANOMIMAGE_DATA));
   if (diffImage->dataType == GRAVITY_DATA)  /* Gravity */
      memcpy (&(diffImage->imageDisplay),
              &(projectOptions.gravityDisplay), sizeof (IMAGE_DISPLAY_DATA));
   else  /* 333 - Magnetics */
      memcpy (&(diffImage->imageDisplay),
              &(projectOptions.magneticsDisplay), sizeof (IMAGE_DISPLAY_DATA));

   diffImage->pixmap = NULL_PIXMAP;   
   diffImage->imageData = result;
   diffImage->dataXSize = resultX;
   diffImage->dataYSize = resultY;
   diffImage->minValue = minValue;
   diffImage->maxValue = maxValue;
   if (maxValue == minValue)
      diffImage->scale = 1.0;
   else
      diffImage->scale = (double) 255.0 / (maxValue - minValue);
   if (diffImage->imageDisplay.clippingType == RELATIVE_CLIPPING)
   {
      double percentInc;  /* convert percent values into real image values */
      
      percentInc = (diffImage->maxValue - diffImage->minValue)/100.0;

      diffImage->imageDisplay.minClip = diffImage->minValue
                             + percentInc*diffImage->imageDisplay.minClip;
      diffImage->imageDisplay.maxClip = diffImage->minValue
                             + percentInc*diffImage->imageDisplay.maxClip;
   }                                          

   if (tempImage)
      destroy2DArray ((char **) tempImage, resultY, resultX);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        calcVertDerivImage
DESCRIPTION
     convert a mag or grv image to ERM format

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
calcVertDerivImage (char *filename, int power)
#else
calcVertDerivImage (filename, power)
char *filename;
int power;
#endif
{
   FILE_SPEC fileSpec;
   FILE *fi;
   double **imageData;
   int fileId, icsize, xmax, ymax, zmax, sizeX, sizeY;
   long csize;
   float dat[3], position[6], grid, airgap;
   double maxValue, minValue;
   int binary, magImage = FALSE, grvImage = FALSE;
   GEOPHYSICS_OPTIONS options;
   BLOCK_VIEW_OPTIONS *realBlockView = getViewOptions (), blockView;
   
   memcpy (&options, &geophysicsOptions, sizeof(GEOPHYSICS_OPTIONS));
   memcpy (&blockView, realBlockView, sizeof(BLOCK_VIEW_OPTIONS));
   if (strlen(filename) == 0)
      return (FALSE);
                          /* ******************* */
                          /* Load the input file */
   if (!(fi = fopen (filename, "r")))
   {
      //xvt_dm_post_error("Could not open Anomalies file to read.");
      return (FALSE);
   }
                      /* check to see if this is an anomily file */
   if ((!readGeophysHeader (fi, &fileId, &icsize, &xmax, &ymax, &zmax,
                  dat, position, &grid, &airgap, &sizeX, &sizeY, &binary))
        || ((fileId != 444) && (fileId != 333)))
   {
      fclose (fi);
      //xvt_dm_post_error("This is not an Anomaly Data File");
      return (FALSE);
   }
   csize = (long) icsize;
   blockView.geophysicsCubeSize = grid;
   blockView.originX = position[0];
   blockView.originY = position[1];
   blockView.originZ = position[5];
   blockView.lengthX = position[3]-position[0];
   blockView.lengthY = position[4]-position[1];
   blockView.lengthZ = position[5]-position[2];
   options.inclination = dat[0];
   options.declination = dat[1];
   options.intensity = dat[2];
   options.calculationRange = (int) (grid*csize);
   options.calculationAltitude == AIRBORNE;
   options.altitude = airgap;
   if (fileId == 333)
      magImage = TRUE;
   if (fileId == 444)
      grvImage = TRUE;
   
   if (!(imageData = (double **) create2DArray (sizeY, sizeX, sizeof(double))))
   {
      fclose (fi);
      //xvt_dm_post_error("Not enough memory, try closing some windows");
      return (FALSE);
   }
   
   initLongJob (0, (int) (sizeY*2), "Loading Image...", NULL);
   
   readGeophys (fi, (unsigned char **) NULL, (double **) imageData, sizeX, sizeY,
                                     &maxValue, &minValue, FALSE, binary);
   fclose (fi);
   finishLongJob ();
                           
                      /* ************************************* */
                      /* First get the name of the output file */
   strcpy (fileSpec.name, "untitled.mag");

#if (XVTWS == MACWS)
   strcpy (fileSpec.type, "TEXT");
#else
   if (magImage)
      strcpy (fileSpec.type, "mag");
   else  /* grvImage */
      strcpy (fileSpec.type, "grv");
#endif
   addFileExtention (fileSpec.name, fileSpec.type);

                      /* First get the name of the output file */
   getDefaultDirectory(&(fileSpec.dir));
   switch (xvt_dm_post_file_save(&fileSpec, "Derivative Output File..."))
   {
      case FL_OK:
         if (strlen(fileSpec.name) == 0)
         {
            //xvt_dm_post_error ("Error, No file Specified");
            destroy2DArray ((char **) imageData, sizeY, sizeX);
            return (FALSE);
         }
         setDefaultDirectory(&(fileSpec.dir));
         break;
      case FL_BAD:
         //xvt_dm_post_error("Error getting file name.");
         destroy2DArray ((char **) imageData, sizeY, sizeX);
         return (FALSE);
         break;
      case FL_CANCEL:
         destroy2DArray ((char **) imageData, sizeY, sizeX);
         return (FALSE);
         break;
   }

         
   initLongJob (0, sizeY*3, "Writing Vertical Deriv...", NULL);
   
   if (!Vertical_Derivative(imageData, sizeX, sizeY, power-1, imageData))
	{
		//xvt_dm_post_error ("Error, Not enough memory to do Vertical Derivative");
      destroy2DArray ((char **) imageData, sizeY, sizeX);
		return (FALSE);
	}

   xvt_fsys_set_dir(&fileSpec.dir);
   writeStandardAnomFile (fileSpec.name, imageData, (grvImage?0:1), sizeX, sizeY, 1,
                          &blockView, &options);

   finishLongJob ();
                             /* Free the memory we are using */
   destroy2DArray ((char **) imageData, sizeY, sizeX);

                             /* Display the image we just calculated */
   displayAnomaliesImage (fileSpec.name, NULL);

   return (TRUE);
}

/* ======================================================================
FUNCTION        loadAnomImage
DESCRIPTION
     Load a anomalies image into the ANOMIMAGE_DATA structure

INPUT  

OUTPUT

RETURNED
====================================================================== */
ANOMIMAGE_DATA *
#if XVT_CC_PROTO
loadAnomImage (char *filename)
#else
loadAnomImage (filename)
char *filename;
#endif
{
   FILE *fi;
   ANOMIMAGE_DATA *anomImageData;
   double **imageData;
   double minValue, maxValue;
   int fileId, csize, xmax, ymax, zmax, sizeX, sizeY;
   float dat[3], position[6], grid, airgap;
   int binary;

   if (!(anomImageData = (ANOMIMAGE_DATA *) xvt_mem_zalloc (sizeof(ANOMIMAGE_DATA))))
   {
      //xvt_dm_post_error ("Error, Not enough memory to load reference image");
      return (FALSE);
   }                        

   if (!(fi = fopen (filename, "r")))
   {
      //xvt_dm_post_error("Could not open image to read.");
      xvt_mem_free ((char *) anomImageData);
      return (FALSE);
   }
                      /* check to see if this is an anomily file */
   if ((!readGeophysHeader (fi, &fileId, &csize, &xmax, &ymax, &zmax,
             dat, position, &grid, &airgap, &sizeX, &sizeY, &binary))
        || ((fileId != 444) && (fileId != 333)))
   {
      fclose (fi);
      //xvt_dm_post_error("This is not an Anomaly Data File");
      xvt_mem_free ((char *) anomImageData);
      return (FALSE);
   }

                       /* init image Display options */
   if (fileId == 444)  /* Gravity */
      memcpy (&(anomImageData->imageDisplay),
              &(projectOptions.gravityDisplay), sizeof (IMAGE_DISPLAY_DATA));
   else  /* 333 - Magnetics */
      memcpy (&(anomImageData->imageDisplay),
              &(projectOptions.magneticsDisplay), sizeof (IMAGE_DISPLAY_DATA));

   if (!(imageData = (double **) create2DArray (sizeY, sizeX,
                                             sizeof (double))))
   {
      fclose (fi);
      //xvt_dm_post_error("Not enough memory, try closing some windows");
      xvt_mem_free ((char *) anomImageData);
      return (FALSE);
   }

   initLongJob (0, (int) (sizeY*2 + sizeY), "Loading Anomalies Image...", NULL);

   readGeophys (fi, (unsigned char **) NULL, (double **) imageData, sizeX, sizeY,
                           &maxValue, &minValue, FALSE, binary);
	
	if (!readGeophysComment(fi, ANOM_HISTORY_TAG, anomImageData->historyFile, SZ_FNAME, TRUE))
		strcpy(anomImageData->historyFile, "unknown");
	if (!readGeophysComment(fi, ANOM_DATETIME_TAG, anomImageData->dateTime, 30, TRUE))
		strcpy(anomImageData->dateTime, "unknown");

   fclose (fi);

                              /* Fill in structure data */
   anomImageData->imageData = imageData;
   anomImageData->dataXSize = sizeX;
   anomImageData->dataYSize = sizeY;
   anomImageData->minValue = minValue;
   anomImageData->maxValue = maxValue;
   if (maxValue == minValue)
      anomImageData->scale = 1.0;
   else
      anomImageData->scale = (double) 255.0 / (maxValue - minValue);
   anomImageData->geoXStart = MIN(position[0], position[3]);
   anomImageData->geoXEnd = MAX(position[0], position[3]);
   anomImageData->geoYStart = MIN(position[1], position[4]);
   anomImageData->geoYEnd = MAX(position[1], position[4]);
   anomImageData->geoHeight = airgap;
   anomImageData->fileBlockTop = (double) position[5];
   anomImageData->fileBlockBottom = (double) position[2];
   anomImageData->fileCubeSize = (double) grid;
   if (fileId == 333)
      anomImageData->dataType = MAGNETICS_DATA;
   else
      anomImageData->dataType = GRAVITY_DATA;
   anomImageData->inclination = dat[0];
   anomImageData->declination = dat[1];
   anomImageData->intensity = dat[2];
   if (anomImageData->imageDisplay.clippingType == RELATIVE_CLIPPING)
   {
      double percentInc;  /* convert percent values into real image values */
      
      percentInc = (anomImageData->maxValue - anomImageData->minValue)/100.0;

      anomImageData->imageDisplay.minClip = anomImageData->minValue
                             + percentInc*anomImageData->imageDisplay.minClip;
      anomImageData->imageDisplay.maxClip = anomImageData->minValue
                             + percentInc*anomImageData->imageDisplay.maxClip;
   }
   
   finishLongJob ();

   return (anomImageData);
}

/* ======================================================================
FUNCTION        setReferenceImage
DESCRIPTION
     Load in the binary image for reference

INPUT  

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
setReferenceImage (ANOMIMAGE_DATA *anomImageData)
#else
setReferenceImage (anomImageData)
ANOMIMAGE_DATA *anomImageData;
#endif
{
   if (!anomImageData)
      return (FALSE);
   
   switch (anomImageData->dataType)
   {
      case (MAGNETICS_DATA):
         if (magneticsReferenceData)
         {
            if (magneticsReferenceWindow)
               xvt_vobj_destroy (magneticsReferenceWindow);
            destroy2DArray ((char **) magneticsReferenceData->imageData,
                                      magneticsReferenceData->dataYSize,
                                      magneticsReferenceData->dataXSize);
            xvt_mem_free ((char *) magneticsReferenceData);
            magneticsReferenceData = NULL;
         }
         magneticsReferenceData = anomImageData;
         updateMenuOptions (TASK_MENUBAR, NULL_WIN);
         break;
      case (GRAVITY_DATA):
         if (gravityReferenceData)
         {
            if (gravityReferenceWindow)
               xvt_vobj_destroy (gravityReferenceWindow);
            destroy2DArray ((char **) gravityReferenceData->imageData,
                                      gravityReferenceData->dataYSize,
                                      gravityReferenceData->dataXSize);
            xvt_mem_free ((char *) gravityReferenceData);
            gravityReferenceData = NULL;
         }
         gravityReferenceData = anomImageData;
         updateMenuOptions (TASK_MENUBAR, NULL_WIN);
         break;
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        loadTraceImage
DESCRIPTION
     read in a picture file to trace over

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadTraceImage (OBJECT *object, char *filename)
#else
loadTraceImage (object, filename)
OBJECT *object;
char *filename;
#endif
{
   WINDOW pictureWindow;
   XVT_IMAGE image;
   char title[OBJECT_TEXT_LENGTH+100];
   WINDOW frontWin = (WINDOW) NULL;
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   RCT position;
   TRACE_DATA *traceData;
 
   if (!(image = xvt_image_read (filename)))
   {
      //xvt_dm_post_error("Error, Could Not read Image File");
      return (FALSE);
   }
   
   if (!(traceData = (TRACE_DATA *) xvt_mem_zalloc (sizeof(TRACE_DATA))))
   {
      xvt_image_destroy (image);
      return (FALSE);
   }
                    
   sprintf (title, "Trace %s - %s", object->text, filename);
   
   position.left = 0; position.top = 0;
   xvt_image_get_dimensions (image, &position.right, &position.bottom);
   
   if (!(pixmap = xvt_pmap_create (TASK_WIN, XVT_PIXMAP_DEFAULT,
                             position.right, position.bottom, 0L)))
   {
      xvt_image_destroy (image);
      //xvt_dm_post_error("Error, Not Enough Memory to Create Picture.");
      return (FALSE);
   }
   traceData->pixmap = pixmap;
   traceData->object = object;
   xvt_dwin_clear (pixmap, COLOR_WHITE);

   xvt_dwin_draw_image (pixmap, image, &position, &position);   
   
   xvt_image_destroy (image);

   position.top += 60; position.bottom += 60;
   position.left += 60; position.right += 60;
   if (!(pictureWindow = xvt_win_create (W_DOC, &position,
                   title, (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,
                   EM_ALL, traceEventHandler, (long) traceData)))
   {
      //xvt_dm_post_error("Cannot Create Picture Window.");
      return (FALSE);
   }

   xvt_vobj_set_visible (pictureWindow, TRUE);
   bringWindowToFront(pictureWindow);
   return (TRUE);
}

/* ======================================================================
FUNCTION        convertTraceData
DESCRIPTION
     convert the trace point information into the event options
     window

INPUT
   TRACE_DATA *traceData

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
convertTraceData (TRACE_DATA *traceData)
#else
convertTraceData (traceData)
TRACE_DATA *traceData;
#endif
{
   PROFILE_POINT *point, *prevPoint = NULL, *lastPoint, *firstPoint;
   PNT topPoint, bottomPoint, leftPoint, rightPoint;
   double topLeftX, topLeftY, topLeftZ, botRightX, botRightY, botRightZ;
   double xInc, yInc, zInc;
   int refFrameDefined;
   RCT imageSize;
   
   if (!(firstPoint = traceData->points))
      return (FALSE);

         /* get the reference frame of the image */                             
   xvt_vobj_get_client_rect (traceData->imageWindow, &imageSize);
   switch (traceData->object->shape)
   {
      case (FOLD):
         topLeftX = getFloatTextFieldValue (traceData->menuWindow, FOLD_TRACE_XTOP);
         topLeftY = getFloatTextFieldValue (traceData->menuWindow, FOLD_TRACE_YTOP);
         topLeftZ = getFloatTextFieldValue (traceData->menuWindow, FOLD_TRACE_ZTOP);
         botRightX = getFloatTextFieldValue (traceData->menuWindow, FOLD_TRACE_XBOT);
         botRightY = getFloatTextFieldValue (traceData->menuWindow, FOLD_TRACE_YBOT);
         botRightZ = getFloatTextFieldValue (traceData->menuWindow, FOLD_TRACE_ZBOT);
         break;
      case (FAULT): case (SHEAR_ZONE):
         topLeftX = getFloatTextFieldValue (traceData->menuWindow, FAULT_TRACE_XTOP);
         topLeftY = getFloatTextFieldValue (traceData->menuWindow, FAULT_TRACE_YTOP);
         topLeftZ = getFloatTextFieldValue (traceData->menuWindow, FAULT_TRACE_ZTOP);
         botRightX = getFloatTextFieldValue (traceData->menuWindow, FAULT_TRACE_XBOT);
         botRightY = getFloatTextFieldValue (traceData->menuWindow, FAULT_TRACE_YBOT);
         botRightZ = getFloatTextFieldValue (traceData->menuWindow, FAULT_TRACE_ZBOT);
         break;
      default:
         topLeftX = topLeftY = topLeftZ = 0.0;
         botRightX= botRightY= botRightZ= 0.0;
   }
         /* only use reference frame if have real values */
   if ((topLeftX != botRightX) || (topLeftY != botRightY)
                               || (topLeftZ != botRightZ))
   {
      refFrameDefined = TRUE;
      xInc = (botRightX - topLeftX)/((double) imageSize.right);
      yInc = (botRightY - topLeftY)/((double) imageSize.bottom);
      zInc = (botRightZ - topLeftZ)/((double) imageSize.bottom);
   }
   else
      refFrameDefined = FALSE;
      
   
   topPoint = bottomPoint = leftPoint = rightPoint
                                      = firstPoint->point;
   for (point = firstPoint; point; point = point->next)
   {
      if (point->point.h < leftPoint.h)
         leftPoint = point->point;
      if (point->point.h > rightPoint.h)
         rightPoint = point->point;
      if (point->point.v < topPoint.v)
         topPoint = point->point;
      if (point->point.v > bottomPoint.v)
         bottomPoint = point->point;
      
      prevPoint = point;
   }
       /* the last point in the profile */
   lastPoint = prevPoint;
   
   switch (traceData->object->shape)
   {
      case (FOLD):
         {
            double wavelength, amplitude; 
            double xPos, yPos, zPos, *nullPtr = NULL;
            double gradient, constant;
            double length, height, baseX;
            FOLD_OPTIONS *options;
            int foldType = FOURIER_FOLD;
            
            gradient = (double) (lastPoint->point.v - firstPoint->point.v)
                       / (double) (lastPoint->point.h - firstPoint->point.h);
            constant = (double) firstPoint->point.v
                            - (double) firstPoint->point.h*gradient;
            length = rightPoint.h - leftPoint.h;

            options = (FOLD_OPTIONS *) traceData->object->options;
            profilePointsFree (options->profile.points);
            
            for (point = firstPoint; point; point = point->next)
            {
               baseX = gradient * (double) point->point.h + constant;
               point->point.h = (short) floor ((double) (point->point.h - leftPoint.h)
                                        / length * (double) X_PROFILE_RESOLUTION);
                        /* work out heights from base line as we go so
                        ** we can scale by in the next pass */
               point->point.v = (short) floor (baseX - (double) point->point.v);
               if (abs(point->point.v) > height)
                  height = abs(point->point.v);
            }
                  /* Scale by the height to get in correct range */
            for (point = firstPoint; point; point = point->next)
               point->point.v = (short) floor (((double) point->point.v)/height
                                               * (double) Y_PROFILE_RESOLUTION);

                        /* use the points we hav e (after remaping) */
            options->type = FOURIER_FOLD;
            options->profile.points = firstPoint;
            traceData->points = NULL; /* make sure they are not freed */

            if (refFrameDefined)  /* work out the position of event */
            {
               xPos = topLeftX + leftPoint.h*xInc;
               yPos = topLeftY + leftPoint.v*yInc;
               zPos = topLeftZ + leftPoint.v*zInc;
               
               if ((amplitude = fabs(height*zInc)) < 1.0)
                  if ((amplitude = fabs(height*yInc)) < 1.0)
                     amplitude = fabs(height*xInc);
               if ((wavelength = fabs(length*xInc)) < 1.0)
                  if ((wavelength = fabs(length*yInc)) < 1.0)
                     wavelength = fabs(length*zInc);
               setFoldValues (&foldType, &wavelength, &amplitude, &xPos, &yPos, &zPos);
            }
            else
               setFoldValues (&foldType, nullPtr, nullPtr, nullPtr, nullPtr, nullPtr);
         }
         break;
      case (FAULT):
         {
            FAULT_OPTIONS *options;
            double dist, xDist, yDist;
            double xPos, yPos, zPos, dipDir, dip, *nullPtr = NULL;
               
              /* need ref frame to work out anything for faults */
            if (!refFrameDefined)
            {
               //xvt_dm_post_error ("Error, Reference frame not defined Properly.");
               break;
            }
              
            options = (FAULT_OPTIONS *) traceData->object->options;
            xDist = rightPoint.h - leftPoint.h;
            yDist = bottomPoint.v - topPoint.v;
            dist = sqrt(xDist*xDist + yDist*yDist);
            
            if (fabs(zInc) < 1.0)  /* Map view so work out direction */
            {
/*
               dipDir = 90.0 - RAD_TO_DEG(acos(yDist/dist));
               setFaultValues (nullPtr, nullPtr, nullPtr, &dipDir, nullPtr);
*/
               dip = 90.0;
               if (leftPoint.v > rightPoint.v)
               {  /* Dipping to the left */
                  dipDir = 180.0 - (90.0 - RAD_TO_DEG(acos(yDist/dist)));
                  setFaultValues (nullPtr, nullPtr, nullPtr, &dipDir, &dip);
               }
               else /* (leftPoint.v < rightPoint.v) */
               {  /* Dipping to the right */
                  dipDir = 90.0 - RAD_TO_DEG(acos(yDist/dist));
                  setFaultValues (nullPtr, nullPtr, nullPtr, &dipDir, &dip);
               }
            }
            else  /* Section so work out dip */
            {
               dip = RAD_TO_DEG(asin(yDist/dist));
               setFaultValues (nullPtr, nullPtr, nullPtr, nullPtr, &dip);
                    /* work out the dip direction as well */
               if (leftPoint.v > rightPoint.v)
               {  /* Dipping to the right */
                  dipDir = 270.0;
                  setFaultValues (nullPtr, nullPtr, nullPtr, &dipDir, nullPtr);
               }
               else /* (leftPoint.v < rightPoint.v) */
               {  /* Dipping to the left */
                  dipDir = 90.0;
                  setFaultValues (nullPtr, nullPtr, nullPtr, &dipDir, nullPtr);
               }
            }

            if (refFrameDefined)  /* work out the position of event */
            {
               xPos = topLeftX + leftPoint.h*xInc;
               yPos = topLeftY + leftPoint.v*yInc;
               zPos = topLeftZ + leftPoint.v*zInc;
               setFaultValues (&xPos, &yPos, &zPos, nullPtr, nullPtr);
            }
            
         }
         break;
      case (SHEAR_ZONE):
         {
            SHEAR_OPTIONS *options;
            double dist, xDist, yDist;
            double xPos, yPos, zPos, dipDir, dip, *nullPtr = NULL;
               
              /* need ref frame to work out anything for faults */
            if (!refFrameDefined)
            {
               //xvt_dm_post_error ("Error, Reference frame not defined Properly.");
               break;
            }
              
            options = (SHEAR_OPTIONS *) traceData->object->options;
            xDist = rightPoint.h - leftPoint.h;
            yDist = bottomPoint.v - topPoint.v;
            dist = sqrt(xDist*xDist + yDist*yDist);
            
            if (fabs(zInc) < 1.0)  /* Map view so work out direction */
            {
               dipDir = 90.0 - RAD_TO_DEG(acos(yDist/dist));
               setShearValues (nullPtr, nullPtr, nullPtr, &dipDir, nullPtr);
            }
            else  /* Section so work out dip */
            {
               dip = RAD_TO_DEG(asin(yDist/dist));
               setShearValues (nullPtr, nullPtr, nullPtr, nullPtr, &dip);
            }

            if (refFrameDefined)  /* work out the position of event */
            {
               xPos = topLeftX + leftPoint.h*xInc;
               yPos = topLeftY + leftPoint.v*yInc;
               zPos = topLeftZ + leftPoint.v*zInc;
               setShearValues (&xPos, &yPos, &zPos, nullPtr, nullPtr);
            }
            
         }
         break;
   }
   
   update3dPreview (NULL_WIN, traceData->object);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        loadPictureFile
DESCRIPTION
     read in a picture file

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadPictureFile (char *filename)
#else
loadPictureFile (filename)
char *filename;
#endif
{
   WINDOW pictureWindow;
   XVT_IMAGE image;
   char title[100];
   WINDOW frontWin = (WINDOW) NULL;
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   RCT position;
 
 
   if (!(image = xvt_image_read (filename)))
   {
      //xvt_dm_post_error("Error, Could Not read Image File");
      return;
   }         
                    
   strcpy (title, filename);
   
   position.left = 0; position.top = 0;
   xvt_image_get_dimensions (image, &position.right, &position.bottom);
   
   if (!(pixmap = xvt_pmap_create (TASK_WIN, XVT_PIXMAP_DEFAULT,
                             position.right, position.bottom, 0L)))
   {
      xvt_image_destroy (image);
      //xvt_dm_post_error("Error, Not Enough Memory to Create Picture.");
      return;
   }
   xvt_dwin_clear (pixmap, COLOR_WHITE);

   xvt_dwin_draw_image (pixmap, image, &position, &position);   
   
   xvt_image_destroy (image);

   position.top += 60; position.bottom += 60;
   position.left += 60; position.right += 60;
   if (!(pictureWindow = xvt_win_create (W_DOC, &position,
                   title, (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,
                   EM_ALL, pictureEventHandler, 0L)))
   {
      //xvt_dm_post_error("Cannot Create Picture Window.");
      return;
   }

   xvt_vobj_set_data (pictureWindow, (long) pixmap);

   xvt_vobj_set_visible (pictureWindow, TRUE);
   bringWindowToFront(pictureWindow);
}

/* ======================================================================
FUNCTION        savePictureFile
DESCRIPTION
     write out a picture file

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
savePictureFile (WINDOW win, char *filename)
#else
savePictureFile (win, filename)
WINDOW win;
char *filename;
#endif
{
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   char title[100];
   XVT_PIXMAP sectionPixmap = NULL_PIXMAP;

	if (DEMO_VERSION)
	{
		//xvt_dm_post_error ("Sorry, You can not save Pictures in the Demo Version");
	   return;
	}

   if (!win)
      //xvt_dm_post_error("Error, No Window Specified. Click on it First.");
                
   if (!(pixmap = getWindowPixmap (win)))
   {
      //xvt_dm_post_error("Sorry, You Cannot Save this Window.");
      return;
   }

   xvt_vobj_get_title (win, title, 100);
   if (strstr(title, SECTION_TITLE) || strstr(title, LINEMAP_TITLE))
   {
      RCT position, pixmapSize;
      SECTION_DATA *sectionData;
      
      if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
      {
         xvt_vobj_get_client_rect (win, &position);
         if (sectionPixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL))
         {
            xvt_vobj_get_client_rect (sectionData->pixmap, &pixmapSize);

            xvt_dwin_draw_pmap (sectionPixmap, pixmap, &position, &pixmapSize);
            refreshSectionSymbols(sectionPixmap, sectionData->surfaceData, &position,
                                  sectionData->orientations, sectionData->count);
            pixmap = sectionPixmap;
         }
      }
   }

   if (strstr(title, IMAGE_TITLE))
   {
      RCT position, pixmapSize;
      ANOMIMAGE_DATA *anomImageData;
      
      if (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
      {
         xvt_vobj_get_client_rect (win, &position);
         if (sectionPixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL))
         {
            xvt_vobj_get_client_rect (pixmap, &pixmapSize);

            xvt_dwin_draw_pmap (sectionPixmap, pixmap, &position, &pixmapSize);
            overlayAnomGrid (sectionPixmap, anomImageData);
            pixmap = sectionPixmap;
         }
      }
   }

   savePixmap (pixmap, filename);
   
   if (sectionPixmap)
      xvt_pmap_destroy (sectionPixmap);
}
/* ======================================================================
FUNCTION        savePixmap
DESCRIPTION
     write out a pixmap to a file

INPUT  char *filename

OUTPUT

RETURNED   TRUE - sucess, FALSE - failure
====================================================================== */
int 
#if XVT_CC_PROTO
savePixmap (XVT_PIXMAP pixmap, char *filename)
#else
savePixmap (pixmap, filename)
XVT_PIXMAP pixmap;
char *filename;
#endif
{
   FILE *fo;
   XVT_IOSTREAM stream;
   RCT position;
/*   XVT_PALETTE palette; */
/*
   int numColors, c;
   COLOR colors[256];
*/
   XVT_IMAGE image;
   int error = FALSE;
   
   if (!pixmap)
   {
      //xvt_dm_post_error("Error, Could not get Picture to Save.");
      return (FALSE);
   }               

   xvt_vobj_get_client_rect (pixmap, &position);
      
#ifdef OLD
   if (!(palette = xvt_palet_create (XVT_PALETTE_CUBE256, NULL)))
   {
      //xvt_dm_post_error("Error, Not Enough Memory to Save Image.");
      return (FALSE);
   }
#endif
   if (!(image = xvt_image_create (XVT_IMAGE_RGB, /* CL8 */
                           position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Error, Not Enough Memory to Save Picture.");
      return (FALSE);
   }                   

#ifdef OLD
   numColors = xvt_palet_get_colors (palette, colors, 256);                                               
   xvt_image_set_ncolors (image, numColors);
   for (c = 0; c < numColors; c++)
      xvt_image_set_clut (image, c, colors[c]);
   xvt_palet_destroy (palette);
#endif
   
   xvt_image_get_from_pmap (image, pixmap, &position, &position);
   
                    /* ******************************* */
                    /* write out the picture to a file */
   if (!(fo = fopen (filename, "wb+")))
   {
      //xvt_dm_post_error("Cannot Open File to Write Picture Too.");
      xvt_image_destroy (image);
      return (FALSE);
   }

   stream = xvt_iostr_create_fwrite (fo);

                                             
   if (!xvt_image_write_bmp_to_iostr (image, stream))
                                              
   {
      ////xvt_dm_post_error("Error, %s may not be written correctly.", filename);
      error = TRUE;
   }

   xvt_iostr_destroy (stream);
   xvt_image_destroy (image);
   fclose (fo);

#if (XVTWS == MACWS)
   if (!error)    /* set the Mac file attributes for pictures */
   {
      FILE_SPEC fileSpec;
      
      xvt_fsys_get_dir (&(fileSpec.dir));
      strcpy (fileSpec.name, filename);
      xvt_fsys_set_file_attr (&fileSpec, XVT_FILE_ATTR_TYPESTR, (long) "PICT");
      xvt_fsys_set_file_attr (&fileSpec, XVT_FILE_ATTR_CREATORSTR, (long) "Nody");
   }
#endif

   return (!error);
}

/* ======================================================================
FUNCTION        loadPixmap
DESCRIPTION
     write out a pixmap to a file

INPUT  char *filename

OUTPUT

RETURNED   TRUE - sucess, FALSE - failure
====================================================================== */
XVT_PIXMAP 
#if XVT_CC_PROTO
loadPixmap (char *filename)
#else
loadPixmap (filename)
char *filename;
#endif
{
   XVT_PIXMAP pixmap = NULL_WIN;
   XVT_IMAGE image;
   RCT position;
	FILE *fi;
 
	if (!(fi = (FILE *) fopen(filename, "r")))
		return (pixmap);
	fclose(fi);  /* close the file */
 
   if (!(image = xvt_image_read (filename)))
   {
      //xvt_dm_post_error("Error, Could Not read Image File");
      return (pixmap);
   }         
                    
   position.left = 0; position.top = 0;
   xvt_image_get_dimensions (image, &position.right, &position.bottom);
   
   if (!(pixmap = xvt_pmap_create (TASK_WIN, XVT_PIXMAP_DEFAULT,
                             position.right, position.bottom, 0L)))
   {
      xvt_image_destroy (image);
      //xvt_dm_post_error("Error, Not Enough Memory to Create Picture.");
      return (pixmap);
   }
   xvt_dwin_clear (pixmap, backgroundColor);

   xvt_dwin_draw_image (pixmap, image, &position, &position);   
   
   xvt_image_destroy (image);
   
   return (pixmap);
}

/* ======================================================================
FUNCTION        getWindowPixmap
DESCRIPTION
     return the pixmap associated with a window if it has one

INPUT  WINDOW win - the window to get the pixmap for

OUTPUT

RETURNED   the pixmap, NULL if it could not be got
====================================================================== */
XVT_PIXMAP 
#if XVT_CC_PROTO
getWindowPixmap (WINDOW win)
#else
getWindowPixmap (win)
WINDOW win;
#endif
{
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   SECTION_DATA *sectionData;
   MOVIE_DATA *movieData;
   THREED_IMAGE_DATA *threedData;
   ANOMIMAGE_DATA *anomImageData;
   BLOCK_DIAGRAM_DATA *blockDiagramData;
   TRACE_DATA *traceData;
   char title[100];

   if (!win)
      return (pixmap);
         
   xvt_vobj_get_title (win, title, 100);
                
   if (strstr(title, LEGEND_TITLE))
   {
      pixmap = (XVT_PIXMAP) getLegendPixmap();
   }
   if (strstr(title, "Trace"))
   {
      if (traceData = (TRACE_DATA *) xvt_vobj_get_data (win))
         pixmap = (XVT_PIXMAP) traceData->pixmap;
   }
   if (strstr(title, "Surface Plane"))
   {
      pixmap = (XVT_PIXMAP) xvt_vobj_get_data (win);
   }
   if (strstr(title, PROFILE_FROM_IMAGE_TITLE))
   {
      if (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
         pixmap = (XVT_PIXMAP) anomImageData->pixmap;
   }
   else if (strstr(title, PROFILE_TITLE))
   {
      pixmap = (XVT_PIXMAP) xvt_vobj_get_data (win);
   }
   else if (strstr(title, BLOCK_WINDOW_TITLE))
   {
      if (blockDiagramData = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win))
         pixmap = (XVT_PIXMAP) blockDiagramData->threedData.pixmap;
   }
   else if (strstr(title, TOPOGRAPHY_3D_TITLE) || strstr(title, STRATIGRAPHY_3D_TITLE))
   {
      if (threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (win))
         pixmap = (XVT_PIXMAP) threedData->pixmap;
   }
   else if (strstr(title, WELL_LOG_TITLE)
         || strstr (title, BLOCK_WINDOW_TITLE)
         || strstr (title, CONTOUR_TITLE)      || strstr (title, TOPOGRAPHY_TITLE)
         || strstr (title, LINEMAP_PLOT_TITLE) || strstr (title, STRATIGRAPHY_TITLE)) 
   {
      pixmap = (XVT_PIXMAP) xvt_vobj_get_data (win);
   }
   else if (strstr(title, SECTION_TITLE) || strstr(title, LINEMAP_TITLE))
   {
      if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
         pixmap = (XVT_PIXMAP) sectionData->pixmap;
   }
   else if (strstr(title, MOVIE_WINDOW_TITLE))
   {
      if (movieData = (MOVIE_DATA *) xvt_vobj_get_data (win))
         pixmap = (XVT_PIXMAP) movieData->pixmap;
   }
   else if (strstr(title, IMAGE_TITLE))
   {
      if (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
         pixmap = (XVT_PIXMAP) anomImageData->pixmap;
   }

   return (pixmap);
}

/* ======================================================================
FUNCTION        printWindow
DESCRIPTION
     print the current window to a printer

INPUT  

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
printWindow (WINDOW win)
#else
printWindow (win)
WINDOW win;
#endif
{
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   char title[100];

   if (!(pixmap = getWindowPixmap (win)))
   {
      //xvt_dm_post_error("Error, Could not get Picture to Print.");
      return;
   }


   xvt_vobj_get_title (win, title, 100);
   if (strstr(title, SECTION_TITLE) || strstr(title, LINEMAP_TITLE))
   {
      RCT position, pixmapSize;
      XVT_PIXMAP sectionPixmap;
      SECTION_DATA *sectionData;
      
      if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
      {
         xvt_vobj_get_client_rect (win, &position);
         if (sectionPixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL))
         {
            xvt_vobj_get_client_rect (sectionData->pixmap, &pixmapSize);

            xvt_dwin_draw_pmap (sectionPixmap, pixmap, &position, &pixmapSize);
            refreshSectionSymbols(sectionPixmap, sectionData->surfaceData, &position,
                                  sectionData->orientations, sectionData->count);
            pixmap = sectionPixmap;
         }
      }
   }

   xvt_print_start_thread (printPicture, (long) pixmap);
}

/* ======================================================================
FUNCTION        loadHistoryFile
DESCRIPTION
     read in a histroy file

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadHistoryFile (char *filename)
#else
loadHistoryFile (filename)
char *filename;
#endif
{
   FILE_SPEC fileSpec;

   if (Load_status (filename))
   {
      invalidateCurrentLineMap ();
      strcpy (fileSpec.name, filename);
      xvt_fsys_get_dir (&(fileSpec.dir));
      setCurrentFileName (&fileSpec);
   }

   updateMenuOptions (TASK_MENUBAR, NULL_WIN);
}

/* ======================================================================
FUNCTION        saveHistoryFile
DESCRIPTION
     save a histroy file

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveHistoryFile (char *filename)
#else
saveHistoryFile (filename)
char *filename;
#endif
{
   FILE_SPEC fileSpec;

   if (report_status (filename))
   {
      strcpy (fileSpec.name, filename);
      xvt_fsys_get_dir (&(fileSpec.dir));
      setCurrentFileName (&fileSpec);
   }
}
/* ======================================================================
FUNCTION        getImportFile
DESCRIPTION
      get the filename for the imported block model
      NOTE the actual loading of the file is done with "loadImportFile"

INPUT  FILE_SPEC *fileSpec;

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
getImportFile (FILE_SPEC *fileSpec)
#else
getImportFile (fileSpec)
FILE_SPEC *fileSpec;
#endif
{
   getDefaultDirectory(&(fileSpec->dir));
   switch (xvt_dm_post_file_open(fileSpec, "Import Block from ..."))
   {
      case FL_OK:
         if (strlen(fileSpec->name) == 0)
         {
            //xvt_dm_post_error ("Error, No file Specified");
            break;
         }
         setDefaultDirectory(&(fileSpec->dir));
         return (TRUE);
         break;
      case FL_BAD:
         //xvt_dm_post_error("Error getting file name.");
         break;
      case FL_CANCEL:
         break;
   }             

   return (FALSE);   
}

/* ======================================================================
FUNCTION        loadImportFile
DESCRIPTION
     load a imported block model

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadImportFile (IMPORT_OPTIONS *options)
#else
loadImportFile (options)
IMPORT_OPTIONS *options;
#endif
{
   short ***blockData = NULL;
   DIRECTORY copyDir;
   int numLayers, **layerDimensions = NULL,
       numCubeSizes, *cubeSizes = NULL;
   double topX, topY, topZ, botX, botY, botZ;
   double inclination, intensity, declination;
   int calcRange, numProps, i, sucess = TRUE, cubeSize = 0;
   LAYER_PROPERTIES *rockProps[200];
   int densityCalc, susCalc, remCalc, aniCalc, indexCalc;
   char filename[50];

   initLongJob (0, 5,"Reading Block File...", NULL);

   xvt_fsys_get_dir (&copyDir);
   xvt_fsys_set_dir (&(options->importFile.dir));
   
   strcpy (filename, options->importFile.name);
   
   addFileExtention (filename, ANOM_HEADER_FILE_EXT);
   if (!readBlockHeaderFromFile (filename, &numLayers,
               &layerDimensions, &topX, &topY, &topZ, &botX, &botY, &botZ,
               &numCubeSizes, &cubeSizes, &calcRange, &inclination,
               &intensity, &declination, &densityCalc, &susCalc,
               &remCalc, &aniCalc, &indexCalc, &numProps, rockProps))
   {
      //xvt_dm_post_error("Could not read Block header Correctly");
      finishLongJob ();
      return (FALSE);
   }

   cubeSize = cubeSizes[0];        /* Get Finest cube Size */
   for (i = 1; i < numCubeSizes; i++)
   {
      if (cubeSizes[i] < cubeSize)
         cubeSize = cubeSizes[i];
   }
               
   if (!indexCalc)   /* Only allow importing indexed block files */
   {
      //xvt_dm_post_error("You can only Import Indexed block Files");
      finishLongJob ();
      return (FALSE);
   }
   
   incrementLongJob (INCREMENT_JOB);

   if (!(blockData = (short ***) create3DIregArray(numLayers,
                               layerDimensions, sizeof(short))))
   {
      //xvt_dm_post_error("Not enough memory, try closing some windows");
      finishLongJob ();
      return (FALSE);
   }

   incrementLongJob (INCREMENT_JOB);

   addFileExtention (filename, ANOM_INDEX_FILE_EXT);
   if (!(read3DIregBlockFromFile (filename,
                                  (char ***) blockData, numLayers,
                                  layerDimensions, sizeof(short))))
   {
      destroy3DIregArray((char ***) blockData, numLayers, layerDimensions);
      sucess = FALSE;
   }
   else   /* setup the options structure to describe the block */
   {
      if (options->blockData)
      {      
         destroy3DIregArray((char ***) options->blockData,
                     options->numLayers, options->layerDimensions);
         options->blockData = NULL;
      }
      options->blockData = blockData;
      options->numLayers = numLayers;
      options->layerDimensions = layerDimensions;
      options->blockSize = (double) cubeSize;
      options->positionX = topX;
      options->positionY = topY;
      options->positionZ = topZ;
      options->positionBy = POS_BY_CORNER;
               /* build the array of rock properties */
       
      if (options->properties)
      {      
         xvt_mem_free((char *) options->properties);
         options->properties = NULL;
         options->numProperties = 0;
      }
      if (rockProps)
      {
         for (i = 0; i < numProps; i++)
         {
            sprintf (rockProps[i]->unitName, "Rock %d", i+1);
            rockProps[i]->height = 0;

            getSpacedColorRGB (numProps, i+1, &(rockProps[i]->color));
         }
         options->properties = rockProps[0];
         options->numProperties = numProps;
      }
   }

   incrementLongJob (INCREMENT_JOB);
   incrementLongJob (INCREMENT_JOB);
   
   finishLongJob ();

   return (sucess);
}

/* ======================================================================
FUNCTION        loadTopographyFile
DESCRIPTION
     load a topography file

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadTopographyFile (FILE_SPEC *fileSpec)
#else
loadTopographyFile (fileSpec)
FILE_SPEC *fileSpec;
#endif
{
   DIRECTORY copyDir;
   FILE *topoFile = NULL;
   int fileId, jj, kk;
   int sucess = FALSE;
   double value;

              /* close the old file if it was being used */
   if (topographyMap)
   {
      free_dmatrix (topographyMap, 0, TopoCol, 0, TopoRow);
      topographyMap = NULL;
   }

   xvt_fsys_get_dir (&copyDir);
   xvt_fsys_set_dir (&(fileSpec->dir));
   if (!(topoFile = fopen (fileSpec->name, "r")))
   {
      DIRECTORY saveDir;
      
      xvt_fsys_set_dir (&copyDir);
      
      if (!findFile(fileSpec))
      {
         //xvt_dm_post_error("Error, Could Not Open File.");
         return (sucess);
      }
      xvt_fsys_get_dir (&saveDir);
      xvt_fsys_set_dir (&(fileSpec->dir));
      if (!(topoFile = fopen (fileSpec->name, "r")))
      {
         //xvt_dm_post_error("Error, Could Not Open File.");
         return (sucess);
      }
      xvt_fsys_set_dir (&saveDir);
   }
   xvt_fsys_set_dir (&copyDir);
   
   fscanf (topoFile, "%d", &fileId);
   if (fileId != 9876)
   {
      fclose (topoFile);
      topoFile = NULL;
      //xvt_dm_post_error("Error, File was not a Topography File.");
      return (sucess);
   }

   fscanf(topoFile,"%d %d",&TopoRow, &TopoCol);
   fscanf(topoFile,"%lf %lf",&TopomapXW,&TopomapYW);
   fscanf(topoFile,"%lf %lf",&TopomapXE,&TopomapYE);

   if ((topographyMap = (double **) dmatrix(0,TopoCol,0,TopoRow))==0L)
   {
      fclose (topoFile);
      topoFile = NULL;
      //xvt_dm_post_error("Not enough memory, try closing some windows");
      return (sucess);
   }

   for (jj = 1; jj <= TopoRow; jj++)
   {
      for (kk = 1; kk <= TopoCol; kk++)
      {
         fscanf(topoFile, "%lf", &value);
         if ((jj == 1) && (kk == 1))
         {
            minTopoValue = value;
            maxTopoValue = value;
         }
         if (minTopoValue > value) minTopoValue = value;
         if (maxTopoValue < value) maxTopoValue = value;
         topographyMap[kk/*(TopoCol-kk)+1*/][jj] = value;
      }
   }

   fclose (topoFile);
             /* what we have is a topo file */
   sucess = TRUE;

   memcpy (&topoFileSpec, fileSpec, sizeof(FILE_SPEC));
   return (sucess);
}

/* ======================================================================
FUNCTION        updateTopographyMap
DESCRIPTION

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateTopographyMap ()
#else
updateTopographyMap ()
#endif
{
   BOOLEAN created = FALSE;
   XVT_PIXMAP topoPixmap;
   RCT position;

   if (!(geologyOptions.useTopography && topographyMap))
   {
      //xvt_dm_post_error("Error, A Topography must be loaded in Geology Options First.");
      return;
   }
   

   getWindowStartPosition(TOPOGRAPHY_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);

   if (projectOptions.newWindowEachStage || (!topoWindow))
   {                             /* create a window if we need it */

      if (!(topoWindow = xvt_win_create (W_DOC, &position, TOPOGRAPHY_TITLE,
                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, pictureEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create Section Window.");
         return;
      }
      created = TRUE;
   }
   else
   {
      xvt_pmap_destroy ((XVT_PIXMAP) xvt_vobj_get_data (topoWindow));
      xvt_vobj_set_data (topoWindow, 0L);
   }

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the topography in a PIXMAP */
   if (!(topoPixmap = xvt_pmap_create (topoWindow, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Topography Representation.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif
      return;
   }
   xvt_dwin_clear (topoPixmap, COLOR_WHITE);

                  /* create the PIXMAP to describe the topography
                  ** and attach it to topoWindow as app_data */
   DoTopoMap (topoPixmap);


#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif

   xvt_vobj_set_data (topoWindow, (long) topoPixmap);

   if (!created)
      xvt_dwin_invalidate_rect (topoWindow, NULL);
   else
      xvt_vobj_set_visible (topoWindow, TRUE);

   bringWindowToFront(topoWindow);
}

/* ======================================================================
FUNCTION        update3dTopographyMap
DESCRIPTION

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
update3dTopographyMap ()
#else
update3dTopographyMap ()
#endif
{
   BOOLEAN created = FALSE;
   XVT_PIXMAP topoPixmap;
   THREED_IMAGE_DATA *threedData;
   RCT position;
   double topoBlockSize;

   if (!(geologyOptions.useTopography && topographyMap))
   {
      //xvt_dm_post_error("Error, A Topography must be loaded in Geology Options First.");
      return;
   }
   
   getWindowStartPosition(TOPOGRAPHY_3D_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);

   if (projectOptions.newWindowEachStage || (!topo3dWindow))
   {                             /* create a window if we need it */

      if (!(topo3dWindow = xvt_win_create (W_DOC,&position,TOPOGRAPHY_3D_TITLE,
                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, threedEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create 3D Topography Window.");
         return;
      }
      created = TRUE;
   }
   else
   {
      threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (topo3dWindow);
      xvt_pmap_destroy (threedData->pixmap);
      xvt_mem_hfree ((char __huge *) threedData->surface);
      xvt_mem_free ((char *) threedData);
      xvt_vobj_set_data (topo3dWindow, 0L);
      threedData = NULL;
   }
   setStratPalet (topo3dWindow);

   threedData = (THREED_IMAGE_DATA *) xvt_mem_zalloc(sizeof(THREED_IMAGE_DATA));
   threedData->surface = (THREED_POINT_INFO __huge *) NULL;
   topoBlockSize = (TopomapXE - TopomapXW)/TopoCol;
   threedData->focus.x = 0.0;
   threedData->focus.y = ((maxTopoValue - minTopoValue) / (topoBlockSize/20))/2;
   threedData->focus.z = 0.0;
   setCameraPosition (threedData, threedViewOptions.declination,
                                  threedViewOptions.azimuth,
                                  threedViewOptions.scale);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the topography in a PIXMAP */
   if (!(topoPixmap = xvt_pmap_create (topo3dWindow, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Topography 3D Representation.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif
      return;
   }
   xvt_dwin_clear (topoPixmap, COLOR_WHITE);
   threedData->pixmap = topoPixmap;

                  /* create the PIXMAP to describe the topography
                  ** and attach it to topoWindow as app_data */
   do3dTopoMap (threedData);


#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   finishLongJob ();  /* take it down if one was never needed */
#endif

   xvt_vobj_set_data (topo3dWindow, (long) threedData);

   if (!created)
      xvt_dwin_invalidate_rect (topo3dWindow, NULL);
   else
      xvt_vobj_set_visible (topo3dWindow, TRUE);

   bringWindowToFront(topo3dWindow);
}

/* ======================================================================
FUNCTION        update3dStratigraphy
DESCRIPTION

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
update3dStratigraphy ()
#else
update3dStratigraphy ()
#endif
{
   BOOLEAN created = FALSE;
   XVT_PIXMAP stratPixmap;
   THREED_IMAGE_DATA *threedData;
   RCT position;
   FILE_SPEC fileSpec;
   char message[50];

   if ((threedViewOptions.fillType == DXF_FILE_FILL_3DFACE) ||
       (threedViewOptions.fillType == DXF_FILE_FILL_POLYLINE) ||
       (threedViewOptions.fillType == VULCAN_FILE_FILL))
   {
      if ((threedViewOptions.fillType == DXF_FILE_FILL_3DFACE) ||
          (threedViewOptions.fillType == DXF_FILE_FILL_POLYLINE))
      {
         strcpy (message, "DXF Output File ...");
         strcpy (fileSpec.name, "untitled.dxf");
         strcpy (fileSpec.type, "dxf");
      }
      else if (threedViewOptions.fillType == VULCAN_FILE_FILL)
      {
         strcpy (message, "Vulcan Output File ...");
         strcpy (fileSpec.name, "untitled.vul");
         strcpy (fileSpec.type, "vul");
      }
#if (XVTWS == MACWS)
         strcpy (fileSpec.type, "TEXT");
#endif

      getDefaultDirectory(&(fileSpec.dir));
      switch (xvt_dm_post_file_save(&fileSpec, message))
      {
         case FL_OK:
            if (strlen(fileSpec.name) == 0)
            {
               //xvt_dm_post_error ("Error, No file Specified");
               break;
            }
            setDefaultDirectory(&(fileSpec.dir));
            break;
         case FL_BAD:
            //xvt_dm_post_error("Error getting file name.");
            break;
         case FL_CANCEL:
            break;
      }             
      if (strlen(fileSpec.name) == 0)
         return;                 
         
      xvt_fsys_set_dir (&fileSpec.dir);
      do3dStratMap ((THREED_IMAGE_DATA *) NULL, fileSpec.name);
      
      return;
   }

   getWindowStartPosition(STRATIGRAPHY_3D_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);

   if (projectOptions.newWindowEachStage || (!strat3dWindow))
   {                             /* create a window if we need it */

      if (!(strat3dWindow = xvt_win_create (W_DOC, &position,
                                                        STRATIGRAPHY_3D_TITLE,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, threedEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create 3D Stratigraphy Window.");
         return;
      }
      created = TRUE;
   }
   else
   {
      xvt_vobj_get_client_rect (strat3dWindow, &position);  /* get size of window for pixmap - may have be sized since creation */
      threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (strat3dWindow);
      xvt_vobj_set_data (strat3dWindow, 0L);
      xvt_pmap_destroy (threedData->pixmap);
      xvt_mem_hfree ((char __huge *) threedData->surface);
      xvt_mem_free ((char *) threedData);
      threedData = NULL;
   }
   setStratPalet (strat3dWindow);

   threedData = (THREED_IMAGE_DATA *)xvt_mem_zalloc(sizeof(THREED_IMAGE_DATA));
   threedData->surface = (THREED_POINT_INFO __huge *) NULL;
   threedData->surfaceWidth = 0;
   threedData->surfaceHeight = 0;
   threedData->focus.x = threedData->focus.y = threedData->focus.z = 0.0;
   setCameraPosition (threedData, threedViewOptions.declination+180,
                                  threedViewOptions.azimuth,
                                  threedViewOptions.scale);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the topography in a PIXMAP */
   if (!(stratPixmap = xvt_pmap_create (strat3dWindow, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Topography 3D Representation.");
      xvt_mem_free ((char *) threedData);
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif
      return;
   }
   xvt_dwin_clear (stratPixmap, backgroundColor);
   threedData->pixmap = stratPixmap;

                  /* create the PIXMAP to describe the stratigraphy */
   do3dStratMap (threedData, (char *) NULL);

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   finishLongJob ();  /* take it down if one was never needed */
#endif

   xvt_vobj_set_data (strat3dWindow, (long) threedData);

   if (!created)
      xvt_dwin_invalidate_rect (strat3dWindow, NULL);
   else
      xvt_vobj_set_visible (strat3dWindow, TRUE);

   bringWindowToFront(strat3dWindow);
}

static int
#if XVT_CC_PROTO
noPreviewAvailable (WINDOW previewWin, char *message)
#else
noPreviewAvailable (previewWin, message)
WINDOW previewWin;
char *message;
#endif
{
   char *label;
   char defaultLabel[50];
   RCT previewRect;
   int textWidth, ascent, descent;
   
   if (!previewWin)
      return (FALSE);
               
   xvt_dwin_clear (previewWin, backgroundColor);
   xvt_vobj_get_client_rect (previewWin, &previewRect);
               
   xvt_dwin_set_fore_color(previewWin, COLOR_LTGRAY);

   strcpy (defaultLabel, "No Preview Available");
   label = defaultLabel;
   textWidth = xvt_dwin_get_text_width(previewWin, label, -1);
   xvt_dwin_draw_text (previewWin, xvt_rect_get_width(&previewRect)/2 - textWidth/2,
                                   xvt_rect_get_height(&previewRect)/2, label, -1);

   if (message)
   {
      label = message;
      xvt_dwin_get_font_metrics(previewWin, NULL, &ascent, &descent);
      textWidth = xvt_dwin_get_text_width(previewWin, label, -1);
      xvt_dwin_draw_text (previewWin, xvt_rect_get_width(&previewRect)/2 - textWidth/2,
                                      xvt_rect_get_height(&previewRect)/2 + ascent + descent + 1, label, -1);
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        update3dPreview
DESCRIPTION

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
update3dPreview (WINDOW prevWin, OBJECT *object)
#else
update3dPreview (prevWin, object)
WINDOW prevWin;
OBJECT *object;
#endif
{
   WINDOW previewWin = NULL_WIN;
   double posX, posY, posZ, dip, dipDir, pitch;
   THREED_IMAGE_DATA threedData;
   int drawPitch = TRUE, previewOk = FALSE;
   BLOCK_DIAGRAM_DATA blockDiagram;
   double declination, azimuth, scale;
   int previewType, previewTypeOptions;
   WINDOW previewTypeWin, previewTypeOptionsWin;
   OBJECT *stratObject, *stratNext;
   BLOCK_VIEW_OPTIONS *copyBlockView = NULL, *blockView, tempView;
   THREED_VIEW_OPTIONS *copy3DViewOptions = NULL;
                        /* Get the preview Window */
   if (object && ((long) object > 100))  /* any real address will be greater than 100 */
      previewWin = object->previewWin;
   else
   {
		object = NULL;
      if (prevWin)
         previewWin = prevWin;
      else
         previewWin = currentPreviewWindow;
   }
      
   if (!previewWin)
      return;
                            /* Only update if preview is turned On */
   if (!xvt_ctl_is_checked (xvt_win_get_ctl(previewWin, PREVIEW_UPDATE)))
      return;

#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
   xvt_win_set_cursor (TASK_WIN, CURSOR_WAIT);
#endif      

   blockView = getViewOptions ();
   if (!object)
   {
      WINDOW previewParent = xvt_vobj_get_parent(previewWin);
      char title[50];
      xvt_vobj_get_title(previewParent, title, 50);
      if (strstr(title, "Block"))
      {
         if (!(copyBlockView = (BLOCK_VIEW_OPTIONS *) xvt_mem_zalloc(sizeof(BLOCK_VIEW_OPTIONS))))
         {
#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
            xvt_win_set_cursor (TASK_WIN, CURSOR_ARROW);
#endif      
            return;
         }
         memcpy (copyBlockView, blockView, sizeof(BLOCK_VIEW_OPTIONS));
         saveViewOptions (previewParent);  /* Preview is for Block View Options window */
      }
      else if (strstr(title, "3D"))
      {
         if (!(copy3DViewOptions = (THREED_VIEW_OPTIONS *) xvt_mem_zalloc(sizeof(THREED_VIEW_OPTIONS))))
         {
#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
            xvt_win_set_cursor (TASK_WIN, CURSOR_ARROW);
#endif      
            return;
         }
         memcpy (copy3DViewOptions, &threedViewOptions, sizeof(BLOCK_VIEW_OPTIONS));
         save3dOptions (previewParent);
      }
      else if ((strcmp(title, "Options") == 0) || strstr(title, "- Options"))
      {
         saveOptions (NULL_WIN, UNKNOWN_OP, TRUE);
      }
   }
   
   


   previewTypeWin = xvt_win_get_ctl(previewWin, PREVIEW_TYPE);
   previewTypeOptionsWin = xvt_win_get_ctl(previewWin, PREVIEW_TYPE_OPTIONS);
   previewType = xvt_list_get_sel_index(previewTypeWin);
   previewTypeOptions = xvt_list_get_sel_index(previewTypeOptionsWin);
   switch (previewType)
   {
      case (0):  /* Plane quick preview */
         if (!object)
         {
            previewOk = TRUE;
         }
         else
         {
            switch (object->shape)
            {
               case (STRATIGRAPHY):
                  createStratLayersPicture (previewWin, object);
#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
                  xvt_win_set_cursor (TASK_WIN, CURSOR_ARROW);
#endif      
                  return;
                  break;
               case (FOLD):
                  previewOk = getFoldPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (FAULT):
                  previewOk = getFaultPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (UNCONFORMITY):
                  if (!(previewOk = getUnconformityPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch)))
                  {
                     createStratLayersPicture (previewWin, object);
#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
                     xvt_win_set_cursor (TASK_WIN, CURSOR_ARROW);
#endif      
                     return;
                  }
                  drawPitch = FALSE;
                  break;
               case (SHEAR_ZONE):
                  previewOk = getShearPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (DYKE):
                  previewOk = getDykePlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (PLUG):
                  previewOk = getPlugPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (STRAIN):
                  previewOk = getStrainPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (TILT):
                  previewOk = getTiltPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (LINEATION):
                  previewOk = getLineationPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (FOLIATION):
                  previewOk = getFoliationPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (IMPORT):
                  previewOk = getImportPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
               case (GENERIC):
                  previewOk = getGenericPlane (object, &posX, &posY, &posZ, &dip, &dipDir, &pitch);
                  break;
            }
         }
                  
         if (!previewOk)  /* Dont have a preview for this event */
         {
            noPreviewAvailable (previewWin, NULL);
            break;
         }

         memset ((char *) &threedData, 0, sizeof(THREED_IMAGE_DATA));
         threedData.pixmap = previewWin;
         setCameraPosition (&threedData, threedViewOptions.declination,
                                         threedViewOptions.azimuth,
                                         threedViewOptions.scale*DEFAULT_LPREVIEW_SCALING);
                   /*         (double) 150, (double) 15, (double) 0.7); */
         
         do3dPreview (&threedData, blockView, object,
                      posX, posY, posZ, dip, dipDir, pitch, drawPitch);
                      
         break;
      case (1):  /* Block Diagram */
         {
            if (countObjects(NULL_WIN) == 0)
            {
               noPreviewAvailable (previewWin, "Create a Base Stratigraphy");
               break;
            }
            if (object)
            {
               if (!saveEventOptions (NULL_WIN, object, TRUE))
               {
                  noPreviewAvailable (previewWin, "Please Check your Values");
                  break;
               }
            }
                     
            noStatusWin = TRUE;
         
            if ((previewTypeOptions == 0) && (!object || (object->shape != STRATIGRAPHY)))
            {                                 /* fudge the 1 event history */
               stratObject = nthObject (NULL_WIN, 0);
               stratNext = stratObject->next;  /* keep were it was for undo */
               stratObject->next = object;
            }
            else if ((previewTypeOptions == 2) && object)
            {                                 /* fudge the full event history */
               stratNext = object->next;      /* keep were it was for undo */
               object->next = object->next->next;
            }
                                    /* Draw the preview */
            memset ((char *) &blockDiagram, 0, sizeof(BLOCK_DIAGRAM_DATA));
            initBlockDiagramData (NULL_WIN, &blockDiagram, 1.0/PREVIEW_SIZE_RATIO);
            blockDiagram.threedData.pixmap = previewWin;
            getCameraPosition (&blockDiagram.threedData, &declination, &azimuth, &scale);
            setCameraPosition (&blockDiagram.threedData, threedViewOptions.declination,
                               threedViewOptions.azimuth, threedViewOptions.scale*PREVIEW_SIZE_RATIO);
            DoBlockDiagram (&blockDiagram, TRUE);
                  
            if ((previewTypeOptions == 0) && (!object || (object->shape != STRATIGRAPHY)))
               stratObject->next = stratNext; /* undo fudge of 1 event history */
            else if ((previewTypeOptions == 2) && object)
               object->next = stratNext; /* Undo fudge of full event history */
         
            freeBlockDiagramData (&blockDiagram, FALSE);
            noStatusWin = FALSE;
            break;
         }
         break;
      case (2):  /* Gravity */
      case (3):  /* Magnetic */
      case (4):  /* Gravity & Linemap*/
      case (5):  /* Magnetic & Linemap*/
      case (6):  /* Gravity Diff */
      case (7):  /* Magnetic Diff */
         {
            DOUBLE_2D_IMAGE anomImage;
            
            if (geophysicsOptions.calculationMethod == SPATIAL_FULL)
				{                  /* No preview for Full Spatial as it takes too long */
					noPreviewAvailable (previewWin, "Full Spatial method too Slow");
					break;
				}

            if (countObjects(NULL_WIN) == 0)
               break;
            anomImage.data = NULL;

            if (object)
               saveEventOptions (NULL_WIN, object, TRUE);
            noStatusWin = TRUE;  /* Turn OFF Status Window of Job Completion */
   
            if ((previewTypeOptions == 0) && (!object || (object->shape != STRATIGRAPHY)))
            {                                 /* fudge the 1 event history */
               stratObject = nthObject (NULL_WIN, 0);
               stratNext = stratObject->next;  /* keep were it was for undo */
               stratObject->next = object;
            }
            else if ((previewTypeOptions == 2) && object)
            {                                 /* fudge the full event history */
               stratNext = object->next;      /* keep were it was for undo */
               object->next = object->next->next;
            }
                                    /* Calculate Anomalies Image */
            memcpy (&tempView, blockView, sizeof(BLOCK_VIEW_OPTIONS));
            tempView.geophysicsCubeSize /= PREVIEW_SIZE_RATIO;

            if ((previewType == 2) || (previewType == 4) || (previewType == 6))
               doGeophysics (ANOM, &tempView, &geophysicsOptions, (char *) NULL, (char *) NULL,
                          (double **) NULL, 0, (FILE_SPEC *) NULL, NULL, &anomImage);
            else if ((previewType == 3) || (previewType == 5) || (previewType == 7))
               doGeophysics (ANOM, &tempView, &geophysicsOptions, (char *) NULL, (char *) NULL,
                          (double **) NULL, 0, (FILE_SPEC *) NULL, &anomImage, NULL);
                          
            if ((previewType == 6) || (previewType == 7)) /* Take Difference */
            {
               ANOMIMAGE_DATA anomImageData, resultImageData;
               int success;
               
               anomImageData.imageData = anomImage.data;
               anomImageData.dataXSize = anomImage.dim2;
               anomImageData.dataYSize = anomImage.dim1;
               anomImageData.geoXStart = tempView.originX + projectOptions.easting;
               anomImageData.geoYStart = tempView.originY + projectOptions.northing;
               anomImageData.geoXEnd = anomImageData.geoXStart + tempView.lengthX;
               anomImageData.geoYEnd = anomImageData.geoYStart + tempView.lengthY;
               if (previewType == 6)  /* Gravity */
                  success = calcImageDifference (&anomImageData, gravityReferenceData, &resultImageData);
               else
                  success = calcImageDifference (&anomImageData, magneticsReferenceData, &resultImageData);
                      
               destroy2DArray ((char **) anomImage.data, anomImage.dim1, anomImage.dim2);
               anomImage.data = NULL;
               anomImage.dim1 = anomImage.dim2 = 0;
               if (success)
               {
                  anomImage.data = resultImageData.imageData;
                  anomImage.dim1 = resultImageData.dataYSize;
                  anomImage.dim2 = resultImageData.dataXSize;
               }
            }
                              
            if (anomImage.data)      /* Draw Anomalies Image */
            {
               XVT_PIXMAP pixmap;
               NODDY_COLOUR lut[256];
               RCT sourceRect, destRect, previewRect;
               double interpolate;
               int minValue = 0, maxValue = 255, x, y;
               double clipMinValue, clipMaxValue;
               char label[50];
               int textAscent, textDecent;

                                    /* Make sure that image fits inside preview */
               xvt_vobj_get_client_rect (previewWin, &previewRect);
               if (anomImage.dim2 > anomImage.dim1)
                  interpolate = floor (((double) xvt_rect_get_width(&previewRect)) / ((double) anomImage.dim2));
               else
                  interpolate = floor (((double) xvt_rect_get_height(&previewRect)) / ((double) anomImage.dim1));

               clipMinValue = clipMaxValue = anomImage.data[0][0];
               for (x = 0; x < anomImage.dim1; x++)  /* Find image Max/Min */
               {
                  for (y = 0; y < anomImage.dim2; y++)
                  {
                     if (anomImage.data[x][y] < clipMinValue)
                        clipMinValue = anomImage.data[x][y];
                     else if (anomImage.data[x][y] > clipMaxValue)
                        clipMaxValue = anomImage.data[x][y];
                  }
               }
               assignGreyLut(lut, NULL, 256, 26, (int) minValue, (int) maxValue);
               if (pixmap = drawDoubleImage(previewWin, anomImage.data,
                                clipMinValue, clipMaxValue, anomImage.dim2,
                                anomImage.dim1 , lut, interpolate))
               {
                  if ((previewType == 4) || (previewType == 5))  /* Geology Linemap Overlay */
                  {
                     BLOCK_SURFACE_DATA sectionSurface;
                                                /* Create Top block surface */
                     sectionSurface.xStart = blockView->originX;
                     sectionSurface.xEnd = blockView->originX + blockView->lengthX;
                     sectionSurface.yStart = blockView->originY;
                     sectionSurface.yEnd = blockView->originY + blockView->lengthY;
                     sectionSurface.zStart = blockView->originZ;
                     sectionSurface.zEnd = blockView->originZ;
                     sectionSurface.dip = 180.0; sectionSurface.dipDirection = 90.0;
                     sectionSurface.forwardCheck = FORWARD_GREATER;
                     sectionSurface.depth = (short) 2;
                                                /* Draw Linemap on existing pixmap */
                     createLineSection(previewWin, &sectionSurface, -1.0, -1, pixmap);
                  }
                                       /* Position Pixmap in middle of Preview Window */
                  xvt_vobj_get_client_rect (pixmap, &sourceRect);
                  destRect = sourceRect;
                  destRect.top = (xvt_rect_get_height(&previewRect) - xvt_rect_get_height(&sourceRect))/2;
                  destRect.left = (xvt_rect_get_width(&previewRect) - xvt_rect_get_width(&sourceRect))/2;
                  destRect.bottom += destRect.top;
                  destRect.right += destRect.left;
                  xvt_dwin_clear (previewWin, backgroundColor);
                  xvt_dwin_draw_pmap (previewWin, pixmap, &destRect, &sourceRect);
                  xvt_vobj_set_attr (previewWin, ATTR_SUPPRESS_UPDATE_CHECK, TRUE);
                  xvt_pmap_destroy (pixmap);
                  xvt_vobj_set_attr (previewWin, ATTR_SUPPRESS_UPDATE_CHECK, FALSE);

                              /* Write image Ranage in the corner */
                  xvt_dwin_set_fore_color(previewWin, COLOR_RED);
                  xvt_dwin_get_font_metrics(previewWin, NULL, &textAscent, &textDecent);
                  sprintf (label, "Min: %lf",clipMinValue);
                  xvt_dwin_draw_text (previewWin, 2, textAscent+2, label, -1);
                  sprintf (label, "Max: %lf",clipMaxValue);
                  xvt_dwin_draw_text (previewWin, 2, textAscent+textAscent+4, label, -1);
               }
            }
            else  /* Error calculating the information */
               noPreviewAvailable (previewWin, NULL);

            if ((previewTypeOptions == 0) && (!object || (object->shape != STRATIGRAPHY)))
               stratObject->next = stratNext; /* undo fudge of 1 event history */
            else if ((previewTypeOptions == 2) && object)
               object->next = stratNext; /* Undo fudge of full event history */

            if (anomImage.data)      /* Free Memory Associated with image */
               destroy2DArray ((char **) anomImage.data, anomImage.dim1, anomImage.dim2);
            
            noStatusWin = FALSE;  /* Turn ON Status Window of Job Completion */
         }
         break;
   }

   if (!object)   /* restore block view options */
   {
      if (copyBlockView)
      {
         memcpy (blockView, copyBlockView, sizeof(BLOCK_VIEW_OPTIONS));
         xvt_mem_free ((char *) copyBlockView);
      }
      if (copy3DViewOptions)
      {
         memcpy (&threedViewOptions, copy3DViewOptions, sizeof(THREED_VIEW_OPTIONS));
         xvt_mem_free ((char *) copy3DViewOptions);
      }
   }

#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
      xvt_win_set_cursor (TASK_WIN, CURSOR_ARROW);
#endif      

}

/* ======================================================================
FUNCTION        load3dSurface
DESCRIPTION

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
load3dSurface (FILE_SPEC *fileSpec, OBJECT *object, DXFHANDLE *dxfHandle)
#else
load3dSurface (fileSpec, object, dxfHandle)
FILE_SPEC *fileSpec;
OBJECT *object;
DXFHANDLE *dxfHandle;
#endif
{
   DXFHANDLE *dxfData;
   int result = FALSE;
   double rangeX, scaleX, rangeY, scaleY, rangeZ, scaleZ;
   ftype moveMat[4][4], scaleMat[4][4], resultMat[4][4];
   
   if (dxfHandle)
      dxfData = dxfHandle;
   else
   {
      if (!(dxfData = getDxfSurfaceStructure (object)))
         return (FALSE);
   }
      
   xvt_fsys_set_dir(&(fileSpec->dir));
   DXFfree(dxfData); /* Free anything that is already there */
   DXFread( dxfData, fileSpec->name );
   if (dxfData->resultFlags)
      //xvt_dm_post_error ("Error, could not load %s",fileSpec->name);

   if (dxfData->resultFlags & fFATALMASK)
   {
      DXFfree(dxfData);
   }
   else
   {            /* success loading dxf file */
      result = TRUE;
      
         /* convert for use by surface moduel */
      DXFsurfConv(dxfData, aCentroid, NULL, (int) TRUE);

         /* work out a scale factor so data width is between 0 */
      rangeX = dxfData->max[0] - dxfData->min[0];
      rangeY = dxfData->max[1] - dxfData->min[1];
      rangeZ = dxfData->max[2] - dxfData->min[2];
      scaleX = 1000.0 / rangeX;
      scaleY = 1000.0 / rangeY;
      scaleZ = 1000.0 / rangeZ;
      dxfData->aspect[0] = (ftype) rangeY;
      dxfData->aspect[1] = (ftype) rangeX;
      dxfData->aspect[2] = (ftype) rangeZ;

      translateBy(moveMat, (ftype) -dxfData->min[0], (ftype) -dxfData->min[1], (ftype) -dxfData->min[2]);
      scaleBy(scaleMat, (ftype) scaleX, (ftype) scaleY, (ftype) scaleZ);
      multiplyMatrix (resultMat, scaleMat, moveMat);

      DXFsurfConv(dxfData, aCentroid, resultMat, (int) TRUE);
      
      if (object)    /* Store the filename and path */
      {
         FILE_SPEC *filename;
         
         if (filename = getDxfSurfaceFilename (object))
            memcpy ((char *) filename, (char *) fileSpec, sizeof (FILE_SPEC));
      }
   }

   return (result);
}


/* ======================================================================
FUNCTION        view3dSurface
DESCRIPTION
     display the 3d surface in the window given

INPUT  WINDOW win
       OBJECT *object

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
view3dSurface (WINDOW win, OBJECT *object)
#else
view3dSurface (win, object)
WINDOW win;
OBJECT *object;
#endif
{
   NODDY_COLOUR lut[256];
   XVT_IMAGE image = (XVT_IMAGE) NULL;
   DXFHANDLE *dxfData;
   XVT_PIXMAP pixmap;
   double stepX, stepY;
   int across, down, x, y, value;
   double sizeX, sizeY, ratio;
   ftype height, rangeHeight, minHeight, minX, minY;
   RCT diagRect, rect;
   CBRUSH *brush;
   DRAW_CTOOLS tools;
   char title[100];
   FILE_SPEC *filename;

   if (!(filename = getDxfSurfaceFilename (object)))
      return (FALSE);
   sprintf (title,"Surface Plane - %s", filename->name);
   xvt_vobj_set_title (win, title);
   
   if (!(dxfData = getDxfSurfaceStructure (object)))
      return (FALSE);
      
   assignGreyLut (lut, NULL, 256, 26, 0, 256);
           /* set the window size based on the x dim of the
           ** window and the aspect of the surface */
   xvt_vobj_get_client_rect (win, &diagRect);
   down = xvt_rect_get_width (&diagRect);
   
   sizeX = dxfData->max[0] - dxfData->min[0];
   sizeY = dxfData->max[1] - dxfData->min[1];
   if ((sizeX < TOLERANCE) || (sizeY < TOLERANCE))
   {
      //xvt_dm_post_error ("Error, Surface has 0 dimension.");
      return (FALSE);
   }
   
   ratio = dxfData->aspect[0]/dxfData->aspect[1]; /* Y/X aspect ratio */
   across = (int) floor ((double) (down * ratio));
   xvt_rect_set_width (&diagRect, (short) across);
   
   if (!(pixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                                  (short) across, (short) down, NULL)))
      return (FALSE);

   if (!(image = xvt_image_create (XVT_IMAGE_RGB, (short) across, (short) down, NULL)))
   {
      xvt_pmap_destroy (pixmap);
      //xvt_dm_post_error("Cannot Create Raster Image.");
      return (FALSE);
   }
#ifdef OLD
   if (!(palet = xvt_palet_create (XVT_PALETTE_USER, NULL)))
   {
      xvt_pmap_destroy (pixmap);
      xvt_image_destroy (image);
      //xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }   
   xvt_palet_set_tolerance (palet, 0L);  /* disable closness of colors */

   for (arrayPixel = 0; arrayPixel < 255; arrayPixel++)
   {
      color = XVT_MAKE_COLOR (lut[arrayPixel].red, lut[arrayPixel].green,
                                                   lut[arrayPixel].blue);
      xvt_palet_add_colors (palet, &color, 1);
   }
#endif

   xvt_dwin_get_draw_ctools (win, &tools);
   tools.pen.width = 0;
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.mode = M_COPY;
   xvt_dwin_set_draw_ctools (win, &tools);
   brush = &(tools.brush);
   xvt_dwin_clear (pixmap, backgroundColor);
   
                            /* generate & display heights */
   minX = dxfData->min[0];
   minY = dxfData->min[1];
   minHeight = dxfData->min[2];
   rangeHeight = dxfData->max[2] - dxfData->min[2];

   stepX = sizeX / across;
   stepY = sizeY / down;
   xvt_rect_set (&rect, 0, 0, 3, 3);
   
   for( x = 0; x < across; x++ )
   {
      for( y = 0; y < down; y++ )
      {
         if( DXFsurfHeight( dxfData, 0, (ftype) (y*stepY + minY),
                  (ftype) (x*stepX + minX), (ftype) 0, &height ) )
         {
            value = lut[(int) floor ((double) ((height-minHeight)/rangeHeight)*255.0)].red;
            xvt_image_set_pixel (image, (short) (across-x-1), (short) y,
                      XVT_MAKE_COLOR (value, value, value));
         }
         else
            xvt_image_set_pixel (image, (short) (across-x-1), (short) y, COLOR_BLACK);
      }
   }

   xvt_dwin_draw_image (pixmap, image, &diagRect, &diagRect);
/*   xvt_palet_destroy (palet); */
   xvt_image_destroy (image);

   xvt_vobj_set_data (win, (long) pixmap);
   CORRECT_WIN_RESIZE (win, diagRect)
   xvt_vobj_move (win, &diagRect);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        saveOrientationFile
DESCRIPTION
     save a orientation file

INPUT  char *filename

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveOrientationFile (WINDOW win, char *filename)
#else
saveOrientationFile (win, filename)
WINDOW win;
char *filename;
#endif
{
   WINDOW oriWindow = NULL_WIN;
   SECTION_DATA *lineMapData;
   char title[100];

   if (win)
      if (xvt_vobj_get_title (win, title, 100))  /* first try window with menu */
         if (strstr(title, LINEMAP_TITLE) || strstr(title, SECTION_TITLE))
            oriWindow = (WINDOW) win;

   if (oriWindow == NULL_WIN)
   {
      //xvt_dm_post_error("Error, There is currently no Line Map window Active");
      return;
   }
   
   if (!(lineMapData = (SECTION_DATA *) xvt_vobj_get_data (oriWindow)))
   {
      //xvt_dm_post_error("Error, Could not get any Orientation Data to save");
      return;
   }

   SaveOrientations (filename, lineMapData->orientations, lineMapData->count);
}

/* ======================================================================
FUNCTION        plotOrientations
DESCRIPTION
     create the orientation plot picture

INPUT  

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
plotOrientations (WINDOW win)
#else
plotOrientations (win)
WINDOW win;
#endif
{
   WINDOW oriWindow = NULL_WIN;
   SECTION_DATA *lineMapData;
   BOOLEAN created = FALSE;
   XVT_PIXMAP lineMapPlotPixmap;
   RCT position;
   char title[100];

   if (win)
      if (xvt_vobj_get_title (win, title, 100))  /* first try window with menu */
         if (strstr(title, LINEMAP_TITLE) || strstr(title, SECTION_TITLE))
            oriWindow = (WINDOW) win;

   if (oriWindow == NULL_WIN)
   {
      //xvt_dm_post_error("Error, There is currently no Line Map window Active");
      return;
   }
   
   if (!(lineMapData = (SECTION_DATA *) xvt_vobj_get_data (oriWindow)))
   {
      //xvt_dm_post_error("Error, Could not get any Orientation Data to Plot");
      return;
   }
   else
   {
      if (lineMapData->count <= 0)
      {
         //xvt_dm_post_error("No Orientation data to be plotted");
         return;
      }
   }

   position.top = LINEMAP_PLOT_POS_Y; position.left = LINEMAP_PLOT_POS_X;
   position.bottom = position.top + LINEMAP_PLOT_HEIGHT;
   position.right = position.left + LINEMAP_PLOT_WIDTH;


   if (projectOptions.newWindowEachStage || (!lineMapPlotWindow))
   { 
                            /* create a window if we need it */
      if (!(lineMapPlotWindow = xvt_win_create (W_DOC, &position,
                                               LINEMAP_PLOT_TITLE,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, pictureEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create Orientation Plot Window.");
         return;
      }
      created = TRUE;
   }
   else
   {
      xvt_pmap_destroy ((XVT_PIXMAP) xvt_vobj_get_data (lineMapPlotWindow));
      xvt_vobj_set_data (lineMapPlotWindow, 0L);
   }
   setStratPalet (lineMapPlotWindow);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the Line map in a PIXMAP */
   if (!(lineMapPlotPixmap = xvt_pmap_create (lineMapPlotWindow, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Orientations Plot Representation.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif
      return;
   }
   xvt_dwin_clear (lineMapPlotPixmap, COLOR_WHITE);

                  /* create the PIXMAP to describe the line map
                  ** and attach it to lineMapPlotWindow as app_data */
   DrawStereoNet (lineMapPlotPixmap, lineMapData->orientations,
                                     lineMapData->count);


#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif

   xvt_vobj_set_data (lineMapPlotWindow, (long) lineMapPlotPixmap);

   if (!created)
      xvt_dwin_invalidate_rect (lineMapPlotWindow, NULL);
   else
      xvt_vobj_set_visible (lineMapPlotWindow, TRUE);

   bringWindowToFront(lineMapPlotWindow);
}

/* ======================================================================
FUNCTION        updateStratColumns
DESCRIPTION
     update the display of the stratigraphy columns picture

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateStratColumns ()
#else
updateStratColumns ()
#endif
{
#define STRAT_COLUMN_WIDTH        230
#define NUM_SINGLE_UNITS_IN_STRAT 8
   BOOLEAN created = FALSE;
   XVT_PIXMAP stratPixmap;
   int totalWidthStrat = 0, totalDykes = 0, totalPlugs = 0;
   int totalWidth = 0;
   RCT position;
   OBJECT *object;
   WINDOW eventDrawWindow = (WINDOW) getEventDrawingWindow ();
   WINDOW_INFO *wip;

   wip = get_win_info(eventDrawWindow);

                      /* count num of columns to work out window width */
   for (object = wip->head; object != NULL; object = object->next)
   {
      if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY) || (object->shape == IMPORT))
         totalWidthStrat += STRAT_COLUMN_WIDTH;
      if (object->shape == DYKE)
         totalDykes++;
		if (object->shape == PLUG)
         totalPlugs++;
   }
	totalWidth = totalWidthStrat;
	if (totalDykes > 0)
	{
		totalWidth += (totalDykes/NUM_SINGLE_UNITS_IN_STRAT)*STRAT_COLUMN_WIDTH;
		if (totalDykes%NUM_SINGLE_UNITS_IN_STRAT) totalWidth += STRAT_COLUMN_WIDTH;
	}
	if (totalPlugs > 0)
	{
		totalWidth += (totalPlugs/NUM_SINGLE_UNITS_IN_STRAT)*STRAT_COLUMN_WIDTH;
		if (totalPlugs%NUM_SINGLE_UNITS_IN_STRAT) totalWidth += STRAT_COLUMN_WIDTH;
	}

   getWindowStartPosition(STRATIGRAPHY_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);
   position.right = position.left + totalWidth;

   if (projectOptions.newWindowEachStage || (!stratColumnsWindow))
   { 
                            /* create a window if we need it */
      if (!(stratColumnsWindow = xvt_win_create (W_DOC, &position,
                                                STRATIGRAPHY_TITLE,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, pictureEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create Stratigraphy Columns Window.");
         return;
      }
      created = TRUE;
   }
   else
   {
      xvt_pmap_destroy ((XVT_PIXMAP) xvt_vobj_get_data (stratColumnsWindow));
      xvt_vobj_set_data (stratColumnsWindow, 0L);
   }
   setStratPalet (stratColumnsWindow);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the Strat Columns in a PIXMAP */
   if (!(stratPixmap = xvt_pmap_create (stratColumnsWindow, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Stratigraphy Columns Representation.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif
      return;
   }
   xvt_dwin_clear (stratPixmap, COLOR_WHITE);

                  /* create the PIXMAP to describe the line map
                  ** and attach it to stratColumnsWindow as app_data */
   plotStratColumns (stratPixmap);


#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif

   xvt_vobj_set_data (stratColumnsWindow, (long) stratPixmap);

   if (!created)
      xvt_dwin_invalidate_rect (stratColumnsWindow, NULL);
   else
      xvt_vobj_set_visible (stratColumnsWindow, TRUE);

   bringWindowToFront(stratColumnsWindow);
}

/* ======================================================================
FUNCTION        updateLineMap
DESCRIPTION
     update the display of the linemap diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateLineMap (int solidColor)
#else
updateLineMap (solidColor)
int solidColor;
#endif
{
   SECTION_DATA *lineMapData;
   BOOLEAN created = FALSE;
   XVT_PIXMAP lineMapPixmap;
   RCT position;
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   
   getWindowStartPosition(LINEMAP_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);

   if (projectOptions.newWindowEachStage || (!lineMapWindow))
   { 
      if (!(lineMapData = (SECTION_DATA *) xvt_mem_zalloc (sizeof(SECTION_DATA))))
      {
         //xvt_dm_post_error("Not enough memory to create a Line Map Window.");
         return;
      }
                            /* create a window if we need it */
      if (!(lineMapWindow = xvt_win_create (W_DOC, &position,
                   LINEMAP_TITLE, MENU_BAR_2, TASK_WIN,
                   WSF_CLOSE | WSF_SIZE | WSF_INVISIBLE |
                   WSF_ICONIZABLE | WSF_HSCROLL | WSF_VSCROLL,
                   EM_ALL, lineMapEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create Line Map Window.");
         return;
      }
      created = TRUE;
   }
   else
   {
      if (!(lineMapData = (SECTION_DATA *) xvt_vobj_get_data (lineMapWindow)))
      {
         //xvt_dm_post_error("Invalid existing Line Map window.");
         return;
      }
      xvt_pmap_destroy ((XVT_PIXMAP) lineMapData->pixmap);
      lineMapData->pixmap = (XVT_PIXMAP) NULL;
      xvt_vobj_set_data (lineMapWindow, 0L);
   }
   createLineMapMenubar (lineMapWindow, lineMapData);
   setStratPalet (lineMapWindow);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the Line map in a PIXMAP */
   if (!(lineMapPixmap = xvt_pmap_create (lineMapWindow, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Line Map Representation.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif
      return;
   }
   xvt_dwin_clear (lineMapPixmap, COLOR_WHITE);

#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
   if (projectOptions.calculationType == REMOTE_JOB)
   {
      REMOTE_CMD_STATUS errorStatus;
      char inputFiles[100], outputFiles[100], remoteCommand[100];

      sprintf(inputFiles, "/tmp/temp_%s.his",projectOptions.accountName);
      sprintf(outputFiles,"/tmp/linemap_%s.geo",projectOptions.accountName);
      sprintf(remoteCommand, "%s %s -o %s -linemap",
                       projectOptions.noddyPath, inputFiles, outputFiles);
      report_status (inputFiles);  /* save the current history */

      if (!runRemoteCommand (inputFiles, outputFiles,
                             projectOptions.internetAddress,
                             projectOptions.accountName,
                             remoteCommand, &errorStatus))
      {
         //xvt_dm_post_error("Error, Remote Command Failed");
      }
      plotLinemapFile (lineMapPixmap, outputFiles);
      sprintf (remoteCommand, "rm -f %s",outputFiles);
      system (remoteCommand);  /* clean up the file in temp */
   }
   else
#endif   
   {              /* create the PIXMAP to describe the line map
                  ** and attach it to lineMapWindow as app_data */
      if (solidColor)
         DoLineMap(lineMapPixmap, (int) blockView->geologyCubeSize);
      else
         DoLineMap_Line (lineMapPixmap);
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif

   lineMapData->pixmap = lineMapPixmap;
   xvt_vobj_set_data (lineMapWindow, (long) lineMapData);

   if (!created)
      xvt_dwin_invalidate_rect (lineMapWindow, NULL);
   else
      xvt_vobj_set_visible (lineMapWindow, TRUE);

   bringWindowToFront(lineMapWindow);
}
/* ======================================================================
FUNCTION        invalidateCurrentLineMap
DESCRIPTION
     makes the current linemap (if any) non interactive

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
invalidateCurrentLineMap ()
#else
invalidateCurrentLineMap ()
#endif
{
   if (lineMapWindow)
      lineMapWindow = (WINDOW) NULL_WIN;
   if (sectionWindow)
      sectionWindow = (WINDOW) NULL_WIN;
}

/* ======================================================================
FUNCTION        updateSection
DESCRIPTION
     update the display of the section diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateSection (int solidColor, BLOCK_SURFACE_DATA *surface)
#else
updateSection (solidColor, surface)
int solidColor;
BLOCK_SURFACE_DATA *surface;
#endif
{
   WINDOW win;
   BOOLEAN created = FALSE;
   XVT_PIXMAP sectionPixmap;
   RCT position;
   SECTION_DATA *sectionData;
   BLOCK_SURFACE_DATA *sectionSurface;
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   char label[50];
   int mapView;


   if (surface && (fabs(surface->zStart - surface->zEnd) < TOLERANCE))
   {
      getWindowStartPosition(LINEMAP_TITLE, &position.left, &position.top, &position.right, NULL, &position.bottom, NULL);
      position.bottom += position.top;
      mapView = TRUE;
   }
   else
   {
      getWindowStartPosition(SECTION_TITLE, &position.left, &position.top, &position.right, NULL, &position.bottom, NULL);
      position.bottom += position.top;
      mapView = FALSE;
   }

   if (projectOptions.newWindowEachStage || ((!mapView) && (!sectionWindow))
                                         || ((mapView) && (!lineMapWindow)))
   {                             /* create a window if we need it */
      if (mapView)
         strcpy (label, LINEMAP_TITLE);
      else
         strcpy (label, SECTION_TITLE);
      
      if (!(win = xvt_win_create (W_DOC, &position, label,
                   MENU_BAR_2, TASK_WIN,
                   WSF_CLOSE | WSF_SIZE | WSF_INVISIBLE | WSF_ICONIZABLE,
                   EM_ALL, sectionEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create Section Window.");
         return;
      }
      
      if (mapView)
         lineMapWindow = win;
      else
         sectionWindow = win;
      created = TRUE;
   }
   else
   {
      if (mapView)
         win = lineMapWindow;
      else
         win = sectionWindow;

      if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
      {
         xvt_vobj_set_data (win, 0L);
         xvt_pmap_destroy (sectionData->pixmap);
         if (sectionData->surfaceData)
            freeSurfaceData(sectionData->surfaceData);
         xvt_mem_free ((char *) sectionData);
      }
   }

   if (!(sectionData = (SECTION_DATA *) xvt_mem_zalloc(sizeof(SECTION_DATA))))
      return;
   createLineMapMenubar (win, sectionData);
   setStratPalet (win);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
   
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
   if (projectOptions.calculationType == REMOTE_JOB)
   {
      REMOTE_CMD_STATUS errorStatus;
      char inputFiles[100], outputFiles[100], remoteCommand[100];

      sprintf(inputFiles, "/tmp/temp_%s.his",projectOptions.accountName);
      sprintf(outputFiles,"/tmp/section_%s.geo",projectOptions.accountName);
      sprintf(remoteCommand, "%s %s -o %s -section",
                       projectOptions.noddyPath, inputFiles, outputFiles);
      report_status (inputFiles);  /* save the current history */

      if (!runRemoteCommand (inputFiles, outputFiles,
                             projectOptions.internetAddress,
                             projectOptions.accountName,
                             remoteCommand, &errorStatus))
      {
         //xvt_dm_post_error("Error, Remote Command Failed");
      }
      plotSectionFile (sectionPixmap, outputFiles);
      sprintf (remoteCommand, "rm -f %s",outputFiles);
      system (remoteCommand);  /* clean up the file in temp */
   }
   else
#endif   
   {
                       /* ********** Section surface that we are about to draw */
      if (!(sectionSurface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc (sizeof(BLOCK_SURFACE_DATA))))
      {
         xvt_pmap_destroy (sectionPixmap);
         xvt_mem_free ((char *) sectionData);
         //xvt_dm_post_error("Cannot Create Section Surface. Out of Memory.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
         finishLongJob ();  /* take it down if one was never needed */
#endif
         return;
      }
         
      if (surface)  /* Used the surface passed in */
      {
         memcpy (sectionSurface, surface, sizeof(BLOCK_SURFACE_DATA));
         sectionSurface->surfaceData = (COLOR **) NULL;
         sectionSurface->next = (BLOCK_SURFACE_DATA *) NULL;
      }
      else
      {               /* Default surface is front of block */
         sectionSurface->xStart = blockView->originX;
         sectionSurface->xEnd = blockView->originX + blockView->lengthX;
         sectionSurface->yStart = blockView->originY;
         sectionSurface->yEnd = blockView->originY;
         sectionSurface->zStart = blockView->originZ - blockView->lengthZ;
         sectionSurface->zEnd = blockView->originZ;
         sectionSurface->dip = 180.0; sectionSurface->dipDirection = 90.0;
         sectionSurface->forwardCheck = FORWARD_GREATER;
         sectionSurface->depth = (short) 2;
      }
      sectionData->surfaceData = sectionSurface;
                  /* create the PIXMAP to describe the section
                  ** and attach it to sectionWindow as app_data */
      if (solidColor)
         sectionPixmap = createSolidSection(win, sectionSurface, 6);
      else
         sectionPixmap = createLineSection(win, sectionSurface,
                           (sectionSurface->xEnd - sectionSurface->xStart)
                           / (((double) xvt_rect_get_width(&position))/6.0),
                           6, NULL_WIN);
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif

   sectionData->pixmap = sectionPixmap;
   xvt_vobj_set_data (win, (long) sectionData);

             /* Generate a Size event to scale window correctly */
   xvt_vobj_get_client_rect (win, &position);
   position.bottom--;
   CORRECT_WIN_RESIZE(win, position)
   xvt_vobj_move (win, &position);

   if (!created)
      xvt_dwin_invalidate_rect (win, NULL);
   else
      xvt_vobj_set_visible (win, TRUE);

   bringWindowToFront(win);
}



/* ======================================================================
FUNCTION        updateWellLog
DESCRIPTION
     update the display of the well log diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateWellLog ()
#else
updateWellLog ()
#endif
{
   BOOLEAN created = FALSE;
   XVT_PIXMAP wellLogPixmap;
   RCT position;

   getWindowStartPosition(WELL_LOG_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);
                                                    
   if (projectOptions.newWindowEachStage || (!boreHoleWindow))
   {                             /* create a window if we need it */

      if (!(boreHoleWindow = xvt_win_create (W_DOC, &position, WELL_LOG_TITLE,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, wellLogEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create Well Log Window.");
         return;
      }
      created = TRUE;
   }
   else
   {
      xvt_pmap_destroy ((XVT_PIXMAP) xvt_vobj_get_data (boreHoleWindow));
      xvt_vobj_set_data (boreHoleWindow, 0L);
   }
   setStratPalet (boreHoleWindow);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the well log in a PIXMAP */
   if (!(wellLogPixmap = xvt_pmap_create (boreHoleWindow, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Well Log Representation.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif
      return;
   }
   xvt_dwin_clear (wellLogPixmap, COLOR_WHITE);

                  /* create the PIXMAP to describe the well log
                  ** and attach it to boreHoleWindow as app_data */
   DoWellLog (wellLogPixmap, (double **) NULL, 0);


#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif

   xvt_vobj_set_data (boreHoleWindow, (long) wellLogPixmap);

   if (!created)
      xvt_dwin_invalidate_rect (boreHoleWindow, NULL);
   else
      xvt_vobj_set_visible (boreHoleWindow, TRUE);

   bringWindowToFront(boreHoleWindow);
}


/* ======================================================================
FUNCTION        importBoreholePoints
DESCRIPTION
     read xyz points for a borehole from a file

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
importBoreholePoints (FILE_SPEC *xyzFile, int threedView)
#else
importBoreholePoints (xyzFile, threedView)
FILE_SPEC *xyzFile;
int threedView;
#endif
{
   WINDOW win;
   XYZ_IMPORT *xyzImport;
   
   if (!(xyzImport = (XYZ_IMPORT *) xvt_mem_zalloc (sizeof(XYZ_IMPORT))))
      return (FALSE);
      
   memcpy (&(xyzImport->fileSpec), xyzFile, sizeof(FILE_SPEC));
   xyzImport->type = BOREHOLE_IMPORT;
   xyzImport->data = (long) threedView;
   
   if (!(win = createCenteredWindow(XYZ_CONVERT_WINDOW, TASK_WIN,
                 EM_ALL, XYZ_CONVERT_WINDOW_eh, (long) xyzImport)))
      //xvt_dm_post_error("Can't open window");

   return (TRUE);
}

/* ======================================================================
FUNCTION        generateBoreholePoints
DESCRIPTION
     update the display of the well log diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
double **
#if XVT_CC_PROTO
generateBoreholePoints(double originX, double originY, double originZ,
                       double declination, double angleWithZ, double depth,
                       double blockSize, int *numPoints)
#else
generateBoreholePoints (originX, originY, originZ, declination,
                        angleWithZ, depth, blockSize, numPoints)
double originX, originY, originZ;
double declination, angleWithZ, depth, blockSize;
int *numPoints;
#endif
{
   double **xyzPoints = NULL;
   int point;
   double dx, dy, dz, dzs;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   dx = sin(declination*3.14159/180.0);
   dy = cos(declination*3.14159/180.0);
   dz = cos(angleWithZ*3.14159/180.0);
   dzs= sin(angleWithZ*3.14159/180.0);

   *numPoints = (int) (depth/blockSize) + 1;
   if (!(xyzPoints = (double **) create2DArray (*numPoints, 3, sizeof(double))))
      return (xyzPoints);

   for (point = 0; point < *numPoints; point++) 
   {
      xyzPoints[point][0] = originX + (blockSize*point)*dzs*dx;
      xyzPoints[point][1] = originY + (blockSize*point)*dzs*dy;
      xyzPoints[point][2] = originZ - (blockSize*point)*dz;
   }

   return (xyzPoints);
}

/* ======================================================================
FUNCTION        updateBorehole
DESCRIPTION
     update the display of the well log diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateBorehole (double **xyzPoints, int numPoints, int threedView)
#else
updateBorehole (xyzPoints, numPoints, threedView)
double **xyzPoints;
int numPoints;
int threedView;
#endif
{
   BOOLEAN created = FALSE;
   XVT_PIXMAP boreholePixmap;
   RCT position;


   if (threedView)
   {
      BLOCK_DIAGRAM_DATA *blockDiagramData;
      int *numHolePoints;
      double ***holes;
                         
      getWindowStartPosition(BLOCK_WINDOW_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);
 
      if (projectOptions.newWindowEachStage || (!boreHole3DWindow))
      {                             /* create a window if we need it */
   
         if (!(boreHole3DWindow = xvt_win_create (W_DOC, &position,
                                                   BLOCK_WINDOW_TITLE,

                      (int) NULL, TASK_WIN,
                      WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                      WSF_INVISIBLE | WSF_ICONIZABLE,

                      EM_ALL, blockDiagramEventHandler, 0L)))
         {
            //xvt_dm_post_error("Cannot Create Block Diagram Window.");
            return (FALSE);
         }
         created = TRUE;
         
         if (!(blockDiagramData = (BLOCK_DIAGRAM_DATA *) xvt_mem_zalloc(sizeof(BLOCK_DIAGRAM_DATA))))
            return (FALSE);
         if (blockDiagramData)
            initBlockDiagramData (NULL_WIN, blockDiagramData, -1.0);
            
         xvt_vobj_set_data (boreHole3DWindow, (long) blockDiagramData);
      }
      else
         blockDiagramData = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data(boreHole3DWindow);
      
               /* Increase Arrays to hold the new borehole information */
      if (!(numHolePoints = (int *) xvt_mem_zalloc ((blockDiagramData->numBoreHoles+1)*sizeof(int))))
         return (FALSE);
      if (!(holes = (double ***) xvt_mem_zalloc ((blockDiagramData->numBoreHoles+1)*sizeof(double **))))
      {
         xvt_mem_free ((char *) numPoints);
         return (FALSE);
      }
                             /* Copy the OLD borehole data into NEW arrays */
      memcpy (numHolePoints, blockDiagramData->numPointInBoreHoles, blockDiagramData->numBoreHoles*sizeof(int));
      memcpy (holes, blockDiagramData->boreHoles, blockDiagramData->numBoreHoles*sizeof(double **));
                             /* Assign the NEW bore hole data */
      numHolePoints[blockDiagramData->numBoreHoles] = numPoints;
      holes[blockDiagramData->numBoreHoles] = xyzPoints;
                             /* Free memory pointing to old hole list */
      if (blockDiagramData->numPointInBoreHoles)
         xvt_mem_free ((char *) blockDiagramData->numPointInBoreHoles);
      if (blockDiagramData->boreHoles)
         xvt_mem_free ((char *) blockDiagramData->boreHoles);
                             /* Put the new data in the blockDiagramData structure */
      blockDiagramData->numPointInBoreHoles = numHolePoints;
      blockDiagramData->boreHoles = holes;
      blockDiagramData->numBoreHoles++;
            
      updateBlockDiagram (boreHole3DWindow);

      if (created)
      {
         xvt_vobj_set_visible (boreHole3DWindow, TRUE);
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
#else
         bringWindowToFront(boreHole3DWindow);
#endif
      }

      return (TRUE);
   }
   

                         /* Not 3D Just the basic schematic view */
   getWindowStartPosition(WELL_LOG_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);

   if (projectOptions.newWindowEachStage || (!boreHoleWindow))
   {                             /* create a window if we need it */

      if (!(boreHoleWindow = xvt_win_create (W_DOC, &position, WELL_LOG_TITLE,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, wellLogEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create Well Log Window.");
         return (FALSE);
      }
      created = TRUE;
   }
   else
   {
      xvt_pmap_destroy ((XVT_PIXMAP) xvt_vobj_get_data (boreHoleWindow));
      xvt_vobj_set_data (boreHoleWindow, 0L);
   }
   setStratPalet (boreHoleWindow);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the well log in a PIXMAP */
   if (!(boreholePixmap = xvt_pmap_create (boreHoleWindow, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Well Log Representation.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif
      return (FALSE);
   }
   xvt_dwin_clear (boreholePixmap, COLOR_WHITE);

                  /* create the PIXMAP to describe the well log
                  ** and attach it to boreHoleWindow as app_data */
   DoWellLog (boreholePixmap, (double **) xyzPoints, numPoints);


#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif

   xvt_vobj_set_data (boreHoleWindow, (long) boreholePixmap);

   if (!created)
      xvt_dwin_invalidate_rect (boreHoleWindow, NULL);
   else
      xvt_vobj_set_visible (boreHoleWindow, TRUE);

   bringWindowToFront(boreHoleWindow);
   
   destroy2DArray ((char **) xyzPoints, numPoints, 3);

   return (TRUE);
}


/* ======================================================================
FUNCTION        updateBlockDiagram
DESCRIPTION
     update the display of the block diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateBlockDiagram (WINDOW blockWin)
#else
updateBlockDiagram (blockWin)
WINDOW blockWin;
#endif
{
   BOOLEAN created = FALSE;
   BLOCK_DIAGRAM_DATA *blockDiagramData = NULL;
   XVT_PIXMAP blockDiagramPixmap, oldBlockPixmap;
   RCT position;
   int dataChanged = FALSE;
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();

   if (blockWin)
      xvt_vobj_get_client_rect (blockWin, &position);
   else
   {
      getWindowStartPosition(BLOCK_WINDOW_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL);
   }

   if ((blockWin == NULL_WIN) && (projectOptions.newWindowEachStage || (!blockDiagramWindow)))
   {                             /* create a window if we need it */

      if (!(blockDiagramWindow = xvt_win_create (W_DOC, &position,
                                                BLOCK_WINDOW_TITLE,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, blockDiagramEventHandler, 0L)))
      {
         //xvt_dm_post_error("Cannot Create Block Diagram Window.");
         return;
      }
      created = TRUE;
      blockWin = blockDiagramWindow;
      if (!(blockDiagramData = (BLOCK_DIAGRAM_DATA *) xvt_mem_zalloc(sizeof(BLOCK_DIAGRAM_DATA))))
         return;
      if (blockDiagramData)
         initBlockDiagramData (NULL_WIN, blockDiagramData, -1.0);
   }
   else
   {
      if (!blockWin) /* new block diagram but in same window */
      {
         blockWin = blockDiagramWindow;
         xvt_vobj_get_client_rect (blockWin, &position);
         dataChanged = TRUE;
      }
      if (!(blockDiagramData = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data(blockWin)))
      {
         //xvt_dm_post_error ("Error, The Current block Diagram Window is not valid");
         return;
      }
                    /* Remove stored data for last block */
      if (dataChanged)
      {
         double declination, azimuth, scale;
         BLOCK_DIAGRAM_DATA *newBlockDiagramData;
                      /* preserve the way we are looking at it */
         getCameraPosition (&(blockDiagramData->threedData),
                            &declination, &azimuth, &scale);

         if (!(newBlockDiagramData = (BLOCK_DIAGRAM_DATA *) xvt_mem_zalloc(sizeof(BLOCK_DIAGRAM_DATA))))
         {
            //xvt_dm_post_error("Error, Not enough memory");
            return;
         }
         xvt_vobj_set_data (blockWin, 0L); 
         newBlockDiagramData->threedData.pixmap = blockDiagramData->threedData.pixmap;
         blockDiagramData = freeBlockDiagramData (blockDiagramData, TRUE);
         blockDiagramData = newBlockDiagramData;
         initBlockDiagramData (NULL_WIN, blockDiagramData, -1.0);
                          /* Restore way we were looking at it */
         setCameraPosition (&(blockDiagramData->threedData),
                            declination, azimuth, scale);
      }
   }
   setStratPalet (blockWin);
   

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
             /* capture the drawing of the block diagram in a PIXMAP */
   if (!(blockDiagramPixmap = xvt_pmap_create (blockWin, XVT_PIXMAP_DEFAULT,
                                  position.right, position.bottom, NULL)))
   {
      //xvt_dm_post_error("Cannot Create Block Diagram Representation.");
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
         finishLongJob ();  /* take it down if one was never needed */
#endif
      return;
   }

#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
   if (projectOptions.calculationType == REMOTE_JOB)
   {
      REMOTE_CMD_STATUS errorStatus;
      char inputFiles[100], outputFiles[100], remoteCommand[100];

      sprintf(inputFiles, "/tmp/temp_%s.his",projectOptions.accountName);
      sprintf(outputFiles,"/tmp/blockdia_%s.geo",projectOptions.accountName);
      sprintf(remoteCommand, "%s %s -o %s -geoBlock",
                       projectOptions.noddyPath, inputFiles, outputFiles);
      report_status (inputFiles);  /* save the current history */

      if (!runRemoteCommand (inputFiles, outputFiles,
                             projectOptions.internetAddress,
                             projectOptions.accountName,
                             remoteCommand, &errorStatus))
      {
         //xvt_dm_post_error("Error, Remote Command Failed");
      }
      plotBlockDiagramFile (blockDiagramPixmap, outputFiles);
      sprintf (remoteCommand, "rm -f %s",outputFiles);
      system (remoteCommand);  /* clean up the file in temp */
   }
   else
#endif   
   {              /* create the PIXMAP to describe the block diagram 
                  ** and attach it to blockDiagramWindow as app_data */
      oldBlockPixmap = blockDiagramData->threedData.pixmap;
      blockDiagramData->threedData.pixmap = blockDiagramPixmap;
      setStratPalet (blockDiagramData->threedData.pixmap);
      DoBlockDiagram (blockDiagramData, (int) blockView->geologyCubeSize);
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      finishLongJob ();  /* take it down if one was never needed */
#endif

   if (!created)  /* remove old pixmap if not aborted */
   {
      if (abortLongJob ())
      {
         xvt_pmap_destroy ((XVT_PIXMAP) blockDiagramPixmap);
         blockDiagramData->threedData.pixmap = oldBlockPixmap;
      }
      else
      {
         if (oldBlockPixmap)
            xvt_pmap_destroy (oldBlockPixmap);
      }
      xvt_vobj_set_data (blockWin, (long) blockDiagramData);
   }
   else
      xvt_vobj_set_data (blockWin, (long) blockDiagramData);

   if (!created)
      xvt_dwin_invalidate_rect (blockWin, NULL);

   xvt_vobj_set_visible (blockWin, TRUE);  /* Set visible all the time for UNIX platforms */

   bringWindowToFront(blockWin);
}

/* ======================================================================
FUNCTION        createMovie
DESCRIPTION
     create a move of the events in the geological history

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
createMovie (FILE_SPEC *filename)
#else
createMovie (filename)
FILE_SPEC *filename;
#endif
{
   WINDOW movieWindow;
   XVT_PIXMAP moviePixmap;
   BLOCK_DIAGRAM_DATA blockDiagram;
   THREED_IMAGE_DATA threedData;
   FILE_SPEC currentFile;
   RCT position; 
   PNT movieWinPos;
   char anomFilename[100];
   int totalEvents = (int) countObjects (NULL_WIN);
   int framesPerEvent = projectOptions.movieFramesPerEvent;
   int event, frame, optionsCopySize, lastFrame, sizeX, sizeY;
   short width, height;
   OBJECT *object, tempStop;
   char *optionsCopy;
   MOVIE_DATA *movieData = NULL, *prevMovieData = NULL;
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   FILE *movieFile = NULL;
	EVENT ep;

 	if (DEMO_VERSION)
	{
		//xvt_dm_post_error ("Sorry, You can not create Movies in the Demo Version");
	   return;
	}

	if (movieFile = (FILE *) fopen (filename->name, "w"))
   {
      fprintf(movieFile, "# NODDY MOVIE FILE\n");
      fprintf(movieFile, "VERSION = %.3lf\n", (double) VERSION_NUMBER);
      fprintf(movieFile, "FILE LIST:\n");
   }

                         /* images Next to be saved out */
   memcpy (&currentFile, filename, sizeof(FILE_SPEC));
#if (XVTWS == MACWS)
   addFileExtention (currentFile.name, ".PICT");
   strcpy (currentFile.type, "PICT");
#else
   addFileExtention (currentFile.name, ".bmp");
   strcpy (currentFile.type, "bmp");
#endif

   getWindowStartPosition(MOVIE_WINDOW_TITLE, &position.left, &position.top, NULL, NULL, NULL, NULL);
   movieWinPos.h = position.left;
   movieWinPos.v = position.right;

                    /* SetUp anything we need setup */
   switch (projectOptions.movieType)
   {
      case (BLOCK_DIAGRAM_MOVIE):
         getWindowStartPosition(BLOCK_WINDOW_TITLE, NULL, NULL, NULL, NULL, &width, &height);
         position.bottom = position.top + height;
         position.right = position.left + width;
         memset (&blockDiagram, 0, sizeof(BLOCK_DIAGRAM_DATA));
         break;
      case (MAP_MOVIE):
         getWindowStartPosition(LINEMAP_TITLE, NULL, NULL, NULL, NULL, &width, &height);
         position.bottom = position.top + height;
         position.right = position.left + width;
         break;
      case (LINE_MAP_MOVIE):
         getWindowStartPosition(LINEMAP_TITLE, NULL, NULL, NULL, NULL, &width, &height);
         position.bottom = position.top + height;
         position.right = position.left + width;
         break;
      case (SECTION_MOVIE):
         getWindowStartPosition(SECTION_TITLE, NULL, NULL, NULL, NULL, &width, &height);
         position.bottom = position.top + height;
         position.right = position.left + width;
         break;
      case (LINE_SECTION_MOVIE):
         getWindowStartPosition(SECTION_TITLE, NULL, NULL, NULL, NULL, &width, &height);
         position.bottom = position.top + height;
         position.right = position.left + width;
         break;
      case (WELL_LOG_MOVIE):
         getWindowStartPosition(WELL_LOG_TITLE, NULL, NULL, NULL, NULL, &width, &height);
         position.bottom = position.top + height;
         position.right = position.left + width;
         break;
      case (THREED_STRAT_MOVIE):
         getWindowStartPosition(STRATIGRAPHY_3D_TITLE, NULL, NULL, NULL, NULL, &width, &height);
         position.bottom = position.top + height;
         position.right = position.left + width;

         memset (&threedData, 0, sizeof(THREED_IMAGE_DATA));
         setCameraPosition (&threedData, threedViewOptions.declination+180,
                               threedViewOptions.azimuth,
                               threedViewOptions.scale);
         break;
      case (GRAVITY_IMAGE_MOVIE):
      case (MAGNETICS_IMAGE_MOVIE):
         position.bottom = position.top + MOVIE_WINDOW_HEIGHT;
         position.right = position.left + MOVIE_WINDOW_WIDTH;
         break;
   }
	if (position.top >= position.bottom) position.bottom = position.bottom + 100;
	if (position.left >= position.right) position.right = position.right + 100;
   
                             /* create a window if we need it */
   if (!(movieWindow = xvt_win_create (W_DOC, &position,
                                              MOVIE_WINDOW_TITLE,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, movieEventHandler, 0L)))
   {
      //xvt_dm_post_error("Cannot Create Movie Window.");
      if (movieFile)
         fclose (movieFile);
      return;
   }
   setStratPalet (movieWindow);

         /* put the position rectange back to zero for the drawing */
   if (position.top > 0)
   {
      position.bottom = position.bottom - position.top;
      position.top = 0;
   }
   if (position.left > 0)
   {
      position.right = position.right - position.left;
      position.left = 0;
   }

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   initLongJob (0, 100, "", NULL);  /* must bring up status win before lock win */
#endif
   xvt_vobj_set_visible (movieWindow, TRUE);
   bringWindowToFront(movieWindow);

   tempStop.drawEvent = FALSE;
   tempStop.selected = FALSE;
   tempStop.shape = STOP;
   for (event = 0; event < totalEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
                                  /* Take a copy of the options */
      optionsCopySize = sizeofOptions (object->shape);
      if (optionsCopySize)
      {
         if (optionsCopy = (char *) xvt_mem_zalloc (optionsCopySize))
            memcpy (optionsCopy, object->options, optionsCopySize);
      }
      else
         optionsCopy = NULL;
         
                                 /* Insert A STOP after this event */
      tempStop.next = object->next;
      object->next = &tempStop;
         
      for (frame = 0, lastFrame = FALSE;
                            (frame < framesPerEvent) && (!lastFrame); frame++)
      {
             /* capture the drawing of the movie frame in a PIXMAP */
         if (!(movieData = (MOVIE_DATA *) xvt_mem_zalloc (sizeof(MOVIE_DATA))))
         {
            //xvt_dm_post_error("Out of Memory, Cannot Create Movie.");
            finishLongJob ();  /* take it down if one was never needed */
            if (movieFile)
               fclose (movieFile);
            return;
         }
         if (!(moviePixmap = xvt_pmap_create (movieWindow,
              XVT_PIXMAP_DEFAULT, position.right, position.bottom, NULL)))
         {
            //xvt_dm_post_error("Out of Memory, Cannot Create Movie.");
            xvt_mem_free ((char *) movieData);
            finishLongJob ();  /* take it down if one was never needed */
            if (movieFile)
               fclose (movieFile);
            return;
         }

         xvt_dwin_clear (moviePixmap, COLOR_WHITE);

         movieData->prev = prevMovieData;
         movieData->next = (MOVIE_DATA *) NULL;
         movieData->pixmap = moviePixmap;

         if (prevMovieData)
            prevMovieData->next = movieData;

         prevMovieData = movieData;
         
         xvt_vobj_set_data (movieWindow, (long) movieData);
         updateMovieTitle (movieWindow);

         if (!interpolateEvent (object, frame, framesPerEvent))
            lastFrame = TRUE;

         incrementFilename (&currentFile);

#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
         if (projectOptions.calculationType == REMOTE_JOB)
         {
            REMOTE_CMD_STATUS errorStatus;
            char inputFiles[100], outputFiles[100], remoteCommand[100];

            sprintf(inputFiles, "/tmp/temp_%s.his",projectOptions.accountName);
            sprintf(outputFiles,"/tmp/blockdia_%s.geo",projectOptions.accountName);
            sprintf(remoteCommand, "%s %s -o %s -geoBlock",
                       projectOptions.noddyPath, inputFiles, outputFiles);
            report_status (inputFiles);  /* save the current history */

            if (!runRemoteCommand (inputFiles, outputFiles,
                             projectOptions.internetAddress,
                             projectOptions.accountName,
                             remoteCommand, &errorStatus))
            {
               //xvt_dm_post_error("Error, Remote Command Failed");
            }
            plotBlockDiagramFile (moviePixmap, outputFiles);
            sprintf (remoteCommand, "rm -f %s",outputFiles);
            system (remoteCommand);  /* clean up the file in temp */
         }
         else
#endif   
         {        /* Calculate Pixmap for movie */
            switch (projectOptions.movieType)
            {
               case (BLOCK_DIAGRAM_MOVIE):
			         freeBlockDiagramData (&blockDiagram, FALSE);
                  initBlockDiagramData (NULL_WIN, &blockDiagram, -1.0);
                  blockDiagram.threedData.pixmap = moviePixmap;
                  DoBlockDiagram (&blockDiagram, (int) blockView->geologyCubeSize);
                  break;
               case (MAP_MOVIE):
						{
							BLOCK_VIEW_OPTIONS *blockView = getViewOptions();
							BLOCK_SURFACE_DATA *surface;
							
							xvt_vobj_destroy(moviePixmap);

							if (surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc (sizeof(BLOCK_SURFACE_DATA)))
							{
								surface->xStart = blockView->originX;
								surface->xEnd = blockView->originX + blockView->lengthX;
								surface->yStart = blockView->originY;
								surface->yEnd = blockView->originY + blockView->lengthY;
								surface->zStart = blockView->originZ;
								surface->zEnd = blockView->originZ;
							
								moviePixmap = createSolidSection(movieWindow, surface, 6);
								xvt_mem_free ((char *) surface);
							}
						}
						break;
               case (LINE_MAP_MOVIE):
						{
							BLOCK_VIEW_OPTIONS *blockView = getViewOptions();
							BLOCK_SURFACE_DATA *surface;
							
							xvt_vobj_destroy(moviePixmap);

							if (surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc (sizeof(BLOCK_SURFACE_DATA)))
							{
								surface->xStart = blockView->originX;
								surface->xEnd = blockView->originX + blockView->lengthX;
								surface->yStart = blockView->originY;
								surface->yEnd = blockView->originY + blockView->lengthY;
								surface->zStart = blockView->originZ;
								surface->zEnd = blockView->originZ;
							
								moviePixmap = createLineSection(movieWindow, surface,
														(surface->xEnd - surface->xStart)
														/ (((double) xvt_rect_get_width(&position))/6.0),
														6, NULL_WIN);
								xvt_mem_free ((char *) surface);
							}
						}
						break;
               case (SECTION_MOVIE):
						{
							BLOCK_VIEW_OPTIONS *blockView = getViewOptions();
							BLOCK_SURFACE_DATA *surface;
							double dx, dy;
							
							xvt_vobj_destroy(moviePixmap);

							if (surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc (sizeof(BLOCK_SURFACE_DATA)))
							{
								dx = sin(geologyOptions.sectionDecl*0.01745329)
																		*geologyOptions.sectionLength;
								dy = cos(geologyOptions.sectionDecl*0.01745329)
																		*geologyOptions.sectionLength;
							
								surface->xStart = geologyOptions.sectionX;
								surface->xEnd = geologyOptions.sectionX + dx;
								surface->yStart = geologyOptions.sectionY;
								surface->yEnd = geologyOptions.sectionY + dy;
								surface->zStart = blockView->originZ - blockView->lengthZ;
								surface->zEnd = blockView->originZ;
							
								moviePixmap = createSolidSection(movieWindow, surface, 6);
								xvt_mem_free ((char *) surface);
							}
						}
						break;
               case (LINE_SECTION_MOVIE):
						{
							BLOCK_VIEW_OPTIONS *blockView = getViewOptions();
							BLOCK_SURFACE_DATA *surface;
							double dx, dy;
							
							xvt_vobj_destroy(moviePixmap);

							if (surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc (sizeof(BLOCK_SURFACE_DATA)))
							{
								dx = sin(geologyOptions.sectionDecl*0.01745329)
																		*geologyOptions.sectionLength;
								dy = cos(geologyOptions.sectionDecl*0.01745329)
																		*geologyOptions.sectionLength;
							
								surface->xStart = geologyOptions.sectionX;
								surface->xEnd = geologyOptions.sectionX + dx;
								surface->yStart = geologyOptions.sectionY;
								surface->yEnd = geologyOptions.sectionY + dy;
								surface->zStart = blockView->originZ - blockView->lengthZ;
								surface->zEnd = blockView->originZ;
							
								moviePixmap = createLineSection(movieWindow, surface,
														(surface->xEnd - surface->xStart)
														/ (((double) xvt_rect_get_width(&position))/6.0),
														6, NULL_WIN);
								xvt_mem_free ((char *) surface);
							}
						}
						break;
               case (WELL_LOG_MOVIE):
                  DoWellLog (moviePixmap, (double **) NULL, 0);
                  break;
               case (THREED_STRAT_MOVIE):
                  threedData.pixmap = moviePixmap;
                  do3dStratMap (&threedData, (char *) NULL);
                  xvt_mem_hfree ((char __huge *) threedData.surface);
                  threedData.surface = (THREED_POINT_INFO __huge *) NULL;
                  break;
               case (GRAVITY_IMAGE_MOVIE):
               case (MAGNETICS_IMAGE_MOVIE):
                  {
                     unsigned char **imageData;
                     double minValue, maxValue;
                     int fileId, csize, xmax, ymax, zmax;
                     float dat[3], posit[6], grid, airgap;
                     double interpolate = projectOptions.imageScalingFactor;
                     NODDY_COLOUR lut[260];
                     int binary;
                     FILE *fi; 
                     RCT rect;
                     char ext[5];
                     IMAGE_DISPLAY_DATA *imageDisplay;
                     

                     if (projectOptions.movieType == GRAVITY_IMAGE_MOVIE)
                     {
                        strcpy (ext, GRAVITY_FILE_EXT);
                        imageDisplay = &(projectOptions.gravityDisplay);
                     }
                     else
                     {
                        strcpy (ext, MAGNETICS_FILE_EXT);
                        imageDisplay = &(projectOptions.magneticsDisplay);
                     }
                     
                     strcpy (anomFilename, currentFile.name);
                     addFileExtention (anomFilename, ext);
                     calculateAnomalies (anomFilename, ANOM, FALSE);
      
                                        /* check to see if this is an anomily file */
                     addFileExtention (anomFilename, ext);
                     if (fi = fopen (anomFilename, "r"))
                     {
                        if ((!readGeophysHeader (fi, &fileId, &csize, &xmax, &ymax, &zmax,
                                  dat, posit, &grid, &airgap, &sizeX, &sizeY, &binary))
                             || ((fileId != 444) && (fileId != 333)))
                        {
                           fclose (fi);
                           if (movieFile)
                              fclose (movieFile);
                           return;
                        }
                        
                        if (!(imageData = (unsigned char **) create2DArray (sizeY, sizeX,
                                                                  sizeof (unsigned char))))
                        {
                           fclose (fi);
                           if (movieFile)
                              fclose (movieFile);
                           return;
                        }
                        
                        initLongJob (0, (int) (sizeY*2 + sizeY), "Loading Anomalies Image...", NULL);
                        
                        readGeophys (fi, (unsigned char **) imageData, (double **) NULL, sizeX, sizeY,
                                                &maxValue, &minValue, FALSE, binary);
                        fclose (fi);
                     }
                     xvt_pmap_destroy (moviePixmap);

                     position.bottom= (short) ((sizeY-1) * interpolate + position.top);
                     position.right = (short) ((sizeX-1) * interpolate + position.left);

                     rect.top = 0;        rect.left = 0;
                     rect.bottom = sizeY; rect.right = sizeX;
                     if (imageDisplay->display == PSEUDO_COLOR)
                        assignPseudoLut (lut, NULL, 256, 36,
                           (int) (imageDisplay->minClip/100.0*256),
                           (int) (imageDisplay->maxClip/100.0*256), 0);
                     else
                        assignGreyLut (lut, NULL, 256, 36,
                           (int) (imageDisplay->minClip/100.0*256),
                           (int) (imageDisplay->maxClip/100.0*256));
                     moviePixmap = createRasterImage (movieWindow, imageData, rect, lut, interpolate);
                     movieData->pixmap = moviePixmap;  /* as we just made a new one */

                     destroy2DArray ((char **) imageData, sizeY, sizeX);
                     finishLongJob ();
                     xvt_vobj_get_client_rect (moviePixmap, &rect);
                     xvt_rect_set_pos (&rect, movieWinPos);
                     CORRECT_WIN_RESIZE(movieWindow, rect)
                  }
                  break;
            }
         }
         
         savePixmap (moviePixmap, currentFile.name);
         if (movieFile)
            fprintf (movieFile, "%s\n", currentFile.name);

         bringWindowToFront(movieWindow);
         xvt_dwin_invalidate_rect (movieWindow, NULL);
#if (XVTWS != MTFWS)  /* causes double refresh under motif */
         xvt_dwin_update (movieWindow);
#endif
         if (abortLongJob ())
            lastFrame = TRUE;
      }
      
      if (optionsCopy)  /* restore the copy of the options */
      {
         memcpy (object->options, optionsCopy, optionsCopySize);
         xvt_mem_free (optionsCopy);
         optionsCopy = NULL;
      }
                                 /* Remove the STOP after this event */
      object->next = tempStop.next;
      
      if (abortLongJob ())
         break;
   }

                    /* Clean Up anything we setup */
   switch (projectOptions.movieType)
   {
      case (BLOCK_DIAGRAM_MOVIE):
         freeBlockDiagramData (&blockDiagram, FALSE);
         break;
      case (THREED_STRAT_MOVIE):
         xvt_mem_hfree ((char __huge *) threedData.surface);
         threedData.surface = (THREED_POINT_INFO __huge *) NULL;
         break;
      case (GRAVITY_IMAGE_MOVIE):
      case (MAGNETICS_IMAGE_MOVIE):
         {
            double interpolate = projectOptions.imageScalingFactor;

            xvt_vobj_get_client_rect (movieWindow, &position);
            position.bottom= (short) ((sizeY-1) * interpolate + position.top);
            position.right = (short) ((sizeX-1) * interpolate + position.left);
            CORRECT_WIN_RESIZE(movieWindow, position)
            xvt_vobj_move (movieWindow, &position);
         }
         break;
   }

   if (movieFile)
      fclose (movieFile);

   ep.type = E_SIZE;
   movieEventHandler (movieWindow, &ep);

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   finishLongJob ();  /* take it down if one was never needed */
#endif

}

/* ======================================================================
FUNCTION        loadMovie
DESCRIPTION
     load a move of the events in the geological history

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadMovie (FILE_SPEC *filename)
#else
loadMovie (filename)
FILE_SPEC *filename;
#endif
{
   WINDOW movieWindow;
   XVT_PIXMAP moviePixmap;
   FILE *movieFile = NULL;
   int result = TRUE;
   char line[200];
   double version;
   RCT position;
   PNT movieWinPos;
   MOVIE_DATA *movieData = NULL, *prevMovieData = NULL;
   
   getWindowStartPosition(MOVIE_WINDOW_TITLE, &position.left, &position.top, NULL, NULL, NULL, NULL);
   movieWinPos.h = position.left;
   movieWinPos.v = position.right;
   position.right = position.left + 100;
   position.bottom = position.top + 100;
                                        /* create a window if we need it */
   if (!(movieWindow = xvt_win_create (W_DOC, &position, MOVIE_WINDOW_TITLE,

                   (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE,

                   EM_ALL, movieEventHandler, 0L)))
   {
      //xvt_dm_post_error("Cannot Create Movie Window.");
      return (FALSE);
   }
   setStratPalet (movieWindow);


   xvt_fsys_set_dir(&(filename->dir));
   if (movieFile = fopen (filename->name, "r"))
   {
      loadit(movieFile,"%lf",(void *) &version);
      fgets (line, 200, movieFile);  /* FILE LIST: */
      while (fgets(line, 200, movieFile))  /* Read in Files */
      {
         if (strlen(line) > 0)
         {
            line[strlen(line)-1] = '\0';
            if (moviePixmap = loadPixmap (line))
            {
               if (movieData = (MOVIE_DATA *) xvt_mem_zalloc (sizeof(MOVIE_DATA)))
               {
                  movieData->pixmap = moviePixmap;
                  
                  if (prevMovieData)
                  {
                     prevMovieData->next = movieData;
                     movieData->prev = prevMovieData;
                  }
                  else
                     xvt_vobj_set_data (movieWindow, (long) movieData);
                  prevMovieData = movieData;
               }
               else
                  xvt_pmap_destroy (moviePixmap);
            }
         }
      }
   }
   else
      result = FALSE;
   
   if (movieFile)
      fclose (movieFile);

   xvt_vobj_set_visible (movieWindow, TRUE);

   xvt_vobj_get_client_rect (moviePixmap, &position);
   CORRECT_WIN_RESIZE(movieWindow, position)
   xvt_vobj_move (movieWindow, &position);
   xvt_dwin_invalidate_rect (movieWindow, NULL);

   bringWindowToFront(movieWindow);

   return (result);
}

/* ======================================================================
FUNCTION        winHasTaskMenu

DESCRIPTION
        return TRUE is the window has a TASK MENU attached

RETURNED
====================================================================== */
BOOLEAN
#if XVT_CC_PROTO
winHasTaskMenu (WINDOW win)
#else
winHasTaskMenu (win)
WINDOW win;
#endif
{
   char title[150];

#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   xvt_vobj_get_title (win, title, 150);
   if (     strstr (title, WELL_LOG_TITLE)
         || strstr (title, LINEMAP_TITLE)
         || strstr (title, SECTION_TITLE)
         || strstr (title, BLOCK_WINDOW_TITLE)
         || strstr (title, IMAGE_TITLE)
         || strstr (title, CONTOUR_TITLE)
         || strstr (title, PROFILE_TITLE)
         || strstr (title, TOPOGRAPHY_TITLE)
         || strstr (title, LINEMAP_PLOT_TITLE)
         || strstr (title, STRATIGRAPHY_TITLE) 
         || strstr (title, MOVIE_WINDOW_TITLE) 
         || strstr (title, STRATIGRAPHY_3D_TITLE) 
         || strstr (title, TOPOGRAPHY_3D_TITLE)) 
      return (TRUE);
   else
      return (FALSE);
#else
   return (FALSE);
#endif

}

/* ======================================================================
FUNCTION        newRockInDatabase

DESCRIPTION
        add a rock to the rock database

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
newRockInDatabase (WINDOW win)
#else
newRockInDatabase (win)
WINDOW win;
#endif
{
   ROCK_DATABASE *rockDatabase;
   LAYER_PROPERTIES *newDatabase, *newRock;
   WINDOW listWin;
   int selIndex;
   
   rockDatabase = (ROCK_DATABASE *) xvt_vobj_get_data (win);
   
   if (!(newDatabase = (LAYER_PROPERTIES *) xvt_mem_zalloc ((rockDatabase->numProps+1)
                                                            * sizeof(LAYER_PROPERTIES))))
   {
      //xvt_dm_post_error ("Error, Not Enough Memory to Add a Rock");
      return (FALSE);
   }
                    /* Copy rocks before added one */
   memcpy (newDatabase,
           rockDatabase->database,
           rockDatabase->numProps*sizeof(LAYER_PROPERTIES));
           
   newRock = &(newDatabase[rockDatabase->numProps]);
   sprintf (newRock->unitName, "Rock %d", rockDatabase->numProps+1);

   xvt_mem_free ((char *) rockDatabase->database);
   rockDatabase->database = newDatabase;
   rockDatabase->numProps++;

   updateRocksInDatabase (win);
   
               /* select and edit the one we added */
   listWin = xvt_win_get_ctl (win, ROCK_DATABASE_WIN_LBOX_2);
   selIndex = xvt_list_set_sel (listWin, xvt_list_count_all (listWin)-1, TRUE);
   editRockInDatabase (win);

   return (TRUE);
}

/* ======================================================================
FUNCTION        deleteRockInDatabase

DESCRIPTION
        delete rock from rock database

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
deleteRockInDatabase (WINDOW win)
#else
deleteRockInDatabase (win)
WINDOW win;
#endif
{
   ROCK_DATABASE *rockDatabase;
   LAYER_PROPERTIES *newDatabase;
   WINDOW listWin;
   int selIndex; 
   
   listWin = xvt_win_get_ctl (win, ROCK_DATABASE_WIN_LBOX_2);
   selIndex = xvt_list_get_sel_index (listWin);
   rockDatabase = (ROCK_DATABASE *) xvt_vobj_get_data (win);
   
   if (selIndex == -1)
   {
      //xvt_dm_post_error ("Please Select a Rock from the list first.");
      return (FALSE);
   }

   if (!(newDatabase = (LAYER_PROPERTIES *) xvt_mem_zalloc ((rockDatabase->numProps-1)
                                                            * sizeof(LAYER_PROPERTIES))))
   {
      //xvt_dm_post_error ("Error, Not Enough Memory to Delete Rock");
      return (FALSE);
   }
                    /* Copy rocks before deleted one */
   memcpy (newDatabase,
           rockDatabase->database,
           selIndex*sizeof(LAYER_PROPERTIES));
                    /* Copy rocks after deleted one */
   memcpy (&(newDatabase[selIndex]),
           &(rockDatabase->database[selIndex+1]),
           (rockDatabase->numProps-(selIndex+1))*sizeof(LAYER_PROPERTIES));
           
   xvt_mem_free ((char *) rockDatabase->database);
   rockDatabase->database = newDatabase;
   rockDatabase->numProps--;

   updateRocksInDatabase (win);

   return (TRUE);
}

/* ======================================================================
FUNCTION        editRockInDatabase

DESCRIPTION
        edit a rock in the rock database

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
editRockInDatabase (WINDOW win)
#else
editRockInDatabase (win)
WINDOW win;
#endif
{
   ROCK_DATABASE *rockDatabase;
   OBJECT *tempObject;
   WINDOW listWin;
   int selIndex;
   
   listWin = xvt_win_get_ctl (win, ROCK_DATABASE_WIN_LBOX_2);
   selIndex = xvt_list_get_sel_index (listWin);
   rockDatabase = (ROCK_DATABASE *) xvt_vobj_get_data (win);
   
   if (selIndex == -1)  /* make sure we have one to edit */
   {
      //xvt_dm_post_error ("Please Select a Rock from the list first.");
      return (FALSE);
   }

              /* used to edit a single layer */
   if (!(tempObject = (OBJECT *) xvt_mem_zalloc (sizeof(OBJECT))))
   {
      //xvt_dm_post_error ("Error, Not Enough Memory to Edit Database.");
      return (FALSE);
   }
   

   tempObject->generalData = -(selIndex+1); /* -ve is Flag for a specal layer only options */
   tempObject->options = (char *) rockDatabase;
   tempObject->shape = STOP; /* use so height is greyed out */
   tempObject->drawEvent = TRUE;
   tempObject->previewWin = win;

   if (!(createCenteredWindow(ROCK_WINDOW, TASK_WIN, EM_ALL, ROCK_WINDOW_eh, (long) tempObject)))
      //xvt_dm_post_error("Can't open window");

   return (TRUE);
}

/* ======================================================================
FUNCTION        updateRocksInDatabase

DESCRIPTION
        update the window to show all the rocks in the database

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateRocksInDatabase (WINDOW win)
#else
updateRocksInDatabase (win)
WINDOW win;
#endif
{
   WINDOW listWin;
   int prop;
   ROCK_DATABASE *rockDatabase;
   
   rockDatabase = (ROCK_DATABASE *) xvt_vobj_get_data (win);
   
   if (rockDatabase)
   {
      listWin = xvt_win_get_ctl (win, ROCK_DATABASE_WIN_LBOX_2);
      xvt_list_suspend (listWin);
      xvt_list_clear (listWin);
      for (prop = 0; prop < rockDatabase->numProps; prop++)
         xvt_list_add (listWin, -1, rockDatabase->database[prop].unitName);
      xvt_list_resume (listWin);
   }
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        initBlockDiagramData

DESCRIPTION
        initialise the block diagram data structure

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
initBlockDiagramData (WINDOW win, BLOCK_DIAGRAM_DATA *blockDiagramData,
                      double blockSizeScale)
#else
initBlockDiagramData (win, blockDiagramData, blockSizeScale)
WINDOW win;
BLOCK_DIAGRAM_DATA *blockDiagramData;
double blockSizeScale;
#endif
{
   int nx, ny, nz, i, numSurfaces;
   double minX, minY, minZ, maxX, maxY, maxZ;
   BLOCK_SURFACE_DATA *surface, *lastSurface = NULL, *nextSurface;
   double blockSize;
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   
   blockSize = blockView->geologyCubeSize;
   if (blockSizeScale > 1.0)   /* preview Scaleing factor */
      blockSize = blockSize*blockSizeScale;

   nx = (int) floor ((double) (blockView->lengthX) / blockSize + 0.5);
   ny = (int) floor ((double) (blockView->lengthY) / blockSize + 0.5);
   nz = (int) floor ((double) (blockView->lengthZ) / blockSize + 0.5);
   
   if ((nx <= 0) && (ny <= 0) && (nz <= 0))
      return (FALSE);

   blockDiagramData->blockSize = blockSize;

   blockDiagramData->blockData = (COLOR ***) NULL;
   blockDiagramData->nx = nx;
   blockDiagramData->ny = ny;
   blockDiagramData->nz = nz;
   
   blockDiagramData->minXLoc = minX = blockView->originX;
   blockDiagramData->minYLoc = minY = blockView->originY;
   blockDiagramData->minZLoc = minZ = blockView->originZ - blockView->lengthZ;
   maxX = minX + nx*blockSize;
   maxY = minY + ny*blockSize;
   maxZ = minZ + nz*blockSize;

   blockDiagramData->threedData.focus.x = minY + ny*blockSize*0.5;
   blockDiagramData->threedData.focus.y = minZ + nz*blockSize*0.5;
   blockDiagramData->threedData.focus.z = /* minX + */ nx*blockSize*0.5;

   setCameraPosition (&(blockDiagramData->threedData), threedViewOptions.declination,
                            threedViewOptions.azimuth, threedViewOptions.scale);

               /* Fill in the structues that say what type of block diagram this is */
   blockDiagramData->type = geologyOptions.blockDiagram.type;
   if (geologyOptions.blockDiagram.lut)
   {
      if (blockDiagramData->lut = (COLOR *) initBlockImageColorLut (NUM_PROPERTIES_LUT))
         blockDiagramData->lutSize = NUM_PROPERTIES_LUT;
      memcpy (blockDiagramData->lut, geologyOptions.blockDiagram.lut, sizeof(COLOR)*geologyOptions.blockDiagram.lutSize);
   }
   if (geologyOptions.blockDiagram.layersToDraw)
   {
      if (blockDiagramData->layersToDraw = (LAYER_PROPERTIES **) create1DArray(geologyOptions.blockDiagram.numLayersToDraw, sizeof(LAYER_PROPERTIES *)))
      {
         blockDiagramData->numLayersToDraw = geologyOptions.blockDiagram.numLayersToDraw;
         memcpy (blockDiagramData->layersToDraw, geologyOptions.blockDiagram.layersToDraw,
                        sizeof(LAYER_PROPERTIES *)*geologyOptions.blockDiagram.numLayersToDraw);
      }
   }
   saveBlockImageOptions (NULL_WIN, blockDiagramData);

              /* Remove any only surfaces that may be there */
   for (surface = blockDiagramData->surfaces; surface; surface = nextSurface)
   {
      nextSurface = surface->next;
      destroy2DArray ((char **) surface->surfaceData,
                      surface->dataDim1, surface->dataDim2);
      xvt_mem_free ((char *) surface);
   }
   
           /* 6 sides for a rect prism - 1 for bottom */
   numSurfaces = 5;
   if (nz <= 0) numSurfaces -= 4;  /* No Sides */
   if (ny <= 0) numSurfaces -= 4;  /* No Sides */
   if (nx <= 0) numSurfaces -= 4;  /* No Sides */
   if (numSurfaces < 1)
      return (FALSE);
   for (i = 0; i < numSurfaces; i++)
   {
      if (!(surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
      {
         for (surface = blockDiagramData->surfaces; surface; surface = nextSurface)
         {       /* Free the ones we have already allocated */
            nextSurface = surface->next;
            xvt_mem_free ((char *) surface);
         }
         blockDiagramData->surfaces = (BLOCK_SURFACE_DATA *) NULL;
         return (FALSE);
      }

      if (lastSurface)
         lastSurface->next = surface;
      else
         blockDiagramData->surfaces = surface;

      lastSurface = surface;
   }

   surface = blockDiagramData->surfaces;
               
   if ((nx > 0) && (ny > 0))              /* **** Top Surface z = maxZ **** */
   {
      surface->xStart = minX;    surface->xEnd = maxX;
      surface->yStart = minY;    surface->yEnd = maxY;
      surface->zStart = maxZ;    surface->zEnd = maxZ;
      surface->dip = 90.0; surface->dipDirection = 270.0;
      surface->forwardCheck = FORWARD_GREATER;
      surface->depth = (short) 2;
      surface = surface->next;
   }
               
   if ((ny > 0) && (nz > 0))              /* **** Left Surface x = minX **** */
   {
      surface->xStart = minX;    surface->xEnd = minX;
      surface->yStart = minY;    surface->yEnd = maxY;
      surface->zStart = minZ;    surface->zEnd = maxZ;
      surface->dip = 90.0; surface->dipDirection = 0.0;
      surface->forwardCheck = FORWARD_GREATER;
      surface->depth = (short) 2;
      surface = surface->next;
   }

                                          /* **** Right Surface x = maxX **** */                      
   if ((ny > 0) && (nz > 0) && (numSurfaces > 1))
   {
      surface->xStart = maxX;    surface->xEnd = maxX;
      surface->yStart = minY;    surface->yEnd = maxY;
      surface->zStart = minZ;    surface->zEnd = maxZ;
      surface->dip = 90.0; surface->dipDirection = 180.0;
      surface->forwardCheck = FORWARD_GREATER;
      surface->depth = (short) 2;
      surface = surface->next;
   }
                               
   if ((nx > 0) && (nz > 0))              /* **** Front Surface y = minY **** */
   {
      surface->xStart = minX;    surface->xEnd = maxX;
      surface->yStart = minY;    surface->yEnd = minY;
      surface->zStart = minZ;    surface->zEnd = maxZ;
      surface->dip = 180.0; surface->dipDirection = 90.0;
      surface->forwardCheck = FORWARD_GREATER;
      surface->depth = (short) 2;
      surface = surface->next;
   }
                               
                                          /* **** Back Surface y = maxY **** */
   if ((nx > 0) && (nz > 0) && (numSurfaces > 1))
   {
      surface->xStart = minX;    surface->xEnd = maxX;
      surface->yStart = maxY;    surface->yEnd = maxY;
      surface->zStart = minZ;    surface->zEnd = maxZ;
      surface->dip = 0.0; surface->dipDirection = 90.0;
      surface->forwardCheck = FORWARD_GREATER;
      surface->depth = (short) 2;
      surface = surface->next;
   }                            
/*
   if ((nx > 0) && (ny > 0))              .* **** Bottom Surface z = minZ **** *.
   {
      surface->xStart = minX;    surface->xEnd = maxX;
      surface->yStart = minY;    surface->yEnd = maxY;
      surface->zStart = minZ;    surface->zEnd = minZ;
      surface->dip = 0.0;
   }
*/

           /* Add surfaces for Chair Diagram if we need them */
   if (geologyOptions.calculateChairDiagram)
      addChairDiagramData (blockDiagramData, geologyOptions.chair_X,
                                             geologyOptions.chair_Y,
                                             geologyOptions.chair_Z);

   return (TRUE);
}

/* ======================================================================
FUNCTION        freeSurfaceData

DESCRIPTION
        destroy the block diagram data structure

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
freeSurfaceData (BLOCK_SURFACE_DATA *surfaceData)
#else
freeSurfaceData (surfaceData)
BLOCK_SURFACE_DATA *surfaceData;
#endif
{
   BLOCK_SURFACE_DATA *surface, *nextSurface;

   for (surface = surfaceData; surface; surface = nextSurface)
   {
      nextSurface = surface->next;
      destroy2DArray ((char **) surface->surfaceData,
                      surface->dataDim1, surface->dataDim2);
      xvt_mem_free ((char *) surface);
   }
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        destroyBlockDiagramData

DESCRIPTION
        destroy the block diagram data structure

RETURNED
====================================================================== */
BLOCK_DIAGRAM_DATA *
#if XVT_CC_PROTO
freeBlockDiagramData (BLOCK_DIAGRAM_DATA *blockDiagramData, int andPtr)
#else
freeBlockDiagramData (blockDiagramData, andPtr)
BLOCK_DIAGRAM_DATA *blockDiagramData;
int andPtr;
#endif
{
   if (blockDiagramData->blockData)
   {
      destroy3DArray ((char ***) blockDiagramData->blockData, blockDiagramData->nz,
                                        blockDiagramData->nx, blockDiagramData->ny);
      blockDiagramData->blockData = (COLOR ***) NULL;
   }

   if (blockDiagramData->valueData)
   {
      destroy3DArray ((char ***) blockDiagramData->valueData, blockDiagramData->nz,
                                        blockDiagramData->nx, blockDiagramData->ny);
      blockDiagramData->valueData = (float ***) NULL;
   }

   if (blockDiagramData->lut)
   {
      destroy1DArray ((char *) blockDiagramData->lut);
      blockDiagramData->lut = (COLOR *) NULL;
      blockDiagramData->lutSize = 0;
   }

   if (blockDiagramData->layersToDraw)
   {
      destroy1DArray ((char *) blockDiagramData->layersToDraw);
      blockDiagramData->layersToDraw = (LAYER_PROPERTIES **) NULL;
      blockDiagramData->numLayersToDraw = 0;
   }

   if (blockDiagramData->threedData.surface)
      xvt_mem_hfree ((char __huge *) blockDiagramData->threedData.surface);

   if (freeSurfaceData (blockDiagramData->surfaces))
      blockDiagramData->surfaces = NULL;
   
   if (blockDiagramData->boreHoles)
   {
      int boreHole;
             
      for (boreHole = 0; boreHole < blockDiagramData->numBoreHoles; boreHole++)
      {
         destroy2DArray ((char **) blockDiagramData->boreHoles[boreHole],
                         blockDiagramData->numPointInBoreHoles[boreHole], 3);
      }
      destroy1DArray ((char *) blockDiagramData->numPointInBoreHoles);
      
      blockDiagramData->boreHoles = (double ***) NULL;
      blockDiagramData->numPointInBoreHoles = (int *) NULL;
      blockDiagramData->numBoreHoles = 0;
   }
   
   if (andPtr)
   {
      xvt_mem_free ((char *) blockDiagramData);
      blockDiagramData = NULL;
   }
      
   return (blockDiagramData);
}

/* ======================================================================
FUNCTION        addChairDiagramData

DESCRIPTION
        Add chair diagram surfaces to the block diagram 

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
addChairDiagramData (BLOCK_DIAGRAM_DATA *blockDiagramData,
                     double xLoc, double yLoc, double zLoc)
#else
addChairDiagramData (blockDiagramData, xLoc, yLoc, zLoc)
BLOCK_DIAGRAM_DATA *blockDiagramData;
double xLoc, yLoc, zLoc;
#endif
{
   double minX, minY, minZ, maxX, maxY, maxZ, blockSize;
   BLOCK_SURFACE_DATA *firstSurface= NULL, *surface,
                      *lastSurface = NULL, *nextSurface;
   int i;

   blockSize = blockDiagramData->blockSize;

   minX = blockDiagramData->minXLoc;
   minY = blockDiagramData->minYLoc;
   minZ = blockDiagramData->minZLoc;
   maxX = minX + blockDiagramData->nx*blockSize;
   maxY = minY + blockDiagramData->ny*blockSize;
   maxZ = minZ + blockDiagramData->nz*blockSize;

              /* Make sure we have a chair to actually draw */
   if (((xLoc - minX) < blockSize) || ((yLoc - minY) < blockSize)
                                   || ((maxZ - zLoc) < blockSize))
      return (FALSE);

	if (xLoc > maxX) xLoc = maxX;
	if (yLoc > maxY) yLoc = maxY;
	if (zLoc < minZ) zLoc = minZ;

           /* 3 extra surfaces for a chair diagram */
   for (i = 0; i < 3; i++)
   {
      if (!(surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
      {
         for (surface = firstSurface; surface; surface = nextSurface)
         {       /* Free the ones we have already allocated */
            nextSurface = surface->next;
            xvt_mem_free ((char *) surface);
         }
         return (FALSE);
      }

      if (lastSurface)
         lastSurface->next = surface;
      else
         firstSurface = surface;

      lastSurface = surface;
   }

                            /* **** Bottom Surface z = zLoc **** */
   surface = firstSurface;
   surface->xStart = minX;    surface->xEnd = xLoc;
   surface->yStart = minY;    surface->yEnd = yLoc;
   surface->zStart = zLoc;    surface->zEnd = zLoc;
   surface->dip = 90.0; surface->dipDirection = 270.0;
   surface->forwardCheck = FORWARD_GREATER;
   surface->depth = (short) 1;
                            /* **** Back Surface y = yLoc **** */
   surface = surface->next;
   surface->xStart = minX;    surface->xEnd = xLoc;
   surface->yStart = yLoc;    surface->yEnd = yLoc;
   surface->zStart = zLoc;    surface->zEnd = maxZ;
   surface->dip = 180.0; surface->dipDirection = 90.0;
   surface->forwardCheck = FORWARD_GREATER;
   surface->depth = (short) 1;
                            /* **** Right Surface x = xLoc **** */
   surface = surface->next;
   surface->xStart = xLoc;    surface->xEnd = xLoc;
   surface->yStart = minY;    surface->yEnd = yLoc;
   surface->zStart = zLoc;    surface->zEnd = maxZ;
   surface->dip = 90.0; surface->dipDirection = 0.0;
   surface->forwardCheck = FORWARD_GREATER;
   surface->depth = (short) 1;

                            /* **** In front face y = minY **** */
   addEmptyRectData (blockDiagramData, minX, minY, zLoc, xLoc, minY, maxZ);
                            /* **** In side  face x = minX **** */
   addEmptyRectData (blockDiagramData, minX, minY, zLoc, minX, yLoc, maxZ);
                            /* **** In side  face z = maxZ **** */
   addEmptyRectData (blockDiagramData, minX, minY, maxZ, xLoc, yLoc, maxZ);

                     /* get last surface in block so far */
   for (surface = blockDiagramData->surfaces; surface && surface->next; surface = surface->next)
      ; /* do nothing except find last surface */
   if (surface)   /* Add after all the current surfaces */
      surface->next = firstSurface;
   else
      blockDiagramData->surfaces = firstSurface;
      
   return (TRUE);
}

/* ======================================================================
FUNCTION        addIregSufaceData

DESCRIPTION
        Add chair diagram surfaces to the block diagram 

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
addIregSurfaceData (BLOCK_DIAGRAM_DATA *blockDiagramData,
                    short ***iregData, int nxIreg, int nyIreg, int nzIreg,
                    double minXIreg, double minYIreg, double minZIreg)
#else
addIregSurfaceData (blockDiagramData, iregData, nxIreg, nyIreg, nzIreg,
                                      minXIreg, minYIreg, minZIreg)
BLOCK_DIAGRAM_DATA *blockDiagramData;
short ***iregData;
int nxIreg, nyIreg, nzIreg;
double minXIreg, minYIreg, minZIreg;
#endif
{
   double minX, minY, minZ, maxX, maxY, maxZ, blockSize;
   double maxXIreg, maxYIreg, maxZIreg;
   double posXIreg, posYIreg, posZIreg;
   int nx, ny, nz, x, y, z;
   BLOCK_SURFACE_DATA *firstSurface= NULL, *surface,
                      *lastSurface = NULL, *nextSurface;
                
   blockSize = blockDiagramData->blockSize;
   nx = blockDiagramData->nx;
   ny = blockDiagramData->ny;
   nz = blockDiagramData->nz;

   minX = blockDiagramData->minXLoc;
   minY = blockDiagramData->minYLoc;
   minZ = blockDiagramData->minZLoc;
   maxX = minX + nx*blockSize;
   maxY = minY + ny*blockSize;
   maxZ = minZ + nz*blockSize;

   maxXIreg = minXIreg + nxIreg*blockSize;
   maxYIreg = minYIreg + nyIreg*blockSize;
   maxZIreg = minZIreg + nzIreg*blockSize;

              /* Make sure we have a surface to actually draw */
   if ((maxXIreg < minX) || (maxYIreg < minY) || (maxZIreg < minZ)
    || (minXIreg > maxX) || (minYIreg > maxY) || (minZIreg > maxY))
      return (FALSE);


                  /* Create Block for every surface element that can be visible */
   for (x = 0, posXIreg = minXIreg; x < nxIreg; x++, posXIreg+=blockSize)
   {
      for (y = 0, posYIreg = minYIreg; y < nyIreg; y++, posYIreg+=blockSize)
      {
         for (z = 0, posZIreg = minZIreg; z < nzIreg; z++, posZIreg+=blockSize)
         {
            if (iregData[z][y][x] > 0)  /* Block on */
            {
               
               if ((z == 0) || (!iregData[z-1][y][x]))   /* Top or nothing above */
               {
                  if (!(surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
                  {
                     for (surface = firstSurface; surface; surface = nextSurface)
                     {       /* Free the ones we have already allocated */
                        nextSurface = surface->next;
                        xvt_mem_free ((char *) surface);
                     }
                     return (FALSE);
                  }
                  if (lastSurface)
                     lastSurface->next = surface;
                  else
                     firstSurface = surface;
               
                  lastSurface = surface;

                            /* **** Top Surface z = maxZ **** */
                  surface = blockDiagramData->surfaces;
                  surface->xStart = posXIreg;    surface->xEnd = posXIreg+blockSize;
                  surface->yStart = posYIreg;    surface->yEnd = posYIreg+blockSize;
                  surface->zStart = posZIreg;    surface->zEnd = posZIreg;
                  surface->dip = 90.0; surface->dipDirection = 270.0;
                  surface->forwardCheck = FORWARD_GREATER;
                  surface->depth = (short) 2;
               }

               if ((x == 0) || (!iregData[z][y][x-1]))   /* Left or nothing beside */
               {
                  if (!(surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
                  {
                     for (surface = firstSurface; surface; surface = nextSurface)
                     {       /* Free the ones we have already allocated */
                        nextSurface = surface->next;
                        xvt_mem_free ((char *) surface);
                     }
                     return (FALSE);
                  }
                  if (lastSurface)
                     lastSurface->next = surface;
                  else
                     firstSurface = surface;
               
                  lastSurface = surface;

                            /* **** Left Surface x = minX **** */
                  surface = surface->next;
                  surface->xStart = posXIreg;    surface->xEnd = posXIreg;
                  surface->yStart = posYIreg;    surface->yEnd = posYIreg+blockSize;
                  surface->zStart = posZIreg;    surface->zEnd = posZIreg+blockSize;
                  surface->dip = 90.0; surface->dipDirection = 0.0;
                  surface->forwardCheck = FORWARD_GREATER;
                  surface->depth = (short) 2;
               }

               if ((x == nx-1) || (!iregData[z][y][x+1]))   /* Right or nothing beside */
               {
                  if (!(surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
                  {
                     for (surface = firstSurface; surface; surface = nextSurface)
                     {       /* Free the ones we have already allocated */
                        nextSurface = surface->next;
                        xvt_mem_free ((char *) surface);
                     }
                     return (FALSE);
                  }
                  if (lastSurface)
                     lastSurface->next = surface;
                  else
                     firstSurface = surface;
               
                  lastSurface = surface;

                            /* **** Right Surface x = maxX **** */
                  surface = surface->next;
                  surface->xStart = posXIreg;    surface->xEnd = posXIreg;
                  surface->yStart = posYIreg;    surface->yEnd = posYIreg+blockSize;
                  surface->zStart = posZIreg;    surface->zEnd = posZIreg+blockSize;
                  surface->dip = 90.0; surface->dipDirection = 180.0;
                  surface->forwardCheck = FORWARD_GREATER;
                  surface->depth = (short) 2;
               }

               if ((y == 0) || (!iregData[z][y-1][x]))   /* Front or nothing infront */
               {
                  if (!(surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
                  {
                     for (surface = firstSurface; surface; surface = nextSurface)
                     {       /* Free the ones we have already allocated */
                        nextSurface = surface->next;
                        xvt_mem_free ((char *) surface);
                     }
                     return (FALSE);
                  }
                  if (lastSurface)
                     lastSurface->next = surface;
                  else
                     firstSurface = surface;
               
                  lastSurface = surface;

                            /* **** Front Surface y = minY **** */
                  surface = surface->next;
                  surface->xStart = posXIreg;    surface->xEnd = posXIreg+blockSize;
                  surface->yStart = posYIreg;    surface->yEnd = posYIreg;
                  surface->zStart = posZIreg;    surface->zEnd = posZIreg+blockSize;
                  surface->dip = 180.0; surface->dipDirection = 90.0;
                  surface->forwardCheck = FORWARD_GREATER;
                  surface->depth = (short) 2;
               }
               
               if ((y == ny-1) || (!iregData[z][y+1][x]))   /* Back or nothing behind */
               {
                  if (!(surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
                  {
                     for (surface = firstSurface; surface; surface = nextSurface)
                     {       /* Free the ones we have already allocated */
                        nextSurface = surface->next;
                        xvt_mem_free ((char *) surface);
                     }
                     return (FALSE);
                  }
                  if (lastSurface)
                     lastSurface->next = surface;
                  else
                     firstSurface = surface;
               
                  lastSurface = surface;

                            /* **** Back Surface y = maxY **** */
                  surface = surface->next;
                  surface->xStart = posXIreg;    surface->xEnd = posXIreg+blockSize;
                  surface->yStart = posYIreg;    surface->yEnd = posYIreg;
                  surface->zStart = posZIreg;    surface->zEnd = posZIreg+blockSize;
                  surface->dip = 0.0; surface->dipDirection = 90.0;
                  surface->forwardCheck = FORWARD_GREATER;
                  surface->depth = (short) 2;
               }

               if ((z == nz-1) || (!iregData[z+1][y][x]))   /* Bottom or nothing below */
               {
                  if (!(surface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
                  {
                     for (surface = firstSurface; surface; surface = nextSurface)
                     {       /* Free the ones we have already allocated */
                        nextSurface = surface->next;
                        xvt_mem_free ((char *) surface);
                     }
                     return (FALSE);
                  }
                  if (lastSurface)
                     lastSurface->next = surface;
                  else
                     firstSurface = surface;
               
                  lastSurface = surface;

                            /* **** Bottom Surface z = minZ **** */
                  surface = surface->next;
                  surface->xStart = posXIreg;    surface->xEnd = posXIreg+blockSize;
                  surface->yStart = posYIreg;    surface->yEnd = posYIreg+blockSize;
                  surface->zStart = posZIreg;    surface->zEnd = posZIreg;
                  surface->dip = 90.0; surface->dipDirection = 270.0;
                  surface->forwardCheck = FORWARD_LESS;
                  surface->depth = (short) 2;
               }
            }
         }
      }
   }
   
                     /* get last surface in block so far */
   for (surface = blockDiagramData->surfaces; surface && surface->next; surface = surface->next)
      ; /* do nothing except find last surface */
   if (surface)   /* Add after all the current surfaces */
      surface->next = firstSurface;
   else
      blockDiagramData->surfaces = firstSurface;
      
   return (TRUE);
}

/* ======================================================================
FUNCTION        addEmptyRectData

DESCRIPTION
        Add a blank area in an already constructed plane.
        This will involve breaking the constructed plane into
        at least 2 smaller planes.

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
addEmptyRectData (BLOCK_DIAGRAM_DATA *blockDiagramData,
                  double xLocStart, double yLocStart, double zLocStart,
                  double xLocEnd,   double yLocEnd,   double zLocEnd)
#else
addEmptyRectData (blockDiagramData, xLocStart, yLocStart, zLocStart,
                                    xLocEnd,   yLocEnd,   zLocEnd)
BLOCK_DIAGRAM_DATA *blockDiagramData;
double xLocStart, yLocStart, zLocStart;
double xLocEnd, yLocEnd, zLocEnd;
#endif
{
   double blockSize;
   BLOCK_SURFACE_DATA *firstSurface= NULL, *surface,
                      *lastSurface = NULL, *nextSurface;

   blockSize = blockDiagramData->blockSize;         

                     /* Find the surface the whole in in */
   for (surface = blockDiagramData->surfaces; surface; surface = surface->next)
   {
      if ((surface->xStart-TOLERANCE < xLocStart) &&
          (surface->xEnd  +TOLERANCE > xLocEnd)   &&
          (surface->yStart-TOLERANCE < yLocStart) &&
          (surface->yEnd  +TOLERANCE > yLocEnd)   &&
          (surface->zStart-TOLERANCE < zLocStart) &&
          (surface->zEnd  +TOLERANCE > zLocEnd))
      {        /* split this surface around the whole */
         double startX, startY, startZ, endX, endY, endZ;

                         /* Store the extent of this plane */
         startX = surface->xStart; endX = surface->xEnd;
         startY = surface->yStart; endY = surface->yEnd;
         startZ = surface->zStart; endZ = surface->zEnd;
         
           /* extra surface for a chair diagram */
         if (!(nextSurface = (BLOCK_SURFACE_DATA *) xvt_mem_zalloc(sizeof(BLOCK_SURFACE_DATA))))
            return (FALSE);
         memcpy (nextSurface, surface, sizeof(BLOCK_SURFACE_DATA));

            /* locate the position of the hole in y = 0 plane */
			if ((fabs(xLocEnd-xLocStart) > TOLERANCE) && (endX < xLocEnd+TOLERANCE) && (startX > xLocStart-TOLERANCE)) /* NEW */
			{
            surface->xStart = startX;        /* beside hole */
            surface->xEnd = endX;            /* beside hole */
            surface->yStart = yLocEnd;       /* beside hole */
            surface->yEnd = endY;            /* beside hole */
            surface->zStart = startZ;        /* beside hole */
            surface->zEnd = zLocStart;       /* beside hole */

				xvt_mem_free((char *) nextSurface);
				nextSurface = NULL;
			}
         else if (startX < xLocStart-TOLERANCE)  /* endX end */
         {
            surface->xStart = startX;        /* beside hole */
            surface->xEnd = xLocStart;       /* beside hole */
            surface->yStart = startY;        /* beside hole */
            surface->yEnd = endY;            /* beside hole */
            surface->zStart = startZ;        /* beside hole */
            surface->zEnd = endZ;            /* beside hole */
            
            nextSurface->xStart = xLocStart; /* below hole */
            nextSurface->xEnd = endX;        /* below hole */
            nextSurface->yStart = startY;    /* below hole */
            nextSurface->yEnd = yLocEnd;     /* below hole */
            nextSurface->zStart = startZ;    /* below hole */
            nextSurface->zEnd = zLocStart;   /* below hole */
         }
         else if (endX > xLocEnd+TOLERANCE) /* startX end */
         {
            surface->xStart = xLocEnd;       /* beside hole */
            surface->xEnd = endX;            /* beside hole */
            surface->yStart = startY;        /* beside hole */
            surface->yEnd = endY;            /* beside hole */
            surface->zStart = startZ;        /* beside hole */
            surface->zEnd = endZ;            /* beside hole */

            nextSurface->xStart = startX;    /* below hole */
            nextSurface->xEnd = xLocEnd;     /* below hole */
            nextSurface->yStart = yLocEnd;   /* below hole */
            nextSurface->yEnd = endY;        /* below hole */
            nextSurface->zStart = startZ;    /* below hole */
            nextSurface->zEnd = zLocStart;   /* below hole */
         }
                  
                       
			else if ((fabs(yLocEnd-yLocStart) > TOLERANCE) && (endY < yLocEnd+TOLERANCE) && (startY > yLocStart-TOLERANCE)) /* NEW */
			{
            surface->xStart = startX;        /* beside hole */
            surface->xEnd = xLocStart;            /* beside hole */
            surface->yStart = yLocStart;       /* beside hole */
            surface->yEnd = endY;            /* beside hole */
            surface->zStart = startZ;        /* beside hole */
            surface->zEnd = zLocStart;       /* beside hole */

				xvt_mem_free((char *) nextSurface);
				nextSurface = NULL;
			}
         else if (startY < yLocStart-TOLERANCE)  /* endY end */
         {
            surface->xStart = startX;        /* below hole */
            surface->xEnd = xLocStart;       /* below hole */
            surface->yStart = yLocStart;     /* below hole */
            surface->yEnd = endY;            /* below hole */
            surface->zStart = startZ;        /* below hole */
            surface->zEnd = zLocStart;       /* below hole */

            nextSurface->xStart = startX;    /* beside hole */
            nextSurface->xEnd = endX;        /* beside hole */
            nextSurface->yStart = startY;    /* beside hole */
            nextSurface->yEnd = yLocStart;   /* beside hole */
            nextSurface->zStart = startZ;    /* beside hole */
            nextSurface->zEnd = endZ;        /* beside hole */
         }
         else if (endY > yLocEnd+TOLERANCE) /* startY end */
         {
            surface->xStart = startX;        /* below hole */
            surface->xEnd = xLocStart;       /* below hole */
            surface->yStart = startY;        /* below hole */
            surface->yEnd = yLocEnd;         /* below hole */
            surface->zStart = startZ;        /* below hole */
            surface->zEnd = zLocStart;       /* below hole */

            nextSurface->xStart = startX;    /* beside hole */
            nextSurface->xEnd = endX;        /* beside hole */
            nextSurface->yStart = yLocEnd;   /* beside hole */
            nextSurface->yEnd = endY;        /* beside hole */
            nextSurface->zStart = startZ;    /* beside hole */
            nextSurface->zEnd = endZ;        /* beside hole */
         }

#ifdef PAUL
         if (startZ < zLocStart) /* endZ end */
         {
            surface->yEnd = yLocStart;     /* beside hole */
            nextSurface->yStart = yLocStart; /* below hole */
            nextSurface->xEnd = xLocStart; /* below hole */
         }
         else if (endZ > zLocEnd+TOLERANCE) /* startZ end */
         {
            surface->yStart = yLocEnd;     /* beside hole */
            nextSurface->yEnd = yLocEnd; /* below hole */
            nextSurface->xEnd = xLocStart; /* below hole */
         }
#endif
            
                     /* insert in the list of surfaces */
			if (nextSurface)
			{
				nextSurface->next = surface->next;
				surface->next = nextSurface;
			}
         break;
      }
   }
   
   return (TRUE);
}

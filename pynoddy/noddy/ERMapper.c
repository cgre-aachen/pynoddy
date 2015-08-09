#include "xvt.h"
#include "noddy.h"
#include <time.h>
#include <math.h>
#include "titles.h"

#define DEBUG(X)

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;

#define ERM_FORMAT_LENGTH 30
                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
int writeERMAnomImage (char *, double **, int, int, int, int, double, double, double, GEOPHYSICS_OPTIONS *, int, int);
int ERMapperHeader(char *, int, double, double, int, int, int, int);
int ERMapperVecHeader(char *);
int DoERMapperLineMap(int, int, char *);
int ERMapper_mcontr(double **, int, int, double [20], int, double, double, double, double, FILE *, int);
WINDOW displayERMImage(char *);
int readERMHeader(char *, int *, int *, int *, char *, int *, double *, double *, double *);
int readERMGeophys(char *, double **, int, int, int, char *, int, double *, double *);
#else
int writeERMAnomImage ();
int ERMapperHeader();
int ERMapperVecHeader();
int DoERMapperLineMap();
int ERMapper_mcontr();
WINDOW displayERMImage();
int readERMHeader();
int readERMGeophys();
#endif



/* *****************************************************************
FUNCTION writeERMAnomFile

INPUT

OUTPUT

DESCRIPTION 
   write out both gravity and magnetics to an ERMapper format
   output file

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
int
#if XVT_CC_PROTO
writeERMAnomImage (char *filename, double **imageData,
      int nx, int ny, int nz, int slaveSize, double cubeSize,
      double easting, double northing,
      GEOPHYSICS_OPTIONS *options, int magImage, int grvImage)
#else
writeERMAnomImage (filename, imageData, nx, ny, nz, slaveSize,
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
   float magBuf = (float) 0.0, buffer2[1];
   char *fbuffer, *pointPtr;
   char dataFilename[100];
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   
   if (!filename)
      return (FALSE);

   strcpy (dataFilename, filename);
   
   addFileExtention (filename, ".ers");
   if (!(ERMapperHeader(filename, (int) floor(cubeSize), easting, northing, nx-1, ny, magImage, grvImage)))
      return (FALSE);
      
   
   if (pointPtr = (char *) strchr (dataFilename, '.'))
      *pointPtr = '\0';  /* remove any extention we have */
   if (!(fo = (FILE *) fopen (dataFilename, "wb")))
      return (FALSE);

   fbuffer = (char *) buffer2;
        
   for (y = ny-1; y >= 0; y--)
   {
      for (x = 1; x < nx; x++)
      {
         if (imageData)
            magBuf = (float) imageData[x][y];
            
         buffer2[0] = magBuf;
         fwrite (fbuffer, 4, 1, fo);
      }
   }

   fclose (fo);

   return (TRUE);
}


int
#if XVT_CC_PROTO
ERMapperHeader(char *filename, int cellSize, double easting, double northing, 
               int cols, int rows, int magBand, int grvBand)
#else
ERMapperHeader(filename, cellSize, easting, northing, cols, rows, magBand, grvBand)
char *filename;
int cellSize;
double easting, northing;
int cols, rows, magBand, grvBand;
#endif
{
   FILE *fo;
   time_t longTime;
   struct tm *today;
   char timeString[50];
   int length;

   time(&longTime); /* current Time */
   today = gmtime(&longTime);

   addFileExtention (filename, ".ers");
   
   if (!(fo = (FILE *) fopen (filename, "w")))
      return (FALSE);

   fprintf(fo,"DatasetHeader Begin\n");
   fprintf(fo,"\tVersion \t= \"3.0\"\n");
   sprintf(timeString,"%s",asctime(today));
   if (!strstr(timeString,"GMT"))
   {
      length = strlen(timeString);
      strcpy (&(timeString[40]), &(timeString[length-6])); /* preserve " 1996\n" */
      strcpy (&(timeString[length-5]), "GMT");             /* add "GMT" */
      strcpy (&(timeString[length-2]), &(timeString[40])); /* append " 1996\n" */
   }
   fprintf(fo,"\tLastUpdated\t= %s", timeString);
   fprintf(fo,"\tDataSetType\t= ERStorage\n");
   fprintf(fo,"\tDataType\t= Raster\n");
#if XVTWS == WIN32WS	
   fprintf(fo,"\tByteOrder\t= LSBFirst\n");
#else
   fprintf(fo,"\tByteOrder\t= MSBFirst\n");
#endif
/*
   fprintf(fo,"\tCoordinateSpace Begin\n");
   fprintf(fo,"\t\tDatum\t\t= \"RAW\"\n");
   fprintf(fo,"\t\tProjection\t= \"RAW\"\n");
   fprintf(fo,"\t\tCoordinateType\t= RAW\n");
   fprintf(fo,"\t\tUnits\t\t= \"METERS\"\n");
   fprintf(fo,"\t\tRotation\t= 0:0:0.0\n");
   fprintf(fo,"\tCoordinateSpace End\n");
*/
   fprintf(fo,"\tCoordinateSpace Begin\n");
   fprintf(fo,"\t\tDatum\t\t= \"AGD66\"\n");
   fprintf(fo,"\t\tProjection\t= \"TMAMG55\"\n");
   fprintf(fo,"\t\tCoordinateType\t= EN\n");
   fprintf(fo,"\t\tUnits\t\t= \"METERS\"\n");
   fprintf(fo,"\t\tRotation\t= 0:0:0.0\n");
   fprintf(fo,"\tCoordinateSpace End\n");

   fprintf(fo,"\tRasterInfo Begin\n");
   fprintf(fo,"\t\tCellType\t= IEEE4ByteReal\n");
   fprintf(fo,"\t\tNullCellValue\t= -9999\n");

   fprintf(fo,"\t\tCellInfo Begin\n");
   fprintf(fo,"\t\t\tXdimension\t=%d\n",cellSize);
   fprintf(fo,"\t\t\tYdimension\t=%d\n",cellSize);
   fprintf(fo,"\t\tCellInfo End\n");

   fprintf(fo,"\t\tNrOfLines\t= %d\n",rows);
   fprintf(fo,"\t\tNrOfCellsPerLine\t= %d\n",cols);
/*
   fprintf(fo,"\t\tRegistrationCoord Begin\n");
   fprintf(fo,"\t\t\tMetersX\t\t= 0\n");
   fprintf(fo,"\t\t\tMetersY\t\t= 0\n");
   fprintf(fo,"\t\tRegistrationCoord End\n");
*/
   fprintf(fo,"\t\tRegistrationCellX\t= 0\n");
   fprintf(fo,"\t\tRegistrationCellY\t= %d\n",rows);
   fprintf(fo,"\t\tRegistrationCoord Begin\n");
   fprintf(fo,"\t\t\tEastings\t\t= %lf\n", easting);
   fprintf(fo,"\t\t\tNorthings\t\t= %lf\n", northing);
   fprintf(fo,"\t\tRegistrationCoord End\n");

   if (magBand && grvBand)
      fprintf(fo,"\t\tNrOfBands\t= 2\n");
   else
      fprintf(fo,"\t\tNrOfBands\t= 1\n");
   if (magBand)
   {
      fprintf(fo,"\t\tBandId Begin\n");
      fprintf(fo,"\t\t\tValue\t\t= \"magnetics\"\n");
      fprintf(fo,"\t\tBandId End\n");
   }
   if (grvBand)
   {
      fprintf(fo,"\t\tBandId Begin\n");
      fprintf(fo,"\t\t\tValue\t\t= \"gravity\"\n");
      fprintf(fo,"\t\tBandId End\n");
   }
   fprintf(fo,"\tRasterInfo End\n");
   fprintf(fo,"DatasetHeader End\n");

   fclose (fo);
   return (TRUE);
}                                                                                                                                                                                   

int
#if XVT_CC_PROTO
ERMapperVecHeader(char *fname)
#else
ERMapperVecHeader(fname)
char *fname;
#endif
{
   FILE *fo;
   long cellsize, rows, cols;
   double gblock;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   double gx1, gy1, gx2, gy2;
   time_t longTime;
   struct tm *today;
   char timeString[50];
   int length;

   time(&longTime); /* current Time */
   today = gmtime(&longTime);

   gx1 = viewOptions->originX;
   gy1 = viewOptions->originY;
   gx2 = viewOptions->originX + viewOptions->lengthX;
   gy2 = viewOptions->originY + viewOptions->lengthY;
   gblock = viewOptions->geophysicsCubeSize;
   
   cellsize=(long)gblock;
   rows=(long)((gy2-gy1)/gblock);
   cols=(long)((gx2-gx1)/gblock);

   addFileExtention (fname, ".erv");
   
   if (!(fo = (FILE *) fopen (fname, "w")))
      return (FALSE);

   fprintf(fo,"DatasetHeader Begin\n");
   fprintf(fo,"\tVersion \t= \"3.0\"\n");
   sprintf(timeString,"%s",asctime(today));
   if (!strstr(timeString,"GMT"))
   {
      length = strlen(timeString);
      strcpy (&(timeString[40]), &(timeString[length-6])); /* preserve " 1996\n" */
      strcpy (&(timeString[length-5]), "GMT");             /* add "GMT" */
      strcpy (&(timeString[length-2]), &(timeString[40])); /* append " 1996\n" */
   }
   fprintf(fo,"\tLastUpdated\t= %s", timeString);
   fprintf(fo,"\tDataSetType\t= ERStorage\n");
   fprintf(fo,"\tDataType\t= Vector\n");
#if XVTWS == WIN32WS	
   fprintf(fo,"\tByteOrder\t= LSBFirst\n");
#else
   fprintf(fo,"\tByteOrder\t= MSBFirst\n");
#endif


/*
   fprintf(fo,"\tCoordinateSpace Begin\n");
   fprintf(fo,"\t\tDatum\t\t= \"RAW\"\n");
   fprintf(fo,"\t\tProjection\t= \"RAW\"\n");
   fprintf(fo,"\t\tCoordinateType\t= RAW\n");
   fprintf(fo,"\t\tUnits\t\t= \"METERS\"\n");
   fprintf(fo,"\tCoordinateSpace End\n");
*/
   fprintf(fo,"\tCoordinateSpace Begin\n");
   fprintf(fo,"\t\tDatum\t\t= \"AGD66\"\n");
   fprintf(fo,"\t\tProjection\t= \"TMAMG55\"\n");
   fprintf(fo,"\t\tCoordinateType\t= EN\n");
   fprintf(fo,"\t\tUnits\t\t= \"METERS\"\n");
   fprintf(fo,"\t\tRotation\t= 0:0:0.0\n");
   fprintf(fo,"\tCoordinateSpace End\n");

   fprintf(fo,"\tVectorInfo Begin\n");
   fprintf(fo,"\t\tType\t= ERVEC\n");
   fprintf(fo,"\t\tFileFormat\t= ASCII\n");

   fprintf(fo,"\t\tExtents Begin\n");

   fprintf(fo,"\t\t\tTopLeftCorner Begin\n");
   fprintf(fo,"\t\t\t\tEastings\t=%lf\n",gx1+projectOptions.easting);
   fprintf(fo,"\t\t\t\tNorthings\t=%lf\n",gy2+projectOptions.northing);
   fprintf(fo,"\t\t\tTopLeftCorner End\n");

   fprintf(fo,"\t\t\tBottomRightCorner Begin\n");
   fprintf(fo,"\t\t\t\tEastings\t=%lf\n",gx2);
   fprintf(fo,"\t\t\t\tNorthings\t=%lf\n",gy1);
   fprintf(fo,"\t\t\tBottomRightCorner End\n");

   fprintf(fo,"\t\tExtents End\n");

   fprintf(fo,"\tVectorInfo End\n");

   fprintf(fo,"DatasetHeader End\n");

   fclose(fo);
   
   return (TRUE);
}                                                                                                                                                                                   
   
int
#if XVT_CC_PROTO
DoERMapperLineMap(int nx, int ny, char *filename)
#else
DoERMapperLineMap(nx, ny, filename)
int nx,ny;
char *filename;
#endif
{
   register int l, m, j, k;
   int cyphno=0, index, ratio, ncon;
   OBJECT *event;
   STRATIGRAPHY_OPTIONS *stratOptions = NULL;
   unsigned char cypher[200][ARRAY_LENGTH_OF_STRAT_CODE];
   unsigned int flavor;
   double dots1, ***dots, **ERMap;
   double gblock;
   double msize = 0.0; /* geophysicsOptions.calculationRange; */
   FILE *MacNewOpen(),*svec;
   char vecname[255];
   char memLabel[10], *pointPtr;
   struct story **histoire;
   int numEvents = (int) countObjects(NULL_WIN);
   double cval[16];
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   gblock = viewOptions->geophysicsCubeSize;

   strcpy (memLabel, "ERMVec");
   
   strcpy(vecname, filename);
   ERMapperVecHeader(filename);

   if (pointPtr = (char *) strchr (vecname, '.'))
      *pointPtr = '\0';  /* remove any extention we have */
   if (!(svec = (FILE *) fopen (vecname, "w")))
      return (FALSE);

   ncon=7;

   if (!(histoire = (STORY **) create2DArray (ny+1, nx+1, sizeof(STORY))))
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      memManagerFreeAfterLabel (memLabel);
      return (FALSE);
   }

   if (!(dots = (double ***) create3DArray (ny+1, nx+1, 4, sizeof(double))))
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      destroy2DArray ((char **) histoire, ny+1, nx+1);
      memManagerFreeAfterLabel (memLabel);
      return (FALSE);
   }
  
   if (!(ERMap = (double **) create2DArray (ny+1, nx+1, sizeof(double))))
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      destroy3DArray ((char ***) dots, ny+1, nx+1, 4);
      destroy2DArray ((char **) histoire, ny+1, nx+1);
      memManagerFreeAfterLabel (memLabel);
      return (FALSE);
   }
     
 
   ldotmp(dots, histoire, 99, cypher, &cyphno, nx, ny);

   /* mplot(4.0,3.3,-3);   */

   initLongJob (0, cyphno, "Writing ERM Vectory Data", memLabel);
  
   for (j = 1; j <= cyphno; j++)
   {
      if (abortLongJob ())
         return (FALSE);
         
      incrementLongJob (INCREMENT_JOB);
                
      taste(numEvents, cypher[j], &flavor, &index);

      if (flavor != IGNEOUS_STRAT)
      {
         for (k = 1; k <= nx; k++)
         {
            for (l = 1; l <= ny; l++)
            {
               dots1 = dots[l][k][3];
               if (coincide(histoire[l][k].sequence, cypher[j]))
                  ERMap[ny-l+1][k]=dots1;
               else
                  ERMap[ny-l+1][k]=1.0e8;
            }
         }

         if (flavor == BASE_STRAT)
            index = 0;
               
           /* if the current event does not have a stratigraphy
           ** associated with it then use the base stratigraphy */
         if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
            event = (OBJECT *) nthObject (NULL_WIN, 0);
         if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
                                getStratigraphyOptionsStructure (event)))
         {
            if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
               getStratigraphyOptionsStructure(
                                      (OBJECT *) nthObject (NULL_WIN, 0))))
            {
                memManagerFreeAfterLabel (memLabel);
               return (FALSE);  /* not even a base strat is defined yet */
            }
         }

         for (m = 0; m < 10; m++)
            cval[m] = -99999.0;  /* initialise off the edge */
                         /* assign cval the height of each layer but first */
         for (m = 1; m <= (stratOptions->numLayers-1); m++)
            cval[m] = stratOptions->properties[m].height;
               
         ncon = stratOptions->numLayers;

         ERMapper_mcontr(ERMap, ny, nx, cval, ncon, 1.0e6,
                         gblock, -gblock*1.0, -gblock*2.0,
                         svec, (int) msize);
      }

      for (k = 1; k <= nx; k++)
      {
         for (l = 1; l <= ny; l++)
         {
            dots1 = dots[l][k][3];
            if (coincide(histoire[l][k].sequence, cypher[j]))
               ERMap[ny-l+1][k]=1.0;
            else
               ERMap[ny-l+1][k]=-1.0;
         }
      }
      ncon=1;

      cval[1]=0.0;
      ratio=1;

      ERMapper_mcontr(ERMap, ny, nx, cval, ncon, 1.0e6,
                      gblock, -gblock*1.0, -gblock*2.0,
                      svec, (int) msize);
   }
   
   fclose (svec);

   destroy3DArray ((char ***) dots, ny+1, nx+1, 4);
   destroy2DArray ((char **) histoire, ny+1, nx+1);
   destroy2DArray ((char **) ERMap, ny+1, nx+1);

   memManagerFreeAfterLabel (memLabel);
   finishLongJob ();

   return (TRUE);
}

int
#if XVT_CC_PROTO
ERMapper_mcontr(double **zmap, int jmax, int kmax, double conval[20], int ncon,
                double cutoff, double sc, double xo, double yo, FILE *svec, int arrayoff)
#else
ERMapper_mcontr(zmap,jmax,kmax,conval,ncon,cutoff,sc,xo,yo,svec,arrayoff)
double  **zmap,sc;
double  cutoff,xo,yo;
double  conval[20];
int jmax,kmax,ncon,arrayoff;
FILE *svec;
#endif
{
   int tj1,tk1,tj2,tk2;
   static int pnts[13][3];
   int icon[20];
   double  xscal,yscal,conx[20][3],cony[20][3];
   register int j,k,l,m,inc,itri,iside;
   double  x1,y1,z1,x2,y2,z2,delcon;
   BOOLEAN flag = FALSE;

   xscal=sc;
   yscal=sc;

   pnts[1][1]=1;
   pnts[1][2]=0;
   pnts[2][1]=0;
   pnts[2][2]=1;
     
   pnts[3][1]=0;
   pnts[3][2]=0;
   pnts[4][1]=0;
   pnts[4][2]=1;
     
   pnts[5][1]=0;
   pnts[5][2]=0;
   pnts[6][1]=1;
   pnts[6][2]=0;
     
   pnts[7][1]=1;
   pnts[7][2]=0;
   pnts[8][1]=0;
   pnts[8][2]=1;
     
   pnts[9][1]=0;
   pnts[9][2]=1;
   pnts[10][1]=1;
   pnts[10][2]=1;
     
   pnts[11][1]=1;
   pnts[11][2]=0;
   pnts[12][1]=1;
   pnts[12][2]=1;
        
   for (j = 1; j <= jmax-1; j++)
   {
      for (k = 1;k <= kmax-1; k++)
      {
         
         for (itri = 1, inc = 1; itri <= 2; itri++)
         {
            for (m = 0; m < ncon; m++)
            {
               icon[m]=0;
            }
                
            for (iside = 1; iside <= 3; iside++)
            {
               tj1 = j + pnts[inc][1];
               tk1 = k + pnts[inc][2];
               z1 = zmap[tj1][tk1];
               tj2 = j + pnts[inc+1][1];
               tk2 = k + pnts[inc+1][2];
               z2 = zmap[tj2][tk2];
               y1 = yo + (tj1*yscal);
               x1 = xo + (tk1*xscal);
               y2 = yo + (tj2*yscal);
               x2 = xo + (tk2*xscal);
               inc = inc + 2;
         
               for (l = 0; l < ncon; l++)
               {
                  if (z1 < cutoff && z2 < cutoff)
                  {
                     if ((z1 < conval[l+1] && z2 >= conval[l+1]) || (z1 >= conval[l+1] && z2 < conval[l+1]))
                     {
                        icon[l]=icon[l]+1;
                        delcon=(conval[l+1]-z1)/(z2-z1);
                        if (iside == 1)
                        {
                           conx[l][icon[l]]=x1+((x2-x1)*delcon);
                           cony[l][icon[l]]=y1+((y2-y1)*delcon);
                        }
                        else
                        {
                           conx[l][icon[l]]=((x2-x1)*delcon)+x1;
                           cony[l][icon[l]]=((y2-y1)*delcon)+y1;
                        }
                     }
                  }
               }
            }
 
 
            for (l = 0; l < ncon; l++)
            {
               if(icon[l] == 2)
               {
                  fprintf(svec,"poly(a line, 2, [%lf,%lf,%lf,%lf],0,0,1,0,0,1).\n",
                       conx[l][1]+projectOptions.easting, cony[l][1]+projectOptions.northing,
                       conx[l][2]+projectOptions.easting, cony[l][2]+projectOptions.northing);
               }
            }
         }
      }
   }

   return (TRUE);
}
 

WINDOW
#if XVT_CC_PROTO
displayERMImage(char *filename)
#else
displayERMImage(filename)
char *filename;
#endif
{
   WINDOW imageWindow;
   ANOMIMAGE_DATA *anomImageData;
   double **imageData;
   double minValue, maxValue;
   double interpolate = projectOptions.imageScalingFactor;
	double cellSize, originX, originY;
   int msbFirst;
	int nx, ny, bands;
	char format[ERM_FORMAT_LENGTH];
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
   if (!readERMHeader (filename, &nx, &ny, &bands, format, &msbFirst, &cellSize, &originX, &originY))
   {
      xvt_dm_post_error("This is not an Valid ERMapper File");
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

   readERMGeophys (filename, imageData, nx, ny, bands, format, msbFirst,
                             &maxValue, &minValue);

	strcpy(anomImageData->historyFile, "ERM Import");
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
   if (!updateAnomaliesImageDisplay(imageWindow))
	{
      xvt_dm_post_error("Try reducing the Geophysics Display Scale.");
      xvt_vobj_set_data (imageWindow, (long) 0L);
      destroy2DArray ((char **) imageData, ny, nx);
      xvt_mem_free ((char *) anomImageData);
		xvt_vobj_destroy(imageWindow);
	   finishLongJob ();
		return (NULL_WIN);
	}
   
   xvt_vobj_set_data (imageWindow, (long) anomImageData);

   finishLongJob ();

   xvt_dwin_invalidate_rect (imageWindow, NULL);

   xvt_vobj_set_visible (imageWindow, TRUE);
   bringWindowToFront(imageWindow);
   
   return (imageWindow);
}

int
#if XVT_CC_PROTO
readERMHeader(char *filename, int *nx, int *ny, int *bands, char format[ERM_FORMAT_LENGTH], int *msbFirst,
				  double *cellSize, double *originX, double *originY)
#else
readERMHeader(filename, nx, ny, bands, format, msbFirst, cellSize, originX, originY)
char *filename;
int *nx, *ny, *bands;
char format[ERM_FORMAT_LENGTH];
int *msbFirst;
double *cellSize, *originX, *originY;
#endif
{
#define FIELD_LENGTH 100
   FILE *fi;
	char value[FIELD_LENGTH], *ptr;
	int error = FALSE;
	double regXLoc, regYLoc, xCellDim, yCellDim;

   if (!(fi = fopen (filename, "r")))
      return (FALSE);

	*originX = 0.0;
	*originY = 0.0;
	*cellSize = 100.0;

	if (readGeophysComment (fi, "DataType", value, FIELD_LENGTH-1, TRUE))
	{
		if (!strstr(value, "Raster"))
			error = TRUE;
	}
	else
		error = TRUE;

	if (readGeophysComment (fi, "ByteOrder", value, FIELD_LENGTH-1, TRUE))
	{
		if (strstr(value, "MSBFirst"))
			*msbFirst = TRUE;
		else
			*msbFirst = FALSE;
	}
	else
		error = TRUE;

	if (readGeophysComment (fi, "RasterInfo Begin", value, FIELD_LENGTH-1, TRUE))
	{
		if (!error)
		{
		   error = !readGeophysComment (fi, "CellType", format, ERM_FORMAT_LENGTH-1, FALSE);
		}
		if (!error)
		{
		   error = !readGeophysComment (fi, "NrOfLines", value, FIELD_LENGTH-1, FALSE);
			if (ptr = strchr(value, '='))
				sscanf(ptr+1,"%d", ny);
		}
		if (!error)
		{
		   error = !readGeophysComment (fi, "NrOfCellsPerLine", value, FIELD_LENGTH-1, FALSE);
			if (ptr = strchr(value, '='))
				sscanf(ptr+1,"%d", nx);
		}
		if (!error)
		{
		   error = !readGeophysComment (fi, "NrOfBands", value, FIELD_LENGTH-1, FALSE);
			if (ptr = strchr(value, '='))
				sscanf(ptr+1,"%d", bands);
		}

		         /* The optional material */
		if (readGeophysComment (fi, "CellInfo Begin", value, FIELD_LENGTH-1, TRUE))
		{
		   xCellDim = yCellDim = *cellSize;
			if (readGeophysComment (fi, "Xdimension", value, FIELD_LENGTH-1, FALSE))
				if (ptr = strchr(value, '='))
					sscanf(ptr+1,"%lf", &xCellDim);
		   if (readGeophysComment (fi, "Ydimension", value, FIELD_LENGTH-1, FALSE))
				if (ptr = strchr(value, '='))
					sscanf(ptr+1,"%lf", &yCellDim);

			*cellSize = MIN(xCellDim, yCellDim);
		}

		if (readGeophysComment (fi, "RegistrationCoord Begin", value, FIELD_LENGTH-1, TRUE))
		{
		   if (readGeophysComment (fi, "Eastings", value, FIELD_LENGTH-1, FALSE))
				if (ptr = strchr(value, '='))
					sscanf(ptr+1,"%lf", originX);
		   if (readGeophysComment (fi, "Northings", value, FIELD_LENGTH-1, FALSE))
				if (ptr = strchr(value, '='))
					sscanf(ptr+1,"%lf", originY);

			if (readGeophysComment (fi, "RegistrationCellX", value, FIELD_LENGTH-1, TRUE))
			{
				if (ptr = strchr(value, '='))
					sscanf(ptr+1,"%lf", &regXLoc);
			}
			else
				regXLoc = 0.0;
			if (readGeophysComment (fi, "RegistrationCellY", value, FIELD_LENGTH-1, TRUE))
			{
				if (ptr = strchr(value, '='))
					sscanf(ptr+1,"%lf", &regYLoc);
			}
			else
				regYLoc = 0.0;

		}
                  /* Move the origin from Top Left to Bottom Left */
		*originX = *originX - regXLoc*xCellDim;
		*originY = *originY - (*ny - regYLoc)*yCellDim;
	}
	else
		error = TRUE;

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
readERMGeophys(char *filename, double **imageData,
					int nx, int ny, int bands, char format[ERM_FORMAT_LENGTH], int msbFirst,
				   double *maxValue, double *minValue)
#else
readERMGeophys(filename, imageData, nx, ny, bands, format, msbFirst, maxValue, minValue)
char *filename;
double **imageData;
int nx, ny, bands;
char format[ERM_FORMAT_LENGTH];
int msbFirst;
double *maxValue, *minValue;
#endif
{
	FILE *fi;
	int size, x, y, band, numRead, bandToRead = 0;
	char buffer[16], *ptr;
   unsigned char *usign8Int;
   char *sign8Int;
   unsigned short *usign16Int;
   short *sign16Int;
   unsigned long *usign32Int;
   long *sign32Int;
	float *ieee4Real;
	double *ieee8Real, value;
	enum { Unsigned8BitInteger,  Signed8BitInteger,
			 Unsigned16BitInteger, Signed16BitInteger,
			 Unsigned32BitInteger, Signed32BitInteger,
			 IEEE4ByteReal,        IEEE8ByteReal } type;

   if (!filename || !imageData)
      return (FALSE);

	usign8Int = (unsigned char *) &(buffer[0]);
	sign8Int = (char *) &(buffer[0]);
	usign16Int = (unsigned short *) &(buffer[0]);
	sign16Int = (short *) &(buffer[0]);
	usign32Int = (unsigned long *) &(buffer[0]);
	sign32Int = (long *) &(buffer[0]);
	ieee4Real = (float *) &(buffer[0]);
	ieee8Real = (double *) &(buffer[0]);

   if (ptr = strrchr (filename, '.'))
		*ptr = '\0';
	if (!(fi = fopen (filename, "rb")))
      return (FALSE);
   if (ptr)  /* Restore '.' */
		*ptr = '.';
 

	if (strstr(format, "Unsigned8BitInteger"))
		type = Unsigned8BitInteger;
	else if (strstr(format, "Signed8BitInteger"))
		type = Signed8BitInteger;
	else if (strstr(format, "Unsigned16BitInteger"))
		type = Unsigned16BitInteger;
	else if (strstr(format, "Signed16BitInteger"))
		type = Signed16BitInteger;
	else if (strstr(format, "Unsigned32BitInteger"))
		type = Unsigned32BitInteger;
	else if (strstr(format, "Signed32BitInteger"))
		type = Signed32BitInteger;
	else if (strstr(format, "IEEE4ByteReal"))
		type = IEEE4ByteReal;
	else if (strstr(format, "IEEE8ByteReal"))
		type = IEEE8ByteReal;
	else
		return (FALSE);

	switch (type)
	{
	case (Unsigned8BitInteger):  case (Signed8BitInteger):
		size = 1;
		break;
	case (Unsigned16BitInteger): case (Signed16BitInteger):
		size = 2;
		break;
	case (Unsigned32BitInteger): case (Signed32BitInteger):
		size = 4;
		break;
	case (IEEE4ByteReal):
		size = 4;
		break;
	case (IEEE8ByteReal):
		size = 8;
		break;
	}

	for (y = 0; y < ny; y++)
	{
	   for (band = 0; band < bands; band++)
		{
		   for (x = 0; x < nx; x++)
			{
				numRead = fread (buffer, size, 1, fi);
				if (numRead == 0)
				{
					fclose(fi);
					return (FALSE);
				}

				if (band == bandToRead)
				{
					switch (type)
					{
					case (Unsigned8BitInteger):
						value = (double) *usign8Int;
						break;
					case (Signed8BitInteger):
						value = (double) *sign8Int;
						break;
					case (Unsigned16BitInteger):
						value = (double) *usign16Int;
						break;
					case (Signed16BitInteger):
						value = (double) *sign16Int;
						break;
					case (Unsigned32BitInteger):
						value = (double) *usign32Int;
						break;
					case (Signed32BitInteger):
						value = (double) *sign32Int;
						break;
					case (IEEE4ByteReal):
						value = (double) *ieee4Real;
						break;
					case (IEEE8ByteReal):
						value = (double) *ieee8Real;
						break;
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
	}

	fclose (fi);

	return (TRUE);
}

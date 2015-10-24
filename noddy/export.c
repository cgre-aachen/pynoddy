#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include <time.h>

#define DEBUG(X)    

#define STRAT_LIMIT           100


extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;


#if XVT_CC_PROTO
int loadFileFormatWindow (WINDOW, FILE_FORMAT *);
int saveFileFormatWindow (WINDOW, FILE_FORMAT *);
int updateFileFormatWindow (WINDOW, FILE_FORMAT *);
int saveDicerBlock (FILE_SPEC *, FILE_FORMAT *, double, double, double, double, int, int, int);
int writeTabulatedColumnBlock(FILE_SPEC *, FILE_FORMAT *, double, double, double, double, int, int, int);
#else
int loadFileFormatWindow ();
int saveFileFormatWindow ();
int updateFileFormatWindow ();
int saveDicerBlock ();
int writeTabulatedColumnBlock();
#endif


/* ======================================================================
FUNCTION        loadFileFormatWindow
DESCRIPTION
     loads all the options for the file format window

INPUT 

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadFileFormatWindow (WINDOW win, FILE_FORMAT *fileFormat)
#else
loadFileFormatWindow (win, fileFormat)
WINDOW win;
FILE_FORMAT *fileFormat;
#endif
{
   WINDOW propList;

   if (!fileFormat)
      fileFormat = (FILE_FORMAT *) xvt_vobj_get_data(win);
         
   if (!fileFormat || !win)
      return (FALSE);
   
   propList = xvt_win_get_ctl(win, FILE_PROPERTY);
   xvt_list_suspend (propList);
   xvt_list_clear(propList);
   xvt_list_add(propList, 0, "Rock Types (Indexed)");
   xvt_list_add(propList, 1, "Density");
   xvt_list_add(propList, 2, "Susceptibility X");
   xvt_list_add(propList, 3, "Susceptibility Y");
   xvt_list_add(propList, 4, "Susceptibility Z");
   xvt_list_add(propList, 5, "Susceptibility Dip");
   xvt_list_add(propList, 6, "Susceptibility Dip Direction");
   xvt_list_add(propList, 7, "Susceptibility Pitch");
   xvt_list_add(propList, 8, "Remanence Strength");
   xvt_list_add(propList, 9, "Remanence Inclination");
   xvt_list_add(propList, 10, "Remanence Declination");
   xvt_list_set_sel(propList, fileFormat->property, TRUE);
   xvt_list_resume (propList);
   
   xdCheckRadioButton(win, FILE_ORDER_X1+fileFormat->xOrder, FILE_ORDER_X1, FILE_ORDER_Z3);
   xdCheckRadioButton(win, FILE_ORDER_Y1+fileFormat->yOrder, FILE_ORDER_Y1, FILE_ORDER_Y3);
   xdCheckRadioButton(win, FILE_ORDER_Z1+fileFormat->zOrder, FILE_ORDER_Z1, FILE_ORDER_Z3);

   xvt_ctl_set_checked(xvt_win_get_ctl(win, FILE_SINGLE_VALUE), (BOOLEAN) fileFormat->singleValuePerLine);

   xdCheckRadioButton(win, FILE_DIR_X_EW+fileFormat->xDirection, FILE_DIR_X_EW, FILE_DIR_X_WE);
   xdCheckRadioButton(win, FILE_DIR_Y_NS+fileFormat->yDirection, FILE_DIR_Y_NS, FILE_DIR_Y_SN);
   xdCheckRadioButton(win, FILE_DIR_Z_UD+fileFormat->zDirection, FILE_DIR_Z_UD, FILE_DIR_Z_DU);


   updateFileFormatWindow (win, fileFormat);

   return (TRUE);
}

/* ======================================================================
FUNCTION        saveFileFormatWindow
DESCRIPTION
     save all the options for the file format window
     and calls the writeing routines for the appropriate files

INPUT 

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
saveFileFormatWindow (WINDOW win, FILE_FORMAT *fileFormat)
#else
saveFileFormatWindow (win, fileFormat)
WINDOW win;
FILE_FORMAT *fileFormat;
#endif
{
   FILE_SPEC fs;
   BLOCK_VIEW_OPTIONS *blockView = (BLOCK_VIEW_OPTIONS *) getViewOptions();
   double blockSize;
   int nx, ny, nz;

   if (!fileFormat)
      fileFormat = (FILE_FORMAT *) xvt_vobj_get_data(win);
         
   if (!fileFormat || !win)
      return (FALSE);
   
   fileFormat->property = xvt_list_get_sel_index(xvt_win_get_ctl(win, FILE_PROPERTY));

   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_X1)))
      fileFormat->xOrder = FIRST;
   else if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_X2)))
      fileFormat->xOrder = SECOND;
   else
      fileFormat->xOrder = THIRD;
   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_Y1)))
      fileFormat->yOrder = FIRST;
   else if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_Y2)))
      fileFormat->yOrder = SECOND;
   else
      fileFormat->yOrder = THIRD;
   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_Z1)))
      fileFormat->zOrder = FIRST;
   else if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_Z2)))
      fileFormat->zOrder = SECOND;
   else
      fileFormat->zOrder = THIRD;
      
   fileFormat->singleValuePerLine = xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_SINGLE_VALUE));


   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_DIR_X_EW)))
      fileFormat->xDirection = EAST_WEST;
   else
      fileFormat->xDirection = WEST_EAST;
   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_DIR_Y_NS)))
      fileFormat->yDirection = NORTH_SOUTH;
   else
      fileFormat->yDirection = SOUTH_NORTH;
   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_DIR_Z_UD)))
      fileFormat->zDirection = UP_DOWN;
   else
      fileFormat->zDirection = DOWN_UP;


   xvt_vobj_set_visible(win, FALSE);

   if (fileFormat->access == OUTPUT)
      getCurrentFileName (&fs);
   
   switch (fileFormat->type)
   {
      case (DICER_BLOCK_FILE):
#if (XVTWS == MACWS)
         strcpy (fs.type, "TEXT");
#else
         strcpy (fs.type, "dic");
#endif
         if (fileFormat->access == OUTPUT)
            addFileExtention (fs.name, ".dic");
         break;
      case (TABULATED_COL_BLOCK_FILE):
#if (XVTWS == MACWS)
         strcpy (fs.type, "TEXT");
#else
         strcpy (fs.type, "tab");
#endif
         if (fileFormat->access == OUTPUT)
            addFileExtention (fs.name, ".tab");
         break;
   }

                              /* Get the filename */
   getDefaultDirectory(&(fs.dir));
   if (fileFormat->access == INPUT)
   {
      switch (xvt_dm_post_file_open(&fs, "Import from ...")) {
      case FL_OK:
         if (strlen(fs.name) == 0)
         {
            xvt_dm_post_error ("Error, No file Specified");
            break;
         }
         break;
      case FL_BAD:
         xvt_dm_post_error("Error getting file name.");
         return (FALSE);
         break;
      case FL_CANCEL:
         return (FALSE);
         break;
      }
   }
   else
   {
      switch (xvt_dm_post_file_save(&fs, "Export too ...")) {
      case FL_OK:
         if (strlen(fs.name) == 0)
         {
            xvt_dm_post_error ("Error, No file Specified");
            break;
         }
         break;
      case FL_BAD:
         xvt_dm_post_error("Error getting file name.");
         return (FALSE);
         break;
      case FL_CANCEL:
         return (FALSE);
         break;
      }
   }

                        /* do the operation */
   setDefaultDirectory(&(fs.dir));
   switch (fileFormat->type)
   {
      case (DICER_BLOCK_FILE):
         blockSize = blockView->geologyCubeSize;
         nx = (int) (blockView->lengthX/blockSize);
         ny = (int) (blockView->lengthY/blockSize);
         nz = (int) (blockView->lengthZ/blockSize);
         if (!saveDicerBlock (&fs, fileFormat, blockView->originX,
                           blockView->originY, blockView->originZ,
                           blockSize, nx, ny, nz))
            xvt_dm_post_error("Error, Could not create Dicer file");
         break;
      case (TABULATED_COL_BLOCK_FILE):
         blockSize = blockView->geologyCubeSize;
         nx = (int) (blockView->lengthX/blockSize);
         ny = (int) (blockView->lengthY/blockSize);
         nz = (int) (blockView->lengthZ/blockSize);
         if (!writeTabulatedColumnBlock (&fs, fileFormat, blockView->originX,
                           blockView->originY, blockView->originZ,
                           blockSize, nx, ny, nz))
            xvt_dm_post_error("Error, Could not create Dicer file");
         break;
   }
      
   return (TRUE);
}

/* ======================================================================
FUNCTION        updateFileFormatWindow
DESCRIPTION
     update all the options for the file format window

INPUT 

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateFileFormatWindow (WINDOW win, FILE_FORMAT *fileFormat)
#else
updateFileFormatWindow (win, fileFormat)
WINDOW win;
FILE_FORMAT *fileFormat;
#endif
{
   int xOrder, yOrder, zOrder;
   if (!fileFormat)
      fileFormat = (FILE_FORMAT *) xvt_vobj_get_data(win);
         
   if (!fileFormat || !win)
      return (FALSE);
      
   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_X1)))
      xOrder = 0;
   else if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_X2)))
      xOrder = 1;
   else
      xOrder = 2;

   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_Y1)))
      yOrder = 0;
   else if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_Y2)))
      yOrder = 1;
   else
      yOrder = 2;
      
   if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_Z1)))
      zOrder = 0;
   else if (xvt_ctl_is_checked(xvt_win_get_ctl(win, FILE_ORDER_Z2)))
      zOrder = 1;
   else
      zOrder = 2;
   
   if ((xOrder == yOrder) || (xOrder == zOrder) || (yOrder == zOrder))
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, FILE_FORMAT_OK), FALSE);
   else
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, FILE_FORMAT_OK), TRUE);


   return (TRUE);
}

/* ======================================================================
FUNCTION        saveDicerBlock
DESCRIPTION
     save a block model in dicer format

INPUT 
     char *filename;     the output file for the block

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
saveDicerBlock (FILE_SPEC *filename, FILE_FORMAT *fileFormat, double originX,
    double originY, double originZ, double blockSize, int nx, int ny, int nz)
#else
saveDicerBlock (filename, fileFormat, originX, originY, originZ, blockSize, nx, ny, nz)
FILE_SPEC *filename;
FILE_FORMAT *fileFormat;
double originX, originY, originZ;
double blockSize;
int nx, ny, nz;
#endif
{
   FILE *outFile;
   float ***valueData = NULL, scaleValue = (float) 1.00000000;
   int dim1, dim2, dim3;
   int dim1Max, dim2Max, dim3Max, dim1Dir, dim2Dir, dim3Dir;
   int *dimMap[3], dimMaxMap[3], dimDirMap[3], *x, *y, *z;
   
   if (!(valueData = (float ***) calcBlockPropertiesData (fileFormat->property,
                                 originX, originY, originZ, blockSize, nx, ny, nz)))
      return (FALSE);

   if ((fileFormat->property == SUS_X_PROP) || (fileFormat->property == SUS_X_PROP) || (fileFormat->property == SUS_X_PROP))
	{
	   if (projectOptions.susceptibilityUnits == SI)
			scaleValue = (float) CGS_TO_SI;
	}
   else if (fileFormat->property == DENSITY_PROP)
	{
		scaleValue = (float) (1.0/1000.0);  /* Density is stored *1000 */
	}
                  
          /* Setup mapping of data */
   dimMap[0] = &dim1;
   dimMap[1] = &dim2;
   dimMap[2] = &dim3;
   
   dimMaxMap[fileFormat->xOrder] = nx;
   dimMaxMap[fileFormat->yOrder] = ny;
   dimMaxMap[fileFormat->zOrder] = nz;
   dimDirMap[fileFormat->xOrder] = (int) fileFormat->xDirection; 
   dimDirMap[fileFormat->yOrder] = !((int) fileFormat->yDirection);
   dimDirMap[fileFormat->zOrder] = (int) fileFormat->zDirection; 
   x = dimMap[fileFormat->xOrder];
   y = dimMap[fileFormat->yOrder];
   z = dimMap[fileFormat->zOrder];
   
   dim1Max = dimMaxMap[0];
   dim2Max = dimMaxMap[1];
   dim3Max = dimMaxMap[2];
   dim1Dir = dimDirMap[0];
   dim2Dir = dimDirMap[1];
   dim3Dir = dimDirMap[2];

            /* Write out a small header with dimensions etc. */
   xvt_fsys_set_dir (&(filename->dir));
   addFileExtention(filename->name, ".hdr");
   if (outFile = (FILE *) fopen(filename->name, "w"))
   {
      fprintf(outFile, "#Noddy Raw Header File\n");
      fprintf(outFile, "#Version = 1.1\n");
      fprintf(outFile, "Dim 1 = %d\n",dimMaxMap[0]);
      fprintf(outFile, "Dim 2 = %d\n",dimMaxMap[1]);
      fprintf(outFile, "Dim 3 = %d\n",dimMaxMap[2]);
      fprintf(outFile, "Block Size = %lf\n",blockSize);
      fclose (outFile);
   }

   addFileExtention(filename->name, ".dic");
   if (!(outFile = (FILE *) fopen(filename->name, "w")))
   {
      destroy3DArray ((char ***) valueData, nz, nx, ny);
      return (FALSE);
   }
    
         /* Write Out the data */
   if (dim3Dir)
      dim3 = 0;
   else
      dim3 = dim3Max-1;
   while ((dim3 >= 0) && (dim3 < dim3Max))
   {
      if (dim2Dir)
         dim2 = 0;
      else
         dim2 = dim2Max-1;
      while ((dim2 >= 0) && (dim2 < dim2Max))
      {
         if (dim1Dir)
            dim1 = 0;
         else
            dim1 = dim1Max-1;
         while ((dim1 >= 0) && (dim1 < dim1Max))
         {
            fprintf(outFile, "%f\t", valueData[*z][*x][*y]*scaleValue);

            if (dim1Dir)
               dim1++;
            else
               dim1--;

            if (fileFormat->singleValuePerLine)
               fprintf(outFile,"\n");
         }
         
         if (dim2Dir)
            dim2++;
         else
            dim2--;

         if (!fileFormat->singleValuePerLine) /* Dont want blank lines in singleValuePerLine */
            fprintf(outFile,"\n");
      }
      
      if (dim3Dir)
         dim3++;
      else
         dim3--;

      if (!fileFormat->singleValuePerLine) /* Dont want blank lines in singleValuePerLine */
	      fprintf(outFile,"\n");
   }
   
   fclose (outFile);
   
   destroy3DArray ((char ***) valueData, nz, nx, ny);
   
   return (TRUE);
}


/* ======================================================================
FUNCTION        writeTabulatedColumnBlock
DESCRIPTION
     save out a block in tabulated column format

INPUT  

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
writeTabulatedColumnBlock (FILE_SPEC *filename, FILE_FORMAT *fileFormat, double originX,
    double originY, double originZ, double blockSize, int nx, int ny, int nz)
#else
writeTabulatedColumnBlock (filename, fileFormat, originX, originY, originZ, blockSize, nx, ny, nz)
FILE_SPEC *filename;
FILE_FORMAT *fileFormat;
double originX, originY, originZ;
double blockSize;
int nx, ny, nz;
#endif
{
   FILE *outFile;
   float ***valueData = NULL, scaleValue = (float) 1.0000000;
   int dim1, dim2, dim3;
   int dim1Max, dim2Max, dim3Max, dim1Dir, dim2Dir, dim3Dir;
   int *dimMap[3], dimMaxMap[3], dimDirMap[3], *x, *y, *z;
   char label[40];
   double halfBlock, minZ;
   
   if (!(valueData = (float ***) calcBlockPropertiesData (fileFormat->property,
                                 originX, originY, originZ, blockSize, nx, ny, nz)))
      return (FALSE);

   if ((fileFormat->property == SUS_X_PROP) || (fileFormat->property == SUS_X_PROP) || (fileFormat->property == SUS_X_PROP))
	{
	   if (projectOptions.susceptibilityUnits == SI)
			scaleValue = (float) CGS_TO_SI;
	}
   else if (fileFormat->property == DENSITY_PROP)
	{
		scaleValue = (float) (1.0/1000.0);  /* Density is stored *1000 */
	}

   minZ = originZ - nz*blockSize;
   halfBlock = blockSize / 2.0;
   switch (fileFormat->property)
   {
   case (ROCK_PROP):
      strcpy (label, "Rock#");
      break;
   case (DENSITY_PROP):
      strcpy (label, "Density");
      break;
   case (SUS_X_PROP):
      strcpy (label, "SusX");
      break;
   case (SUS_Y_PROP):
      strcpy (label, "SusY");
      break;
   case (SUS_Z_PROP):
      strcpy (label, "SusZ");
      break;
   case (SUS_DIP_PROP):
      strcpy (label, "SusDip");
      break;
   case (SUS_DDIR_PROP):
      strcpy (label, "SusDipDir");
      break;
   case (SUS_PITCH_PROP):
      strcpy (label, "SusPitch");
      break;
   case (REM_STR_PROP):
      strcpy (label, "RemStr");
      break;
   case (REM_INC_PROP):
      strcpy (label, "RemInc");
      break;
   case (REM_DEC_PROP):
      strcpy (label, "RemDec");
      break;
   default:
      strcpy (label, "Value");
      break;
   }

   addFileExtention(filename->name, ".tab");
   xvt_fsys_set_dir (&(filename->dir));
   if (!(outFile = (FILE *) fopen(filename->name, "w")))
   {
      destroy3DArray ((char ***) valueData, nz, nx, ny);
      return (FALSE);
   }
    
   dimMap[0] = &dim1;
   dimMap[1] = &dim2;
   dimMap[2] = &dim3;
   
   dimMaxMap[fileFormat->xOrder] = nx;
   dimMaxMap[fileFormat->yOrder] = ny;
   dimMaxMap[fileFormat->zOrder] = nz;
   dimDirMap[fileFormat->xOrder] = (int) fileFormat->xDirection; 
   dimDirMap[fileFormat->yOrder] = !((int) fileFormat->yDirection);
   dimDirMap[fileFormat->zOrder] = (int) fileFormat->zDirection; 
   x = dimMap[fileFormat->xOrder];
   y = dimMap[fileFormat->yOrder];
   z = dimMap[fileFormat->zOrder];
   
   dim1Max = dimMaxMap[0];
   dim2Max = dimMaxMap[1];
   dim3Max = dimMaxMap[2];
   dim1Dir = dimDirMap[0];
   dim2Dir = dimDirMap[1];
   dim3Dir = dimDirMap[2];

   fprintf(outFile, "X\tY\tZ\t%s\n", label);
   if (dim3Dir)
      dim3 = 0;
   else
      dim3 = dim3Max-1;
   while ((dim3 >= 0) && (dim3 < dim3Max))
   {
      if (dim2Dir)
         dim2 = 0;
      else
         dim2 = dim2Max-1;
      while ((dim2 >= 0) && (dim2 < dim2Max))
      {
         if (dim1Dir)
            dim1 = 0;
         else
            dim1 = dim1Max-1;
         while ((dim1 >= 0) && (dim1 < dim1Max))
         {
            fprintf(outFile, "%lf\t%lf\t%lf\t%f\n", (*x)*blockSize+originX+halfBlock,
                                                    (*y)*blockSize+originY+halfBlock,
                                                    (*z)*blockSize+ minZ  +halfBlock,
                                                    valueData[*z][*x][*y]*scaleValue);

            if (dim1Dir)
               dim1++;
            else
               dim1--;
         }
         
         if (dim2Dir)
            dim2++;
         else
            dim2--;
      }
      
      if (dim3Dir)
         dim3++;
      else
         dim3--;
   }
   
   fclose (outFile);
   
   destroy3DArray ((char ***) valueData, nz, nx, ny);
   
   return (TRUE);
}




/* ======================================================================
FUNCTION        writeFractalGraphicsBlock
DESCRIPTION
     save out a block in a fractal graphics format

INPUT  

OUTPUT

RETURNED		TRUE - all successful
				FALSE - somthing went wrong
====================================================================== */
int
#if XVT_CC_PROTO
writeFractalGraphicsBlock (FILE_SPEC *filename,
									double originX, double originY, double originZ,
									double blockSize, int nx, int ny, int nz)
#else
writeFractalGraphicsBlock (filename, originX, originY, originZ, blockSize, nx, ny, nz)
FILE_SPEC *filename;
double originX, originY, originZ;
double blockSize;
int nx, ny, nz;
#endif
{
   FILE *fo;
	FILE_SPEC blockFile;
	char valueFormat[50];
   float ***valueData = NULL, scaleValue = (float) 1.0000000;
	PROPERTY_TYPE propertyType;
	int rockNum, result = TRUE;
   int dim1, dim2, dim3, numProps;
   LAYER_PROPERTIES *rockProps[STRAT_LIMIT];
	time_t aclock;
   char label[40];

	addFileExtention(filename->name, ".fgr");
   xvt_fsys_set_dir (&(filename->dir));
   if (!(fo = (FILE *) fopen(filename->name, "w")))
      return (FALSE);

	fprintf(fo, "VERSION = %.2lf\n", VERSION_NUMBER);
	getCurrentFileName(&blockFile);
	fprintf(fo, "BLOCK FILE = %s\n", blockFile.name);
	time(&aclock);
	fprintf(fo, "DATE = %s\n", asctime(localtime(&aclock)));
	fprintf(fo, "\n");

	fprintf(fo, "BLOCK DIMENSIONS\n");
	fprintf(fo, "ORIGIN X = %lf\n", originX);
	fprintf(fo, "ORIGIN Y = %lf\n", originY);
	fprintf(fo, "ORIGIN Z = %lf\n", originZ);
	fprintf(fo, "BOTTOM X = %lf\n", originX + nx*blockSize);
	fprintf(fo, "BOTTOM Y = %lf\n", originY + ny*blockSize);
	fprintf(fo, "BOTTOM Z = %lf\n", originZ - nz*blockSize);
	fprintf(fo, "BLOCK SIZE = %lf\n", blockSize);
	fprintf(fo, "NUM BLOCKS X = %d\n", nx);
	fprintf(fo, "NUM BLOCKS Y = %d\n", ny);
	fprintf(fo, "NUM BLOCKS Z = %d\n", nz);
	fprintf(fo, "\n");

	fprintf(fo, "LAYER INFORMATION\n");
   numProps = assignPropertiesForStratLayers(rockProps, STRAT_LIMIT);
	for (rockNum = 0; rockNum < numProps; rockNum++)
	{
		fprintf(fo,"LAYER INDEX = %d\n", rockNum+1);
		fprintf(fo,"\tDESCRIPTION = %s\n", rockProps[rockNum]->unitName);
		fprintf(fo,"\tDENSITY = %lf\n", rockProps[rockNum]->density);
		fprintf(fo,"\tANISOTROPIC = %d\n", rockProps[rockNum]->anisotropicField);
		fprintf(fo,"\tSUS X = %lf\n", rockProps[rockNum]->sus_X);
		fprintf(fo,"\tSUS Y = %lf\n", rockProps[rockNum]->sus_Y);
		fprintf(fo,"\tSUS Z = %lf\n", rockProps[rockNum]->sus_Z);
		fprintf(fo,"\tSUS DIP = %lf\n", rockProps[rockNum]->sus_dip);
		fprintf(fo,"\tSUS DIP DIRECTION = %lf\n", rockProps[rockNum]->sus_dipDirection);
		fprintf(fo,"\tSUS PITCH = %lf\n", rockProps[rockNum]->sus_pitch);
		fprintf(fo,"\tREMANENT = %d\n", rockProps[rockNum]->anisotropicField);
		fprintf(fo,"\tREM STRENGTH = %lf\n", rockProps[rockNum]->strength);
		fprintf(fo,"\tREM INCLINATION = %lf\n", rockProps[rockNum]->inclination);
		fprintf(fo,"\tREM DECLINATION = %lf\n", rockProps[rockNum]->angleWithNorth);
		fprintf(fo,"\tALTERATION EFFECT = %lf\n", rockProps[rockNum]->applyAlterations);
		fprintf(fo,"\tDISPLAY COLOR RED = %lf\n", rockProps[rockNum]->color.red/255.0);
		fprintf(fo,"\tDISPLAY COLOR GREEN = %lf\n", rockProps[rockNum]->color.green/255.0);
		fprintf(fo,"\tDISPLAY COLOR BLUE = %lf\n", rockProps[rockNum]->color.blue/255.0);
	}
	fprintf(fo, "\n");

	for (propertyType = ROCK_PROP; propertyType <= REM_DEC_PROP; propertyType++)
	{
		if (!(valueData = (float ***) calcBlockPropertiesData (propertyType,
												originX, originY, originZ, blockSize, nx, ny, nz)))
		{
			result = FALSE;
			break;
		};

		switch (propertyType)
		{
		case (ROCK_PROP): /* INDEX */
			fprintf(fo, "INDEX BLOCK\n");
			strcpy(valueFormat,"%.0f\t");
			scaleValue = (float) 1.0000000;
			break;
		case (DENSITY_PROP): /* Density */
			fprintf(fo, "DENSITY BLOCK\n");
			strcpy(valueFormat,"%.3f\t");
			scaleValue = (float) 0.0010000;
			break;
		case (SUS_X_PROP): /* Sus X */
			fprintf(fo, "SUS_X BLOCK\n");
			strcpy(valueFormat,"%e\t");
			scaleValue = (float) 1.0000000;
			break;
		case (SUS_Y_PROP): /* Sus Y */
			fprintf(fo, "SUS_Y BLOCK\n");
			strcpy(valueFormat,"%e\t");
			scaleValue = (float) 1.0000000;
			break;
		case (SUS_Z_PROP): /* Sus Z */
			fprintf(fo, "SUS_Z BLOCK\n");
			strcpy(valueFormat,"%e\t");
			scaleValue = (float) 1.0000000;
			break;
		case (SUS_DIP_PROP): /* Sus Dip */
			fprintf(fo, "SUS_DIP BLOCK\n");
			strcpy(valueFormat,"%.2f\t");
			scaleValue = (float) 1.0000000;
			break;
		case (SUS_DDIR_PROP): /* Sus Dip Direction */
			fprintf(fo, "SUS_DDIR BLOCK\n");
			strcpy(valueFormat,"%.2f\t");
			scaleValue = (float) 1.0000000;
			break;
		case (SUS_PITCH_PROP): /* Sus Pitch */
			fprintf(fo, "SUS_PITCH BLOCK\n");
			strcpy(valueFormat,"%.2f\t");
			scaleValue = (float) 1.0000000;
			break;
		case (REM_STR_PROP): /* Rem Intensity */
			fprintf(fo, "REM_STR BLOCK\n");
			strcpy(valueFormat,"%e\t");
			scaleValue = (float) 1.0000000;
			break;
		case (REM_INC_PROP): /* Rem Inclination */
			fprintf(fo, "REM_INC BLOCK\n");
			strcpy(valueFormat,"%.2f\t");
			scaleValue = (float) 1.0000000;
			break;
		case (REM_DEC_PROP): /* Rem Declination */
			fprintf(fo, "REM_DEC BLOCK\n");
			strcpy(valueFormat,"%.2f\t");
			scaleValue = (float) 1.0000000;
			break;
		}

		for (dim1 = 0; dim1 < nz; dim1++)
		{
			for (dim3 = 0; dim3 < ny; dim3++)
			{
				for (dim2 = 0; dim2 < nx; dim2++)
				{
					fprintf(fo, valueFormat, valueData[dim1][dim2][dim3]*scaleValue);
				}
				fprintf(fo, "\n");
			}
			fprintf(fo, "\n");
		}
		fprintf(fo, "\n");
		
		destroy3DArray ((char ***) valueData, nz, nx, ny);
		valueData = NULL;
	}

	fclose (fo);

	return (result);
}

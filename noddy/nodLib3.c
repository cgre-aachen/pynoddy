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
#include "xvt.h"
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
#undef BOOLEAN
typedef short SHORT;
typedef long LONG;
typedef char CHAR;
#include <windows.h>
/*
//#include <shellapi.h>
//#include <commdlg.h>    // includes common dialog functionality
//#include <dlgs.h>       // includes common dialog template defines
*/
#endif
#if (XVTWS == MACWS)
#include "stdio.h"
#include "math.h"
#include "SCSI.h"
#endif
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include <time.h>
#include "titles.h"

#define DEBUG(X)     

#if XVTWS == XOLWS
#define RAND_MAX  2147483647
#endif

#if (XVTWS == MACWS)
#define kInquirySize  5
#define kInquiryCMD  0x12
#define kVendorIDSize 8
#define kProductIDSize 16
#define kRevisionSize 4
#define kWaitTime 4
#define kMySCSIID 0

struct InquiryResponse
{
            char DeviceType;       /* {SCSI types such as disk, tape} */
            char DeviceQualifier;  /* {a 7-bit user specified code} */
            char Version;          /* {version of ANSI standard} */
            char ResponseFormat;   /* {reserved} */
            char AdditionalLength; /* {length beyond 6-byte CDB format} */
            char VendorUse1;       /* {unused} */
            int Reserved1;
            char VendorID[kVendorIDSize]; 
            char ProductID[kProductIDSize];
            char Revision[kRevisionSize];
            char VendorUse2[20];
            char Reserved2[42];
            char VendorUse3[158];
} InquiryResponse;
#endif

                                /* Constants to this file */
#define MEM_MANAGER_RECORD_INCREMENT    10

                                /* External variables to this file */
extern PROJECT_OPTIONS projectOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;
extern int batchExecution;
extern WINDOW_INFO batchWindowInfo;
extern double **topographyMap;
extern int TopoCol, TopoRow;
extern ANOMIMAGE_DATA *gravityReferenceData, *magneticsReferenceData;
extern BLOCK_VIEW_OPTIONS *blockViewOptions;
extern DIRECTORY defaultDir;

                                /* Gloabal Variables */
char registrationID[100];
typedef struct s_memManager_record {

   char *data;
   int numDimensions;
   int dim1, dim2, dim3;
   int **iregDimensions;

} MEM_MANAGER_RECORD;
static MEM_MANAGER_RECORD *memManagerData = NULL;
static int numMemManagerData = 0;
static int sizeMemManagerData = 0;
int DEMO_VERSION = FALSE;

                                /* Extern Functions in this file */
#if XVT_CC_PROTO
extern int Load_status(char *);
extern int freeObjectMemory (OBJECT *);
#else
extern int Load_status();
extern int freeObjectMemory ();
#endif

                                /* Functions in this file */
#if XVT_CC_PROTO
int getWindowStartPosition (char *, short *, short *, short *, short *, short *, short *);
PLANE *getPlaneStructure (OBJECT *);
ROTATION_MATRICES *getRotationMatrices (OBJECT *);
NODDY_COLOUR *getNoddyColourStructure (OBJECT *);
double getLayerWidth (OBJECT *);
LAYER_PROPERTIES *getLayerPropertiesStructure (OBJECT *);
STRATIGRAPHY_OPTIONS *getStratigraphyOptionsStructure (OBJECT *);
PROFILE_OPTIONS *getProfileOptionsStructure (OBJECT *);
DXFHANDLE *getDxfSurfaceStructure (OBJECT *);
FILE_SPEC *getDxfSurfaceFilename (OBJECT *);
int assignPropertiesForStratLayers (LAYER_PROPERTIES **, int);
int assignEventNumForStratLayers (int *, int);
int assignFlavorForStratLayers (unsigned int *, int);
int checkHistoryForAlterations ();
void setDefaultOptions (OBJECT *);
void initSineFourierCoeff (double [2][11]);
PROFILE_POINT *initGraphPoints (PROFILE_POINT *, int, int, PROFILE_OPTIONS *);
PROFILE_OPTIONS *initAlterationZones (OBJECTS);
int initAllProfiles (OBJECT *);
int interpDblArray (double **, double **, int, int, int, int, double, double);
int overlayTopoOnArray (double **, int, int, double, double, double, double,
                        double **, int, int, double, double, double, double);
int mapArrayToArray (double **, int, int, double, double, double, double,
                     double **, int, int, double, double, double, double, int);
double getTopoValueAtLoc (double **, int, int, double, double, double,
                                               double, double, double, int *);
int findFile (FILE_SPEC *);
int incrementFilename(FILE_SPEC *);
int addFileExtention (char *, char *);
void copyit (FILE *, char *, void *);
int equalstring2 (char *, char *);
int equalstring3 (char *, char *);
void loadit (FILE *, char *, void *);
void loaditrock (FILE *, char *, void *);
void setWindowState ( WINDOW, int );
double setBlockSize (int);
int displayHelp (char *);
void processCommandLine (char **, int, char **, char **, char **,
                                                char *, OPERATIONS *);
void displayUsage (char *);
int performBatchOperations (char *, char *, OPERATIONS);
int noddy (DATA_INPUT, DATA_OUTPUT, int, char *, char *,
           BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *);
int memManagerAdd (char *, int, int, int, int, int**);
int memManagerAddLabel (char *);
int memManagerRemove (char *);
int memManagerFree (char *);
int memManagerFreeLabel (char *);
int memManagerFreeAfterLabel (char *);
int memManagerFreeAll ();
int buildUserList (char **, int *, char **, int *);
int checkLicence ();
int writeLicenceFile (char *, char *, char *);
int createLicence (int);
int createLicenceInfoFile ();
int getMachineUniqueLicence (char *);
int initProject (int, int);
#else
int getWindowStartPosition ();
PLANE *getPlaneStructure ();
ROTATION_MATRICES *getRotationMatrices ();
NODDY_COLOUR *getNoddyColourStructure ();
double getLayerWidth ();
LAYER_PROPERTIES *getLayerPropertiesStructure ();
STRATIGRAPHY_OPTIONS *getStratigraphyOptionsStructure ();
PROFILE_OPTIONS *getProfileOptionsStructure ();
DXFHANDLE *getDxfSurfaceStructure ();
FILE_SPEC *getDxfSurfaceFilename ();
int assignPropertiesForStratLayers ();
int assignEventNumForStratLayers ();
int assignFlavorForStratLayers ();
int checkHistoryForAlterations ();
void setDefaultOptions ();
void initSineFourierCoeff ();
PROFILE_POINT *initGraphPoints ();
PROFILE_OPTIONS *initAlterationZones ();
int initAllProfiles ();
int interpDblArray ();
int overlayTopoOnArray ();
int mapArrayToArray ();
double getTopoValueAtLoc ();
int findFile ();
int incrementFilename();
int addFileExtention ();
void copyit ();
int equalstring2 ();
int equalstring3 ();
void loadit ();
void loaditrock ();
void setWindowState ();
double setBlockSize ();
double getBlockSize ();
int displayHelp ();
void processCommandLine ();
void displayUsage ();
int performBatchOperations ();
int noddy ();
int memManagerAdd ();
int memManagerAddLabel ();
int memManagerRemove ();
int memManagerFree ();
int memManagerFreeLabel ();
int memManagerFreeAfterLabel ();
int memManagerFreeAll ();
int buildUserList ();
int checkLicence ();
int writeLicenceFile ();
int createLicence ();
int createLicenceInfoFile ();
int getMachineUniqueLicence ();
int initProject ();
#endif
int doTopology(char *output, char ***data);

/* ======================================================================
FUNCTION        getWindowStartPosition
DESCRIPTION
        pick out start position of a type of window
INPUT

OUTPUT
    Fills in x, y, x2, y2, width, height parameters if they are not NULL
    
RETURNED
   TRUE - success
   FALSE- fail
====================================================================== */
int
#if XVT_CC_PROTO
getWindowStartPosition (char *name, short *x, short *y, short *x2, short *y2,
                                    short *width, short *height)
#else
getWindowStartPosition (name, x, y, x2, y2, width, height)
char *name;
short *x, *y, *x2, *y2, *width, *height;
#endif
{
   int winNum;
   int screenWidth, screenHeight;
   
   for (winNum = 0; winNum < winPositionOptions.numWindows; winNum++)
   {
      if (strcmp (name, winPositionOptions.winPositions[winNum].name) == 0)
      {
         screenWidth = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_WIDTH);
         screenHeight = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_HEIGHT);

         if (x)
         {
            *x = winPositionOptions.winPositions[winNum].x;
            if (*x < 0) *x = 0;
            if (*x > screenWidth-10) *x = screenWidth-10;
         }
         if (y)
         {
            *y = winPositionOptions.winPositions[winNum].y;
            if (*y < 0) *y = 0;
            if (*y > screenHeight-10) *y = screenHeight-10;
         }
         if (x2)
         {
            *x2 = winPositionOptions.winPositions[winNum].x
                + winPositionOptions.winPositions[winNum].width;
            if (*x2 < 0) *x2 = 0;
            if (*x2 > screenWidth) *x2 = screenWidth;
         }
         if (y2)
         {
            *y2 = winPositionOptions.winPositions[winNum].y
                + winPositionOptions.winPositions[winNum].height;
            if (*y2 < 0) *y2 = 0;
            if (*y2 > screenHeight) *y2 = screenHeight;
         }
         if (width)
         {
            *width = winPositionOptions.winPositions[winNum].width;
            if (*width > screenWidth) *width = screenWidth;
         }
         if (height)
         {
            *height = winPositionOptions.winPositions[winNum].height;
            if (*height > screenHeight) *height = screenHeight;
         }
         
         return (TRUE);
      }
   }
   
   return (FALSE);
}

/* ======================================================================
FUNCTION        getPlaneStructure
DESCRIPTION
        pick out the structure stucture inside another structure
INPUT

OUTPUT

RETURNED
    pointer to the structure OR
    NULL = the struct is not in this object
====================================================================== */
PLANE *
#if XVT_CC_PROTO
getPlaneStructure (OBJECT *object)
#else
getPlaneStructure (object)
OBJECT *object;
#endif
{
   PLANE *options = NULL;

   if (object->shape == FAULT)
   {
      FAULT_OPTIONS *tempOptions;

      tempOptions = (FAULT_OPTIONS *) object->options;
      options = (PLANE *) &(tempOptions->plane);
   }
   else if (object->shape == UNCONFORMITY)
   {
      UNCONFORMITY_OPTIONS *tempOptions;

      tempOptions = (UNCONFORMITY_OPTIONS *) object->options;
      options = (PLANE *) &(tempOptions->plane);
   }

   return (options);
}
/* ======================================================================
FUNCTION        getRotationMatrices
DESCRIPTION
        pick out the rotatio Matricies stucture inside another structure
INPUT

OUTPUT

RETURNED
    pointer to the structure OR
    NULL = the struct is not in this object
====================================================================== */
ROTATION_MATRICES *
#if XVT_CC_PROTO
getRotationMatrices (OBJECT *object)
#else
getRotationMatrices (object)
OBJECT *object;
#endif
{
   ROTATION_MATRICES *options = NULL;

   if (object->shape == FOLD)
   {
      FOLD_OPTIONS *tempOptions;

      tempOptions = (FOLD_OPTIONS *) object->options;
      options = (ROTATION_MATRICES *) &(tempOptions->rotationMatrix);
   }
   else if (object->shape == FAULT)
   {
      FAULT_OPTIONS *tempOptions;

      tempOptions = (FAULT_OPTIONS *) object->options;
      options = (ROTATION_MATRICES *) &(tempOptions->rotationMatrix);
   }
   else if (object->shape == UNCONFORMITY)
   {
      UNCONFORMITY_OPTIONS *tempOptions;

      tempOptions = (UNCONFORMITY_OPTIONS *) object->options;
      options = (ROTATION_MATRICES *) &(tempOptions->rotationMatrix);
   }
   else if (object->shape == SHEAR_ZONE)
   {
      SHEAR_OPTIONS *tempOptions;

      tempOptions = (SHEAR_OPTIONS *) object->options;
      options = (ROTATION_MATRICES *) &(tempOptions->rotationMatrix);
   }
   else if (object->shape == PLUG)
   {
      PLUG_OPTIONS *tempOptions;

      tempOptions = (PLUG_OPTIONS *) object->options;
      options = (ROTATION_MATRICES *) &(tempOptions->rotationMatrix);
   }
   else if (object->shape == DYKE)
   {
      DYKE_OPTIONS *tempOptions;

      tempOptions = (DYKE_OPTIONS *) object->options;
      options = (ROTATION_MATRICES *) &(tempOptions->rotationMatrix);
   }
   else if (object->shape == FOLIATION)
   {
      FOLIATION_OPTIONS *tempOptions;

      tempOptions = (FOLIATION_OPTIONS *) object->options;
      options = (ROTATION_MATRICES *) &(tempOptions->rotationMatrix);
   }
   else if (object->shape == LINEATION)
   {
      LINEATION_OPTIONS *tempOptions;

      tempOptions = (LINEATION_OPTIONS *) object->options;
      options = (ROTATION_MATRICES *) &(tempOptions->rotationMatrix);
   }

   return (options);
}

/* ======================================================================
FUNCTION        getNoddyColourStructure
DESCRIPTION
        pick out the noddy color stucture inside another structure
INPUT

OUTPUT

RETURNED
    pointer to the structure OR
    NULL = the struct is not in this object
====================================================================== */
NODDY_COLOUR *
#if XVT_CC_PROTO
getNoddyColourStructure (OBJECT *object)
#else
getNoddyColourStructure (object)
OBJECT *object;
#endif
{
   NODDY_COLOUR *options = NULL;
   
   if (!object)
      return (options);
   
   if (object->shape == STRATIGRAPHY)
   {
      STRATIGRAPHY_OPTIONS *tempOptions;
      int layer;

      tempOptions = (STRATIGRAPHY_OPTIONS *) object->options;
         
      if (object->generalData > 0)
         layer = object->generalData - 1;
      else     /* -ve to go from other end (generalData = -1, -2 etc) */
         layer = tempOptions->numLayers + object->generalData;

      options = (NODDY_COLOUR *) &(tempOptions->properties[layer].color);
   }
   else if (object->shape == FAULT)
   {
      FAULT_OPTIONS *tempOptions;

      tempOptions = (FAULT_OPTIONS *) object->options;

      options = (NODDY_COLOUR *) &(tempOptions->color);
   }
   else if (object->shape == UNCONFORMITY)
   {
      UNCONFORMITY_OPTIONS *tempOptions;
      int layer;

      tempOptions = (UNCONFORMITY_OPTIONS *) object->options;

      if (object->generalData > 0)
         layer = object->generalData - 1;
      else     /* -ve to go from other end (generalData = -1, -2 etc) */
         layer = tempOptions->stratigraphyDetails.numLayers + object->generalData;

      options = (NODDY_COLOUR *)
           &(tempOptions->stratigraphyDetails.properties[layer].color);
   }
   else if (object->shape == PLUG)
   {
      PLUG_OPTIONS *tempOptions;

      tempOptions = (PLUG_OPTIONS *) object->options;

      options = (NODDY_COLOUR *) &(tempOptions->properties.color);
   }
   else if (object->shape == DYKE)
   {
      DYKE_OPTIONS *tempOptions;

      tempOptions = (DYKE_OPTIONS *) object->options;

      options = (NODDY_COLOUR *) &(tempOptions->properties.color);
   }
   else if (object->shape == IMPORT)
   {
      IMPORT_OPTIONS *tempOptions;
      int layer = object->generalData;

      tempOptions = (IMPORT_OPTIONS *) object->options;
      options = (NODDY_COLOUR *) &(tempOptions->properties[layer].color);
   }
   else if (object->shape == STOP)
   {
      ROCK_DATABASE *tempOptions;
      int layer = -(object->generalData + 1);

      tempOptions = (ROCK_DATABASE *) object->options;
      options = (NODDY_COLOUR *) &(tempOptions->database[layer].color);
   }

   return (options);
}

/* ======================================================================
FUNCTION        getLayerWidth
DESCRIPTION
        calculate the layer Width for a particular layer
INPUT

OUTPUT

RETURNED
    width of layer
    -1 on error or width unavailable
====================================================================== */
double
#if XVT_CC_PROTO
getLayerWidth (OBJECT *object)
#else
getLayerWidth (object)
OBJECT *object;
#endif
{
   LAYER_PROPERTIES *layer1 = NULL, *layer2 = NULL;
   double width = -1.0;
   
            /* special case of rockDatabase */
   if (object->shape == STRATIGRAPHY)
   {
      STRATIGRAPHY_OPTIONS *tempOptions;
      int layer = object->generalData - 1;
   
      tempOptions = (STRATIGRAPHY_OPTIONS *) object->options;
                          /* Only return what is within range */
      if ((layer < 0) || (layer >= tempOptions->numLayers))
         return (width);
   
      if (layer < 1)   /* Bottom Layer should be infinite */
         return (width);

      layer1 = (LAYER_PROPERTIES *) &(tempOptions->properties[layer]);

      layer++;
                          /* Only return what is within range */
      if ((layer < 0) || (layer >= tempOptions->numLayers))
         return (width);
   
      layer2 = (LAYER_PROPERTIES *) &(tempOptions->properties[layer]);

   }
   else if (object->shape == UNCONFORMITY)
   {
      UNCONFORMITY_OPTIONS *tempOptions;
      int layer = object->generalData - 1;
   
      tempOptions = (UNCONFORMITY_OPTIONS *) object->options;
                          /* Only return what is within range */
      if ((layer < 0) || (layer >= tempOptions->stratigraphyDetails.numLayers))
         return (width);
   
      if (layer < 1)   /* Bottom Layer should be infinite */
         return (width);
         
      layer1 = (LAYER_PROPERTIES *)
                &(tempOptions->stratigraphyDetails.properties[layer]);

      layer++;
                          /* Only return what is within range */
      if ((layer < 0) || (layer >= tempOptions->stratigraphyDetails.numLayers))
         return (width);
   
      layer2 = (LAYER_PROPERTIES *)
                &(tempOptions->stratigraphyDetails.properties[layer]);
   }
   
   if (layer2 && layer1)
      width = layer2->height - layer1->height;
   
   return (width);
}

/* ======================================================================
FUNCTION        getLayerPropertiesStructure
DESCRIPTION
        pick out the noddy color stucture inside another structure
INPUT

OUTPUT

RETURNED
    pointer to the structure OR
    NULL = the struct is not in this object
====================================================================== */
LAYER_PROPERTIES *
#if XVT_CC_PROTO
getLayerPropertiesStructure (OBJECT *object)
#else
getLayerPropertiesStructure (object)
OBJECT *object;
#endif
{
   LAYER_PROPERTIES *options = NULL;
   
            /* special case of rockDatabase */
   if ((object->generalData < 0) && (object->shape == STOP))
   {
      ROCK_DATABASE *rockDatabase;
      int layer = -(object->generalData + 1);
      
      rockDatabase = (ROCK_DATABASE *) object->options;
      
      if ((layer < 0) || (layer >= rockDatabase->numProps))
         return (options);
         
      options = &(rockDatabase->database[layer]);
   }
   else
   {
      if (object->shape == STRATIGRAPHY)
      {
         STRATIGRAPHY_OPTIONS *tempOptions;
         int layer;

         tempOptions = (STRATIGRAPHY_OPTIONS *) object->options;
         
         if (object->generalData > 0)
            layer = object->generalData - 1;
         else     /* -ve to go from other end (generalData = -1, -2 etc) */
            layer = tempOptions->numLayers + object->generalData;
   
                             /* Only return what is within range */
         if ((layer < 0) || (layer >= tempOptions->numLayers))
            return (options);
   
         options = (LAYER_PROPERTIES *) &(tempOptions->properties[layer]);
      }
      else if (object->shape == UNCONFORMITY)
      {
         UNCONFORMITY_OPTIONS *tempOptions;
         int layer;
   
         tempOptions = (UNCONFORMITY_OPTIONS *) object->options;

         if (object->generalData > 0)
            layer = object->generalData - 1;
         else     /* -ve to go from other end (generalData = -1, -2 etc) */
            layer = tempOptions->stratigraphyDetails.numLayers + object->generalData;

                             /* Only return what is within range */
         if ((layer < 0) || (layer >= tempOptions->stratigraphyDetails.numLayers))
            return (options);
   
         options = (LAYER_PROPERTIES *)
                   &(tempOptions->stratigraphyDetails.properties[layer]);
      }
      else if (object->shape == PLUG)
      {
         PLUG_OPTIONS *tempOptions;
   
         tempOptions = (PLUG_OPTIONS *) object->options;
   
         options = (LAYER_PROPERTIES *) &(tempOptions->properties);
      }
      else if (object->shape == DYKE)
      {
         DYKE_OPTIONS *tempOptions;
   
         tempOptions = (DYKE_OPTIONS *) object->options;
   
         options = (LAYER_PROPERTIES *) &(tempOptions->properties);
      }
      else if (object->shape == IMPORT)
      {
         IMPORT_OPTIONS *tempOptions;
         int layer = object->generalData;
   
         tempOptions = (IMPORT_OPTIONS *) object->options;
                             /* Only return what is within range */
         if ((layer < 0) || (layer >= tempOptions->numProperties))
            return (options);
   
         if (tempOptions->properties)
            options = (LAYER_PROPERTIES *) &(tempOptions->properties[layer]);
      }
   }
   
   return (options);
}

/* ======================================================================
FUNCTION        getStratigraphyOptionsStructure
DESCRIPTION
        pick out the noddy color stucture inside another structure
INPUT

OUTPUT

RETURNED
    pointer to the structure OR
    NULL = the struct is not in this object
====================================================================== */
STRATIGRAPHY_OPTIONS *
#if XVT_CC_PROTO
getStratigraphyOptionsStructure (OBJECT *object)
#else
getStratigraphyOptionsStructure (object)
OBJECT *object;
#endif
{
   STRATIGRAPHY_OPTIONS *options = NULL;

   if (object->shape == STRATIGRAPHY)
   {
      options = (STRATIGRAPHY_OPTIONS *) object->options;
   }
   else if (object->shape == UNCONFORMITY)
   {
      UNCONFORMITY_OPTIONS *tempOptions;

      tempOptions = (UNCONFORMITY_OPTIONS *) object->options;
      options = (STRATIGRAPHY_OPTIONS *) &(tempOptions->stratigraphyDetails);
   }

   return (options);
}


/* ======================================================================
FUNCTION        getDxfSurfaceStructure
DESCRIPTION
        pick out the dxf surface stucture inside another structure
INPUT

OUTPUT

RETURNED
    pointer to the structure OR
    NULL = the struct is not in this object
====================================================================== */
DXFHANDLE *
#if XVT_CC_PROTO
getDxfSurfaceStructure (OBJECT *object)
#else
getDxfSurfaceStructure (object)
OBJECT *object;
#endif
{
   DXFHANDLE *dxfData = NULL;
   
   switch (object->shape)
   {
      case (UNCONFORMITY):
         {
            UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;
            dxfData = &(options->dxfData);
         }
         break;
      case (FAULT):
         {
            FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;
            dxfData = &(options->dxfData);
         }
         break;
      case (SHEAR_ZONE):
         {
            SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;
            dxfData = &(options->dxfData);
         }
         break;
   }
   
   return (dxfData);
}

/* ======================================================================
FUNCTION        getDxfSurfaceFilename
DESCRIPTION
        pick out the dxf surface filename inside another structure
INPUT

OUTPUT

RETURNED
    pointer to the structure OR
    NULL = the struct is not in this object
====================================================================== */
FILE_SPEC *
#if XVT_CC_PROTO
getDxfSurfaceFilename (OBJECT *object)
#else
getDxfSurfaceFilename (object)
OBJECT *object;
#endif
{
   FILE_SPEC *filename = NULL;
   
   switch (object->shape)
   {
      case (UNCONFORMITY):
         {
            UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;
            filename = &(options->surfaceFilename);
         }
         break;
      case (FAULT):
         {
            FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;
            filename = &(options->surfaceFilename);
         }
         break;
      case (SHEAR_ZONE):
         {
            SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;
            filename = &(options->surfaceFilename);
         }
         break;
   }
   
   return (filename);
}

/* ======================================================================
FUNCTION        getProfileOptionsStructure
DESCRIPTION
        pick out the noddy profile options stucture inside another structure
INPUT

OUTPUT

RETURNED
    pointer to the structure OR
    NULL = the struct is not in this object
====================================================================== */
PROFILE_OPTIONS *
#if XVT_CC_PROTO
getProfileOptionsStructure (OBJECT *object)
#else
getProfileOptionsStructure (object)
OBJECT *object;
#endif
{
   PROFILE_OPTIONS *options = NULL;
   
   switch (object->shape)
   {
      case (UNCONFORMITY):
         {
            UNCONFORMITY_OPTIONS *unconOptions;
            unconOptions = (UNCONFORMITY_OPTIONS *) object->options;
            if (object->generalData == ALTERATION_PROFILE)
               options = unconOptions->alterationFuntions;
         }
         break;
      case (FOLD):
         {
            FOLD_OPTIONS *foldOptions;
            foldOptions = (FOLD_OPTIONS *) object->options;
            if (object->generalData == STANDARD_PROFILE)
               options = &(foldOptions->profile);
         }
         break;
      case (FAULT):
         {
            FAULT_OPTIONS *faultOptions;
            faultOptions = (FAULT_OPTIONS *) object->options;
            if (object->generalData == ALTERATION_PROFILE)
               options = faultOptions->alterationFuntions;
            else
               options = &(faultOptions->profile);
         }
         break;
      case (SHEAR_ZONE):
         {
            SHEAR_OPTIONS *shearOptions;
            shearOptions = (SHEAR_OPTIONS *) object->options;
            if (object->generalData == ALTERATION_PROFILE)
               options = shearOptions->alterationFuntions;
            else
               options = &(shearOptions->profile);
         }
         break;
      case (DYKE):
         {
            DYKE_OPTIONS *dykeOptions;
            dykeOptions = (DYKE_OPTIONS *) object->options;
            if (object->generalData == ALTERATION_PROFILE)
               options = dykeOptions->alterationFuntions;
         }
         break;
      case (PLUG):
         {
            PLUG_OPTIONS *plugOptions;
            plugOptions = (PLUG_OPTIONS *) object->options;
            if (object->generalData == ALTERATION_PROFILE)
               options = plugOptions->alterationFuntions;
         }
         break;
   }
   
   return (options);
}


/* ======================================================================
FUNCTION        assignPropertiesForStratLayers
DESCRIPTION
        copy into the array passed in the pointers to the properties
        structure that co-inside with the layer number in a block model
        NOTE this array is filled from 0 while the first block model layer
             number is one, so use layerNum-1 as a reference into this array
        ie 0 - first layer of base Strat
             - next the layers of any unconformities
             - next any dykes or plugs in the order they appear
INPUT/OUTPUT
        LAYER_PROPERTIES *properties - array of at least "arraySize" elememts
                                       to hold pointers to all the sructures
        int arraySize;

RETURNED
    FALSE - error (mabe there is no history)
    TRUE - sucess
====================================================================== */
int
#if XVT_CC_PROTO
assignPropertiesForStratLayers (LAYER_PROPERTIES **properties, int arraySize)
#else
assignPropertiesForStratLayers (properties, arraySize)
LAYER_PROPERTIES **properties;
int arraySize;
#endif
{
   OBJECT *object;
   STRATIGRAPHY_OPTIONS *stratOptions;
   IMPORT_OPTIONS *importOptions;
   int numEvents, totalLayers = 0, event, layer;
  
                            /* ********************************************* */
                            /* make up an array off the colors of the layers */
   for (event = 0, numEvents = (int) countObjects (NULL_WIN);
                                                  event < numEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
      if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                            getStratigraphyOptionsStructure (object);
         for (layer = stratOptions->numLayers-1; layer >= 0; layer--)
         {
            properties[totalLayers] = &(stratOptions->properties[layer]);
            totalLayers++;
                         /* make sure we dont overflow array */
            if (totalLayers >= arraySize)
               return (totalLayers);
         }

      }
      if (object->shape == IMPORT)
      {
         importOptions = (IMPORT_OPTIONS *) object->options;
         for (layer = 0; layer < importOptions->numProperties; layer++)
         {
            properties[totalLayers] = &(importOptions->properties[layer]);
            totalLayers++;
                         /* make sure we dont overflow array */
            if (totalLayers >= arraySize)
               return (totalLayers);
         }
      }      
   }
   for (event = 0; event < numEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
      if ((object->shape == DYKE) || (object->shape == PLUG))
      {
         properties[totalLayers] = getLayerPropertiesStructure (object);
         totalLayers++;
                      /* make sure we dont overflow array */
         if (totalLayers >= arraySize)
            return (totalLayers);
      }
   }

   return (totalLayers);
}

/* ======================================================================
FUNCTION        assignEventNumForStratLayers
DESCRIPTION
        copy into the array passed in the event numbers
        that co-inside with the layer number in a block model
        NOTE this array is filled from 0 while the first block model layer
             number is one, so use layerNum-1 as a reference into this arary
        ie 0 - first layer of base Strat
             - next the layers of any un conformities
             - next any dykes or plugs in the order they appear
INPUT/OUTPUT
        int *eventNumbers - array of at least "arraySize" elememts
                                       to hold the event Numbers
        int arraySize;

RETURNED
    FALSE - error (mabe there is no history)
    TRUE - sucess
====================================================================== */
int
#if XVT_CC_PROTO
assignEventNumForStratLayers (int *eventNumbers, int arraySize)
#else
assignEventNumForStratLayers (eventNumbers, arraySize)
int *eventNumbers;
int arraySize;
#endif
{
   OBJECT *object;
   STRATIGRAPHY_OPTIONS *stratOptions;
   IMPORT_OPTIONS *importOptions;
   int numEvents, totalLayers = 0, event, layer;
  
                            /* ********************************************* */
                            /* make up an array off the colors of the layers */
   for (event = 0, numEvents = (int) countObjects (NULL_WIN);
                                                  event < numEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
      if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                            getStratigraphyOptionsStructure (object);
         for (layer = 0; layer < stratOptions->numLayers; layer++)
         {
            eventNumbers[totalLayers] = event;
            totalLayers++;
                         /* make sure we dont overflow array */
            if (totalLayers >= arraySize)
               return (totalLayers);
         }

      }
      if (object->shape == IMPORT)
      {
         importOptions = (IMPORT_OPTIONS *) object->options;
         for (layer = 0; layer < importOptions->numProperties; layer++)
         {
            eventNumbers[totalLayers] = event;
            totalLayers++;
                         /* make sure we dont overflow array */
            if (totalLayers >= arraySize)
               return (totalLayers);
         }
      }      
   }
   for (event = 0; event < numEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
      if ((object->shape == DYKE) || (object->shape == PLUG))
      {
         eventNumbers[totalLayers] = event;
         totalLayers++;
                      /* make sure we dont overflow array */
         if (totalLayers >= arraySize)
            return (totalLayers);
      }
   }

   return (totalLayers);
}

/* ======================================================================
FUNCTION        assignFlavorForStratLayers
DESCRIPTION
        copy into the array passed in the rock flavor
        that co-inside with the layer number in a block model
        NOTE this array is filled from 0 while the first block model layer
             number is one, so use layerNum-1 as a reference into this arary
        ie 0 - first layer of base Strat (BASE_STRAT)
             - next the layers of any un conformities (UNC_STRAT)
             - next any dykes or plugs in the order they appear  (IGNEOUS_STRAT)

INPUT/OUTPUT
        int *flavors - array of at least "arraySize" elememts
                                       to hold the flavors of the rock
        int arraySize;

RETURNED
    FALSE - error (mabe there is no history)
    TRUE - sucess
====================================================================== */
int
#if XVT_CC_PROTO
assignFlavorForStratLayers (unsigned int *flavors, int arraySize)
#else
assignFlavorForStratLayers (flavors, arraySize)
unsigned int *flavors;
int arraySize;
#endif
{
   OBJECT *object;
   STRATIGRAPHY_OPTIONS *stratOptions;
   IMPORT_OPTIONS *importOptions;
   int numEvents, totalLayers = 0, event, layer;
  
                            /* ********************************************* */
                            /* make up an array off the colors of the layers */
   for (event = 0, numEvents = (int) countObjects (NULL_WIN);
                                                  event < numEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
      if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                            getStratigraphyOptionsStructure (object);
         for (layer = 0; layer < stratOptions->numLayers; layer++)
         {
            if (object->shape == STRATIGRAPHY)
               flavors[totalLayers] = BASE_STRAT;
            else if (object->shape == STRATIGRAPHY)
               flavors[totalLayers] = UNC_STRAT;
            totalLayers++;
                         /* make sure we dont overflow array */
            if (totalLayers >= arraySize)
               return (totalLayers);
         }

      }
      if (object->shape == IMPORT)
      {
         importOptions = (IMPORT_OPTIONS *) object->options;
         for (layer = 0; layer < importOptions->numProperties; layer++)
         {
            flavors[totalLayers] = BASE_STRAT;
            totalLayers++;
                         /* make sure we dont overflow array */
            if (totalLayers >= arraySize)
               return (totalLayers);
         }
      }      
   }
   for (event = 0; event < numEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
      if ((object->shape == DYKE) || (object->shape == PLUG))
      {
         flavors[totalLayers] = IGNEOUS_STRAT;
         totalLayers++;
                      /* make sure we dont overflow array */
         if (totalLayers >= arraySize)
            return (totalLayers);
      }
   }

   return (totalLayers);
}


/* ======================================================================
FUNCTION        checkHistoryForAlterations
DESCRIPTION
        Checks to see if any alterations are present in the history 

INPUT/OUTPUT

RETURNED
    FALSE - No alterations present
    TRUE - Alterations need to be considered in further calculations
====================================================================== */
int checkHistoryForAlterations ()
{
   int altZones = FALSE;
   OBJECT *objectPtr;
   
   for (objectPtr = nthObject(NULL_WIN, 0); 
                     (objectPtr && (objectPtr->shape != STOP));
                                    objectPtr = objectPtr->next)
   {
      switch (objectPtr->shape)
      {
         case (FAULT):
            {
               FAULT_OPTIONS *options = (FAULT_OPTIONS *) objectPtr->options;
               if (options->alterationZones)
                  altZones = TRUE;
            }
            break;
         case (SHEAR_ZONE):
            {
               SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) objectPtr->options;
               if (options->alterationZones)
                  altZones = TRUE;
            }
            break;
         case (UNCONFORMITY):
            {
               UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) objectPtr->options;
               if (options->alterationZones)
                  altZones = TRUE;
            }
            break;
         case (PLUG):
            {
               PLUG_OPTIONS *options = (PLUG_OPTIONS *) objectPtr->options;
               if (options->alterationZones)
                  altZones = TRUE;
            }
            break;
         case (DYKE):
            {
               DYKE_OPTIONS *options = (DYKE_OPTIONS *) objectPtr->options;
               if (options->alterationZones)
                  altZones = TRUE;
            }
            break;
      }
   }
   
   return (altZones);
}

/* ======================================================================
FUNCTION        setDefaultOptions
DESCRIPTION
        set up the options structure to the default values
        This is called to initially setup the options structures
        with there default values.
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
setDefaultOptions (OBJECT *object)
#else
setDefaultOptions (object)
OBJECT *object;
#endif
{

   switch (object->shape)
   {
      case STRATIGRAPHY:
         {
            STRATIGRAPHY_OPTIONS *options = (STRATIGRAPHY_OPTIONS *) object->options;
            int i, numLayers = 7;
            unsigned char red, green, blue;

            if (!(options->properties = (LAYER_PROPERTIES *) xvt_mem_zalloc (numLayers*sizeof(LAYER_PROPERTIES))))
            {
               options->numLayers = 0;
               xvt_dm_post_error ("Error, Not enough memory to correctly create Stratigraphy.");
               break;
            }
            options->numLayers = numLayers;

            i = 0;
            strcpy (options->properties[i].unitName, "Base");
            options->properties[i].height = -31000;
            options->properties[i].density = 4.0;
            options->properties[i].sus_X =
               options->properties[i].sus_Y =
               options->properties[i].sus_Z =
               options->properties[i].strength = 1.6e-03;
            getColorRGB ("Color 92", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties[i].color),
                          "Color 92", (int) red, (int) green, (int) blue);

            i = 1;
            strcpy (options->properties[i].unitName, "Lower");
            options->properties[i].height = 1000;
            options->properties[i].density = 3.0;
            options->properties[i].sus_X =
               options->properties[i].sus_Y =
               options->properties[i].sus_Z =
               options->properties[i].strength = 1.5e-03;
            getColorRGB ("Color 141", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties[i].color),
                           "Color 141", (int) red, (int) green, (int) blue);

            i = 2;
            strcpy (options->properties[i].unitName, "L Middle");
            options->properties[i].height = 2000;
            options->properties[i].density = 2.8;
            options->properties[i].sus_X =
               options->properties[i].sus_Y =
               options->properties[i].sus_Z =
               options->properties[i].strength = 1.4e-03;
            getColorRGB ("Color 130", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties[i].color),
                           "Color 130", (int) red, (int) green, (int) blue);

            i = 3;
            strcpy (options->properties[i].unitName, "Middle");
            options->properties[i].height = 3000;
            options->properties[i].density = 2.6;
            options->properties[i].sus_X =
               options->properties[i].sus_Y =
               options->properties[i].sus_Z =
               options->properties[i].strength = 1.3e-03;
            getColorRGB ("Color 42", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties[i].color),
                            "Color 42", (int) red, (int) green, (int) blue);

            i = 4;
            strcpy (options->properties[i].unitName, "U Middle");
            options->properties[i].height = 3500;
            options->properties[i].density = 2.4;
            options->properties[i].sus_X =
               options->properties[i].sus_Y =
               options->properties[i].sus_Z =
               options->properties[i].strength = 1.2e-03;
            getColorRGB ("Color 70", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties[i].color),
                            "Color 70", (int) red, (int) green, (int) blue);

            i = 5;
            strcpy (options->properties[i].unitName, "Upper");
            options->properties[i].height = 4500;
            options->properties[i].density = 2.2;
            options->properties[i].sus_X =
               options->properties[i].sus_Y =
               options->properties[i].sus_Z =
               options->properties[i].strength = 1.1e-03;
            getColorRGB ("Color 75", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties[i].color),
                            "Color 75", (int) red, (int) green, (int) blue);

            i = 6;
            strcpy (options->properties[i].unitName, "Top");
            options->properties[i].height = 5500;
            options->properties[i].density = 2.0;
            options->properties[i].sus_X =
               options->properties[i].sus_Y =
               options->properties[i].sus_Z =
               options->properties[i].strength = 1.0e-03;
            getColorRGB ("Color 66", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties[i].color),
                           "Color 66", (int) red, (int) green, (int) blue);

                   /* these properties are the same in each layer */
            for (i = 0; i < 7; i++)
            {
               options->properties[i].applyAlterations = TRUE;
               options->properties[i].anisotropicField = FALSE;
               options->properties[i].remanentMagnetization = FALSE;
               options->properties[i].inclination = 30.0;
               options->properties[i].angleWithNorth = 30.0;
               options->properties[i].sus_dip = 90.0;
               options->properties[i].sus_dipDirection = 90.0;
               options->properties[i].sus_pitch = 0.0;
            }
         }
         break;
      case FOLD:
         {
            FOLD_OPTIONS *options = (FOLD_OPTIONS *) object->options;

            options->type = 0;
            options->singleFold = FALSE;
            options->positionX = 0.0;
            options->positionY = 0.0;
            options->positionZ = 0.0;
            options->dipDirection = 90.0;
            options->dip = 90.0;
            options->axisPitch = 0.0;
            options->wavelength = 4000.0;
            options->amplitude = 1000.0;
            options->cycle = 0.0;

                   /* setup The profiles this event has */
            initAllProfiles (object);
         }
         break;
      case FAULT:
         {
            FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;
            unsigned char red, green, blue;

            options->type = FAULT_EVENT;
            options->geometry = TRANSLATION;
            options->movement = HANGING_WALL;
            options->positionX = 0.0;
            options->positionY = 0.0;
            options->positionZ = 5000.0;
            options->dipDirection = 135.0;
            options->dip = 45.0;
            options->pitch = 90.0;
            options->rotation = 30.0;
            options->slip = 2000.0;
            options->amplitude = 2000.0;
            options->width = 2000.0;
            options->radius = 1000.0;
            options->xAxis = 2000.0;
            options->yAxis = 2000.0;
            options->zAxis = 2000.0;
            options->cylindricalIndex = 0.0;
            options->profilePitch = 90.0;

            getColorRGB ("Color 36", &red, &green, &blue);
            assignNoddyColourStructure(&(options->color),
                             "Color 36", (int) red, (int) green, (int) blue);

                   /* setup The profiles this event has */
            initAllProfiles (object);
         }
         break;
      case UNCONFORMITY:
         {
            UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;
            int i, numLayers = 7;
            unsigned char red, green, blue;

            options->positionX = 0.0;
            options->positionY = 0.0;
            options->positionZ = 5000.0;
            options->dipDirection = 90.0;
            options->dip = 45.0;

                   /* setup The profiles or Alterations this event has */
            initAllProfiles (object);

            if (!(options->stratigraphyDetails.properties = (LAYER_PROPERTIES *) xvt_mem_zalloc (numLayers*sizeof(LAYER_PROPERTIES))))
            {
               options->stratigraphyDetails.numLayers = 0;
               xvt_dm_post_error ("Error, Not enough memory to correctly create Stratigraphy.");
               break;
            }
            options->stratigraphyDetails.numLayers = numLayers;

            i = 0;
            strcpy(options->stratigraphyDetails.properties[i].unitName,"Base");
            options->stratigraphyDetails.properties[i].height = -31000;
            options->stratigraphyDetails.properties[i].density = 4.0;
            options->stratigraphyDetails.properties[i].sus_X =
               options->stratigraphyDetails.properties[i].sus_Y =
               options->stratigraphyDetails.properties[i].sus_Z =
               options->stratigraphyDetails.properties[i].strength = 1.6e-03;
            getColorRGB ("Color 189", &red, &green, &blue);
            assignNoddyColourStructure(
                     &(options->stratigraphyDetails.properties[i].color),
                     "Color 189", (int) red, (int) green, (int) blue);

            i = 1;
            strcpy(options->stratigraphyDetails.properties[i].unitName,"Lower");
            options->stratigraphyDetails.properties[i].height = 1000;
            options->stratigraphyDetails.properties[i].density = 3.0;
            options->stratigraphyDetails.properties[i].sus_X =
               options->stratigraphyDetails.properties[i].sus_Y =
               options->stratigraphyDetails.properties[i].sus_Z =
               options->stratigraphyDetails.properties[i].strength = 1.5e-03;
            getColorRGB ("Color 135", &red, &green, &blue);
            assignNoddyColourStructure(
                     &(options->stratigraphyDetails.properties[i].color),
                     "Color 135", (int) red, (int) green, (int) blue);

            i = 2;
            strcpy(options->stratigraphyDetails.properties[i].unitName,"L Middle");
            options->stratigraphyDetails.properties[i].height = 2000;
            options->stratigraphyDetails.properties[i].density = 2.8;
            options->stratigraphyDetails.properties[i].sus_X =
               options->stratigraphyDetails.properties[i].sus_Y =
               options->stratigraphyDetails.properties[i].sus_Z =
               options->stratigraphyDetails.properties[i].strength = 1.4e-03;
            getColorRGB ("Color 140", &red, &green, &blue);
            assignNoddyColourStructure(
                     &(options->stratigraphyDetails.properties[i].color),
                     "Color 140", (int) red, (int) green, (int) blue);

            i = 3;
            strcpy(options->stratigraphyDetails.properties[i].unitName,"Middle");
            options->stratigraphyDetails.properties[i].height = 3000;
            options->stratigraphyDetails.properties[i].density = 2.6;
            options->stratigraphyDetails.properties[i].sus_X =
               options->stratigraphyDetails.properties[i].sus_Y =
               options->stratigraphyDetails.properties[i].sus_Z =
               options->stratigraphyDetails.properties[i].strength = 1.3e-03;
            getColorRGB ("Color 106", &red, &green, &blue);
            assignNoddyColourStructure(
                     &(options->stratigraphyDetails.properties[i].color),
                     "Color 106", (int) red, (int) green, (int) blue);

            i = 4;
            strcpy(options->stratigraphyDetails.properties[i].unitName,"U Middle");
            options->stratigraphyDetails.properties[i].height = 3500;
            options->stratigraphyDetails.properties[i].density = 2.4;
            options->stratigraphyDetails.properties[i].sus_X =
               options->stratigraphyDetails.properties[i].sus_Y =
               options->stratigraphyDetails.properties[i].sus_Z =
               options->stratigraphyDetails.properties[i].strength = 1.2e-03;
            getColorRGB ("Color 38", &red, &green, &blue);
            assignNoddyColourStructure(
                     &(options->stratigraphyDetails.properties[i].color),
                     "Color 38", (int) red, (int) green, (int) blue);

            i = 5;
            strcpy(options->stratigraphyDetails.properties[i].unitName,"Upper");
            options->stratigraphyDetails.properties[i].height = 4500;
            options->stratigraphyDetails.properties[i].density = 2.2;
            options->stratigraphyDetails.properties[i].sus_X =
               options->stratigraphyDetails.properties[i].sus_Y =
               options->stratigraphyDetails.properties[i].sus_Z =
               options->stratigraphyDetails.properties[i].strength = 1.1e-03;
            getColorRGB ("Color 102", &red, &green, &blue);
            assignNoddyColourStructure(
                     &(options->stratigraphyDetails.properties[i].color),
                     "Color 102", (int) red, (int) green, (int) blue);

            i = 6;
            strcpy(options->stratigraphyDetails.properties[i].unitName,"Top");
            options->stratigraphyDetails.properties[i].height = 5500;
            options->stratigraphyDetails.properties[i].density = 2.0;
            options->stratigraphyDetails.properties[i].sus_X =
               options->stratigraphyDetails.properties[i].sus_Y =
               options->stratigraphyDetails.properties[i].sus_Z =
               options->stratigraphyDetails.properties[i].strength = 1.0e-03;
            getColorRGB ("Color 89", &red, &green, &blue);
            assignNoddyColourStructure(
                     &(options->stratigraphyDetails.properties[i].color),
                     "Color 89", (int) red, (int) green, (int) blue);

                   /* these properties are the same in each layer */
            for (i = 0; i < 7; i++)
            {
               options->stratigraphyDetails.properties[i].applyAlterations = TRUE;
               options->stratigraphyDetails.properties[i].anisotropicField = FALSE;
               options->stratigraphyDetails.properties[i].remanentMagnetization= FALSE;
               options->stratigraphyDetails.properties[i].inclination = 30.0;
               options->stratigraphyDetails.properties[i].angleWithNorth = 30.0;
               options->stratigraphyDetails.properties[i].sus_dip = 90.0;
               options->stratigraphyDetails.properties[i].sus_dipDirection = 90.0;
               options->stratigraphyDetails.properties[i].sus_pitch = 0.0;
            }
         }
         break;
      case SHEAR_ZONE:
         {
            SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;
            unsigned char red, green, blue;

            options->type = SHEAR_ZONE_EVENT;
            options->geometry = TRANSLATION;
            options->movement = HANGING_WALL;
            options->positionX = 0.0;
            options->positionY = 0.0;
            options->positionZ = 5000.0;
            options->dipDirection = 135.0;
            options->dip = 45.0;
            options->pitch = 90.0;
            options->rotation = 30.0;
            options->slip = 2000.0;
            options->amplitude = 2000.0;
            options->width = 2000.0;
            options->radius = 1000.0;
            options->xAxis = 2000.0;
            options->yAxis = 2000.0;
            options->zAxis = 2000.0;
            options->cylindricalIndex = 0.0;
            options->profilePitch = 90.0;

            getColorRGB ("Color 36", &red, &green, &blue);
            assignNoddyColourStructure(&(options->color),
                         "Color 36", (int) red, (int) green, (int) blue);

                   /* setup The profiles or Alterations this event has */
            initAllProfiles (object);
         }
         break;
      case DYKE:
         {
            DYKE_OPTIONS *options = (DYKE_OPTIONS *) object->options;
            unsigned char red, green, blue;

            options->type = DILATION_DYKE;
            options->mergeEvents = FALSE;
            options->positionX = 0.0;
            options->positionY = 0.0;
            options->positionZ = 5000.0;
            options->dipDirection = 110.0;
            options->dip = 45.0;
            options->slipPitch = 90.0;
            options->slipLength = 0.0;
            options->width = 500.0;

                   /* setup The profiles or Alterations this event has */
            initAllProfiles (object);

            strcpy (options->properties.unitName, "Basalt");
            options->properties.height = 5500;
            options->properties.density = 5.0;
            options->properties.sus_X =
               options->properties.sus_Y =
               options->properties.sus_Z =
               options->properties.strength = 1.0e-02;
            options->properties.sus_dip = 90.0;
            options->properties.sus_dipDirection = 90.0;
            options->properties.sus_pitch = 0.0;
            options->properties.applyAlterations = TRUE;
            options->properties.anisotropicField = FALSE;
            options->properties.remanentMagnetization = FALSE;
            options->properties.inclination = 30.0;
            options->properties.angleWithNorth = 30.0;
            getColorRGB ("Color 35", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties.color),
                     "Color 35", (int) red, (int) green, (int) blue);
         }
         break;
      case PLUG:
         {
            PLUG_OPTIONS *options = (PLUG_OPTIONS *) object->options;
            unsigned char red, green, blue;

            options->type = 0;
            options->mergeEvents = FALSE;
            options->positionX = 0.0;
            options->positionY = 0.0;
            options->positionZ = 5000.0;
            options->dipDirection = 90.0;
            options->dip = 90.0;
            options->axisPitch = 0.0;
            options->radius = 2000.0;
            options->apicalAngle = 30.0;
            options->BValue = 2000.0;
            options->axisA = 1000.0;
            options->axisB = 1000.0;
            options->axisC = 1000.0;

                   /* setup The profiles or Alterations this event has */
            initAllProfiles (object);

            strcpy (options->properties.unitName, "Gabbro");
            options->properties.height = 5500;
            options->properties.density = 5.0;
            options->properties.sus_X =
               options->properties.sus_Y =
               options->properties.sus_Z =
               options->properties.strength = 1.0e-02;
            options->properties.sus_dip = 90.0;
            options->properties.sus_dipDirection = 90.0;
            options->properties.sus_pitch = 0.0;
            options->properties.applyAlterations = TRUE;
            options->properties.anisotropicField = FALSE;
            options->properties.remanentMagnetization = FALSE;
            options->properties.inclination = 30.0;
            options->properties.angleWithNorth = 30.0;
            getColorRGB ("Color 28", &red, &green, &blue);
            assignNoddyColourStructure(&(options->properties.color),
                     "Color 28", (int) red, (int) green, (int) blue);
         }
         break;
      case STRAIN:
         {
            STRAIN_OPTIONS *options = (STRAIN_OPTIONS *) object->options;

            options->tensor[0][0] = options->tensor[1][1] =
                                    options->tensor[2][2] = 1.0;

            options->tensor[1][0] = options->tensor[2][1] =
            options->tensor[0][1] = options->tensor[2][1] =
            options->tensor[0][2] = options->tensor[1][2] = 0.0;
         }
         break;
      case TILT:
         {
            TILT_OPTIONS *options = (TILT_OPTIONS *) object->options;

            options->positionX = 0.0;
            options->positionY = 0.0;
            options->positionZ = 0.0;
            options->rotation = 30.0;
            options->plungeDirection = 45.0;
            options->plunge = 45.0;
         }
         break;
      case FOLIATION:
         {
            FOLIATION_OPTIONS *options = (FOLIATION_OPTIONS *) object->options;

            options->dipDirection = 45.0;
            options->dip = 45.0;
         }
         break;
      case LINEATION:
         {
            LINEATION_OPTIONS *options = (LINEATION_OPTIONS *) object->options;

            options->plungeDirection = 45.0;
            options->plunge = 45.0;
         }
         break;
      case IMPORT:
         {
            IMPORT_OPTIONS *options = (IMPORT_OPTIONS *) object->options;
            
            options->positionBy = POS_BY_CORNER;
            options->positionX = 0.0;
            options->positionY = 0.0;
            options->positionZ = 5000.0;
            
            options->blockSize = 1.0;
            options->fileType = NODDY_BLOCK_MODEL;
            options->properties = (LAYER_PROPERTIES *) NULL;
            options->numProperties = 0;
            options->blockData = NULL;
            options->numLayers = 0;
            options->layerDimensions = NULL;
            options->transparentLayer = 0;
            
            strcpy (options->importFile.name, "");            
            xvt_fsys_get_default_dir(&(options->importFile.dir));
#if (XVTWS == MACWS)
            strcpy (options->importFile.type, "TEXT");
#else
            strcpy (options->importFile.type, "g00");
#endif
         }
         break;
      case GENERIC:
         {
            GENERIC_OPTIONS *options = (GENERIC_OPTIONS *) object->options;
            
            options->param1 = 0.0;
            options->param2 = 0.0;
            options->param3 = 0.0;
            options->param4 = 0.0;
            options->param5 = 0.0;
            options->param6 = 0.0;
            options->param7 = 0.0;
            options->param8 = 0.0;
            options->param9 = 0.0;
            options->param10 = 0.0;
         }
         break;
   }
}


/* ======================================================================
FUNCTION        initSineFourierCoeff
DESCRIPTION
        initialise the coefficients for a fourier fold to that of
        a simple sine wave.
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
initSineFourierCoeff (double fourierCoeff[2][11])
#else
initSineFourierCoeff (fourierCoeff)
double fourierCoeff[2][11];
#endif
{
   int i;

   for (i = 0; i < 11; i++)
   {
      fourierCoeff[0][i] = 0.0;
      fourierCoeff[1][i] = 0.0;
   }
   fourierCoeff[1][1] = 1.0;
}


/* ======================================================================
FUNCTION        initGraphPoints
DESCRIPTION
        initialise the coefficients for a fourier fold to that of
        a simple sine wave.
INPUT

OUTPUT

RETURNED
====================================================================== */
PROFILE_POINT *
#if XVT_CC_PROTO
initGraphPoints (PROFILE_POINT *point, int numPoints,
                 int pattern, PROFILE_OPTIONS *profOptions)
#else
initGraphPoints (point, numPoints, pattern, profOptions)
PROFILE_POINT *point;
int numPoints, pattern;
PROFILE_OPTIONS *profOptions;
#endif
{
   PROFILE_POINT *tempPoint, *firstPoint;
   double ratio;
   int x, y, i;

            /* Free the list we have first */
   while (point)
   {
      tempPoint = point->next;
      xvt_mem_free ((char *) point);
      point = tempPoint;
   }
   
            /* Allocate the new points */
   tempPoint = NULL;
   for (i = 0; i < numPoints; i++)
   {
      point = (PROFILE_POINT *) xvt_mem_zalloc (sizeof(PROFILE_POINT));
      if (tempPoint)
         tempPoint->next = point;
      x = (int) (i * ((double) X_PROFILE_RESOLUTION
                                   / (double) (numPoints-1)));
      switch (pattern)
      {
         case (STRAIGHT_PROFILE):
            if (profOptions && (profOptions->joinType == LINE_SEGMENTS))
            {                /* make points at zero Level */
               ratio = ((double) (profOptions->scaleOrigin-profOptions->minYScale))
                            / (double) (profOptions->maxYScale - profOptions->minYScale);
               y = (int) (-Y_PROFILE_RESOLUTION
                            + floor (ratio*Y_PROFILE_RESOLUTION*2.0 + 0.5));
            }
            else
               y = 0;   /* keep on the middle axis */
            break;
         case (SINE_PROFILE):
            {
               double xRad = ((double)x/X_PROFILE_RESOLUTION)*3.14159*2.0;
               y = (int) floor ((double) sin(xRad)*Y_PROFILE_RESOLUTION);
            }
            break;
      }
      point->point.h = (short) x;
      point->point.v = (short) y;
      
      if (!tempPoint)
         firstPoint = point;

      tempPoint = point;
   }
   
   return (firstPoint);
}

/* ======================================================================
FUNCTION        initAlterationZones
DESCRIPTION
        initialise the profiles for defining alteration zones
INPUT

OUTPUT

RETURNED
====================================================================== */
PROFILE_OPTIONS *
#if XVT_CC_PROTO
initAlterationZones (OBJECTS type)
#else
initAlterationZones (type)
OBJECTS type;
#endif
{
   PROFILE_OPTIONS *firstOption = NULL,
                   *lastOption = NULL,
                   *profOption = NULL;
   int param;
   
   switch (type)
   {
      case (FAULT):
      case (SHEAR_ZONE):
      case (UNCONFORMITY):
      case (PLUG):
      case (DYKE):
      default:
         for (param = 0; param < TOTAL_ALTERATIONS; param++)
         {
            if (!(profOption = (PROFILE_OPTIONS *) xvt_mem_zalloc (sizeof(PROFILE_OPTIONS))))
               break;
               
            switch (param)
            {
               case (0):
                  strcpy (profOption->name, ALTERATION_PARAM_0);
                  profOption->type = DENSITY_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = 0.0;
                  profOption->maxYScale = 4.0;
                  profOption->scaleOrigin = 1.0;
                  profOption->minYReplace = 0.0;
                  profOption->maxYReplace = 10.0;
                  break;
               case (1):
                  strcpy (profOption->name, ALTERATION_PARAM_1);
                  profOption->type = ANI_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -10.0;
                  profOption->maxYScale =  10.0;
                  profOption->scaleOrigin = 0.0;
                  profOption->minYReplace = -10.0;
                  profOption->maxYReplace =  10.0;
                  break;
               case (2):
                  strcpy (profOption->name, ALTERATION_PARAM_2);
                  profOption->type = SUS_X_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -5.0;
                  profOption->maxYScale =  5.0;
                  profOption->scaleOrigin = 0.0;
                  profOption->minYReplace = 2.0;
                  profOption->maxYReplace = 8.0;
                  break;
               case (3):
                  strcpy (profOption->name, ALTERATION_PARAM_3);
                  profOption->type = SUS_Y_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -5.0;
                  profOption->maxYScale =  5.0;
                  profOption->scaleOrigin = 0.0;
                  profOption->minYReplace = 2.0;
                  profOption->maxYReplace = 8.0;
                  break;
               case (4):
                  strcpy (profOption->name, ALTERATION_PARAM_4);
                  profOption->type = SUS_Z_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -5.0;
                  profOption->maxYScale =  5.0;
                  profOption->scaleOrigin = 0.0;
                  profOption->minYReplace = 2.0;
                  profOption->maxYReplace = 8.0;
                  break;
               case (5):
                  strcpy (profOption->name, ALTERATION_PARAM_5);
                  profOption->type = SUS_DIP_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -180.0;
                  profOption->maxYScale =  180.0;
                  profOption->scaleOrigin = 1.0;
                  profOption->minYReplace = -180.0;
                  profOption->maxYReplace =  180.0;
                  break;
               case (6):
                  strcpy (profOption->name, ALTERATION_PARAM_6);
                  profOption->type = SUS_DDIR_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -360.0;
                  profOption->maxYScale =  360.0;
                  profOption->scaleOrigin = 1.0;
                  profOption->minYReplace = -360.0;
                  profOption->maxYReplace =  360.0;
                  break;
               case (7):
                  strcpy (profOption->name, ALTERATION_PARAM_7);
                  profOption->type = SUS_PITCH_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -360.0;
                  profOption->maxYScale =  360.0;
                  profOption->scaleOrigin = 1.0;
                  profOption->minYReplace = -360.0;
                  profOption->maxYReplace =  360.0;
                  break;
               case (8):                                     
                  strcpy (profOption->name, ALTERATION_PARAM_8);
                  profOption->type = REM_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -10.0;
                  profOption->maxYScale =  10.0;
                  profOption->scaleOrigin = 0.0;
                  profOption->minYReplace = -10.0;
                  profOption->maxYReplace =  10.0;
                  break;
               case (9):
                  strcpy (profOption->name, ALTERATION_PARAM_9);
                  profOption->type = REM_DEC_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -360.0;
                  profOption->maxYScale =  360.0;
                  profOption->scaleOrigin = 1.0;
                  profOption->minYReplace = -360.0;
                  profOption->maxYReplace =  360.0;
                  break;
               case (10):
                  strcpy (profOption->name, ALTERATION_PARAM_10);
                  profOption->type = REM_INC_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -360.0;
                  profOption->maxYScale =  360.0;
                  profOption->scaleOrigin = 1.0;
                  profOption->minYReplace = -360.0;
                  profOption->maxYReplace =  360.0;
                  break;
               case (11):
                  strcpy (profOption->name, ALTERATION_PARAM_11);
                  profOption->type = REM_STR_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = -5.0;
                  profOption->maxYScale =  5.0;
                  profOption->scaleOrigin = 0.0;
                  profOption->minYReplace = -5.0;
                  profOption->maxYReplace =  5.0;
                  break;
               default:
                  strcpy (profOption->name, "Graph");
                  profOption->type = UNDEFINED_PROFILE;
                  profOption->minX = 0.0;
                  profOption->maxX = 0.0;
                  profOption->minYScale = 0.0;
                  profOption->maxYScale = 1.0;
                  profOption->scaleOrigin = 0.5;
                  profOption->minYReplace = 0.0;
                  profOption->maxYReplace = 1.0;
            }
            profOption->joinType = LINE_SEGMENTS;
            profOption->graphLength = 200.0;
            profOption->array = NULL;
            profOption->next = NULL;
            profOption->points = initGraphPoints ((PROFILE_POINT *) NULL,
                                 2, STRAIGHT_PROFILE, profOption);
            profOption->array = getArrayForProfile (profOption);
            
            if (!firstOption)
               firstOption = profOption;
            if (lastOption)
               lastOption->next = profOption;

            lastOption = profOption;
         }
         break;
   }
   
   return (firstOption);
}

/* ======================================================================
FUNCTION        initAllProfiles
DESCRIPTION
        initialise the profiles for defining alteration zones or
        stand surfaces
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
initAllProfiles (OBJECT *object)
#else
initAllProfiles (object)
OBJECT *object;
#endif
{
   switch (object->shape)
   {
      case FOLD:
         {
            FOLD_OPTIONS *options = (FOLD_OPTIONS *) object->options;

                   /* setup the four co-efficients for a sine wave */
            initSineFourierCoeff (options->fourierCoeff);
            strcpy (options->profile.name, "Fold Profile");
            options->profile.joinType = LINE_SEGMENTS;
            options->profile.graphLength = 200.0;
            options->profile.array = NULL;
            options->profile.next = NULL;
            options->profile.type = PLANE_PROFILE;
            options->profile.minX = 0.0;
            options->profile.maxX = 6.28;
            options->profile.minYScale = -1.0;
            options->profile.maxYScale = 1.0;
            options->profile.scaleOrigin = 0.0;
            options->profile.minYReplace = -1.0;
            options->profile.maxYReplace = 1.0;
            options->profile.points = initGraphPoints ((PROFILE_POINT *) NULL,
                                          21, SINE_PROFILE, &(options->profile));
            options->profile.array = getArrayForProfile (&(options->profile));
         }
         break;
      case FAULT:
         {
            FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;

                   /* setup the four co-efficients for a sine wave */
            initSineFourierCoeff (options->fourierCoeff);
            strcpy (options->profile.name, "Fault Plane");
            options->profile.joinType = LINE_SEGMENTS;
            options->profile.graphLength = 200.0;
            options->profile.array = NULL;
            options->profile.next = NULL;
            options->profile.type = PLANE_PROFILE;
            options->profile.minX = 0.0;
            options->profile.maxX = 6.28;
            options->profile.minYScale = -1.0;
            options->profile.maxYScale = 1.0;
            options->profile.scaleOrigin = 0.0;
            options->profile.minYReplace = -1.0;
            options->profile.maxYReplace = 1.0;
            options->profile.points = initGraphPoints ((PROFILE_POINT *) NULL,
                                          21, SINE_PROFILE, &(options->profile));
            options->profile.array = getArrayForProfile (&(options->profile));

            options->alterationZones = NONE_ALTERATION;
            options->alterationFuntions = initAlterationZones (object->shape);
         }
         break;
      case UNCONFORMITY:
         {
            UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;

            options->alterationZones = NONE_ALTERATION;
            options->alterationFuntions = initAlterationZones (object->shape);

         }
         break;
      case SHEAR_ZONE:
         {
            SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;

                   /* setup the four co-efficients for a sine wave */
            initSineFourierCoeff (options->fourierCoeff);
            strcpy (options->profile.name, "Shear Plane");
            options->profile.joinType = LINE_SEGMENTS;
            options->profile.graphLength = 200.0;
            options->profile.array = NULL;
            options->profile.next = NULL;
            options->profile.type = PLANE_PROFILE;
            options->profile.minX = 0.0;
            options->profile.maxX = 6.28;
            options->profile.minYScale = -1.0;
            options->profile.maxYScale = 1.0;
            options->profile.scaleOrigin = 0.0;
            options->profile.minYReplace = -1.0;
            options->profile.maxYReplace = 1.0;
            options->profile.points = initGraphPoints ((PROFILE_POINT *) NULL,
                                          21, SINE_PROFILE, &(options->profile));
            options->profile.array = getArrayForProfile (&(options->profile));

            options->alterationZones = NONE_ALTERATION;
            options->alterationFuntions = initAlterationZones (object->shape);
         }
         break;
      case DYKE:
         {
            DYKE_OPTIONS *options = (DYKE_OPTIONS *) object->options;

            options->alterationZones = NONE_ALTERATION;
            options->alterationFuntions = initAlterationZones (object->shape);
         }
         break;
      case PLUG:
         {
            PLUG_OPTIONS *options = (PLUG_OPTIONS *) object->options;

            options->alterationZones = NONE_ALTERATION;
            options->alterationFuntions = initAlterationZones (object->shape);
         }
         break;
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        interpDblArray
DESCRIPTION
        do a linear interpolation of a 2 dimension array of double to
        another size array allowing for gain (scale) and an offset
INPUT

OUTPUT

RETURNED
====================================================================== */
int 
#if XVT_CC_PROTO
interpDblArray (double **dataArray, double **interpArray, int numDataX, int numDataY,
                int numInterpX, int numInterpY, double gain, double offset)
#else
interpDblArray (dataArray, interpArray, numDataX, numDataY,
                numInterpX, numInterpY, gain, offset)
double **dataArray, **interpArray;
int numDataX, numDataY, numInterpX, numInterpY;
double gain, offset;
#endif
{
   int interpX, interpY, dataX, dataY;
   int nextActualInterpX, nextActualInterpY;
   double ratioA, ratioB, ratioC, ratioA_preCalc, newValue;
   double interpolationX, interpolationY;
   
   interpolationX = (double) ((double) numInterpX)/((double) (numDataX));  /* numDataX-1 */
   interpolationY = (double) ((double) numInterpY)/((double) (numDataY));  /* numDataY-1 */

   for (interpY = 0; interpY < numInterpY; interpY++)
   {
      dataY = (int) floor (interpY/interpolationY);
      if (dataY > numDataY-2)
         dataY = numDataY - 2;
                         /* interpY index coresponding to actual data point */
      nextActualInterpY = (int) floor(dataY * interpolationY);
                                /*            ratioA_preCalc
                                       -|<--> Distance as percent to next point 
                                      |\    |
                                      |  \  |
                                      |    \|
                                       -----
                                */
      ratioA_preCalc = (nextActualInterpY+interpolationY-interpY)/interpolationY;

      for (interpX = 0; interpX < numInterpX; interpX++)
      {
         dataX = (int) floor ((interpX/interpolationX));
         if (dataX > numDataX-2)
            dataX = numDataX - 2;
                         /* interpX index coresponding to actual data point */
         nextActualInterpX = (int) floor(dataX * interpolationX);
         
                                 /* Divide into triangles |\| */
         if (interpX - nextActualInterpX > interpY - nextActualInterpY)
         {                                        /* Top */
            ratioA = 1.0 - ratioA_preCalc;
            ratioC = (nextActualInterpX+interpolationX-interpX)/interpolationX;
            ratioB = 1.0 - ratioA - ratioC;
            newValue = ratioA * dataArray[dataX+1][dataY+1] +
                       ratioB * dataArray[dataX+1][dataY  ] +
                       ratioC * dataArray[dataX  ][dataY  ];
            interpArray[interpX][interpY] = (newValue * gain) + offset;
         }
         else
         {                                        /* Bottom */
            ratioA = ratioA_preCalc;
            ratioC = 1.0 -
                     (nextActualInterpX+interpolationX-interpX)/interpolationX;
            ratioB = 1.0 - ratioA - ratioC;
            newValue = ratioA * dataArray[dataX  ][dataY  ] +
                       ratioB * dataArray[dataX  ][dataY+1] +
                       ratioC * dataArray[dataX+1][dataY+1];
            interpArray[interpX][interpY] = (newValue * gain) + offset;
         }
      }
   }
   return (TRUE);
}

/* ======================================================================
FUNCTION        overlayTopoOnArray
DESCRIPTION
   Match a topography file for an area to an array which may have
   very different dimensions and area
INPUT


OUTPUT

RETURNED
====================================================================== */
int 
#if XVT_CC_PROTO
overlayTopoOnArray (double ** topoArray, int numTopoX, int numTopoY,
                    double topoTop, double topoBot,
                    double topoLeft, double topoRight,
                    double ** mapArray, int numMapX, int numMapY,
                    double mapTop, double mapBot,
                    double mapLeft, double mapRight)
#else
overlayTopoOnArray (topoArray, numTopoX, numTopoY,
                    topoTop, topoBot, topoLeft, topoRight,
                    mapArray, numMapX, numMapY,
                    mapTop, mapBot, mapLeft, mapRight)
double ** topoArray;
int numTopoX, numTopoY;
double topoTop, topoBot, topoLeft, topoRight;
double ** mapArray;
int numMapX, numMapY;
double mapTop, mapBot, mapLeft, mapRight;
#endif
{
   double topoBlockSizeX, topoBlockSizeY;
   double mapBlockSizeX, mapBlockSizeY;
   double mapLocX, mapLocY;
   int topoX, topoY, mapX, mapY;
	int writeOutTopo = FALSE;
   
   topoBlockSizeX = (double) (topoRight - topoLeft)/numTopoX;
   topoBlockSizeY = (double) (topoBot - topoTop)/numTopoY;
   mapBlockSizeX = (double) (mapRight - mapLeft)/numMapX;
   mapBlockSizeY = (double) (mapBot - mapTop)/numMapY;

                      /* start at 1 and goes to <= as mapArray start at 1,1 */
   for (mapY = 1, mapLocY = mapTop+mapBlockSizeY/2.0;  /* was mapLocY = mapTop */
                   mapY <= numMapY; mapY++, mapLocY += mapBlockSizeY)
   {
                      /* +1's because values start at 1,1 in topoArray */
      topoY = (int) floor(((mapLocY-topoTop) / topoBlockSizeY) /*+ 0.5*/)+ 1;  /* -topoTop */
                      /* Make sure the indexs to topoArray are in range */
      if (topoY < 1) topoY = 1;
      else if (topoY > numTopoY) topoY = numTopoY;

      for (mapX = 1, mapLocX = mapLeft+mapBlockSizeX/2.0; /* was mapLocX = mapLeft */
                   mapX <= numMapX; mapX++, mapLocX += mapBlockSizeX)
      {
                      /* +1's because values start at 1,1 in topoArray */
         topoX = (int) floor(((mapLocX-topoLeft)/ topoBlockSizeX) /*+ 0.5*/)+ 1;
                      /* Make sure the indexs to topoArray are in range */
         if (topoX < 1) topoX = 1;
         else if (topoX > numTopoX) topoX = numTopoX;
                      /* No Interpolation we just get the geologically
                      ** closest value and use it */
         mapArray[mapX][numMapY-mapY+1] = topoArray[topoX][numTopoY-topoY+1];

if (writeOutTopo)
{
	FILE *fo;
	fo = fopen("./topoout.geo","a");
	fprintf(fo, "%lf\t",mapArray[mapX][numMapY-mapY+1]);
	fclose (fo);
}
		}
if (writeOutTopo)
{
	FILE *fo;
	fo = fopen("./topoout.geo","a");
	fprintf(fo, "\n");
	fclose (fo);
}
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        mapArrayToArray
DESCRIPTION
   Match a topography file for an area to an array which may have
   very different dimensions and area
INPUT


OUTPUT

RETURNED
====================================================================== */
int 
#if XVT_CC_PROTO
mapArrayToArray (double **initArray, int numInitX, int numInitY,
                 double initTop,  double initBot,
                 double initLeft, double initRight,
                 double **mapArray, int numMapX, int numMapY,
                 double mapTop,   double mapBot,
                 double mapLeft,  double mapRight, int interpData)
#else
mapArrayToArray (initArray, numInitX, numInitY, initTop, initBot, initLeft, initRight,
                 mapArray, numMapX, numMapY, mapTop, mapBot, mapLeft, mapRight, interpData)
double ** initArray;
int numInitX, numInitY;
double initTop, initBot, initLeft, initRight;
double ** mapArray;
int numMapX, numMapY;
double mapTop, mapBot, mapLeft, mapRight;
int interpData;
#endif
{
   double initBlockSizeX, initBlockSizeY;
   double mapBlockSizeX, mapBlockSizeY;
   double mapLocX, mapLocY;
   int initX, initY, mapX, mapY;
   double padValue = 0.0, gain = 1.0, offset = 0.0;
   int outOfBoundsX = FALSE, outOfBoundsY = FALSE;
   
   initBlockSizeX = (double) (initRight- initLeft)/numInitX;
   initBlockSizeY = (double) (initTop  - initBot)/numInitY;
   mapBlockSizeX  = (double) (mapRight - mapLeft)/numMapX;
   mapBlockSizeY  = (double) (mapTop   - mapBot)/numMapY;

   if (interpData)
   {
      int nextActualInterpX, nextActualInterpY, dataX, dataY;
      double ratioA, ratioB, ratioC, ratioA_preCalc, newValue;
      double interpolationX, interpolationY;
      
      interpolationX = (double) (((double) initBlockSizeX)/((double) mapBlockSizeX));
      interpolationY = (double) (((double) initBlockSizeY)/((double) mapBlockSizeY));
   
      for (mapY = 0, mapLocY = mapBot + mapBlockSizeY/2.0;
                     mapY < numMapY; mapY++, mapLocY += mapBlockSizeY)
      {
         if ((mapLocY < initBot) || (mapLocY > initTop))
            outOfBoundsY = TRUE;
         else
         {
            outOfBoundsY = FALSE;
            dataY = (int) floor (mapY/interpolationY);
            if (dataY > numInitY-2)
               dataY = numInitY-2;
                         /* mapY index coresponding to actual data point */
            nextActualInterpY = (int) floor (dataY * interpolationY);
                                /*            ratioA_preCalc
                                       -|<--> Distance as percent to next point 
                                      |\    |
                                      |  \  |
                                      |    \|
                                       -----
                                */
            ratioA_preCalc = (nextActualInterpY+interpolationY-mapY)/interpolationY;
         }
            
         for (mapX = 0, mapLocX = mapLeft + mapBlockSizeX/2.0;
                        mapX < numMapX; mapX++, mapLocX += mapBlockSizeX)
         {
            if ((mapLocX < initLeft) || (mapLocX > initRight))
               outOfBoundsX = TRUE;
            else
            {
               outOfBoundsX = FALSE;
               dataX = (int) floor (mapX/interpolationX);
               if (dataX > numInitX-2)
                  dataX = numInitX-2;
                         /* mapX index coresponding to actual data point */
               nextActualInterpX = (int) floor (dataX * interpolationX);
            }
            
            if (outOfBoundsX || outOfBoundsY)
               mapArray[mapY][mapX] = padValue;
            else
            {
                                 /* Divide into triangles |\| */
               if (mapX - nextActualInterpX > mapY - nextActualInterpY)
               {                                        /* Top */
                  ratioA = 1.0 - ratioA_preCalc;
                  ratioC = (nextActualInterpX+interpolationX-mapX)/interpolationX;
                  ratioB = 1.0 - ratioA - ratioC;
                  newValue = ratioA * initArray[dataY+1][dataX+1] +
                             ratioB * initArray[dataY  ][dataX+1] +
                             ratioC * initArray[dataY  ][dataX  ];
                  mapArray[mapY][mapX] = (newValue * gain) + offset;
               }
               else
               {                                        /* Bottom */
                  ratioA = ratioA_preCalc;
                  ratioC = 1.0 -
                           (nextActualInterpX+interpolationX-mapX)/interpolationX;
                  ratioB = 1.0 - ratioA - ratioC;
                  newValue = ratioA * initArray[dataY  ][dataX  ] +
                             ratioB * initArray[dataY+1][dataX  ] +
                             ratioC * initArray[dataY+1][dataX+1];
                  mapArray[mapY][mapX] = (newValue * gain) + offset;
               }
            }
         }
      }
   }
   else
#ifdef BROKEN
   if (interpData)
   {
      int initY2, initX2;
      double value, percent, pointX1, pointX2, pointY1, pointY2;
      double differenceX, differenceY, percentX, percentY;
      double valueDiffX, valueDiffY;
      double mapLocX2, mapLocY2;
      
      for (mapY = 0, mapLocY = mapBot + mapBlockSizeY/2.0; 
                     mapY < numMapY; mapY++, mapLocY += mapBlockSizeY)
      {
         outOfBoundsY = FALSE;
   
         mapLocY2 = mapLocY + mapBlockSizeY;
         initY = (int) floor(((mapLocY-initBot) / initBlockSizeY));
         initY2= (int) ceil(((mapLocY2-initBot) / initBlockSizeY));
                         /* Make sure the indexs to initArray are in range */
         if (initY < 0)
            outOfBoundsY = TRUE;
         else if (initY > numInitY-1)
            outOfBoundsY = TRUE;
   
         for (mapX = 0, mapLocX = mapLeft + mapBlockSizeX/2.0; 
                        mapX < numMapX; mapX++, mapLocX += mapBlockSizeX)
         {
            outOfBoundsX = FALSE;

            mapLocX2 = mapLocX + mapBlockSizeX;
            initX = (int) floor(((mapLocX-initLeft)/ initBlockSizeX));
            initX2= (int) ceil(((mapLocX2-initLeft)/ initBlockSizeX));
                         /* Make sure the indexs to initArray are in range */
            if (initX < 0)
               outOfBoundsX = TRUE;
            else if (initX > numInitX-1)
               outOfBoundsX = TRUE;
               
                         /* No Interpolation we just get the geologically
                         ** closest value and use it */
            if (outOfBoundsX || outOfBoundsY)
               mapArray[mapY][mapX] = padValue;
            else
            {
               pointX1 = initX *initBlockSizeX + initLeft;
               pointX2 = initX2*initBlockSizeX + initLeft;
               differenceX = pointX2 - pointX1;
               percentX = (mapLocX - pointX1)/differenceX;
              
                           
               pointY1 = initY *initBlockSizeY + initBot;
               pointY2 = initY2*initBlockSizeY + initBot;
               differenceY = (initY2*initBlockSizeY + initBot)
                           - (initY *initBlockSizeY + initBot);
               percentY = (mapLocY - pointY1)/differenceY;
               
               valueDiffX = initArray[initY][initX] - initArray[initY][initX2];
               valueDiffY = initArray[initY][initX] - initArray[initY2][initX];

               mapArray[mapY][mapX] = (value * gain) + offset;
            }
         }
      }
   }
   else
#endif
   {
      for (mapY = 0, mapLocY = mapBot + mapBlockSizeY/2.0 - 1.0; 
                     mapY < numMapY; mapY++, mapLocY += mapBlockSizeY)
      {
         outOfBoundsY = FALSE;
   
         initY = (int) floor(((mapLocY-initBot) / initBlockSizeY) + 0.5);
                         /* Make sure the indexs to initArray are in range */
         if (initY < 0)
            outOfBoundsY = TRUE;
         else if (initY > numInitY-1)
            outOfBoundsY = TRUE;
   
         for (mapX = 0, mapLocX = mapLeft + mapBlockSizeX/2.0 - 1.0; 
                        mapX < numMapX; mapX++, mapLocX += mapBlockSizeX)
         {
            outOfBoundsX = FALSE;

            initX = (int) floor(((mapLocX-initLeft)/ initBlockSizeX) + 0.5);
                         /* Make sure the indexs to initArray are in range */
            if (initX < 0)
               outOfBoundsX = TRUE;
            else if (initX > numInitX-1)
               outOfBoundsX = TRUE;
               
                         /* No Interpolation we just get the geologically
                         ** closest value and use it */
            if (outOfBoundsX || outOfBoundsY)
               mapArray[mapY][mapX] = padValue;
            else
               mapArray[mapY][mapX] = (initArray[initY][initX] * gain) + offset;
         }
      }
   }
   
   return (TRUE);
}


/* ======================================================================
FUNCTION        getTopoValueAtLoc
DESCRIPTION
   Get the closes value of a topography at a particular
   real world location
INPUT


OUTPUT

RETURNED
====================================================================== */
double 
#if XVT_CC_PROTO
getTopoValueAtLoc (double ** topoArray, int numTopoX, int numTopoY,
                    double topoTop, double topoBot,
                    double topoLeft, double topoRight,
                    double xLoc, double yLoc, int *error)
#else
getTopoValueAtLoc (topoArray, numTopoX, numTopoY,
                   topoTop, topoBot, topoLeft, topoRight,
                   xLoc, yLoc, error)
double ** topoArray;
int numTopoX, numTopoY;
double topoTop, topoBot, topoLeft, topoRight;
double xLoc, yLoc;
int *error;
#endif
{
   double topoBlockSizeX, topoBlockSizeY;
   int topoX, topoY;
	double minTopoX, maxTopoX, minTopoY, maxTopoY;
   double value = 0.0;

	if (!topoArray)
	{
		*error = TRUE;
      return (value);
	}

   minTopoX = MIN(topoRight, topoLeft);
   minTopoY = MIN(topoTop, topoBot);
   maxTopoX = MAX(topoRight, topoLeft);
   maxTopoY = MAX(topoTop, topoBot);

   if ((xLoc < minTopoX) || (xLoc > maxTopoX) ||
       (yLoc < minTopoY) || (yLoc > maxTopoY))
	{
		*error = TRUE;
      return (value);
	}
   
   topoBlockSizeX = (double) (maxTopoX - minTopoX)/numTopoX;
   topoBlockSizeY = (double) (maxTopoY - minTopoY)/numTopoY;
   
   topoX = (int) floor ((xLoc-minTopoX)/topoBlockSizeX + 0.5) + 1;
   topoY = (int) floor ((yLoc-minTopoY)/topoBlockSizeY + 0.5) + 1;

   if ((topoX < 1) || (topoX > numTopoX) || (topoY < 1) || (topoY > numTopoY))
		*error = TRUE;
	else
	{
		/*value = topoArray[topoX][numTopoY-topoY+1];   .* Changed 9/4/97 - because draped survery was around the wrong way */
		value = topoArray[topoX][topoY];
		*error = FALSE;
	}
   
   return (value);
}

#ifdef PAUL
int
#if XVT_CC_PROTO
getFileToOpen(FILE_SPEC *filename, char *fileTypes, char *openMessage)
#else
getFileToOpen(filename, fileTypes, openMessage)
FILE_SPEC *filename;
char *fileTypes;
char *openMessage;
#endif
{
#if ((XVTWS == WIN16WS) || (XVTWS == WIN32WS))
	OPENFILENAME OpenFileName;
	char name[MAX_PATH];

	strcpy(name, "");

	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
   OpenFileName.hwndOwner         = NULL;
   OpenFileName.hInstance         = NULL;
   OpenFileName.lpstrFilter       = fileTypes;
   OpenFileName.lpstrCustomFilter = NULL;
   OpenFileName.nMaxCustFilter    = 0;
   OpenFileName.nFilterIndex      = 0;
   OpenFileName.lpstrFile         = name;
   OpenFileName.nMaxFile          = sizeof(name);
   OpenFileName.lpstrFileTitle    = NULL;
   OpenFileName.nMaxFileTitle     = 0;
   OpenFileName.lpstrInitialDir   = NULL;
   OpenFileName.lpstrTitle        = openMessage;
   OpenFileName.nFileOffset       = 0;
   OpenFileName.nFileExtension    = 0;
   OpenFileName.lpstrDefExt       = NULL;
   OpenFileName.lCustData         = 0;
	OpenFileName.lpfnHook 		   = NULL;
	OpenFileName.lpTemplateName    = 0;
   OpenFileName.Flags             = OFN_SHOWHELP | OFN_EXPLORER;

	/* Call the common dialog function. */
   if (GetOpenFileName(&OpenFileName))
   {
		strcpy (filename->name, &(name[OpenFileName.nFileOffset]));
		name[OpenFileName.nFileOffset] = '\0';
		xvt_fsys_convert_str_to_dir(name, &(filename->dir));

		return (TRUE);
	}
	else
	{
		DWORD errorType;
		errorType = CommDlgExtendedError();
		return (FALSE);
	}
#endif

}
#endif

/* ======================================================================
FUNCTION        findFile
DESCRIPTION
        Search logical directories for the specified file and return
        the path in the "file" paramater                             
INPUT

OUTPUT

RETURNED
      TRUE - file was found, FALSE - not found
====================================================================== */
int 
#if XVT_CC_PROTO
findFile(FILE_SPEC *file)
#else
findFile(file)
FILE_SPEC *file;
#endif
{
   FILE_SPEC fileToTry;
   char *strPtr;
   
   memcpy (&fileToTry, file, sizeof(FILE_SPEC));

                    /* First try the directory it is supposed to be in */
   if ((BOOLEAN) xvt_fsys_get_file_attr(&fileToTry, XVT_FILE_ATTR_READ) == TRUE)
   {
      memcpy (file, &fileToTry, sizeof(FILE_SPEC));
      return (TRUE);
   }
  
                    /* Next try the default directory  */
   xvt_fsys_get_default_dir (&(fileToTry.dir));
   if ((BOOLEAN) xvt_fsys_get_file_attr(&fileToTry, XVT_FILE_ATTR_READ) == TRUE)
   {
      memcpy (file, &fileToTry, sizeof(FILE_SPEC));
      return (TRUE);
   }


                    /* Next try the HOME directory  */
   if (strPtr = (char *) getenv ("HOME"))
   {
      xvt_fsys_convert_str_to_dir (strPtr, &(fileToTry.dir));
      if ((BOOLEAN) xvt_fsys_get_file_attr(&fileToTry, XVT_FILE_ATTR_READ) == TRUE)
      {
         memcpy (file, &fileToTry, sizeof(FILE_SPEC));
         return (TRUE);
      }
   }

                    /* Next try the NODDY_HOME directory  */
   if (strPtr = (char *) getenv ("NODDY_HOME"))
   {
      xvt_fsys_convert_str_to_dir (strPtr, &(fileToTry.dir));
      if ((BOOLEAN) xvt_fsys_get_file_attr(&fileToTry, XVT_FILE_ATTR_READ) == TRUE)
      {
         memcpy (file, &fileToTry, sizeof(FILE_SPEC));
         return (TRUE);
      }
   }

	                    /* Next try the NODDY_HELP directory  */
   if (strPtr = (char *) getenv ("NODDY_HELP"))
   {
      xvt_fsys_convert_str_to_dir (strPtr, &(fileToTry.dir));
      if ((BOOLEAN) xvt_fsys_get_file_attr(&fileToTry, XVT_FILE_ATTR_READ) == TRUE)
      {
         memcpy (file, &fileToTry, sizeof(FILE_SPEC));
         return (TRUE);
      }
   }

   return (FALSE);
}

/* ======================================================================
FUNCTION        incrementFilename
DESCRIPTION
        increment the name passed to the next non existing file
        using the passed in name as a base.
INPUT

OUTPUT

RETURNED
      TRUE - increment ok, FALSE - some trouble incrementing
====================================================================== */
int 
#if XVT_CC_PROTO
incrementFilename(FILE_SPEC *file)
#else
incrementFilename(file)
FILE_SPEC *file;
#endif
{  
   char filename[100];
   char ending[50];
   char *position;
   char *extPtr; /* pointer to Extention */
   int currentNum;
   
   strcpy (filename, file->name);

#if (XVTWS == MACWS)
   position = &(filename[strlen(filename)]);
   strcpy (ending, position); /* take a copy of what comes after */
#else
   if (extPtr = (char *) strrchr (filename, '.'))
      position = extPtr;
   else
      position = &(filename[strlen(filename)]);
      
   strcpy (ending, position); /* take a copy of what comes after */
#endif

   *position = '\0';  /* terminate the string were we are */

            /* first back up over any number that is already there */
   while (isdigit((int) *(position-1)))
      position--;
      
   currentNum = 0;
   sscanf (position, "%d", &currentNum);  /* read of the current file num */

      /* increment to the next file num */
   currentNum++;         
   
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS) 
             /* On PC we only have 8 chars for file name so make sure
             ** we have at 3 characters for the number */
   if (position - filename  > 5)
      position = filename + 5;
#endif
   
      /* put this number into the filename and append the old ending */
   sprintf(position, "%03d%s", currentNum, ending);
     
   strcpy (file->name, filename);
    
   return (TRUE);
}  

/* ======================================================================
FUNCTION        addFileExtention
DESCRIPTION
        Add entention to the filename passed in
INPUT

OUTPUT

RETURNED
      TRUE - added, FALSE - could not add
====================================================================== */
int
#if XVT_CC_PROTO
addFileExtention (char *filename, char *extension)
#else
addFileExtention (filename, extension)
char *filename,  *extension;
#endif
{
   char *ptr;
   
   if (!filename)
      return (FALSE);
   if (!extension)
      return (FALSE);
      
   if (ptr = (char *) strrchr (filename, '.'))
   {
      if (extension[0] == '.')     
         strcpy (ptr, extension);  /* overwrite '.' */
      else
         strcpy (++ptr, extension); /* preserve '.' */
   }
   else
   {
      if (extension[0] == '.')     
         strcat (filename, extension);  /* includes '.' so dont add */
      else
      {
         strcat (filename, "."); /* add '.' */
         strcat (filename, extension);
      }
   }
      
   return (TRUE);
}
/* ======================================================================
FUNCTION        copyit
DESCRIPTION
        copy a values out of a line and into a variable
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
copyit(FILE *in, char *fmt, void *var)
#else
copyit(in, fmt, var)
FILE *in;
char *fmt;
void *var;
#endif
{
   char oneline[255];
   char *ptr;

   readGoodLine (in, oneline);
   if (ptr = (char *) strchr (oneline, '='))
      sscanf((char *)ptr+1,(char *) fmt, var);
}

/* ======================================================================
FUNCTION        equalstring2
DESCRIPTION
        check if two strings are equal
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
equalstring2(char *a, char *b)
#else
equalstring2(a,b)
char *a,*b;
#endif
{
   int i=0;

   do
   {
      if (a[i] == b[i])
         i++;
      else
         return (FALSE);
   } while (a[i-1] != '\0');

   return (TRUE);
}

/* ======================================================================
FUNCTION        equalstring3
DESCRIPTION
        check if two strings are equal
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
equalstring3(char *a, char *b)
#else
equalstring3(a,b)
char *a,*b;
#endif
{
   int i=0;

   do
   {
      if (a[i] == b[i])
         i++;
      else
         return (FALSE);
   } while (a[i-1] != '\0' && i < 12);

   return(TRUE);
}

/* ======================================================================
FUNCTION        loadit
DESCRIPTION
        copy a values out of a line and into a variable
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadit(FILE *in, char *fmt, void *var)
#else
loadit(in, fmt, var)
FILE *in;
char *fmt;
void *var;
#endif
{
   char oneline[255];
   char *ptr;
   int i=0;

   readGoodLine(in, oneline);
   while ((oneline[i] != '=') && (oneline[i] != '\0'))
      i++;
   if (oneline[i] != '\n')
   {
      if (strcmp (fmt, "%s") == 0)  /* copy multiple words with %s */
      {
         strcpy (var, "");
         sscanf((char *)&oneline[i+1], "%s", (char *) var);
         if (ptr = strstr ((char *)&oneline[i+1], var))
            strcpy ((char *) var, (char *) ptr);
         if (ptr = (char *) strchr (var, '\n'))
            *ptr = '\0';
      }
      else
         sscanf((char *)&oneline[i+1],(char *) fmt, var);
   }
}

/* ======================================================================
FUNCTION        loaditrock
DESCRIPTION
        copy a values out of a line and into a variable
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loaditrock(FILE *in, char *fmt, void *var)
#else
loaditrock(in, fmt, var)
FILE *in;
char *fmt;
void *var;
#endif
{
   char oneline[255];
   char *ptr;
   int i=0;

   readGoodLine(in,oneline);
   if (ptr = (char *) strchr (oneline, '='))
      strcpy(var, (char *) ptr+1);
}

/* ======================================================================
FUNCTION        setWindowState
DESCRIPTION
        Maximise of minimise windows
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
setWindowState ( WINDOW win, int state )
#else
setWindowState (win, state)
WINDOW win;
int state;
#endif
{
#if (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
    HWND hwnd = (HWND)xvt_vobj_get_attr((win), ATTR_NATIVE_WINDOW);
    int winflag;

    switch (state) {
        case WIN_MINIMIZED:
            winflag =  SW_SHOWMINNOACTIVE;
            break;
        case WIN_MAXIMIZED:
            winflag =  SW_SHOWMAXIMIZED;
            break;
        case WIN_NORMAL:
        default:
            winflag =  SW_SHOWNORMAL;
            break;
    }
    ShowWindow( hwnd, winflag );
#endif
}

/* ======================================================================
FUNCTION        setBlockSize
DESCRIPTION
        return the block size to be used on the screen diagrams.
INPUT

OUTPUT

RETURNED
====================================================================== */
double
#if XVT_CC_PROTO
setBlockSize (int resolution)
#else
setBlockSize (resolution)
int resolution;
#endif
{
   double BSize;
   int size;

   if (resolution == 50) /* BLOCK_HIGH */
   {
      BSize = 0.0558;
      size = SMALL_PEN_SIZE;
   }
   else if (resolution == 100) /* BLOCK_MEDHIGH */
   {
      BSize = 0.115;
      size = MED_SMALL_PEN_SIZE;
   }
   else if (resolution == 200) /* BLOCK_MEDIUM */
   {
      BSize = 0.23;
      size = MEDIUM_PEN_SIZE;
   }
   else if (resolution == 350) /* BLOCK_MEDLOW */
   {
      BSize = 0.46;
      size = MED_LARGE_PEN_SIZE;
   }
   else if (resolution == 500) /* BLOCK_LOW */
   {
      BSize = 0.92;
      size = LARGE_PEN_SIZE;
   }


   SetPointSize (size);

   return (BSize);
}

/*
======================================================================
FUNCTION        displayHelp
DESCRIPTION
   display a particular help tab
INPUT
     char *topic
OUTPUT
RETURNED
     TRUE - sucess, FALSE - failure
======================================================================
*/
int
#if XVT_CC_PROTO
displayHelp (char *topic)
#else
displayHelp (topic)
char *topic;
#endif
{
	int result;
	char command[200], dir[100];
	char *cmdPtr, *pathPtr;
	FILE_SPEC helpFile;

	if (!(pathPtr = (char *) strstr(projectOptions.helpPath, "%h")))
	{
		xvt_dm_post_error("Error, No %%h to specify html file in Help Path. Check Options.");
		return (FALSE);
	}

	strcpy(command, projectOptions.helpPath);
	if (!(cmdPtr = (char *) strstr(command, "%h")))
	{
		xvt_dm_post_error("Error, No %%h to specify html file in Command. Check Options.");
		return (FALSE);
	}
		/* Build the correct path for the file */
	xvt_fsys_convert_dir_to_str(&defaultDir, dir, 100);
#if (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
	strcat (dir, "\\Help");
#endif
#if (XVTWS == MTFWS) || (XVTWS == XOLWS)
	strcat (dir, "/Help");
#endif
#if (XVTWS == MACWS)
	strcat (dir, ":Help");
#endif
	strcpy (helpFile.name, topic);
	xvt_fsys_convert_str_to_dir(dir, &(helpFile.dir));
   if (!findFile (&helpFile))
	{
		//xvt_dm_post_error("Error, Cannot Find File (%s).", helpFile.name);
		return (FALSE);
	}
	xvt_fsys_convert_dir_to_str(&(helpFile.dir), dir, 100);
#if (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
	strcat (dir, "\\");
#endif
#if (XVTWS == MTFWS) || (XVTWS == XOLWS)
	strcat (dir, "/");
#endif
#if (XVTWS == MACWS)
	strcat (dir, ":");
#endif
	strcat (dir, helpFile.name);
		/* Put the file we have found in the command */
	strcpy(cmdPtr, dir);
	strcat(command, pathPtr+2);

   result = system (command);
	if (result == -1)
	{
		//xvt_dm_post_error("Error, Help command Failed (%s)", command);
		return (FALSE);
	}

	return (TRUE);
}

/*
======================================================================
FUNCTION        processCommandLine
DESCRIPTION
   pick appart the command line to see what operations are wanted
INPUT
     char **argv - command options array
     char argc - number of args in argv array
OUTPUT
     char **historyFile - pointer to name of history input file
        char **outputFile - pointer to the output file name
        char *operationName - string that describes the operation
        OPERATIONS *operation - the operation that will be performed
RETURNED
     nothing
======================================================================
*/
void
#if XVT_CC_PROTO
processCommandLine (char **argv, int argc,
                    char **historyFile, char **outputFile, char **blockFile,
                    char *operationName, OPERATIONS *operation)
#else
processCommandLine (argv, argc, historyFile, outputFile, blockFile,
                                operationName, operation)
char **argv;
int argc;
char **historyFile, **outputFile, **blockFile;
char *operationName;
OPERATIONS *operation;
#endif
{
   int option;
   int recognised;

                      /* pick out exactly what we are about to do */
   for (option = 1; option < argc; option++)
   {
      recognised = FALSE;
                               /* any .his file is a history file */
      if (strstr(argv[option],".his"))
      {
         *historyFile = argv[option];
         recognised = TRUE;
      }                        /* -block means produce a block Diagram */
      else if (strcmp(argv[option], "-block") == 0)
      {
         *operation = CALC_BLOCK_MODEL; //mwj_hack
         strcpy (operationName,"Block Model");
         recognised = TRUE;
      }                     /* -dicer means produce a block Diagram (Dicer) */
      else if (strcmp(argv[option], "-dicer") == 0)
      {
         *operation = DICER_BLOCK_MODEL;
         strcpy (operationName,"Block Model (Dicer Format)");
         recognised = TRUE;
      }                        /* -linemap means produce a Line Map */
      else if (strcmp(argv[option], "-linemap") == 0)
      {
         *operation = LINE_MAP_DIAGRAM;
         strcpy (operationName,"Line Map Diagram");
         recognised = TRUE;
      }                        /* -section means produce a Section */
      else if (strcmp(argv[option], "-section") == 0)
      {
         *operation = SECTION_DIAGRAM;
         strcpy (operationName,"Section Diagram");
         recognised = TRUE;
      }                  /* -geoBlock means produce a hollow block Diagram */
      else if (strcmp(argv[option], "-geoBlock") == 0)
      {
         *operation = BLOCK_DIAGRAM;
         strcpy (operationName,"Block Diagram");
         recognised = TRUE;
      }                        /* -anom means produce a Anomalies */
      else if (strcmp(argv[option], "-anom") == 0)
      {
         *operation = ANOMALIES;
         strcpy (operationName,"Anomalies");
         recognised = TRUE;
      }        /* -anom+block means produce a Anomalies Image + block file */
      else if ((strcmp(argv[option], "-anomAndBlock") == 0)
               || (strcmp(argv[option], "-blockAndAnom") == 0))
      {
         *operation = ANOMALIES_AND_BLOCK;
         strcpy (operationName,"Anomalies Image and Block");
         recognised = TRUE;
      }       /* -anom means produce a Anomalies Image from Block File */
      else if (strcmp(argv[option], "-anomFromBlock") == 0)
      {
         *operation = ANOMALIES_FROM_BLOCK;
         strcpy (operationName,"Anomalies Image from Block");
         recognised = TRUE;
      }                        /* -profile means produce a Anomalies Profile */
      else if (strcmp(argv[option], "-profile") == 0)
      {
         *operation = ANOMALIES_PROFILE;
         strcpy (operationName,"Anomalies Profile");
         recognised = TRUE;
      }                        /* -image means produce a Anomalies Image */
      else if (strcmp(argv[option], "-image") == 0)
      {
         *operation = ANOMALIES_IMAGE;
         strcpy (operationName,"Anomalies Image");
         recognised = TRUE;
      }                        /* -h also means next option is history file */
      else if ((argv[option][0] == '-') && (argv[option][1] == 'h'))
      {
         *historyFile = argv[option+1];
         recognised = TRUE;
      }                        /* -o means next option is output file */
      else if ((argv[option][0] == '-') && (argv[option][1] == 'o'))
      {
         *outputFile = argv[option+1];
         recognised = TRUE;
      }                        /* -b means next option is block file */
      else if ((argv[option][0] == '-') && (argv[option][1] == 'b'))
      {
         *blockFile = argv[option+1];
         recognised = TRUE;
      }             /* option was a -o or -h (so this is the other half) */
      else if ((option > 0) && (argv[option-1][0] == '-') &&
               (argv[option-1][1] == 'h') || (argv[option-1][1] == 'o')
                                          || (argv[option-1][1] == 'b'))
      {
         recognised = TRUE;
      }

                               /* -help is asking for help */
      if ((!recognised) || (strcmp(argv[option], "-help") == 0)
                        || (strcmp(argv[option], "-?") == 0))
      {
         if (!recognised)
            fprintf(stderr,"\nERROR - Invalid Option: %s\n",argv[option]);
         *historyFile = NULL;
         break;
      }
   }
}


/*
======================================================================
FUNCTION        displayUsage
DESCRIPTION
   display the command line options for this command to the stderr
INPUT   char *commandName;    The name of the command run
OUTPUT
RETURNED
======================================================================
*/
void
#if XVT_CC_PROTO
displayUsage (char *commandName)
#else
displayUsage (commandName)
char *commandName;
#endif
{
   fprintf (stderr,"\nUSAGE: %s <history_file> [options]", commandName);
   fprintf (stderr,"\n");
   fprintf (stderr,"\nOptions Include:");
   fprintf (stderr,"\n\t-h history_file");
   fprintf (stderr,"\n\t-o output_file");
   fprintf (stderr,"\n\t-b block_file (see anomAndBlock, anomFromBlock)");
   fprintf (stderr,"\n\t-block produce a block model of geophysical area");
   fprintf (stderr,"\n\t-dicer same as block but in Dicer Format");
   fprintf (stderr,"\n\t-linemap produce geological linemap diagram");
   fprintf (stderr,"\n\t-section produce geological section diagram");
   fprintf (stderr,"\n\t-geoBlock produce geological block diagram");
   fprintf (stderr,"\n\t-anom  produce anomalies gravity and magnetic files");
   fprintf (stderr,"\n\t-anomAndBlock same as -anom but also a block file");
   fprintf (stderr,"\n\t-anomFromBlock same as -anom but from a specified block file");
   fprintf (stderr,"\n\t-profile force anomalies profile to be produced");
   fprintf (stderr,"\n\t-image force anomalies image to be produced");
#ifndef _MPL
   fprintf (stderr,"\n\t-parallel do on parallel machine (if available)");
#endif
   fprintf (stderr,"\n\t-help or -? show this USAGE");
   fprintf (stderr,"\n\n");
}

/*
======================================================================
FUNCTION        performBatchOperations
DESCRIPTION
   execute the appropriate procedures to perform the batch operation
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
performBatchOperations (char *outputFile, char *blockFile,
                        OPERATIONS operation)
#else
performBatchOperations (outputFile, blockFile, operation)
char *outputFile, *blockFile;
OPERATIONS operation;
#endif
{
   int result = TRUE;
   int nx, ny, nz;
   double blockSize;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions();
   strcpy((char *)blockFile,(char *)outputFile);//mwj_hack
   switch (operation)
   {
      case (BLOCK_MODEL):
         doGeophysics (BLOCK_ONLY, viewOptions, &geophysicsOptions, outputFile, blockFile, NULL, 0, NULL, NULL, NULL);
         break;
      case (DICER_BLOCK_MODEL):
         DoSaveBlockDicer (blockFile);
         break;
      case (LINE_MAP_DIAGRAM):
         blockSize = setBlockSize ((int) viewOptions->geologyCubeSize);
         nx = (int) (25/blockSize);
         ny = (int) (17/blockSize);
         nz = 1;
         calcAndSaveBlock (blockFile, "LINE_MAP", nx, ny, nz);
         break;
      case (SECTION_DIAGRAM):
         blockSize = setBlockSize ((int) viewOptions->geologyCubeSize);
         nx = (int) (25/blockSize);
         ny = 1;
         nz = (int) (12/blockSize);
         calcAndSaveBlock (blockFile, "SECTION", nx, ny, nz);
         break;
      case (BLOCK_DIAGRAM):
         blockSize = setBlockSize ((int) viewOptions->geologyCubeSize);
         nx = (int) (25/blockSize);
         ny = (int) (17/blockSize);
         nz = (int) (11.3/blockSize);
         calcAndSaveBlock (blockFile, "BLOCK", nx, ny, nz);
         break;
      case (ANOMALIES):
         doGeophysics (ANOM, viewOptions, &geophysicsOptions, outputFile, NULL, NULL, 0, NULL, NULL, NULL);
         break;
      case (ANOMALIES_AND_BLOCK):
         doGeophysics (BLOCK_AND_ANOM, viewOptions, &geophysicsOptions, outputFile, blockFile, NULL, 0, NULL, NULL, NULL);
         break;
      case (ANOMALIES_FROM_BLOCK):
         doGeophysics (ANOM_FROM_BLOCK, viewOptions, &geophysicsOptions, outputFile, blockFile, NULL, 0, NULL, NULL, NULL);
         break;
      case (ANOMALIES_IMAGE):
         doGeophysics (ANOM, viewOptions, &geophysicsOptions, outputFile, NULL, NULL, 0, NULL, NULL, NULL);
         break;
      case (ANOMALIES_PROFILE):
         doGeophysics (ANOM, viewOptions, &geophysicsOptions, outputFile, NULL, NULL, 0, NULL, NULL, NULL);
         break;
   }

   return (result);
}


/*
======================================================================
FUNCTION        noddy
DESCRIPTION
   function version of Noddy, take an array
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
noddy (DATA_INPUT inputFrom, DATA_OUTPUT outputToo, int operation,
       char *input, char *output,
       BLOCK_VIEW_OPTIONS *viewOptions, GEOPHYSICS_OPTIONS *geophOptions)
#else
noddy (inputFrom, outputToo, operation, input, output, viewOptions, geophOptions)
DATA_INPUT inputFrom;
DATA_OUTPUT outputToo;
int operation;
char *input, *output;
BLOCK_VIEW_OPTIONS *viewOptions;
GEOPHYSICS_OPTIONS *geophOptions;
#endif
{
   int calcBlock, calcMag, calcGrav,calcSurf;
   char blockFile[100],dxfname[250];

   if ( !input || !output)
      return (FALSE);
      
       /* setup for batch operation (no interaction) */
   initVariables();
   batchExecution = TRUE;
   
                         /* Find out what we are to calculate */
   calcBlock =  operation && CALC_BLOCK_MODEL;
   calcMag =  operation && CALC_MAGNETICS_IMAGE;
   calcGrav =  operation && CALC_GRAVITY_IMAGE;
   calcSurf =  operation && CALC_SURF_MODEL;
   
   switch (inputFrom)
   {
      case (INPUT_MEM_HISTORY): /* input is a history linked list */
         batchWindowInfo.head = (OBJECT *) input;
         break;
      case (INPUT_FILE_HISTORY): /* input is the history filename */
         if (!Load_status(input))
            return (FALSE);
         break;
      case (INPUT_FILE_BLOCK): /* input is the block filename */
         strcpy (blockFile, input);
         break;
   }
   
   if (!geophOptions)
      geophOptions = &geophysicsOptions;
   if (!viewOptions)
      viewOptions = getViewOptions ();

   if (operation == 1)
         doGeophysics (BLOCK_ONLY, viewOptions, geophOptions, output, output, NULL, 0, NULL, NULL, NULL); //mwj_hack
   if (operation == 16)
         doGeophysics (BLOCK_AND_ANOM, viewOptions, geophOptions, output, output, NULL, 0, NULL, NULL, NULL); //mwj_hack
   if (operation == 2)
         doGeophysics (ANOM, viewOptions, geophOptions, output, NULL, NULL, 0, NULL, NULL, NULL);
   if(operation == 8)
   {
	   sprintf((char *) dxfname,"%s.dxf",output);
	   do3dStratMap ((THREED_IMAGE_DATA *) NULL, dxfname);
   }
   if(operation == 32)
   {
	   sprintf((char *) dxfname,"%s.dxf",output);
	   doGeophysics (BLOCK_AND_ANOM, viewOptions, geophOptions, output, output, NULL, 0, NULL, NULL, NULL);
	   do3dStratMap ((THREED_IMAGE_DATA *) NULL, dxfname);
   }
   if(operation == 64)
   {
	   sprintf((char *) dxfname,"%s.dxf",output);
	   doGeophysics (BLOCK_ONLY, viewOptions, geophOptions, output, output, NULL, 0, NULL, NULL, NULL);
	   do3dStratMap ((THREED_IMAGE_DATA *) NULL, dxfname);
   }
   if(operation == 128)
   {
	   sprintf((char *) dxfname,"%s.dxf",output);
	   doGeophysics (TOPOLOGY, viewOptions, geophOptions, output, output, NULL, 0, NULL, NULL, NULL);
   }
   if(operation == 256)
   {
	   doGeophysics (ANOM_FROM_BLOCK, viewOptions, geophOptions, output, output, NULL, 0, NULL, NULL, NULL);
   }
   //memManagerFreeAll ();
   
   return (TRUE);
}


int doTopology(char *output, char ***data)
{
   register double ***dots, ***dots3D;
   register STORY ***histoire;
   register int x, y, z;
   int xMax, yMax, zMax;
   int numEvents = (int) countObjects(NULL_WIN);
   double height;
   double blockSize;
   double xLoc, yLoc, zLoc;
   int dx,dy,dz;
   FILE *foutx,*fouty,*foutz;
   char fnamex[250],fnamey[250],fnamez[250],fnamei[250];
   unsigned char h1[ARRAY_LENGTH_OF_STRAT_CODE],h2[ARRAY_LENGTH_OF_STRAT_CODE];
   int index1,index2;
   unsigned int flavor1,flavor2;
   STRATIGRAPHY_OPTIONS *stratOptions;
   int strat_code1, layer_code1, strat_code2, layer_code2, dum_flav, dum_index;
   double xdum,ydum;
   
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   xLoc = viewOptions->originX;
   yLoc = viewOptions->originY;
   zLoc = viewOptions->originZ - viewOptions->lengthZ;
   
   blockSize = viewOptions->geophysicsCubeSize;

   xMax = (int) (viewOptions->lengthX / blockSize) + 1;
   yMax = (int) (viewOptions->lengthY / blockSize) + 1;
   zMax = (int) (viewOptions->lengthZ / blockSize) + 1;
   
   initLongJob (0, zMax + zMax*yMax*(numEvents-1) + xMax*yMax,
                "Calculating 3D Stratigraphy", NULL);

   if ((dots = (double ***) qdtrimat(0,yMax,0,xMax,0,3))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         xvt_dm_post_error("Not enough memory, try closing some windows");
      return (0);
   }
   if ((histoire = (STORY ***) qdtristrsmat(0,zMax,0,yMax,0,xMax))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         xvt_dm_post_error("Not enough memory, try closing some windows");
      freeqdtrimat(dots,0,zMax,0,yMax,0,xMax);
      return (0);
   }
   if ((dots3D = (double ***) qdtrimat(0,zMax,0,yMax,0,xMax))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Error, Not enough Memory");
      else
         xvt_dm_post_error("Error, Not enough Memory");
      freeqdtrimat(dots,0,zMax,0,yMax,0,xMax);
      free_qdtristrsmat(histoire,0,zMax,0,yMax,0,xMax);
      return (0);
   }

   for (z = 0, height = zLoc; z < zMax; z++, height += blockSize)
   {
      incrementLongJob (INCREMENT_JOB);
      for (y = 0; y < yMax; y++)
      {
         for (x = 0; x < xMax; x++)
         {
            dots[y+1][x+1][1] = x*blockSize + xLoc+0.000001; /* mwj_fix */
            dots[y+1][x+1][2] = (yMax-1-y)*blockSize + yLoc+0.000001; /* mwj_fix */
            dots[y+1][x+1][3] = height+0.000001; /* mwj_fix */

            histoire[z+1][y+1][x+1].again=1;
            izero(histoire[z+1][y+1][x+1].sequence);
         }
      }
      reverseEvents (dots, histoire[z+1], yMax, xMax);

      for (y = 0; y < yMax; y++)
      {
         for (x = 0; x < xMax; x++)
         {
            dots3D[z+1][y+1][x+1] = dots[y+1][x+1][3]; /* Z location */
         }
      }
   }
	 sprintf((char *)fnamex,"%s._x",(char *) output);
	 foutx=fopen(fnamex,"w");
	 sprintf((char *)fnamey,"%s._y",(char *) output);
	 fouty=fopen(fnamey,"w");
	 sprintf((char *)fnamez,"%s._z",(char *) output);
	 foutz=fopen(fnamez,"w");
	 
   for (z = zMax-1; z > 0; z--)
   {
      incrementLongJob (INCREMENT_JOB);
      for (x = 1; x < xMax; x++)
      {
      		for (y = 1; y < yMax; y++)
      		{
         	  
         	  iequal((unsigned char *) &h1[0],(unsigned char *) &(histoire[z][y][x].sequence[0]));
         	  iequal((unsigned char *) &h2[0],(unsigned char *) &(histoire[z][y][x+1].sequence[0]));										
						dx=(int) lastdiff((unsigned char *) &h1[0],(unsigned char *) &h2[0]);
           	  fprintf(foutx,"%d\t",dx);
         	  
          	iequal((unsigned char *) &h1[0],(unsigned char *) &(histoire[z][y][x].sequence[0]));
         	  iequal((unsigned char *) &h2[0],(unsigned char *) &(histoire[z][y+1][x].sequence[0]));
						dy=(int) lastdiff((unsigned char *) &h1[0],(unsigned char *) &h2[0]);

						if (dy==0 && flavor1 !=flavor2)
							dy=-1;
          	fprintf(fouty,"%d\t",dy);
        	  
         	  iequal((unsigned char *) &h1[0],(unsigned char *) &(histoire[z][y][x].sequence[0]));
         	  iequal((unsigned char *) &h2[0],(unsigned char *) &(histoire[z+1][y][x].sequence[0]));

						dz=(int) lastdiff((unsigned char *) &h1[0],(unsigned char *) &h2[0]);
						if (dz==0 && flavor1 != flavor2)
							dy=-1;
          	fprintf(foutz,"%d\t",dz);
        }
        fprintf(foutx,"\n");
        fprintf(fouty,"\n");
        fprintf(foutz,"\n");
      }
   }
   fclose(foutx);
   fclose(fouty);
   fclose(foutz);
   
   freeqdtrimat(dots,0,yMax,0,xMax,0,3);
   free_qdtristrsmat(histoire,0,zMax,0,yMax,0,xMax);
   freeqdtrimat(dots3D,0,zMax,0,yMax,0,xMax);


}

/*
======================================================================
FUNCTION        checkMemManagerDataSize
DESCRIPTION
   add a array to the memory manager
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
checkMemManagerDataSize (int dataSizeNeeded)
#else
checkMemManagerDataSize (dataSizeNeeded)
int dataSizeNeeded;
#endif
{
   MEM_MANAGER_RECORD *tempData;
   int newDataSize;

   if (dataSizeNeeded > sizeMemManagerData)
   {
      newDataSize = sizeMemManagerData + MEM_MANAGER_RECORD_INCREMENT;
      tempData = (MEM_MANAGER_RECORD *) xvt_mem_alloc (newDataSize*sizeof(MEM_MANAGER_RECORD));
      if (!tempData)
         return (FALSE);
      
      if (memManagerData)
      {
         memcpy ((char *) tempData, (char *) memManagerData,
                       numMemManagerData*sizeof(MEM_MANAGER_RECORD));
         xvt_mem_free ((char *) memManagerData);
      }       
      memManagerData = tempData;
      sizeMemManagerData = newDataSize;
   }
   
   return (TRUE);
}

/*
======================================================================
FUNCTION        memManagerAdd
DESCRIPTION
   add a array to the memory manager
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
memManagerAdd (char *array, int numDimensions, int dim1, int dim2, int dim3,
               int **iregDims)
#else
memManagerAdd (array, numDimensions, dim1, dim2, dim3, iregDims)
char *array;
int numDimensions, dim1, dim2, dim3, **iregDims;
#endif
{
   if (!checkMemManagerDataSize (numMemManagerData+1))
      return (FALSE);
   
   memManagerData[numMemManagerData].data = array;
   memManagerData[numMemManagerData].numDimensions = numDimensions;
   memManagerData[numMemManagerData].dim1 = dim1;
   memManagerData[numMemManagerData].dim2 = dim2;
   memManagerData[numMemManagerData].dim3 = dim3;
   memManagerData[numMemManagerData].iregDimensions = iregDims;
   numMemManagerData++;
      
   return (TRUE);
}

/*
======================================================================
FUNCTION        memManagerAddLabel
DESCRIPTION
   add a array to the memory manager
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
memManagerAddLabel (char *label)
#else
memManagerAddLabel (label)
char *label;
#endif
{
   int length;
   char *text;
   
   length = strlen (label) + 1;
   text = xvt_mem_alloc (length);
   if (!text)
      return (FALSE);
      
   strcpy (text, label);
                                           /* -1 signify a label */
   if (!memManagerAdd (text, 1, length, -1, -1, NULL))
   {
      xvt_mem_free (text);  /* failed to add so free mem */
      return (FALSE);
   }
   else
      return (TRUE);
}

/*
======================================================================
FUNCTION        memManagerRemove
DESCRIPTION
   remove an array from the memory manager
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
memManagerRemove (char *array)
#else
memManagerRemove (array)
char *array;
#endif
{
   int i, found = FALSE;
   
   if (array == NULL)
      return (FALSE);
      
        /* search backwards until we find the array then free it */
   for (i = numMemManagerData-1; i >= 0; i--)
   {
      if (memManagerData[i].data == array)
      {
         memManagerData[i].data = NULL;   /* Zero the record */
         memManagerData[i].numDimensions = 0;
         memManagerData[i].dim1 = -1;
         memManagerData[i].dim2 = -1;
         memManagerData[i].dim3 = -1;
         found = TRUE;
      }
   }
   
   if (found)
   {
          /* move back down the array untill we find a filled spot
          ** This is the new number of records in the memManager.   */
      while (numMemManagerData && (memManagerData[numMemManagerData-1].data == NULL))
         numMemManagerData--;
   }
   
   return (found);
}

/*
======================================================================
FUNCTION        memManagerFree
DESCRIPTION
   remove and free an array from the memory manager
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
memManagerFree (char *array)
#else
memManagerFree (array)
char *array;
#endif
{
   int i, found = FALSE;
   
   if (array == NULL)
      return (FALSE);
      
        /* search backwards until we find the array then free it */
   for (i = numMemManagerData-1; i >= 0; i--)
   {
      if (memManagerData[i].data == array)
      {
         found = TRUE;
         switch (memManagerData[i].numDimensions)
         {
            case (1):
               xvt_mem_free (memManagerData[i].data);
               break;
            case (2):
               destroy2DArray((char **) memManagerData[i].data,
                                                 memManagerData[i].dim1,
                                                 memManagerData[i].dim2);
               break;
            case (3):
               destroy3DArray((char ***) memManagerData[i].data,
                                                 memManagerData[i].dim1,
                                                 memManagerData[i].dim2,
                                                 memManagerData[i].dim3);
               break;
            case (-1):
               destroy3DIregArray((char ***) memManagerData[i].data,
                                       memManagerData[i].dim1,
                                       memManagerData[i].iregDimensions);
               break;
            default:
               xvt_dm_post_error ("Sorry, Can't remove Array from Mem Manager");
         }
         break;
      }
   }
   
   if (found)   /* Zero the record */
   {
      memManagerData[i].data = NULL;
      memManagerData[i].numDimensions = 0;
      memManagerData[i].dim1 = -1;
      memManagerData[i].dim2 = -1;
      memManagerData[i].dim3 = -1;
          /* move back down the array untill we find a filled spot
          ** This is the new number of records in the memManager.   */
      while (numMemManagerData && (memManagerData[numMemManagerData-1].data == NULL))
         numMemManagerData--;
   }
   
   return (found);
}


/*
======================================================================
FUNCTION        memManagerFreeLabel
DESCRIPTION
   remove a label from the memory manager
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
memManagerFreeLabel (char *label)
#else
memManagerFreeLabel (label)
char *label;
#endif
{
   int i;

   for (i = numMemManagerData-1; i >= 0; i--)
   {                           /* Match the label */
      if ((memManagerData[i].numDimensions == 1) 
               && (memManagerData[i].dim2 == -1)
               && (memManagerData[i].dim3 == -1)
               && (strcmp (label, memManagerData[i].data) == 0))
      {                                   /* Remove the label */
         return (memManagerFree (memManagerData[i].data));
      }
   }

   return (FALSE);
}


/*
======================================================================
FUNCTION        memManagerFreeAfterLabel
DESCRIPTION
   free memory associated with array and remove from the memory manager
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
memManagerFreeAfterLabel (char *label)
#else
memManagerFreeAfterLabel (label)
char *label;
#endif
{
   int i, j;

   for (i = numMemManagerData-1; i >= 0; i--)
   {                           /* Match the label */
      if ((memManagerData[i].numDimensions == 1) 
               && (memManagerData[i].dim2 == -1)
               && (memManagerData[i].dim3 == -1)
               && (strcmp (label, memManagerData[i].data) == 0))
      {                        /* Remove the label and all after */
         for (j = i; j < numMemManagerData; j++)
            memManagerFree (memManagerData[j].data);
            
         return (TRUE);
      }
   }

   return (FALSE);
}

/*
======================================================================
FUNCTION        memManagerFreeAll
DESCRIPTION
   free memory associated with all arrays and remove from the memory manager
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
memManagerFreeAll ()
#else
memManagerFreeAll ()
#endif
{
   int i;
   OBJECT *object, *nextObject;
   BLOCK_VIEW_OPTIONS *blockView;

   for (i = numMemManagerData-1; i >= 0; i--)
      memManagerFree (memManagerData[i].data);
      
   if (memManagerData)
      xvt_mem_free ((char *) memManagerData);
   memManagerData = (MEM_MANAGER_RECORD *) NULL;
   sizeMemManagerData = 0;
   numMemManagerData = 0;
   
   if (batchExecution)
   {
      if (topographyMap)
      {
         free_dmatrix (topographyMap, 0, TopoCol, 0, TopoRow);
         topographyMap = NULL;
      }
                      
      freeRockPropDatabase ();
      
                 /* Free memory associated with history */
      for (object = nthObject(NULL_WIN, 0); object != NULL; object = nextObject)
      {
         nextObject = object->next;
         freeObjectMemory (object);
      }
      
                         /* Free All the block Views */
      if (blockView = getViewOptions ())
      {
         BLOCK_VIEW_OPTIONS *nextView;
                         
         while (blockView->prev)  /* move to start */
            blockView = blockView->prev;
                   
         while (blockView)
         {
            nextView = blockView->next;
            xvt_mem_free ((char *) blockView);
            blockView = nextView;
         }
         blockViewOptions = NULL;
      }   
                      
      if (magneticsReferenceData)
      {
         destroy2DArray ((char **) magneticsReferenceData->imageData,
                                   magneticsReferenceData->dataYSize,
                                   magneticsReferenceData->dataXSize);
         xvt_mem_free ((char *) magneticsReferenceData);
         magneticsReferenceData = NULL;
      }
               
      if (gravityReferenceData)
      {
         destroy2DArray ((char **) gravityReferenceData->imageData,
                                   gravityReferenceData->dataYSize,
                                   gravityReferenceData->dataXSize);
         xvt_mem_free ((char *) gravityReferenceData);
         gravityReferenceData = NULL;
      }
   }
   
   return (TRUE);
}

/*
======================================================================
FUNCTION        buildUserList
DESCRIPTION
   Build an array of user Names
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
buildUserList (char **userList, int *numUser,
               char **encoding, int *numEncodings)
#else
buildUserList (userList, numUser, encoding, numEncodings)
char **userList;
int *numUser;
char **encoding;
int *numEncodings;
#endif
{
   int i;
   
   *numUser = 0;
                                  /****** Us - The Author ******/
   strcpy ((char *) userList[*numUser], "Monash University"); (*numUser)++;

                                  /****** Companies ******/
   strcpy ((char *) userList[*numUser], "AMIRA"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "Aberfoyle"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "AGSO"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "BHP Co Ltd"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "Billiton Exploration"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "CRA Exploration Pty Ltd"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "Geopeko"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "MIM Exploration Pty Ltd"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "Newcrest Mining Limited"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "Pasminco Exploration"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "RGC Exploration Pty Ltd"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "RTZ Ltd"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "Sumitomo Metal Mining Oceania"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "Western Mining Corporation Ltd"); (*numUser)++;
                                  /****** Universities ******/
   strcpy ((char *) userList[*numUser], "LaTrobe University"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "Melbourne University"); (*numUser)++;
                                  /****** CSIRO Departments ******/
   strcpy ((char *) userList[*numUser], "CSIRO DIT"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "CSIRO DEM"); (*numUser)++;
                                  /****** Individuals ******/
   strcpy ((char *) userList[*numUser], "L Harris: UWA"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "P James: Adelaide Uni"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "R Holcombe: Uni of Queensland"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "N Mancktelow: ETH"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "W Means: SUNYA"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "M Lackie: Macquarie"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "K McClay: Royal Holloway"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "D DePaor: GWU"); (*numUser)++;
                                  /****** Other Special Accounts ******/
   strcpy ((char *) userList[*numUser], "Unlimited Demostration"); (*numUser)++;
   strcpy ((char *) userList[*numUser], "A Demostration"); (*numUser)++;  /* January */
   strcpy ((char *) userList[*numUser], "B Demostration"); (*numUser)++;  /* Febuary */
   strcpy ((char *) userList[*numUser], "C Demostration"); (*numUser)++;  /* March */
   strcpy ((char *) userList[*numUser], "D Demostration"); (*numUser)++;  /* April */
   strcpy ((char *) userList[*numUser], "E Demostration"); (*numUser)++;  /* May */
   strcpy ((char *) userList[*numUser], "F Demostration"); (*numUser)++;  /* June */
   strcpy ((char *) userList[*numUser], "G Demostration"); (*numUser)++;  /* July */
   strcpy ((char *) userList[*numUser], "H Demostration"); (*numUser)++;  /* August */
   strcpy ((char *) userList[*numUser], "I Demostration"); (*numUser)++;  /* September */
   strcpy ((char *) userList[*numUser], "J Demostration"); (*numUser)++;  /* October */
   strcpy ((char *) userList[*numUser], "K Demostration"); (*numUser)++;  /* November */
   strcpy ((char *) userList[*numUser], "L Demostration"); (*numUser)++;  /* December */

   strcpy ((char *) userList[*numUser], "M Demostration"); (*numUser)++;  /* January */
   strcpy ((char *) userList[*numUser], "N Demostration"); (*numUser)++;  /* Febuary */
   strcpy ((char *) userList[*numUser], "O Demostration"); (*numUser)++;  /* March */
   strcpy ((char *) userList[*numUser], "P Demostration"); (*numUser)++;  /* April */
   strcpy ((char *) userList[*numUser], "Q Demostration"); (*numUser)++;  /* May */
   strcpy ((char *) userList[*numUser], "R Demostration"); (*numUser)++;  /* June */
   strcpy ((char *) userList[*numUser], "S Demostration"); (*numUser)++;  /* July */
   strcpy ((char *) userList[*numUser], "T Demostration"); (*numUser)++;  /* August */
   strcpy ((char *) userList[*numUser], "U Demostration"); (*numUser)++;  /* September */
   strcpy ((char *) userList[*numUser], "V Demostration"); (*numUser)++;  /* October */
   strcpy ((char *) userList[*numUser], "W Demostration"); (*numUser)++;  /* November */
   strcpy ((char *) userList[*numUser], "X Demostration"); (*numUser)++;  /* December */

                                  /****** Machine Specific ******/
   strcpy ((char *) userList[*numUser], "Machine Specific"); (*numUser)++;

   i = 0;        /* each letter MUST only appear ONCE on a side */
   encoding[i][0] = ':';        encoding[i][1] = 'z';      i++;
   encoding[i][0] = ' ';        encoding[i][1] = 'n';      i++;
   encoding[i][0] = 'a';        encoding[i][1] = 'Z';      i++;
   encoding[i][0] = 'b';        encoding[i][1] = 'y';      i++;
   encoding[i][0] = 'c';        encoding[i][1] = 'X';      i++;
   encoding[i][0] = 'd';        encoding[i][1] = 'w';      i++;
   encoding[i][0] = 'e';        encoding[i][1] = 'V';      i++;
   encoding[i][0] = 'f';        encoding[i][1] = 'u';      i++;
   encoding[i][0] = 'g';        encoding[i][1] = 'T';      i++;
   encoding[i][0] = 'h';        encoding[i][1] = 's';      i++;
   encoding[i][0] = 'i';        encoding[i][1] = 'R';      i++;
   encoding[i][0] = 'j';        encoding[i][1] = 'q';      i++;
   encoding[i][0] = 'k';        encoding[i][1] = 'P';      i++;
   encoding[i][0] = 'l';        encoding[i][1] = 'o';      i++;
   encoding[i][0] = 'm';        encoding[i][1] = 'N';      i++;
   encoding[i][0] = 'n';        encoding[i][1] = 'm';      i++;
   encoding[i][0] = 'o';        encoding[i][1] = 'L';      i++;
   encoding[i][0] = 'p';        encoding[i][1] = 'k';      i++;
   encoding[i][0] = 'q';        encoding[i][1] = 'J';      i++;
   encoding[i][0] = 'r';        encoding[i][1] = 'i';      i++;
   encoding[i][0] = 's';        encoding[i][1] = 'H';      i++;
   encoding[i][0] = 't';        encoding[i][1] = 'g';      i++;
   encoding[i][0] = 'u';        encoding[i][1] = 'F';      i++;
   encoding[i][0] = 'v';        encoding[i][1] = 'e';      i++;
   encoding[i][0] = 'w';        encoding[i][1] = 'D';      i++;
   encoding[i][0] = 'x';        encoding[i][1] = 'c';      i++;
   encoding[i][0] = 'y';        encoding[i][1] = 'B';      i++;
   encoding[i][0] = 'z';        encoding[i][1] = 'a';      i++;
   encoding[i][0] = 'A';        encoding[i][1] = ':';      i++;
   encoding[i][0] = 'B';        encoding[i][1] = 'Y';      i++;
   encoding[i][0] = 'C';        encoding[i][1] = 'x';      i++;
   encoding[i][0] = 'D';        encoding[i][1] = 'W';      i++;
   encoding[i][0] = 'E';        encoding[i][1] = 'v';      i++;
   encoding[i][0] = 'F';        encoding[i][1] = 'U';      i++;
   encoding[i][0] = 'G';        encoding[i][1] = 't';      i++;
   encoding[i][0] = 'H';        encoding[i][1] = 'S';      i++;
   encoding[i][0] = 'I';        encoding[i][1] = 'r';      i++;
   encoding[i][0] = 'J';        encoding[i][1] = 'Q';      i++;
   encoding[i][0] = 'K';        encoding[i][1] = 'p';      i++;
   encoding[i][0] = 'L';        encoding[i][1] = 'O';      i++;
   encoding[i][0] = 'M';        encoding[i][1] = ' ';      i++;
   encoding[i][0] = 'N';        encoding[i][1] = 'M';      i++;
   encoding[i][0] = 'O';        encoding[i][1] = 'l';      i++;
   encoding[i][0] = 'P';        encoding[i][1] = 'K';      i++;
   encoding[i][0] = 'Q';        encoding[i][1] = 'j';      i++;
   encoding[i][0] = 'R';        encoding[i][1] = 'I';      i++;
   encoding[i][0] = 'S';        encoding[i][1] = 'h';      i++;
   encoding[i][0] = 'T';        encoding[i][1] = 'G';      i++;
   encoding[i][0] = 'U';        encoding[i][1] = 'f';      i++;
   encoding[i][0] = 'V';        encoding[i][1] = 'E';      i++;
   encoding[i][0] = 'W';        encoding[i][1] = 'd';      i++;
   encoding[i][0] = 'X';        encoding[i][1] = 'C';      i++;
   encoding[i][0] = 'Y';        encoding[i][1] = 'b';      i++;
   encoding[i][0] = 'Z';        encoding[i][1] = 'A';      i++;
   
   *numEncodings = i;

   return (TRUE);
}

/*
======================================================================
FUNCTION        checkLicence
DESCRIPTION
   check the licence file and set appropriate global variables
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
checkLicence ()
#else
checkLicence ()
#endif
{
   FILE_SPEC licenceFile;
   FILE *fi;
   char lineIn[100], readableName[100], validLicenceCode[100],*linePtr;
   int validLicence = FALSE;
   int i, index, pos, numUser, numEncoding;
   char **userList, **encoding;
   char userName[100];
   time_t longTime, createTime;
   struct tm *today;
   int updateFile = FALSE;
   int destroyFile = FALSE;
   
   if (!(userList = (char **) create2DArray (100, 100, sizeof(char *))))
      return (FALSE);
   if (!(encoding = (char **) create2DArray (100, 2, sizeof(char *))))
   {
      destroy2DArray (userList, 100, 100);
      return (FALSE);
   }
   buildUserList (userList, &numUser, encoding, &numEncoding);


   xvt_fsys_get_default_dir (&(licenceFile.dir));
   strcpy (licenceFile.name, LICENCE_FILE_NAME);
   if (findFile (&licenceFile))
   {
      xvt_fsys_set_dir (&(licenceFile.dir));
      createTime = xvt_fsys_get_file_attr (&licenceFile, XVT_FILE_ATTR_CTIME);
      if (fi = (FILE *) fopen (licenceFile.name, "r"))
      {
         fgets (readableName, 100, fi);   /* Read in the name at start of Licence */
         while (fgets (lineIn, 100, fi))   /* Read in the licence */
         {                       /* see if a code is in this line */
            if (linePtr = (char *) strchr (lineIn, 'W'))
            {
               linePtr++;
               pos = 0;         /* Extract the name */
               while (*linePtr != '\0')
               {  
                  for (index = 0; index < numEncoding; index++)
                  {
                     if (encoding[index][1] == *linePtr)
                     {
                        userName[pos++] = encoding[index][0];
                        break;
                     }
                  }
                  linePtr++;
               }
               userName[pos] = '\0';  /* make sure the decoded name is termated */
               
                            /* Check through to see if we have a match */
               for (i = 0; i < numUser; i++)
               {
                  if (strstr (userName, userList[i]))
                  {                      /* make sure demo in time limit */
                     if (strstr (userList[i], "Demostration"))
                     {
                        DEMO_VERSION = TRUE;
                        updateFile = TRUE;
                        strcpy(validLicenceCode, lineIn);
                        if (strstr(userList[i], "Unlimited"))
                        {
                           validLicence = TRUE;
                           strcpy(registrationID, "DEMOSTRATION");
                        }
                        else  /* check range for demo */
                        {
                           int endMonth, endYear = 99;
									char *yearPos;

									yearPos = strstr(userName, "YEAR"); /* eg YEAR101  = Year 2001 ie 1900 + 101 years */
									if (yearPos)
									{
										endYear = 0;
										endYear = (yearPos[4] - 'A')*100;
										endYear += (yearPos[5] - 'A')*10;
										endYear += (yearPos[5] - 'A');
									}
                                                   /* month licenced was from */
                           endMonth = userList[i][0] - 'A' + 1;
									if (endMonth > 12)  /* Demostrations that are Fully enabled */
									{
										endMonth -= 12;
		                        DEMO_VERSION = FALSE;
									}

                           time(&longTime); /* current Time */
                           today = localtime(&longTime);
                           
                                /* If they set back the system clock then Fail */
                           if (createTime > longTime)
                              validLicence = FALSE;
                           else    /* not valid if more than 1 month from licence month */
                           {
                              if (today->tm_year <= endYear)
                              {
                                 if (endMonth >= today->tm_mon)
                                 {
                                    validLicence = TRUE;
                                    strcpy(registrationID, "Demostration");
                                 }
                              }
                           }
                        }
                        if (!validLicence)
                           destroyFile = TRUE;
                        
                     }
                     else if (strstr (userList[i], "Machine Specific"))
                     {
                        char uniqueId[256];
                        
                        if (getMachineUniqueLicence (uniqueId))
                        {
                                     /* Encode the Unique ID Twice */
                           linePtr = uniqueId;          
                           while (*linePtr != '\0')
                           {  
                              for (index = 0; index < numEncoding; index++)
                              {
                                 if (encoding[index][0] == *linePtr)
                                 {
                                    *linePtr = encoding[index][1];
                                    break;
                                 }
                              }
                              linePtr++;
                           }
                                      /* The second encoding of the already encoded id */
                           linePtr = uniqueId;
                           while (*linePtr != '\0')
                           {  
                              for (index = 0; index < numEncoding; index++)
                              {
                                 if (encoding[index][0] == *linePtr)
                                 {
                                    *linePtr = encoding[index][1];
                                    break;
                                 }
                              }
                              linePtr++;
                           }
                                     /* See if that ID appears in this line */
                           if (strstr (lineIn, uniqueId))
                           {
                              validLicence = TRUE;
                              strcpy (registrationID, "this machine for education use only");
                           }
                        }
                     }
                     else
                     {
                        validLicence = TRUE;
                        strcpy(registrationID, userList[i]);
                     }
                     break;
                  }
               }
            }
         }
         fclose (fi);
      }
   }   

   if (updateFile)  /* Update The licence File so we can check dates */
   {
      char *ptr;
                /* Write out real licence again so time of file changed */
      if (ptr = (char *) strchr(readableName, '\n'))
         *ptr = '\0';
      if (ptr = (char *) strchr(validLicenceCode, '\n'))
         *ptr = '\0';
      writeLicenceFile (LICENCE_FILE_NAME, readableName, validLicenceCode);
   }
   
   if (destroyFile) /* Licence Expired so quietly overwrite the licence to avoid temptation */
   {
      char *ptr;
                /* Write out some junk to look like a licence */
      if (ptr = (char *) strchr(readableName, '\n'))
         *ptr = '\0';
      writeLicenceFile (LICENCE_FILE_NAME, readableName,
                        "HOvpsTPFKaA JsSijSauCufQLeybrTGHJUsjcTaOgmxrvvuViInaSewkkYJcHIkLLALOxRMIgafnAxZ");
      validLicence = FALSE;
   }

   destroy2DArray (userList, 100, 100);
   destroy2DArray (encoding, 100, 2);

   if (!validLicence)
   {
      if (batchExecution)
      {
         fprintf (stderr, "Sorry, You do no have a valid Licence File.\n");
         fprintf (stderr, "Please Contact Your NODDY distributer.\n");
      }
      else
      {
         xvt_dm_post_error ("Sorry, You do no have a valid Licence File.\n");
      }
      return (FALSE);
   }
   else
   {
      return (TRUE);
   }
}

int
#if XVT_CC_PROTO
writeLicenceFile (char *filename, char *company, char *code)
#else
writeLicenceFile (filename, company, code)
char *filename, *company, *code;
#endif
{
   char licenceFile[100];
   FILE *fo;
   int i, index, numEncoding, numUser, uniqueMachineId;
   double randNum, length;
   char **userList, **encoding, lineOut[100], *uniqueIdPrefix;
   
   if (!(userList = (char **) create2DArray (100, 100, sizeof(char *))))
      return (FALSE);
   if (!(encoding = (char **) create2DArray (100, 2, sizeof(char *))))
   {
      destroy2DArray (userList, 100, 100);
      return (FALSE);
   }
   buildUserList (userList, &numUser, encoding, &numEncoding);

   if (uniqueIdPrefix = strstr (code, LICENCE_KEY))
   {
      uniqueMachineId = TRUE;
      *uniqueIdPrefix = '\0';  /* Remove the prefix for encoding purposes */
   }
   else
      uniqueMachineId = FALSE;
   
   strcpy (licenceFile, filename);
   
   if (!(fo = fopen (licenceFile, "w")))
   {
      destroy2DArray (userList, 100, 100);
      destroy2DArray (encoding, 100, 2);
      return (FALSE);
   }

   fprintf (fo, "%s\n", company);

                /* Create a line full of junk first */
   for (i = 0; i < 80; i++)
   {
      randNum = (double) rand();
      randNum = (randNum/RAND_MAX) * numEncoding;
      lineOut[i] = (char) encoding[(int) randNum][1];
   }
   lineOut[i] = '\0';
   

   fprintf (fo, "%s\n", lineOut);

   if (uniqueMachineId)
   {
      sprintf (lineOut, "WMachine Specific");  /* so we know it is machine specific code */
      strcat (lineOut, code);  /* Add the machine Specific identification */
      length = strlen (lineOut);
        /* Start from 1 so we dont change the 'W' which is our secret start code character */
      for (i = 1; i < length; i++) 
      {                           
         for (index = 0; index < numEncoding; index++)
         {
            if (encoding[index][0] == lineOut[i])
            {
               lineOut[i] = encoding[index][1];
               break;
            }
         }
      }
      fprintf (fo, "%s\n", lineOut);
   }
   else          /* We mailed them the right ID so just write it out */
      fprintf (fo, "%s\n", code);

   fclose (fo);

   destroy2DArray (userList, 100, 100);
   destroy2DArray (encoding, 100, 2);
   
   return (TRUE);
}

int createLicence (userNum)
int userNum;
{
   char licenceFile[100];
   FILE *fo;
   char lineOut[100], *linePtr;
   int i, index, numUser, numEncoding, length;
   double randNum;
   char **userList, **encoding;
   char userName[100];

   if (!(userList = (char **) create2DArray (100, 100, sizeof(char *))))
      return (FALSE);
   if (!(encoding = (char **) create2DArray (100, 2, sizeof(char *))))
   {
      destroy2DArray (userList, 100, 100);
      return (FALSE);
   }

   buildUserList (userList, &numUser, encoding, &numEncoding);
   
   strcpy (userName, userList[userNum]);
   length = strlen(userName);

   strcpy (licenceFile, LICENCE_FILE_NAME);
   
   if (!(fo = fopen (licenceFile, "w")))
   {
      destroy2DArray (userList, 100, 100);
      destroy2DArray (encoding, 100, 2);
      fprintf(stderr, "Cannot open %s.\n", licenceFile);
      return (FALSE);
   }
                /* Write Out the name in a readable format */
   fprintf (fo, "%s\n", userName);

                /* Create a line full of junk first */
   for (i = 0; i < 80; i++)
   {
      randNum = (double) rand();
      randNum = (randNum/RAND_MAX) * numEncoding;
      lineOut[i] = (char) encoding[(int) randNum][1];
   }
   lineOut[i] = '\0';
   
   fprintf (fo, "%s\n", lineOut);
            
                /* Few more character of junk then the code */
   for (i = 0; i < 8; i++)
   {
      randNum = (double) rand();
      randNum = (randNum/RAND_MAX) * numEncoding;
      lineOut[i] = (char) encoding[(int) randNum][1];
   }
   lineOut[i] = 'W';
   lineOut[i+1] = '\0';
   linePtr = &(lineOut[i+1]);
                /* The actual Code showing the users registration name */
   for (i = 0; i < length; i++)
   {
      for (index = 0; index < numEncoding; index++)
      {
         if (encoding[index][0] == userList[userNum][i])
         {
            linePtr[i] = encoding[index][1];
            break;
         }
      }
   }
   linePtr[i] = '\0';
                /* Pad out with junk until 80 characters */
   length = strlen (lineOut);
   for (i = length; i < 80; i++)
   {
      randNum = (double) rand();
      randNum = (randNum/RAND_MAX) * numEncoding;
      lineOut[i] = (char) encoding[(int) randNum][1];
   }
   lineOut[i] = '\0';

   fprintf (fo, "%s\n", lineOut);

   fclose (fo);

   destroy2DArray (userList, 100, 100);
   destroy2DArray (encoding, 100, 2);

   return (TRUE);
}

int createLicenceInfoFile ()
{
   char licenceFile[100];
   char licenceInfo[256];
   FILE *fo;
   char lineOut[100];
   int i, index, numUser, numEncoding, length;
   double randNum;
   char **userList, **encoding;
   
   if (!(userList = (char **) create2DArray (100, 100, sizeof(char *))))
      return (FALSE);
   if (!(encoding = (char **) create2DArray (100, 2, sizeof(char *))))
   {
      destroy2DArray (userList, 100, 100);
      return (FALSE);
   }
   buildUserList (userList, &numUser, encoding, &numEncoding);

   if (!getMachineUniqueLicence (licenceInfo))
   {
      destroy2DArray (encoding, 100, 2);
      return (FALSE);
   }

   strcpy (licenceFile, LICENCE_FILE_NAME);
   addFileExtention (licenceFile, ".txt");
   
   if (!(fo = fopen (licenceFile, "w")))
   {
      destroy2DArray (userList, 100, 100);
      destroy2DArray (encoding, 100, 2);
      return (FALSE);
   }
                /* Write Out the name in a readable format */
   fprintf (fo, "Noddy Licence Information File\n");

                /* Create a line full of junk first */
   for (i = 0; i < 80; i++)
   {
      randNum = (double) rand();
      randNum = (randNum/RAND_MAX) * numEncoding;
      lineOut[i] = (char) encoding[(int) randNum][1];
   }
   lineOut[i] = '\0';
   
   fprintf (fo, "%s\n", lineOut);     /* write out one junk line */
            

                /* The actual Code showing the users registration name */
   length = strlen(licenceInfo);
   for (i = 0; i < length; i++)
   {
      for (index = 0; index < numEncoding; index++)
      {
         if (encoding[index][0] == licenceInfo[i])
         {
            lineOut[i] = encoding[index][1];
            break;
         }
      }
   }
   lineOut[i] = '\0';
                /* Pad out with junk until 80 characters */
   length = strlen (lineOut);
   for (i = length; i < 80; i++)
   {
      randNum = (double) rand();
      randNum = (randNum/RAND_MAX) * numEncoding;
      lineOut[i] = (char) encoding[(int) randNum][1];
   }
   lineOut[i] = '\0';

   fprintf (fo, "%s\n", lineOut);

   fclose (fo);

   destroy2DArray (encoding, 100, 2);
   destroy2DArray (userList, 100, 100);

   return (TRUE);
}

int
#if XVT_CC_PROTO
getMachineUniqueLicence (char *uniqueId)
#else
getMachineUniqueLicence (uniqueId)
char *uniqueId;
#endif
{
   int length;
#if (XVTWS == MTFWS) || (XVTWS == XOLWS)
   int hostId;
   hostId = gethostid ();
   sprintf (uniqueId, "%d", hostId);
#endif

#if (XVTWS == MACWS)
   int TargetID;
   SCSIInstr TIB[2];
   OSErr myErr;
   char CMD[5];
   struct InquiryResponse Response;
   int Counter, pos;
   short RetStat;
   short RetMessage;
   int i;
   
   TIB[0].scOpcode = scNoInc;           /* { specify an SCNOINC instruction} */
   TIB[0].scParam1 = (long) &Response;  /* { pointer to buffer } */
   TIB[0].scParam2 = kInquirySize;      /* { number of bytes to move } */
   TIB[1].scOpcode = scStop;            /* { second TIB is stop instruction} */
   TIB[1].scParam1 = (long) NULL;       /* { unused TIB parameter } */
   TIB[1].scParam2 = (long) NULL;       /* { unused TIB parameter } */
   
   /* {set up the command buffer with the SCSI INQUIRY command} */
   CMD[0] = kInquiryCMD;   /* {opcode is Inquiry command} */
   CMD[1] = 0;             /* {reserved} */
   CMD[2] = 0;             /* {reserved} */
   CMD[3] = 0;             /* {reserved} */
   CMD[4] = kInquirySize;  /* {initially, 5 for Inquiry cmd} */
   CMD[5] = 0;             /* {reserved} */
   
   TargetID = kMySCSIID;   /* { here, the SCSI ID is a constant } */
   
   
   /* { Loop twice to inquire and read. The first time to obtain }
      { the value in byte 4 of the inquiry response record and the }
      { second time to read that additional amount. Obviously, if }
      { the arbitration and selection fails, the inquire and read }
      { cant occur. And if the inquire and read fails, theres }
      { nothing useful to print out. Notice that SCSIComplete is }
      { called only after a SCSI transaction. }*/

   for (i = 1; i < 3; i++) 
   {
      for (TargetID = 0; TargetID < 7; TargetID++)
      {
         DEBUG(printf("SCSI inquiry demo. Testing SCSI ID: %d\n", TargetID);)
         myErr = SCSIGet();
         if(myErr == noErr)
         { 
            myErr = SCSISelect(TargetID);
            if(myErr == noErr)
               break;
         }
      }
      
      if(myErr != noErr)
      {
         xvt_dm_post_error("Error, No SCSI devices found!");
         xvt_dm_post_error ("Please Contact Your Noddy Distributor");
         return(FALSE);
      }
         
      myErr = SCSICmd((Ptr) &CMD,6);
      if (myErr == noErr)
      {
         myErr = SCSIRead((Ptr) &TIB);
         if(myErr == noErr)
         {
            if(i == 1)
            {
               CMD[4] = CMD[4] + Response.AdditionalLength;
               TIB[0].scParam2 = TIB[0].scParam2 + Response.AdditionalLength;
            }
         }
      }
      
      myErr = SCSIComplete(&RetStat, &RetMessage, kWaitTime);
      if(myErr != noErr)
      {
         xvt_dm_post_error ("Error, Bad send or read command, err: %d", myErr);
         xvt_dm_post_error ("Please Contact Your Noddy Distributor");
         return (FALSE);
      }
   }

   
   pos = 0;
   /* printf("\nVendorID: "); */
   /* for(Counter = 0; Counter < kVendorIDSize; Counter++) */
   /*    uniqueId[pos++] = Response.VendorID[Counter]; */
      
   /* printf("\nProductID: "); */
   for(Counter = 0; Counter < kProductIDSize; Counter++)
      uniqueId[pos++] = Response.ProductID[Counter];

   /* printf("\nRevision: "); */
   for(Counter = 0; Counter < kRevisionSize; Counter++)
      uniqueId[pos++] = Response.Revision[Counter];

   /* printf("\nVendorUse2: "); */
   for(Counter = 0; Counter < 20; Counter++)
      uniqueId[pos++] = Response.VendorUse2[Counter];

   /* printf("\nReserved2: "); */
   /* for(Counter = 0; Counter < 42; Counter++) */
   /*    uniqueId[pos++] = Response.Reserved2[Counter]; */

   uniqueId[pos] = '\0';  /* terminate the string */
#endif

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   static unsigned long FirstSerialNumber = 0;
   struct MID 
   {
     int            midInfoLevel;
     unsigned long  midSerialNum;
     char           midVolLabel[11];
     char           midFileSysType[8];
   } MediaID, * pMediaID;
	unsigned long filenameLength, fileSystemFlags;
	char fileSystemName[20];

/*
   VOID FAR PASCAL DOS3Call(VOID);   // Use instead of INT 21h
   int nMajor;                       // MS-DOS major version
   int nMinor;                       // MS-DOS minor version, revision
   int nOEMNumber;                   // OEM serial number
   static char szUserMsg[80];        // holds user message
   _asm
     {
        mov   ax, 0x3000        ; Get MS-DOS version
        call  DOS3Call
        mov   nMajor, al        ; Save major number
        mov   nMinor, ah        ; Save minor version number
        mov   nOEMNumber, bh    ; Save OEM Serial number
     }
   wsprintf(szUserMsg,
            "Running on MS-DOS %d.%d OEM Serial Number %d",
            nMajor, nMinor, nOEMNumber);
   MessageBox(hWnd, szUserMsg, "MS-DOS Version", MB_OK);
*/
   if (!FirstSerialNumber)
   {
		int Drive = 3;
		char *tempPtr, *tempPtr2;
      MediaID.midInfoLevel = 0;
      pMediaID = &MediaID;
		tempPtr = (char *) pMediaID;
		tempPtr2= (char *) tempPtr+2;//&(pMediaID->midSerialNum);

#if (XVTWS == WIN32WS)
		if (GetVolumeInformation("c:", MediaID.midVolLabel, 11,
			                  &(MediaID.midSerialNum), &filenameLength,
									&fileSystemFlags, fileSystemName, 20))
			sprintf (uniqueId, "%s", MediaID.midVolLabel);
		else
			sprintf (uniqueId, "%ld%s", MediaID.midSerialNum, MediaID.midVolLabel);
      FirstSerialNumber = MediaID.midSerialNum;
#endif
#if (XVTWS == WIN16WS)
    _asm {
         mov bx, 3          /* 3=C drive */
         mov cx, 0866h     /* CH=08; CL=66 (Minor Code) */
         mov ds, WORD PTR [pMediaID+2]
         mov dx, WORD PTR [pMediaID]
         mov ax, 440Dh
         int 21h
      }
      sprintf (uniqueId, "%ld", MediaID.midSerialNum);
      FirstSerialNumber = MediaID.midSerialNum;
#endif
   }
   else
      sprintf (uniqueId, "%ld", FirstSerialNumber);

   DEBUG(xvt_dm_post_error ("Hard Drive C ID = %ld",FirstSerialNumber);)
   
#endif               

   length = strlen(uniqueId);
   while (length)
   {
      if (!isalpha(uniqueId[length-1]))
      {
         uniqueId[length-1] = 'A' + uniqueId[length-1]%25;
      }
      length--;
   }

   return (TRUE);
}

int
#if XVT_CC_PROTO
importLicenceFile (char *filename)
#else
importLicenceFile (filename)
char *filename;
#endif
{
   FILE *fi, *fo;
   int sucess = FALSE;
   char lineIn[256];
                 /* just copy the file to Noddylic.lic */
   if (fi = fopen (filename, "r"))
   {
      xvt_fsys_set_dir_startup ();
      if (fo = fopen (LICENCE_FILE_NAME, "w"))
      {
         while (fgets (lineIn, 255, fi))
         {
            fputs (lineIn, fo);
            sucess = TRUE;
         }
         fclose (fo);
      }
      fclose (fi);
   }
   
   return (sucess);
}


int
#if XVT_CC_PROTO
createLicenceFileFrom (char *filename)
#else
createLicenceFileFrom (filename)
char *filename;
#endif
{
   FILE *fi;
   int sucess = FALSE;
   char lineIn[256];
   char outputFile[256];
   char company[256], code[256];
   char *ptr;
                 /* just copy the file to Noddylic.lic */
   strcpy (outputFile, filename);
   addFileExtention(outputFile, ".lic");
   
   if (strcmp (filename, outputFile) == 0)
   {
      xvt_dm_post_error ("Error, Input and output names must be different");
      return (FALSE);
   }
   
   if (fi = fopen (filename, "r"))
   {
      fgets (company, 256, fi);
      fgets (lineIn, 256, fi);   /* Junk line with no info */
      fgets (code, 256, fi);
      
      fclose (fi);
   }

   if (ptr = (char *) strchr(company, '\n'))
      *ptr = '\0';
   strcat (code, LICENCE_KEY);
   writeLicenceFile (outputFile, company, code);
   
   return (sucess);
}


/*
======================================================================
FUNCTION        initProject
DESCRIPTION
   Initilise the memory and things associalted with the History and options to default values
INPUT   
OUTPUT
RETURNED
======================================================================
*/
int
#if XVT_CC_PROTO
initProject (int confirm, int factorySettings)
#else
initProject (confirm, factorySettings)
int confirm, factorySettings;
#endif
{
   BLOCK_VIEW_OPTIONS *blockView;
   int i;

   if (topographyMap)
   {
      free_dmatrix (topographyMap, 0, TopoCol, 0, TopoRow);
      topographyMap = NULL;
   }

   if (magneticsReferenceData)
   {
      destroy2DArray ((char **) magneticsReferenceData->imageData,
                              magneticsReferenceData->dataYSize,
                              magneticsReferenceData->dataXSize);
      xvt_mem_free ((char *) magneticsReferenceData);
      magneticsReferenceData = NULL;
   }
         
   if (gravityReferenceData)
   {
      destroy2DArray ((char **) gravityReferenceData->imageData,
                              gravityReferenceData->dataYSize,
                              gravityReferenceData->dataXSize);
      xvt_mem_free ((char *) gravityReferenceData);
      gravityReferenceData = NULL;
   }

   deleteSelectedObjects (getEventDrawingWindow(), TRUE);  /* Delte All events */

   if (!factorySettings)
   {
       loadHistoryFile (NODDY_DEFAULT_SETUP_FILE);
       setCurrentFileName (NULL);
   }
   else
   {                                                      /* Project Options */
      projectOptions.susceptibilityUnits = CGS;
      projectOptions.geophysicalCalc = CALC_GRAV_AND_MAG;
      projectOptions.calculationType = LOCAL_JOB;
      projectOptions.lengthScale = METRES;
      projectOptions.printScalingFactor = (double) 1.0;
      projectOptions.imageScalingFactor = (double) 10.0;

      projectOptions.gravityDisplay.clippingType = RELATIVE_CLIPPING;
      projectOptions.gravityDisplay.minClip = 0.0;
      projectOptions.gravityDisplay.maxClip = 100.0;
      projectOptions.gravityDisplay.display = GREY_COLOR;
      projectOptions.gravityDisplay.numContours = 25;

      projectOptions.magneticsDisplay.clippingType = RELATIVE_CLIPPING;
      projectOptions.magneticsDisplay.minClip = 0.0;
      projectOptions.magneticsDisplay.maxClip = 100.0;
      projectOptions.magneticsDisplay.display = GREY_COLOR;
      projectOptions.magneticsDisplay.numContours = 25;

      strcpy(projectOptions.internetAddress,"255.255.255.255");
      strcpy(projectOptions.accountName, "");
      strcpy(projectOptions.noddyPath,"./noddy");
      projectOptions.movieFramesPerEvent = 3;
      projectOptions.moviePlaySpeed = 10.0;
      projectOptions.movieType = BLOCK_DIAGRAM_MOVIE;
      projectOptions.newWindowEachStage = FALSE;

      projectOptions.easting = 0.0;
      projectOptions.northing = 0.0;

                                                          /* BlockView Options */
      do {          /* Delete all but one view */
         blockView = getViewOptions ();
      } while (deleteViewOptions (blockView)); 
      if (blockView = getViewOptions ())   /* assign it the default properties */
      {
         strcpy(blockView->viewName, DEFAULT_VIEW_NAME);
         blockView->originX = DEFAULT_VIEW_ORIGIN_X;
         blockView->originY = DEFAULT_VIEW_ORIGIN_Y;
         blockView->originZ = DEFAULT_VIEW_ORIGIN_Z;
         blockView->lengthX = DEFAULT_VIEW_LENGTH_X;
         blockView->lengthY = DEFAULT_VIEW_LENGTH_Y;
         blockView->lengthZ = DEFAULT_VIEW_LENGTH_Z;
         blockView->geologyCubeSize = DEFAULT_VIEW_GEOL_CUBE;
         blockView->geophysicsCubeSize = DEFAULT_VIEW_GEOP_CUBE;
      }

                                                          /* Geophysics Options */
      geophysicsOptions.calculationRange = 1200;
      geophysicsOptions.inclination = -67.0;
      geophysicsOptions.intensity = 63000.0;
      geophysicsOptions.declination = 0.0;
      geophysicsOptions.calculationAltitude = 0;
      geophysicsOptions.altitude = 80.0;
      geophysicsOptions.calculationMethod = SPATIAL;
      geophysicsOptions.constantBoxDepth = 0.0;
      geophysicsOptions.cleverBoxRatio = 1.0;
      geophysicsOptions.drapedSurvey = FALSE;
      geophysicsOptions.deformableRemanence = FALSE;
      geophysicsOptions.deformableAnisotropy = FALSE;
      geophysicsOptions.magneticVectorComponents = FALSE;
      geophysicsOptions.projectVectorsOntoField = TRUE;
      geophysicsOptions.padWithRealGeology = TRUE;
      geophysicsOptions.spectralPaddingType = RAMP_PADDING;
      geophysicsOptions.spectralFence = 50;
      geophysicsOptions.spectralPercent = 100;

                                                          /* threedView Options */
      threedViewOptions.declination = 150.0;
      threedViewOptions.azimuth = 30.0;
      threedViewOptions.scale = 1.0;
      threedViewOptions.offset_X = 1.0;
      threedViewOptions.offset_Y = 1.0;
      threedViewOptions.offset_Z = 1.0;
      threedViewOptions.allLayers = TRUE;
      memset(threedViewOptions.layerOn, 0, sizeof(short)*100);
      threedViewOptions.fillType = LINES_FILL;

                                                          /* winPosition Options */
      winPositionOptions.numWindows = 16;
      i = 0;
      strcpy (winPositionOptions.winPositions[i].name, BLOCK_WINDOW_TITLE);
      winPositionOptions.winPositions[i].x = BLOCK_WINDOW_POS_X;
      winPositionOptions.winPositions[i].y = BLOCK_WINDOW_POS_Y;
      winPositionOptions.winPositions[i].width = BLOCK_WINDOW_WIDTH;
      winPositionOptions.winPositions[i].height = BLOCK_WINDOW_HEIGHT;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, MOVIE_WINDOW_TITLE);
      winPositionOptions.winPositions[i].x = MOVIE_WINDOW_POS_X;
      winPositionOptions.winPositions[i].y = MOVIE_WINDOW_POS_Y;
      winPositionOptions.winPositions[i].width = -1;
      winPositionOptions.winPositions[i].height = -1;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, WELL_LOG_TITLE);
      winPositionOptions.winPositions[i].x = WELL_LOG_POS_X;
      winPositionOptions.winPositions[i].y = WELL_LOG_POS_Y;
      winPositionOptions.winPositions[i].width = WELL_LOG_WIDTH;
      winPositionOptions.winPositions[i].height = WELL_LOG_HEIGHT;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, SECTION_TITLE);
      winPositionOptions.winPositions[i].x = SECTION_POS_X;
      winPositionOptions.winPositions[i].y = SECTION_POS_Y;
      winPositionOptions.winPositions[i].width = SECTION_WIDTH;
      winPositionOptions.winPositions[i].height = -1;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, TOPOGRAPHY_TITLE);
      winPositionOptions.winPositions[i].x = TOPOGRAPHY_POS_X;
      winPositionOptions.winPositions[i].y = TOPOGRAPHY_POS_Y;
      winPositionOptions.winPositions[i].width = TOPOGRAPHY_WIDTH;
      winPositionOptions.winPositions[i].height = TOPOGRAPHY_HEIGHT;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, TOPOGRAPHY_3D_TITLE);
      winPositionOptions.winPositions[i].x = TOPOGRAPHY_3D_POS_X;
      winPositionOptions.winPositions[i].y = TOPOGRAPHY_3D_POS_Y;
      winPositionOptions.winPositions[i].width = TOPOGRAPHY_3D_WIDTH;
      winPositionOptions.winPositions[i].height = TOPOGRAPHY_3D_HEIGHT;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, STRATIGRAPHY_3D_TITLE);
      winPositionOptions.winPositions[i].x = STRATIGRAPHY_3D_POS_X;
      winPositionOptions.winPositions[i].y = STRATIGRAPHY_3D_POS_Y;
      winPositionOptions.winPositions[i].width = STRATIGRAPHY_3D_WIDTH;
      winPositionOptions.winPositions[i].height = STRATIGRAPHY_3D_HEIGHT;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, LINEMAP_TITLE);
      winPositionOptions.winPositions[i].x = LINEMAP_POS_X;
      winPositionOptions.winPositions[i].y = LINEMAP_POS_Y;
      winPositionOptions.winPositions[i].width = LINEMAP_WIDTH;
      winPositionOptions.winPositions[i].height = -1;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, PROFILE_FROM_IMAGE_TITLE);
      winPositionOptions.winPositions[i].x = PROFILE_POS_X;
      winPositionOptions.winPositions[i].y = PROFILE_POS_Y;
      winPositionOptions.winPositions[i].width = PROFILE_WIDTH;
      winPositionOptions.winPositions[i].height = PROFILE_HEIGHT;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, LINEMAP_PLOT_TITLE);
      winPositionOptions.winPositions[i].x = LINEMAP_PLOT_POS_X;
      winPositionOptions.winPositions[i].y = LINEMAP_PLOT_POS_Y;
      winPositionOptions.winPositions[i].width = LINEMAP_PLOT_WIDTH;
      winPositionOptions.winPositions[i].height = LINEMAP_PLOT_HEIGHT;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, STRATIGRAPHY_TITLE);
      winPositionOptions.winPositions[i].x = STRATIGRAPHY_POS_X;
      winPositionOptions.winPositions[i].y = STRATIGRAPHY_POS_Y;
      winPositionOptions.winPositions[i].width = STRATIGRAPHY_WIDTH;
      winPositionOptions.winPositions[i].height = STRATIGRAPHY_HEIGHT;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, IMAGE_TITLE);
      winPositionOptions.winPositions[i].x = IMAGE_POS_X;
      winPositionOptions.winPositions[i].y = IMAGE_POS_Y;
      winPositionOptions.winPositions[i].width = -1;
      winPositionOptions.winPositions[i].height = -1;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, CONTOUR_TITLE);
      winPositionOptions.winPositions[i].x = CONTOUR_POS_X;
      winPositionOptions.winPositions[i].y = CONTOUR_POS_Y;
      winPositionOptions.winPositions[i].width = -1;
      winPositionOptions.winPositions[i].height = -1;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, TOOLBAR_TITLE);
      winPositionOptions.winPositions[i].x = TOOLBAR_POS_X;
      winPositionOptions.winPositions[i].y = TOOLBAR_POS_Y;
      winPositionOptions.winPositions[i].width = -1;
      winPositionOptions.winPositions[i].height = -1;
      i++;
      strcpy (winPositionOptions.winPositions[i].name, HISTORY_TITLE);
      winPositionOptions.winPositions[i].x = HISTORY_POS_X;
      winPositionOptions.winPositions[i].y = HISTORY_POS_Y;
      winPositionOptions.winPositions[i].width = -1;
      winPositionOptions.winPositions[i].height = -1;
      i++;

   }

   return (TRUE);
}



/*
Open GL Stuff
*/
#ifndef OPENGL
/*#define OPENGL */
#endif

#ifdef OPENGL
#if XVTWS == WIN32WS
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#else
#undef Matrix

#include <GL/glx.h>		/* this includes X and gl.h headers */
#include <GL/glu.h>		/* gluPerspective(), gluLookAt(), GLU polygon tesselator */
#include <X11/Xatom.h>		/* for XA_RGB_DEFAULT_MAP atom */
#include <X11/Xmu/StdCmap.h>	/* for XmuLookupStandardColormap() */
#include <X11/keysym.h>		/* for XK_Escape keysym */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xirisw/GlxMDraw.h>
#include <X11/Xm/Xm.h>
#include <X11/Xm/Frame.h>
#include <gl/glws.h>


extern void draw_scene_callback ();
extern void do_resize ();
extern void initWindowForDrawing ();

#endif


int m_GLPixelIndex = 0;
#if XVTWS == WIN32WS
HGLRC m_hGLContext = NULL;
HDC m_hDC = NULL;
#else
GLXContext m_hGLContext = NULL;
XVisualInfo *m_hDC = NULL;
static XtAppContext app_context;
static Widget frame;
static unsigned long background;
float scrnaspect;                            /* aspect ratio value         */
long zfar;               /* holds specific machine's maximum Z depth value */
Widget glWidget;

/* The GLX configuration parameter:
 * 	Double buffering
 *	RGB mode
 *	Z buffering
 *	nothing else special
 */
static GLXconfig glxConfig [] = {
    { GLX_NORMAL, GLX_DOUBLE, TRUE },
    { GLX_NORMAL, GLX_RGB, TRUE },
    { GLX_NORMAL, GLX_ZSIZE, GLX_NOCONFIG },
    { 0, 0, 0 }
};

#endif

#if XVTWS == WIN32WS
int SetWindowPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pixelDesc;

	pixelDesc.nSize		= sizeof(PIXELFORMATDESCRIPTOR);
	pixelDesc.nVersion	= 1;

	pixelDesc.dwFlags	=	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | 
								PFD_DOUBLEBUFFER   | PFD_STEREO_DONTCARE;

	pixelDesc.iPixelType		= PFD_TYPE_RGBA;
	pixelDesc.cColorBits		= 32;
	pixelDesc.cRedBits		= 8;
	pixelDesc.cRedShift		= 16;
	pixelDesc.cGreenBits		= 8;
	pixelDesc.cGreenShift		= 8;
	pixelDesc.cBlueBits		= 8;
	pixelDesc.cBlueShift		= 0;
	pixelDesc.cAlphaBits		= 0;
	pixelDesc.cAlphaShift		= 0;
	pixelDesc.cAccumBits		= 64;	
	pixelDesc.cAccumRedBits		= 16;
	pixelDesc.cAccumGreenBits	= 16;
	pixelDesc.cAccumBlueBits	= 16;
	pixelDesc.cAccumAlphaBits	= 0;
	pixelDesc.cDepthBits		= 32;
	pixelDesc.cStencilBits		= 8;
	pixelDesc.cAuxBuffers		= 0;
	pixelDesc.iLayerType		= PFD_MAIN_PLANE;
	pixelDesc.bReserved		= 0;
	pixelDesc.dwLayerMask		= 0;
	pixelDesc.dwVisibleMask		= 0;
	pixelDesc.dwDamageMask		= 0;

	m_GLPixelIndex = ChoosePixelFormat( hDC, &pixelDesc);
	if (m_GLPixelIndex==0) // Let's choose a default index.
	{
		m_GLPixelIndex = 1;	
		if (DescribePixelFormat(hDC, m_GLPixelIndex, sizeof(PIXELFORMATDESCRIPTOR), &pixelDesc)==0)
		{
			xvt_dm_post_error("DescribePixelFormat Failed");
			return FALSE;
		}
	}

	if (SetPixelFormat( hDC, m_GLPixelIndex, &pixelDesc)==FALSE)
	{
		xvt_dm_post_error("SetPixelFormat Failed");
		return FALSE;
	}

	m_hDC = hDC;

	return TRUE;
}
#else
Colormap
getColormap(Display *dpy, XVisualInfo * vi)
{
	Status          status;
	XStandardColormap *standardCmaps;
	Colormap        cmap;
	int             i, numCmaps;

	/* be lazy; using DirectColor too involved for this example */
	if (vi->class != TrueColor)
		xvt_dm_post_error("no support for non-TrueColor visual");
						/* if no standard colormap but TrueColor, just make an unshared one */
	status = XmuLookupStandardColormap(dpy, vi->screen, vi->visualid,
					vi->depth, XA_RGB_DEFAULT_MAP, /* replace */ False, /* retain */ True);
	
	if (status == 1)
	{
		status = XGetRGBColormaps(dpy, RootWindow(dpy, vi->screen),
						  &standardCmaps, &numCmaps, XA_RGB_DEFAULT_MAP);
		if (status == 1)
			for (i = 0; i < numCmaps; i++)
				if (standardCmaps[i].visualid == vi->visualid)
				{
					 cmap = standardCmaps[i].colormap;
					 XFree(standardCmaps);
					 return cmap;
				}
	}
	cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen),
									vi->visual, AllocNone);

	return cmap;
}


/*
 * This file includes the function CopyGlColormap that creates a new
 * colormap for a GlxMDraw widget.  The colors specified
 * by the colorInfo structure (describe in copycmap.h) are allocated
 * in the colormap
 * When possible, they will match the pixel value in parent,
 * to avoid having incorrect colors installed in the X-based portion of
 * the application.
 * This function cannot be called until the widget is realized.
 */


CopyGlColormap(glw, colorInfo, colorInfoSize)
Widget glw;
struct glxcColorInfo *colorInfo;
int colorInfoSize;
{
	XVisualInfo *visualInfo;
	int depth;
	int maxcolor;
	Colormap colormap;
	Colormap pmap;
	Arg args[10];
	int n;
	register i;
	XColor rgb_db_def;
	Display *display = XtDisplay(glw);

	/* first get the visual and depth*/
	n = 0;
	XtSetArg(args[n], XtNvisual, &visualInfo); n++;
	XtSetArg(args[n], XtNdepth, &depth); n++;
	XtGetValues(glw, args, n);
	maxcolor = 1<<depth;
	/* Create a new colormap */
	colormap = XCreateColormap(display, XtWindow(glw), visualInfo->visual, AllocAll);
	/* Add the colormap to the window */
	n = 0;
	XtSetArg(args[n], XtNcolormap, colormap); n++;
	XtSetValues(glw, args, n);

	/* get the parent's colormap */
	n = 0;
	XtSetArg(args[n], XtNcolormap, &pmap); n++;
	XtGetValues(XtParent(glw), args, n);

	/* for each color in the colorInfo, determine the correct pixel value */
	for (i=0; i<colorInfoSize; i++)
	{
		colorInfo[i].color.flags = DoRed|DoGreen|DoBlue;
		switch (colorInfo[i].type)
		{
			case GLXC_ABSOLUTE:
				colorInfo[i].color.pixel = (Pixel)colorInfo[i].value;
				XQueryColor (display, pmap, &colorInfo[i].color);
				break;
			case GLXC_NAMED:
				if (!XAllocNamedColor(display, pmap, (char *)colorInfo[i].value,
														  &colorInfo[i].color, &rgb_db_def))
				{
					fprintf (stderr, "could not allocate %s\n", (char *)colorInfo[i].value);
					exit (1);
				}
				break;
			case GLXC_RESOURCE:
				n = 0;
				XtSetArg(args[n], colorInfo[i].value, &colorInfo[i].color.pixel); n++;
				XtGetValues(glw, args, n);
				XQueryColor (display, pmap, &colorInfo[i].color);
				break;
			default:
				fprintf (stderr, "unknown type in colorInfo\n");
				exit (1);
		}
	}
	/* We have determined all the colors.  Loop through the colors and
	* make sure that they fit into the GL colormap.  If they do not,
	* choose another color that fits into the colormap.
	* After this check, store the colors into the colormap,
	* and save in the variables */
	for (i=0; i<colorInfoSize; i++)
	{
		if (colorInfo[i].color.pixel >= maxcolor)
		{
			/* it was too high.  find another pixel to use.
			* start at the highest color and work down
			*/
			register try, check;

			for (try = maxcolor-1; try >= 0 ; try--)
			{
				for (check = 0; check < colorInfoSize; check++)
				{
					if (colorInfo[check].color.pixel == try)
					break;
				}
				if (check == colorInfoSize) /* didn't find a match */
				{
					colorInfo[i].color.pixel = try;
					break;
				}
			}
		}
		XStoreColor(display, colormap, &colorInfo[i].color);
		*colorInfo[i].result = colorInfo[i].color.pixel;
	}
}	

/* This routine converts a widget's background pixel to an RGB color
 * suitable for gl.
 */

unsigned long
WidgetBackgroundToGlRgb(widget)
Widget widget;
{
    Arg args[10];
    int n;
    Pixel xbg;		/* x background pixel */
    Colormap xcolormap;
    XColor xcolor;
    unsigned long glbg;	/* gl bacground color */

    /* First get the background pixel from the widget. */
    n = 0;
    XtSetArg(args[n], XtNbackground, &xbg); n++;
    XtGetValues(widget, args, n);

    /* Now get the colormap from the top level.  We can't use the widget's
     * colormap because it might not contain the same colors as the
     * colors that the background color were allocated from, so we use it's
     * parent
     */
    n = 0;
    XtSetArg(args[n], XtNcolormap, &xcolormap); n++;
    XtGetValues(XtParent(widget), args, n);

    /* Now obtain RGB values */
    xcolor.flags = DoRed | DoGreen | DoBlue;
    xcolor.pixel = xbg;
    XQueryColor (XtDisplay(widget), xcolormap, &xcolor);

    /* Now pack into an RGB color suitable for GL.  The format is 0x00BBGGRR
     * Since the x format has colors values from 0 to 65535 first shift the
     * x values right by 8 and then shift left to fit into the rgb color.*/
    glbg = (xcolor.red >> 8) + ((xcolor.green >> 8) << 8) +
	((xcolor.blue >> 8) << 16);
    return (glbg);
}
	
/*
 * This routine will install a particular gl widgets's colormap onto the
 * top level window.  It may not be called until after the windows have
 * been realized.
 */
installColormap(toplevel, glw)
Widget toplevel, glw;
{
    Window windows[2];

    windows[0] = XtWindow(glw);
    windows[1] = XtWindow(toplevel);
    XSetWMColormapWindows(XtDisplay(toplevel), XtWindow(toplevel), windows, 2);
}

#endif

int
#if XVTWS == WIN32WS
CreateViewGLContext(HDC hDC)
#else
CreateViewGLContext(Display *dpy, Window win, XVisualInfo *hDC)
#endif
{
#if XVTWS == WIN32WS
	m_hGLContext = wglCreateContext(hDC);
	if (m_hGLContext == NULL)
	{
		xvt_dm_post_error("wglCreateContext Failed");
		return FALSE;
	}

	if (wglMakeCurrent(hDC, m_hGLContext)==FALSE)
	{
		xvt_dm_post_error("wglMakeCurrent Failed");
		return FALSE;
	}
#else
	
   m_hGLContext = glXCreateContext(dpy, hDC, /* no sharing of display lists */ NULL,
						                      /* direct rendering if possible */ GL_TRUE);
	if (m_hGLContext == NULL)
	{
		xvt_dm_post_error("wglCreateContext Failed");
		return FALSE;
	}

	if (glXMakeCurrent(dpy, win, m_hGLContext) == FALSE)
	{
		xvt_dm_post_error("wglMakeCurrent Failed");
		return FALSE;
	}
#endif

	return TRUE;
}

#if XVTWS == WIN32WS
int
OnCreate(HDC hDC) 
{
	if (SetWindowPixelFormat(hDC) == FALSE)
		return 0;

	if (CreateViewGLContext(hDC) == FALSE)
			return 0;

	return 0;
}
#else
void
createGLwidget(xvtWidget)
Widget xvtWidget;
{
	Widget toplevel, glw, w;
	Arg args[20];
	int n;

		/* create a frame widget to contain the GL widget */
	frame = XtVaCreateManagedWidget("GLframe", xmFrameWidgetClass, xvtWidget,
		  XmNwidth, 200, XmNheight, 200, XmNx, 0, XmNy, 0,
		  XmNshadowType, XmSHADOW_IN, NULL); 

		/* get toplevel widget for the XVT widget */
	for (w = xvtWidget; w != NULL; w = XtParent(toplevel))
		 toplevel = w;

		/* add actions and create a translation table */
	app_context = XtWidgetToApplicationContext(xvtWidget);
		 
		/* create the GL widget */
	n = 0;
	XtSetArg(args[n], GlxNglxConfig, glxConfig); n++;
	XtSetArg(args[n], XmNheight, 200); n++;
	XtSetArg(args[n], XmNwidth, 200); n++;
	XtSetArg(args[n], XmNx, 0); n++;
	XtSetArg(args[n], XmNy, 0); n++;
	glw = GlxCreateMDraw(frame, "glwidget", args, n);
        glWidget = glw;   /* Assign the global */
	XtAddCallback(glw, GlxNexposeCallback, draw_scene_callback, 0);
	XtAddCallback(glw, GlxNresizeCallback, do_resize, 0);
/*
	XtAddCallback(glw, GlxNginitCallback, initWindowForDrawing, 0);
*/
	XtManageChild (glw);

	    /* Obtain the proper background pixel and install the colormap */
/*
	background = WidgetBackgroundToGlRgb(glw);
	installColormap(toplevel,glw);
*/
}

int initializeGLWindow()
{
	long xscrnsize;              /* size of screen in x used to set globals  */
	long testifZinst;

	/*
	* This program requires the following to run:
	*  -- z buffer
	*  -- ability to do double-buffered RGB mode
	*/
			/* Test for Z buffer */
	/*
	testifZinst = getgdesc(GD_BITS_NORM_ZBUFFER);
	if (testifZinst == FALSE)
	{
		xvt_dm_post_error("Won't work on this machine--zbuffer option not installed.");
		exit(0);
	}
	*/
			/* Test for double-buffered RGB */
	/*
	if (getgdesc(GD_BITS_NORM_DBL_RED) == 0)
	{
		xvt_dm_post_error("Won't work on this machine--not enough bitplanes.");
		exit(0);
	}
	*/

			/* Code to keep same aspec ratio as the screen */
	/*
	keepaspect(getgdesc(GD_XMMAX), getgdesc(GD_YMMAX));
	scrnaspect = (float)getgdesc(GD_XMMAX)/(float)getgdesc(GD_YMMAX);
	*/
/*
	doublebuffer();
	RGBmode();
	zbuffer(TRUE);
	glcompat(GLC_ZRANGEMAP, 0);
	zfar = getgdesc(GD_ZMAX);

        gconfig ();
*/
}

void
draw_scene_callback(w, client_data, call_data)
    Widget w;
    caddr_t client_data;
    caddr_t call_data;
{
	extern void RenderScene(Display *, Window, double, double);
	extern double xRotate, yRotate;
	Display *dpy = NULL;
	Window win;


	GLXwinset(XtDisplay(w), XtWindow(w)); 
	RenderScene(XtDisplay(w), XtWindow(w), xRotate, yRotate);
}

void
initWindowForDrawing(w, client_data, call_data)
    Widget w;
    caddr_t client_data;
    caddr_t call_data;
{
    GLXwinset(XtDisplay(w), XtWindow(w)); 
    initializeGLWindow();
}

void
do_resize(w, client_data, call_data)
    Widget w;
    caddr_t client_data;
    GlxDrawCallbackStruct *call_data;
{
    GLXwinset(XtDisplay(w), XtWindow(w));
/*
    glViewport(0, (Screencoord) call_data->width-1,
	     0, (Screencoord) call_data->height-1);
    draw_scene_callback(w, 0, 0);
*/
}

void
resizeFrame(w, h)
int w, h;
{
	XtVaSetValues(frame, XmNwidth, w, XmNheight, h, NULL);
}

void
setVisGLwidget(vis)
int vis;
{
	if (vis)
		XtMapWidget(frame);
	else
		XtUnmapWidget(frame);
}
#endif

void OnSize(int cx, int cy) 
{
	GLsizei width, height;
	GLdouble aspect;

	width = cx;
	height = cy;

	if (cy==0)
		aspect = (GLdouble)width;
	else
		aspect = (GLdouble)width/(GLdouble)height;
	
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, aspect, 1, 10.0);
/*
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glDrawBuffer(GL_BACK);
*/
}

void
#if XVTWS == WIN32WS
RenderScene(GLdouble m_xRotate, GLdouble m_yRotate)
#else
RenderScene(Display *dpy, Window win,
            GLdouble m_xRotate, GLdouble m_yRotate)
#endif
{
	GLfloat RedSurface[]   = { 1.0f, 0.0f, 0.0f, 1.0f};
	GLfloat GreenSurface[] = { 0.0f, 1.0f, 0.0f, 1.0f};
	GLfloat BlueSurface[]  = { 0.0f, 0.0f, 1.0f, 1.0f};

	GLfloat LightAmbient[]	= { 0.1f, 0.1f, 0.1f, 0.1f };
	GLfloat LightDiffuse[]	= { 0.7f, 0.7f, 0.7f, 0.7f };
	GLfloat LightSpecular[]	= { 0.0f, 0.0f, 0.0f, 0.1f };
	GLfloat LightPosition[]	= { 5.0f, 5.0f, 5.0f, 0.0f };

/*
	glDrawBuffer(GL_FRONT_AND_BACK);
*/
	glDrawBuffer(GL_FRONT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT0);

	glPushMatrix();	
		glTranslated(0.0, 0.0, -8.0);
		glRotated(m_xRotate, 1.0, 0.0, 0.0);
		glRotated(m_yRotate, 0.0, 1.0, 0.0);

		glMaterialfv(GL_FRONT, GL_AMBIENT, RedSurface);
		glBegin(GL_POLYGON);
			glNormal3d(  1.0,  0.0,  0.0);
			glVertex3d(  1.0,  1.0,  1.0);
			glVertex3d(  1.0, -1.0,  1.0);
			glVertex3d(  1.0, -1.0, -1.0);
			glVertex3d(  1.0,  1.0, -1.0);
		glEnd();

		glBegin(GL_POLYGON);
			glNormal3d( -1.0,  0.0,  0.0);
			glVertex3d( -1.0, -1.0,  1.0);
			glVertex3d( -1.0,  1.0,  1.0);
			glVertex3d( -1.0,  1.0, -1.0);
			glVertex3d( -1.0, -1.0, -1.0);
		glEnd();

		glMaterialfv(GL_FRONT, GL_AMBIENT, GreenSurface);
		glBegin(GL_POLYGON);
			glNormal3d(  0.0,  1.0,  0.0);
			glVertex3d(  1.0,  1.0,  1.0);
			glVertex3d( -1.0,  1.0,  1.0);
			glVertex3d( -1.0,  1.0, -1.0);
			glVertex3d(  1.0,  1.0, -1.0);
		glEnd();				  

		glBegin(GL_POLYGON);
			glNormal3d(  0.0, -1.0,  0.0);
			glVertex3d( -1.0, -1.0,  1.0);
			glVertex3d(  1.0, -1.0,  1.0);
			glVertex3d(  1.0, -1.0, -1.0);
			glVertex3d( -1.0, -1.0, -1.0);
		glEnd();

		glMaterialfv(GL_FRONT, GL_AMBIENT, BlueSurface);
		glBegin(GL_POLYGON);
			glNormal3d(  0.0,  0.0,  1.0);
			glVertex3d(  1.0,  1.0,  1.0);
			glVertex3d( -1.0,  1.0,  1.0);
			glVertex3d( -1.0, -1.0,  1.0);
			glVertex3d(  1.0, -1.0,  1.0);
		glEnd();

		glBegin(GL_POLYGON);
			glNormal3d(  0.0,  0.0, -1.0);
			glVertex3d( -1.0,  1.0, -1.0);
			glVertex3d(  1.0,  1.0, -1.0);
			glVertex3d(  1.0, -1.0, -1.0);
			glVertex3d( -1.0, -1.0, -1.0);
		glEnd();
	glPopMatrix();

	glFlush();
#if XVTWS == WIN32WS
	if (!SwapBuffers(m_hDC))
		xvt_dm_post_error("SwapBuffers Failed");
#else
/*
	glXSwapBuffers(dpy, win);
*/
#endif
	glFlush();
}


#endif  /* OPENGL */

/*
*/
#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>

#ifndef FCN_NODDY_ONLY
#endif

#define DEBUG(X)    
                                /* Constants to this file */
#define TOLERANCE             0.001
#define STRAT_LIMIT           100
                                /* Gloabal Variables */
extern double iscale; /* scaling factor for geology */
extern PROJECT_OPTIONS projectOptions;
extern BLOCK_VIEW_OPTIONS *blockViewOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;
extern double minTopoValue, maxTopoValue;
extern double **topographyMap;
extern int TopoRow, TopoCol;
extern int batchExecution, noStatusWin;
extern WINDOW lastActiveWindow;
extern ROCK_DATABASE rockDatabase;
extern COLOR backgroundColor;

#if XVT_CC_PROTO
extern PROFILE_OPTIONS *getCurrentProfileOptions (WINDOW, PROFILE_OPTIONS *);
extern int isProfileReadOnly (OBJECT *);
#else
extern PROFILE_OPTIONS *getCurrentProfileOptions ();
extern int isProfileReadOnly ();
#endif

                                /* Gloabal Variables to this file */
BOOLEAN processingLongJob = FALSE;
static WINDOW jobStatusWindow = (WINDOW) NULL;
static WINDOW percentJobWindow = (WINDOW) NULL;
static int firstToProcess;  /* used to gauge long running processes */
static int lastToProcess;
static int rangeToProcess;
static int minPercentIncrement;
static int lastProcessed;
static int currentBeingProcessed;
static BOOLEAN cancelLongJob;

                                /* Functions in this file */
#if XVT_CC_PROTO
void loadProjectOptions (WINDOW);
void saveProjectOptions (WINDOW);
void loadParallelOptions (WINDOW);
void saveParallelOptions (WINDOW);
void loadViewOptions (WINDOW);
void saveViewOptions (WINDOW);
BLOCK_VIEW_OPTIONS *newViewOptions (char *, double, double, double, double, double, double, double, double);
int deleteViewOptions (BLOCK_VIEW_OPTIONS *);
BLOCK_VIEW_OPTIONS *getViewOptions ();
BLOCK_VIEW_OPTIONS *setViewOptions (char *);
void loadGeophysicsOptions (WINDOW);
void saveGeophysicsOptions (WINDOW);
int updateGeophysicsOptions (WINDOW);
void loadGeologyOptions (WINDOW);
void saveGeologyOptions (WINDOW);
void loadGeophysicsImageOptions (WINDOW);
void saveGeophysicsImageOptions (WINDOW);
void updateGeophysicsImageOptions (WINDOW);
int setBlockImageColorIndex (WINDOW, PNT *, double *, double *);
COLOR *initBlockImageColorLut (int);
int loadBlockImageOptions (WINDOW);
int saveBlockImageOptions (WINDOW, BLOCK_DIAGRAM_DATA *);
int updateBlockImageOptions (WINDOW);
int loadWinPositionOptions (WINDOW);
int saveWinPositionOptions (WINDOW);
int updateWinPositionOptions (WINDOW);
void load3dOptions (WINDOW);
void save3dOptions (WINDOW);
void setGeologyTopoRange (WINDOW);
void loadGeologyTopoOptions (WINDOW);
void saveGeologyTopoOptions (WINDOW);
void loadGeologyMovieOptions (WINDOW);
void saveGeologyMovieOptions (WINDOW);
void loadXYZImportOptions (WINDOW, XYZ_IMPORT *);
void saveXYZImportOptions (WINDOW, XYZ_IMPORT *);
int loadRockPropOptions (WINDOW, OBJECT *);
int saveRockPropOptions (WINDOW, OBJECT *);
int loadProfileOptions (WINDOW, OBJECT *);
int saveProfileOptions (WINDOW, OBJECT *);
int saveProfileOptionsOptions (WINDOW, OBJECT *);
void loadWindowOptions (WINDOW, int, OBJECT *);
void saveWindowOptions (WINDOW, int, OBJECT *);
int setEventPosition (OBJECT *, double, double, double);
int getEventPosition (OBJECT *, double *, double *, double *);
void initLongJob (int, int, char *, char *);
int incrementLongJob (int);
void finishLongJob ();
int abortLongJob ();
void setAbortLongJob ();
WINDOW getJobStatusWindow ();
void setControlFont (WINDOW);
#else
void loadProjectOptions ();
void saveProjectOptions ();
void loadParallelOptions ();
void saveParallelOptions ();
void loadViewOptions ();
void saveViewOptions ();
BLOCK_VIEW_OPTIONS *newViewOptions ();
int deleteViewOptions ();
BLOCK_VIEW_OPTIONS *getViewOptions ();
BLOCK_VIEW_OPTIONS *setViewOptions ();
void loadGeophysicsOptions ();
void saveGeophysicsOptions ();
int updateGeophysicsOptions ();
void loadGeologyOptions ();
void saveGeologyOptions ();
void loadGeophysicsImageOptions ();
void saveGeophysicsImageOptions ();
void updateGeophysicsImageOptions ();
int setBlockImageColorIndex ();
COLOR *initBlockImageColorLut ();
int loadBlockImageOptions ();
int saveBlockImageOptions ();
int updateBlockImageOptions ();
int loadWinPositionOptions ();
int saveWinPositionOptions ();
int updateWinPositionOptions ();
void load3dOptions ();
void save3dOptions ();
void setGeologyTopoRange ();
void loadGeologyTopoOptions ();
void saveGeologyTopoOptions ();
void loadGeologyMovieOptions ();
void saveGeologyMovieOptions ();
void loadXYZImportOptions ();
void saveXYZImportOptions ();
int loadRockPropOptions ();
int saveRockPropOptions ();
int loadProfileOptions ();
int saveProfileOptions ();
int saveProfileOptionsOptions ();
void loadWindowOptions ();
void saveWindowOptions ();
int setEventPosition ();
int getEventPosition ();
void initLongJob ();
int incrementLongJob ();
void finishLongJob ();
int abortLongJob ();
void setAbortLongJob ();
WINDOW getJobStatusWindow ();
void setControlFont ();
#endif


/* ======================================================================
FUNCTION        loadProjectOptions
DESCRIPTION
        load the project options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadProjectOptions (WINDOW parentWin)
#else
loadProjectOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   int buttonIndex;
   WINDOW radioButtons[3];

   if (!parentWin) return;

   radioButtons[0]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_WINDOW_C13);
   radioButtons[1]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_WINDOW_C14);
   buttonIndex = projectOptions.susceptibilityUnits;
   if ((buttonIndex < 0) || (buttonIndex > 1))
      buttonIndex = 0;
   xvt_ctl_check_radio_button (radioButtons[buttonIndex], radioButtons, 2);

   radioButtons[0]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_REMOTE);
   radioButtons[1]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_LOCAL);
   buttonIndex = projectOptions.geophysicalCalc;
   if ((buttonIndex < 0) || (buttonIndex > 1))
      buttonIndex = 0;
   xvt_ctl_check_radio_button (radioButtons[buttonIndex], radioButtons, 2);

   radioButtons[0]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_LOCAL);
   radioButtons[1]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_REMOTE);
   buttonIndex = projectOptions.calculationType;
   if ((buttonIndex < 0) || (buttonIndex > 1))
      buttonIndex = 0;
   xvt_ctl_check_radio_button (radioButtons[buttonIndex], radioButtons, 2);

   radioButtons[0]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_WINDOW_C10);
   radioButtons[1]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_WINDOW_C11);
   buttonIndex = projectOptions.lengthScale;
   if ((buttonIndex < 0) || (buttonIndex > 1))
      buttonIndex = 0;
   xvt_ctl_check_radio_button (radioButtons[buttonIndex], radioButtons, 2);

   win = updateFloatTextField (parentWin, PROJECT_OPTIONS_PRINT_SCALE,
                        projectOptions.printScalingFactor, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, PROJECT_OPTIONS_IMAGE_SCALE,
                        projectOptions.imageScalingFactor, 1);
   setControlFont (win);

   win = xvt_win_get_ctl (parentWin, PROJECT_OPTIONS_NEW_WINDOWS);
   xvt_ctl_set_checked(win, projectOptions.newWindowEachStage);

   win = updateFloatTextField (parentWin, PROJECT_EASTING, projectOptions.easting, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, PROJECT_NORTHING, projectOptions.northing, 1);
   setControlFont (win);

                 /* save the options in the window so they can be restored **
                 ** if cancel is pressed to exit the object */
   copyOptions (PROJECT_OPTIONS_WINDOW, (OBJECT *) &projectOptions, OPTIONS_SAVE);

#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   win = xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_LOCAL);
   xvt_vobj_set_enabled (win, FALSE);
   win = xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_REMOTE);
   xvt_vobj_set_enabled (win, FALSE);
#else
   win = xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_LOCAL);
   xvt_vobj_set_enabled (win, TRUE);
   win = xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_REMOTE);
   xvt_vobj_set_enabled (win, TRUE);
#endif
}

/* ======================================================================
FUNCTION        saveProjectOptions
DESCRIPTION
        save from the project options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveProjectOptions (WINDOW parentWin)
#else
saveProjectOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   int buttonIndex;
   WINDOW radioButtons[3];

   if (!parentWin) return;

   radioButtons[0]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_WINDOW_C13);
   radioButtons[1]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_WINDOW_C14);
   for (buttonIndex = 0; buttonIndex < 2; buttonIndex++)
   {
      if (xvt_ctl_is_checked (radioButtons[buttonIndex]))
         break;
   }
   projectOptions.susceptibilityUnits = buttonIndex;

   radioButtons[0]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_GRAVITY);
   radioButtons[1]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_MAGNETICS);
   for (buttonIndex = 0; buttonIndex < 2; buttonIndex++)
   {
      if (xvt_ctl_is_checked (radioButtons[buttonIndex]))
         break;
   }
   projectOptions.geophysicalCalc = buttonIndex;

   radioButtons[0]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_LOCAL);
   radioButtons[1]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_REMOTE);
   for (buttonIndex = 0; buttonIndex < 2; buttonIndex++)
   {
      if (xvt_ctl_is_checked (radioButtons[buttonIndex]))
         break;
   }
   projectOptions.calculationType = buttonIndex;

   radioButtons[0]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_WINDOW_C10);
   radioButtons[1]= xvt_win_get_ctl(parentWin, PROJECT_OPTIONS_WINDOW_C11);
   for (buttonIndex = 0; buttonIndex < 2; buttonIndex++)
   {
      if (xvt_ctl_is_checked (radioButtons[buttonIndex]))
         break;
   }
   projectOptions.lengthScale = buttonIndex;
   
   projectOptions.printScalingFactor = getFloatTextFieldValue (parentWin,
                                          PROJECT_OPTIONS_PRINT_SCALE);
   projectOptions.imageScalingFactor = getFloatTextFieldValue (parentWin,
                                          PROJECT_OPTIONS_IMAGE_SCALE);

   win = xvt_win_get_ctl (parentWin, PROJECT_OPTIONS_NEW_WINDOWS);
   projectOptions.newWindowEachStage = (int) xvt_ctl_is_checked (win);

   projectOptions.easting = getFloatTextFieldValue (parentWin, PROJECT_EASTING);
   projectOptions.northing = getFloatTextFieldValue (parentWin, PROJECT_NORTHING);

}
/* ======================================================================
FUNCTION        loadParallelOptions
DESCRIPTION
        load the project options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadParallelOptions (WINDOW parentWin)
#else
loadParallelOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;

   if (!parentWin) return;

   win = xvt_win_get_ctl (parentWin, PARALLEL_OPTIONS_ADDRESS);
   xvt_vobj_set_title (win, projectOptions.internetAddress);
   win = xvt_win_get_ctl (parentWin, PARALLEL_OPTIONS_LOGIN);
   xvt_vobj_set_title (win, projectOptions.accountName);
   win = xvt_win_get_ctl (parentWin, PARALLEL_OPTIONS_PATH);
   xvt_vobj_set_title (win, projectOptions.noddyPath);

}
/* ======================================================================
FUNCTION        saveProjectOptions
DESCRIPTION
        save from the project options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveParallelOptions (WINDOW parentWin)
#else
saveParallelOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   char text[100];

   win = xvt_win_get_ctl (parentWin, PARALLEL_OPTIONS_ADDRESS);
   xvt_vobj_get_title (win, text, 100);
   strcpy (projectOptions.internetAddress, text);

   win = xvt_win_get_ctl (parentWin, PARALLEL_OPTIONS_LOGIN);
   xvt_vobj_get_title (win, text, 50);
   strcpy (projectOptions.accountName, text);

   win = xvt_win_get_ctl (parentWin, PARALLEL_OPTIONS_PATH);
   xvt_vobj_get_title (win, text, 100);
   strcpy (projectOptions.noddyPath, text);
}


/* ======================================================================
FUNCTION        loadViewOptions
DESCRIPTION
        load the view options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadViewOptions (WINDOW parentWin)
#else
loadViewOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   int index;
   WINDOW viewList;
   BLOCK_VIEW_OPTIONS *viewOptions = NULL, *currentOptions = NULL;

   if (!parentWin) return;
   if (!(currentOptions = getViewOptions ()))
      return;
   
   
   viewList = xvt_win_get_ctl (parentWin, VIEW_CURRENT_LIST);
   xvt_list_suspend (viewList);
   xvt_list_clear (viewList);
   viewOptions = currentOptions;
   while (viewOptions->prev)    /* move to start */
      viewOptions = viewOptions->prev;
   index = 0;
   while (viewOptions)
   {
      xvt_list_add (viewList, index, viewOptions->viewName);
      if (viewOptions == currentOptions)     /* Select current */
         xvt_list_set_sel (viewList, index, TRUE);
      viewOptions = viewOptions->next;
   }
   xvt_list_resume (viewList);
   
   win = updateFloatTextField (parentWin, VIEW_ORIGIN_X,
                               currentOptions->originX, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, VIEW_ORIGIN_Y,
                               currentOptions->originY, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, VIEW_ORIGIN_Z,
                               currentOptions->originZ, 1);
   setControlFont (win);


   win = updateFloatTextField (parentWin, DIM_X,
                               currentOptions->lengthX, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, DIM_Y,
                               currentOptions->lengthY, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, DIM_Z, 
                               currentOptions->lengthZ, 1);
   setControlFont (win);
   
   updateFloatTextField (parentWin, VIEW_GEOLOGY_CUBE,
                         currentOptions->geologyCubeSize, 1);
   updateFloatTextField (parentWin, VIEW_GEOPHYSICS_CUBE,
                         currentOptions->geophysicsCubeSize, 1);

         /* Disable Block View button if we have already come from their */
   if (stackParentWin(parentWin) != getEventDrawingWindow())
      if (win = xvt_win_get_ctl(parentWin, VIEW_3D_OPTIONS_BUTTON))
         xvt_vobj_set_enabled (win, FALSE);
}

/* ======================================================================
FUNCTION        saveViewOptions
DESCRIPTION
        save from the geology options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveViewOptions (WINDOW parentWin)
#else
saveViewOptions (parentWin)
WINDOW parentWin;
#endif
{
   BLOCK_VIEW_OPTIONS *viewOptions;
   
   if (!parentWin) return;
   if (!(viewOptions = getViewOptions ()))
      return;
   
   viewOptions->originX = getFloatTextFieldValue (parentWin, VIEW_ORIGIN_X);
   viewOptions->originY = getFloatTextFieldValue (parentWin, VIEW_ORIGIN_Y);
   viewOptions->originZ = getFloatTextFieldValue (parentWin, VIEW_ORIGIN_Z);

   viewOptions->lengthX = getFloatTextFieldValue (parentWin, DIM_X);
   viewOptions->lengthY = getFloatTextFieldValue (parentWin, DIM_Y);
   viewOptions->lengthZ = getFloatTextFieldValue (parentWin, DIM_Z);

   viewOptions->geologyCubeSize = getFloatTextFieldValue (parentWin, VIEW_GEOLOGY_CUBE);
   viewOptions->geophysicsCubeSize = getFloatTextFieldValue (parentWin, VIEW_GEOPHYSICS_CUBE);
}

/* ======================================================================
FUNCTION        newViewOptions
DESCRIPTION
        save from the geology options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
BLOCK_VIEW_OPTIONS *
#if XVT_CC_PROTO
newViewOptions (char *viewName, double originX, double originY, double originZ,
                                double dimX,    double dimY,    double dimZ,
                                double geolCubeSize, double geophCubeSize)
#else
newViewOptions (viewName, originX, originY, originZ, dimX, dimY, dimZ,
                          geolCubeSize, geophCubeSize)
char *viewName;
double originX, originY, originZ;
double dimX, dimY, dimZ, geolCubeSize, geophCubeSize;
#endif
{
   BLOCK_VIEW_OPTIONS *newView = NULL;

   if (!viewName)
      return (newView);

               /* If another one has same name delete it so this replaces it */
   if (newView = setViewOptions (viewName))
      deleteViewOptions (newView);

   if (!(newView = (BLOCK_VIEW_OPTIONS *) xvt_mem_zalloc (sizeof(BLOCK_VIEW_OPTIONS))))
      return (newView);
      
   strcpy (newView->viewName, viewName);
   newView->originX = originX;
   newView->originY = originY;
   newView->originZ = originZ;
   newView->lengthX = dimX;
   newView->lengthY = dimY;
   newView->lengthZ = dimZ;
   newView->geologyCubeSize = geolCubeSize;
   newView->geophysicsCubeSize = geophCubeSize;
   newView->prev = NULL;
   newView->next = NULL;
   
   if (blockViewOptions)
   {
      blockViewOptions->prev = newView;
      newView->next = blockViewOptions;
   }

   blockViewOptions = newView;  /* set the new one to be the current one */
   
   return (newView);
}
/* ======================================================================
FUNCTION        newViewOptions
DESCRIPTION
        save from the geology options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
deleteViewOptions (BLOCK_VIEW_OPTIONS *view)
#else
deleteViewOptions (view)
BLOCK_VIEW_OPTIONS *view;
#endif
{
   BLOCK_VIEW_OPTIONS *current;
   
   current = getViewOptions ();
   
   if (current != view)
   {
      while (current->prev)  /* move to start */
         current = current->prev;
      while (current && (current != view))
         current = current->next;
   }
   
   if (current == view)
   {
      if (!current->next && !current->prev)  /* Only one view so we need it */
      {
         return (FALSE);
      }

      if (current->prev) 
         current->prev->next = current->next;
         
      if (current->next)
         current->next->prev = current->prev;
         
      if (current == blockViewOptions)
         if (current->prev)
            blockViewOptions = current->prev;
         else if (current->next)
            blockViewOptions = current->next;

      xvt_mem_free ((char *) current);
         
      return (TRUE);
   }
   else
      return (FALSE);
}
/* ======================================================================
FUNCTION        getViewOptions
DESCRIPTION
        save from the geology options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
BLOCK_VIEW_OPTIONS *
#if XVT_CC_PROTO
getViewOptions ()
#else
getViewOptions ()
#endif
{
   if (!blockViewOptions)
   {
      newViewOptions (DEFAULT_VIEW_NAME,
            DEFAULT_VIEW_ORIGIN_X, DEFAULT_VIEW_ORIGIN_Y, DEFAULT_VIEW_ORIGIN_Z,
            DEFAULT_VIEW_LENGTH_X, DEFAULT_VIEW_LENGTH_Y, DEFAULT_VIEW_LENGTH_Z,
            DEFAULT_VIEW_GEOL_CUBE, DEFAULT_VIEW_GEOP_CUBE);
   }

   return (blockViewOptions);
}

/* ======================================================================
FUNCTION        setViewOptions
DESCRIPTION
        save from the geology options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
BLOCK_VIEW_OPTIONS *
#if XVT_CC_PROTO
setViewOptions (char *viewName)
#else
setViewOptions (viewName)
char *viewName;
#endif
{  
   BLOCK_VIEW_OPTIONS *viewOptions = NULL;
   
   if (!blockViewOptions)
      return (viewOptions);
   
   viewOptions = blockViewOptions;
   while (viewOptions->prev)
      viewOptions = viewOptions->prev;   /* move to start */
      
                  /* Look for matching name */
   while (viewOptions && strcmp (viewOptions->viewName, viewName))
      viewOptions = viewOptions->next;
   
   if (viewOptions)
      blockViewOptions = viewOptions;
   
   return (viewOptions);
}

/* ======================================================================
FUNCTION        loadGeophysicsOptions
DESCRIPTION
        load the Geophysics options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadGeophysicsOptions (WINDOW parentWin)
#else
loadGeophysicsOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win, win2;
   int buttonIndex, enable;
   WINDOW radioButtons[3];

   if (!parentWin) return;
                                   /* update the cube label */
   
   win = updateNumericTextField (parentWin, GEOPHYSICS_RANGE,
                                    geophysicsOptions.calculationRange);
   setControlFont (win);
   win = updateFloatTextField (parentWin, WIN_118_EDIT_49,
                                    geophysicsOptions.inclination, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, WIN_118_EDIT_48,
                                    geophysicsOptions.intensity, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, GEOPHYSICS_OPTIONS_DECL,
                                    geophysicsOptions.declination, 1);
   setControlFont (win);

   radioButtons[0]= xvt_win_get_ctl(parentWin, WIN_118_RADIOBUTTON_42);
   radioButtons[1]= xvt_win_get_ctl(parentWin, WIN_118_RADIOBUTTON_43);
   buttonIndex = geophysicsOptions.calculationAltitude;
   if ((buttonIndex < 0) || (buttonIndex > 1))
      buttonIndex = 0;
   xvt_ctl_check_radio_button (radioButtons[buttonIndex], radioButtons, 2);
   enable = buttonIndex;
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin,GEOPHYSICS_OPTIONS_WIN_C1),(BOOLEAN) enable);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin,GEOPHYSICS_OPTIONS_WIN_C2),(BOOLEAN) enable);

   win = updateFloatTextField (parentWin, GEOPHYSICS_OPTIONS_WIN_C2,
                                    geophysicsOptions.altitude, 1);
   setControlFont (win);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_OPTIONS_WIN_C4a);
   xvt_ctl_set_checked(win, geophysicsOptions.drapedSurvey);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_OPTIONS_WIN_C3);
   xvt_ctl_set_checked(win, geophysicsOptions.deformableRemanence);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_OPTIONS_WIN_C4);
   xvt_ctl_set_checked(win, geophysicsOptions.deformableAnisotropy);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_MAG_COMPONENTS);
   xvt_ctl_set_checked(win, geophysicsOptions.magneticVectorComponents);

   radioButtons[0]= xvt_win_get_ctl(parentWin, GEOPHYSICS_OPTIONS_SPATIAL);
   radioButtons[1]= xvt_win_get_ctl(parentWin, GEOPHYSICS_OPTIONS_SPECTRAL);
   buttonIndex = geophysicsOptions.calculationMethod;
   if ((buttonIndex < 0) || (buttonIndex > 1))
      buttonIndex = 0;
   xvt_ctl_check_radio_button (radioButtons[buttonIndex], radioButtons, 2);
   if (geophysicsOptions.calculationMethod == SPATIAL)
      xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin,GEOPHYSICS_OPTIONS_WIN_C4a), FALSE); 

   win2 = xvt_win_get_ctl (parentWin, GEOPHYSICS_BOXING_CLEVER);
   if (fabs(geophysicsOptions.constantBoxDepth) < TOLERANCE)
   {
      win = xvt_win_get_ctl (parentWin, GEOPHYSICS_BOXING_CONSTANT);
      xvt_vobj_set_title (win, "All");
      xvt_vobj_set_enabled (win2, FALSE);
   }   
   else
   {
      win = updateFloatTextField (parentWin, GEOPHYSICS_BOXING_CONSTANT,
                                    geophysicsOptions.constantBoxDepth, 1);
      xvt_vobj_set_enabled (win2, TRUE);
   }
   setControlFont (win);

   xvt_list_set_sel(win2, (int) floor(geophysicsOptions.cleverBoxRatio-1.0), TRUE);

   if (win = xvt_win_get_ctl (parentWin, GEOPHYSICS_PADDING_TYPE))
   {
      xvt_list_suspend (win);
      xvt_list_add (win, 0, "Ramp");
      xvt_list_add (win, 1, "Fence using mean");
      xvt_list_add (win, 2, "Fence using set value");
      xvt_list_add (win, 3, "Set to mean (non cyclic)");
      xvt_list_add (win, 4, "Set to value (non cyclic)");
      xvt_list_add (win, 5, "Reflection");
      xvt_list_set_sel (win, geophysicsOptions.spectralPaddingType, TRUE);
      xvt_list_resume (win);
   }
   
   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_PADDING_PARAM);
   setControlFont (win);

   updateGeophysicsOptions (parentWin);
}

/* ======================================================================
FUNCTION        saveGeophysicsOptions
DESCRIPTION
        save from the geophysics options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveGeophysicsOptions (WINDOW parentWin)
#else
saveGeophysicsOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   int buttonIndex;
   WINDOW radioButtons[3];
   double calcDistance;

   if (!parentWin) return;

   calcDistance = getFloatTextFieldValue (parentWin, GEOPHYSICS_RANGE);
   geophysicsOptions.calculationRange = (int) floor (calcDistance + 0.5);

   geophysicsOptions.inclination = getFloatTextFieldValue (parentWin,
                                    WIN_118_EDIT_49);
   geophysicsOptions.intensity = getFloatTextFieldValue (parentWin,
                                    WIN_118_EDIT_48);
   geophysicsOptions.declination = getFloatTextFieldValue (parentWin,
                                    GEOPHYSICS_OPTIONS_DECL);

   radioButtons[0]= xvt_win_get_ctl(parentWin, WIN_118_RADIOBUTTON_42);
   radioButtons[1]= xvt_win_get_ctl(parentWin, WIN_118_RADIOBUTTON_43);
   for (buttonIndex = 0; buttonIndex < 2; buttonIndex++)
   {
      if (xvt_ctl_is_checked (radioButtons[buttonIndex]))
         break;
   }
   geophysicsOptions.calculationAltitude = buttonIndex;

   geophysicsOptions.altitude = getFloatTextFieldValue (parentWin,
                                    GEOPHYSICS_OPTIONS_WIN_C2);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_OPTIONS_WIN_C4a);
   geophysicsOptions.drapedSurvey = (int) xvt_ctl_is_checked (win);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_OPTIONS_WIN_C3);
   geophysicsOptions.deformableRemanence = (int) xvt_ctl_is_checked (win);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_OPTIONS_WIN_C4);
   geophysicsOptions.deformableAnisotropy = (int) xvt_ctl_is_checked (win);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_MAG_COMPONENTS);
   geophysicsOptions.magneticVectorComponents = (int) xvt_ctl_is_checked (win);

   radioButtons[0]= xvt_win_get_ctl(parentWin, GEOPHYSICS_OPTIONS_SPATIAL);
   radioButtons[1]= xvt_win_get_ctl(parentWin, GEOPHYSICS_OPTIONS_SPECTRAL);
   for (buttonIndex = 0; buttonIndex < 2; buttonIndex++)
   {
      if (xvt_ctl_is_checked (radioButtons[buttonIndex]))
         break;
   }
   geophysicsOptions.calculationMethod = buttonIndex;

   geophysicsOptions.constantBoxDepth = getFloatTextFieldValue (parentWin,
                                    GEOPHYSICS_BOXING_CONSTANT);

   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_BOXING_CLEVER);
   geophysicsOptions.cleverBoxRatio = xvt_list_get_sel_index(win)+1.0;

   
   win = xvt_win_get_ctl (parentWin, GEOPHYSICS_PADDING_TYPE);
   geophysicsOptions.spectralPaddingType = xvt_list_get_sel_index (win);
   if (geophysicsOptions.spectralPaddingType == RECLECTION_PADDING)
      geophysicsOptions.spectralPercent = getIntegerTextFieldValue (parentWin,
                                    GEOPHYSICS_PADDING_PARAM);
   if ((geophysicsOptions.spectralPaddingType == FENCE_VALUE_PADDING) ||
       (geophysicsOptions.spectralPaddingType == SET_VALUE_PADDING))
      geophysicsOptions.spectralFence = getIntegerTextFieldValue (parentWin,
                                    GEOPHYSICS_PADDING_PARAM);

}
/* ======================================================================
FUNCTION        updateGeophysicsOptions
DESCRIPTION
        update the geophysics options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int 
#if XVT_CC_PROTO
updateGeophysicsOptions (WINDOW parentWin)
#else
updateGeophysicsOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   int numCubes;
   double range;
   char label[50];
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions();
   
   if (win = xvt_win_get_ctl (parentWin, GEOPHYSICS_PADDING_TYPE))
   {
      switch (xvt_list_get_sel_index (win))
      {
         case (RAMP_PADDING):
         case (FENCE_MEAN_PADDING):
         case (SET_MEAN_PADDING):
            xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, GEOPHYSICS_PADDING_PARAM_LABEL), FALSE);
            xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, GEOPHYSICS_PADDING_PARAM), FALSE);
            break;
         case (RECLECTION_PADDING):
            xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, GEOPHYSICS_PADDING_PARAM_LABEL), TRUE);
            xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, GEOPHYSICS_PADDING_PARAM), TRUE);
            updateNumericTextField (parentWin, GEOPHYSICS_PADDING_PARAM, geophysicsOptions.spectralPercent);
            break;
         case (FENCE_VALUE_PADDING):
         case (SET_VALUE_PADDING):
            xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, GEOPHYSICS_PADDING_PARAM_LABEL), TRUE);
            xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, GEOPHYSICS_PADDING_PARAM), TRUE);
            updateNumericTextField (parentWin, GEOPHYSICS_PADDING_PARAM, geophysicsOptions.spectralFence);
            break;
      }
   }
   
   range = getFloatTextFieldValue (parentWin, GEOPHYSICS_RANGE);
   if (range > 1.0)
   {
      numCubes = (int) floor (range/viewOptions->geophysicsCubeSize);
      win = xvt_win_get_ctl (parentWin, GEOPHYSICS_RANGE_IN_CUBES);
      sprintf (label, "(%d cubes in current view)", numCubes);
   }
   else
      strcpy (label, "");
   xvt_vobj_set_title (win, label);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        loadGeologyOptions
DESCRIPTION
        load the geology options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadGeologyOptions (WINDOW parentWin)
#else
loadGeologyOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   int state;

   if (!parentWin) return;

   win = updateFloatTextField (parentWin, WIN_116_EDIT_57,
                                    geologyOptions.chair_X, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, WIN_116_EDIT_59,
                                    geologyOptions.chair_Y, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, WIN_116_EDIT_58, 
                                    geologyOptions.chair_Z, 1);
   setControlFont (win);

   win = updateFloatTextField (parentWin, WIN_116_EDIT_76,
                                    geologyOptions.welllogAngle, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, WIN_116_EDIT_78,
                                    geologyOptions.welllogDeclination, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, WIN_116_EDIT_77, 
                                    geologyOptions.welllogDepth, 1);
   setControlFont (win);


   win = xvt_win_get_ctl (parentWin, WIN_116_CHECKBOX_68);
   xvt_ctl_set_checked(win, geologyOptions.useTopography);

   win = xvt_win_get_ctl (parentWin, GEOLOGY_OPTIONS_WINDOW_C1);
   xvt_ctl_set_checked(win, geologyOptions.calculateChairDiagram);
   state = geologyOptions.calculateChairDiagram;
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_GROUPBOX_50), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_TEXT_54), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_HSCROLL_51), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_EDIT_57), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_TEXT_55), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_HSCROLL_52), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_EDIT_59), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_TEXT_56), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_HSCROLL_53), (BOOLEAN) state);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_116_EDIT_58), (BOOLEAN) state);
}

/* ======================================================================
FUNCTION        saveGeologyOptions
DESCRIPTION
        save from the geology options window the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveGeologyOptions (WINDOW parentWin)
#else
saveGeologyOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;

   if (!parentWin) return;

   geologyOptions.chair_X = getFloatTextFieldValue (parentWin,
                                    WIN_116_EDIT_57);
   geologyOptions.chair_Y = getFloatTextFieldValue (parentWin,
                                    WIN_116_EDIT_59);
   geologyOptions.chair_Z = getFloatTextFieldValue (parentWin,
                                    WIN_116_EDIT_58);

   geologyOptions.welllogAngle = getFloatTextFieldValue (parentWin,
                                    WIN_116_EDIT_76);
   geologyOptions.welllogDeclination = getFloatTextFieldValue (parentWin,
                                    WIN_116_EDIT_78);
   geologyOptions.welllogDepth = getFloatTextFieldValue (parentWin,
                                    WIN_116_EDIT_77);


   win = xvt_win_get_ctl (parentWin, WIN_116_CHECKBOX_68);
   geologyOptions.useTopography = (int) xvt_ctl_is_checked (win);

   win = xvt_win_get_ctl (parentWin, GEOLOGY_OPTIONS_WINDOW_C1);
   geologyOptions.calculateChairDiagram = (int) xvt_ctl_is_checked (win);

   invalidateCurrentLineMap ();
   updateMenuOptions (TASK_MENUBAR, NULL_WIN);  /* needed in case topo selected */
}

/* ======================================================================
FUNCTION        load3dOptions
DESCRIPTION
        load the 3d options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
load3dOptions (WINDOW parentWin)
#else
load3dOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   OBJECT *strat;
   int i, numStrat;
   WINDOW stratOutputTypeWin;
   char layerName[100];
   LAYER_PROPERTIES *propertiesArray[50];
   int eventNumArray[50];

   if (!parentWin) return;

   win = updateFloatTextField (parentWin, THREED_DECLINATION,
                                    threedViewOptions.declination, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, THREED_ELEVATION,
                                    threedViewOptions.azimuth, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, THREED_SCALE,
                                    threedViewOptions.scale * 100.0, 1);  /* 10000 / three... */
   setControlFont (win);

                      /* List all the Layers that can be selected */
   assignEventNumForStratLayers (eventNumArray, 50);
   numStrat = assignPropertiesForStratLayers (propertiesArray, 50);
   win = xvt_win_get_ctl (parentWin, VIEW_OPTIONS_STRAT_LIST);
   xvt_list_suspend (win);
   for (i = 1; i < numStrat; i++)  /* Add the normal strat layers */
   {         /* note base layer not in list as cannot be seen */
      if (strat = (OBJECT *) nthObject (NULL_WIN, eventNumArray[i]))
      {
         sprintf(layerName, "%s - %s (%d)", strat->text,
                            propertiesArray[i]->unitName, i+1);
         xvt_list_add (win , i-1, layerName);
                   /* turn in on if it was on */
         if (threedViewOptions.layerOn[i])
            xvt_list_set_sel (win, i-1, TRUE);
         else
            xvt_list_set_sel (win, i-1, FALSE);
      }
   }
   for (i = 0; i < countObjects(NULL_WIN); i++)  /* Add the Fault break planes at the end */
   {
      strat = nthObject (NULL_WIN, i);
      if (strat->shape == FAULT)
      {
         sprintf(layerName, "%s - FAULT PLANE (%d)", strat->text, numStrat+1);
         xvt_list_add (win , numStrat-1, layerName);
                   /* turn in on if it was on */
         if (threedViewOptions.layerOn[numStrat])
            xvt_list_set_sel (win, numStrat-1, TRUE);
         else
            xvt_list_set_sel (win, numStrat-1, FALSE);

         numStrat++;
      }
   }
   xvt_list_resume (win);
   xvt_vobj_set_enabled (win, (BOOLEAN) (!threedViewOptions.allLayers));
   
   win = xvt_win_get_ctl (parentWin, VIEW_OPTIONS_ALL_SURFACES);
   xvt_ctl_set_checked (win, (BOOLEAN) threedViewOptions.allLayers);

   stratOutputTypeWin = xvt_win_get_ctl (parentWin, VIEW_STRAT_TYPE_LIST);
   xvt_list_suspend (stratOutputTypeWin);
   xvt_list_add (stratOutputTypeWin, 0, "XYZ Color");
   xvt_list_add (stratOutputTypeWin, 1, "Shading");
   xvt_list_add (stratOutputTypeWin, 2, "Lines");
   xvt_list_add (stratOutputTypeWin, 3, "File (DXF 3D Faces)");
   xvt_list_add (stratOutputTypeWin, 4, "File (DXF Polylines)");
   xvt_list_add (stratOutputTypeWin, 5, "File (Vulcan)");
   xvt_list_set_sel (stratOutputTypeWin, threedViewOptions.fillType, TRUE);
   xvt_list_resume (stratOutputTypeWin);

         /* Disable Block View button if we have already come from their */
   if (stackParentWin(parentWin) != getEventDrawingWindow())
      if (win = xvt_win_get_ctl(parentWin, VIEW_BLOCK_VIEW_OPTIONS))
         xvt_vobj_set_enabled (win, FALSE);
}

/* ======================================================================
FUNCTION        save3dOptions
DESCRIPTION
        save the 3d options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
save3dOptions (WINDOW parentWin)
#else
save3dOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   int i;
   WINDOW stratOutputTypeWin;

   if (!parentWin) return;

   threedViewOptions.declination = getFloatTextFieldValue (parentWin,
                                    THREED_DECLINATION);
   threedViewOptions.azimuth = getFloatTextFieldValue (parentWin,
                                    THREED_ELEVATION);
   threedViewOptions.scale = getFloatTextFieldValue (parentWin,
                                    THREED_SCALE);
   threedViewOptions.scale = threedViewOptions.scale / 100.0;

   win = xvt_win_get_ctl (parentWin, VIEW_OPTIONS_STRAT_LIST);
   threedViewOptions.layerOn[0] = FALSE;  /* base layer can not be seen */
   for (i = 0; i < xvt_list_count_all(win); i++)
      threedViewOptions.layerOn[i+1] = xvt_list_is_sel (win, i);

   win = xvt_win_get_ctl (parentWin, VIEW_OPTIONS_ALL_SURFACES);
   threedViewOptions.allLayers = xvt_ctl_is_checked (win);

   stratOutputTypeWin = xvt_win_get_ctl (parentWin, VIEW_STRAT_TYPE_LIST);
   threedViewOptions.fillType = xvt_list_get_sel_index (stratOutputTypeWin);
}

/* ======================================================================
FUNCTION        setGeologyTopoRange
DESCRIPTION
        set the label showing the current range of topo file
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
setGeologyTopoRange (WINDOW parentWin)
#else
setGeologyTopoRange (parentWin)
WINDOW parentWin;
#endif
{
   char text[100];
   WINDOW win;
   double offset, scale;


   scale = getFloatTextFieldValue (parentWin,
                                   TOPO_OPTIONS_SCALE);
   offset= getFloatTextFieldValue (parentWin,
                                   TOPO_OPTIONS_OFFSET);

   sprintf (text, "Adjusted Range %.2f - %.2f",
                                  (minTopoValue*scale)+offset,
                                  (maxTopoValue*scale)+offset);
   win = xvt_win_get_ctl (parentWin, TOPO_OPTIONS_ADJ_RANGE);
   xvt_vobj_set_title (win, text);   

}

/* ======================================================================
FUNCTION        loadGeologyTopoOptions
DESCRIPTION
        load the geology topography options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadGeologyTopoOptions (WINDOW parentWin)
#else
loadGeologyTopoOptions (parentWin)
WINDOW parentWin;
#endif
{
   char text[100];
   WINDOW win;

   sprintf (text, "Topography File Range is %.2f - %.2f",
                                               minTopoValue, maxTopoValue);
   win = xvt_win_get_ctl (parentWin, TOPO_OPTIONS_FULL_RANGE);
   xvt_vobj_set_title (win, text);
   
   win = updateFloatTextField (parentWin, TOPO_OPTIONS_SCALE,
                                    geologyOptions.topoScaleFactor, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, TOPO_OPTIONS_OFFSET,
                                    geologyOptions.topoOffsetFactor, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, TOPO_OPTIONS_CONTOUR,
                                    geologyOptions.topoLowContour, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, TOPO_OPTIONS_INTERVAL,
                                    geologyOptions.topoContourInterval, 1);
   setControlFont (win);

   setGeologyTopoRange (parentWin);

}
/* ======================================================================
FUNCTION        saveGeologyTopoOptions
DESCRIPTION
        save the geology topography options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveGeologyTopoOptions (WINDOW parentWin)
#else
saveGeologyTopoOptions (parentWin)
WINDOW parentWin;
#endif
{
   int jj, kk;

   geologyOptions.topoScaleFactor = getFloatTextFieldValue (parentWin,
                                          TOPO_OPTIONS_SCALE);
   geologyOptions.topoOffsetFactor = getFloatTextFieldValue (parentWin,
                                          TOPO_OPTIONS_OFFSET);
   geologyOptions.topoLowContour = getFloatTextFieldValue (parentWin,
                                          TOPO_OPTIONS_CONTOUR);
   geologyOptions.topoContourInterval = getFloatTextFieldValue (parentWin,
                                          TOPO_OPTIONS_INTERVAL);

                      /* OK must have been pressed so we will also
                      ** scale the values with what has been entered */
   if (!topographyMap)
      return;
   for (jj = 1; jj <= TopoRow; jj++)
   {
      for (kk = 1; kk <= TopoCol; kk++)
      {
         topographyMap[kk][jj] = geologyOptions.topoOffsetFactor
                    + (topographyMap[kk][jj] * geologyOptions.topoScaleFactor);
      }
   }
   minTopoValue = geologyOptions.topoOffsetFactor
                               + (minTopoValue*geologyOptions.topoScaleFactor);
   maxTopoValue = geologyOptions.topoOffsetFactor
                               + (maxTopoValue*geologyOptions.topoScaleFactor);
}

/* ======================================================================
FUNCTION        loadGeologyMovieOptions
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadGeologyMovieOptions (WINDOW parentWin)
#else
loadGeologyMovieOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;

   win = xvt_win_get_ctl (parentWin, MOVIE_TYPE);
   xvt_list_suspend (win);
   xvt_list_add (win , 0, "Block Diagram");
   xvt_list_add (win , 1, "Map (Solid Color)");
   xvt_list_add (win , 2, "Map (Lines)");
   xvt_list_add (win , 3, "Section (Solid Color)");
   xvt_list_add (win , 4, "Section (Lines)");
   xvt_list_add (win , 5, "Well Log");
   xvt_list_add (win , 6, "3D Strat");
   xvt_list_add (win , 7, "Gravity Image");
   xvt_list_add (win , 8, "Magnetics Image");
   xvt_list_set_sel (win, projectOptions.movieType, TRUE);
   xvt_list_resume (win);

   win = updateNumericTextField (parentWin, MOVIE_OPTIONS_FRAMES,
                                    projectOptions.movieFramesPerEvent);
   setControlFont (win);

   win = updateFloatTextField (parentWin, MOVIE_OPTIONS_SPEED,
                                    projectOptions.moviePlaySpeed, 1);
   setControlFont (win);
}

/* ======================================================================
FUNCTION        saveGeologyMovieOptions
DESCRIPTION
        save the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveGeologyMovieOptions (WINDOW parentWin)
#else
saveGeologyMovieOptions (parentWin)
WINDOW parentWin;
#endif
{
   projectOptions.movieType = xvt_list_get_sel_index (xvt_win_get_ctl(parentWin, MOVIE_TYPE));
   projectOptions.movieFramesPerEvent = getIntegerTextFieldValue (parentWin,
                                          MOVIE_OPTIONS_FRAMES);
   projectOptions.moviePlaySpeed = getFloatTextFieldValue (parentWin,
                                          MOVIE_OPTIONS_SPEED);
}

/* ======================================================================
FUNCTION        loadGeophysicsImageOptions
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadGeophysicsImageOptions (WINDOW parentWin)
#else
loadGeophysicsImageOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   IMAGE_DISPLAY_DATA *imageDisplayData;
   double minClip, maxClip, percentInc;
   int lutType, decimalPlaces;
   ANOMIMAGE_DATA *anomImageData;
   double minImageValue, maxImageValue;
   
   imageDisplayData = (IMAGE_DISPLAY_DATA *) xvt_vobj_get_data (parentWin);
   if (!imageDisplayData)
      return;

   if (lastActiveWindow && (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (lastActiveWindow)))
   {
      minImageValue = anomImageData->minValue;
      maxImageValue = anomImageData->maxValue;
   }
   else
   {
      minImageValue = 0;
      maxImageValue = 100;
   }

   win = xvt_win_get_ctl (parentWin, IMAGE_CLIPPING_TYPE);
   xvt_list_suspend (win);
   xvt_list_add (win , 0, "Relative Clipping");
   xvt_list_add (win , 1, "Absolute Clipping");
   xvt_list_set_sel (win, imageDisplayData->clippingType, TRUE);
   xvt_list_resume (win);
   
   switch (imageDisplayData->clippingType)
   {
      case (RELATIVE_CLIPPING):
         percentInc = (maxImageValue - minImageValue)/100.0;

         minClip = (imageDisplayData->minClip - minImageValue)/percentInc;
         maxClip = (imageDisplayData->maxClip - minImageValue)/percentInc;

         if (minClip < 0.0) minClip = 0.0;
         if (maxClip < 0.0) maxClip = 0.0;
         if (maxClip > 100.0) maxClip = 100.0;
         if (minClip > 100.0) minClip = 100.0;
         decimalPlaces = 1;
         xvt_vobj_set_visible (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT), TRUE);
         xvt_vobj_set_visible (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MAXCLIP_UNIT), TRUE);
         xvt_vobj_set_data (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT), (long) RELATIVE_CLIPPING);
         break;
      case (ABSOLUTE_CLIPPING):
         minClip = imageDisplayData->minClip;
         maxClip = imageDisplayData->maxClip;
         decimalPlaces = 3;
         xvt_vobj_set_visible (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT), FALSE);
         xvt_vobj_set_visible (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MAXCLIP_UNIT), FALSE);
         xvt_vobj_set_data (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT), (long) ABSOLUTE_CLIPPING);
         break;
   }

   win = updateFloatTextField (parentWin, IMAGE_RANGE_MIN, minClip, decimalPlaces);
   setControlFont (win);
   win = updateFloatTextField (parentWin, IMAGE_RANGE_MAX, maxClip, decimalPlaces);
   setControlFont (win);

   updateScrollAndText(parentWin, IMAGE_RANGE_MIN_BAR, IMAGE_RANGE_MIN, FALSE);
   updateScrollAndText(parentWin, IMAGE_RANGE_MAX_BAR, IMAGE_RANGE_MAX, FALSE);
   
   if (imageDisplayData->display == GREY_COLOR)
      lutType = IMAGE_LUT_GREY;
   else if (imageDisplayData->display == PSEUDO_COLOR)
      lutType = IMAGE_LUT_PSEUDO;
   else if (imageDisplayData->display == CONTOUR_IMAGE)
      lutType = IMAGE_LUT_CONTOUR;
   xdCheckRadioButton(parentWin, lutType,
                      IMAGE_LUT_GREY, IMAGE_LUT_CONTOUR);

   updateNumericTextField (parentWin, IMAGE_NUM_CONTOUR, imageDisplayData->numContours);

#ifndef ENCOM_V6
	xvt_ctl_set_checked(xvt_win_get_ctl(parentWin, IMAGE_GRID), imageDisplayData->gridOverlay);
	xvt_ctl_set_checked(xvt_win_get_ctl(parentWin, IMAGE_GRID_AUTO), imageDisplayData->autoGridSpacing);
	xvt_ctl_set_checked(xvt_win_get_ctl(parentWin, IMAGE_GRID_SENSOR), imageDisplayData->sensorOverlay);
   win = updateFloatTextField (parentWin, IMAGE_GRID_XSTART, imageDisplayData->gridStartX, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, IMAGE_GRID_YSTART, imageDisplayData->gridStartY, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, IMAGE_GRID_XINC, imageDisplayData->gridIncX, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, IMAGE_GRID_YINC, imageDisplayData->gridIncY, 1);
   setControlFont (win);
#endif

   updateGeophysicsImageOptions(parentWin);
}

/* ======================================================================
FUNCTION        saveGeophysicsImageOptions
DESCRIPTION
        save the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveGeophysicsImageOptions (WINDOW parentWin)
#else
saveGeophysicsImageOptions (parentWin)
WINDOW parentWin;
#endif
{
   IMAGE_DISPLAY_DATA *imageDisplayData;
   double minClip, maxClip, percentInc;
   ANOMIMAGE_DATA *anomImageData;
   double minImageValue, maxImageValue;
   WIN_TYPE winType;
   
   imageDisplayData = (IMAGE_DISPLAY_DATA *) xvt_vobj_get_data (parentWin);
   if (!imageDisplayData)
      return;

   xvt_vobj_set_visible(parentWin, FALSE);

   if (lastActiveWindow && (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (lastActiveWindow)))
   {
      minImageValue = anomImageData->minValue;
      maxImageValue = anomImageData->maxValue;
   }
   else
   {
      minImageValue = 0;
      maxImageValue = 100;
   }
      
   imageDisplayData->clippingType = xvt_list_get_sel_index (xvt_win_get_ctl(parentWin, IMAGE_CLIPPING_TYPE));
   minClip = getFloatTextFieldValue (parentWin, IMAGE_RANGE_MIN);
   maxClip = getFloatTextFieldValue (parentWin, IMAGE_RANGE_MAX);
   switch (imageDisplayData->clippingType)
   {
      case (RELATIVE_CLIPPING):
         percentInc = (maxImageValue - minImageValue)/100.0;
         minClip = minImageValue + percentInc*minClip;
         maxClip = minImageValue + percentInc*maxClip;
         imageDisplayData->minClip = minClip;
         imageDisplayData->maxClip = maxClip;
         break;
      case (ABSOLUTE_CLIPPING):
         imageDisplayData->minClip = minClip;
         imageDisplayData->maxClip = maxClip;
         break;
   }

   if (xvt_ctl_is_checked (xvt_win_get_ctl (parentWin, IMAGE_LUT_GREY)))
      imageDisplayData->display = GREY_COLOR;
   else if (xvt_ctl_is_checked (xvt_win_get_ctl (parentWin, IMAGE_LUT_PSEUDO)))
      imageDisplayData->display = PSEUDO_COLOR;
   else
      imageDisplayData->display = CONTOUR_IMAGE;

   imageDisplayData->numContours = getIntegerTextFieldValue (parentWin, IMAGE_NUM_CONTOUR);
#ifndef ENCOM_V6
	imageDisplayData->gridOverlay = xvt_ctl_is_checked(xvt_win_get_ctl(parentWin, IMAGE_GRID));
	imageDisplayData->autoGridSpacing = xvt_ctl_is_checked(xvt_win_get_ctl(parentWin, IMAGE_GRID_AUTO));
	imageDisplayData->sensorOverlay = xvt_ctl_is_checked(xvt_win_get_ctl(parentWin, IMAGE_GRID_SENSOR));
   imageDisplayData->gridStartX = getFloatTextFieldValue (parentWin, IMAGE_GRID_XSTART);
   imageDisplayData->gridStartY = getFloatTextFieldValue (parentWin, IMAGE_GRID_YSTART);
   imageDisplayData->gridIncX = getFloatTextFieldValue (parentWin, IMAGE_GRID_XINC);
   imageDisplayData->gridIncY = getFloatTextFieldValue (parentWin, IMAGE_GRID_YINC);
#endif
         
   winType = xvt_vobj_get_type(parentWin);
   xvt_vobj_destroy(parentWin);
                      /* Update the anom image that the menu was called from */
   if (winType == W_DBL)
   {
#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
      xvt_win_set_cursor (TASK_WIN, CURSOR_WAIT);
#endif      
      updateAnomaliesImageDisplay (lastActiveWindow);
#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
      xvt_win_set_cursor (TASK_WIN, CURSOR_ARROW);
#endif      
   }
}

/* ======================================================================
FUNCTION        updateGeophysicsImageOptions
DESCRIPTION
        update labels showing what is happening to the image
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateGeophysicsImageOptions (WINDOW parentWin)
#else
updateGeophysicsImageOptions (parentWin)
WINDOW parentWin;
#endif
{
   int contourImage, numContours; 
   int clippingType, oldClippingType, gridOn, autoGrid, enableGridPos;
   double minClip, maxClip, minClipValue, clipRange;
   double maxClipValue, minClipPercent, maxClipPercent;
   char label[100];
   double percentInc;
   ANOMIMAGE_DATA *anomImageData;
   double minImageValue, maxImageValue;

   if (lastActiveWindow && (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (lastActiveWindow)))
   {
      minImageValue = anomImageData->minValue;
      maxImageValue = anomImageData->maxValue;
   }
   else
   {
      minImageValue = 0;
      maxImageValue = 100;
   }
      
   clippingType = (int) xvt_list_get_sel_index (xvt_win_get_ctl(parentWin, IMAGE_CLIPPING_TYPE));
   oldClippingType = (int) xvt_vobj_get_data (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT));
   minClip = getFloatTextFieldValue (parentWin, IMAGE_RANGE_MIN);
   maxClip = getFloatTextFieldValue (parentWin, IMAGE_RANGE_MAX);
   
   percentInc = (maxImageValue - minImageValue)/100.0;

                   /* Convert the co-ordinates as the type has changed */
   if (clippingType != oldClippingType)
   { 
      switch (clippingType)
      {
         case (RELATIVE_CLIPPING):   /* was Absolute */
            minClipValue = minClip;
            maxClipValue = maxClip;

            minClipPercent = (minClipValue - minImageValue)/percentInc;
            maxClipPercent = (maxClipValue - minImageValue)/percentInc;

            setScrollRange (parentWin, IMAGE_RANGE_MAX_BAR, 0, 100);
            setScrollRange (parentWin, IMAGE_RANGE_MIN_BAR, 0, 100);
            xvt_vobj_set_visible (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT), TRUE);
            xvt_vobj_set_visible (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MAXCLIP_UNIT), TRUE);
            xvt_vobj_set_data (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT), (long) RELATIVE_CLIPPING);

            if (minClipPercent < 0.0) minClipPercent = 0.0;
            if (minClipPercent > 100.0) minClipPercent = 100.0;
            if (maxClipPercent > 100.0) maxClipPercent = 100.0;
            if (maxClipPercent < 0.0) maxClipPercent = 0.0;
            updateFloatTextField (parentWin, IMAGE_RANGE_MIN, minClipPercent, 1);
            updateFloatTextField (parentWin, IMAGE_RANGE_MAX, maxClipPercent, 1);
            break;
         case (ABSOLUTE_CLIPPING):   /* was Relative */
            minClipValue = minImageValue + percentInc*minClip;
            maxClipValue = minImageValue + percentInc*maxClip;
            minClipPercent = minClip;
            maxClipPercent = maxClip;
            setScrollRange (parentWin, IMAGE_RANGE_MAX_BAR, (int) floor(minImageValue), (int) ceil(maxImageValue));
            setScrollRange (parentWin, IMAGE_RANGE_MIN_BAR, (int) floor(minImageValue), (int) ceil(maxImageValue));
            xvt_vobj_set_visible (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT), FALSE);
            xvt_vobj_set_visible (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MAXCLIP_UNIT), FALSE);
            xvt_vobj_set_data (xvt_win_get_ctl(parentWin, IMAGE_OPTION_MINCLIP_UNIT), (long) ABSOLUTE_CLIPPING);

            updateFloatTextField (parentWin, IMAGE_RANGE_MIN, minClipValue, 3);
            updateFloatTextField (parentWin, IMAGE_RANGE_MAX, maxClipValue, 3);
            break;
      }
   }
   else  /* Hasn't changed */
   {
      switch (clippingType)
      {
         case (RELATIVE_CLIPPING):   /* was Relative */
            minClipValue = minImageValue + percentInc*minClip;
            maxClipValue = minImageValue + percentInc*maxClip;
            setScrollRange (parentWin, IMAGE_RANGE_MAX_BAR, 0, 100);
            setScrollRange (parentWin, IMAGE_RANGE_MIN_BAR, 0, 100);
            break;
         case (ABSOLUTE_CLIPPING):   /* was Absolute*/
            minClipValue = minClip;
            maxClipValue = maxClip;
            setScrollRange (parentWin, IMAGE_RANGE_MAX_BAR, (int) floor(minImageValue), (int) ceil(maxImageValue));
            setScrollRange (parentWin, IMAGE_RANGE_MIN_BAR, (int) floor(minImageValue), (int) ceil(maxImageValue));
            break;
      }
   }
   updateScrollAndText(parentWin, IMAGE_RANGE_MIN_BAR, IMAGE_RANGE_MIN, FALSE);
   updateScrollAndText(parentWin, IMAGE_RANGE_MAX_BAR, IMAGE_RANGE_MAX, FALSE);
   clipRange = maxClipValue - minClipValue;


   contourImage = xvt_ctl_is_checked (xvt_win_get_ctl (parentWin, IMAGE_LUT_CONTOUR));
   xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, IMAGE_NUM_CONTOUR_LABEL), (BOOLEAN) contourImage);
   xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, IMAGE_NUM_CONTOUR), (BOOLEAN) contourImage);

                          /* Range Check for the number of contours */
   if (contourImage)
   {
      numContours = getIntegerTextFieldValue (parentWin, IMAGE_NUM_CONTOUR);
      if (numContours < 1)
      {
         numContours = 1;
         updateNumericTextField (parentWin, IMAGE_NUM_CONTOUR, numContours);
      }
      if (numContours > 50)
      {
         numContours = 50;
         updateNumericTextField (parentWin, IMAGE_NUM_CONTOUR, numContours);
      }
   }

                            /* We can give a real range on the clipping */
   if (xvt_vobj_get_type(parentWin) == W_DBL)
   {
      if (clipRange < 5.0)
         sprintf (label, "Display Range: %.3f to %.3f", minClipValue, maxClipValue);
      else
         sprintf (label, "Display Range: %.1f to %.1f", minClipValue, maxClipValue);
   }
   else  /* Just say way it is an percentages */
      sprintf (label, "Display Range: %.1f to %.1f %%", minClip, maxClip);
   xvt_vobj_set_title (xvt_win_get_ctl (parentWin, IMAGE_RANGE_LABEL), label);

#ifndef ENCOM_V6
	gridOn = xvt_ctl_is_checked(xvt_win_get_ctl(parentWin, IMAGE_GRID));
	autoGrid = xvt_ctl_is_checked(xvt_win_get_ctl(parentWin, IMAGE_GRID_AUTO));
	if (gridOn && !autoGrid)
		enableGridPos = TRUE;
	else
		enableGridPos = FALSE;
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_AUTO), (BOOLEAN) gridOn); 
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_XSTART), (BOOLEAN) enableGridPos); 
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_YSTART), (BOOLEAN) enableGridPos); 
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_XINC), (BOOLEAN) enableGridPos); 
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_YINC), (BOOLEAN) enableGridPos); 
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_XSTART_LABEL), (BOOLEAN) enableGridPos); 
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_YSTART_LABEL), (BOOLEAN) enableGridPos); 
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_XINC_LABEL), (BOOLEAN) enableGridPos); 
	xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, IMAGE_GRID_YINC_LABEL), (BOOLEAN) enableGridPos); 
#endif
}

/* ======================================================================
FUNCTION        loadBlockImageLayerList
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
static int
#if XVT_CC_PROTO
loadBlockImageLayerList (WINDOW layerList, BLOCK_DIAGRAM_DATA *diagram)
#else
loadBlockImageLayerList (layerList, diagram)
WINDOW layerList;
BLOCK_DIAGRAM_DATA *diagram;
#endif
{
   LAYER_PROPERTIES *layerProps[100];
   int numLayers, layer, temp, select;

   numLayers = assignPropertiesForStratLayers (layerProps, 100);
   xvt_list_suspend (layerList);
   xvt_list_clear (layerList);
   for (layer = 0; layer < numLayers; layer++)
   {
      xvt_list_add (layerList, layer, layerProps[layer]->unitName);
      if (diagram->layersToDraw)
      {
         for (select = FALSE, temp = 0; temp < diagram->numLayersToDraw; temp++)
         {
            if (layerProps[layer] == diagram->layersToDraw[temp])
            {
               select = TRUE;
               break;
            }
         }
      }
      xvt_list_set_sel(layerList, layer, (BOOLEAN) select);
   }
   xvt_list_resume (layerList);

   return (TRUE);
}

/* ======================================================================
FUNCTION        drawBlockImageColorScale
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
static int
#if XVT_CC_PROTO
getBlockImageColorBarPos (WINDOW win, RCT *barPos)
#else
getBlockImageColorBarPos (win, barPos)
WINDOW win;
RCT *barPos;
#endif
{
   WINDOW layerListGB, displayTypeGB;
   RCT layerListRect, displayTypeRect;

   if (!barPos)
      return (FALSE);

   layerListGB = xvt_win_get_ctl (win, LAYER_DISPLAY_LAYER_GB);
   displayTypeGB = xvt_win_get_ctl (win, LAYER_DISPLAY_DISPLAY_GB);
   xvt_vobj_get_outer_rect (layerListGB, &layerListRect);
   xvt_vobj_get_outer_rect (displayTypeGB, &displayTypeRect);
   *barPos = layerListRect;
   barPos->top += 5;
   barPos->left = displayTypeRect.right + 5;
   barPos->right = layerListRect.left - 5;

   return (TRUE);
}

int
#if XVT_CC_PROTO
drawBlockImageColorScale (WINDOW win, COLOR *lut, RCT *barPos, int updateLabels)
#else
drawBlockImageColorScale (win, lut, barPos, updateLabels)
WINDOW win;
COLOR *lut;
RCT *barPos;
int updateLabels;
#endif
{
   RCT rect, tempPos;
   DRAW_CTOOLS tools;
   double length, lutLength, unitLength, dStartIndex, dEndIndex;
   int pos, startIndex, endIndex, blockType;
   LAYER_PROPERTIES *layerProps[100];
   int numLayers, layer, allLayers;
   WINDOW layerList = xvt_win_get_ctl(win, LAYER_DISPLAY_LIST);
   char label[20];
   
   if (!barPos)
   {  
      getBlockImageColorBarPos (win, &tempPos);
      barPos = &tempPos;
   }

   length = xvt_rect_get_height(barPos); 
   xvt_dwin_get_draw_ctools(win, &tools);
   tools.pen.width = 0;

   blockType = xvt_list_get_sel_index(xvt_win_get_ctl(win, LAYER_DISPLAY_TYPE));

   if (blockType > 1)  /* Propties block view */
   {
      if (!lut)
         return (FALSE);
      
      lutLength = length/100.0;
      rect.left = barPos->left;
      rect.right = barPos->left + (barPos->right - barPos->left)/2;

      if (lut[0] != COLOR_INVALID)
         startIndex = 1;
      else
         startIndex = 0;
      endIndex = 100;

      for (pos = 1; pos <= 100; pos++)
      {
         tools.pen.color = tools.brush.color = lut[pos-1];
         if (tools.pen.color == COLOR_INVALID)
         {
            tools.pen.color = tools.brush.color = COLOR_BLACK;
                  /* Move down to just before first blank found */
            if (startIndex && (endIndex == 100))
               endIndex = pos-1;
         }
         else
         {
            if (!startIndex)  /* set start to first color found */
               startIndex = pos;
         }
         xvt_dwin_set_draw_ctools(win, &tools);
      
         rect.bottom = barPos->bottom - (int) ((pos-1)*lutLength);
         rect.top = barPos->bottom - (int) (pos*lutLength);

         xvt_dwin_draw_rect(win, &rect);
      }
         
      if (updateLabels)
      {
         BLOCK_DIAGRAM_DATA *diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data(win);
            
         if (diagram && diagram->valueData)
         {
            double valueInc;
               
            valueInc = (diagram->maxValueData - diagram->minValueData)/100.0;
               
            dStartIndex = diagram->minValueData + ((double) startIndex)*valueInc;
            dEndIndex = diagram->minValueData + ((double) endIndex)*valueInc;
            updateFloatTextField (win, LAYER_LUT_START, dStartIndex, 3);
            updateFloatTextField (win, LAYER_LUT_END, dEndIndex, 3);
         }
         else
         {
            sprintf(label,"%d",startIndex);
            xvt_vobj_set_title(xvt_win_get_ctl(win, LAYER_LUT_START), label);
            sprintf(label,"%d",endIndex);
            xvt_vobj_set_title(xvt_win_get_ctl(win, LAYER_LUT_END), label);
         }
      }
   }
   
   

         /* Draw in the Layer colors and show which are selected */
   if (blockType == 0)
      allLayers = TRUE;  /* Full Block */
   else
      allLayers = xvt_ctl_is_checked(xvt_win_get_ctl(win, LAYER_DISPLAY_ALL));

   tools.pen.width = 2;
   numLayers = assignPropertiesForStratLayers (layerProps, 100);
   unitLength = length/numLayers;
   if (blockType > 1)  /* Propties block view */
   {
      rect.left = barPos->left + (barPos->right-barPos->left)/2;
      rect.right = barPos->right;
   }
   else  /* Just showing the layers without any lut */
   {
      rect.left = barPos->left;
      rect.right = barPos->right;
   }
   for (layer = 0; layer < numLayers; layer++)
   {
      tools.pen.color = tools.brush.color = XVT_MAKE_COLOR(layerProps[layer]->color.red,
                                                           layerProps[layer]->color.green,
                                                           layerProps[layer]->color.blue);
      if (allLayers || xvt_list_is_sel (layerList, layer))
         tools.pen.color = COLOR_BLACK;
      else
         tools.pen.color = tools.brush.color;
      xvt_dwin_set_draw_ctools(win, &tools);

      rect.top = barPos->top + (int) (layer*unitLength);
      rect.bottom = barPos->top + (int) ((layer+1)*unitLength);
      xvt_dwin_draw_rect(win, &rect);
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        getIndexBlockImageColorScale
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
static double
#if XVT_CC_PROTO
getIndexBlockImageColorScale (RCT *barPos, PNT pnt)
#else
getIndexBlockImageColorScale (barPos, pnt)
RCT *barPos;
PNT pnt;
#endif
{
   if (xvt_rect_has_point(barPos, pnt))
   {
      double index;
      
      index = (double) ((double) (pnt.v - barPos->top)
            /((double) xvt_rect_get_height(barPos)));
      
      index = (1.0 - index) * (double) NUM_PROPERTIES_LUT;
      
      if (index < 0.0) index = 0.0;
      else if (index > NUM_PROPERTIES_LUT) index = (double) NUM_PROPERTIES_LUT;
      
      return (index);
   }
   else
		return (-1.0);
}

/* ======================================================================
FUNCTION        setBlockImageColorIndex
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
setBlockImageColorIndex (WINDOW win, PNT *point, double *minValue, double *maxValue)
#else
setBlockImageColorIndex (win, point, minValue, maxValue)
WINDOW win;
PNT *point;
double *minValue, *maxValue;
#endif
{
   BLOCK_DIAGRAM_DATA *diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data(win);
   int startIndex, endIndex, index, midIndex, lutSize, updateLabels;
   WINDOW typeList;
   double newIndex;
   RCT barPos;

	if (!diagram->lut)
      return (FALSE);
      
   if (diagram->valueData && minValue)
      *minValue = *minValue *
                 ((diagram->maxValueData - diagram->minValueData)/100.0);

   if (diagram->valueData && maxValue)
      *maxValue = *maxValue *
                 ((diagram->maxValueData - diagram->minValueData)/100.0);
   
   typeList = xvt_win_get_ctl (win, LAYER_DISPLAY_TYPE);
   if (xvt_list_get_sel_index (typeList) < 2)
      return (FALSE);   
   
   for (index = 0; index < diagram->lutSize; index++)
   {
      if (diagram->lut[index] != COLOR_INVALID)
      {
         startIndex = index;
         break;
      }
   }
      
   for (index = diagram->lutSize-1; index >= 0; index--)
   {
      if (diagram->lut[index] != COLOR_INVALID)
      {
         endIndex = index;
         break;
      }
   }

   midIndex = startIndex + ((endIndex - startIndex) / 2);

                 /* Get the index of location clicked */
   getBlockImageColorBarPos(win, &barPos);
   if (point)
   {
      newIndex = getIndexBlockImageColorScale (&barPos, *point);
      updateLabels = TRUE;
   }
   else
   {
      if (minValue)
         newIndex = *minValue;
      else if (maxValue)
         newIndex = *maxValue;
      else
         minValue = 0;
      updateLabels = FALSE;
   }
      
   if (newIndex < 0.0)
      return (FALSE);

   if (minValue || maxValue)   
   {
      if (minValue)
         startIndex = (int) floor (*minValue+0.5);
         
      if (maxValue)
         endIndex = (int) floor (*maxValue+0.5);
         
      if (startIndex > endIndex)
         return (FALSE);
   }
   else
   {
      if (newIndex < midIndex)
         startIndex = (int) floor (newIndex+0.5);
      else 
         endIndex = (int) floor (newIndex+0.5);
   }
   
   if (xvt_list_get_sel_index(xvt_win_get_ctl(win, LAYER_LUT_STRETCH)) == 0)
   {                                /* Stretch */
      lutSize = endIndex - startIndex + 1;
      if ((startIndex + lutSize) > diagram->lutSize)
         lutSize = diagram->lutSize - startIndex;
      memset(diagram->lut, (int) 0xFFFF, diagram->lutSize*sizeof(COLOR));
      assignPseudoLut(NULL, &(diagram->lut[startIndex]), lutSize,
            diagram->lutSize, 0, lutSize, PROPERTIES_COLORS_SAT);
   }
   else  /* Clip */
   {
      COLOR *lut;

      if (lut = (COLOR *) initBlockImageColorLut (diagram->lutSize))
      {
         destroy1DArray((char *) diagram->lut);
         diagram->lut = lut;
      }
      for (index = 0; index < startIndex; index++)
         lut[index] = COLOR_INVALID;
      for (index = diagram->lutSize-1; index > endIndex; index--)
         lut[index] = COLOR_INVALID;
   }

   drawBlockImageColorScale (win, diagram->lut, &barPos, updateLabels);

   return (TRUE);
}

/* ======================================================================
FUNCTION        initBlockImageColorLut
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
COLOR *
#if XVT_CC_PROTO
initBlockImageColorLut (int size)
#else
initBlockImageColorLut (size)
int size;
#endif
{
   int numLut = 0, tempSize;
   COLOR *lut = NULL;
   
   if (!size)
      return (lut);
   numLut = size;
   if (lut = (COLOR *) create1DArray(numLut+1, sizeof(COLOR)))
   {
      int startIndex, endIndex;
               
      memset(lut, (int) 0xFFFF, numLut*sizeof(COLOR));
         
      startIndex = 0;
      endIndex = size;
      tempSize = endIndex - startIndex; 
                  
      assignPseudoLut(NULL, &(lut[startIndex]), tempSize,
                      NUM_PROPERTIES_COLORS, 0, tempSize,
                      PROPERTIES_COLORS_SAT);
   }

   return (lut);
}

/* ======================================================================
FUNCTION        calcBlockImageValueData
DESCRIPTION
        calculate the valueData for the specific dataType
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
calcBlockImageValueData (BLOCK_DIAGRAM_DATA *diagram, int dataType)
#else
calcBlockImageValueData (diagram, dataType)
BLOCK_DIAGRAM_DATA *diagram;
int dataType;
#endif
{
   float value, minValue, maxValue;
   int x, y, z, calcValueData = TRUE;
   PROPERTY_TYPE calcType;

   switch (dataType)
   {
      case (DENSITY_BLOCK): /* Density */
         if ((diagram->type != DENSITY_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = DENSITY_BLOCK;
         calcType = DENSITY_PROP;
         break;
      case (SUS_X_BLOCK): /* Sus X */
         if ((diagram->type != SUS_X_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = SUS_X_BLOCK;      
         calcType = SUS_X_PROP;
         break;
      case (SUS_Y_BLOCK): /* Sus Y */
         if ((diagram->type != SUS_Y_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = SUS_Y_BLOCK;      
         calcType = SUS_Y_PROP;
         break;
      case (SUS_Z_BLOCK): /* Sus Z */
         if ((diagram->type != SUS_Z_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = SUS_Z_BLOCK;      
         calcType = SUS_Z_PROP;
         break;
      case (SUS_DIP_BLOCK): /* Sus Dip */
         if ((diagram->type != SUS_DIP_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = SUS_DIP_BLOCK;      
         calcType = SUS_DIP_PROP;
         break;
      case (SUS_DDIR_BLOCK): /* Sus Dip Direction */
         if ((diagram->type != SUS_DDIR_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = SUS_DDIR_BLOCK;
         calcType = SUS_DDIR_PROP;
         break;
      case (SUS_PITCH_BLOCK): /* Sus Pitch */
         if ((diagram->type != SUS_PITCH_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = SUS_PITCH_BLOCK;      
         calcType = SUS_PITCH_PROP;
         break;
      case (REM_STR_BLOCK): /* Rem Intensity */
         if ((diagram->type != REM_STR_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = REM_STR_BLOCK;      
         calcType = REM_STR_PROP;
         break;
      case (REM_INC_BLOCK): /* Rem Inclination */
         if ((diagram->type != REM_INC_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = REM_INC_BLOCK;
         calcType = REM_INC_PROP;
         break;
      case (REM_DEC_BLOCK): /* Rem Declination */
         if ((diagram->type != REM_DEC_BLOCK) || (!diagram->valueData))
            calcValueData = TRUE;
         else
            calcValueData = FALSE;
         diagram->type = REM_DEC_BLOCK;      
         calcType = REM_DEC_PROP;
         break;
   }


   if (calcValueData)
   {
      if (diagram->valueData)
      {
         destroy3DArray ((char ***) diagram->valueData, diagram->nz, diagram->nx, diagram->ny);
         diagram->valueData = (float ***) NULL;
      }
      if (diagram->valueData = calcBlockPropertiesData (calcType,
                         diagram->minXLoc, diagram->minYLoc,
                         diagram->minZLoc+diagram->nz*diagram->blockSize,
                         diagram->blockSize, diagram->nx, diagram->ny, diagram->nz))
      {
         minValue = maxValue = diagram->valueData[0][0][0];
         for (z = 0; z < diagram->nz; z++)
         {
            for (x = 0; x < diagram->nx; x++)
            {
               for (y = 0; y < diagram->ny; y++)
               {
                  value = diagram->valueData[z][x][y];
                  if (value < minValue)
                     minValue = value;
                  else if (value > maxValue)
                     maxValue = value;
               }
            }
         }
         diagram->minValueData = (float) minValue;
         diagram->maxValueData = (float) maxValue;
      }
   }

   return (calcValueData);
}

/* ======================================================================
FUNCTION        loadBlockImageOptions
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadBlockImageOptions (WINDOW parentWin)
#else
loadBlockImageOptions (parentWin)
WINDOW parentWin;
#endif
{
   int result = TRUE;
   BLOCK_DIAGRAM_DATA *diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (parentWin);
   WINDOW typeList, layerList, lutStretchList;
   WIN_TYPE winType;
   
   if (!diagram)
      return (FALSE);

   winType = xvt_vobj_get_type(parentWin);

                                                /* type List */
   if (typeList = xvt_win_get_ctl (parentWin, LAYER_DISPLAY_TYPE))
   {
      xvt_list_suspend (typeList);
      xvt_list_add (typeList, 0, "Solid Block Diagram");
      xvt_list_add (typeList, 1, "Specific Layers");
#ifndef ENCOM_V6
      xvt_list_add (typeList, 2, "Density");
      xvt_list_add (typeList, 3, "Sus X");
      xvt_list_add (typeList, 4, "Sus Y");
      xvt_list_add (typeList, 5, "Sus Z");
      xvt_list_add (typeList, 6, "Sus Dip");
      xvt_list_add (typeList, 7, "Sus Dip Direction");
      xvt_list_add (typeList, 8, "Sus Pitch");
      xvt_list_add (typeList, 9, "Rem Intensity");
      xvt_list_add (typeList, 10, "Rem Inclination");
      xvt_list_add (typeList, 11, "Rem Declination");
#endif
      xvt_list_resume (typeList);
   }
   
   if (lutStretchList = xvt_win_get_ctl (parentWin, LAYER_LUT_STRETCH))
   {
      xvt_list_suspend (lutStretchList);
      xvt_list_add (lutStretchList, 0, "Stretch Over Range");
      xvt_list_add (lutStretchList, 1, "Clip within Range");
      xvt_list_resume (lutStretchList);
   }
   
   layerList = xvt_win_get_ctl (parentWin, LAYER_DISPLAY_LIST);
   
             /* Setup things to reflect current values */
   switch (diagram->type)
   {
      case (UNDEFINED_BLOCK):
         xvt_list_set_sel (typeList, 0, TRUE);
         diagram->type = SOLID_BLOCK; 
         break;
      case (SOLID_BLOCK):
         xvt_list_set_sel (typeList, 0, TRUE);
         break;
      case (LAYERED_BLOCK):            /* Layer List */
         xvt_list_set_sel (typeList, 1, TRUE);
         xvt_ctl_set_checked(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_ALL), FALSE);  /* specific layers by Default */
         break;
      case (DENSITY_BLOCK): case (SUS_X_BLOCK): case (SUS_Y_BLOCK):
      case (SUS_Z_BLOCK): case (SUS_DIP_BLOCK): case (SUS_DDIR_BLOCK):
      case (SUS_PITCH_BLOCK): case (REM_DEC_BLOCK): case (REM_INC_BLOCK):
      case (REM_STR_BLOCK):
         xvt_list_set_sel (typeList, (diagram->type - DENSITY_BLOCK + 2), TRUE);
         if (diagram->layersToDraw)    /* selected layers */
            xvt_ctl_set_checked(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_ALL), FALSE);
         else      /* All layers by Default */
            xvt_ctl_set_checked(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_ALL), TRUE);
         
         if (!diagram->lut)
         {
            if (diagram->lut = initBlockImageColorLut (NUM_PROPERTIES_LUT))
               diagram->lutSize = NUM_PROPERTIES_LUT;
         }
         break;
   }
   loadBlockImageLayerList (layerList, diagram);
   updateBlockImageOptions (parentWin);

   return (result);
}

/* ======================================================================
FUNCTION        saveBlockImageOptions
DESCRIPTION
        save all the options in the window
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
saveBlockImageOptions (WINDOW parentWin, BLOCK_DIAGRAM_DATA *diagram)
#else
saveBlockImageOptions (parentWin, diagram)
WINDOW parentWin;
BLOCK_DIAGRAM_DATA *diagram;
#endif
{
   int result = TRUE;
   WINDOW layerList;
   LAYER_PROPERTIES **propsToDraw, *layerProps[STRAT_LIMIT];
   int numDrawProps, numLayers, layer;
   int blockType, allLayers;
   WIN_TYPE winType;
                      /* If we dont have a structure or window we can do nothing */
   if (!diagram && !parentWin)
      return (FALSE);

   if (!diagram)  /* If structure not passed in it should be attached to the window */
      diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data(parentWin);

   if (!diagram)                                       
      return (FALSE);

   if (parentWin)
   {
      winType = xvt_vobj_get_type(parentWin);
      layerList = xvt_win_get_ctl (parentWin, LAYER_DISPLAY_LIST);
      xvt_vobj_set_visible(parentWin, FALSE);

                           /* Setup things to reflect current values */
      allLayers = xvt_ctl_is_checked(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_ALL));
      blockType = xvt_list_get_sel_index (xvt_win_get_ctl (parentWin, LAYER_DISPLAY_TYPE));
   }
   else  /* No window so initilise according to the values in the structure */
   {
      allLayers = FALSE;
      blockType = diagram->type-1;
   }
   
   switch (blockType)
   {
      case (0):  /* Block */
         diagram->type = SOLID_BLOCK;      
         destroy3DArray ((char ***) diagram->blockData, diagram->nz, diagram->nx, diagram->ny);
         diagram->blockData = (COLOR ***) NULL;
         destroy1DArray((char *) diagram->layersToDraw);
         diagram->layersToDraw = (LAYER_PROPERTIES **) NULL;
         diagram->numLayersToDraw = 0;
         if (parentWin)
            xvt_vobj_destroy(parentWin);
         if (parentWin && (winType == W_DBL))
            updateBlockDiagram (lastActiveWindow);  /* Update the display in the window */
         break;
      case (1):      /* See though Layers */
         diagram->type = LAYERED_BLOCK;      
         numDrawProps = 0;
         numLayers = assignPropertiesForStratLayers (layerProps, STRAT_LIMIT);

         if (parentWin)
         {
            for (layer = 0; layer < numLayers; layer++)
            {
               if (allLayers || xvt_list_is_sel (layerList, layer))
               {        /* Put layer we WANT to draw at the start */
                  layerProps[numDrawProps] = layerProps[layer];
                  numDrawProps++;
               }
            }
         
            if (diagram->layersToDraw)
            {
               destroy1DArray((char *) diagram->layersToDraw);
               diagram->layersToDraw = (LAYER_PROPERTIES **) NULL;
               diagram->numLayersToDraw = 0;
            }
         
            if (!(propsToDraw = (LAYER_PROPERTIES **) create1DArray(numDrawProps, sizeof(LAYER_PROPERTIES *))))
               return (FALSE);
         
            for (layer = 0; layer < numDrawProps; layer++)
               propsToDraw[layer] = layerProps[layer];
         
            diagram->layersToDraw = propsToDraw;
            diagram->numLayersToDraw = numDrawProps;
         
            xvt_vobj_destroy(parentWin);
         }      
         if (diagram->blockData)  /* Get rid of old Data */
         {
            destroy3DArray ((char ***) diagram->blockData, diagram->nz, diagram->nx, diagram->ny);
            diagram->blockData = (COLOR ***) NULL;
         }
         if (!parentWin || (winType == W_DBL))
         {
            initLongJob (0, diagram->nz*diagram->nx, "Calculating Block Data...", NULL);
            diagram->blockData = calcBlockDiagramColors(diagram->minXLoc,
                       diagram->minYLoc, diagram->minZLoc, diagram->nx,
                       diagram->ny, diagram->nz, diagram->blockSize,
                       diagram->numLayersToDraw, diagram->layersToDraw);
            finishLongJob();
         }
         if (parentWin && (winType == W_DBL))
            updateBlockDiagram (lastActiveWindow);  /* Update the display in the window */
         break;
      default:  /* See through property */
      {
         COLOR ***layerData = NULL;
         int x, y, z, dataType;

         if (diagram->blockData)  /* Get rid of old Data */
         {
            destroy3DArray ((char ***) diagram->blockData, diagram->nz, diagram->nx, diagram->ny);
            diagram->blockData = (COLOR ***) NULL;
         }
         
         if (allLayers || (!parentWin && !diagram->layersToDraw))
         {
            if (diagram->layersToDraw)
            {
               destroy1DArray((char *) diagram->layersToDraw);
               diagram->layersToDraw = (LAYER_PROPERTIES **) NULL;
               diagram->numLayersToDraw = 0;
            }

				if (parentWin)
					xvt_vobj_destroy(parentWin);
         }
         else    /* Turn Off layers that are not Wanted */
         {
            numDrawProps = 0;
            numLayers = assignPropertiesForStratLayers (layerProps, STRAT_LIMIT);
            if (parentWin)
            {
               for (layer = 0; layer < numLayers; layer++)
               {
                  if (xvt_list_is_sel (layerList, layer))
                  {        /* Put layer we WANT to draw at the start */
                     layerProps[numDrawProps] = layerProps[layer];
                     numDrawProps++;
                  }
               }
                  
               if (diagram->layersToDraw)
               {
                  destroy1DArray((char *) diagram->layersToDraw);
                  diagram->layersToDraw = (LAYER_PROPERTIES **) NULL;
                  diagram->numLayersToDraw = 0;
               }
                  
               if (!(propsToDraw = (LAYER_PROPERTIES **) create1DArray(numDrawProps, sizeof(LAYER_PROPERTIES *))))
                  return (FALSE);
                  
               for (layer = 0; layer < numDrawProps; layer++)
                  propsToDraw[layer] = layerProps[layer];
                  
               diagram->layersToDraw = propsToDraw;
               diagram->numLayersToDraw = numDrawProps;

					xvt_vobj_destroy(parentWin);
            }

            if (!parentWin || (winType == W_DBL))
            {
               initLongJob (0, diagram->nz*diagram->nx, "Calculating Layer Data...", NULL);
               layerData = calcBlockDiagramColors(diagram->minXLoc,
                          diagram->minYLoc, diagram->minZLoc, diagram->nx,
                          diagram->ny, diagram->nz, diagram->blockSize,
                          diagram->numLayersToDraw, diagram->layersToDraw);
            }
         }

         switch (blockType)
         {
            case (2): /* Density */
               dataType = DENSITY_BLOCK;      
               break;
            case (3): /* Sus X */
               dataType = SUS_X_BLOCK;      
               break;
            case (4): /* Sus Y */
               dataType = SUS_Y_BLOCK;      
               break;
            case (5): /* Sus Z */
               dataType = SUS_Z_BLOCK;      
               break;
            case (6): /* Sus Dip */
               dataType = SUS_DIP_BLOCK;      
               break;
            case (7): /* Sus Dip Direction */
               dataType = SUS_DDIR_BLOCK;
               break;
            case (8): /* Sus Pitch */
               dataType = SUS_PITCH_BLOCK;      
               break;
            case (9): /* Rem Intensity */
               dataType = REM_STR_BLOCK;      
               break;
            case (10): /* Rem Inclination */
               dataType = REM_INC_BLOCK;
               break;
            case (11): /* Rem Declination */
               dataType = REM_DEC_BLOCK;
               break;
         }
                   /* Get the values for the specified rock properties */
         if (!parentWin || (winType == W_DBL))
         {
            calcBlockImageValueData (diagram, dataType);

                      /* Make everything but what we set transparent */
            initLongJob (0, 3*diagram->nx, "Calculating Color Data...", NULL);
            incrementLongJob(INC_JOB_BY(diagram->nx));
            diagram->blockData = convertFloatBlockDataToColors(diagram->valueData,
                                             diagram->nx, diagram->ny, diagram->nz,
                                             diagram->lut, NUM_PROPERTIES_LUT);
            incrementLongJob(INC_JOB_BY(diagram->nx));
      
            if (layerData)     /* Turn Off layers that are not Wanted */
            {
               for (z = 0; z < diagram->nz; z++)
               {
                  for (x = 0; x < diagram->nx; x++)
                  {
                     for (y = 0; y < diagram->ny; y++)
                     {
                        if (layerData[z][x][y] == COLOR_INVALID)
                           diagram->blockData[z][x][y] = COLOR_INVALID;
                     }
                  }
               }
               destroy3DArray((char ***) layerData, diagram->nz, diagram->nx, diagram->ny);
            }
            finishLongJob();
         }
         else
         {
            diagram->type = dataType;
         }
         
         if (parentWin && (winType == W_DBL))
            updateBlockDiagram (lastActiveWindow);  /* Update the display in the window */
      }
      break;
   }
      
   return (result);
}

/* ======================================================================
FUNCTION        updateBlockImageOptions
DESCRIPTION
        load the geology movie options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateBlockImageOptions (WINDOW parentWin)
#else
updateBlockImageOptions (parentWin)
WINDOW parentWin;
#endif
{
   int result = TRUE, blockType, dataType;
   WINDOW typeList, layerList;
   BLOCK_DIAGRAM_DATA *diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (parentWin);
   int lutGB, layersGB, loadValuesBut = FALSE;
   char label[50];
   WIN_TYPE winType;

   winType = xvt_vobj_get_type(parentWin);
                                                /* type List */
   typeList = xvt_win_get_ctl (parentWin, LAYER_DISPLAY_TYPE);
   layerList = xvt_win_get_ctl (parentWin, LAYER_DISPLAY_LIST);
                   /* Setup things to reflect current values */
   blockType = xvt_list_get_sel_index (typeList);
   switch (blockType)
   {
      case (0): /* Block then grey layer list */
         lutGB = FALSE;
         layersGB = FALSE;
         break;
      case (1): /* layers then select */
         lutGB = FALSE;
         layersGB = TRUE;
         break;
      default:  /* Particular Properties */
         lutGB = TRUE;
         layersGB = TRUE;
         
         if (diagram->valueData)
         {
            switch (blockType)
            {
               case (2): /* Density */
                  dataType = DENSITY_BLOCK;      
                  break;
               case (3): /* Sus X */
                  dataType = SUS_X_BLOCK;      
                  break;
               case (4): /* Sus Y */
                  dataType = SUS_Y_BLOCK;      
                  break;
               case (5): /* Sus Z */
                  dataType = SUS_Z_BLOCK;      
                  break;
               case (6): /* Sus Dip */
                  dataType = SUS_DIP_BLOCK;      
                  break;
               case (7): /* Sus Dip Direction */
                  dataType = SUS_DDIR_BLOCK;
                  break;
               case (8): /* Sus Pitch */
                  dataType = SUS_PITCH_BLOCK;      
                  break;
               case (9): /* Rem Intensity */
                  dataType = REM_STR_BLOCK;      
                  break;
               case (10): /* Rem Inclination */
                  dataType = REM_INC_BLOCK;
                  break;
               case (11): /* Rem Declination */
                  dataType = REM_DEC_BLOCK;
                  break;
            }
            if (diagram->type != dataType)
               loadValuesBut = TRUE;
         }
         else
            loadValuesBut = TRUE;

         if (loadValuesBut)
            sprintf(label,"0 - 100%%");
         else
         {
            if ((diagram->maxValueData < 0.1) && (diagram->minValueData < 0.1))
               sprintf(label,"%e - %e",diagram->minValueData, diagram->maxValueData);
            else
               sprintf(label,"%.1lf - %.1lf",diagram->minValueData, diagram->maxValueData);
         }
         xvt_vobj_set_title(xvt_win_get_ctl(parentWin,LAYER_LUT_VALUE), label); 
         
         if (!diagram->lut) /* Init Lut */
         {
            if (diagram->lut = initBlockImageColorLut (NUM_PROPERTIES_LUT))
               diagram->lutSize = NUM_PROPERTIES_LUT;
         }
         drawBlockImageColorScale (parentWin, diagram->lut, NULL, TRUE);
         break;
   }
   
                     /* Turn the LUT GB on or off as needed */
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_LUT_GB), (BOOLEAN) lutGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_LUT_STRETCH), (BOOLEAN) lutGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_RESET_COLORS), (BOOLEAN) lutGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_CALC_VALUES), (BOOLEAN) (lutGB && loadValuesBut));
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_EDIT_COLORS), (BOOLEAN) FALSE /* lutGB */);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_LUT_VALUE_LABEL), (BOOLEAN) lutGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_LUT_VALUE), (BOOLEAN) lutGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_LUT_START_LABEL), (BOOLEAN) lutGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_LUT_START), (BOOLEAN) lutGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_LUT_END_LABEL), (BOOLEAN) lutGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_LUT_END), (BOOLEAN) lutGB);
                     /* Turn the LAYERS GB on or off as needed */
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_LAYER_GB), (BOOLEAN) layersGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_LIST), (BOOLEAN) layersGB);
   xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_ALL), (BOOLEAN) layersGB);
   if (layersGB)  /* Turn on List if 'ALL' not selected */
      xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_LIST),
       (BOOLEAN) (!xvt_ctl_is_checked(xvt_win_get_ctl(parentWin, LAYER_DISPLAY_ALL))));

   drawBlockImageColorScale (parentWin, diagram->lut, NULL, FALSE);   

   return (result);
}

/* ======================================================================
FUNCTION        loadWinPositionOptions
DESCRIPTION
        load the window position options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadWinPositionOptions (WINDOW parentWin)
#else
loadWinPositionOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW winList;
   int winNum;
   
   winList = xvt_win_get_ctl (parentWin, WINPOS_WINDOW_LIST);
   xvt_list_suspend(winList);
   for (winNum = 0; winNum < winPositionOptions.numWindows; winNum++)
   {
      xvt_list_add(winList, winNum, winPositionOptions.winPositions[winNum].name);
   }
   xvt_list_set_sel(winList, 0, TRUE);
   xvt_list_resume(winList);

   copyOptions (WINPOS_WINDOW, (OBJECT *) &winPositionOptions, OPTIONS_SAVE);

   updateWinPositionOptions (parentWin);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        saveWinPositionOptions
DESCRIPTION
        save the window positions options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
saveWinPositionOptions (WINDOW parentWin)
#else
saveWinPositionOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW winList;
   int winNum;
   
   winList = xvt_win_get_ctl (parentWin, WINPOS_WINDOW_LIST);
   winNum = xvt_list_get_sel_index(winList);
   
   winPositionOptions.winPositions[winNum].x
                      = (short) getIntegerTextFieldValue (parentWin, WINPOS_X);
   winPositionOptions.winPositions[winNum].y
                      = (short) getIntegerTextFieldValue (parentWin, WINPOS_Y);
   winPositionOptions.winPositions[winNum].width
                      = (short) getIntegerTextFieldValue (parentWin, WINPOS_WIDTH);
   winPositionOptions.winPositions[winNum].height
                      = (short) getIntegerTextFieldValue (parentWin, WINPOS_HEIGHT);

   return (TRUE);
}

/* ======================================================================
FUNCTION        updateWinPositionOptions
DESCRIPTION
        update labels showing what is happening to the image
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateWinPositionOptions (WINDOW parentWin)
#else
updateWinPositionOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW winList, win;
   int winNum, enableHeight, enableWidth;
   
   winList = xvt_win_get_ctl (parentWin, WINPOS_WINDOW_LIST);
   winNum = xvt_list_get_sel_index(winList);
   
   win = updateNumericTextField (parentWin, WINPOS_X,
                          (int) winPositionOptions.winPositions[winNum].x);
   setControlFont (win);
   win = updateNumericTextField (parentWin, WINPOS_Y,
                          (int) winPositionOptions.winPositions[winNum].y);
   setControlFont (win);
   win = updateNumericTextField (parentWin, WINPOS_WIDTH,
                          (int) winPositionOptions.winPositions[winNum].width);
   setControlFont (win);
   win = updateNumericTextField (parentWin, WINPOS_HEIGHT,
                          (int) winPositionOptions.winPositions[winNum].height);
   setControlFont (win);
   
           /* disable width and height if they are -ve values 
           ** This is needed for windows like anomalist that specifing
           ** a width and height for is silly */
   if (winPositionOptions.winPositions[winNum].height < 0.0)
   {
      win = xvt_win_get_ctl (parentWin, WINPOS_HEIGHT);
      xvt_vobj_set_title (win, "N/A");
      enableHeight = FALSE;
   }
   else
      enableHeight = TRUE;

   if (winPositionOptions.winPositions[winNum].width < 0.0)
   {
      win = xvt_win_get_ctl (parentWin, WINPOS_WIDTH);
      xvt_vobj_set_title (win, "N/A");
      enableWidth = FALSE;
   }
   else
      enableWidth = TRUE;
      
   win = xvt_win_get_ctl(parentWin, WINPOS_HEIGHT);
   xvt_vobj_set_enabled(win, (BOOLEAN) enableHeight);
   win = xvt_win_get_ctl(parentWin, WINPOS_HEIGHT_BAR);
   xvt_vobj_set_enabled(win, (BOOLEAN) enableHeight);
   win = xvt_win_get_ctl(parentWin, WINPOS_HEIGHT_LABEL);
   xvt_vobj_set_enabled(win, (BOOLEAN) enableHeight);

   win = xvt_win_get_ctl(parentWin, WINPOS_WIDTH);
   xvt_vobj_set_enabled(win, (BOOLEAN) enableWidth);
   win = xvt_win_get_ctl(parentWin, WINPOS_WIDTH_BAR);
   xvt_vobj_set_enabled(win, (BOOLEAN) enableWidth);
   win = xvt_win_get_ctl(parentWin, WINPOS_WIDTH_LABEL);
   xvt_vobj_set_enabled(win, (BOOLEAN) enableWidth);

   return (TRUE);
}


/* ======================================================================
FUNCTION        loadXYZImportOptions
DESCRIPTION
        laod the XYZ import options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadXYZImportOptions (WINDOW parentWin, XYZ_IMPORT *xyzImport)
#else
loadXYZImportOptions (parentWin, xyzImport)
WINDOW parentWin;
XYZ_IMPORT *xyzImport;
#endif
{
   FILE *fi;
   char text[200];
   int line, col, numColumns = 0;
   WINDOW win;
   TXEDIT twin;

   if (!xyzImport || !extractXYZHeader (&(xyzImport->fileSpec), &numColumns))
   {
      xvt_dm_post_error ("Error, Could not parse XYZ file. Invalid Format.");
      xvt_vobj_destroy (parentWin);
      return;
   }

   /* Build the noddy Element List */
   win = xvt_win_get_ctl (parentWin, XYZ_CONVERT_XCOLUMN);
   xvt_list_suspend (win);
   xvt_list_add (win , 0, "None");
   for (col = 0; col < numColumns; col++)
   {
      sprintf (text, "%d", col+1);
      xvt_list_add (win , col+1, text);
   }
   xvt_list_set_sel (win, (numColumns < 1)?0:1, TRUE);
   xvt_list_resume (win);

   win = xvt_win_get_ctl (parentWin, XYZ_CONVERT_YCOLUMN);
   xvt_list_suspend (win);
   xvt_list_add (win , 0, "None");
   for (col = 0; col < numColumns; col++)
   {
      sprintf (text, "%d", col+1);
      xvt_list_add (win , col+1, text);
   }
   xvt_list_set_sel (win, (numColumns < 2)?0:2, TRUE);
   xvt_list_resume (win);

   win = xvt_win_get_ctl (parentWin, XYZ_CONVERT_ZCOLUMN);
   xvt_list_suspend (win);
   xvt_list_add (win , 0, "None");
   for (col = 0; col < numColumns; col++)
   {
      sprintf (text, "%d", col+1);
      xvt_list_add (win , col+1, text);
   }
   xvt_list_set_sel (win, (numColumns < 3)?0:3, TRUE);
   xvt_list_resume (win);


   win = updateFloatTextField (parentWin, XYZ_CONVERT_XOFFSET, 0.0, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, XYZ_CONVERT_YOFFSET, 0.0, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, XYZ_CONVERT_ZOFFSET, 0.0, 1);
   setControlFont (win);

                         /* Read in the first few lines as a sample */
   xvt_fsys_set_dir (&(xyzImport->fileSpec.dir));
   if (fi = fopen (xyzImport->fileSpec.name, "r"))
   {
      twin = (TXEDIT) xvt_win_get_tx (parentWin, XYZ_CONVERT_PREVIEW);
      xvt_tx_suspend ((TXEDIT) twin);
      xvt_tx_clear ((TXEDIT) twin);
      for (line = 0; line < 10; line++)
      {
         if (fgets (text, 200-1, fi))
         {
            xvt_tx_add_par ((TXEDIT) twin, (T_PNUM) line, text);
         }
         else
            break;
      }
      xvt_tx_resume ((TXEDIT) twin);
      fclose (fi);
   }
}

/* ======================================================================
FUNCTION        saveXYZImportOptions
DESCRIPTION
        save the XYZ import options window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveXYZImportOptions (WINDOW parentWin, XYZ_IMPORT *xyzImport)
#else
saveXYZImportOptions (parentWin, xyzImport)
WINDOW parentWin;
XYZ_IMPORT *xyzImport;
#endif
{
   double offsetX, offsetY, offsetZ;
   int xColumn, yColumn, zColumn;
   double **pointPos;
   int numPoints, point, fileType = 1;
   double minX, minY, minZ, maxX, maxY, maxZ, cubeSize;
   FILE_SPEC outputFileSpec;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   int gotXYZ;

   xColumn = xvt_list_get_sel_index (xvt_win_get_ctl (parentWin, XYZ_CONVERT_XCOLUMN));
   yColumn = xvt_list_get_sel_index (xvt_win_get_ctl (parentWin, XYZ_CONVERT_YCOLUMN));
   zColumn = xvt_list_get_sel_index (xvt_win_get_ctl (parentWin, XYZ_CONVERT_ZCOLUMN));
   
   offsetX = getFloatTextFieldValue (parentWin, XYZ_CONVERT_XOFFSET);
   offsetY = getFloatTextFieldValue (parentWin, XYZ_CONVERT_YOFFSET);
   offsetZ = getFloatTextFieldValue (parentWin, XYZ_CONVERT_ZOFFSET);

   gotXYZ = extractXYZFromFile (&(xyzImport->fileSpec), xColumn, yColumn, zColumn,
                                          offsetX, offsetY, offsetZ,
                                          &pointPos, &numPoints);
   if (xyzImport->type == BOREHOLE_IMPORT)
   {
      if (gotXYZ)
         updateBorehole (pointPos, numPoints, (int) xyzImport->data);
      else
         xvt_dm_post_error ("Error, Cannot create Borehole");
   }
   else if (xyzImport->type == XYZ_IMAGE_IMPORT)
   {
		strcpy (outputFileSpec.name, xyzImport->fileSpec.name);
		addFileExtention(outputFileSpec.name, ".mag");
      strcpy (outputFileSpec.type, "mag");
#if (XVTWS == MACWS)
      strcpy (outputFileSpec.type, "TEXT");
#endif

      getDefaultDirectory(&(outputFileSpec.dir));
      switch (xvt_dm_post_file_save(&outputFileSpec, "Save Image As ..."))
      {
         case FL_OK:
            if (strlen(outputFileSpec.name) == 0)
            {
               xvt_dm_post_error ("Error, No file Specified");
               return;
               break;
            }
            setDefaultDirectory(&(outputFileSpec.dir));
            break;
         case FL_BAD:
            xvt_dm_post_error("Error getting file name.");
            return;
            break;
         case FL_CANCEL:
            return;
            break;
      }             

		/* work out the cube size */
		minX = fabs(pointPos[1][0]-pointPos[0][0]);
		minY = fabs(pointPos[1][1]-pointPos[0][1]);
		cubeSize = MIN(minX, minY);
		if (cubeSize < 0.0001)   /* Dont end up with 0 if it hasn't varied */
			cubeSize = MAX(minX, minY);

		/* get the extents */
		minX = maxX = pointPos[0][0];
		minY = maxY = pointPos[0][1];
		minZ = maxZ = pointPos[0][2];
      for (point = 0; point < numPoints; point++)
      {
         if (pointPos[point][0] < minX) minX = pointPos[point][0];
         if (pointPos[point][1] < minY) minY = pointPos[point][1];
         if (pointPos[point][2] < minZ) minZ = pointPos[point][2];
         if (pointPos[point][0] > maxX) maxX = pointPos[point][0];
         if (pointPos[point][1] > maxY) maxY = pointPos[point][1];
         if (pointPos[point][2] > maxZ) maxZ = pointPos[point][2];
      }

		importColumnFormatImage(&(xyzImport->fileSpec), &outputFileSpec,
										minX, minY, minZ, maxX, maxY, maxZ, cubeSize,
										pointPos, numPoints);
		
      destroy2DArray ((char **) pointPos, numPoints, 3);
	}
   else   /* XYZ_GEO_IMPORT */
   {
      if (gotXYZ)
      {
         strcpy (outputFileSpec.name, "untitled.geo");
         strcpy (outputFileSpec.type, "geo");
#if (XVTWS == MACWS)
         strcpy (outputFileSpec.type, "TEXT");
#endif

         getDefaultDirectory(&(outputFileSpec.dir));
         switch (xvt_dm_post_file_save(&outputFileSpec, "Save XYZ As ..."))
         {
            case FL_OK:
               if (strlen(outputFileSpec.name) == 0)
               {
                  xvt_dm_post_error ("Error, No file Specified");
                  return;
                  break;
               }
               setDefaultDirectory(&(outputFileSpec.dir));
               break;
            case FL_BAD:
               xvt_dm_post_error("Error getting file name.");
               return;
               break;
            case FL_CANCEL:
               return;
               break;
         }             
   
         xvt_fsys_set_dir(&outputFileSpec.dir);
   
      
         if (fileType == 0)  /* Geological */
            writeLineThroughBlock (outputFileSpec.name, pointPos, numPoints);
         else
         {
/*
            minX = maxX = pointPos[0][0];
            minY = maxY = pointPos[0][1];
            minZ = maxZ = pointPos[0][2];
*/
            minX = viewOptions->originX;  /* Use the default block as the default bounds */
            minY = viewOptions->originY;
            minZ = viewOptions->originZ - viewOptions->lengthZ;
            maxX = viewOptions->originX + viewOptions->lengthX;
            maxY = viewOptions->originY + viewOptions->lengthY;
            maxZ = viewOptions->originZ;

            for (point = 0; point < numPoints; point++)
            {
               if (pointPos[point][0] < minX) minX = pointPos[point][0];
               if (pointPos[point][1] < minY) minY = pointPos[point][1];
               if (pointPos[point][2] < minZ) minZ = pointPos[point][2];
               if (pointPos[point][0] > maxX) maxX = pointPos[point][0];
               if (pointPos[point][1] > maxY) maxY = pointPos[point][1];
               if (pointPos[point][2] > maxZ) maxZ = pointPos[point][2];
            }
                             /* Make sure the block covers the xyz locations */
				/*
            if ((minX < viewOptions->originX)
                  || (minY < viewOptions->originY)
                  || (maxX > viewOptions->originX-viewOptions->lengthX)
                  || (maxY > viewOptions->originY-viewOptions->lengthY))
            {
               xvt_dm_post_error ("Warning, Points lay outside of Geophysical block Area.");
            }
            else
				*/
            {
               GEOPHYSICS_OPTIONS options;
               BLOCK_VIEW_OPTIONS blockOptions;
               
               memcpy (&options, &geophysicsOptions, sizeof(GEOPHYSICS_OPTIONS));
					if (options.calculationMethod == SPATIAL) /* Only spatial cannot do xyz calcs (onlyt Spectral/Full) */
					{
						options.calculationMethod = SPECTRAL; /* Default to spectral as it is quicker */ 
						xvt_dm_post_error("You were trying to calculate with a Spatial convolution. Only Spectral and Full Spatial method support this operation. You are now using a Spectral Calculation Method.");
					}
					{
						int calcRangeStore, padStore = TRUE;
      
						if (options.calculationMethod == SPECTRAL)
						{                              /* So we dont get the 6 pixel border */
							calcRangeStore = options.calculationRange;
							options.calculationRange = 0;
						}
						if ((options.calculationMethod == SPATIAL_FULL) && (!options.padWithRealGeology))
						{                              /* So we dont get the 6 pixel border */
							calcRangeStore = options.calculationRange;
							padStore = options.padWithRealGeology;
							if (calcRangeStore > 0)
							{
								options.padWithRealGeology = options.calculationRange;
								options.calculationRange = 0;
							}
						}

						/* init blockOptions with the current view information */
						memcpy (&blockOptions, viewOptions, sizeof(BLOCK_VIEW_OPTIONS));

						if ((blockOptions.originX > minX) || (blockOptions.originY > minY)
										 || (blockOptions.originX+blockOptions.lengthX < maxX) 
										 || (blockOptions.originY+blockOptions.lengthY < maxY))
						{
							xvt_dm_post_error("Warning! Some point lay outside the block area.");
							/*
							blockOptions.originX = minX;
							blockOptions.originY = minY;
							blockOptions.lengthX = maxX - minX;
							blockOptions.lengthY = maxY - minY;
							*/
						}

						/* blockOptions.lengthZ = maxZ - minZ; */
						doGeophysics(XYZ_ANOM, &blockOptions, &options,
										 outputFileSpec.name, NULL, pointPos,
										 numPoints, &(xyzImport->fileSpec), NULL, NULL);

	               finishLongJob ();
						if ((options.calculationMethod == SPATIAL_FULL) && !padStore)
						{                              /* So we dont get the 6 pixel border */
							if (calcRangeStore > 0)
							{
								options.calculationRange = calcRangeStore;
								options.padWithRealGeology = padStore;
							}
						}
						if (options.calculationMethod == SPECTRAL)
						{
							options.calculationRange = calcRangeStore;
						}
					}
               options.calculationRange = 0;
            }
         }
         destroy2DArray ((char **) pointPos, numPoints, 3);
      }
   }

   if (gotXYZ)
      xvt_vobj_destroy (parentWin);
}

/* ======================================================================
FUNCTION        loadRockPropOptions
DESCRIPTION
        load the rock properties options window with the
        current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadRockPropOptions (WINDOW parentWin, OBJECT *object)
#else
loadRockPropOptions (parentWin, object)
WINDOW parentWin;
OBJECT *object;
#endif
{
   WINDOW win, heightWin;
   LAYER_PROPERTIES *options;
   double layerWidth;
   int prop;
   double unitScale;

   options = (LAYER_PROPERTIES *) getLayerPropertiesStructure (object);
   if (!options)
      return (FALSE);
      
   layerWidth = getLayerWidth (object);
   
   if (projectOptions.susceptibilityUnits == SI)
      unitScale = CGS_TO_SI;
   else  /* CGS */
      unitScale = 1.00000;
   
   win = xvt_win_get_ctl (parentWin, ROCK_NAME);
   setControlFont (win);
   if (rockDatabase.numProps && rockDatabase.database)
   {
      xvt_list_suspend (win);
      for (prop = 0; prop < rockDatabase.numProps; prop++)
         xvt_list_add (win, -1, rockDatabase.database[prop].unitName);
      xvt_list_resume (win);
   }
   xvt_vobj_set_title (win, options->unitName);
   
   xvt_ctl_set_checked (xvt_win_get_ctl(parentWin, ROCK_ALTER),
                        (BOOLEAN) options->applyAlterations);

   heightWin = updateNumericTextField (parentWin, ROCK_HEIGHT, options->height);
   setControlFont (heightWin);
   if (layerWidth >= 0.0)
   {
      win = updateFloatTextField (parentWin, ROCK_WIDTH, layerWidth, 0);
      xvt_vobj_set_enabled (win, TRUE);
      xvt_vobj_set_enabled (heightWin, FALSE);
   }
   else
   {
      win = xvt_win_get_ctl (parentWin, ROCK_WIDTH);
      xvt_vobj_set_title (win, "Inf.");
      xvt_vobj_set_enabled (win, FALSE);
      xvt_vobj_set_enabled (heightWin, TRUE);
   }
   setControlFont (win);
   win = updateFloatTextField (parentWin, ROCK_DENSITY, options->density, 3);
   setControlFont (win);

   win = xvt_win_get_ctl (parentWin, ROCK_ANISOTROPY);
   xvt_ctl_set_checked(win, (BOOLEAN) options->anisotropicField);

   win = updateFloatTextField (parentWin, ROCK_X, options->sus_X*unitScale, 4);
   setControlFont (win);
   win = updateFloatTextField (parentWin, ROCK_Y, options->sus_Y*unitScale, 4);
   setControlFont (win);
   win = updateFloatTextField (parentWin, ROCK_Z, options->sus_Z*unitScale, 4);
   setControlFont (win);
   win = updateFloatTextField (parentWin, ROCK_DIP, options->sus_dip, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, ROCK_DDIR, options->sus_dipDirection, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, ROCK_PITCH, options->sus_pitch, 1);
   setControlFont (win);

   win = xvt_win_get_ctl (parentWin, ROCK_REMANENCE);
   xvt_ctl_set_checked(win, (BOOLEAN) options->remanentMagnetization);

   win = updateFloatTextField (parentWin, ROCK_INCLINATION, options->inclination, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, ROCK_DECLINATION, options->angleWithNorth, 1);
   setControlFont (win);
   win = updateFloatTextField (parentWin, ROCK_INTENSITY, options->strength*unitScale, 4);
   setControlFont (win);

   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_Y), (BOOLEAN) options->anisotropicField);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_Y_LABEL), (BOOLEAN) options->anisotropicField);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_Z), (BOOLEAN) options->anisotropicField);           
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_Z_LABEL), (BOOLEAN) options->anisotropicField);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DIP), (BOOLEAN) options->anisotropicField);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DIP_LABEL), (BOOLEAN) options->anisotropicField);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DDIR), (BOOLEAN) options->anisotropicField);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DDIR_LABEL), (BOOLEAN) options->anisotropicField);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_PITCH), (BOOLEAN) options->anisotropicField);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_PITCH_LABEL), (BOOLEAN) options->anisotropicField);

   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DECLINATION), (BOOLEAN) options->remanentMagnetization);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DECLINATION_LABEL), (BOOLEAN) options->remanentMagnetization);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_INCLINATION), (BOOLEAN) options->remanentMagnetization);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_INCLINATION_LABEL), (BOOLEAN) options->remanentMagnetization);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_INTENSITY), (BOOLEAN) options->remanentMagnetization);
   xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_INTENSITY_LABEL), (BOOLEAN) options->remanentMagnetization);

                          /* Grey height when in appropriate */
   if ((object->shape == PLUG) || (object->shape == DYKE)
                               || (object->shape == IMPORT)
                               || (object->shape == STOP))
      xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, ROCK_HEIGHT),
                    FALSE);
   else if (((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY))
                                             && (object->generalData == 1))
      xvt_vobj_set_enabled(xvt_win_get_ctl(parentWin, ROCK_HEIGHT),
                    FALSE);

   return (TRUE);
}

/* ======================================================================
FUNCTION        saveRockPropOptions
DESCRIPTION
        save the rock properties options window with the
        current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
saveRockPropOptions (WINDOW parentWin, OBJECT *object)
#else
saveRockPropOptions (parentWin, object)
WINDOW parentWin;
OBJECT *object;
#endif
{
   WINDOW win;
   LAYER_PROPERTIES *options;
   char text[100];
   double unitScale;
   int newHeight, heightDiff;

   options = (LAYER_PROPERTIES *) getLayerPropertiesStructure (object);
   if (!options)
      return (FALSE);

   if (projectOptions.susceptibilityUnits == SI)
      unitScale = CGS_TO_SI;
   else  /* CGS */
      unitScale = 1.00000;
            
   win = xvt_win_get_ctl (parentWin, ROCK_NAME);
   xvt_vobj_get_title (win, text, UNIT_NAME_LENGTH);
   strcpy (options->unitName, text);

   options->applyAlterations = xvt_ctl_is_checked (xvt_win_get_ctl(parentWin, ROCK_ALTER));
                       
   newHeight = getIntegerTextFieldValue (parentWin, ROCK_HEIGHT);
   heightDiff = newHeight - options->height;
   options->height = newHeight;
   options->density = getFloatTextFieldValue (parentWin, ROCK_DENSITY);

   win = xvt_win_get_ctl (parentWin, ROCK_ANISOTROPY);
   options->anisotropicField = (int) xvt_ctl_is_checked (win);

   options->sus_X = getFloatTextFieldValue (parentWin, ROCK_X)/unitScale;
   options->sus_Y = getFloatTextFieldValue (parentWin, ROCK_Y)/unitScale;
   options->sus_Z = getFloatTextFieldValue (parentWin, ROCK_Z)/unitScale;
   options->sus_dip = getFloatTextFieldValue (parentWin, ROCK_DIP);
   options->sus_dipDirection = getFloatTextFieldValue (parentWin, ROCK_DDIR);
   options->sus_pitch = getFloatTextFieldValue (parentWin, ROCK_PITCH);

   win = xvt_win_get_ctl (parentWin, ROCK_REMANENCE);
   options->remanentMagnetization = (int) xvt_ctl_is_checked (win);

   options->inclination = getFloatTextFieldValue (parentWin, ROCK_INCLINATION);
   options->angleWithNorth = getFloatTextFieldValue (parentWin, ROCK_DECLINATION);
   options->strength = getFloatTextFieldValue (parentWin, ROCK_INTENSITY)/unitScale;

                      /* calculate and store the forward and
                      ** reverse rotation matricies for remenants */
   convrt(90.0 - options->inclination, 180.0 + options->angleWithNorth,
                                       0.0, TRUE);
   rotset(90.0 - options->inclination, 180.0 + options->angleWithNorth,
                             0.0, options->remRotationMatrix.forward,
                                  options->remRotationMatrix.reverse);

   convrt(options->sus_dip, options->sus_dipDirection,
                            options->sus_pitch, TRUE);
   rotset(options->sus_dip, options->sus_dipDirection,
                            options->sus_pitch,
                            options->aniRotationMatrix.forward,
                            options->aniRotationMatrix.reverse);

                      /* Check to see if the layer Height is valid */
   if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY))
   {
      STRATIGRAPHY_OPTIONS *stratOptions;
      int layer;
               
      stratOptions = getStratigraphyOptionsStructure(object);

                 /* Move all the layers by the amount that the layer height was moved */
      for (layer = object->generalData-2; layer > 0; layer--)
         stratOptions->properties[layer].height += heightDiff;
                 
                 /* Start from layer 1 instead of 0 as the bottom layer is 
                 ** considered always lower than any layer above it */   
      if ((stratOptions->numLayers > 1) &&
          (stratOptions->properties[0].height > stratOptions->properties[1].height))
         stratOptions->properties[0].height = stratOptions->properties[1].height-1000;
      for (layer = 1; layer < stratOptions->numLayers-1; layer++)
      {
         if (stratOptions->properties[layer].height
                 >= stratOptions->properties[layer+1].height)
         {
           // xvt_dm_post_error ("Error, layer '%s' is not above layer '%s'. Adjusting.",
                                           //   stratOptions->properties[layer+1].unitName,
                                            //  stratOptions->properties[layer].unitName);
            stratOptions->properties[layer+1].height
                      = stratOptions->properties[layer].height + 1;
         }
                  
      }
   }
                  /* update windows to show changes */
   if (object->shape == STRATIGRAPHY)
      updateStratigraphyOptions (object);
   else if (object->shape == UNCONFORMITY)
      updateUnconformityOptions (object);

            
      /* Free the temporary object used to rock Database editing */
   if ((object->generalData < 1) && (object->shape == STOP))
   {
      updateRocksInDatabase (object->previewWin);
      xvt_mem_free ((char *) object);
   }
   return (TRUE);
}


/* ======================================================================
FUNCTION        loadProfileOptions
DESCRIPTION
        load the profile options window with the
        current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
loadProfileOptions (WINDOW parentWin, OBJECT *object)
#else
loadProfileOptions (parentWin, object)
WINDOW parentWin;
OBJECT *object;
#endif
{
   PROFILE_OPTIONS *options;
   BOOLEAN alteration = FALSE, readOnly = FALSE;
   PROFILE_OPTIONS *tempOptions;
   WINDOW listWin;
   RCT graphPos, barPos;
            
   if (options = getProfileOptionsStructure (object))
   {
           /* save the options in the window so they can be restored **
           ** if cancel is pressed to exit the object */
      copyOptions (PROFILE_WINDOW, object, OPTIONS_SAVE);
                   
           /* Position the graph relative to some scalable objects in the window */
      xvt_vobj_get_outer_rect(lastActiveWindow, &graphPos);
      xvt_vobj_get_outer_rect(xvt_win_get_ctl(parentWin, PROFILE_WINDOW_HSCROLL_66), &barPos);
      graphPos.right = barPos.right;
      graphPos.bottom = barPos.top - 1;
      xvt_vobj_move (lastActiveWindow, &graphPos);

           /* Fill the list of graphs */
      listWin = xvt_win_get_ctl (parentWin, PROFILE_CURRENT_GRAPH);
      for (tempOptions = options; tempOptions; tempOptions = tempOptions->next)
         xvt_list_add (listWin, -1, tempOptions->name);
      xvt_list_set_sel (listWin, 0, TRUE);
      if (options->next)    /* more than one profile then it is an alteration */
         alteration = TRUE;
               
               
      xdCheckRadioButton(parentWin, PROFILE_LINE_JOIN + options->joinType,
                                    PROFILE_LINE_JOIN, PROFILE_SQUARE_JOIN);
      if (alteration)
      {
         xvt_vobj_set_title(xvt_win_get_ctl(parentWin, PROFILE_LINE_JOIN),   "Line (Alt.)");
         xvt_vobj_set_title(xvt_win_get_ctl(parentWin, PROFILE_CURVE_JOIN),  "Curve (Alt.)");
         xvt_vobj_set_title(xvt_win_get_ctl(parentWin, PROFILE_SQUARE_JOIN), "Square (Rep.)");
      }

                            /* make readonly if needed */
      if (readOnly = isProfileReadOnly(object))
      {
         xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, PROFILE_LINE_JOIN), FALSE);
         xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, PROFILE_CURVE_JOIN), FALSE);
         xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, PROFILE_SQUARE_JOIN), FALSE);
         xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, PROFILE_DELETE_POINT), FALSE);
         xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, PROFILE_DELETE_ALL), FALSE);
         xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, PROFILE_WINDOW_PUSHBUTTON_37), FALSE); 
         xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, PROFILE_OPTIONS_BUT), FALSE); 
         xvt_vobj_set_enabled (lastActiveWindow, FALSE); 
      }
               /* no options on a normal plane profile */
      if (options->type == PLANE_PROFILE)
      {
         switch (object->shape)
         {
            case (FOLD):
               {
                  FOLD_OPTIONS *foldOptions = (FOLD_OPTIONS *) object->options;

                  saveEventOptions (parentWin, object, TRUE);
                  options->minX = 0.0;
                  options->maxX = foldOptions->wavelength;
                  options->minYScale = -foldOptions->amplitude;
                  options->maxYScale = foldOptions->amplitude;
                  options->minYReplace = -foldOptions->amplitude;
                  options->maxYReplace = foldOptions->amplitude;
               }
               break;
            case (FAULT): case (SHEAR_ZONE):
               {
                  FAULT_OPTIONS *faultOptions = (FAULT_OPTIONS *) object->options;

                  saveEventOptions (parentWin, object, TRUE);
                  options->minX = 0.0;
                  options->maxX = faultOptions->xAxis;
                  options->minYScale = -faultOptions->amplitude;
                  options->maxYScale = faultOptions->amplitude;
                  options->minYReplace = -faultOptions->amplitude;
                  options->maxYReplace = faultOptions->amplitude;
               }
               break;
         }
            
         xvt_vobj_set_enabled (xvt_win_get_ctl (parentWin, PROFILE_OPTIONS_BUT), FALSE);
      }
      

      updateProfileOptions (parentWin);
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        saveProfileOptions
DESCRIPTION
        save the profile options window with the
        current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
saveProfileOptions (WINDOW parentWin, OBJECT *object)
#else
saveProfileOptions (parentWin, object)
WINDOW parentWin;
OBJECT *object;
#endif
{
   PROFILE_OPTIONS *options;
                                          
   if (options = getProfileOptionsStructure (object))
   {
      while (options)
      {
         options->array = getArrayForProfile (options);
         options = options->next;
      }
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        saveProfileOptionsOptions
DESCRIPTION
        save the profile options window with the
        current options
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
saveProfileOptionsOptions (WINDOW parentWin, OBJECT *object)
#else
saveProfileOptionsOptions (parentWin, object)
WINDOW parentWin;
OBJECT *object;
#endif
{
   PROFILE_OPTIONS *options;
                                          
   if (options = getCurrentProfileOptions(stackParentWin(parentWin),
                          getProfileOptionsStructure(object)))
   {
      options->minYScale = getFloatTextFieldValue (parentWin,
                                 PROFILE_OPTION_MIN_SCALE);
      options->maxYScale = getFloatTextFieldValue (parentWin,
                                 PROFILE_OPTION_MAX_SCALE);
      options->minYReplace = getFloatTextFieldValue (parentWin,
                                 PROFILE_OPTION_MIN_REPLACE);
      options->maxYReplace = getFloatTextFieldValue (parentWin,
                                 PROFILE_OPTION_MAX_REPLACE);
      options->maxX = getFloatTextFieldValue (parentWin,
                                 PROFILE_OPTION_ALTDIST);
      options->graphLength = getFloatTextFieldValue (parentWin,
                                 PROFILE_OPTION_WINDOW_EDIT_66);

      xvt_dwin_invalidate_rect (lastActiveWindow, (RCT *) NULL);
      xvt_dwin_update(lastActiveWindow);
   }
                        /* ************************************************* */
                        /* if Anisotropy Length Changed then change them all */
   if ((options->type >= ANI_PROFILE) && (options->type <= SUS_PITCH_PROFILE))
   {
      PROFILE_OPTIONS *tempOptions;
                 /* Set the same length for all other ones todo with anisotropy */
      for (tempOptions = getProfileOptionsStructure(object);
                                      tempOptions; tempOptions = tempOptions->next)
      {
         if ((tempOptions->type >= ANI_PROFILE) && (tempOptions->type <= SUS_PITCH_PROFILE))
            tempOptions->maxX = options->maxX;
      }
   }
                        /* ****************************************************** */
                        /* if Anisotropy Axis Length Changed then change them all */
   if ((options->type >= SUS_X_PROFILE) && (options->type <= SUS_Z_PROFILE))
   {
      PROFILE_OPTIONS *tempOptions;
                 /* Set the same length for all other ones todo with anisotropy */
      for (tempOptions = getProfileOptionsStructure(object);
                                      tempOptions; tempOptions = tempOptions->next)
      {
         if ((tempOptions->type >= SUS_X_PROFILE) && (tempOptions->type <= SUS_Z_PROFILE))
         {
            tempOptions->minYScale = options->minYScale;
            tempOptions->maxYScale = options->maxYScale;
            tempOptions->scaleOrigin = options->scaleOrigin;
            tempOptions->minYReplace = options->minYReplace;
            tempOptions->maxYReplace = options->maxYReplace;
         }
      }
   }
                        /* ************************************************ */
                        /* if Remanence Length Changed then change them all */
   if ((options->type >= REM_PROFILE) && (options->type <= REM_STR_PROFILE))
   {
      PROFILE_OPTIONS *tempOptions;
                 /* Set the same length for all other ones todo with remanence */
      for (tempOptions = getProfileOptionsStructure(object);
                                      tempOptions; tempOptions = tempOptions->next)
      {
         if ((tempOptions->type >= REM_PROFILE) && (tempOptions->type <= REM_STR_PROFILE))
            tempOptions->maxX = options->maxX;
      }
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        loadWindowOptions
DESCRIPTION
        load the window with the current options
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
loadWindowOptions (WINDOW parentWin, int parentId, OBJECT *object)
#else
loadWindowOptions (parentWin, parentId, object)
WINDOW parentWin;
int parentId;
OBJECT *object;
#endif
{
   WINDOW win;

   if (/*!object || */ !parentWin || !parentId) return;

   switch (parentId)
   {
      case DEFINE_COLOUR_WINDOW:
         {
            NODDY_COLOUR *options;
            NODDY_COLOUR justAColor;

            if (options = (NODDY_COLOUR *) getNoddyColourStructure (object))
               selectColor (parentWin, DEFINE_COLOUR_WINDOW_LISTEDIT_41, *options);
            else
            {
               justAColor.red = XVT_COLOR_GET_RED(backgroundColor);
               justAColor.green = XVT_COLOR_GET_GREEN(backgroundColor);
               justAColor.blue = XVT_COLOR_GET_BLUE(backgroundColor);
               strcpy(justAColor.name, "");
               selectColor (parentWin, DEFINE_COLOUR_WINDOW_LISTEDIT_41, justAColor);
            }
         }
         break;
      case PROFILE_OPTION_WINDOW:
         {
            PROFILE_OPTIONS *options;
            BOOLEAN alteration = FALSE;
            
            if (options = getCurrentProfileOptions(stackParentWin(parentWin),
                                   getProfileOptionsStructure(object)))
            {
               win = updateFloatTextField (parentWin, PROFILE_OPTION_MIN_SCALE,
                                             options->minYScale, 1);
               setControlFont (win);
               win = updateFloatTextField (parentWin, PROFILE_OPTION_MAX_SCALE,
                                             options->maxYScale, 1);
               setControlFont (win);
               win = updateFloatTextField (parentWin, PROFILE_OPTION_MIN_REPLACE,
                                             options->minYReplace, 1);
               setControlFont (win);
               win = updateFloatTextField (parentWin, PROFILE_OPTION_MAX_REPLACE,
                                             options->maxYReplace, 1);
               setControlFont (win);
               win = updateFloatTextField (parentWin, PROFILE_OPTION_ALTDIST,
                                             options->maxX, 1);
               setControlFont (win);
               win = updateFloatTextField (parentWin, PROFILE_OPTION_WINDOW_EDIT_66,
                                             options->graphLength, 1);
               setControlFont (win);
            }
         }
         break;
   }
}

/* ======================================================================
FUNCTION        saveWindowOptions
DESCRIPTION
        Save the options in the window to the current options
     This is called when the OK button is pressed on one of the
        displayed windows.
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
saveWindowOptions (WINDOW parentWin, int parentId, OBJECT *object)
#else
saveWindowOptions (parentWin, parentId, object)
WINDOW parentWin;
int parentId;
OBJECT *object;
#endif
{
   if (/*!object || */ !parentWin || !parentId) return;

   switch (parentId)
   {
      case DEFINE_COLOUR_WINDOW:
         {
            NODDY_COLOUR *options;
            NODDY_COLOUR justAColor;

            if (options = (NODDY_COLOUR *) getNoddyColourStructure (object))
               getColorSelection (parentWin, options);
            else
            {
               getColorSelection (parentWin, &justAColor);

               backgroundColor = XVT_MAKE_COLOR (justAColor.red, justAColor.green, justAColor.blue);
               update3dPreview(NULL_WIN, NULL);
            }
         }
         break;
   }

   invalidateCurrentLineMap ();
}

/* ======================================================================
FUNCTION        setEventPosition
DESCRIPTION
        Set the X,Y,Z position of an event (assuming it has one)
INPUT

OUTPUT

RETURNED  TRUE if the event had a positin (now been set) otherwise FALSE
====================================================================== */
int
#if XVT_CC_PROTO
setEventPosition (OBJECT *object, double x, double y, double z)
#else
setEventPosition (object, x, y, z)
OBJECT *object;
double x, y, z;
#endif
{
   switch (object->shape)
   {
      case (FOLD):
         {
            FOLD_OPTIONS *tempOptions;

            tempOptions = (FOLD_OPTIONS *) object->options;
            tempOptions->positionX = x;
            tempOptions->positionY = y;
            tempOptions->positionZ = z;
         }
         break;
      case (FAULT):
         {
            FAULT_OPTIONS *tempOptions;

            tempOptions = (FAULT_OPTIONS *) object->options;
            tempOptions->positionX = x;
            tempOptions->positionY = y;
            tempOptions->positionZ = z;

            plane(&tempOptions->plane.a, &tempOptions->plane.b,
                  &tempOptions->plane.c, &tempOptions->plane.d,
                  x, y, z);
         }
         break;
      case (UNCONFORMITY):
         {
            UNCONFORMITY_OPTIONS *tempOptions;

            tempOptions = (UNCONFORMITY_OPTIONS *) object->options;
            tempOptions->positionX = x;
            tempOptions->positionY = y;
            tempOptions->positionZ = z;

            plane(&tempOptions->plane.a, &tempOptions->plane.b,
                  &tempOptions->plane.c, &tempOptions->plane.d,
                  x, y, z);
         }
         break;
      case (SHEAR_ZONE):
         {
            SHEAR_OPTIONS *tempOptions;

            tempOptions = (SHEAR_OPTIONS *) object->options;
            tempOptions->positionX = x;
            tempOptions->positionY = y;
            tempOptions->positionZ = z;
         }
         break;
      case (DYKE):
         {
            DYKE_OPTIONS *tempOptions;

            tempOptions = (DYKE_OPTIONS *) object->options;
            tempOptions->positionX = x;
            tempOptions->positionY = y;
            tempOptions->positionZ = z;
         }
         break;
      case (PLUG):
         {
            PLUG_OPTIONS *tempOptions;

            tempOptions = (PLUG_OPTIONS *) object->options;
            tempOptions->positionX = x;
            tempOptions->positionY = y;
            tempOptions->positionZ = z;
         }
         break;
      case (TILT):
         {
            TILT_OPTIONS *tempOptions;

            tempOptions = (TILT_OPTIONS *) object->options;
            tempOptions->positionX = x;
            tempOptions->positionY = y;
            tempOptions->positionZ = z;
         }
         break;
      default:
         return (FALSE);
   }
      
   return (TRUE);
}
/* ======================================================================
FUNCTION        getEventPosition
DESCRIPTION
        Get the X,Y,Z position of an event (assuming it has one)
INPUT

OUTPUT

RETURNED  TRUE if the event has a position (and it is returned)
====================================================================== */
int
#if XVT_CC_PROTO
getEventPosition (OBJECT *object, double *x, double *y, double *z)
#else
getEventPosition (object, x, y, z)
OBJECT *object;
double *x, *y, *z;
#endif
{
   if (!object)
      return (FALSE);

   switch (object->shape)
   {
      case (FOLD):
         {
            FOLD_OPTIONS *tempOptions;

            tempOptions = (FOLD_OPTIONS *) object->options;
            *x = tempOptions->positionX;
            *y = tempOptions->positionY;
            *z = tempOptions->positionZ;
         }
         break;
      case (FAULT):
         {
            FAULT_OPTIONS *tempOptions;

            tempOptions = (FAULT_OPTIONS *) object->options;
            *x = tempOptions->positionX;
            *y = tempOptions->positionY;
            *z = tempOptions->positionZ;
         }
         break;
      case (UNCONFORMITY):
         {
            UNCONFORMITY_OPTIONS *tempOptions;

            tempOptions = (UNCONFORMITY_OPTIONS *) object->options;
            *x = tempOptions->positionX;
            *y = tempOptions->positionY;
            *z = tempOptions->positionZ;
         }
         break;
      case (SHEAR_ZONE):
         {
            SHEAR_OPTIONS *tempOptions;

            tempOptions = (SHEAR_OPTIONS *) object->options;
            *x = tempOptions->positionX;
            *y = tempOptions->positionY;
            *z = tempOptions->positionZ;
         }
         break;
      case (DYKE):
         {
            DYKE_OPTIONS *tempOptions;

            tempOptions = (DYKE_OPTIONS *) object->options;
            *x = tempOptions->positionX;
            *y = tempOptions->positionY;
            *z = tempOptions->positionZ;
         }
         break;
      case (PLUG):
         {
            PLUG_OPTIONS *tempOptions;

            tempOptions = (PLUG_OPTIONS *) object->options;
            *x = tempOptions->positionX;
            *y = tempOptions->positionY;
            *z = tempOptions->positionZ;
         }
         break;
      case (TILT):
         {
            TILT_OPTIONS *tempOptions;

            tempOptions = (TILT_OPTIONS *) object->options;
            *x = tempOptions->positionX;
            *y = tempOptions->positionY;
            *z = tempOptions->positionZ;
         }
         break;
      default:
         return (FALSE);
   }
      
   return (TRUE);
}

/* ======================================================================
FUNCTION        initLongJob
DESCRIPTION
        display and set paramaters for the status window that
        gives an indication of the status of events as they run
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
initLongJob (int start, int end, char *message, char *cancelLabel)
#else
initLongJob (start, end, message, cancelLabel)
int start, end;
char *message, *cancelLabel;
#endif
{
   char title[200];
   SLIST winList;
   SLIST_ELT element;
   WINDOW win;
   WINDOW messageWindow, cancelButton;

   if (batchExecution)
   {
      firstToProcess = start;
      lastToProcess = end;
      rangeToProcess = end - start;
      lastProcessed = 0;
      currentBeingProcessed = 0;
   
      minPercentIncrement = (rangeToProcess) / 50;  /* 2% increments */

      if (minPercentIncrement < 0)
         minPercentIncrement = 0;
      return;
   }

   processingLongJob = TRUE;

#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
   xvt_win_set_cursor (TASK_WIN, CURSOR_WAIT);
#endif      

   if (noStatusWin)
      return;

   if (!jobStatusWindow)   /* create the window the first time */
   {
      jobStatusWindow = createCenteredWindow (JOB_STATUS_WINDOW,
                   TASK_WIN, EM_ALL, win_124_eh, 0L);
      percentJobWindow = xvt_win_get_ctl (jobStatusWindow, JOB_PERCENTAGE);
      xvt_vobj_set_title (jobStatusWindow, "Calculation Status");
   }

                                      /* Include Cancel if needed */   
   cancelLongJob = FALSE;
   cancelButton = xvt_win_get_ctl (jobStatusWindow, JOB_STATUS_CANCEL);
   if (cancelLabel)
      xvt_vobj_set_visible (cancelButton, (BOOLEAN) TRUE);
   else
      xvt_vobj_set_visible (cancelButton, (BOOLEAN) FALSE);

                     /* position the window in the centre of the window */
   messageWindow = xvt_win_get_ctl (jobStatusWindow, JOB_MESSAGE);
   if (message && strlen(message))
      strcpy (title, message);
   else
      strcpy (title, "Calculating...");
   xvt_vobj_set_title (messageWindow, title);

   xvt_vobj_set_title (percentJobWindow, "0");

   firstToProcess = start;
   lastToProcess = end;
   rangeToProcess = end - start;
   lastProcessed = 0;
   currentBeingProcessed = 0;
   
   minPercentIncrement = (rangeToProcess) / 50;  /* 2% increments */

   if (minPercentIncrement < 0)
      minPercentIncrement = 0;

   xvt_vobj_set_visible (jobStatusWindow, TRUE);
   xvt_dwin_invalidate_rect (jobStatusWindow, NULL);  /* refresh it */
   bringWindowToFront(jobStatusWindow);

#if (XVTWS != MACWS)
                           /* disable all other windows */
   if ((winList = xvt_scr_list_wins ()) && (xvt_vobj_get_title (jobStatusWindow, title, 200)))
   {
      for (element = xvt_slist_get_first(winList); element != NULL;
                                    element = xvt_slist_get_next (winList, element))
      {
         xvt_slist_get (winList, element, (long *) &win);
         if (win != jobStatusWindow)/* disable all windows but this one */
            xvt_vobj_set_enabled (win, FALSE);
      }
      xvt_slist_destroy (winList);
   }
#endif

   xvt_app_process_pending_events();
   updateMenuOptions (TASK_MENUBAR, NULL_WIN);
}

/* ======================================================================
FUNCTION        incrementLongJob
DESCRIPTION
        updates how complete a long job that is running is
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
incrementLongJob (int position)
#else
incrementLongJob (position)
int position;
#endif
{
   char percent[100]; 
   int value;

	if (batchExecution)
	{
		if (position < 0)
		{                             /* use -= as position is already -ve */
			currentBeingProcessed -= position;
			position = currentBeingProcessed;
		}
		else
		{
			position -= firstToProcess;
			currentBeingProcessed = position;
		}

		if ((position-lastProcessed) > minPercentIncrement)
		{
			value = (int) floor((double) ((double) position
														 / (double) rangeToProcess) * 100.0);
			if (value > 100) value = 100;
			if (value < 0) value = 0;
			sprintf (percent,"Complete %d %%\n",value);
			updateBatchStatus(percent);
			lastProcessed = position;
		}

		return (TRUE);
	}

   if (noStatusWin)
      return (FALSE);

   xvt_app_process_pending_events();
   if (!processingLongJob)    /* dont do rest if no status win up */
      return (FALSE);

   if (cancelLongJob)
      return (ABORT_JOB);
      
   if (position < 0)
   {                             /* use -= as position is already -ve */
      currentBeingProcessed -= position;
      position = currentBeingProcessed;
   }
   else
   {
      position -= firstToProcess;
      currentBeingProcessed = position;
   }

   if ((position-lastProcessed) > minPercentIncrement)
   {
      if (percentJobWindow)
      {
         value = (int) floor((double) ((double) position
                                           / (double) rangeToProcess) * 100.0);
         if (value > 100) value = 100;
         if (value < 0) value = 0;
         sprintf (percent,"%d",value);
         xvt_vobj_set_title (percentJobWindow, percent);
         lastProcessed = position;
         xvt_app_process_pending_events();
      }
   }
   xvt_app_process_pending_events();

	return (TRUE);
}
/* ======================================================================
FUNCTION        finishLongJob
DESCRIPTION
        display and set paramaters for the status window that
        gives an indication of the status of events as they run
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
finishLongJob ()
#else
finishLongJob ()
#endif
{
   WINDOW win;
   SLIST winList;
   SLIST_ELT element;

   if (batchExecution || !processingLongJob)
      return;

#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
   xvt_win_set_cursor (TASK_WIN, CURSOR_ARROW);
#endif
   processingLongJob = FALSE;

   if (noStatusWin)
      return;

   xvt_app_process_pending_events();
   if (jobStatusWindow)
   {
#if (XVTWS == MACWS)
      xvt_vobj_destroy (jobStatusWindow);
      jobStatusWindow = NULL_WIN;
#else
      xvt_vobj_set_visible (jobStatusWindow, FALSE); 
#endif
   }
                               /* enable all other windows */
   if (winList = xvt_scr_list_wins ())
   {
      for (element = xvt_slist_get_first(winList); element != NULL;
                                    element = xvt_slist_get_next (winList, element))
      {
         xvt_slist_get (winList, element, (long *) &win);
         if (win != jobStatusWindow) /* disable all windows but this one */
         {
            xvt_vobj_set_enabled (win, TRUE);
            xvt_app_process_pending_events();
         }
      }
      xvt_slist_destroy (winList);
   }

   updateMenuOptions (TASK_MENUBAR, NULL_WIN);
                   /* update the history window */
   xvt_dwin_invalidate_rect(getEventDrawingWindow (), NULL);
}

/* ======================================================================
FUNCTION        abortLongJob
DESCRIPTION
        interrupt a long calculation when the cancel button is pressed
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
abortLongJob ()
#else
abortLongJob ()
#endif
{
   return (cancelLongJob);
}                                               

void
#if XVT_CC_PROTO
setAbortLongJob ()
#else
setAbortLongJob ()
#endif
{
   cancelLongJob = TRUE;
}                                               

/* ======================================================================
FUNCTION        getJobStatusWindow
DESCRIPTION
        return the window which the job status is displayed in
INPUT

OUTPUT

RETURNED
====================================================================== */
WINDOW
#if XVT_CC_PROTO
getJobStatusWindow ()
#else
getJobStatusWindow ()
#endif
{
   return ((WINDOW) jobStatusWindow);
}

/* ======================================================================
FUNCTION        setControlFont
DESCRIPTION
        Function to set the font in controls on MS-WINDOWS
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
setControlFont (WINDOW win)
#else
setControlFont (win)
WINDOW win;
#endif
{
#ifndef FCN_NODDY_ONLY
#if (XVTWS == WINWS) || (XVTWS == WIN16WS)
#undef hmemcpy
#undef MoveTo
#undef LineTo
#include <windows.h>
   HWND handle;
   handle = (HWND) xvt_vobj_get_attr (win, ATTR_NATIVE_WINDOW);
   SendMessage (handle, WM_SETFONT,
                        GetStockObject (ANSI_FIXED_FONT), TRUE);
#endif
#endif
}


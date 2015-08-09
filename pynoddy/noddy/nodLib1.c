/*
*/
#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include "titles.h"

#define DEBUG(X)    

                                /* Constants to this file */
#define TOLERANCE   0.001
#define CLIPBOARD_NAME   "Nod"
#define MAX_EVENTS  64
#define NUM_MENUS 6

#ifndef ENCOM_V6
#define NUM_WINMENU_ITEMS 7 /* 5 */
#else
#define NUM_WINMENU_ITEMS 5 /* 5 */
#endif


                                /* External variables to this file */
#if XVT_CC_PROTO
extern PROFILE_OPTIONS *getCurrentProfileOptions (WINDOW, PROFILE_OPTIONS *);
#else
extern PROFILE_OPTIONS *getCurrentProfileOptions ();
#endif

extern BOOLEAN processingLongJob;
extern WINDOW lastActiveWindow;
extern WINDOW statusBar;
extern ANOMIMAGE_DATA *gravityReferenceData, *magneticsReferenceData;
extern BOOLEAN legendShowing;
extern WINDOW legendWindow;
extern int batchExecution;



                                /* Gloabal Variables */
double iscale = 10.0; /* scaling factor for geology */
static PRINT_RCD *printerSetup = NULL;

PROJECT_OPTIONS projectOptions = {
          CGS,               /* susceptibilityUnits */
          CALC_GRAV_AND_MAG, /* geophysicalCalc */
          LOCAL_JOB,         /* calculationType */
          METRES,            /* lengthScale */
          (double) 1.0,      /* printScalingFactor */
          (double) 10.0,     /* imageScalingFactor */
          {  /* gravityDisplay */
             RELATIVE_CLIPPING, /* clippingType */
             (double) 0.0,      /* minClip */
             (double) 100.0,    /* maxClip */
             GREY_COLOR,        /* display */
             25,                /* numContours */
				 NULL,              /* lut */
				 0,                 /* lutSize */
				 FALSE,             /* gridOverlay */
				 TRUE,              /* autoGridSpacing */
				 FALSE,             /* sensorOverlay */
				 0.0, 0.0,          /* gridStartX, gridStartY */
				 1000.0, 1000.0,    /* gridIncX, gridIncY */
          },
          {  /* magneticsDisplay */
             RELATIVE_CLIPPING, /* clippingType */
             (double) 0.0,      /* minClip */
             (double) 100.0,    /* maxClip */
             GREY_COLOR,        /* display */
             25,                /* numContours */
				 NULL,              /* lut */
				 0,                 /* lutSize */
				 FALSE,             /* gridOverlay */
				 TRUE,              /* autoGridSpacing */
				 FALSE,             /* sensorOverlay */
				 0.0, 0.0,          /* gridStartX, gridStartY */
				 1000.0, 1000.0,    /* gridIncX, gridIncY */
          },
          (BOOLEAN) FALSE,      /* newWindowEachStage */
          
          "medusa.cc.monash.edu.au", /* internetAddress */
          "paul",               /* accountName */
          "./noddy",            /* noddyPath */
          "c:\\progra~1\\plus!\\micros~1\\iexplore %h" ,    /* helpPath */
          3,                    /* movieFramesPerEvent */
          10.0,                 /* moviePlaySpeed */
          BLOCK_DIAGRAM_MOVIE,  /* movieType */
          
          (double) 0.0,         /* Easting */
          (double) 0.0          /* Northing */
};

BLOCK_VIEW_OPTIONS *blockViewOptions = NULL;

GEOPHYSICS_OPTIONS geophysicsOptions = {
          1200,   /* calculationRange */
			 0, /* analyticRange */
			 TRUE, /* allExact */
          -67.0,    /* inclination */
          63000.0,/* intensity */
          0.0, /* declination */
          0.0, /* xPos */
          0.0, /* yPos */
          5000.0, /* zPos */
          0.0,    /* inclinationOri */
          0.0,    /* inclinationChange */
          90.0,    /* declinationOri */
          0.0,    /* declinationChange */
          0.0,    /* intensityOri */
          0.0,    /* intensityChange */
			 FIXED_FIELD, /* fieldType */
          0,   /* calculationAltitude */
          0.0, /* altitude */
          SPATIAL,  /* calculationMethod */
          0.0,      /* constantBoxDepth */
          1.0,      /* cleverBoxRatio */
          FALSE,    /* drapedSurvey */
          FALSE,     /* deformableRemanence */
          FALSE,     /* deformableAnistropy */
          FALSE,     /* magneticVectorComponents */
          TRUE,     /* projectVectorsOntoField */
			 TRUE,		/* padWithRealGeology */
          RAMP_PADDING, /* spectralPaddingType */
          50,        /* spectralFence */
          100,       /* spectralPercent */
};
GEOLOGY_OPTIONS geologyOptions = {
          FALSE,  /* calculateChairDiagram */
          0.0, /* chair_X */
          0.0, /* chair_Y */
          5000.0,   /* chair_Z */
          0.0,  /* boreholeX */
          0.0,  /* boreholeY */
          5000.0,  /* boreholeZ */
          90.0,  /* boreholeDecl */
          0.0,  /* boreholeDip */
          5000.0,  /* boreholeLength */
          0.0,  /* SectionX */
          0.0,  /* SectionY */
          5000.0,  /* SectionZ */
          90.0,  /* SectionDecl */
          10000.0,  /* SectionLength */
          5000.0,  /* SectionHeight */
          0.0, /* welllogAngle */
          90.0,     /* welllogDeclination */
          5000.0,   /* welllogDepth */
          FALSE,    /* useTopography */
          1.0, /* topoScaleFactor */
          0.0, /* topoOffsetFactor */
          100.0,    /* topoLowContour */
          100.0,    /* topoContourInterval */
          
           NULL, /* blockData */
           NULL, /* valueData */
           (float) 0.0, /* minValueData */
           (float) 0.0, /* maxValueData */
           NULL, /* lut */
           0, /* lutSize */
           SOLID_BLOCK, /* type */
           NULL, /* layersToDraw */
           0, /* numLayersToDraw */
           0, /* numBoreHoles */
           NULL, /* boreHoles */
           NULL, /* numPointInBoreHoles */
           0, 0, 0, /* nx, ny, nz */
           (double) 0.0,(double)  0.0,(double)  0.0, /* minXLoc, minYLoc, minZLoc */
           0.0, /* blockSize */
           
           LAYERS_3D, /* imageType */
           NULL_PIXMAP, /* pixmap */
           0.0, 0.0, 0.0, /* camera */
           0.0, 0.0, 0.0, /* focus */
           NULL, /* surface */
           0, /* surfaceWidth */
           0, /* surfaceHeight */
           0.0, 0.0, 0.0, /* declination, azimuth, scale */
           
           NULL /* surfaces */
};
THREED_VIEW_OPTIONS threedViewOptions = {
          150.0,  /* declination */
          30.0, /* azimuth */
          1.0,  /* scale */
          1.0,  /* offset_X */
          1.0,  /* offset_Y */
          1.0,  /* offset_Z */
          TRUE, /* allLayers */
          {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          }, /* layerOn[100] */
          LINES_FILL, /* fillType */
};

WINDOW_POSITION_OPTIONS winPositionOptions = {
          16,   /* numWindows */
          /* name,               x,                     y,           **
          **                     width,                 height       */
          BLOCK_WINDOW_TITLE,    BLOCK_WINDOW_POS_X,    BLOCK_WINDOW_POS_Y,
                                 BLOCK_WINDOW_WIDTH,    BLOCK_WINDOW_HEIGHT,
          MOVIE_WINDOW_TITLE,    MOVIE_WINDOW_POS_X,    MOVIE_WINDOW_POS_Y,
                                 -1,                    -1,
          WELL_LOG_TITLE,        WELL_LOG_POS_X,        WELL_LOG_POS_Y,
                                 WELL_LOG_WIDTH,        WELL_LOG_HEIGHT,
          SECTION_TITLE,         SECTION_POSITION_X,         SECTION_POSITION_Y,
                                 SECTION_WIDTH,         -1,
          TOPOGRAPHY_TITLE,      TOPOGRAPHY_POS_X,      TOPOGRAPHY_POS_Y,
                                 TOPOGRAPHY_WIDTH,      TOPOGRAPHY_HEIGHT,
          TOPOGRAPHY_3D_TITLE,   TOPOGRAPHY_3D_POS_X,   TOPOGRAPHY_3D_POS_Y,
                                 TOPOGRAPHY_3D_WIDTH,   TOPOGRAPHY_3D_HEIGHT,
          STRATIGRAPHY_3D_TITLE, STRATIGRAPHY_3D_POS_X, STRATIGRAPHY_3D_POS_Y,
                                 STRATIGRAPHY_3D_WIDTH, STRATIGRAPHY_3D_HEIGHT,
          LINEMAP_TITLE,         LINEMAP_POS_X,         LINEMAP_POS_Y,
                                 LINEMAP_WIDTH,         -1,
          PROFILE_FROM_IMAGE_TITLE, PROFILE_POS_X,      PROFILE_POS_Y,
                                 -1,                    -1,
          PROFILE_TITLE,         PROFILE_POS_X,         PROFILE_POS_Y,
                                 PROFILE_WIDTH,         PROFILE_HEIGHT,
          LINEMAP_PLOT_TITLE,    LINEMAP_PLOT_POS_X,    LINEMAP_PLOT_POS_Y,
                                 LINEMAP_PLOT_WIDTH,    LINEMAP_PLOT_HEIGHT,
          STRATIGRAPHY_TITLE,    STRATIGRAPHY_POS_X,    STRATIGRAPHY_POS_Y,
                                 STRATIGRAPHY_WIDTH,    STRATIGRAPHY_HEIGHT,
          IMAGE_TITLE,           IMAGE_POS_X,           IMAGE_POS_Y,
                                 -1,                    -1,
          CONTOUR_TITLE,         CONTOUR_POS_X,         CONTOUR_POS_Y,
                                 -1,                    -1,
          TOOLBAR_TITLE,         TOOLBAR_POS_X,         TOOLBAR_POS_Y,
                                 -1,                    -1,
          HISTORY_TITLE,         HISTORY_POS_X,         HISTORY_POS_Y,
                                 HISTORY_WIDTH,         HISTORY_HEIGHT
};

ROCK_DATABASE rockDatabase = { NULL, 0 };
DIRECTORY defaultDir;
                          
                                /* Gloabal Variables to this file */
static WINDOW winStack[20];
static int winStackHeight = 0;


                                /* Functions in this file */
#if XVT_CC_PROTO
int setDefaultDirectory (DIRECTORY *);
DIRECTORY getDefaultDirectory (DIRECTORY *);
void updateMenuOptions (int, WINDOW);
void updateTaskMenuOptions (WINDOW, WINDOW);
void updateWindowsMenu (WINDOW, WINDOW);
void tileWindows ();
void cascadeWindows ();
int restoreWindowSize (WINDOW);
void bringWindowToFront (WINDOW);
void removeAllWindows ();
void raiseWindow (int);
void setupPageForPrinter ();
BOOLEAN printPicture (long);
void getObjectsFromClipboard (OBJECT **);
int putObjectsInClipboard (OBJECT **, int);
void pasteObjects (WINDOW);
int copySelectedObjects (WINDOW);
void cutSelectedObjects (WINDOW);
void duplicateSelectedObjects (WINDOW);
void createLineMapMenubar (WINDOW, SECTION_DATA *);
void assignNoddyColourStructure (NODDY_COLOUR *, char *, int, int, int);
void addWinToStack (WINDOW);
void takeWinFromStack (WINDOW);
WINDOW stackParentWin (WINDOW);
WINDOW createPopupWindow (WINDOW, PNT, int, EVENT_HANDLER, long);
void copyOptions (int, OBJECT *, OPTION_OPERATIONS);
void updateDykeOptions (WINDOW);
void updatePlugOptions (WINDOW);
void updateXYZImportOptions (WINDOW);
void updateProfileOptions (WINDOW);
void updatePropertiesOptions (WINDOW);
double getFloatTextFieldValue (WINDOW, int);
int getIntegerTextFieldValue (WINDOW, int);
WINDOW updateFloatTextField (WINDOW, int, double, int);
WINDOW updateNumericTextField (WINDOW, int, int);
int updateScrollField (WINDOW, int, int);
int updateScrollAndText (WINDOW, int, int, int);
void setScrollRange (WINDOW, int, int, int);
int setScrollPosition (WINDOW, SCROLL_TYPE, int);
void setObjectName (OBJECT *, char *);
void getObjectName (OBJECT *, char *);
int readRockPropDatabase ();
int writeRockPropDatabase (ROCK_DATABASE *);
int freeRockPropDatabase ();
int makeEditOptionsHistory (OBJECT *);
int removeEditOptionsHistory (OBJECT *);
#else
int setDefaultDirectory ();
DIRECTORY getDefaultDirectory ();
void updateMenuOptions ();
void updateTaskMenuOptions ();
void updateWindowsMenu ();
void tileWindows ();
void cascadeWindows ();
int restoreWindowSize ();
void bringWindowToFront ();
void removeAllWindows ();
void raiseWindow ();
void setupPageForPrinter ();
BOOLEAN printPicture ();
void getObjectsFromClipboard ();
int putObjectsInClipboard ();
void pasteObjects ();
int copySelectedObjects ();
void cutSelectedObjects ();
void duplicateSelectedObjects ();
void createLineMapMenubar ();
void assignNoddyColourStructure ();
void addWinToStack ();
void takeWinFromStack ();
WINDOW stackParentWin ();
WINDOW createPopupWindow ();
void copyOptions ();
void updateDykeOptions ();
void updatePlugOptions ();
void updateXYZImportOptions ();
void updateProfileOptions ();
void updatePropertiesOptions ();
double getFloatTextFieldValue ();
int getIntegerTextFieldValue ();
WINDOW updateFloatTextField ();
WINDOW updateNumericTextField ();
int updateScrollField ();
int updateScrollAndText ();
void setScrollRange ();
int setScrollPosition ();
void setObjectName ();
void getObjectName ();
int readRockPropDatabase ();
int writeRockPropDatabase ();
int freeRockPropDatabase ();
int makeEditOptionsHistory ();
int removeEditOptionsHistory ();
#endif

/* ======================================================================
FUNCTION        setDefaultDirectory
DESCRIPTION
          set the default directory to a specific location

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
setDefaultDirectory (DIRECTORY *dir)
#else
setDefaultDirectory (dir)
DIRECTORY *dir;
#endif
{
   if (dir)
      memcpy (&defaultDir, dir, sizeof(DIRECTORY));
   else
      return (FALSE);
      
   return (TRUE);
}

/* ======================================================================
FUNCTION        getDefaultDirectory
DESCRIPTION
          get the default directory as has been set

INPUT

OUTPUT

RETURNED
====================================================================== */
DIRECTORY
#if XVT_CC_PROTO
getDefaultDirectory (DIRECTORY *dir)
#else
getDefaultDirectory (dir)
DIRECTORY *dir;
#endif
{
   if (dir)
      memcpy (dir, &defaultDir, sizeof(DIRECTORY));
      
   return (defaultDir);
}


/* ======================================================================
FUNCTION        updateMenuOptions
DESCRIPTION
          enable the menu options that are currently alloud

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateMenuOptions (int menuId, WINDOW ignoreWin)
#else
updateMenuOptions (menuId, ignoreWin)
int menuId;
WINDOW ignoreWin;
#endif
{
   WINDOW win;
   char *titlePtr;
   SLIST winList;
   SLIST_ELT element;
	WINDOW historyWin = (WINDOW) getEventDrawingWindow ();

	if (batchExecution)
		return;

   if (menuId == TASK_MENUBAR)
   {
                           /* disable all other windows */
      if (winList = xvt_scr_list_wins ())
      {
         for (element = xvt_slist_get_first(winList); element != NULL;
                                    element = xvt_slist_get_next (winList, element))
         {
            titlePtr = xvt_slist_get (winList, element, (long *) &win);
            if (winHasTaskMenu (win))
               updateTaskMenuOptions (win, ignoreWin);
         }
         xvt_slist_destroy (winList);
      }

#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
      updateTaskMenuOptions (TASK_WIN, ignoreWin);
#endif
      updateTaskMenuOptions (historyWin, ignoreWin);
   }

                             /* Make changes visible */
#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)  /* MACWS added */
   xvt_menu_update (TASK_WIN);
#endif
   if (historyWin)
      xvt_menu_update (historyWin);

/*
   if (totalObjects (NULL_WIN) > MAX_EVENTS)
      xvt_dm_post_error("Error, The Maximum Number of Events is %d.  Please Delete Some.",MAX_EVENTS);
*/
}
/* ======================================================================
FUNCTION        updateTaskMenuOptions
DESCRIPTION
          enable the menu options that are currently alloud
                on TASK_MENUBAR
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateTaskMenuOptions (WINDOW win, WINDOW ignoreWin)
#else
updateTaskMenuOptions (win, ignoreWin)
WINDOW win;
WINDOW ignoreWin;
#endif
{
   BOOLEAN historyPresent, topoPresent, grvRef, magRef;

	if (batchExecution)
		return;

   if (!win || (win == ignoreWin))
      return;

   xvt_menu_set_item_enabled (win, TASK_MENUBAR_1, (BOOLEAN) !processingLongJob);
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_2, (BOOLEAN) !processingLongJob);
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4, (BOOLEAN) !processingLongJob);
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_5, (BOOLEAN) !processingLongJob);
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_85, (BOOLEAN) !processingLongJob);
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_77, (BOOLEAN) !processingLongJob);
   if (processingLongJob)   /* everything disabled is all we want */
      return;

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
                        /* disable menus if a front options window is up */
   if (winStackHeight > 1)
   {
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_1, FALSE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_2, FALSE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_4, FALSE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_5, FALSE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_85,FALSE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_77,FALSE);
      return;
   }
   else
   {
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_1, TRUE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_2, TRUE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_4, TRUE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_5, TRUE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_85,TRUE);
      xvt_menu_set_item_enabled (win, TASK_MENUBAR_77,TRUE);
   }
#endif
   
   if (totalObjects (NULL_WIN) == 0)
      historyPresent = FALSE;
   else
      historyPresent = TRUE;
      
   if (gravityReferenceData)
      grvRef = TRUE;
   else
      grvRef = FALSE;
      
   if (magneticsReferenceData)
      magRef = TRUE;
   else
      magRef = FALSE;
      
   topoPresent = geologyOptions.useTopography;

                                              /* ********************* */
                                              /* ******** File ******* */
                                                    /* Create Movie */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_1_138, historyPresent);
                                                    /* Save Orientations */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_1_32, historyPresent);
                                                    /* Save Profile */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_1_93, TRUE);
                                                    /* Page Setup */
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_1_24, TRUE);
#else
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_1_24, FALSE);
#endif

                                              /* ********************* */
                                              /* ******** Edit ******* */
                                                    /* Undo */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_2_9, TRUE);
                                                    /* Cut */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_2_10, historyPresent);
                                                    /* Copy */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_2_75, historyPresent);
                                                    /* Clear */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_2_15, historyPresent);
                                                    /* Duplicate */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_2_20, historyPresent);
                                                    /* Select All */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_2_21, historyPresent);
                                                    /* Tidy Window */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_2_61, historyPresent);

                                              /* ********************* */
                                              /* ****** Geology ****** */
                                                    /* Block Diagram */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_27, historyPresent);
                                                    /* Line Map */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_28, historyPresent);
                                                    /* Topo Map */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_29, topoPresent);
                                                    /* Section */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_47, historyPresent);
                                                    /* Strat Column */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_70, historyPresent);
                                                    /* Well Log */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_48, historyPresent);
                                                    /* 3D Topo */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_49, (BOOLEAN) (topoPresent
                                                          && historyPresent));
                                                    /* 3D Layer */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_50, historyPresent);
                                                    /* Plot Orientations */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_4_69, historyPresent);

                                              /* ********************* */
                                              /* ***** Geophysics **** */
                                                    /* Calculate Anomilies */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_5_11_43, historyPresent);
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_5_11_44, historyPresent);
                                                    /* Difference - gravity */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_5_134_135, grvRef);
                                                    /* Difference - magnetics */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_5_134_136, magRef);
                                                    /* XYZ Points */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_5_11_90, historyPresent);
                                                    /* XYZ import */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_5_11_90_91, historyPresent);
                                                    /* XYZ Generate */
   xvt_menu_set_item_enabled (win, TASK_MENUBAR_5_11_90_92, historyPresent);

                                              /* ********************* */
                                              /* ****** Windows ****** */
#ifndef ENCOM_V6
	xvt_menu_set_item_checked (win, TASK_MENUBAR_77_18, (BOOLEAN) legendShowing);
#endif

                                              /* ********************* */
                                              /* ****** Tools ****** */

   updateWindowsMenu (win, ignoreWin);
}

/* ======================================================================
FUNCTION        updateWindowsMenu
DESCRIPTION
          enable the menu options that are currently alloud
                on TASK_MENUBAR
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateWindowsMenu (WINDOW win, WINDOW ignoreWin)
#else
updateWindowsMenu (win, ignoreWin)
WINDOW win;
WINDOW ignoreWin;
#endif
{
   WINDOW listwin;
   MENU_ITEM *menuBar;
   int numWindows, menuIndex, winMenu, winNum = 0, baseTag;
   char *titlePtr;
   SLIST winList;
   SLIST_ELT element;
   WINDOW statusWindow = (WINDOW) getJobStatusWindow ();
   WINDOW menuWindow = (WINDOW) getFloatingMenuWindow ();
   char statusBarLabel[20];

   strcpy (statusBarLabel, "Status Bar");
   menuBar = xvt_menu_get_tree (win);

                               /* get the list of windows */
   if (winList = xvt_scr_list_wins ())
   {
      numWindows = xvt_slist_count(winList);
      if (statusWindow) numWindows--;
      if (ignoreWin) numWindows--;

              /* Index of 'Windows' menu (last top level menu) */
      winMenu = NUM_MENUS - 1;
      menuBar[winMenu].child = (MENU_ITEM *) xvt_mem_realloc ((DATA_PTR)
                                             menuBar[winMenu].child,
                                             sizeof(MENU_ITEM)
                                             * (NUM_WINMENU_ITEMS+numWindows+2));
      memset((char *) &(menuBar[winMenu].child[NUM_WINMENU_ITEMS]),
                                    0, sizeof(MENU_ITEM)*(numWindows+2));
/*
      xvt_mem_rep((DATA_PTR) &(menuBar[winMenu].child[NUM_WINMENU_ITEMS]),
                                    "\0", 1, sizeof(MENU_ITEM)*(numWindows+2));
*/
            /* Remove All Menu item (-2 = item before seperator)*/
      baseTag = menuBar[winMenu].child[NUM_WINMENU_ITEMS-2].tag;
      baseTag += 2002;

      for (element = xvt_slist_get_first(winList);
                    (element != NULL);
                           element = xvt_slist_get_next (winList, element))
      {
         titlePtr = xvt_slist_get (winList, element, (long *) &listwin);
         if (listwin == statusBar)
            titlePtr = statusBarLabel;
                           /* add all but the status window to the menu */
         if ((listwin != statusWindow) && (listwin != ignoreWin))
         {
                          /* index in menu of this item */
            menuIndex = NUM_WINMENU_ITEMS + winNum;
            memset((char *) &(menuBar[winMenu].child[menuIndex]),
                                            0, sizeof(MENU_ITEM));
/*
            xvt_mem_rep((DATA_PTR) &(menuBar[winMenu].child[menuIndex]),
                                            "\0", 1, sizeof(MENU_ITEM));
*/
            menuBar[winMenu].child[menuIndex].tag = (baseTag++);
            menuBar[winMenu].child[menuIndex].text = xvt_mem_alloc(strlen(titlePtr)+2);
            strcpy(menuBar[winMenu].child[menuIndex].text, titlePtr);
            menuBar[winMenu].child[menuIndex].enabled = TRUE;
#if (XVTWS == XOLWS)
            if (listwin == menuWindow)
               menuBar[winMenu].child[menuIndex].def_item = TRUE;
#endif
            winNum++;
         }
      }
      xvt_slist_destroy (winList);
   }

   xvt_menu_set_tree (win, menuBar);
   xvt_res_free_menu_tree (menuBar);
}

/* ======================================================================
FUNCTION        tileWindows
DESCRIPTION
          Move all created windows into a tile pattern on the screen
====================================================================== */
void
#if XVT_CC_PROTO
tileWindows ()
#else
tileWindows ()
#endif
{
   WINDOW win;
   SLIST winList;
   SLIST_ELT element;
   char *titlePtr;
   RCT position, screenSize;
   PNT corner;
   int titleHeight, borderWidth, borderHeight;
   int numWinInRow, numWindows, winWidth, winHeight;
   WINDOW histWindow = (WINDOW) getEventDrawingWindow ();
   WINDOW menuWindow = (WINDOW) getFloatingMenuWindow ();
   WINDOW statusWindow = (WINDOW) getJobStatusWindow ();

   xvt_vobj_get_client_rect (TASK_WIN, &screenSize);

                               /* tile all other windows */
   if (winList = xvt_scr_list_wins ())
   {
                               /* first decide how to position them */
      numWindows = xvt_slist_count(winList) - 2;
      if (statusWindow) numWindows--; /* ignore this if it is created */
      if (statusBar) numWindows--; /* ignore this if it is created */

      if (numWindows <= 0)
      {
         xvt_slist_destroy (winList);
         return;
      }

      numWinInRow = (int) ceil(sqrt((double) numWindows));
      titleHeight = (int) xvt_vobj_get_attr(histWindow, ATTR_TITLE_HEIGHT);
      borderHeight = (int) xvt_vobj_get_attr(histWindow, ATTR_DOCFRAME_HEIGHT);
      borderWidth = (int) xvt_vobj_get_attr(histWindow, ATTR_DOCFRAME_WIDTH);

      winWidth = xvt_rect_get_width(&screenSize) / numWinInRow;
      winHeight = xvt_rect_get_height(&screenSize) / numWinInRow;
#if (XVTWS == MTFWS)
      xvt_rect_set (&position, (short) borderHeight, (short) ((titleHeight+borderHeight)*2),
                               (short) winWidth, (short) winHeight);
#else
      xvt_rect_set (&position, (short) borderHeight, (short) (titleHeight+borderHeight),
                               (short) winWidth, (short) winHeight);
#endif

      for (element = xvt_slist_get_first(winList); element != NULL;
                           element = xvt_slist_get_next (winList, element))
      {
         titlePtr = xvt_slist_get (winList, element, (long *) &win);
                           /* tile all windows but this one */
         if ((win != histWindow) &&
                          (win != menuWindow) && (win != statusWindow) && (win != statusBar))
         {
            xvt_vobj_move (win, &position);
            bringWindowToFront (win);
            DEBUG(printf("tile %d, %d - %s\n",position.left,position.top,titlePtr);)
            xvt_rect_offset (&position, (short) winWidth, (short) 0);
            if (position.right > screenSize.right)
            {
               corner.h = borderWidth;
               corner.v = position.top + winHeight;
               xvt_rect_set_pos (&position, corner);
               if (position.bottom > screenSize.bottom)
               {
                  corner.h = borderWidth;
                  corner.v = titleHeight + borderHeight;
                  xvt_rect_set_pos (&position, corner);
               }
            }
         }
      }
      xvt_slist_destroy (winList);
   }
}

/* ======================================================================
FUNCTION        cascadeWindows
DESCRIPTION
          Move all created windows into a cascade pattern on the screen
====================================================================== */
void
#if XVT_CC_PROTO
cascadeWindows ()
#else
cascadeWindows ()
#endif
{
   WINDOW win;
   SLIST winList;
   SLIST_ELT element;
   int winNum = 0;
   int cascadeWidth, cascadeHeight, titleHeight, borderHeight;
   int startX, startY;
   char *titlePtr;
   RCT position, screenSize;
   PNT corner;
   WINDOW histWindow = (WINDOW) getEventDrawingWindow ();
   WINDOW menuWindow = (WINDOW) getFloatingMenuWindow ();
   WINDOW statusWindow = (WINDOW) getJobStatusWindow ();

   xvt_vobj_get_client_rect (TASK_WIN, &screenSize);

                               /* cascade all other windows */
   if (winList = xvt_scr_list_wins ())
   {
                               /* first decide how to position them */
      cascadeWidth = (int) xvt_vobj_get_attr(histWindow, ATTR_DOC_STAGGER_HORZ);
      cascadeHeight= (int) xvt_vobj_get_attr(histWindow, ATTR_DOC_STAGGER_VERT);
      titleHeight  = (int) xvt_vobj_get_attr(histWindow, ATTR_TITLE_HEIGHT);
      borderHeight = (int) xvt_vobj_get_attr(histWindow, ATTR_DOCFRAME_HEIGHT);

#if (XVTWS == MTFWS)
      startX = borderHeight;
      startY = (borderHeight + titleHeight) * 2;
#else
      startX = borderHeight;
      startY = borderHeight + titleHeight;
#endif
#if (XVTWS == MACWS)
      startY += 30;
#endif

      for (element = xvt_slist_get_first(winList); element != NULL;
                           element = xvt_slist_get_next (winList, element))
      {
         titlePtr = xvt_slist_get (winList, element, (long *) &win);
                           /* cascade all windows but these */
         if ((win != histWindow) &&
                          (win != menuWindow) && (win != statusWindow) && (win != statusBar))
         {
            xvt_vobj_get_client_rect (win, &position);
            corner.h = startX + (winNum * cascadeWidth);
            corner.v = startY + (winNum * cascadeHeight);
            DEBUG(printf("cascase %d, %d - %s\n",corner.h,corner.v,titlePtr);)

            xvt_rect_set_pos (&position, corner);
            if ((position.bottom-((position.bottom-position.top)/2) > screenSize.bottom))
            {
               winNum = 0;
               corner.h = startX + (winNum * cascadeWidth);
               corner.v = startY + (winNum * cascadeHeight);
               xvt_rect_set_pos (&position, corner);
            }

            xvt_vobj_move (win, &position);
            bringWindowToFront (win);
            winNum++;
         }
      }
      xvt_slist_destroy (winList);
   }
}

/* ======================================================================
FUNCTION        restoreWindowSize
DESCRIPTION
          change the current window back to the size it was created at
RETURN      TRUE - sucess  FALSE - failure (invalid window or something)
====================================================================== */
int
#if XVT_CC_PROTO
restoreWindowSize (WINDOW win)
#else
restoreWindowSize (win)
WINDOW win;
#endif
{
   RCT position;
   XVT_PIXMAP pixmap;
   
   if (!win)
      return (FALSE);
      
   if (!(pixmap = getWindowPixmap (win)))
      return (FALSE);
      

   xvt_vobj_get_client_rect (pixmap, &position);
   
   CORRECT_WIN_RESIZE(win, position)

   xvt_vobj_move (win, &position);

   xvt_dwin_invalidate_rect (win, NULL);

   return (TRUE);
}                                                                     

/* ======================================================================
FUNCTION        bringWindowToFront
DESCRIPTION
          bring the window to the fore most and give focus
====================================================================== */
void
#if XVT_CC_PROTO
bringWindowToFront (WINDOW win)
#else
bringWindowToFront (win)
WINDOW win;
#endif
{
#if (XVTWS == MACWS)
   WindowPtr macWindow;
#endif

   if (!win)
      return;

#if (XVTWS == MACWS)
   macWindow = (WindowPtr) xvt_vobj_get_attr (win, ATTR_NATIVE_WINDOW);
   SelectWindow (macWindow);
#else
   xvt_vobj_raise (win);
   xvt_scr_set_focus_vobj (win);
#endif
}                                                                     

/* ======================================================================
FUNCTION        removeAllWindows
DESCRIPTION
          Move all created windows into a cascade pattern on the screen
====================================================================== */
void
#if XVT_CC_PROTO
removeAllWindows ()
#else
removeAllWindows ()
#endif
{
   WINDOW win;
   SLIST winList;
   SLIST_ELT element;
   char *titlePtr;
   ASK_RESPONSE response;
   WINDOW histWindow = (WINDOW) getEventDrawingWindow ();
   WINDOW menuWindow = (WINDOW) getFloatingMenuWindow ();
   WINDOW statusWindow = (WINDOW) getJobStatusWindow ();

   response = xvt_dm_post_ask ("Cancel", "OK", NULL,
                      "Are you Sure you Want to Destroy All the Windows ?");
   if (response == RESP_DEFAULT)
      return;

                               /* remove all other windows */
   if (winList = xvt_scr_list_wins ())
   {
      for (element = xvt_slist_get_first(winList); element != NULL;
                           element = xvt_slist_get_next (winList, element))
      {
         titlePtr = xvt_slist_get (winList, element, (long *) &win);
                           /* remove all windows but these */
         if ((win != histWindow) && (win != menuWindow) && (win != statusWindow)
				                     && (win != statusBar)  && (win != legendWindow))
			{
				xvt_vobj_set_visible(win, FALSE);
            xvt_vobj_destroy (win);
			}
      }
      xvt_slist_destroy (winList);
   }

   updateMenuOptions (TASK_MENUBAR, NULL_WIN);
}

/* ======================================================================
FUNCTION        raiseWindow
DESCRIPTION
          bring the numbered window to the front so it can be viewed
====================================================================== */
void
#if XVT_CC_PROTO
raiseWindow (int winNum)
#else
raiseWindow (winNum)
int winNum;
#endif
{
   WINDOW win;
   SLIST winList;
   SLIST_ELT element;
   char *titlePtr;
   int currentWin = 0;
   WINDOW statusWindow = (WINDOW) getJobStatusWindow ();

   DEBUG(printf("RAISE WINDOW # %d\n",winNum);)

                               /* remove all other windows */
   if (winList = xvt_scr_list_wins ())
   {
      for (element = xvt_slist_get_first(winList); element != NULL;
                           element = xvt_slist_get_next (winList, element))
      {
         titlePtr = xvt_slist_get (winList, element, (long *) &win);
                           /* remove all windows but this one */
         if (win != statusWindow)
         {
            currentWin++;
            if (currentWin == winNum)
				{
               bringWindowToFront (win);
               setWindowState(win, WIN_NORMAL);
				}
         }
      }
      xvt_slist_destroy (winList);
   }
}

/* ======================================================================
FUNCTION        setupPageForPrinter
DESCRIPTION
          Call the page setup dialog and store the results

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
setupPageForPrinter ()
#else
setupPageForPrinter ()
#endif
{
   int size;

   if (!printerSetup)
   {
      if (!(printerSetup = (PRINT_RCD *) xvt_print_create (&size)))
      {
         xvt_dm_post_error("Error, Could not create a Printer definition");
         return;
      }
   }
   xvt_dm_post_page_setup (printerSetup);
}

/* ======================================================================
FUNCTION        printPicture
DESCRIPTION
          the print function for a given picture

INPUT

OUTPUT

RETURNED    TRUE on sucess, FALSE on failure
====================================================================== */
BOOLEAN
#if XVT_CC_PROTO
printPicture (long data)
#else
printPicture (data)
long data;
#endif
{
   XVT_PIXMAP pixmap;
   int intSize;
   WINDOW printWindow;
   RCT *band;
   RCT scrFrame, dstFrame;

                  /* use default printer setup if we dont already have one */
   if (!printerSetup)
      if (!(printerSetup = (PRINT_RCD *) xvt_print_create (&intSize)))
         return (FALSE);

   pixmap = (XVT_PIXMAP) data;
   xvt_vobj_get_client_rect (pixmap, &scrFrame);
   dstFrame.top = scrFrame.top;
   dstFrame.left = scrFrame.left;
   dstFrame.bottom = (int) (projectOptions.printScalingFactor * (double) scrFrame.bottom);
   dstFrame.right  = (int) (projectOptions.printScalingFactor * (double) scrFrame.right);

   if (!(printWindow = xvt_print_create_win (printerSetup, "Printing Picture...")))
      return (FALSE);

   if (xvt_print_open_page (printerSetup))
   {
      while ((band = xvt_print_get_next_band()) != NULL)
      {
         xvt_dwin_draw_pmap (printWindow, pixmap, &dstFrame, &scrFrame);
      }
      xvt_print_close_page (printerSetup);
   }
   xvt_vobj_destroy(printWindow);

   return (TRUE);
}

/* ======================================================================
FUNCTION        getObjectsFromClipboard
DESCRIPTION
        copy a list of objects into the clipboard
        the format is

        interger + object structure + options structure
                 + object structure + options structure
                 + object structure + options structure
                            .
                            .
                            .

        where integer is the number of objects in this group
        and object structure + options structure are repeated
        for each object
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
getObjectsFromClipboard (OBJECT **objectList)
#else
getObjectsFromClipboard (objectList)
OBJECT **objectList;
#endif
{
   int numObjects;
   long totalSize;
   char far *clipPointer;
   int i, sizeOfObject;
   OBJECT *object, *endList = NULL;

   if (!xvt_cb_open(FALSE))
   {
      xvt_dm_post_error("Error opening clipboard.");
      return;
   }

   DEBUG(printf("\nSTART - GET OBJECT FROM CLIPBOARD");)

   sizeOfObject = sizeof (OBJECT);   /* the room for the object structure */

   if ((clipPointer = xvt_cb_get_data(CB_APPL, CLIPBOARD_NAME, &totalSize)) == NULL)
   {
      xvt_dm_post_note("No data to paste from clipboard.");
   }
   else
   {                           /* ***************************** */
                               /* Assign the malloced structure */
      numObjects =  *(int far *) clipPointer;
      clipPointer += sizeof(int);

      DEBUG(printf("\nGOT %d OBJECTS FROM CLIPBOARD",numObjects);)
      for (i = 0; i < numObjects; i++)
      {
                               /* copy the object structure */
         object = (OBJECT *) xvt_mem_alloc(sizeOfObject);
         memcpy ((char far *) object, (char far *) clipPointer,
                                      sizeOfObject);
         clipPointer += sizeOfObject;
                               /* copy the options structure */
         object->options = (char *) xvt_mem_alloc (sizeofOptions(object->shape));
         memcpy ((char far *) object->options, (char far *) clipPointer,
                                          sizeofOptions(object->shape));

                  /* make sure things with pointers are re-initialised */
                  /* setup The profiles or Alterations this event has */
         initAllProfiles (object);

         clipPointer += sizeofOptions(object->shape);

         DEBUG(printf("\nOBJECTS %d is TYPE %d",i,object->shape);)
         if (endList == NULL)
         {             /* this object is the first */
            object->next = NULL;
            *objectList = object;
            endList = object;
         }
         else
         {             /* put this object on the end of the others */
            endList->next = object;
            endList = object;
            endList->next = NULL;
         }
      }
   }

   if (!xvt_cb_close())
      xvt_dm_post_error("Error closing clipboard.");
   DEBUG(printf("\nEND - GET OBJECT FROM CLIPBOARD");)
}
/* ======================================================================
FUNCTION        putObjectsInClipboard
DESCRIPTION
        copy a list of objects into the clipboard
        the format is

        interger + object structure + options structure
                 + object structure + options structure
                 + object structure + options structure
                            .
                            .
                            .

        where integer is the number of objects in this group
        and object structure + options structure are repeated
        for each object
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
putObjectsInClipboard (OBJECT *objectList[], int numObjects)
#else
putObjectsInClipboard (objectList, numObjects)
OBJECT *objectList[];
int numObjects;
#endif
{
   int totalSize;
   char far *clipPointer;
   int i, sizeOfObject;

   DEBUG(printf("\nCOPYING OBJECTS TO CLIPBOARD");)
   if (!xvt_cb_open(TRUE))
   {
      xvt_dm_post_error("Error opening clipboard.");
      return (FALSE);
   }

   sizeOfObject = sizeof (OBJECT);   /* the room for the object structure */

                    /* ***************************************** */
                    /* Work out the size of the structure needed */
   totalSize = sizeof(int);  /* the room needed to store the count */
   for (i = 0; i < numObjects; i++)
   {
      DEBUG(printf("\nOBJECT %d is SIZE %d TYPE %d",i,sizeOfObject+sizeofOptions(objectList[i]->shape),objectList[i]->shape);)
      totalSize += sizeOfObject;      /* the room for the object structure */
      totalSize += sizeofOptions(objectList[i]->shape); /* room for options */
   }

   if ((clipPointer = xvt_cb_alloc_data(totalSize)) == NULL)
   {
      xvt_dm_post_error("Cannot allocate clipboard memory.");
      xvt_cb_close();
      return (FALSE);
   }

                    /* ***************************** */
                    /* Assign the malloced structure */
   *(int far *) clipPointer = numObjects;
   clipPointer += sizeof(int);
   for (i = 0; i < numObjects; i++)
   {
                               /* copy the object structure */
      memcpy ((char far *) clipPointer, (char far *) objectList[i],
                                         sizeOfObject);
      clipPointer += sizeOfObject;
                               /* copy the options structure */
      memcpy ((char far *) clipPointer, (char far *) objectList[i]->options,
                             sizeofOptions(objectList[i]->shape));
      clipPointer += sizeofOptions(objectList[i]->shape);
   }
   if (!xvt_cb_put_data(CB_APPL, CLIPBOARD_NAME, (long) totalSize, (PICTURE)NULL))
      xvt_dm_post_error("Error putting data onto clipboard.");
   xvt_cb_free_data();

   if (!xvt_cb_close())
      xvt_dm_post_error("Error closing clipboard.");
   DEBUG(printf("\n END COPYING OBJECTS TO CLIPBOARD");)

   return (TRUE);
}
/* ======================================================================
FUNCTION        pasteObjects
DESCRIPTION
     copy anthing from the clipboard back and re-insert them
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
pasteObjects (WINDOW win)
#else
pasteObjects (win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip;
   OBJECT *objectList = NULL, *tempList;
   OBJECT *p;
   EVENT mouseEvent;

   getObjectsFromClipboard (&objectList);
   if (!objectList)
      return;
DEBUG(printf("PASTING OBJECTS - GOT THEM");)

   wip = (WINDOW_INFO *) get_win_info(win);
   copyUndoPositions (wip, FALSE);
   if (wip->head)
   {
                          /* stop the ability to paste in two stratigraphies */
      if ((wip->head->shape == STRATIGRAPHY)
                            && (objectList->shape == STRATIGRAPHY))
      {
         xvt_dm_post_error("Error, You cannot Paste in a Second Stratigraphy");
         tempList = objectList->next;

                        /* free the stratigriphy we were about to paste */
         if (objectList->options)
            xvt_mem_free(objectList->options);
         xvt_mem_free ((char *) objectList);

         if (!tempList)
            return;
         else
            objectList = tempList;
      }

                        /* the only thing selected will be what is pasted */
      for (p = wip->head; p->next != NULL; p = p->next)
         p->selected = FALSE;

      for (p = wip->head; p->next != NULL;)
         p = p->next;


           /* p is now the current last one in the list */
      p->selected = FALSE;
      p->next = objectList;    /* append all the pasted object at the end */
   }
   else
   {
      wip->head = objectList;
      updateMenuOptions (TASK_MENUBAR, NULL_WIN); /* first object */
   }


                    /* store the new objects in the Undo Buffer */
   copyUndoAdditions ((OBJECT *) NULL, FALSE);
   for (p = objectList; p != NULL; p = p->next)
      copyUndoAdditions (p, FALSE);

          /* get the mouse event handler to drag around this paste
          ** buffer as a selection and place it where the button is 
          ** pressed like a move event */
   mouseEvent.type = E_MOUSE_DOWN;
   mouseEvent.v.mouse.where.h = (objectList->bound.left + 5)
                                              - wip->origin.h;
   mouseEvent.v.mouse.where.v = (objectList->bound.top + 5)
                                              - wip->origin.v;
   doMouse (win, &mouseEvent);
   
}

/* ======================================================================
FUNCTION        copySelectedObjects
DESCRIPTION
     copy the selection to the clipboard
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
copySelectedObjects (WINDOW win)
#else
copySelectedObjects (win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip;
   OBJECT **objectList;
   int numSelected, success;
   OBJECT *p;

   wip = (WINDOW_INFO *) get_win_info(win);
   numSelected = 0;
   for (p = wip->head; p != NULL; p = p->next)
   {
      if (p->selected)
         numSelected++;
   }
 
   if (!numSelected)
      return (FALSE);

   objectList = (OBJECT **) xvt_mem_alloc (numSelected * sizeof (OBJECT *));
   
   numSelected = 0;
   for (p = wip->head; p != NULL; p = p->next)
   {
      if (p->selected)
      {
         objectList[numSelected] = p;
         numSelected++;
      }
   }

   DEBUG(printf("\n Num Selected = %d",numSelected);)
   success = putObjectsInClipboard (objectList, numSelected);
   xvt_mem_free ((char *) objectList);

   return (success);
}

/* ======================================================================
FUNCTION        cutSelectedObjects
DESCRIPTION
     copy the selection to the clipboard and then delete it
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
cutSelectedObjects (WINDOW win)
#else
cutSelectedObjects (win)
WINDOW win;
#endif
{
   if (copySelectedObjects (win))
      deleteSelectedObjects (win, FALSE);
}

/* ======================================================================
FUNCTION        duplicateSelectedObjects
DESCRIPTION
     copy the selection to the clipboard and then paste it
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
duplicateSelectedObjects (WINDOW win)
#else
duplicateSelectedObjects (win)
WINDOW win;
#endif
{
   if (copySelectedObjects (win))
      pasteObjects (win);
}

/* ======================================================================
FUNCTION        createLineMapMenubar
DESCRIPTION
        create the menu bar that reflects the hierachy of events
        created and assign it to the window.
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
createLineMapMenubar (WINDOW lineMapWindow, SECTION_DATA *lineMapData)
#else
createLineMapMenubar (lineMapWindow, lineMapData)
WINDOW lineMapWindow;
SECTION_DATA *lineMapData;
#endif
{
   WINDOW_INFO *wip;
   WINDOW eventParentWindow = (WINDOW) getEventDrawingWindow ();
   MENU_ITEM *item, *item1, *item11;
   MENU_ITEM *lineMapMenubar = NULL;
   MENU_ITEM *menuArray1 = NULL, *menuArray2 = NULL;
   MENU_ITEM *menuArray11 = NULL, *menuArray22 = NULL;
   OBJECT *p, *q;
   char label[OBJECT_TEXT_LENGTH];
   int numRows, numColumns, minColumn, row, column;
   int i, j, k, tag = 2000, groupNumber = 0;
   int tagEvent;

   if ((!lineMapWindow) || (!eventParentWindow))
      return;

   DEBUG(printf("\nIN createLineMapMenubar");)

   wip = (WINDOW_INFO *) get_win_info (eventParentWindow);

   if (!(lineMapMenubar = xvt_res_get_menu (MENU_BAR_2)))
      return;

                        /* make sure something is there to process */
   if (wip->head && wip->head->next)
   {
               /* get rid of what is under the menus by default */
      xvt_res_free_menu_tree (lineMapMenubar[1].child);
      xvt_res_free_menu_tree (lineMapMenubar[2].child);

                         /* work out how many levels of menu are needed */
      for (p = wip->head->next; p != NULL; p = p->next)
         numColumns = p->column;
                         /* if nothing on same col as strat then leave it out */
      if (wip->head->column == wip->head->next->column)
         minColumn = 0;
      else
      {
         minColumn = 1;
         numColumns--;
      }

                         /* allocate 1 item for each column + the null one */
      if (!(menuArray1 = (MENU_ITEM *) xvt_mem_zalloc(sizeof(MENU_ITEM)
                                                     * (numColumns+1))))
         return;
      if (!(menuArray2 = (MENU_ITEM *) xvt_mem_zalloc(sizeof(MENU_ITEM) 
                                                     * (numColumns+1))))
         return;


                         /* Assign the names on the initial menu levels */
      for (p = wip->head->next; (p != NULL) && (p->shape != STOP); p = p->next)
      {
                 /* if this is the first of a column or the first one after
                 ** the stratigraphy then it will be at this first level */
         if ((p->row == 1) || ((p->column == 1) && (p->row == 2)))
         {
            column = p->column - minColumn - 1;


                /* check if a group of events in the one column need to be **
                ** processed into a sub menu */
            if (p->next && (p->next->column == p->column))
            {                 /* make the submenu */
               groupNumber++;
               sprintf (label, "Group %d", groupNumber);

               if (!(menuArray1[column].text = xvt_mem_alloc (strlen(label)+1)))
                  return;
               if (!(menuArray2[column].text = xvt_mem_alloc (strlen(label)+1)))
                  return;

               strcpy (menuArray1[column].text, label);
               strcpy (menuArray2[column].text, label);
               menuArray1[column].mkey = (short) label[0];
               menuArray2[column].mkey = (short) label[0];
               menuArray1[column].enabled = TRUE;
               menuArray2[column].enabled = TRUE;
               menuArray1[column].checkable = FALSE;
               menuArray2[column].checkable = FALSE;
               menuArray1[column].tag = tag;
               menuArray2[column].tag = tag;

                         /* work out how many levels of menu are needed */
               for (q = p->next; (q != NULL); q = q->next)
               {
                  if (q->column == p->column)
                     numRows = q->row;
               }
               
                         /* allocate 1 item for each column + the null one */
               if (!(menuArray11 = (MENU_ITEM *) xvt_mem_zalloc(sizeof(MENU_ITEM)
                                                     * (numRows+1))))
                  return;
               if (!(menuArray22 = (MENU_ITEM *) xvt_mem_zalloc(sizeof(MENU_ITEM) 
                                                     * (numRows+1))))
                  return;

               for (q = p; (q != NULL); q = q->next)
               {
                  if (q->column == p->column)
                  {
                     if (q->column == wip->head->column)
                        row = q->row - 2;  /* leave out stratigraphy */
                     else
                        row = q->row - 1;

                     if (!(menuArray11[row].text
                                            = xvt_mem_alloc (strlen(q->text)+1)))
                        return;
                     if (!(menuArray22[row].text
                                            = xvt_mem_alloc (strlen(q->text)+1)))
                        return;

                     strcpy (menuArray11[row].text, q->text);
                     strcpy (menuArray22[row].text, q->text);
                     menuArray11[row].mkey = (short) (q->text)[0];
                     menuArray22[row].mkey = (short) (q->text)[0];
                     menuArray11[row].enabled = TRUE;
                     menuArray22[row].enabled = TRUE;
                     menuArray11[row].checkable = TRUE;
                     menuArray22[row].checkable = TRUE;
                     menuArray11[row].tag = tag;
                     menuArray22[row].tag = tag;
                     menuArray11[row].child = NULL;
                     menuArray22[row].child = NULL;
                     p = q;
                  }
                
               }

               menuArray1[column].child = menuArray11;;
               menuArray2[column].child = menuArray22;
            
            }
            else   /* just add the option at this level of menu */
            {
               if (!(menuArray1[column].text = xvt_mem_alloc (strlen(p->text)+1)))
                  return;
               if (!(menuArray2[column].text = xvt_mem_alloc (strlen(p->text)+1)))
                  return;

               strcpy (menuArray1[column].text, p->text);
               strcpy (menuArray2[column].text, p->text);
               menuArray1[column].mkey = (short) (p->text)[0];
               menuArray2[column].mkey = (short) (p->text)[0];
               menuArray1[column].enabled = TRUE;
               menuArray2[column].enabled = TRUE;
               menuArray1[column].checkable = TRUE;
               menuArray2[column].checkable = TRUE;
               menuArray1[column].tag = tag;
               menuArray2[column].tag = tag;
               menuArray1[column].child = NULL;
               menuArray2[column].child = NULL;
            }
         }
      }
                          /* put in the null terminating one */
      menuArray1[numColumns].tag = 0;
      menuArray2[numColumns].tag = 0;

      lineMapMenubar[1].child = menuArray1;
      lineMapMenubar[2].child = menuArray2;
   }

                 /* set the tags on the structure to unique numbers starting
                 ** at 2000 */
   i = 0;
   item = &lineMapMenubar[i];
   for (i = 0, item = &lineMapMenubar[i]; (item->tag != 0);
                                         i++, item = &lineMapMenubar[i])
   {
      if (item->child != NULL)
      {
         tagEvent = 100;       /* events are labeled startin at tag + 100 */
         menuArray1 = item->child;

         for (j = 0, item1 = &menuArray1[j]; (item1->tag != 0);
                                           j++, item1 = &menuArray1[j])
         {
            if (item1->child == NULL)   /* no submenu then label event */
            {
               if ((tagEvent == 100) && (item1->checkable))
                  item1->checked = TRUE;
               item1->tag = tag + tagEvent + 1;
               tagEvent++;
            }
            else    /* the submenu must contain the event */
            {
               item1->tag = tag + j + 1;  /* label as a sub-menu not an event */
           
               menuArray11 = item1->child;
               for (k = 0, item11 = &menuArray11[k]; (item11->tag != 0);
                                                 k++, item11 = &menuArray11[k])
               {
                  if ((tagEvent == 100) && (item1->checkable))
                     item1->checked = TRUE;
                  item11->tag = tag + tagEvent + 1;
                  tagEvent++;
               }
            }
         }
      }
      item->tag = tag;  /* label top menus in increments of 1000 */
      tag += 1000;
   }
   lineMapData->symbol = BEDDING_SYM;
   lineMapData->event1 = 1;
   lineMapData->event2 = 1;
   lineMapData->define = 1;
   lineMapData->count = 0;


#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
   xvt_menu_set_tree (lineMapWindow, lineMapMenubar);
#else
   {
                     /* append this new menu to the main menubar */
      MENU_ITEM *mainMenubar = NULL;
      MENU_ITEM *newLineMapMenubar = NULL;
      int error = TRUE;

      if (mainMenubar = xvt_res_get_menu (TASK_MENUBAR))
      {
         if (!(newLineMapMenubar = (MENU_ITEM *)
                                   xvt_mem_zalloc(sizeof(MENU_ITEM) * (NUM_MENUS+5))))
         {
            xvt_res_free_menu_tree (mainMenubar);
         }
         else
         {
                      /* put the main menu in the first 4 spaces */
            for (i = 0; i < NUM_MENUS; i++)
               memcpy (&(newLineMapMenubar[i]), &(mainMenubar[i]),
                                                sizeof (MENU_ITEM));
                      /* put out linemap menu in next 4 spaces */
            for (i = 0; i < 4; i++)
               memcpy (&(newLineMapMenubar[i+NUM_MENUS]), &(lineMapMenubar[i]),
                                                  sizeof (MENU_ITEM));
                      /* Terminate this dual menu */
            newLineMapMenubar[NUM_MENUS+4].tag = 0;

            xvt_menu_set_tree (lineMapWindow, newLineMapMenubar);
            xvt_res_free_menu_tree (mainMenubar);
            xvt_mem_free ((char *) newLineMapMenubar);
            error = FALSE;  /* we have succeeded in adding the two together */
         }
      }

      if (error)  /* just do it the normal way with just the menu */
         xvt_menu_set_tree (lineMapWindow, lineMapMenubar);
   }
   updateTaskMenuOptions (lineMapWindow, NULL_WIN);
#endif

   xvt_res_free_menu_tree (lineMapMenubar);

   DEBUG(printf("\nFINISHED createLineMapMenubar");)
}

/* ======================================================================
FUNCTION        assignNoddyColourStructure
DESCRIPTION
        assign all the elements of the noddy color structure
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
assignNoddyColourStructure (NODDY_COLOUR *color, char *name,
                            int red, int green, int blue)
#else
assignNoddyColourStructure (color, name, red, green, blue)
NODDY_COLOUR *color;
char *name;
int red, green, blue;
#endif
{
   strcpy (color->name, name);
   color->red = red;
   color->green = green;
   color->blue = blue;
}

/* ======================================================================
FUNCTION        addWinToStack
DESCRIPTION
        add the window to the heirachy of windows
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
addWinToStack (WINDOW win)
#else
addWinToStack (win)
WINDOW win;
#endif
{
   int i;
   WINDOW parent;
    
                /* Only add top level windows to this stack */
   parent = xvt_vobj_get_parent (win);
   if ((parent != TASK_WIN) && (parent != SCREEN_WIN))
      return;

                       /* make sure only the last window is active */
   for (i = 0; i < winStackHeight; i++)
      xvt_vobj_set_enabled (winStack[i], FALSE);

   winStack[winStackHeight] = win;
   winStackHeight++;

                    /* update the menus for the first window added */
   if (winStackHeight == 2)
      updateMenuOptions (TASK_MENUBAR, win);


}
/* ======================================================================
FUNCTION        takeWinFromStack
DESCRIPTION
        take the window from the heirachy of windows
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
takeWinFromStack (WINDOW win)
#else
takeWinFromStack (win)
WINDOW win;
#endif
{
   int i, found = FALSE;
   WINDOW parent;

                /* Only top level windows in this stack */
   parent = xvt_vobj_get_parent (win);
   if ((parent != TASK_WIN) && (parent != SCREEN_WIN))
      return;

           /* see if the window is in the stack */
   for (i = 0; i < winStackHeight; i++)
   {
      if (winStack[i] == win)
      {
         found = TRUE;
         break;
      }
   }
              /* move everything down the stack */
   for ( ; i < winStackHeight; i++)
   {
      winStack[i] = winStack[i+1];
   }
   
   if (found)
      winStackHeight--;

                       /* make sure the last window is active */
   if (winStackHeight > 0)
      xvt_vobj_set_enabled (winStack[winStackHeight-1], TRUE);

                 /* only drawing window showing so refresh it */
   if (winStackHeight == 1)
   {
      log_invalidate_rect (getEventDrawingWindow (), NULL);
                    /* update the menus for the last window removed */
      updateMenuOptions (TASK_MENUBAR, win);
   }

}
/* ======================================================================
FUNCTION        stackParentWin
DESCRIPTION
        return the parent window in the stack
INPUT

OUTPUT

RETURNED
====================================================================== */
WINDOW
#if XVT_CC_PROTO
stackParentWin (WINDOW win)
#else
stackParentWin (win)
WINDOW win;
#endif
{
   int i;
   WINDOW parent;
   
                /* Only top level windows in this stack */
   parent = xvt_vobj_get_parent (win);
   if ((parent != TASK_WIN) && (parent != SCREEN_WIN))
      return (parent);

           /* see if the window is in the stack */
   for (i = 0; i < winStackHeight; i++)
   {
      if (winStack[i] == win)
      {
         if (i > 0)
            return (winStack[i-1]);
         else
            return (TASK_WIN);
      }
   }

   return (TASK_WIN);
}

/* ======================================================================
FUNCTION        createPopupWindow
DESCRIPTION
        create a popup
INPUT

OUTPUT

RETURNED
====================================================================== */
WINDOW
#if XVT_CC_PROTO
createPopupWindow (WINDOW insideWindow, PNT location, int windowId,
                   EVENT_HANDLER eventHandler, long data)
#else
createPopupWindow (insideWindow, location, windowId, eventHandler, data)
WINDOW insideWindow;
PNT location;
int windowId;
EVENT_HANDLER eventHandler;
long data;
#endif
{
   RCT rect;
   WINDOW popupWin;
   WIN_DEF *windowDefinition;
	int screenWidth, screenHeight, diff;

   screenWidth = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_WIDTH);
   screenHeight = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_HEIGHT);

   windowDefinition = xvt_res_get_win_def (windowId);
                      /* give child windows a set style */
   windowDefinition[0].wtype = W_DBL; 
   
   popupWin = (WINDOW) xvt_win_create_def (windowDefinition,
                       TASK_WIN, EM_ALL, eventHandler, data);
   xvt_res_free_win_def (windowDefinition);

   xvt_vobj_get_client_rect (popupWin, &rect);
   xvt_vobj_translate_points (insideWindow, TASK_WIN, &location, 1);
   xvt_rect_set_pos (&rect, location);
	if ((rect.bottom + 20) > screenHeight)
	{
		diff = (rect.bottom + 20) - screenHeight;
		xvt_rect_offset(&rect, (short) 0, (short) -diff);
	}
	if (rect.right > screenWidth)
	{
		diff = rect.right - screenWidth;
		xvt_rect_offset(&rect, (short) -diff, (short) 0);
	}
   xvt_vobj_move (popupWin, &rect);
   xvt_vobj_set_visible (popupWin, TRUE);
#if (XVTWS == MTFWS) || (XVTWS == XOLWS)
#else
   bringWindowToFront (popupWin);
#endif

   return (popupWin);
}

/* ======================================================================
FUNCTION        copyOptions
DESCRIPTION
        take a copy of the current options that can later be restored
INPUT
       save - TRUE the objects options are stored
            - FALSE the last stored options of this type are restored

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
copyOptions (int parentId, OBJECT *object, OPTION_OPERATIONS save)
#else
copyOptions (parentId, object, save)
int parentId;
OBJECT *object;
OPTION_OPERATIONS save;
#endif
{
   static char names[GENERIC+2][OBJECT_TEXT_LENGTH];
   static STRATIGRAPHY_OPTIONS copyStratOptions = { 0, NULL };
   STRATIGRAPHY_OPTIONS *stratOptions;
   static FOLD_OPTIONS copyFoldOptions;
   FOLD_OPTIONS *foldOptions;
   static FAULT_OPTIONS copyFaultOptions;
   FAULT_OPTIONS *faultOptions;
   static UNCONFORMITY_OPTIONS copyUnconOptions;
   UNCONFORMITY_OPTIONS *unconOptions;
   static SHEAR_OPTIONS copyShearOptions;
   SHEAR_OPTIONS *shearOptions;
   static DYKE_OPTIONS copyDykeOptions;
   DYKE_OPTIONS *dykeOptions;
   static PLUG_OPTIONS copyPlugOptions;
   PLUG_OPTIONS *plugOptions;
   static STRAIN_OPTIONS copyStrainOptions;
   STRAIN_OPTIONS *strainOptions;
   static TILT_OPTIONS copyTiltOptions;
   TILT_OPTIONS *tiltOptions;
   static FOLIATION_OPTIONS copyFolOptions;
   FOLIATION_OPTIONS *folOptions;
   static LINEATION_OPTIONS copyLinOptions;
   LINEATION_OPTIONS *linOptions;
   static IMPORT_OPTIONS copyImportOptions;
   IMPORT_OPTIONS *importOptions;
   static GENERIC_OPTIONS copyGenericOptions;
   GENERIC_OPTIONS *genericOptions;
   static PROJECT_OPTIONS copyProjOptions;
   PROJECT_OPTIONS *projOptions;
   static WINDOW_POSITION_OPTIONS *copyWinPosOptions = NULL;
   WINDOW_POSITION_OPTIONS *winPosOptions;
   static PROFILE_OPTIONS copyProfOptions = { "", UNDEFINED_PROFILE, NULL,
                                              NULL, LINE_SEGMENTS, NULL,
                                              0.0, 0.0, 0.0, 0.0,
                                              0.0, 0.0, 0.0, 0.0, NULL };
   PROFILE_OPTIONS *profOptions;
   static PROFILE_POINT *copyProfPoints = NULL;

   if ((save != OPTIONS_FREE_MEMORY) && (!object || !parentId)) return;

   if (save == OPTIONS_SAVE)   /* take a copy of whatever structure is passed through */
   {
      if (parentId == PROJECT_OPTIONS_WINDOW)
      {
         projOptions = (PROJECT_OPTIONS *) object;
         memcpy (&copyProjOptions, projOptions, sizeof (PROJECT_OPTIONS));
         return;
      }

      if (parentId == WINPOS_WINDOW)
      {
         winPosOptions = (WINDOW_POSITION_OPTIONS *) object;
         if (!copyWinPosOptions)
            copyWinPosOptions = (WINDOW_POSITION_OPTIONS *) xvt_mem_zalloc (sizeof(WINDOW_POSITION_OPTIONS));
         if (copyWinPosOptions)
            memcpy (copyWinPosOptions, winPosOptions, sizeof (WINDOW_POSITION_OPTIONS));
         return;
      }
      
      if (parentId == PROFILE_WINDOW)
      {
         profOptions = getProfileOptionsStructure (object);
         if (copyProfOptions.points)   /* Free the points from last one */
            profilePointsFree (copyProfOptions.points);
         memcpy (&copyProfOptions, profOptions, sizeof (PROFILE_OPTIONS));
         copyProfOptions.points = profilePointsDuplicate (profOptions->points);
         
         return;
      }
      
      switch (object->shape)
      {
         case STRATIGRAPHY:
            stratOptions = (STRATIGRAPHY_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            copyStratOptions.numLayers = stratOptions->numLayers;
            if (copyStratOptions.properties)
               xvt_mem_free ((char *) copyStratOptions.properties);
            
            if (copyStratOptions.properties =  (LAYER_PROPERTIES *) xvt_mem_zalloc(stratOptions->numLayers*sizeof(LAYER_PROPERTIES)))
               memcpy ((char *) copyStratOptions.properties, (char *) stratOptions->properties,
                       stratOptions->numLayers*sizeof(LAYER_PROPERTIES));
            break;
         case FOLD:
            foldOptions = (FOLD_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyFoldOptions, foldOptions,
                                      sizeof(FOLD_OPTIONS));
                          /* Take a copy of the points list */
            if (copyProfPoints)   /* Free the points from last one */
               profilePointsFree (copyProfPoints);
            copyProfPoints = profilePointsDuplicate (foldOptions->profile.points);
            break;
         case FAULT:
            faultOptions = (FAULT_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyFaultOptions, faultOptions,
                                       sizeof(FAULT_OPTIONS));
                          /* Take a copy of the points list */
            if (copyProfPoints)   /* Free the points from last one */
               profilePointsFree (copyProfPoints);
            copyProfPoints = profilePointsDuplicate (faultOptions->profile.points);
            break;
         case UNCONFORMITY:
            unconOptions = (UNCONFORMITY_OPTIONS *) object->options;
            stratOptions = (STRATIGRAPHY_OPTIONS *) &(unconOptions->stratigraphyDetails);

            strcpy (names[object->shape], object->text);

            memcpy (&copyUnconOptions, unconOptions,
                                       sizeof(UNCONFORMITY_OPTIONS));
                               /* Copy the information to do with stratigraphys */
            copyStratOptions.numLayers = stratOptions->numLayers;
            if (copyStratOptions.properties)
               xvt_mem_free ((char *) copyStratOptions.properties);
            
            if (copyStratOptions.properties = (LAYER_PROPERTIES *) xvt_mem_zalloc(stratOptions->numLayers*sizeof(LAYER_PROPERTIES)))
               memcpy ((char *) copyStratOptions.properties, (char *) stratOptions->properties,
                       stratOptions->numLayers*sizeof(LAYER_PROPERTIES));
            break;
         case SHEAR_ZONE:
            shearOptions = (SHEAR_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyShearOptions, shearOptions,
                                      sizeof(SHEAR_OPTIONS));

                          /* Take a copy of the points list */
            if (copyProfPoints)   /* Free the points from last one */
               profilePointsFree (copyProfPoints);
            copyProfPoints = profilePointsDuplicate (shearOptions->profile.points);
            break;
         case DYKE:
            dykeOptions = (DYKE_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyDykeOptions, dykeOptions,
                                      sizeof(DYKE_OPTIONS));
            break;
         case PLUG:
            plugOptions = (PLUG_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyPlugOptions, plugOptions,
                                      sizeof(PLUG_OPTIONS));
            break;
         case STRAIN:
            strainOptions = (STRAIN_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyStrainOptions, strainOptions,
                                        sizeof(STRAIN_OPTIONS));
            break;
         case TILT:
            tiltOptions = (TILT_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyTiltOptions, tiltOptions,
                                      sizeof(TILT_OPTIONS));
            break;
         case FOLIATION:
            folOptions = (FOLIATION_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyFolOptions, folOptions,
                                     sizeof(FOLIATION_OPTIONS));
            break;
         case LINEATION:
            linOptions = (LINEATION_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyLinOptions, linOptions,
                                     sizeof(LINEATION_OPTIONS));
            break;
         case IMPORT:
            importOptions = (IMPORT_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyImportOptions, importOptions,
                                     sizeof(IMPORT_OPTIONS));
            break;
         case GENERIC:
            genericOptions = (GENERIC_OPTIONS *) object->options;

            strcpy (names[object->shape], object->text);

            memcpy (&copyGenericOptions, genericOptions,
                                     sizeof(GENERIC_OPTIONS));
            break;
      }
   }
   else if (save == OPTIONS_RESTORE)
         /* copy back into the structure the appropriate parts for the window */
   {     /* restores any options that are changed in the window case or any
         ** children of that window */
      if (parentId == DEFINE_COLOUR_WINDOW)
      {
         return;
      }
      
      if (parentId == PROJECT_OPTIONS_WINDOW)
      {
         projOptions = (PROJECT_OPTIONS *) object;
         memcpy (projOptions, &copyProjOptions, sizeof (PROJECT_OPTIONS));
         return;
      }
      
      if (parentId == WINPOS_WINDOW)
      {
         winPosOptions = (WINDOW_POSITION_OPTIONS *) object;
         memcpy (winPosOptions, copyWinPosOptions, sizeof (WINDOW_POSITION_OPTIONS));
         return;
      }

      if (parentId == PROFILE_WINDOW)
      {
         if (profOptions = getProfileOptionsStructure (object))
         {
            if (profOptions->points)   /* Free the points from last one */
               profilePointsFree (profOptions->points);
            memcpy (profOptions, &copyProfOptions, sizeof (PROFILE_OPTIONS));
                   /* now profOptions points to the point list */
            copyProfOptions.points = NULL;

					/* Change pointer as that was the same pointer as we just freed in profOptions->points above */
				if (object)
				{
					switch (object->shape)
					{
						case FOLD:
							if (copyFoldOptions.profile.points)
								copyFoldOptions.profile.points = profOptions->points;
							break;
						case FAULT:
							if (copyFaultOptions.profile.points)
								copyFaultOptions.profile.points = profOptions->points;
							break;
						case SHEAR_ZONE:
							if (copyShearOptions.profile.points)
								copyShearOptions.profile.points = profOptions->points;
							break;
					}
				} 
         }
         return;
      }

      switch (object->shape)
      {
         case STRATIGRAPHY:
            stratOptions = (STRATIGRAPHY_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);
            
            if ((copyStratOptions.numLayers > 0) && (copyStratOptions.properties))
            {
               if (stratOptions->properties) /* get rid of layers that might be there already */
                  xvt_mem_free ((char *) stratOptions->properties);
               stratOptions->properties = copyStratOptions.properties; /* Copy Strat Layers */
               stratOptions->numLayers = copyStratOptions.numLayers;
               copyStratOptions.properties = NULL;  /* No longer need the pointer */
            }
            break;
         case FOLD:
            foldOptions = (FOLD_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (foldOptions, &copyFoldOptions, sizeof(FOLD_OPTIONS));

                          /* Restore profile drag points */
            if (foldOptions->profile.points)   /* Free the points that are there */
               profilePointsFree (foldOptions->profile.points);
            foldOptions->profile.points = copyProfPoints;
            copyProfPoints = NULL;
            break;
         case FAULT:
            faultOptions = (FAULT_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (faultOptions, &copyFaultOptions, sizeof(FAULT_OPTIONS));

                          /* Restore profile drag points */
            if (faultOptions->profile.points)   /* Free the points that are there */
               profilePointsFree (faultOptions->profile.points);
            faultOptions->profile.points = copyProfPoints;
            copyProfPoints = NULL;
            break;
         case UNCONFORMITY:
            unconOptions = (UNCONFORMITY_OPTIONS *) object->options;
            stratOptions = (STRATIGRAPHY_OPTIONS *) &(unconOptions->stratigraphyDetails);

            strcpy (object->text, names[object->shape]);

            memcpy (unconOptions, &copyUnconOptions,
                                  sizeof(UNCONFORMITY_OPTIONS));
                               /* Copy the information to do with stratigraphys */
            if ((copyStratOptions.numLayers > 0) && (copyStratOptions.properties))
            {
               if (stratOptions->properties) /* get rid of layers that might be there already */
                  xvt_mem_free ((char *) stratOptions->properties);
               stratOptions->properties = copyStratOptions.properties; /* Copy Strat Layers */
               stratOptions->numLayers = copyStratOptions.numLayers;
               copyStratOptions.properties = NULL;  /* No longer need the pointer */
            }
            break;
         case SHEAR_ZONE:
            shearOptions = (SHEAR_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (shearOptions, &copyShearOptions, sizeof(SHEAR_OPTIONS));

                          /* Restore profile drag points */
            if (shearOptions->profile.points)   /* Free the points that are there */
               profilePointsFree (shearOptions->profile.points);
            shearOptions->profile.points = copyProfPoints;
            copyProfPoints = NULL;
            break;
         case DYKE:
            dykeOptions = (DYKE_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (dykeOptions, &copyDykeOptions, sizeof(DYKE_OPTIONS));
            break;
         case PLUG:
            plugOptions = (PLUG_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (plugOptions, &copyPlugOptions, sizeof(PLUG_OPTIONS));
            break;
         case STRAIN:
            strainOptions = (STRAIN_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (strainOptions, &copyStrainOptions, sizeof(STRAIN_OPTIONS));
            break;
         case TILT:
            tiltOptions = (TILT_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (tiltOptions, &copyTiltOptions, sizeof(TILT_OPTIONS));

            break;
         case FOLIATION:
            folOptions = (FOLIATION_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (folOptions, &copyFolOptions, sizeof(FOLIATION_OPTIONS));
            break;
         case LINEATION:
            linOptions = (LINEATION_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (linOptions, &copyLinOptions, sizeof(LINEATION_OPTIONS));
            break;
         case IMPORT:
            importOptions = (IMPORT_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (importOptions, &copyImportOptions, sizeof(IMPORT_OPTIONS));
            break;
         case GENERIC:
            genericOptions = (GENERIC_OPTIONS *) object->options;

            strcpy (object->text, names[object->shape]);

            memcpy (genericOptions, &copyGenericOptions, sizeof(GENERIC_OPTIONS));
            break;
      }
   }
   else if (save == OPTIONS_FREE_MEMORY)
   {
      if (copyWinPosOptions)
      {
         xvt_mem_free ((char *) copyWinPosOptions);
         copyWinPosOptions = NULL;
      }

      if (copyProfOptions.points)   /* Free the points from last one */
      {
         profilePointsFree (copyProfOptions.points);
         copyProfOptions.points = NULL;
      }
      if (copyStratOptions.properties)
      {
         xvt_mem_free ((char *) copyStratOptions.properties);
         copyStratOptions.properties = NULL;
      }
   }
}

/* ======================================================================
FUNCTION        updateProfileOptions
DESCRIPTION
        make sure the correct options are enabled
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
updateProfileOptions (WINDOW parentWin)
#else
updateProfileOptions (parentWin)
WINDOW parentWin;
#endif
{
   OBJECT *object = (OBJECT *) xvt_vobj_get_data (parentWin);
   PROFILE_OPTIONS *options, *firstOptions;
   
   if (!(firstOptions = getProfileOptionsStructure (object)))
      return;

   options = (PROFILE_OPTIONS *) getCurrentProfileOptions (parentWin, firstOptions);
                   
                    /* set the join type for points */
   if (xvt_ctl_is_checked (xvt_win_get_ctl (parentWin, PROFILE_LINE_JOIN)))
      options->joinType = LINE_SEGMENTS;
   else if (xvt_ctl_is_checked (xvt_win_get_ctl (parentWin, PROFILE_CURVE_JOIN)))
      options->joinType = CURVE_SEGMENTS;
   else if (xvt_ctl_is_checked (xvt_win_get_ctl (parentWin, PROFILE_SQUARE_JOIN)))
      options->joinType = SQUARE_SEGMENTS;

        /* use lastActiveWindow to refer to drawing canvas as I had trouble geting a handle to it */
   xvt_dwin_invalidate_rect (lastActiveWindow, (RCT *) NULL);
   xvt_dwin_update(lastActiveWindow);
}

/* ======================================================================
FUNCTION        updatePropertiesOptions
DESCRIPTION
        make sure the correct options are enabled
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
updatePropertiesOptions (WINDOW parentWin)
#else
updatePropertiesOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win;
   int selIndex = -1, prop;
   LAYER_PROPERTIES *databaseLayer = NULL;
   LAYER_PROPERTIES *currentLayer = NULL;
   char rockTitle[UNIT_NAME_LENGTH];
	double unitScale;
   
    /* update window to relect selection from rock prop database */
   win = xvt_win_get_ctl (parentWin, ROCK_NAME);
   xvt_vobj_get_title (win, rockTitle, UNIT_NAME_LENGTH);
          /* check to see if it is one of the defined rocks */
   for (prop = 0; prop < rockDatabase.numProps; prop++)
   {
      if (strcmp (rockTitle, rockDatabase.database[prop].unitName) == 0)
      {
         selIndex = prop;
         break;
      }
   }
   
   if (selIndex != -1)
   {
		if (projectOptions.susceptibilityUnits == SI)
			unitScale = CGS_TO_SI;
		else  /* CGS */
			unitScale = 1.00000;

		databaseLayer = &(rockDatabase.database[selIndex]);
      
      /* height is not dependent of rock type
      ** updateNumericTextField (parentWin, PROPERTIES_WINDOW_EDIT_3,
                              databaseLayer->height);
      */
      updateFloatTextField (parentWin, ROCK_DENSITY,
                              databaseLayer->density, 3);

      win = xvt_win_get_ctl (parentWin, ROCK_ANISOTROPY);
      xvt_ctl_set_checked(win, (short) databaseLayer->anisotropicField);

      updateFloatTextField (parentWin, ROCK_X,
                              databaseLayer->sus_X*unitScale, 4);
      updateFloatTextField (parentWin, ROCK_Y,
                              databaseLayer->sus_Y*unitScale, 4);
      updateFloatTextField (parentWin, ROCK_Z,
                              databaseLayer->sus_Z*unitScale, 4);
      updateFloatTextField (parentWin, ROCK_DIP,
                              databaseLayer->sus_dip, 1);
      updateFloatTextField (parentWin, ROCK_DDIR,
                              databaseLayer->sus_dipDirection, 1);
      updateFloatTextField (parentWin, ROCK_PITCH,
                              databaseLayer->sus_pitch, 1);

      win = xvt_win_get_ctl (parentWin, ROCK_REMANENCE);
      xvt_ctl_set_checked(win, (short) databaseLayer->remanentMagnetization);

      updateFloatTextField (parentWin, ROCK_INCLINATION,
                              databaseLayer->inclination, 1);
      updateFloatTextField (parentWin, ROCK_DECLINATION,
                              databaseLayer->angleWithNorth, 1);
      updateFloatTextField (parentWin, ROCK_INTENSITY,
                              databaseLayer->strength*unitScale, 4);
                              
      if (currentLayer = getLayerPropertiesStructure ((OBJECT *) xvt_vobj_get_data(parentWin)))
         memcpy (&(currentLayer->color), &(databaseLayer->color),
                                           sizeof (NODDY_COLOUR));


      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_Y),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_Y_LABEL),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_Z),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_Z_LABEL),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DIP),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DIP_LABEL),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DDIR),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DDIR_LABEL),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_PITCH),
                    (short) databaseLayer->anisotropicField);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_PITCH_LABEL),
                    (short) databaseLayer->anisotropicField);

      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_INCLINATION),
                    (short) databaseLayer->remanentMagnetization);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_INCLINATION_LABEL),
                    (short) databaseLayer->remanentMagnetization);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DECLINATION),
                    (short) databaseLayer->remanentMagnetization);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_DECLINATION_LABEL),
                    (short) databaseLayer->remanentMagnetization);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_INTENSITY),
                    (short) databaseLayer->remanentMagnetization);
      xvt_vobj_set_enabled(xvt_win_get_ctl (parentWin,ROCK_INTENSITY_LABEL),
                    (short) databaseLayer->remanentMagnetization);
   }
}

/* ======================================================================
FUNCTION  getFloatTextFieldValue
DESCRIPTION
   return the value of a text field as a double
INPUT

OUTPUT

RETURNED
====================================================================== */
double
#if XVT_CC_PROTO
getFloatTextFieldValue (WINDOW parentWin, int id)
#else
getFloatTextFieldValue (parentWin, id)
WINDOW parentWin;
int id;
#endif
{
   register double value;
   char textPos[30];

   xvt_vobj_get_title (xvt_win_get_ctl (parentWin, id), textPos, 30);
   value = strtod (textPos, NULL);

   return (value);
}
/* ======================================================================
FUNCTION  getIntegerTextFieldValue
DESCRIPTION
   return the value of a text field as a integer
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
getIntegerTextFieldValue (WINDOW parentWin, int id)
#else
getIntegerTextFieldValue (parentWin, id)
WINDOW parentWin;
int id;
#endif
{
   int value;

   value = (int) floor (getFloatTextFieldValue (parentWin, id));

   return (value);
}

/* ======================================================================
FUNCTION  updateFloatTextField
DESCRIPTION
   set the value of the text field to value (as a double)
INPUT

OUTPUT

RETURNED
====================================================================== */
WINDOW
#if XVT_CC_PROTO
updateFloatTextField (WINDOW parentWin, int id, double value, int precision)
#else
updateFloatTextField (parentWin, id, value, precision)
WINDOW parentWin;
int id;
double value;
int precision;
#endif
{
   WINDOW textWin;
   char textPos[30];
   int iValue;

   iValue = (int) floor (value);
   if (((value - (double) iValue) < TOLERANCE) && (precision < 4))
      precision = 0;

   textWin = xvt_win_get_ctl (parentWin, id);

   if (precision == 0)
      sprintf (textPos, "%.0f", value);
   else if (precision == 1)
      sprintf (textPos, "%.1f", value);
   else if (precision == 2)
      sprintf (textPos, "%.2f", value);
   else if (precision == 3)
      sprintf (textPos, "%.3f", value);
   else
      sprintf (textPos, "%.3e", value);

   xvt_vobj_set_title (textWin, textPos);

   return (textWin);
}
/* ======================================================================
FUNCTION  updateNumericTextField
DESCRIPTION
   set the value of the text field to value
INPUT

OUTPUT

RETURNED
====================================================================== */
WINDOW
#if XVT_CC_PROTO
updateNumericTextField (WINDOW parentWin, int id, int value)
#else
updateNumericTextField (parentWin, id, value)
WINDOW parentWin;
int id;
int value;
#endif
{
   WINDOW textWin;
   char textPos[30];

   textWin = xvt_win_get_ctl (parentWin, id);
   sprintf (textPos, "%d", value);
   xvt_vobj_set_title (textWin, textPos);

   return (textWin);
}
/* ======================================================================
FUNCTION  updateScrollField
DESCRIPTION
     set the value of the scrollbar to value
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateScrollField (WINDOW parentWin, int id, int value)
#else
updateScrollField (parentWin, id, value)
WINDOW parentWin;
int id;
int value;
#endif
{
   WINDOW scrollWin;

   scrollWin = xvt_win_get_ctl (parentWin, id);
   return (setScrollPosition (scrollWin, HVSCROLL, (int) value));
}

/* ======================================================================
FUNCTION  updateScrollAndText
DESCRIPTION
     check the scrollbar and text field for consistency in showing
the correct value. The value is taken from the text field.
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateScrollAndText (WINDOW parentWin, int scrollId, int textId,
                     int restrictedRange)
#else
updateScrollAndText (parentWin, scrollId, textId, restrictedRange)
WINDOW parentWin;
int scrollId, textId;
int restrictedRange;
#endif
{
   WINDOW scrollWin, textWin;
   int minValue, maxValue;
   char textPos[30];
   double dValue, newDValue;
   int iValue, newIValue;

   scrollWin = xvt_win_get_ctl (parentWin, scrollId);
   textWin = xvt_win_get_ctl (parentWin, textId);

   xvt_vobj_get_title (textWin, textPos, 30);
   dValue = strtod (textPos, NULL);
   iValue = (int) floor (dValue);

   newDValue = dValue;
   newIValue = iValue;

   if (restrictedRange)
   {
/* All restricted ranges are angles now so use -360 -> +360 as
** restriction reguardless of what the range of the scrollbars are.
      xvt_sbar_get_range (scrollWin, HVSCROLL, &minValue, &maxValue);
*/
      minValue = -360;
      maxValue = 360;
      if (dValue < minValue)
      {
         newDValue = (double) minValue;
         newIValue = (int) minValue;
      }
      if (dValue > maxValue)
      {
         newDValue = (double) maxValue;
         newIValue = (int) maxValue;
      }
   }

   if (fabs(dValue - newDValue) > TOLERANCE)
   {
               /* if nothing in first decimal place then make it an int */
      if ((newDValue - (double) newIValue) < TOLERANCE)
         sprintf (textPos, "%d", newIValue);
      else
         sprintf (textPos, "%.1f", newDValue);

      xvt_vobj_set_title (textWin, textPos);
   }

   return (setScrollPosition (scrollWin, HVSCROLL, (int) iValue));
}

/* ======================================================================
FUNCTION        setScrollRange
DESCRIPTION
        set the range of a scroll bar
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
setScrollRange (WINDOW parentWin, int scrollId, int minValue, int maxValue)
#else
setScrollRange (parentWin, scrollId, minValue, maxValue)
WINDOW parentWin;
int scrollId;
int minValue, maxValue;
#endif
{
   WINDOW scrollWin;

   scrollWin = xvt_win_get_ctl (parentWin, scrollId);
   xvt_sbar_set_range (scrollWin, HVSCROLL, minValue, maxValue);
}

/* ======================================================================
FUNCTION        setScrollPosition
DESCRIPTION
        set the position of a scroll bar
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
setScrollPosition (WINDOW scrollWin, SCROLL_TYPE type, int position)
#else
setScrollPosition (scrollWin, type, position)
WINDOW scrollWin;
SCROLL_TYPE type;
int position;
#endif
{
   int minValue, maxValue;

   xvt_sbar_get_range (scrollWin, type, &minValue, &maxValue);
   if (position < minValue) position = minValue;
   if (position > maxValue) position = maxValue;
   xvt_sbar_set_pos (scrollWin, type, position);

   return (position);
}

/* ======================================================================
FUNCTION        setObjectName
DESCRIPTION
        Function to set the name of the current object
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
setObjectName (OBJECT *p, char *name)
#else
setObjectName (p, name)
register OBJECT *p;
char *name;
#endif
{

   if (!p) return;

           /* assign up to 100 characters of next then just
           ** cut off anything else */
   strncpy (p->text, name, OBJECT_TEXT_LENGTH);
   p->text[99] = '\0';
}

/* ======================================================================
FUNCTION        getObjectName
DESCRIPTION
        Function to get the name of the current object
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
getObjectName (OBJECT *p, char *name)
#else
getObjectName (p, name)
register OBJECT *p;
char *name;
#endif
{
   if (p && p->text)
      strcpy (name, p->text);
   else
      strcpy (name, "");
}

/* ======================================================================
FUNCTION        readRockPropDatabase
DESCRIPTION
          load the rock prop database and initialise the global
          memory associated with it

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
readRockPropDatabase ()
#else
readRockPropDatabase ()
#endif
{
   double version;
   char databaseFile[100];
   FILE *fi;
   int numProps, prop;
   LAYER_PROPERTIES *database = NULL;
   
   strcpy (databaseFile, ROCK_DATABASE_FILE_NAME);
   
   xvt_fsys_set_dir_startup ();
   if (!(fi = fopen (databaseFile, "r")))
      return (FALSE);
      
   loadit(fi,"%lf",(void *) &version);
   loadit(fi,"%d",(void *) &numProps);

   if (numProps > 0)
	{
		database = (LAYER_PROPERTIES *) xvt_mem_zalloc (numProps*sizeof(LAYER_PROPERTIES));
		for (prop = 0; prop < numProps; prop++)
		{
			loadProperties (fi, version, &(database[prop]));
		}
	}
	else
		database = NULL;
   fclose (fi);
   
   if (rockDatabase.database)
      freeRockPropDatabase ();
   
   rockDatabase.database = database;
   rockDatabase.numProps = numProps;

   return (TRUE);
}
/* ======================================================================
FUNCTION        writeRockPropDatabase
DESCRIPTION
          overwrite the rock prop database on the 
          disk with the one passed to this function

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
writeRockPropDatabase (ROCK_DATABASE *rockDatabase)
#else
writeRockPropDatabase (rockDatabase)
ROCK_DATABASE *rockDatabase;
#endif
{
   char databaseFile[100];
   FILE *fo;
   int prop;
   LAYER_PROPERTIES *database = NULL;
   
   strcpy (databaseFile, ROCK_DATABASE_FILE_NAME);
   
   xvt_fsys_set_dir_startup ();
   if (!(fo = fopen (databaseFile, "w")))
   {
      xvt_dm_post_error ("Error, Could open Rock Database for Writing.");
      return (FALSE);
   }
      
   fprintf(fo,"Version = %lf\n", (double) VERSION_NUMBER);
   fprintf(fo,"Number of Rocks = %d\n", rockDatabase->numProps);

   for (prop = 0; prop < rockDatabase->numProps; prop++)
   {
      fprintf (fo, "\n"); /* seperate each rock with blank line */
      reportProperties (fo, &(rockDatabase->database[prop]));
   }
      
   fclose (fo);
   
   return (TRUE);
}

int
#if XVT_CC_PROTO
freeRockPropDatabase ()
#else
freeRockPropDatabase ()
#endif
{
   if (rockDatabase.database)
      xvt_mem_free ((char *) rockDatabase.database);

   rockDatabase.database = (LAYER_PROPERTIES *) NULL;
   rockDatabase.numProps;
      
   return (TRUE);
}

/* ======================================================================
FUNCTION        makeEditOptionsHistory
DESCRIPTION
   Temporarly add a stop event after the current event so we can 
   perform calculations up to that point but no more.
   This is done so we can perform previews of the data so far.

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
makeEditOptionsHistory (OBJECT *object)
#else
makeEditOptionsHistory (object)
OBJECT *object;
#endif
{
   OBJECT *tempStop;
   
   if (!object)
      return (FALSE);
      
   if (!(tempStop = (OBJECT *) xvt_mem_zalloc (sizeof(OBJECT))))
      return (FALSE);
      
   tempStop->drawEvent = FALSE;
   tempStop->selected = FALSE;
   tempStop->shape = STOP;
   tempStop->row = object->row+1;
   tempStop->column = object->column;
                      /* insert in history */
   tempStop->next = object->next;
   object->next = tempStop;
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        removeEditOptionsHistory
DESCRIPTION
   Restore the history to the state it was in before the stop was
   inserted to halt calculations.
   
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
removeEditOptionsHistory (OBJECT *object)
#else
removeEditOptionsHistory (object)
OBJECT *object;
#endif
{
   OBJECT *tempStop;
   
   if (!object || !object->next)
      return (FALSE);
      
   tempStop = object->next;
                     /* make sure we have what we think we have */
   if ((tempStop->drawEvent == FALSE) && (tempStop->shape == STOP))
   {                  /* remove from history */
      object->next = tempStop->next;
   
      xvt_mem_free ((char *) tempStop);
   }

   return (TRUE);
}


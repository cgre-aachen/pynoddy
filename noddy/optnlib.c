#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include "scales.h"

#define DEBUG(X)    
#define TOLERANCE  0.00001

                       /* External dependencies */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;
extern BLOCK_VIEW_OPTIONS *blockViewOptions;

extern ROCK_DATABASE rockDatabase;
extern char errorMessage[ERROR_MESSAGE_LENGTH];
extern int errorSet;


/*
#define GEOLOGY_DISPLAY_INDEX      0
#define GEOPHYSICS_CALC_INDEX      4
#define GEOPHYSICS_SURVEY_INDEX    9
#define GEOPHYSICS_DISPLAY_INDEX   14
#define PROJECT_INDEX              16
#define WINDOW_POSITION_INDEX      19
#define BLOCK_VIEW_INDEX           20
#define MOVIE_INDEX                26
#define BOREHOLE_SECTION_INDEX     27
#define VOLUME_SURFACE_INDEX       29
*/
/*
#define PROJECT_INDEX              0
#define BLOCK_VIEW_INDEX           3
#define MOVIE_INDEX                9
#define GEOLOGY_DISPLAY_INDEX      10
#define BOREHOLE_SECTION_INDEX     14
#define VOLUME_SURFACE_INDEX       16
#define GEOPHYSICS_CALC_INDEX      18
#define GEOPHYSICS_SURVEY_INDEX    23
#define GEOPHYSICS_DISPLAY_INDEX   28
#define WINDOW_POSITION_INDEX      30
*/
#define PROJECT_INDEX              0
#define BLOCK_VIEW_INDEX           4
#define MOVIE_INDEX                10
#define GEOLOGY_DISPLAY_INDEX      11
#define BOREHOLE_SECTION_INDEX     15
#define VOLUME_SURFACE_INDEX       17
#define GEOPHYSICS_CALC_INDEX      19
#define GEOPHYSICS_SURVEY_INDEX    24
#define GEOPHYSICS_DISPLAY_INDEX   29
#define WINDOW_POSITION_INDEX      31

#define MAX_OPTION_GROUPS   40

static TAB optionTabs[MAX_OPTION_GROUPS];

static GEOLOGY_OPTIONS *copyOfGeologyOptions = NULL;
static GEOPHYSICS_OPTIONS *copyOfGeophysicsOptions = NULL;
static PROJECT_OPTIONS *copyOfProjectOptions = NULL;
static THREED_VIEW_OPTIONS *copyOfThreedViewOptions = NULL;
static WINDOW_POSITION_OPTIONS *copyOfWinPositionOptions = NULL;
static BLOCK_VIEW_OPTIONS *copyOfBlockViewOptions = NULL;


                                /* Functions external to this file */
#if XVT_CC_PROTO
#else
#endif
                                /* Global Functions in this file */
#if XVT_CC_PROTO
int createOptions (WINDOW, OPTION_TYPE);
int takeCopyOfOptions (OPTION_TYPE);
int restoreCopyOfOptions (OPTION_TYPE);
int freeCopyOfOptions (OPTION_TYPE);
int optionsMouseUp (WINDOW, struct s_mouse *);
int loadOptions (WINDOW, OPTION_TYPE);
int saveOptions (WINDOW, OPTION_TYPE, int);
int updateOptions (WINDOW, OPTION_TYPE);
int selectOptionGroup (WINDOW, OPTION_TYPE, int);
int displayOptionsHelp (OPTION_TYPE);

static int loadGeologyDisplayOptions (OPTION_TYPE);
static int saveGeologyDisplayOptions (OPTION_TYPE);
static int updateGeologyDisplayOptions (OPTION_TYPE);
static int loadGeophysicsCalcOptions (OPTION_TYPE);
static int saveGeophysicsCalcOptions (OPTION_TYPE);
static int updateGeophysicsCalcOptions (OPTION_TYPE);
static int loadGeophysicsSurveyOptions (OPTION_TYPE);
static int saveGeophysicsSurveyOptions (OPTION_TYPE);
static int updateGeophysicsSurveyOptions (OPTION_TYPE);
static int loadGeophysicsDisplayOptions (OPTION_TYPE);
static int saveGeophysicsDisplayOptions (OPTION_TYPE);
static int updateGeophysicsDisplayOptions (OPTION_TYPE);
static int loadGlobalProjectOptions (OPTION_TYPE);
static int saveGlobalProjectOptions (OPTION_TYPE);
static int updateGlobalProjectOptions (OPTION_TYPE);
static int loadWindowPosOptions (OPTION_TYPE);
static int saveWindowPosOptions (OPTION_TYPE);
static int updateWindowPosOptions (OPTION_TYPE);
int loadBlockViewOptions (OPTION_TYPE);
int saveBlockViewOptions (OPTION_TYPE);
static int updateBlockViewOptions (OPTION_TYPE);
static int loadMovieOptions (OPTION_TYPE);
static int saveMovieOptions (OPTION_TYPE);
static int updateMovieOptions (OPTION_TYPE);
static int loadSectionBoreholeOptions (OPTION_TYPE);
static int saveSectionBoreholeOptions (OPTION_TYPE);
static int updateSectionBoreholeOptions (OPTION_TYPE);
static int loadVolumeSurfaceOptions (OPTION_TYPE);
static int saveVolumeSurfaceOptions (OPTION_TYPE);
static int updateVolumeSurfaceOptions (OPTION_TYPE);
#else
int createOptions ();
int takeCopyOfOptions ();
int restoreCopyOfOptions ();
int freeCopyOfOptions ();
int optionsMouseUp ();
int loadOptions ();
int saveOptions ();
int updateOptions ();
int selectOptionGroup ();
int displayOptionsHelp ();

static int loadGeologyDisplayOptions ();
static int saveGeologyDisplayOptions ();
static int updateGeologyDisplayOptions ();
static int loadGeophysicsCalcOptions ();
static int saveGeophysicsCalcOptions ();
static int updateGeophysicsCalcOptions ();
static int loadGeophysicsSurveyOptions ();
static int saveGeophysicsSurveyOptions ();
static int updateGeophysicsSurveyOptions ();
static int loadGeophysicsDisplayOptions ();
static int saveGeophysicsDisplayOptions ();
static int updateGeophysicsDisplayOptions ();
static int loadGlobalProjectOptions ();
static int saveGlobalProjectOptions ();
static int updateGlobalProjectOptions ();
static int loadWindowPosOptions ();
static int saveWindowPosOptions ();
static int updateWindowPosOptions ();
int loadBlockViewOptions ();
int saveBlockViewOptions ();
static int updateBlockViewOptions ();
static int loadMovieOptions ();
static int saveMovieOptions ();
static int updateMovieOptions ();
static int loadSectionBoreholeOptions ();
static int saveSectionBoreholeOptions ();
static int updateSectionBoreholeOptions ();
static int loadVolumeSurfaceOptions ();
static int saveVolumeSurfaceOptions ();
static int updateVolumeSurfaceOptions ();
#endif

static int
#if XVT_CC_PROTO
setOptionsTab (WINDOW parentWin, int index, int tab,
               char *label, int windowId, EVENT_HANDLER callbackFunction,
               RCT *region, int iconId, long data,
               int xPos, int yPos, RCT *bound)
#else
setOptionsTab (parentWin, index, tab, label, windowId, callbackFunction,
               region, iconId, data, xPos, yPos, bound)
WINDOW parentWin;
int index, tab;
char *label;
int windowId;
EVENT_HANDLER callbackFunction;
RCT *region;
int iconId;
long data;
int xPos, yPos;
RCT *bound;
#endif
{
   strcpy (optionTabs[index].label, label);
   optionTabs[index].group = tab;
   optionTabs[index].icon = iconId;
   if (!region)
      memset ((char *) &(optionTabs[index].region), 0, sizeof(RCT));
   else
      memcpy ((char *) &(optionTabs[index].region), region, sizeof(RCT));
   optionTabs[index].win = createPositionedWindow(windowId, parentWin, xPos, yPos,
                                    EM_ALL, callbackFunction, data, bound);

   return (TRUE);
}

/* ======================================================================
FUNCTION        createOptions
DESCRIPTION
     create all the structures etc to allow editing of this events
     options in the event editing window

INPUT 
     WINDOW win;         the window of the event
     OPTION_TYPE type;   identifies the event being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
createOptions (WINDOW win, OPTION_TYPE type)
#else
createOptions (win, type)
WINDOW win;
OPTION_TYPE type;
#endif
{
   WINDOW listWin;
   int group = 0, numSections, numTabs = 0, index;
   int posX, posY, i, numIcon = 0;
   RCT posRect, iconRects[7];
   int groupXPos, groupYPos, controlXPos, controlYPos;
   char tabLabel[TAB_LABEL_LENGTH];
   long data = 0L;
   
   data = (long) type;

                        /* Zero the memory for the tabs */
   memset (optionTabs, 0, sizeof(TAB)*MAX_OPTION_GROUPS);

/*
   tieWin = xvt_win_get_ctl(win, EVENT_OPTION_SELECTION);
   xvt_vobj_get_outer_rect(tieWin, &posRect);
   groupYPos = posRect.top + 20; 
   groupXPos = posRect.left;
*/

#if XVTWS == MTFWS
   groupYPos = 40;
   groupXPos = 300;  /* 370 */
#endif
#if XVTWS == MACWS
   groupYPos = 30;
   groupXPos = 300;
#endif
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   groupYPos = 16;
   groupXPos = 150;
#endif
#if XVTWS == XOLWS
   groupYPos = 40;
   groupXPos = 300;
#endif
   groupYPos = 0;

   xvt_vobj_get_client_rect(win, &posRect);
   posX = posRect.right - ICON_SIZE - 4; posY = 30;
   for (i = 0; i < 7; i++)
   {
      xvt_rect_set (&(iconRects[i]), (short) posX, (short) posY,
                                     (short) (posX+ICON_SIZE), (short) (posY+ICON_SIZE));
      posY += ICON_SIZE + 2;
   }
   
/*   groupYPos = posY; */
                                               /* Keep current options so we can restore changes */
   
   takeCopyOfOptions (type);
                                               /* Create a preview */
   createPreviewWindow (win, xvt_win_get_ctl (win, OPTIONS_PREVIEW));
   
                                     /* ********* Project ********* */
   strcpy(tabLabel, "Project");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             LOCAL_REMOTE_WINDOW, LOCAL_REMOTE_WINDOW_eh, (RCT *) NULL, 0,
             (long) PROJECT_OP, controlXPos, controlYPos, &posRect);

   controlXPos += xvt_rect_get_width(&posRect);  /* beside Control */
   setOptionsTab(win, group++, numTabs, tabLabel,
             UNIT_WINDOW, UNIT_WINDOW_eh, (RCT *) NULL, 0,
             (long) PROJECT_OP, controlXPos, controlYPos, &posRect);

   controlXPos = groupXPos;
   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             PRINT_SCALE_WINDOW, PRINT_SCALE_WINDOW_eh, (RCT *) NULL, 0,
             (long) PROJECT_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             HELP_OPTION_WINDOW, HELP_OPTION_WINDOW_eh, (RCT *) NULL, 0,
             (long) PROJECT_OP, controlXPos, controlYPos, &posRect);
	numTabs++;

                                     /* ********* Block View ********* */
   strcpy(tabLabel, "Block");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             VIEW_NAME_WINDOW, VIEW_NAME_WINDOW_eh, (RCT *) NULL, 0,
             (long) BLOCK_VIEW_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             ORIGIN_WINDOW, ORIGIN_WINDOW_eh, (RCT *) NULL, 0,
             (long) BLOCK_VIEW_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             DIMENSION_WINDOW, DIMENSION_WINDOW_eh, (RCT *) NULL, 0,
             (long) BLOCK_VIEW_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             CUBE_SIZE_WINDOW, CUBE_SIZE_WINDOW_eh, (RCT *) NULL, 0,
             (long) BLOCK_VIEW_OP, controlXPos, controlYPos, &posRect);

   controlXPos += xvt_rect_get_width(&posRect);  /* Beside Control */
   setOptionsTab(win, group++, numTabs, tabLabel,
             FALSE_ORIGIN_WINDOW, FALSE_ORIGIN_WINDOW_eh, (RCT *) NULL, 0,
             (long) BLOCK_VIEW_OP, controlXPos, controlYPos, &posRect);

   controlXPos = groupXPos;
   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             USE_TOPOGRAPHY_WINDOW, USE_TOPOGRAPHY_WINDOW_eh, (RCT *) NULL, 0,
             (long) BLOCK_VIEW_OP, controlXPos, controlYPos, &posRect);
   numTabs++;
   
                                     /* ********* Movie ********* */
   strcpy(tabLabel, "Movie");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             MOVIE_WINDOW, MOVIE_WINDOW_eh, (RCT *) NULL, 0,
             (long) MOVIE_OP, controlXPos, controlYPos, &posRect);
   numTabs++;
   
                                     /* ********* Geology Display ********* */
   strcpy(tabLabel, "Geology Display");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             THREED_VIEW_WINDOW, THREED_VIEW_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOLOGY_DISPLAY_OP, controlXPos, controlYPos, &posRect);
             
   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             CHAIR_WINDOW, CHAIR_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOLOGY_DISPLAY_OP, controlXPos, controlYPos, &posRect);
             
   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             NEWWIN_WINDOW, NEWWIN_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOLOGY_DISPLAY_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             BACKGROUND_WINDOW, BACKGROUND_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOLOGY_DISPLAY_OP, controlXPos, controlYPos, &posRect);
   numTabs++;

                                     /* ********* Section/Borehole ********* */
   strcpy(tabLabel, "Section/Borehole");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             SECTION_WINDOW, SECTION_WINDOW_eh, (RCT *) NULL, 0,
             (long) BOREHOLE_SECTION_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             BOREHOLE_WINDOW, BOREHOLE_WINDOW_eh, (RCT *) NULL, 0,
             (long) BOREHOLE_SECTION_OP, controlXPos, controlYPos, &posRect);
   numTabs++;

                                     /* ********* Volume/Surface ********* */
   strcpy(tabLabel, "Volume/Surface");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             VOLUME_WINDOW, VOLUME_WINDOW_eh, (RCT *) NULL, 0,
             (long) VOLUME_SURFACE_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             SURFACE_SELECTION_WINDOW, SURFACE_SELECTION_WINDOW_eh, (RCT *) NULL, 0,
             (long) VOLUME_SURFACE_OP, controlXPos, controlYPos, &posRect);
   numTabs++;


                                     /* ********* Geophysics Calculation ********* */
   strcpy(tabLabel, "Geophysics Calculation");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             GENERATE_WINDOW, GENERATE_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_CALC_OP, controlXPos, controlYPos, &posRect);
             
   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             CALC_TYPE_WINDOW, CALC_TYPE_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_CALC_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             VARIABLE_CUBE_WINDOW, VARIABLE_CUBE_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_CALC_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             RANGE_WINDOW, RANGE_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_CALC_OP, controlXPos, controlYPos, &posRect);

   setOptionsTab(win, group++, numTabs, tabLabel,
             PADDING_WINDOW, PADDING_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_CALC_OP, controlXPos, controlYPos, &posRect);

   numTabs++;

                                     /* ********* Geophysics Survey ********* */
   strcpy(tabLabel, "Geophysics Survey");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             FIELD_WINDOW, FIELD_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_SURVEY_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             ALTITUDE_WINDOW, ALTITUDE_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_SURVEY_OP, controlXPos, controlYPos, &posRect);

   controlXPos += xvt_rect_get_width(&posRect);  /* Beside control */
   setOptionsTab(win, group++, numTabs, tabLabel,
             DEFORM_FIELD_WINDOW, DEFORM_FIELD_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_SURVEY_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);  /* Beside and Below control */
   setOptionsTab(win, group++, numTabs, tabLabel,
             SUS_UNITS_WINDOW, SUS_UNITS_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_SURVEY_OP, controlXPos, controlYPos, &posRect);

   controlXPos = groupXPos;
	controlYPos += xvt_rect_get_height(&posRect);  /* Beside and Below control */
   setOptionsTab(win, group++, numTabs, tabLabel,
             DRAPED_SURVEY_WINDOW, DRAPED_SURVEY_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_SURVEY_OP, controlXPos, controlYPos, &posRect);
   numTabs++;

                                     /* ********* Geophysics Display ********* */
   strcpy(tabLabel, "Geophysics Display");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             GEOPHYSICS_SCALE_WINDOW, GEOPHYSICS_SCALE_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_DISPLAY_OP, controlXPos, controlYPos, &posRect);

   controlYPos += xvt_rect_get_height(&posRect);
   setOptionsTab(win, group++, numTabs, tabLabel,
             GEOPHYSICS_IDISPLAY_WINDOW, GEOPHYSICS_IDISPLAY_WINDOW_eh, (RCT *) NULL, 0,
             (long) GEOPHYSICS_DISPLAY_OP, controlXPos, controlYPos, &posRect);
   numTabs++;

                                     /* ********* Windows ********* */
   strcpy(tabLabel, "Windows");
   controlXPos = groupXPos; controlYPos = groupYPos;
   
   setOptionsTab(win, group++, numTabs, tabLabel,
             WINPOSITION_WINDOW, WINPOSITION_WINDOW_eh, (RCT *) NULL, 0,
             (long) WINDOW_POSITION_OP, controlXPos, controlYPos, &posRect);
   numTabs++;

                                      /* ******** Setup everything else ********* */
   numSections = group;
                        /* Add the tabs to a selection list */
   listWin = xvt_win_get_ctl (win, OPTIONS_SELECTION);
   xvt_list_suspend (listWin);
   for (group = 0, index = 0; group < numSections; group++)
   {
      if ((group < 1) || (optionTabs[group-1].group != optionTabs[group].group))
      {
         xvt_list_add (listWin, index, optionTabs[group].label);
         index++;
      }
   }
   xvt_list_resume (listWin);

                 /* Fill the windows with current values */
   loadOptions (win, type);

                 /* Start by showing the last group changed */
   selectOptionGroup (win, type, -1);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        takeCopyOfOptions
DESCRIPTION
   Take a tempory copy of the options structures
INPUT 

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
takeCopyOfOptions (OPTION_TYPE type)
#else
takeCopyOfOptions (type)
OPTION_TYPE type;
#endif
{
   BLOCK_VIEW_OPTIONS *viewOptions, *newViewOptions, *prevViewOption;

   freeCopyOfOptions (type);

   if (copyOfGeologyOptions = (GEOLOGY_OPTIONS *) xvt_mem_zalloc(sizeof(GEOLOGY_OPTIONS)))
      memcpy(copyOfGeologyOptions, &geologyOptions, sizeof(GEOLOGY_OPTIONS));

   if (copyOfGeophysicsOptions = (GEOPHYSICS_OPTIONS *) xvt_mem_zalloc(sizeof(GEOPHYSICS_OPTIONS)))
      memcpy(copyOfGeophysicsOptions, &geophysicsOptions, sizeof(GEOPHYSICS_OPTIONS));

   if (copyOfProjectOptions = (PROJECT_OPTIONS *) xvt_mem_zalloc(sizeof(PROJECT_OPTIONS)))
      memcpy(copyOfProjectOptions, &projectOptions, sizeof(PROJECT_OPTIONS));

   if (copyOfThreedViewOptions = (THREED_VIEW_OPTIONS *) xvt_mem_zalloc(sizeof(THREED_VIEW_OPTIONS)))
      memcpy(copyOfThreedViewOptions, &threedViewOptions, sizeof(THREED_VIEW_OPTIONS));
                             
   if (copyOfWinPositionOptions = (WINDOW_POSITION_OPTIONS *) xvt_mem_zalloc(sizeof(WINDOW_POSITION_OPTIONS)))
      memcpy(copyOfWinPositionOptions, &winPositionOptions, sizeof(WINDOW_POSITION_OPTIONS));

   if (viewOptions = blockViewOptions)
   {
      while (viewOptions->prev)       /* Move to first Option */
         viewOptions = viewOptions->prev;

      prevViewOption = NULL;
      do {
         if (newViewOptions = (BLOCK_VIEW_OPTIONS *) xvt_mem_zalloc(sizeof(BLOCK_VIEW_OPTIONS)))
         {
            memcpy (newViewOptions, viewOptions, sizeof(BLOCK_VIEW_OPTIONS));
            newViewOptions->prev = prevViewOption;
            newViewOptions->next = NULL;
   
            if (prevViewOption)
               prevViewOption->next = newViewOptions;
            
            prevViewOption = newViewOptions;
            
            if (viewOptions == blockViewOptions)  /* Make sure we have the current view that is selected */
               copyOfBlockViewOptions = newViewOptions;
         }
      } while (viewOptions = viewOptions->next);
   }
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        restoreCopyOfOptions
DESCRIPTION
     Copy the copy of the structures back over the real ones (undo any changes)

INPUT 

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
restoreCopyOfOptions (OPTION_TYPE type)
#else
restoreCopyOfOptions (type)
OPTION_TYPE type;
#endif
{
   BLOCK_VIEW_OPTIONS *viewOptions;

   if (copyOfGeologyOptions)
      memcpy(&geologyOptions, copyOfGeologyOptions, sizeof(GEOLOGY_OPTIONS));

   if (copyOfGeophysicsOptions)
      memcpy(&geophysicsOptions, copyOfGeophysicsOptions, sizeof(GEOPHYSICS_OPTIONS));

   if (copyOfProjectOptions)
      memcpy(&projectOptions, copyOfProjectOptions, sizeof(PROJECT_OPTIONS));

   if (copyOfThreedViewOptions)
      memcpy(&threedViewOptions, copyOfThreedViewOptions, sizeof(THREED_VIEW_OPTIONS));

   if (copyOfWinPositionOptions)
      memcpy(&winPositionOptions, copyOfWinPositionOptions, sizeof(WINDOW_POSITION_OPTIONS));

   if (copyOfBlockViewOptions)  /* Just swap with the copy to restore */
   {
      viewOptions = blockViewOptions;
      blockViewOptions = copyOfBlockViewOptions;
      copyOfBlockViewOptions = viewOptions;
   }
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        freeCopyOfOptions
DESCRIPTION
     Free the memory associated with the copy of the options

INPUT 

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
freeCopyOfOptions (OPTION_TYPE type)
#else
freeCopyOfOptions (type)
OPTION_TYPE type;
#endif
{
   BLOCK_VIEW_OPTIONS *viewOptions, *nextViewOptions;

   if (copyOfGeologyOptions)
   {
      xvt_mem_free ((char *) copyOfGeologyOptions);
      copyOfGeologyOptions = NULL;
   }

   if (copyOfGeophysicsOptions)
   {
      xvt_mem_free ((char *) copyOfGeophysicsOptions);
      copyOfGeophysicsOptions = NULL;
   }

   if (copyOfProjectOptions)
   {
      xvt_mem_free ((char *) copyOfProjectOptions);
      copyOfProjectOptions = NULL;
   }

   if (copyOfThreedViewOptions)
   {
      xvt_mem_free ((char *) copyOfThreedViewOptions);
      copyOfThreedViewOptions = NULL;
   }

   if (copyOfWinPositionOptions)
   {
      xvt_mem_free ((char *) copyOfWinPositionOptions);
      copyOfWinPositionOptions = NULL;
   }

   if (copyOfBlockViewOptions)
   {
      viewOptions = copyOfBlockViewOptions;
      while (viewOptions->prev)       /* Move to first Option */
         viewOptions = viewOptions->prev;

      do {     /* Free the list */
         nextViewOptions = viewOptions->next;
         xvt_mem_free ((char *) viewOptions);
      } while (viewOptions = nextViewOptions);
      
      copyOfBlockViewOptions = NULL;
   }
   
   return (TRUE);
}


/* ======================================================================
FUNCTION        optionsMouseUp
DESCRIPTION
     load the selected paramaters from the options structure for
     that particular event

INPUT 
     WINDOW win;         the window of the event
     mouse;   mouse info

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
optionsMouseUp (WINDOW win, struct s_mouse *mouse)
#else
optionsMouseUp (win, mouse)
WINDOW win;
struct s_mouse *mouse;
#endif
{
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        loadEventOptions
DESCRIPTION
     load the selected paramaters from the options structure for
     that particular event

INPUT 
     WINDOW win;         the window of the event
     OPTION_TYPE type;   identifies the event being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
loadOptions (WINDOW win, OPTION_TYPE type)
#else
loadOptions (win, type)
WINDOW win;
OPTION_TYPE type;
#endif
{
   loadGeologyDisplayOptions (type);
   loadGeophysicsCalcOptions (type);
   loadGeophysicsSurveyOptions (type);
   loadGeophysicsDisplayOptions (type);
   loadGlobalProjectOptions (type);
   loadWindowPosOptions (type);
   loadBlockViewOptions (type);
   loadMovieOptions (type);
   loadSectionBoreholeOptions (type);
   loadVolumeSurfaceOptions (type);
   
   return (TRUE);
}


/* ======================================================================
FUNCTION        saveOptions
DESCRIPTION
     store the selected paramaters into the options structure for
     that particular event

INPUT 
     WINDOW win;         the window of the event
     OPTION_TYPE type;   identifies the option being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
saveOptions (WINDOW win, OPTION_TYPE type, int previewOnly)
#else
saveOptions (win, type, previewOnly)
WINDOW win;
OPTION_TYPE type;
int previewOnly;
#endif
{
   saveGeologyDisplayOptions (type);
   saveGeophysicsCalcOptions (type);
   saveGeophysicsSurveyOptions (type);
   saveGeophysicsDisplayOptions (type);
   saveGlobalProjectOptions (type);
   saveWindowPosOptions (type);
   saveBlockViewOptions (type);
   saveMovieOptions (type);
   saveSectionBoreholeOptions (type);
   saveVolumeSurfaceOptions (type);

              /* needed in case topo or alike selected */
   if (!previewOnly)
      updateMenuOptions (TASK_MENUBAR, NULL_WIN);

   return (FALSE);
}

/* ======================================================================
FUNCTION        updateOptions
DESCRIPTION
     Update the event options window display

INPUT 
     WINDOW win;         the window of the event
     OPTION_TYPE type;     the output file for the block

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
updateOptions (WINDOW win, OPTION_TYPE type)
#else
updateOptions (win, type)
WINDOW win;
OPTION_TYPE type;
#endif
{
   updateGeologyDisplayOptions (type);
   updateGeophysicsCalcOptions (type);
   updateGeophysicsSurveyOptions (type);
   updateGeophysicsDisplayOptions (type);
   updateGlobalProjectOptions (type);
   updateWindowPosOptions (type);
   updateBlockViewOptions (type);
   updateMovieOptions (type);
   updateSectionBoreholeOptions (type);
   updateVolumeSurfaceOptions (type);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        selectOptionGroup
DESCRIPTION
     Update the event options window to display current group

INPUT 
     WINDOW win;         the window of the event
     OPTION_TYPE type;     the output file for the block

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
selectOptionGroup (WINDOW win, OPTION_TYPE type, int newIndex)
#else
selectOptionGroup (win, type, newIndex)
WINDOW win;
OPTION_TYPE type;
int newIndex;
#endif
{
   int group, index;
   WINDOW listWin;
   OPTION_TYPE selectedType;

   listWin = xvt_win_get_ctl (win, OPTIONS_SELECTION);

                        /* remove any currently showing */
   for (index = 0; index < MAX_OPTION_GROUPS; index++)
   {
      if (optionTabs[index].win)
         xvt_vobj_set_visible (optionTabs[index].win, FALSE);
		if (newIndex < 0)  /* If it was not picked from a list make the type passed in */
			if ((OPTION_TYPE) xvt_vobj_get_data(optionTabs[index].win) == type)
			   newIndex = optionTabs[index].group;
   }
   
                      /* show the one that was selected  */
                      /* this may be more than one panel */
   for (group = -1, index = 0; index < MAX_OPTION_GROUPS; index++)
   {
      if ((index < 1) || (optionTabs[index-1].group != optionTabs[index].group))
         group++;

      if ((group == newIndex) && optionTabs[index].win)
      {
         xvt_vobj_set_visible (optionTabs[index].win, TRUE);
         selectedType = (OPTION_TYPE) xvt_vobj_get_data (optionTabs[index].win);
#if (XVTWS == MTFWS) || (XVTWS == MACWS) || (XVTWS == XOLWS)
#else
         bringWindowToFront(optionTabs[index].win);
#endif
                        /* make sure we showing correct selection */
         if (listWin)
            xvt_list_set_sel (listWin, group, TRUE);
      }
   }
   
#if (XVTWS == MTFWS) || (XVTWS == MACWS) || (XVTWS == XOLWS)
#else
   if (listWin)
      xvt_scr_set_focus_vobj (listWin);
#endif

   updateOptions (win, selectedType);

   return (TRUE);
}

/* ======================================================================
FUNCTION        displayOptionsHelp
DESCRIPTION
     Display the right help page for the event optoins window

INPUT 
     OPTION_TYPE type;   identifies the options we want help for

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
displayOptionsHelp (OPTION_TYPE type)
#else
displayOptionsHelp (type)
OPTION_TYPE type;
#endif
{
   switch (type)
   {
      case GEOLOGY_DISPLAY_OP:
			displayHelp("geoldisp.htm");
         break;
      case GEOPHYSICS_SURVEY_OP:
			displayHelp("survop.htm");
         break;
      case GEOPHYSICS_CALC_OP:
			displayHelp("gpscalcop.htm");
         break;
      case GEOPHYSICS_DISPLAY_OP:
			displayHelp("gpsdispop.htm");
         break;
      case PROJECT_OP:
			displayHelp("projop.htm");
         break;
      case WINDOW_POSITION_OP:
			displayHelp("winpos.htm");
         break;
      case BLOCK_VIEW_OP:
			displayHelp("blockop.htm");
         break;
      case MOVIE_OP:
			displayHelp("movieop.htm");
         break;
      case BOREHOLE_SECTION_OP:
			displayHelp("boresect.htm");
         break;
      case VOLUME_SURFACE_OP:
			displayHelp("volsurf.htm");
         break;
      default:
			displayHelp("toc.htm");
   }

   return (TRUE);
}

/* ************************************************************* */
/* ********************* Geology Display *********************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadGeologyDisplayOptions (OPTION_TYPE type)
#else
loadGeologyDisplayOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;

   if (win = optionTabs[GEOLOGY_DISPLAY_INDEX+0].win)
   {
      cwin = updateFloatTextField (win, THREED_DECL,    threedViewOptions.declination, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, THREED_AZIMUTH, threedViewOptions.azimuth, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, THREED_SCALE,   threedViewOptions.scale*100.0, 1);
      setControlFont (cwin);

      updateScrollAndText(win, THREED_DECL_BAR,    THREED_DECL,    FALSE);
      updateScrollAndText(win, THREED_AZIMUTH_BAR, THREED_AZIMUTH, FALSE);
      updateScrollAndText(win, THREED_SCALE_BAR,   THREED_SCALE,   FALSE);
   }

   if (win = optionTabs[GEOLOGY_DISPLAY_INDEX+1].win)
   {
      cwin = xvt_win_get_ctl (win, CHAIR_USE);
      xvt_ctl_set_checked(cwin, geologyOptions.calculateChairDiagram);

      cwin = updateFloatTextField (win, CHAIR_X, geologyOptions.chair_X, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, CHAIR_Y, geologyOptions.chair_Y, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, CHAIR_Z, geologyOptions.chair_Z, 1);
      setControlFont (cwin);

      updateScrollAndText(win, CHAIR_X_BAR, CHAIR_X, FALSE);
      updateScrollAndText(win, CHAIR_Y_BAR, CHAIR_Y, FALSE);
      updateScrollAndText(win, CHAIR_Z_BAR, CHAIR_Z, FALSE);
   }

   if (win = optionTabs[GEOLOGY_DISPLAY_INDEX+2].win)
   {
      cwin = xvt_win_get_ctl (win, NEWWIN_EACHTIME);
      xvt_ctl_set_checked(cwin, projectOptions.newWindowEachStage);
   }

   if (win = optionTabs[GEOLOGY_DISPLAY_INDEX+3].win)  /* Background Color */
   {
   }

   updateGeologyDisplayOptions(type);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveGeologyDisplayOptions (OPTION_TYPE type)
#else
saveGeologyDisplayOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[GEOLOGY_DISPLAY_INDEX+0].win)
   {
      threedViewOptions.declination = getFloatTextFieldValue(win, THREED_DECL);
      threedViewOptions.azimuth = getFloatTextFieldValue(win, THREED_AZIMUTH);
      threedViewOptions.scale = getFloatTextFieldValue(win, THREED_SCALE)/100.0;
		if (threedViewOptions.scale < TOLERANCE)
			threedViewOptions.scale = TOLERANCE;
   }

   if (win = optionTabs[GEOLOGY_DISPLAY_INDEX+1].win)
   {
      geologyOptions.calculateChairDiagram = xvt_ctl_is_checked(xvt_win_get_ctl (win, CHAIR_USE));

      geologyOptions.chair_X = getFloatTextFieldValue(win, CHAIR_X);
      geologyOptions.chair_Y = getFloatTextFieldValue(win, CHAIR_Y);
      geologyOptions.chair_Z = getFloatTextFieldValue(win, CHAIR_Z);
   }

   if (win = optionTabs[GEOLOGY_DISPLAY_INDEX+2].win)
   {
      projectOptions.newWindowEachStage = xvt_ctl_is_checked(xvt_win_get_ctl (win, NEWWIN_EACHTIME));
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateGeologyDisplayOptions (OPTION_TYPE type)
#else
updateGeologyDisplayOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;
   BOOLEAN useChair;
   
   if (win = optionTabs[GEOLOGY_DISPLAY_INDEX+1].win)
   {
      useChair = xvt_ctl_is_checked(xvt_win_get_ctl(win, CHAIR_USE));
      
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_X), useChair);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_Y), useChair);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_Z), useChair);

      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_X_BAR), useChair);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_Y_BAR), useChair);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_Z_BAR), useChair);

      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_X_LABEL), useChair);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_Y_LABEL), useChair);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, CHAIR_Z_LABEL), useChair);
   }

   return (TRUE);
}

/* ************************************************************* */
/* ***************** Geophysics Calculation ******************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadGeophysicsCalcOptions (OPTION_TYPE type)
#else
loadGeophysicsCalcOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;

   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+0].win)
   {
      xvt_ctl_set_checked(xvt_win_get_ctl (win, GENERATE_GRAVITY), TRUE);
      xvt_ctl_set_checked(xvt_win_get_ctl (win, GENERATE_MAGNETICS), TRUE);
      xvt_ctl_set_checked(xvt_win_get_ctl (win, GENERATE_COMPONENTS), geophysicsOptions.magneticVectorComponents);
   }

   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+1].win)      /* Calc Type */
   {
      cwin = xvt_win_get_ctl (win, CALC_TYPE_LIST);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin, 0, "Spatial Convolution");
      xvt_list_add (cwin, 1, "Spectral");
#ifndef ENCOM_V6
      xvt_list_add (cwin, 2, "Spatial Full");
#endif
      xvt_list_resume (cwin);

      xvt_list_set_sel(cwin, (int) geophysicsOptions.calculationMethod, TRUE);

      xvt_ctl_set_checked(xvt_win_get_ctl (win, CALC_TYPE_PROJECT_FIELDS), geophysicsOptions.projectVectorsOntoField);
   }

   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+2].win)      /* Variable Cube */
   {
      cwin = xvt_win_get_ctl (win, VARIABLE_CUBE_CUTOFF);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "All");
      xvt_list_add (cwin , 1, "1000");
      xvt_list_add (cwin , 2, "2000");
      xvt_list_add (cwin , 3, "3000");
      xvt_list_add (cwin , 4, "4000");
      xvt_list_add (cwin , 5, "5000");
      xvt_list_add (cwin , 6, "6000");
      xvt_list_add (cwin , 7, "7000");
      xvt_list_add (cwin , 8, "8000");
      xvt_list_add (cwin , 9, "9000");
      xvt_list_add (cwin , 10, "10000");
      xvt_list_resume (cwin);
      
      cwin = xvt_win_get_ctl (win, VARIABLE_CUBE_RATIO);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "None");
      xvt_list_add (cwin , 1, "1:2");
      xvt_list_add (cwin , 2, "1:3");
      xvt_list_add (cwin , 3, "1:4");
      xvt_list_add (cwin , 4, "1:5");
      xvt_list_resume (cwin);

      if (geophysicsOptions.constantBoxDepth < 0.1)
      {
         xvt_vobj_set_title (xvt_win_get_ctl (win, VARIABLE_CUBE_CUTOFF), "All");
         xvt_vobj_set_enabled (xvt_win_get_ctl (win, VARIABLE_CUBE_RATIO), FALSE);
      }   
      else
      {
         updateFloatTextField (win, VARIABLE_CUBE_CUTOFF, geophysicsOptions.constantBoxDepth, 1);
         xvt_vobj_set_enabled (xvt_win_get_ctl (win, VARIABLE_CUBE_RATIO), TRUE);
      }
      xvt_list_set_sel(xvt_win_get_ctl (win, VARIABLE_CUBE_RATIO), (int) ceil(geophysicsOptions.cleverBoxRatio-1.0), TRUE);
   }
   
   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+3].win)           /* Range */
   {
      cwin = updateNumericTextField (win, RANGE_CONV, geophysicsOptions.calculationRange);
      setControlFont (cwin);
      cwin = updateNumericTextField (win, RANGE_ANALYTIC, geophysicsOptions.analyticRange);
      setControlFont (cwin);
		
		xvt_ctl_set_checked(xvt_win_get_ctl(win, RANGE_ALL_EXACT), (BOOLEAN) geophysicsOptions.allExact);
		xvt_ctl_set_checked(xvt_win_get_ctl(win, RANGE_REAL_GEOLOGY_PADDING), (BOOLEAN) geophysicsOptions.padWithRealGeology);
   }
   
   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+4].win)    /* Padding */
   {
      if (cwin = xvt_win_get_ctl (win, PADDING_TYPE))
      {
         xvt_list_suspend (cwin);
         xvt_list_add (cwin, 0, "Ramp");
         xvt_list_add (cwin, 1, "Fence using mean");
         xvt_list_add (cwin, 2, "Fence using set value");
         xvt_list_add (cwin, 3, "Set to mean (non cyclic)");
         xvt_list_add (cwin, 4, "Set to value (non cyclic)");
         xvt_list_add (cwin, 5, "Reflection");
         xvt_list_set_sel (cwin, geophysicsOptions.spectralPaddingType, TRUE);
         xvt_list_resume (cwin);
      }
      cwin = updateNumericTextField (win, PADDING_FENCE, geophysicsOptions.spectralFence);
      setControlFont (cwin);
      cwin = updateNumericTextField (win, PADDING_PERCENT, geophysicsOptions.spectralPercent);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, PADDING_DENSITY, geophysicsOptions.spectralDensity, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, PADDING_SUSX, geophysicsOptions.spectralSusX, 4);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, PADDING_SUSY, geophysicsOptions.spectralSusY, 4);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, PADDING_SUSZ, geophysicsOptions.spectralSusZ, 4);
      setControlFont (cwin);
   }
   
   updateGeophysicsCalcOptions(type);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveGeophysicsCalcOptions (OPTION_TYPE type)
#else
saveGeophysicsCalcOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+0].win)
   {
      geophysicsOptions.magneticVectorComponents = xvt_ctl_is_checked(xvt_win_get_ctl (win, GENERATE_COMPONENTS));
   }

   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+1].win)      /* Calc Type */
   {
      geophysicsOptions.calculationMethod = xvt_list_get_sel_index(xvt_win_get_ctl (win, CALC_TYPE_LIST));
      geophysicsOptions.projectVectorsOntoField = xvt_ctl_is_checked(xvt_win_get_ctl (win, CALC_TYPE_PROJECT_FIELDS));
   }

   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+2].win)      /* Variable Cube */
   {
      geophysicsOptions.constantBoxDepth = getFloatTextFieldValue (win, VARIABLE_CUBE_CUTOFF);
      geophysicsOptions.cleverBoxRatio = xvt_list_get_sel_index(xvt_win_get_ctl (win, VARIABLE_CUBE_RATIO))+1.0;
   }
   
   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+3].win)           /* Range */
   {
      geophysicsOptions.calculationRange = (int) floor (getFloatTextFieldValue (win, RANGE_CONV) +0.5);
      geophysicsOptions.analyticRange = (int) floor (getFloatTextFieldValue (win, RANGE_ANALYTIC) +0.5);
		geophysicsOptions.allExact = (BOOLEAN) xvt_ctl_is_checked(xvt_win_get_ctl(win, RANGE_ALL_EXACT));
		geophysicsOptions.padWithRealGeology = (BOOLEAN) xvt_ctl_is_checked(xvt_win_get_ctl(win, RANGE_REAL_GEOLOGY_PADDING));
   }
   
   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+4].win)    /* Spectral Padding */
   {
      geophysicsOptions.spectralPaddingType = xvt_list_get_sel_index (xvt_win_get_ctl (win, PADDING_TYPE));
      geophysicsOptions.spectralPercent = getIntegerTextFieldValue (win, PADDING_PERCENT);
      geophysicsOptions.spectralFence = getIntegerTextFieldValue (win, PADDING_FENCE);
      geophysicsOptions.spectralDensity = getFloatTextFieldValue (win, PADDING_DENSITY);
      geophysicsOptions.spectralSusX = getFloatTextFieldValue (win, PADDING_SUSX);
      geophysicsOptions.spectralSusY = getFloatTextFieldValue (win, PADDING_SUSY);
      geophysicsOptions.spectralSusZ = getFloatTextFieldValue (win, PADDING_SUSZ);
   }
   
   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateGeophysicsCalcOptions (OPTION_TYPE type)
#else
updateGeophysicsCalcOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   int numCubes, spectralPaddingType, getSpectralValues, getFenceValue, getPercentValue, calcType;
   BOOLEAN spectralCalc, spatialCalc;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions();
   char label[50];
   double range;
   
   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+1].win)  /* Calculation Type */
   {
      calcType = xvt_list_get_sel_index(xvt_win_get_ctl (win, CALC_TYPE_LIST));
      if (calcType == SPECTRAL)
			spectralCalc = TRUE;
		else
			spectralCalc = FALSE;
      spatialCalc = !spectralCalc;

      if (type == GEOPHYSICS_CALC_OP)
      {
         xvt_vobj_set_visible(optionTabs[GEOPHYSICS_CALC_INDEX+3].win, spatialCalc);
         xvt_vobj_set_visible(optionTabs[GEOPHYSICS_CALC_INDEX+4].win, spectralCalc);

         if (spectralCalc)
				xvt_ctl_set_checked(xvt_win_get_ctl (win, CALC_TYPE_PROJECT_FIELDS), TRUE);
         xvt_vobj_set_enabled(xvt_win_get_ctl (win, CALC_TYPE_PROJECT_FIELDS), spatialCalc);
      }
   }

   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+2].win)      /* Variable Cube */
   {
      cwin = xvt_win_get_ctl (win, VARIABLE_CUBE_RATIO);

      if (getFloatTextFieldValue (win, VARIABLE_CUBE_CUTOFF) < 0.1)
      {
         xvt_vobj_set_title (xvt_win_get_ctl (win, VARIABLE_CUBE_CUTOFF), "All");
         xvt_vobj_set_enabled (cwin, FALSE);
         xvt_list_set_sel (cwin, 0, TRUE);
      }   
      else
      {
         xvt_vobj_set_enabled (xvt_win_get_ctl (win, VARIABLE_CUBE_RATIO), TRUE);
         if (geophysicsOptions.cleverBoxRatio < 1.5)
				xvt_list_set_sel (cwin, 1, TRUE);
			else
		      xvt_list_set_sel(cwin, (int) ceil(geophysicsOptions.cleverBoxRatio-1.0), TRUE);
      }
   }
   
   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+3].win)  /* Calculation Range */
   {
      range = getFloatTextFieldValue (win, RANGE_CONV);
      if (range > 1.0)
      {
         numCubes = (int) floor (range/viewOptions->geophysicsCubeSize);
         sprintf (label, "(%d cubes)", numCubes);
      }
      else
         strcpy (label, "");
      xvt_vobj_set_title(xvt_win_get_ctl(win, RANGE_CONV_IN_CUBES_LABEL), label);

      range = getFloatTextFieldValue (win, RANGE_ANALYTIC);
      if (range > 1.0)
      {
         numCubes = (int) floor (range/viewOptions->geophysicsCubeSize);
         sprintf (label, "(%d cubes)", numCubes);
      }
      else
         strcpy (label, "");
      xvt_vobj_set_title(xvt_win_get_ctl(win, RANGE_ANALYTIC_IN_CUBES_LABEL), label);

		if (calcType == SPATIAL_FULL)
			xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_REAL_GEOLOGY_PADDING), (BOOLEAN) TRUE);
		else
			xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_REAL_GEOLOGY_PADDING), (BOOLEAN) FALSE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_ALL_EXACT), (BOOLEAN) FALSE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_CONV_LABEL), (BOOLEAN) TRUE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_CONV), (BOOLEAN) TRUE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_CONV_UNIT_LABEL), (BOOLEAN) TRUE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_CONV_IN_CUBES_LABEL), (BOOLEAN) TRUE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_ANALYTIC_LABEL), (BOOLEAN) TRUE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_ANALYTIC), (BOOLEAN) TRUE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_ANALYTIC_UNIT_LABEL), (BOOLEAN) TRUE);
		xvt_vobj_set_enabled(xvt_win_get_ctl(win, RANGE_ANALYTIC_IN_CUBES_LABEL), (BOOLEAN) TRUE);
	}

   if (win = optionTabs[GEOPHYSICS_CALC_INDEX+4].win)  /* Spectral Padding */
   {
      spectralPaddingType = xvt_list_get_sel_index (xvt_win_get_ctl (win, PADDING_TYPE));
      switch (spectralPaddingType)
      {
         case (FENCE_MEAN_PADDING):
            getFenceValue = TRUE;
            getPercentValue = FALSE;
            getSpectralValues = FALSE;
            break;
         case (RAMP_PADDING):
         case (SET_MEAN_PADDING):
            getFenceValue = FALSE;
            getPercentValue = FALSE;
            getSpectralValues = FALSE;
            break;
         case (RECLECTION_PADDING):
            getFenceValue = FALSE;
            getPercentValue = TRUE;
            getSpectralValues = FALSE;
            break;
         case (FENCE_VALUE_PADDING):
            getFenceValue = TRUE;
            getPercentValue = FALSE;
            getSpectralValues = TRUE;
            break;
         case (SET_VALUE_PADDING):
            getFenceValue = FALSE;
            getPercentValue = FALSE;
            getSpectralValues = TRUE;
            break;
      }

      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_FENCE), (BOOLEAN) getFenceValue);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_PERCENT), (BOOLEAN) getPercentValue);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_DENSITY), (BOOLEAN) getSpectralValues);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_SUSX), (BOOLEAN) getSpectralValues);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_SUSY), (BOOLEAN) getSpectralValues);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_SUSZ), (BOOLEAN) getSpectralValues);

      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_FENCE_LABEL), (BOOLEAN) getFenceValue);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_PERCENT_LABEL), (BOOLEAN) getPercentValue);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_DENSITY_LABEL), (BOOLEAN) getSpectralValues);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_SUSX_LABEL), (BOOLEAN) getSpectralValues);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_SUSY_LABEL), (BOOLEAN) getSpectralValues);
      xvt_vobj_set_enabled(xvt_win_get_ctl (win, PADDING_SUSZ_LABEL), (BOOLEAN) getSpectralValues);
   }

   return (TRUE);
}

/* ************************************************************* */
/* ******************** Geophysics Survey ********************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadGeophysicsSurveyOptions (OPTION_TYPE type)
#else
loadGeophysicsSurveyOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+0].win)    /* field */
   {
      xdCheckRadioButton(win, FIELD_FIXED+geophysicsOptions.fieldType, FIELD_FIXED, FIELD_VARIABLE);
      xdCheckRadioButton(win, FIELD_XYZ, FIELD_XYZ, FIELD_INTEN);

		cwin = updateFloatTextField (win, FIELD_INCLINATION, geophysicsOptions.inclination, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FIELD_DECLINATION, geophysicsOptions.declination, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FIELD_INTENSITY, geophysicsOptions.intensity, 1);
      setControlFont (cwin);

      cwin = updateFloatTextField (win, FIELD_X_POS, geophysicsOptions.xPos, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FIELD_Y_POS, geophysicsOptions.yPos, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FIELD_Z_POS, geophysicsOptions.zPos, 1);
      setControlFont (cwin);
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+1].win)    /* altitude */
   {
      if (geophysicsOptions.calculationAltitude == SURFACE)
         xdCheckRadioButton(win, ALTITUDE_SURFACE, ALTITUDE_SURFACE, ALTITUDE_AIRBORNE);
      else   /* Airborne */
         xdCheckRadioButton(win, ALTITUDE_AIRBORNE, ALTITUDE_SURFACE, ALTITUDE_AIRBORNE);
      cwin = updateFloatTextField (win, ALTITUDE_HEIGHT, geophysicsOptions.altitude, 1);
      setControlFont (cwin);
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+2].win)    /* deform */
   {
      xvt_ctl_set_checked(xvt_win_get_ctl(win, DEFORM_FIELD_REMANENCE), geophysicsOptions.deformableRemanence);
      xvt_ctl_set_checked(xvt_win_get_ctl(win, DEFORM_FIELD_ANISOTROPY), geophysicsOptions.deformableAnisotropy);
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+3].win)    /* sus Units */
   {
      if (projectOptions.susceptibilityUnits == SI)
         xdCheckRadioButton(win, SUS_UNITS_SI, SUS_UNITS_SI, SUS_UNITS_CGS);
      else   /* CGS */
         xdCheckRadioButton(win, SUS_UNITS_CGS, SUS_UNITS_SI, SUS_UNITS_CGS);
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+4].win)    /* draped Survey */
   {
      xvt_ctl_set_checked(xvt_win_get_ctl(win, DRAPED_SURVEY), geophysicsOptions.drapedSurvey);
   } 
   
   updateGeophysicsSurveyOptions (type);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveGeophysicsSurveyOptions (OPTION_TYPE type)
#else
saveGeophysicsSurveyOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+0].win)    /* field */
   {
      geophysicsOptions.fieldType = xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_VARIABLE));

		geophysicsOptions.inclination = getFloatTextFieldValue (win, FIELD_INCLINATION);
		geophysicsOptions.declination = getFloatTextFieldValue (win, FIELD_DECLINATION);
		geophysicsOptions.intensity = getFloatTextFieldValue (win, FIELD_INTENSITY);

		if (xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_XYZ)))
		{
			geophysicsOptions.xPos = getFloatTextFieldValue (win, FIELD_X_POS);
			geophysicsOptions.yPos = getFloatTextFieldValue (win, FIELD_Y_POS);
			geophysicsOptions.zPos = getFloatTextFieldValue (win, FIELD_Z_POS);
		}
		else if (xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_DECL)))
		{
			geophysicsOptions.declinationOri = getFloatTextFieldValue (win, FIELD_X_POS);
			geophysicsOptions.declinationChange = getFloatTextFieldValue (win, FIELD_Y_POS);
		}
		else if (xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_INCL)))
		{
			geophysicsOptions.inclinationOri = getFloatTextFieldValue (win, FIELD_X_POS);
			geophysicsOptions.inclinationChange = getFloatTextFieldValue (win, FIELD_Y_POS);
		}
		else if (xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_INTEN)))
		{
			geophysicsOptions.intensityOri = getFloatTextFieldValue (win, FIELD_X_POS);
			geophysicsOptions.intensityChange = getFloatTextFieldValue (win, FIELD_Y_POS);
		}
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+1].win)    /* altitude */
   {
      geophysicsOptions.calculationAltitude = xvt_ctl_is_checked(xvt_win_get_ctl (win, ALTITUDE_AIRBORNE));
      geophysicsOptions.altitude = getFloatTextFieldValue (win, ALTITUDE_HEIGHT);
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+2].win)    /* deform */
   {
      geophysicsOptions.deformableRemanence = xvt_ctl_is_checked(xvt_win_get_ctl (win, DEFORM_FIELD_REMANENCE));
      geophysicsOptions.deformableAnisotropy = xvt_ctl_is_checked(xvt_win_get_ctl (win, DEFORM_FIELD_ANISOTROPY));
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+3].win)    /* sus Units */
   {
      projectOptions.susceptibilityUnits = xvt_ctl_is_checked(xvt_win_get_ctl (win, SUS_UNITS_CGS));
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+4].win)    /* draped Survey */
   {
      geophysicsOptions.drapedSurvey = xvt_ctl_is_checked(xvt_win_get_ctl (win, DRAPED_SURVEY));
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateGeophysicsSurveyOptions (OPTION_TYPE type)
#else
updateGeophysicsSurveyOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;
   int needHeight, variable;

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+0].win)    /* field */
   {
      variable = xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_VARIABLE));
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_XYZ), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_DECL), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_INCL), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_INTEN), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_X_POS_LABEL), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_X_POS), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_Y_POS_LABEL), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_Y_POS), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_Z_POS_LABEL), (BOOLEAN) variable);
		xvt_vobj_set_enabled(xvt_win_get_ctl (win, FIELD_Z_POS), (BOOLEAN) variable);

		if (xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_XYZ)))
		{
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_X_POS_LABEL), "X :");
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_Y_POS_LABEL), "Y :");
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_Z_POS_LABEL), "Z :");
			updateFloatTextField (win, FIELD_X_POS, geophysicsOptions.xPos, 1);
			updateFloatTextField (win, FIELD_Y_POS, geophysicsOptions.yPos, 1);
			updateFloatTextField (win, FIELD_Z_POS, geophysicsOptions.zPos, 1);
			xvt_vobj_set_visible(xvt_win_get_ctl(win, FIELD_Z_POS_LABEL), TRUE);
			xvt_vobj_set_visible(xvt_win_get_ctl(win, FIELD_Z_POS), TRUE);
		}
		else if (xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_DECL)))
		{
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_X_POS_LABEL), "Orient:");
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_Y_POS_LABEL), "Change:");
			updateFloatTextField (win, FIELD_X_POS, geophysicsOptions.declinationOri, 1);
			updateFloatTextField (win, FIELD_Y_POS, geophysicsOptions.declinationChange, 1);
			xvt_vobj_set_visible(xvt_win_get_ctl(win, FIELD_Z_POS_LABEL), FALSE);
			xvt_vobj_set_visible(xvt_win_get_ctl(win, FIELD_Z_POS), FALSE);
		}
		else if (xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_INCL)))
		{
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_X_POS_LABEL), "Orient:");
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_Y_POS_LABEL), "Change:");
			updateFloatTextField (win, FIELD_X_POS, geophysicsOptions.inclinationOri, 1);
			updateFloatTextField (win, FIELD_Y_POS, geophysicsOptions.inclinationChange, 1);
			xvt_vobj_set_visible(xvt_win_get_ctl(win, FIELD_Z_POS_LABEL), FALSE);
			xvt_vobj_set_visible(xvt_win_get_ctl(win, FIELD_Z_POS), FALSE);
		}
		else if (xvt_ctl_is_checked(xvt_win_get_ctl (win, FIELD_INTEN)))
		{
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_X_POS_LABEL), "Orient:");
			xvt_vobj_set_title(xvt_win_get_ctl (win, FIELD_Y_POS_LABEL), "Change:");
			updateFloatTextField (win, FIELD_X_POS, geophysicsOptions.intensityOri, 1);
			updateFloatTextField (win, FIELD_Y_POS, geophysicsOptions.intensityChange, 1);
			xvt_vobj_set_visible(xvt_win_get_ctl(win, FIELD_Z_POS_LABEL), FALSE);
			xvt_vobj_set_visible(xvt_win_get_ctl(win, FIELD_Z_POS), FALSE);
		}
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+1].win)    /* altitude */
   {
      needHeight = xvt_ctl_is_checked(xvt_win_get_ctl (win, ALTITUDE_AIRBORNE));
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, ALTITUDE_HEIGHT), (BOOLEAN) needHeight);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, ALTITUDE_UNIT_LABEL), (BOOLEAN) needHeight);
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+2].win)    /* deform */
   {
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+3].win)    /* sus Units */
   {
   }

   if (win = optionTabs[GEOPHYSICS_SURVEY_INDEX+4].win)    /* draped Survey */
   {
   }

   return (TRUE);
}

/* ************************************************************* */
/* ******************** Geophysics Display ********************* */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadGeophysicsDisplayOptions (OPTION_TYPE type)
#else
loadGeophysicsDisplayOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;

   if (win = optionTabs[GEOPHYSICS_DISPLAY_INDEX+0].win)    /* image Scale */
   {
      cwin = updateFloatTextField (win, GEOPHYSICS_SCALE, projectOptions.imageScalingFactor, 1);
      setControlFont (cwin);
   }

   if (win = optionTabs[GEOPHYSICS_DISPLAY_INDEX+1].win)    /* Display Options */
   {
   }

   updateGeophysicsDisplayOptions (type);
   
   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveGeophysicsDisplayOptions (OPTION_TYPE type)
#else
saveGeophysicsDisplayOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[GEOPHYSICS_DISPLAY_INDEX+0].win)    /* image Scale */
   {
      projectOptions.imageScalingFactor = getFloatTextFieldValue (win, GEOPHYSICS_SCALE);
   }

   if (win = optionTabs[GEOPHYSICS_DISPLAY_INDEX+1].win)    /* Display Options */
   {
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateGeophysicsDisplayOptions (OPTION_TYPE type)
#else
updateGeophysicsDisplayOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[GEOPHYSICS_DISPLAY_INDEX+0].win)    /* image Scale */
   {
   }

   if (win = optionTabs[GEOPHYSICS_DISPLAY_INDEX+1].win)    /* Display Options */
   {
   }

   return (TRUE);
}

/* ************************************************************* */
/* ************************* Project *************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadGlobalProjectOptions (OPTION_TYPE type)
#else
loadGlobalProjectOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;

   if (win = optionTabs[PROJECT_INDEX+0].win)    /* local / remote */
   {
      if (projectOptions.calculationType == REMOTE_JOB)
         xdCheckRadioButton(win, LOCAL_REMOTE_REMOTE, LOCAL_REMOTE_REMOTE, LOCAL_REMOTE_LOCAL);
      else   /* Local */
         xdCheckRadioButton(win, LOCAL_REMOTE_LOCAL, LOCAL_REMOTE_REMOTE, LOCAL_REMOTE_LOCAL);
   }
   
   if (win = optionTabs[PROJECT_INDEX+1].win)    /* Unit Meters/Feet */
   {
      if (projectOptions.lengthScale == FEET)
         xdCheckRadioButton(win, UNIT_FEET, UNIT_METERS, UNIT_FEET);
      else   /* Meters */
         xdCheckRadioButton(win, UNIT_METERS, UNIT_METERS, UNIT_FEET);
   }

   if (win = optionTabs[PROJECT_INDEX+2].win)    /* Print Scale */
   {
      cwin = updateFloatTextField (win, PRINT_SCALE, projectOptions.printScalingFactor, 1);
      setControlFont (cwin);
   }

   if (win = optionTabs[PROJECT_INDEX+3].win)    /* helpPath */
   {
		char *helpPathOverride;

      cwin = xvt_win_get_ctl(win, HELP_VIEWER);
		if (helpPathOverride = getenv("NODDY_HELP_PATH"))
			xvt_vobj_set_title(cwin, helpPathOverride);
		else
			xvt_vobj_set_title(cwin, projectOptions.helpPath);
      setControlFont (cwin);
   }

   updateGlobalProjectOptions (type);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveGlobalProjectOptions (OPTION_TYPE type)
#else
saveGlobalProjectOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   
   if (win = optionTabs[PROJECT_INDEX+0].win)    /* local / remote */
   {
      if (xvt_ctl_is_checked(xvt_win_get_ctl(win, LOCAL_REMOTE_REMOTE)))
         projectOptions.calculationType = REMOTE_JOB;
      else
         projectOptions.calculationType = LOCAL_JOB;
   }
   
   if (win = optionTabs[PROJECT_INDEX+1].win)    /* Unit Meters/Feet */
   {
      if (xvt_ctl_is_checked(xvt_win_get_ctl(win, UNIT_FEET)))
         projectOptions.calculationType = FEET;
      else
         projectOptions.calculationType = METRES;
   }

   if (win = optionTabs[PROJECT_INDEX+2].win)    /* Print Scale */
   {
      projectOptions.printScalingFactor = getFloatTextFieldValue (win, PRINT_SCALE);
   }

   if (win = optionTabs[PROJECT_INDEX+3].win)    /* helpPath */
   {
      cwin = xvt_win_get_ctl(win, HELP_VIEWER);
		xvt_vobj_get_title(cwin, projectOptions.helpPath, 100);
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateGlobalProjectOptions (OPTION_TYPE type)
#else
updateGlobalProjectOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[PROJECT_INDEX+0].win)    /* local / remote */
   {
   }
   
   if (win = optionTabs[PROJECT_INDEX+1].win)    /* Unit Meters/Feet */
   {
   }

   if (win = optionTabs[PROJECT_INDEX+2].win)    /* Print Scale */
   {
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* ******************** Window Positions *********************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadWindowPosOptions (OPTION_TYPE type)
#else
loadWindowPosOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   int winNum;

   if (win = optionTabs[WINDOW_POSITION_INDEX+0].win)    /* Window Positions */
   {
      cwin = xvt_win_get_ctl (win, WINPOSITION_LIST);
      xvt_list_suspend(cwin);
      for (winNum = 0; winNum < winPositionOptions.numWindows; winNum++)
      {
         xvt_list_add(cwin, winNum, winPositionOptions.winPositions[winNum].name);
      }
      xvt_list_set_sel(cwin, 0, TRUE);
      xvt_list_resume(cwin);

      setControlFont (xvt_win_get_ctl(win, WINPOSITION_X));
      setControlFont (xvt_win_get_ctl(win, WINPOSITION_Y));
      setControlFont (xvt_win_get_ctl(win, WINPOSITION_WIDTH));
      setControlFont (xvt_win_get_ctl(win, WINPOSITION_HEIGHT));
   }
   
   copyOptions (WINPOS_WINDOW, (OBJECT *) &winPositionOptions, OPTIONS_SAVE);
   
   updateWindowPosOptions (type);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveWindowPosOptions (OPTION_TYPE type)
#else
saveWindowPosOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   int winNum;

   if (win = optionTabs[WINDOW_POSITION_INDEX+0].win)    /* Window Positions */
   {
      cwin = xvt_win_get_ctl (win, WINPOSITION_LIST);
      winNum = xvt_list_get_sel_index(cwin);
      
      winPositionOptions.winPositions[winNum].x
                         = (short) getIntegerTextFieldValue (win, WINPOSITION_X);
      winPositionOptions.winPositions[winNum].y
                         = (short) getIntegerTextFieldValue (win, WINPOSITION_Y);
      winPositionOptions.winPositions[winNum].width
                         = (short) getIntegerTextFieldValue (win, WINPOSITION_WIDTH);
      winPositionOptions.winPositions[winNum].height
                         = (short) getIntegerTextFieldValue (win, WINPOSITION_HEIGHT);
   }
   
   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateWindowPosOptions (OPTION_TYPE type)
#else
updateWindowPosOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   int winNum, enableHeight, enableWidth;

   if (win = optionTabs[WINDOW_POSITION_INDEX+0].win)    /* Window Positions */
   {
      cwin = xvt_win_get_ctl (win, WINPOSITION_LIST);
      winNum = xvt_list_get_sel_index(cwin);
      
      updateNumericTextField (win, WINPOSITION_X,
                             (int) winPositionOptions.winPositions[winNum].x);
      updateNumericTextField (win, WINPOSITION_Y,
                             (int) winPositionOptions.winPositions[winNum].y);
      updateNumericTextField (win, WINPOSITION_WIDTH,
                             (int) winPositionOptions.winPositions[winNum].width);
      updateNumericTextField (win, WINPOSITION_HEIGHT,
                             (int) winPositionOptions.winPositions[winNum].height);
      
              /* disable width and height if they are -ve values 
              ** This is needed for windows like anomalist that specifing
              ** a width and height for is silly */
      if (winPositionOptions.winPositions[winNum].height < 0.0)
      {
         cwin = xvt_win_get_ctl (win, WINPOSITION_HEIGHT);
         xvt_vobj_set_title (cwin, "N/A");
         enableHeight = FALSE;
      }
      else
         enableHeight = TRUE;
   
      if (winPositionOptions.winPositions[winNum].width < 0.0)
      {
         cwin = xvt_win_get_ctl (win, WINPOSITION_WIDTH);
         xvt_vobj_set_title (cwin, "N/A");
         enableWidth = FALSE;
      }
      else
         enableWidth = TRUE;
         
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, WINPOSITION_HEIGHT), (BOOLEAN) enableHeight);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, WINPOSITION_HEIGHT_BAR), (BOOLEAN) enableHeight);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, WINPOSITION_HEIGHT_LABEL), (BOOLEAN) enableHeight);
   
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, WINPOSITION_WIDTH), (BOOLEAN) enableWidth);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, WINPOSITION_WIDTH_BAR), (BOOLEAN) enableWidth);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, WINPOSITION_WIDTH_LABEL), (BOOLEAN) enableWidth);

      updateScrollAndText(win, WINPOSITION_X_BAR,      WINPOSITION_X,      FALSE);
      updateScrollAndText(win, WINPOSITION_Y_BAR,      WINPOSITION_Y,      FALSE);
      updateScrollAndText(win, WINPOSITION_WIDTH_BAR,  WINPOSITION_WIDTH,  FALSE);
      updateScrollAndText(win, WINPOSITION_HEIGHT_BAR, WINPOSITION_HEIGHT, FALSE);
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* ************************ Block View ************************* */
/* ************************************************************* */
int
#if XVT_CC_PROTO
loadBlockViewOptions (OPTION_TYPE type)
#else
loadBlockViewOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   BLOCK_VIEW_OPTIONS *viewOptions = NULL, *currentOptions = NULL;
   int index;

   if (!(currentOptions = getViewOptions ()))
      return (FALSE);

   if (win = optionTabs[BLOCK_VIEW_INDEX+0].win)    /* View Name */
   {
      cwin = xvt_win_get_ctl (win, VIEW_NAME_LIST);
      xvt_list_suspend (cwin);
      xvt_list_clear (cwin);
      viewOptions = currentOptions;
      while (viewOptions->prev)    /* move to start */
         viewOptions = viewOptions->prev;
      index = 0;
      while (viewOptions)
      {
         xvt_list_add (cwin, index, viewOptions->viewName);
         if (viewOptions == currentOptions)     /* Select current */
            xvt_list_set_sel (cwin, index, TRUE);
         viewOptions = viewOptions->next;
      }
      xvt_list_resume (cwin);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+1].win)    /* Origin */
   {
      cwin = updateFloatTextField (win, ORIGIN_X, currentOptions->originX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, ORIGIN_Y, currentOptions->originY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, ORIGIN_Z, currentOptions->originZ, 1);
      setControlFont (cwin);

      updateScrollAndText (win, ORIGIN_X_BAR, ORIGIN_X, FALSE);
      updateScrollAndText (win, ORIGIN_Y_BAR, ORIGIN_Y, FALSE);
      updateScrollAndText (win, ORIGIN_Z_BAR, ORIGIN_Z, FALSE);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+2].win)    /* Dimension */
   {
      cwin = updateFloatTextField (win, DIMENSION_X, currentOptions->lengthX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DIMENSION_Y, currentOptions->lengthY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DIMENSION_Z, currentOptions->lengthZ, 1);
      setControlFont (cwin);

      updateScrollAndText (win, DIMENSION_X_BAR, DIMENSION_X, FALSE);
      updateScrollAndText (win, DIMENSION_Y_BAR, DIMENSION_Y, FALSE);
      updateScrollAndText (win, DIMENSION_Z_BAR, DIMENSION_Z, FALSE);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+3].win)    /* Cube Size */
   {
      cwin = xvt_win_get_ctl (win, CUBE_SIZE_GEOLOGY);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin, 0, "50");
      xvt_list_add (cwin, 1, "75");
      xvt_list_add (cwin, 2, "100");
      xvt_list_add (cwin, 3, "150");
      xvt_list_add (cwin, 4, "200");
      xvt_list_add (cwin, 5, "300");
      xvt_list_add (cwin, 6, "400");
      xvt_list_add (cwin, 7, "500");
      xvt_list_add (cwin, 8, "600");
      xvt_list_add (cwin, 9, "700");
      xvt_list_add (cwin, 10, "800");
      xvt_list_add (cwin, 11, "900");
      xvt_list_add (cwin, 12, "1000");
      xvt_list_resume (cwin);
      cwin = xvt_win_get_ctl (win, CUBE_SIZE_GEOPHYSICS);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin, 0, "50");
      xvt_list_add (cwin, 1, "75");
      xvt_list_add (cwin, 2, "100");
      xvt_list_add (cwin, 3, "150");
      xvt_list_add (cwin, 4, "200");
      xvt_list_add (cwin, 5, "300");
      xvt_list_add (cwin, 6, "400");
      xvt_list_add (cwin, 7, "500");
      xvt_list_add (cwin, 8, "600");
      xvt_list_add (cwin, 9, "700");
      xvt_list_add (cwin, 10, "800");
      xvt_list_add (cwin, 11, "900");
      xvt_list_add (cwin, 12, "1000");
      xvt_list_resume (cwin);

      updateFloatTextField (win, CUBE_SIZE_GEOLOGY, currentOptions->geologyCubeSize, 1);
      updateFloatTextField (win, CUBE_SIZE_GEOPHYSICS, currentOptions->geophysicsCubeSize, 1);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+4].win)    /* False Origin */
   {
      cwin = updateFloatTextField (win, FALSE_ORIGIN_EASTING, projectOptions.easting, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FALSE_ORIGIN_NORTHING, projectOptions.northing, 1);
      setControlFont (cwin);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+5].win)    /* Use Topography */
   {
      xvt_ctl_set_checked(xvt_win_get_ctl(win, USE_TOPOGRAPHY), geologyOptions.useTopography);
   }

   updateBlockViewOptions (type);

   return (TRUE);
}

int
#if XVT_CC_PROTO
saveBlockViewOptions (OPTION_TYPE type)
#else
saveBlockViewOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;
   BLOCK_VIEW_OPTIONS *viewOptions = NULL;

   if (!(viewOptions = getViewOptions ()))
      return (FALSE);

   if (win = optionTabs[BLOCK_VIEW_INDEX+0].win)    /* View Name */
   {
      /* Assume the current view is that one we are looking at as this name is what it has been changed too
      ** xvt_vobj_get_title(xvt_win_get_ctl(win, VIEW_NAME_LIST), viewName, OBJECT_TEXT_LENGTH); */
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+1].win)    /* Origin */
   {
      viewOptions->originX = getFloatTextFieldValue (win, ORIGIN_X);
      viewOptions->originY = getFloatTextFieldValue (win, ORIGIN_Y);
      viewOptions->originZ = getFloatTextFieldValue (win, ORIGIN_Z);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+2].win)    /* Dimension */
   {
      viewOptions->lengthX = getFloatTextFieldValue (win, DIMENSION_X);
      viewOptions->lengthY = getFloatTextFieldValue (win, DIMENSION_Y);
      viewOptions->lengthZ = getFloatTextFieldValue (win, DIMENSION_Z);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+3].win)    /* Cube Size */
   {
      viewOptions->geologyCubeSize = getFloatTextFieldValue (win, CUBE_SIZE_GEOLOGY);
      viewOptions->geophysicsCubeSize = getFloatTextFieldValue (win, CUBE_SIZE_GEOPHYSICS);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+4].win)    /* False Origin */
   {
      projectOptions.easting = getFloatTextFieldValue (win, FALSE_ORIGIN_EASTING);
      projectOptions.northing = getFloatTextFieldValue (win, FALSE_ORIGIN_NORTHING);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+5].win)    /* Use Topography */
   {
      geologyOptions.useTopography = (int) xvt_ctl_is_checked (xvt_win_get_ctl (win, USE_TOPOGRAPHY));
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateBlockViewOptions (OPTION_TYPE type)
#else
updateBlockViewOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   BLOCK_VIEW_OPTIONS *viewOptions = NULL, *currentOptions = NULL;
   char viewName[OBJECT_TEXT_LENGTH];

   if (win = optionTabs[BLOCK_VIEW_INDEX+0].win)    /* View Name */
   {
      xvt_vobj_get_title(xvt_win_get_ctl(win, VIEW_NAME_LIST), viewName, OBJECT_TEXT_LENGTH);
      currentOptions = setViewOptions (viewName);
   }

   if (!currentOptions)
      return (FALSE);

   if (win = optionTabs[BLOCK_VIEW_INDEX+1].win)    /* Origin */
   {
      cwin = updateFloatTextField (win, ORIGIN_X, currentOptions->originX, 1);
      cwin = updateFloatTextField (win, ORIGIN_Y, currentOptions->originY, 1);
      cwin = updateFloatTextField (win, ORIGIN_Z, currentOptions->originZ, 1);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+2].win)    /* Dimension */
   {
      cwin = updateFloatTextField (win, DIMENSION_X, currentOptions->lengthX, 1);
      cwin = updateFloatTextField (win, DIMENSION_Y, currentOptions->lengthY, 1);
      cwin = updateFloatTextField (win, DIMENSION_Z, currentOptions->lengthZ, 1);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+3].win)    /* Cube Size */
   {
      updateFloatTextField (win, CUBE_SIZE_GEOLOGY, currentOptions->geologyCubeSize, 1);
      updateFloatTextField (win, CUBE_SIZE_GEOPHYSICS, currentOptions->geophysicsCubeSize, 1);
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+4].win)    /* False Origin */
   {
   }

   if (win = optionTabs[BLOCK_VIEW_INDEX+5].win)    /* Use Topography */
   {
   }

   return (TRUE);
}

/* ************************************************************* */
/* ************************* Movie ***************************** */
/* ************************************************************* */
int
#if XVT_CC_PROTO
loadMovieOptions (OPTION_TYPE type)
#else
loadMovieOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;

   if (win = optionTabs[MOVIE_INDEX+0].win)    /* Movie */
   {
      cwin = xvt_win_get_ctl (win, MOVIE_GENERATION_TYPE);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Block Diagram");
      xvt_list_add (cwin , 1, "Map (Solid Color)");
      xvt_list_add (cwin , 2, "Map (Lines)");
      xvt_list_add (cwin , 3, "Section (Solid Color)");
      xvt_list_add (cwin , 4, "Section (Lines)");
      xvt_list_add (cwin , 5, "Well Log");
      xvt_list_add (cwin , 6, "3D Strat");
      xvt_list_add (cwin , 7, "Gravity Image");
      xvt_list_add (cwin , 8, "Magnetics Image");
      xvt_list_set_sel (cwin, projectOptions.movieType, TRUE);
      xvt_list_resume (cwin);
      
      cwin = updateNumericTextField (win, MOVIE_GENERATION_FRAMES, projectOptions.movieFramesPerEvent);
      setControlFont (cwin);
   
      cwin = updateFloatTextField (win, MOVIE_REPLAY_SPEED, projectOptions.moviePlaySpeed, 1);
      setControlFont (cwin);
      
      updateScrollAndText (win, MOVIE_REPLAY_SPEED_BAR, MOVIE_REPLAY_SPEED, FALSE);
   }

   updateMovieOptions (type);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveMovieOptions (OPTION_TYPE type)
#else
saveMovieOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[MOVIE_INDEX+0].win)    /* Movie */
   {
      projectOptions.movieType = xvt_list_get_sel_index (xvt_win_get_ctl(win, MOVIE_GENERATION_TYPE));
      projectOptions.movieFramesPerEvent = getIntegerTextFieldValue (win, MOVIE_GENERATION_FRAMES);
      projectOptions.moviePlaySpeed = getFloatTextFieldValue (win, MOVIE_REPLAY_SPEED);
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateMovieOptions (OPTION_TYPE type)
#else
updateMovieOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[MOVIE_INDEX+0].win)    /* Movie */
   {
   }

   return (TRUE);
}

/* ************************************************************* */
/* ********************* Borehole/Section ********************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadSectionBoreholeOptions (OPTION_TYPE type)
#else
loadSectionBoreholeOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;

   if (win = optionTabs[BOREHOLE_SECTION_INDEX+0].win)    /* Section */
   {
      cwin = updateFloatTextField (win, SECTION_POS_X, geologyOptions.sectionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SECTION_POS_Y, geologyOptions.sectionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SECTION_POS_Z, geologyOptions.sectionZ, 1);
      setControlFont (cwin);

      cwin = updateFloatTextField (win, SECTION_DECL, geologyOptions.sectionDecl, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SECTION_LENGTH, geologyOptions.sectionLength, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SECTION_HEIGHT, geologyOptions.sectionHeight, 1);
      setControlFont (cwin);

      updateScrollAndText (win, SECTION_POS_X_BAR, SECTION_POS_X, FALSE);
      updateScrollAndText (win, SECTION_POS_Y_BAR, SECTION_POS_Y, FALSE);
      updateScrollAndText (win, SECTION_POS_Z_BAR, SECTION_POS_Z, FALSE);

      updateScrollAndText (win, SECTION_DECL_BAR, SECTION_DECL, FALSE);
      updateScrollAndText (win, SECTION_LENGTH_BAR, SECTION_LENGTH, FALSE);
      updateScrollAndText (win, SECTION_HEIGHT_BAR, SECTION_HEIGHT, FALSE);
   }

   if (win = optionTabs[BOREHOLE_SECTION_INDEX+1].win)    /* Borehole */
   {
      cwin = updateFloatTextField (win, BOREHOLE_POS_X, geologyOptions.boreholeX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, BOREHOLE_POS_Y, geologyOptions.boreholeY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, BOREHOLE_POS_Z, geologyOptions.boreholeZ, 1);
      setControlFont (cwin);

      cwin = updateFloatTextField (win, BOREHOLE_DECL, geologyOptions.boreholeDecl, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, BOREHOLE_DIP, geologyOptions.boreholeDip, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, BOREHOLE_LENGTH, geologyOptions.boreholeLength, 1);
      setControlFont (cwin);

      updateScrollAndText (win, BOREHOLE_POS_X_BAR, BOREHOLE_POS_X, FALSE);
      updateScrollAndText (win, BOREHOLE_POS_Y_BAR, BOREHOLE_POS_Y, FALSE);
      updateScrollAndText (win, BOREHOLE_POS_Z_BAR, BOREHOLE_POS_Z, FALSE);

      updateScrollAndText (win, BOREHOLE_DECL_BAR, BOREHOLE_DECL, FALSE);
      updateScrollAndText (win, BOREHOLE_DIP_BAR, BOREHOLE_DIP, FALSE);
      updateScrollAndText (win, BOREHOLE_LENGTH_BAR, BOREHOLE_LENGTH, FALSE);
   }

   updateSectionBoreholeOptions (type);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveSectionBoreholeOptions (OPTION_TYPE type)
#else
saveSectionBoreholeOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[BOREHOLE_SECTION_INDEX+0].win)    /* Section */
   {
      geologyOptions.sectionX = getFloatTextFieldValue (win, SECTION_POS_X);
      geologyOptions.sectionY = getFloatTextFieldValue (win, SECTION_POS_Y);
      geologyOptions.sectionZ = getFloatTextFieldValue (win, SECTION_POS_Z);

      geologyOptions.sectionDecl = getFloatTextFieldValue (win, SECTION_DECL);
      geologyOptions.sectionLength = getFloatTextFieldValue (win, SECTION_LENGTH);
      geologyOptions.sectionHeight = getFloatTextFieldValue (win, SECTION_HEIGHT);
   }

   if (win = optionTabs[BOREHOLE_SECTION_INDEX+1].win)    /* Borehole */
   {
      geologyOptions.boreholeX = getFloatTextFieldValue (win, BOREHOLE_POS_X);
      geologyOptions.boreholeY = getFloatTextFieldValue (win, BOREHOLE_POS_Y);
      geologyOptions.boreholeZ = getFloatTextFieldValue (win, BOREHOLE_POS_Z);

      geologyOptions.boreholeDecl = getFloatTextFieldValue (win, BOREHOLE_DECL);
      geologyOptions.boreholeDip = getFloatTextFieldValue (win, BOREHOLE_DIP);
      geologyOptions.boreholeLength = getFloatTextFieldValue (win, BOREHOLE_LENGTH);
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateSectionBoreholeOptions (OPTION_TYPE type)
#else
updateSectionBoreholeOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;

   if (win = optionTabs[BOREHOLE_SECTION_INDEX+0].win)    /* Section */
   {
   }

   if (win = optionTabs[BOREHOLE_SECTION_INDEX+1].win)    /* Borehole */
   {
   }

   return (TRUE);
}

/* ************************************************************* */
/* ***************** Volume/Surface Selection ****************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadVolumeSurfaceOptions (OPTION_TYPE type)
#else
loadVolumeSurfaceOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   char layerName[UNIT_NAME_LENGTH+OBJECT_TEXT_LENGTH+3];
   LAYER_PROPERTIES *propertiesArray[50];
   int eventNumArray[50], numStrat, i; 
   OBJECT *strat;
   
   if (win = optionTabs[VOLUME_SURFACE_INDEX+0].win)    /* Volume */
   {
   }

   if (win = optionTabs[VOLUME_SURFACE_INDEX+1].win)    /* Surface */
   {                                           /* List all the Layers that can be selected */
      assignEventNumForStratLayers (eventNumArray, 50);
      numStrat = assignPropertiesForStratLayers (propertiesArray, 50);
      cwin = xvt_win_get_ctl (win, SURFACE_SELECTION_LIST);
      xvt_list_suspend (cwin);
      for (i = 1; i < numStrat; i++)  /* Add the normal strat layers */
      {         /* note base layer not in list as cannot be seen */
         if (strat = (OBJECT *) nthObject (NULL_WIN, eventNumArray[i]))
         {
            sprintf(layerName, "%s - %s (%d)", strat->text, propertiesArray[i]->unitName, i+1);
            xvt_list_add (cwin , i-1, layerName);
                                            /* turn in on if it was on */
            if (threedViewOptions.layerOn[i])
               xvt_list_set_sel (cwin, i-1, TRUE);
            else
               xvt_list_set_sel (cwin, i-1, FALSE);
         }
      }
      for (i = 0; i < countObjects(NULL_WIN); i++)  /* Add the Fault break planes at the end */
      {
         strat = nthObject (NULL_WIN, i);
         if (strat->shape == FAULT)
         {
            sprintf(layerName, "%s - FAULT PLANE (%d)", strat->text, numStrat+1);
            xvt_list_add (cwin , numStrat-1, layerName);
                      /* turn in on if it was on */
            if (threedViewOptions.layerOn[numStrat])
               xvt_list_set_sel (cwin, numStrat-1, TRUE);
            else
               xvt_list_set_sel (cwin, numStrat-1, FALSE);
   
            numStrat++;
         }
      }
      xvt_list_resume (cwin);
                                           /* The type of surface informat to output selection window */      
      cwin = xvt_win_get_ctl (win, SURFACE_SELECTION_TYPE);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin, 0, "XYZ Color");
      xvt_list_add (cwin, 1, "Shading");
      xvt_list_add (cwin, 2, "Lines");
      xvt_list_add (cwin, 3, "File (DXF 3D Faces)");
      xvt_list_add (cwin, 4, "File (DXF Polylines)");
      xvt_list_add (cwin, 5, "File (Vulcan)");
      xvt_list_set_sel (cwin, threedViewOptions.fillType, TRUE);
      xvt_list_resume (cwin);
   }

   updateVolumeSurfaceOptions (type);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveVolumeSurfaceOptions (OPTION_TYPE type)
#else
saveVolumeSurfaceOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win, cwin;
   int i;

   if (win = optionTabs[VOLUME_SURFACE_INDEX+0].win)    /* Volume */
   {
   }

   if (win = optionTabs[VOLUME_SURFACE_INDEX+1].win)    /* Surface */
   {
      cwin = xvt_win_get_ctl (win, SURFACE_SELECTION_LIST);
      threedViewOptions.layerOn[0] = FALSE;  /* base layer can not be seen */
      for (i = 0; i < xvt_list_count_all(cwin); i++)
         threedViewOptions.layerOn[i+1] = xvt_list_is_sel (cwin, i);
   
      threedViewOptions.allLayers = xvt_ctl_is_checked (xvt_win_get_ctl (win, SURFACE_SELECTION_ALL));
      threedViewOptions.fillType = xvt_list_get_sel_index (xvt_win_get_ctl (win, SURFACE_SELECTION_TYPE));
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
updateVolumeSurfaceOptions (OPTION_TYPE type)
#else
updateVolumeSurfaceOptions (type)
OPTION_TYPE type;
#endif
{
   WINDOW win;
   int allLayers;

   if (win = optionTabs[VOLUME_SURFACE_INDEX+0].win)    /* Volume */
   {
   }

   if (win = optionTabs[VOLUME_SURFACE_INDEX+1].win)    /* Surface */
   {
      allLayers = xvt_ctl_is_checked(xvt_win_get_ctl(win, SURFACE_SELECTION_ALL));
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SURFACE_SELECTION_LIST), (BOOLEAN) (!allLayers));
   }

   return (TRUE);
}

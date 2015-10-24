#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include "scales.h"

#define DEBUG(X)    
#define TGMA_WINDOW_eh 1
                       /* External dependencies */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern ROCK_DATABASE rockDatabase;
extern char errorMessage[ERROR_MESSAGE_LENGTH];
extern int errorSet;

                                /* Functions external to this file */
#if XVT_CC_PROTO
#else
#endif
                                /* Global Functions in this file */
#if XVT_CC_PROTO
int createEventOptions (WINDOW, OBJECT *);
int loadEventOptions (WINDOW, OBJECT *);
int saveEventOptions (WINDOW, OBJECT *, int);
int updateEventOptions (WINDOW, OBJECT *);
int selectEventGroup (WINDOW, OBJECT *, int);
int displayEventHelp (OBJECT *);

void updateStratigraphyOptions (OBJECT *);
int addStratigraphy (OBJECT *);
int deleteStratigraphy (OBJECT *);
int moveStratigraphyUp (OBJECT *);
int moveStratigraphyDown (OBJECT *);
int getStratigraphyPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void setFoldValues (int *, double *, double *, double *, double *, double *);
int getFoldPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void setFaultValues (double *, double *, double *, double *, double *);
void updateFaultOptions (OBJECT *);
int getFaultPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void updateUnconformityOptions (OBJECT *);
int getUnconformityPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void setShearValues (double *, double *, double *, double *, double *);
void updateShearOptions (OBJECT *);
int getShearPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void updateDykeOptions (WINDOW);
int getDykePlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void updatePlugOptions (WINDOW);
int getPlugPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getStrainPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getTiltPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getFoliationPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getLineationPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void updateImportOptions (WINDOW, OBJECT *);
int getImportPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getGenericPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
#else
int createEventOptions ();
int loadEventOptions ();
int saveEventOptions ();
int updateEventOptions ();
int selectEventGroup ();
int displayEventHelp ();

void updateStratigraphyOptions ();
int addStratigraphy ();
int deleteStratigraphy ();
int moveStratigraphyUp ();
int moveStratigraphyDown ();
int getStratigraphyPlane ();
void setFoldValues ();
int getFoldPlane ();
void setFaultValues ();
void updateFaultOptions ();
int getFaultPlane ();
void updateUnconformityOptions ();
int getUnconformityPlane ();
void setShearValues ();
void updateShearOptions ();
int getShearPlane ();
void updateDykeOptions ();
int getDykePlane ();
void updatePlugOptions ();
int getPlugPlane ();
int getStrainPlane ();
int getTiltPlane ();
int getFoliationPlane ();
int getLineationPlane ();
void updateImportOptions ();
int getImportPlane ();
int getGenericPlane ();
#endif

                                /* Static Functions in this file */
#if XVT_CC_PROTO
static int numOptionGroups (OBJECTS);
static int loadStratigraphyOptions (OBJECT *);
static int saveStratigraphyOptions (OBJECT *);
static int loadFoldOptions (OBJECT *);
static int saveFoldOptions (OBJECT *);
static int loadFaultOptions (OBJECT *);
static int saveFaultOptions (OBJECT *, int);
static int loadUnconformityOptions (OBJECT *);
static int saveUnconformityOptions (OBJECT *);
static int loadShearOptions (OBJECT *);
static int saveShearOptions (OBJECT *);
static int loadDykeOptions (OBJECT *);
static int saveDykeOptions (OBJECT *);
static int loadPlugOptions (OBJECT *);
static int savePlugOptions (OBJECT *);
static int loadStrainOptions (OBJECT *);
static int saveStrainOptions (OBJECT *);
static int loadTiltOptions (OBJECT *);
static int saveTiltOptions (OBJECT *);
static int loadFoliationOptions (OBJECT *);
static int saveFoliationOptions (OBJECT *);
static int loadLineationOptions (OBJECT *);
static int saveLineationOptions (OBJECT *);
int loadImportOptions (OBJECT *);
static int saveImportOptions (OBJECT *);
static int loadGenericOptions (OBJECT *);
static int saveGenericOptions (OBJECT *);
#else
static int numOptionGroups ();
static int loadStratigraphyOptions ();
static int saveStratigraphyOptions ();
static int loadFoldOptions ();
static int saveFoldOptions ();
static int loadFaultOptions ();
static int saveFaultOptions ();
static int loadUnconformityOptions ();
static int saveUnconformityOptions ();
static int loadShearOptions ();
static int saveShearOptions ();
static int loadDykeOptions ();
static int saveDykeOptions ();
static int loadPlugOptions ();
static int savePlugOptions ();
static int loadStrainOptions ();
static int saveStrainOptions ();
static int loadTiltOptions ();
static int saveTiltOptions ();
static int loadFoliationOptions ();
static int saveFoliationOptions ();
static int loadLineationOptions ();
static int saveLineationOptions ();
int loadImportOptions ();
static int saveImportOptions ();
static int loadGenericOptions ();
static int saveGenericOptions ();
#endif


                       /* Global Data decared in this file */
#define MAX_OPTION_GROUPS   10


static TAB tabs[MAX_OPTION_GROUPS];

/* ======================================================================
FUNCTION        createEventOptions
DESCRIPTION
     create all the structures etc to allow editing of this events
     options in the event editing window

INPUT 
     WINDOW win;         the window of the event
     OBJECT *object;   identifies the event being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
createEventOptions (WINDOW win, OBJECT *object)
#else
createEventOptions (win, object)
WINDOW win;
OBJECT *object;
#endif
{
   WINDOW listWin;
   int group = 0, numGroups, index;
   int posX, posY, i, numIcon = 0;
   RCT posRect, iconRects[7];
   int groupXPos, groupYPos, newGroupYPos;

                        /* Zero the memory for the tabs */
   memset (tabs, 0, sizeof(TAB)*MAX_OPTION_GROUPS);

/*
   tieWin = xvt_win_get_ctl(win, EVENT_OPTION_SELECTION);
   xvt_vobj_get_outer_rect(tieWin, &posRect);
   groupYPos = posRect.top + 20; 
   groupXPos = posRect.left;
*/

#if XVTWS == MTFWS
   groupYPos = 40;
   groupXPos = 300; /* 370 */
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

   xvt_vobj_get_client_rect(win, &posRect);
   posX = posRect.right - ICON_SIZE - 4; posY = 30;
   for (i = 0; i < 7; i++)
   {
      xvt_rect_set (&(iconRects[i]), (short) posX, (short) posY,
			          (short) (posX+ICON_SIZE), (short) (posY+ICON_SIZE));
      posY += ICON_SIZE + 2;
   }
   
/*   groupYPos = posY; */
   

   switch (object->shape)
   {
      case STRATIGRAPHY:
         strcpy (tabs[group].label, "Layers");
         tabs[group].group = 0;
         tabs[group].icon = STRATIGRAPHY_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(STRATIGRAPHY_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, STRATIGRAPHY_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case FOLD:
         strcpy (tabs[group].label, "Form/Scale");
         tabs[group].group = 0;
         tabs[group].icon = FORM_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(FFOLD_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, FFOLD_WINDOW_eh, (long) object, &posRect);
         group++;

         newGroupYPos = groupYPos+xvt_rect_get_height(&posRect);
         strcpy (tabs[group].label, "Form/Scale");
         tabs[group].group = 0;
         tabs[group].icon = SCALE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(FDSCL_WINDOW,
                           win, groupXPos, newGroupYPos,
                           EM_ALL, FDSCL_WINDOW_eh, (long) object, &posRect);
         group++;

         newGroupYPos = newGroupYPos+xvt_rect_get_height(&posRect);
         strcpy (tabs[group].label, "Form/Scale");
         tabs[group].group = 0;
         tabs[group].icon = 0;
         xvt_rect_set (&(tabs[group].region), 0, 0, 0, 0);
         tabs[group].win = createPositionedWindow(TRACE_BUTTON_WINDOW,
                           win, groupXPos, newGroupYPos,
                           EM_ALL, TRACE_BUTTON_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = POSITION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(XYZ_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, XYZ_WINDOW_eh, (long) object, &posRect);
         group++;

         newGroupYPos = groupYPos+xvt_rect_get_height(&posRect);
         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(DDP_WINDOW,
                           win, groupXPos, newGroupYPos,
                           EM_ALL, DDP_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case FAULT:
         strcpy (tabs[group].label, "Form");
         tabs[group].group = 0;
         tabs[group].icon = FORM_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(GMA_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, GMA_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Form");
         tabs[group].group = 0;
         tabs[group].icon = SURFACE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(SURFACE_WINDOW,
                           win, groupXPos,
                           groupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, SURFACE_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = POSITION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(XYZ_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, XYZ_WINDOW_eh, (long) object, &posRect);
         group++;

         newGroupYPos = groupYPos+xvt_rect_get_height(&posRect);
         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(DDP_WINDOW,
                           win, groupXPos, newGroupYPos,
                           EM_ALL, DDP_WINDOW_eh, (long) object, &posRect);
         group++;

         newGroupYPos = newGroupYPos+xvt_rect_get_height(&posRect);
         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = 0;
         xvt_rect_set (&(tabs[group].region), 0, 0, 0, 0);
         tabs[group].win = createPositionedWindow(TRACE_BUTTON_WINDOW,
                           win, groupXPos, newGroupYPos,
                           EM_ALL, TRACE_BUTTON_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale");
         tabs[group].group = 2;
         tabs[group].icon = SCALE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(FSCL_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, FSCL_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale (Ellipsoid)");
         tabs[group].group = 3;
         tabs[group].icon = ELLIPSOID_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(AXIS_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, AXIS_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case UNCONFORMITY:
         strcpy (tabs[group].label, "Form");
         tabs[group].group = 0;
         tabs[group].icon = FORM_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(ALT_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, ALT_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Form");
         tabs[group].group = 0;
         tabs[group].icon = SURFACE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(SURFACE_WINDOW,
                           win, groupXPos,
                           groupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, SURFACE_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = POSITION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(XYZ_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, XYZ_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(DDD_WINDOW,
                           win, groupXPos,
                           groupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, DDD_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Layers");
         tabs[group].group = 2;
         tabs[group].icon = STRATIGRAPHY_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(STRATIGRAPHY_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, STRATIGRAPHY_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case SHEAR_ZONE:
         strcpy (tabs[group].label, "Form");
         tabs[group].group = 0;
         tabs[group].icon = FORM_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(TGMA_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, TGMA_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Form (Surface)");
         tabs[group].group = 1;
         tabs[group].icon = SURFACE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(SURFACE_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, SURFACE_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 2;
         tabs[group].icon = POSITION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(XYZ_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, XYZ_WINDOW_eh, (long) object, &posRect);
         group++;

         newGroupYPos = groupYPos+xvt_rect_get_height(&posRect);
         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 2;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(DDP_WINDOW,
                           win, groupXPos, newGroupYPos,
                           EM_ALL, DDP_WINDOW_eh, (long) object, &posRect);
         group++;

         newGroupYPos = newGroupYPos+xvt_rect_get_height(&posRect);
         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 2;
         tabs[group].icon = 0;
         xvt_rect_set (&(tabs[group].region), 0, 0, 0, 0);
         tabs[group].win = createPositionedWindow(TRACE_BUTTON_WINDOW,
                           win, groupXPos, newGroupYPos,
                           EM_ALL, TRACE_BUTTON_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale");
         tabs[group].group = 3;
         tabs[group].icon = SCALE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(SSCL_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, SSCL_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale (Ellipsoid)");
         tabs[group].group = 4;
         tabs[group].icon = ELLIPSOID_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(AXIS_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, AXIS_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case DYKE:
         strcpy (tabs[group].label, "Form");
         tabs[group].group = 0;
         tabs[group].icon = FORM_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(FDYKE_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, FDYKE_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = POSITION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(XYZ_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, XYZ_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(DDP_WINDOW,
                           win, groupXPos,
                           groupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, DDP_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale");
         tabs[group].group = 2;
         tabs[group].icon = SCALE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(SW_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, SW_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case PLUG:
         strcpy (tabs[group].label, "Form");
         tabs[group].group = 0;
         tabs[group].icon = FORM_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(FPLUG_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, FPLUG_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = POSITION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(XYZ_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, XYZ_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Position/Orientation");
         tabs[group].group = 1;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(DDP_WINDOW,
                           win, groupXPos,
                           groupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, DDP_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale");
         tabs[group].group = 2;
         tabs[group].icon = SCALE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(PSCL_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, PSCL_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale");
         tabs[group].group = 2;
         tabs[group].icon = ELLIPSOID_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(AXIS_WINDOW,
                           win, groupXPos,
                           groupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, AXIS_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case STRAIN:
         strcpy (tabs[group].label, "Matrix");
         tabs[group].group = 0;
         tabs[group].icon = STRAIN_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(MATRIX_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, MATRIX_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case TILT:
         strcpy (tabs[group].label, "Position/Orientation/Scale");
         tabs[group].group = 0;
         tabs[group].icon = POSITION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(XYZ_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, XYZ_WINDOW_eh, (long) object, &posRect);
			xvt_vobj_set_title(xvt_win_get_ctl(tabs[group].win, XYZ_GB), "Position of Rotation Axis");
         group++;

         newGroupYPos = groupYPos+xvt_rect_get_height(&posRect);
         strcpy (tabs[group].label, "Position/Orientation/Scale");
         tabs[group].group = 0;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow (PP_WINDOW,
                           win, groupXPos, newGroupYPos,
                           EM_ALL, PP_WINDOW_eh, (long) object, &posRect);
			xvt_vobj_set_title(xvt_win_get_ctl(tabs[group].win, PP_GROUPBOX), "Orientation of Rotation Axis");
         group++;

         strcpy (tabs[group].label, "Position/Orientation/Scale");
         tabs[group].group = 0;
         tabs[group].icon = SCALE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow (ROTATION_WINDOW,
                           win, groupXPos, newGroupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, ROTATION_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case FOLIATION:
         strcpy (tabs[group].label, "Orientation");
         tabs[group].group = 0;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(DDD_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, DDD_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case LINEATION:
         strcpy (tabs[group].label, "Orientation");
         tabs[group].group = 0;
         tabs[group].icon = ORIENTATION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(PP_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, PP_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case IMPORT:
         strcpy (tabs[group].label, "Form/Position");
         tabs[group].group = 0;
         tabs[group].icon = FORM_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(CORNER_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, CORNER_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Form/Position");
         tabs[group].group = 0;
         tabs[group].icon = POSITION_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(XYZ_WINDOW,
                           win, groupXPos,
                           groupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, XYZ_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale/Properties");
         tabs[group].group = 1;
         tabs[group].icon = SCALE_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(ISCL_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, ISCL_WINDOW_eh, (long) object, &posRect);
         group++;

         strcpy (tabs[group].label, "Scale/Properties");
         tabs[group].group = 1;
         tabs[group].icon = 0;
         xvt_rect_set (&(tabs[group].region), 0, 0, 0, 0);
         tabs[group].win = createPositionedWindow(IPROP_WINDOW,
                           win, groupXPos,
                           groupYPos+xvt_rect_get_height(&posRect),
                           EM_ALL, IPROP_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      case GENERIC:
         strcpy (tabs[group].label, "Options");
         tabs[group].group = 0;
         tabs[group].icon = GENERIC_ICON;
         memcpy ((char *) &(tabs[group].region), (char *) &(iconRects[numIcon++]), sizeof(RCT));
         tabs[group].win = createPositionedWindow(GENERIC_WINDOW,
                           win, groupXPos, groupYPos,
                           EM_ALL, GENERIC_WINDOW_eh, (long) object, &posRect);
         group++;
         break;
      default:
         strcpy (tabs[group].label, "Options");
         tabs[group].group = 0;
         tabs[group].icon = 0;
         xvt_rect_set (&(tabs[group].region), 0, 0, 0, 0);
         tabs[group].win = NULL_WIN;
         group++;
   }
   numGroups = group;

                        /* Add the tabs to a selection list */
   listWin = xvt_win_get_ctl (win, EVENT_OPTION_SELECTION);
   xvt_list_suspend (listWin);
   for (group = 0, index = 0; group < numGroups; group++)
   {
      if ((group < 1) || (tabs[group-1].group != tabs[group].group))
      {
         xvt_list_add (listWin, index, tabs[group].label);
         index++;
      }
   }
   xvt_list_resume (listWin);

                 /* Fill the windows with current values */
   loadEventOptions (win, object);

                 /* Start by showing the last group changed */
   selectEventGroup (win, object, object->optionGroup);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        eventOptionsMouseUp
DESCRIPTION
     load the selected paramaters from the options structure for
     that particular event

INPUT 
     WINDOW win;         the window of the event
     OBJECT *object;   identifies the event being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
eventOptionsMouseUp (WINDOW win, struct s_mouse *mouse)
#else
eventOptionsMouseUp (win, mouse)
WINDOW win;
struct s_mouse *mouse;
#endif
{
   int index;
   
   for (index = 0; index < MAX_OPTION_GROUPS; index++)
   {
      if (!tabs[index].win)  /* finished all this event has so skip out */
         break;
         
      if (xvt_rect_has_point (&(tabs[index].region), mouse->where))
      {
         selectEventGroup (win, (OBJECT *) xvt_vobj_get_data(win), tabs[index].group);
         break;
      }
   }
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        loadEventOptions
DESCRIPTION
     load the selected paramaters from the options structure for
     that particular event

INPUT 
     WINDOW win;         the window of the event
     OBJECT *object;   identifies the event being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
loadEventOptions (WINDOW win, OBJECT *object)
#else
loadEventOptions (win, object)
WINDOW win;
OBJECT *object;
#endif
{
   char text[100];
   WINDOW cwin;
   OBJECT *nextObject = NULL, *prevObject = NULL;
                          /* set the name of the event */
   cwin = xvt_win_get_ctl (win, EVENT_NAME);
   setControlFont (cwin);
   getObjectName (object, text);
   xvt_vobj_set_title (cwin, text);

                          /* enable appropriate Next & Prev Event buttons */
   cwin = xvt_win_get_ctl (win, EVENT_NEXT);
   for (nextObject = object->next; nextObject && (nextObject->shape == STOP); nextObject = nextObject->next)
      ;
   if (nextObject)   /* There is an object to move to so enable button */
      xvt_vobj_set_enabled (cwin, TRUE);
   else
      xvt_vobj_set_enabled (cwin, FALSE);
      
   cwin = xvt_win_get_ctl (win, EVENT_PREVIOUS);
   if (object->shape == STRATIGRAPHY)  /* Strat must be the first event */
      xvt_vobj_set_enabled (cwin, FALSE);
   else
      xvt_vobj_set_enabled (cwin, TRUE);

   switch (object->shape)
   {
      case STRATIGRAPHY:
         xvt_vobj_set_title (win, "Stratigraphy");
         loadStratigraphyOptions (object);
         break;
      case FOLD:
         xvt_vobj_set_title (win, "Fold");
         loadFoldOptions (object);
         break;
      case FAULT:
         xvt_vobj_set_title (win, "Fault");
         loadFaultOptions (object);
         break;
      case UNCONFORMITY:
         xvt_vobj_set_title (win, "Unconformity");
         loadUnconformityOptions (object);
         break;
      case SHEAR_ZONE:
         xvt_vobj_set_title (win, "Shear Zone");
         loadShearOptions (object);
         break;
      case DYKE:
         xvt_vobj_set_title (win, "Dyke");
         loadDykeOptions (object);
         break;
      case PLUG:
         xvt_vobj_set_title (win, "Plug");
         loadPlugOptions (object);
         break;
      case STRAIN:
         xvt_vobj_set_title (win, "Strain");
         loadStrainOptions (object);
         break;
      case TILT:
         xvt_vobj_set_title (win, "Tilt");
         loadTiltOptions (object);
         break;
      case FOLIATION:
         xvt_vobj_set_title (win, "Foliation");
         loadFoliationOptions (object);
         break;
      case LINEATION:
         xvt_vobj_set_title (win, "Lineation");
         loadLineationOptions (object);
         break;
      case IMPORT:
         xvt_vobj_set_title (win, "Import");
         loadImportOptions (object);
         updateImportOptions (win, object);
         break;
      case GENERIC:
         xvt_vobj_set_title (win, "Generic");
         loadGenericOptions (object);
         break;
   }
      /* save the options in the window so they can be restored **
      ** if cancel is pressed to exit the object */
   copyOptions (EVENT_WINDOW, object, OPTIONS_SAVE);

   return (TRUE);
}

/* ======================================================================
FUNCTION        saveEventOptions
DESCRIPTION
     store the selected paramaters into the options structure for
     that particular event

INPUT 
     WINDOW win;         the window of the event
     OBJECT *object;   identifies the event being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
saveEventOptions (WINDOW win, OBJECT *object, int previewOnly)
#else
saveEventOptions (win, object, previewOnly)
WINDOW win;
OBJECT *object;
int previewOnly;
#endif
{
   char text[100];
   WINDOW cwin;
   int result = TRUE;
   
                         /* get the name of the event */
   if (!previewOnly)
   { 
      cwin = xvt_win_get_ctl (win, EVENT_NAME);
      xvt_vobj_get_title (cwin, text, 100);
      setObjectName (object, text);
   }

   switch (object->shape)
   {
      case STRATIGRAPHY:
         result = saveStratigraphyOptions (object);
         break;
      case FOLD:
         result = saveFoldOptions (object);
         break;
      case FAULT:
         result = saveFaultOptions (object, previewOnly);
         break;
      case UNCONFORMITY:
         result = saveUnconformityOptions (object);
         break;
      case SHEAR_ZONE:
         result = saveShearOptions (object);
         break;
      case DYKE:
         result = saveDykeOptions (object);
         break;
      case PLUG:
         result = savePlugOptions (object);
         break;
      case STRAIN:
         result = saveStrainOptions (object);
         break;
      case TILT:
         result = saveTiltOptions (object);
         break;
      case FOLIATION:
         result = saveFoliationOptions (object);
         break;
      case LINEATION:
         result = saveLineationOptions (object);
         break;
      case IMPORT:
         result = saveImportOptions (object);
         break;
      case GENERIC:
         result = saveGenericOptions (object);
         break;
   }

   if (!previewOnly)
      object->newObject = FALSE;

   return (result);
}

/* ======================================================================
FUNCTION        updateEventOptions
DESCRIPTION
     Update the event options window display

INPUT 
     WINDOW win;         the window of the event
     OBJECT *object;     the output file for the block

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
updateEventOptions (WINDOW win, OBJECT *object)
#else
updateEventOptions (win, object)
WINDOW win;
OBJECT *object;
#endif
{
   OPTION_GROUPS currentGroup;
   int tabIndex = -1, index;
   PNT lines[3];
   DRAW_CTOOLS tools;
   RCT rect;
   
   currentGroup = object->optionGroup;

        /* Draw the icons showing which group has been selected */
   for (index = 0; index < MAX_OPTION_GROUPS; index++)
   {
      if (!tabs[index].win)  /* finished all this event has so skip out */
         break;

                /* High light the current group */
      if (currentGroup == tabs[index].group)
         xvt_dwin_set_fore_color (win, COLOR_LTGRAY);
      else     /* Draw the icons for the other groups */
         xvt_dwin_set_fore_color (win, COLOR_BLACK);
      
      if (tabs[index].icon)
         xvt_dwin_draw_icon (win, tabs[index].region.left,
                                  tabs[index].region.top,
                                  tabs[index].icon);
   }
   
   /* Draw a line from the top of the Icons to the list box of options */
   xvt_dwin_set_std_cpen (win, TL_PEN_BLACK);
   xvt_dwin_get_draw_ctools (win, &tools);
   tools.pen.width = 5;
   xvt_dwin_set_draw_ctools (win, &tools);
   
   xvt_vobj_get_outer_rect (xvt_win_get_ctl (win, EVENT_OPTION_SELECTION), &rect);
   lines[0].h = tabs[0].region.left + (tabs[0].region.right - tabs[0].region.left)/2;
   lines[0].v = tabs[0].region.top - 5;
   lines[1].h = lines[0].h;
   lines[1].v = rect.top + 10;
   lines[2].h = rect.right + 5;
   lines[2].v = lines[1].v;
   
   xvt_dwin_draw_polyline (win, lines, 3);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        selectEventGroup
DESCRIPTION
     Update the event options window to display current group

INPUT 
     WINDOW win;         the window of the event
     OBJECT *object;     the output file for the block

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
selectEventGroup (WINDOW win, OBJECT *object, int newIndex)
#else
selectEventGroup (win, object, newIndex)
WINDOW win;
OBJECT *object;
int newIndex;
#endif
{
   int group, index;
   WINDOW listWin;

   listWin = xvt_win_get_ctl (win, EVENT_OPTION_SELECTION);

                        /* remove any currently showing */
   for (index = 0; index < MAX_OPTION_GROUPS; index++)
   {
      if (tabs[index].win)
         xvt_vobj_set_visible (tabs[index].win, FALSE);
   }
   
                      /* show the one that was selected  */
                      /* this may be more than one panel */
   for (group = -1, index = 0; index < MAX_OPTION_GROUPS; index++)
   {
      if ((index < 1) || (tabs[index-1].group != tabs[index].group))
         group++;

      if ((group == newIndex) && tabs[index].win)
      {
         xvt_vobj_set_visible (tabs[index].win, TRUE);
#if (XVTWS == MTFWS) || (XVTWS == MACWS) || (XVTWS == XOLWS)
#else
         bringWindowToFront(tabs[index].win);
#endif
                        /* make sure we showing correct selection */
         if (listWin)
            xvt_list_set_sel (listWin, group, TRUE);

         object->optionGroup = newIndex;
      }
   }
   
#if (XVTWS == MTFWS) || (XVTWS == MACWS) || (XVTWS == XOLWS)
#else
   if (listWin)
      xvt_scr_set_focus_vobj (listWin);
#endif

   updateEventOptions (win, object);
   if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY))
      update3dPreview (NULL_WIN, object);

   return (TRUE);
}

/* ======================================================================
FUNCTION        displayEventHelp
DESCRIPTION
     Display the right help page for the event optoins window

INPUT 
     OBJECT *object;   identifies the event being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
displayEventHelp (OBJECT *object)
#else
displayEventHelp (object)
OBJECT *object;
#endif
{
   switch (object->shape)
   {
      case STRATIGRAPHY:
			displayHelp("BaseEv.htm");
         break;
      case FOLD:
			displayHelp("folds.htm");
         break;
      case FAULT:
			displayHelp("faults.htm");
         break;
      case UNCONFORMITY:
			displayHelp("unc.htm");
         break;
      case SHEAR_ZONE:
			displayHelp("sz.htm");
         break;
      case DYKE:
			displayHelp("dyke.htm");
         break;
      case PLUG:
			displayHelp("plug.htm");
         break;
      case STRAIN:
			displayHelp("strain.htm");
         break;
      case TILT:
			displayHelp("tilt.htm");
         break;
      case FOLIATION:
			displayHelp("foln.htm");
         break;
      case LINEATION:
			displayHelp("lin.htm");
         break;
      case IMPORT:
			displayHelp("import.htm");
         break;
      case GENERIC:
			displayHelp("generic.htm");
         break;
      default:
			displayHelp("toc.htm");
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        createEventOptions
DESCRIPTION
     create all the structures etc to allow editing of this events
     options in the event editing window

INPUT 
     WINDOW win;         the window of the event
     OBJECT *object;   identifies the event being edited

OUTPUT

RETURNED
   TRUE - sucess, FALSE - error
====================================================================== */
static int
#if XVT_CC_PROTO
numOptionGroups (OBJECTS shape)
#else
numOptionGroups (shape)
OBJECTS shape;
#endif
{
   int numGroups = 0;
   
   switch (shape)
   {
      case STRATIGRAPHY:     numGroups = 1;     break;
      case FOLD:             numGroups = 5;     break;
      case FAULT:            numGroups = 5;     break;
      case UNCONFORMITY:     numGroups = 5;     break;
      case SHEAR_ZONE:       numGroups = 5;     break;
      case DYKE:             numGroups = 5;     break;
      case PLUG:             numGroups = 5;     break;
      case TILT:             numGroups = 5;     break;
      case STRAIN:           numGroups = 1;     break;
      case FOLIATION:        numGroups = 1;     break;
      case LINEATION:        numGroups = 1;     break;
      case IMPORT:           numGroups = 3;     break;
      case GENERIC:          numGroups = 1;     break;
   }
                   /* dont want to overflow any arrays */
   if (numGroups > MAX_OPTION_GROUPS)
      numGroups = MAX_OPTION_GROUPS;
      
   return (numGroups);
}


/* ************************************************************* */
/* *********************** Stratigraphy ************************ */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadStratigraphyOptions (OBJECT *object)
#else
loadStratigraphyOptions (object)
OBJECT *object;
#endif
{
   updateStratigraphyOptions (object);
   
   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveStratigraphyOptions (OBJECT *object)
#else
saveStratigraphyOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   STRATIGRAPHY_OPTIONS *options = (STRATIGRAPHY_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, STRATIGRAPHY_LIST);
      options->numLayers = xvt_list_count_all (cwin);
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        updateStratigraphyOptions
DESCRIPTION
        make sure the correct options are enabled
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
updateStratigraphyOptions (OBJECT *object)
#else
updateStratigraphyOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   STRATIGRAPHY_OPTIONS *options = getStratigraphyOptionsStructure (object);
   int numLayers = 0, layer;
   int selIndex;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, STRATIGRAPHY_LIST);
      selIndex = xvt_list_get_sel_index (cwin);  /* get so it wont change */
      xvt_list_suspend (cwin);
      xvt_list_clear (cwin);
      for (layer = options->numLayers-1; layer >= 0; layer--)
      {
         xvt_list_add (cwin, -1, options->properties[layer].unitName);
      }
      xvt_list_resume (cwin);

      if (selIndex >= 0)
         xvt_list_set_sel (cwin, selIndex, TRUE); /* set so it wont change */
   }

   update3dPreview (NULL_WIN, object);
}

int
#if XVT_CC_PROTO
addStratigraphy (OBJECT *object)
#else
addStratigraphy (object)
OBJECT *object;
#endif
{
#define DEFAULT_STRAT_WIDTH 1000.0
   WINDOW win, cwin;
   STRATIGRAPHY_OPTIONS *options = getStratigraphyOptionsStructure (object);
   int selIndex, index, total, tabIndex, numLayers;
   int layer;
   LAYER_PROPERTIES *newProps;
   
   if (object->shape == STRATIGRAPHY)
      tabIndex = 0;
   else if (object->shape == UNCONFORMITY)
      tabIndex = 4;

   if (win = tabs[tabIndex].win)
   {
      cwin = xvt_win_get_ctl (win, STRATIGRAPHY_LIST);
      total = xvt_list_count_all (cwin);
      if ((selIndex = xvt_list_get_sel_index (cwin)) == -1)
         return (FALSE);
      index = total - selIndex - 1;

      numLayers = options->numLayers + 1;      
      if (!(newProps = (LAYER_PROPERTIES *) xvt_mem_zalloc (numLayers*sizeof (LAYER_PROPERTIES))))
         return (FALSE);
         
                  /* Copy those before the inserted stratigraphy */
      memcpy (newProps, options->properties, index*sizeof(LAYER_PROPERTIES));
                  /* Initialise the new Strat */
                  /* give characteristics of the layer inserted under */
      memcpy (&(newProps[index]), &(options->properties[index]), sizeof(LAYER_PROPERTIES));
                  /* Copy those after the inserted stratigraphy */
      memcpy (&(newProps[index+1]), &(options->properties[index]),
                                   (total-index)*sizeof(LAYER_PROPERTIES));

                  /* change height and name */
      sprintf (newProps[index].unitName, "Layer %d", index+1);
      newProps[index].color.red = 0;
      newProps[index].color.green = 0;
      newProps[index].color.blue = 0;

                  /* Move all the layers by new layers size */
			for (layer = index; layer >= 0; layer--)
				newProps[layer].height -= (int) DEFAULT_STRAT_WIDTH;

      xvt_mem_free ((char *) options->properties);
      options->properties = newProps;
      options->numLayers = numLayers;

		if (object->shape == STRATIGRAPHY)
			updateStratigraphyOptions (object);
		else if (object->shape == UNCONFORMITY)
			updateUnconformityOptions (object);

		xvt_list_set_sel (cwin, selIndex+1, TRUE);  /* keep item selected */
   }
   else
	{
		if (object->shape == STRATIGRAPHY)
			updateStratigraphyOptions (object);
		else if (object->shape == UNCONFORMITY)
			updateUnconformityOptions (object);
	}

   return (TRUE);
}

int
#if XVT_CC_PROTO
deleteStratigraphy (OBJECT *object)
#else
deleteStratigraphy (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   STRATIGRAPHY_OPTIONS *options = getStratigraphyOptionsStructure (object);
   int selIndex, index, total, tabIndex, numLayers, layer;
   LAYER_PROPERTIES *newProps;
   double diffHeight;

   if (options->numLayers < 2) /* must have 2 to delete one */
      return (FALSE);

   if (object->shape == STRATIGRAPHY)
      tabIndex = 0;
   else if (object->shape == UNCONFORMITY)
      tabIndex = 4;

   if (win = tabs[tabIndex].win)
   {
      cwin = xvt_win_get_ctl (win, STRATIGRAPHY_LIST);
      total = xvt_list_count_all (cwin);
      if ((selIndex = xvt_list_get_sel_index (cwin)) == -1)
         return (FALSE);
      index = total - selIndex - 1;
      
      numLayers = options->numLayers - 1;      
      if (!(newProps = (LAYER_PROPERTIES *) xvt_mem_zalloc (numLayers*sizeof (LAYER_PROPERTIES))))
      {
         xvt_dm_post_error ("Error, Could Not Delete Layer.");
         return (FALSE);
      }
         
                  /* Copy those before the deleted stratigraphy */
      memcpy (newProps, options->properties,
                                 index*sizeof(LAYER_PROPERTIES));
                  /* Copy those after the deleted stratigraphy */
      memcpy (&(newProps[index]), &(options->properties[index+1]),
                                 (total-index-1)*sizeof(LAYER_PROPERTIES));

      if (index >= total-1)
			diffHeight = options->properties[index].height
                                      - options->properties[index-1].height;
		else
			diffHeight = options->properties[index+1].height
                                      - options->properties[index].height;

                  /* Move all the layers by new layers size */
      for (layer = index-1; layer > 0; layer--)
         newProps[layer].height += (int) diffHeight;

      xvt_mem_free ((char *) options->properties);
      options->properties = newProps;
      options->numLayers = numLayers;

      if (selIndex == total-1) /* keep last item selected */
         xvt_list_set_sel (cwin, selIndex-1, TRUE);
      else
         object->generalData--;  /* Which element is selected */   
         
      if (total == 2)  /* Now only one as one of those was just deleted */
         options->properties[0].height = 0;    /* Set height to zero */
   }                                     

   if (object->shape == STRATIGRAPHY)
      updateStratigraphyOptions (object);
   else if (object->shape == UNCONFORMITY)
      updateUnconformityOptions (object);

   return (TRUE);
}

int
#if XVT_CC_PROTO
moveStratigraphyUp (OBJECT *object)
#else
moveStratigraphyUp (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   STRATIGRAPHY_OPTIONS *options = getStratigraphyOptionsStructure (object);
   LAYER_PROPERTIES tempProp;
   int selIndex, index, total, tempHeight, tabIndex;
   double widthAbove, widthBelow;

   if (object->shape == STRATIGRAPHY)
      tabIndex = 0;
   else if (object->shape == UNCONFORMITY)
      tabIndex = 4;

   if (win = tabs[tabIndex].win)
   {
      cwin = xvt_win_get_ctl (win, STRATIGRAPHY_LIST);
      total = xvt_list_count_all (cwin);
      if ((selIndex = xvt_list_get_sel_index (cwin)) == -1)
         return (FALSE);
      index = total - selIndex - 1;
      if (index > options->numLayers-2) /* first one cannot be moved up */
         return (FALSE);

      if (index <= 0)
         widthBelow = options->properties[index+2].height  /* Use height above to preserve it */
                                  - options->properties[index+1].height; /* DEFAULT_STRAT_WIDTH */
      else
         widthBelow = options->properties[index+1].height
                                  - options->properties[index].height;

      if (index+2 >= options->numLayers-1)
         widthAbove = widthBelow; /* DEFAULT_STRAT_WIDTH; */
      else
         widthAbove = options->properties[index+2].height
                                  - options->properties[index+1].height;

                 /* Copy layer in top position currently */
      memcpy (&tempProp, &(options->properties[index+1]), sizeof(LAYER_PROPERTIES));
                 
                 /* Move Below layer to top position */
      tempHeight = options->properties[index+1].height;
      memcpy (&(options->properties[index+1]), &(options->properties[index]), sizeof(LAYER_PROPERTIES));
      if (index+1 == options->numLayers-1)
         options->properties[index+1].height = tempHeight;
      else
         options->properties[index+1].height = options->properties[index+2].height
                                                                - (int) widthBelow;

      memcpy (&(options->properties[index]), &tempProp, sizeof(LAYER_PROPERTIES));
      options->properties[index].height = options->properties[index+1].height
                                                           - (int) widthAbove;
      
      xvt_list_set_sel (cwin, selIndex-1, TRUE);  /* keep item selected */
      object->generalData++;
   }

   if (object->shape == STRATIGRAPHY)
      updateStratigraphyOptions (object);
   else if (object->shape == UNCONFORMITY)
      updateUnconformityOptions (object);

   return (TRUE);
}

int
#if XVT_CC_PROTO
moveStratigraphyDown (OBJECT *object)
#else
moveStratigraphyDown (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   STRATIGRAPHY_OPTIONS *options = getStratigraphyOptionsStructure (object);
   LAYER_PROPERTIES tempProp;
   int selIndex, index, total, tempHeight, tabIndex;
   double widthAbove, widthBelow;
   
   if (object->shape == STRATIGRAPHY)
      tabIndex = 0;
   else if (object->shape == UNCONFORMITY)
      tabIndex = 4;

   if (win = tabs[tabIndex].win)
   {
      cwin = xvt_win_get_ctl (win, STRATIGRAPHY_LIST);
      total = xvt_list_count_all (cwin);
      selIndex = xvt_list_get_sel_index (cwin);
      index = total - selIndex - 1;
      if (index < 1)  /* last one cannot be moved down */
         return (FALSE);
      
      if (index+1 > options->numLayers-1)
         widthAbove = options->properties[index].height   /* Use width Below to preserve tichness */
                                  - options->properties[index-1].height;  /* DEFAULT_STRAT_WIDTH; */
      else
         widthAbove = options->properties[index+1].height
                                  - options->properties[index].height;

      if (index <= 1)
         widthBelow = widthAbove; /* DEFAULT_STRAT_WIDTH; */
      else
         widthBelow = options->properties[index].height
                                  - options->properties[index-1].height;                                  

      memcpy (&tempProp, &(options->properties[index]), sizeof(LAYER_PROPERTIES));

      tempHeight = options->properties[index].height;
      memcpy (&(options->properties[index]), &(options->properties[index-1]), sizeof(LAYER_PROPERTIES));
      if (index == options->numLayers-1)  /* Top */
         options->properties[index].height = tempHeight;
      else
         options->properties[index].height = options->properties[index+1].height
                                                              - (int) widthBelow;
      
      memcpy (&(options->properties[index-1]), &tempProp, sizeof(LAYER_PROPERTIES));
      options->properties[index-1].height = options->properties[index].height
                                                              - (int) widthAbove;

      xvt_list_set_sel (cwin, selIndex+1, TRUE);  /* keep item selected */
      object->generalData--;
   }

   if (object->shape == STRATIGRAPHY)
      updateStratigraphyOptions (object);
   else if (object->shape == UNCONFORMITY)
      updateUnconformityOptions (object);

   return (TRUE);
}

int
#if XVT_CC_PROTO
getStratigraphyPlane (OBJECT *object,
                      double *x,   double *y,      double *z,
                      double *dip, double *dipDir, double *pitch)
#else
getStratigraphyPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   return (FALSE);
}


/* ************************************************************* */
/* ************************** Fold ***************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadFoldOptions (OBJECT *object)
#else
loadFoldOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   FOLD_OPTIONS *options = (FOLD_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, FFORM_TYPE);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Sine Fold");
      xvt_list_add (cwin , 1, "User Defined");
      xvt_list_resume (cwin);
      if (options->type > 0)
         xvt_list_set_sel (cwin, 1, TRUE);
      else
         xvt_list_set_sel (cwin, 0, TRUE);

      cwin = xvt_win_get_ctl (win, FFORM_SINGLE);
      xvt_ctl_set_checked(cwin, (BOOLEAN) options->singleFold);
   }

   if (win = tabs[1].win)
   {
      cwin = updateFloatTextField (win, FDSCL_WAVE, options->wavelength, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FDSCL_AMPLITUDE, options->amplitude, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FDSCL_CYCL, options->cycle, 1);
      setControlFont (cwin);

      updateScrollAndText(win, FDSCL_WAVE_BAR,      FDSCL_WAVE,      FALSE);
      updateScrollAndText(win, FDSCL_AMPLITUDE_BAR, FDSCL_AMPLITUDE, FALSE);
      updateScrollAndText(win, FDSCL_CYCL_BAR,      FDSCL_CYCL,      FALSE);
   }

   if (win = tabs[3].win)
   {
      cwin = updateFloatTextField (win, XYZ_X, options->positionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Y, options->positionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Z, options->positionZ, 1);
      setControlFont (cwin);
      
      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }

   if (win = tabs[4].win)
   {
      cwin = updateFloatTextField (win, DDP_DDIR, options->dipDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_DIP, options->dip, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_PITCH, options->axisPitch, 1);
      setControlFont (cwin);

      updateScrollAndText(win, DDP_DDIR_BAR,  DDP_DDIR,  FALSE);
      updateScrollAndText(win, DDP_DIP_BAR,   DDP_DIP,   FALSE);
      updateScrollAndText(win, DDP_PITCH_BAR, DDP_PITCH, FALSE);
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveFoldOptions (OBJECT *object)
#else
saveFoldOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   FOLD_OPTIONS *options = (FOLD_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, FFORM_TYPE);
      options->type = xvt_list_get_sel_index (cwin);
      if (options->type == BIOT_FOLD)
         options->type = FOURIER_FOLD;

      cwin = xvt_win_get_ctl (win, FFORM_SINGLE);
      options->singleFold = xvt_ctl_is_checked(cwin);
   }

   if (win = tabs[1].win)
   {
      options->wavelength = getFloatTextFieldValue (win, FDSCL_WAVE);
      options->amplitude = getFloatTextFieldValue (win, FDSCL_AMPLITUDE);
      options->cycle = getFloatTextFieldValue (win, FDSCL_CYCL);
   }

   if (win = tabs[3].win)
   {
      options->positionX = getFloatTextFieldValue (win, XYZ_X);
      options->positionY = getFloatTextFieldValue (win, XYZ_Y);
      options->positionZ = getFloatTextFieldValue (win, XYZ_Z);
   }

   if (win = tabs[4].win)
   {
      options->dipDirection = getFloatTextFieldValue (win, DDP_DDIR);
      options->dip = getFloatTextFieldValue (win, DDP_DIP);
      options->axisPitch = getFloatTextFieldValue (win, DDP_PITCH);
   }

                      /* calculate and store the forward and
                      ** reverse rotation matricies */
   convrt(options->dip, options->dipDirection,
                        options->axisPitch, TRUE);
   rotset(options->dip, options->dipDirection,
                        options->axisPitch,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);

   return (TRUE);
}

/* ======================================================================
FUNCTION        setFoldValues
DESCRIPTION
        set the values that are not NULL on the windows
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
setFoldValues (int *foldType, double *waveLength, double *amplitude,
                              double *xPos, double *yPos, double *zPos)
#else
setFoldValues (foldType, waveLength, amplitude, xPos, yPos, zPos)
int *foldType;
double *waveLength, *amplitude, *xPos, *yPos, *zPos;
#endif
{
   WINDOW win, cwin;

   if (win = tabs[0].win)
   {
      if (foldType)
      {
         cwin = xvt_win_get_ctl (win, FFORM_TYPE);
         if (*foldType > 0)
            xvt_list_set_sel (cwin, 1, TRUE);
         else
            xvt_list_set_sel (cwin, 0, TRUE);
      }
   }

   if (win = tabs[1].win)
   {
      if (waveLength)
         updateFloatTextField (win, FDSCL_WAVE, *waveLength, 1);
      if (amplitude)
         updateFloatTextField (win, FDSCL_AMPLITUDE, *amplitude, 1);

      updateScrollAndText(win, FDSCL_WAVE_BAR,      FDSCL_WAVE,      FALSE);
      updateScrollAndText(win, FDSCL_AMPLITUDE_BAR, FDSCL_AMPLITUDE, FALSE);
   }

   if (win = tabs[3].win)
   {
      if (xPos)
         updateFloatTextField (win, XYZ_X, *xPos, 1);
      if (yPos)
         updateFloatTextField (win, XYZ_Y, *yPos, 1);
      if (zPos)
         updateFloatTextField (win, XYZ_Z, *zPos, 1);

      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }
}

int
#if XVT_CC_PROTO
getFoldPlane (OBJECT *object,
              double *x,   double *y,      double *z,
              double *dip, double *dipDir, double *pitch)
#else
getFoldPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   FOLD_OPTIONS *options = (FOLD_OPTIONS *) object->options;

   if (win = tabs[3].win)
   {
      *x = getFloatTextFieldValue (win, XYZ_X);
      *y = getFloatTextFieldValue (win, XYZ_Y);
      *z = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[4].win)
   {
      *dip = getFloatTextFieldValue (win, DDP_DIP);
      *dipDir = getFloatTextFieldValue (win, DDP_DDIR);
      *pitch = getFloatTextFieldValue (win, DDP_PITCH);
   }
   
   return (TRUE);
}

int
#if XVT_CC_PROTO
isProfileReadOnly (OBJECT *object)
#else
isProfileReadOnly (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   int value;
   
   if (object->shape == FOLD)
   {
      if (win = tabs[0].win)
      {
         cwin = xvt_win_get_ctl (win, FFORM_TYPE);
         value = xvt_list_get_sel_index (cwin);
         if (value == 0)
            return (TRUE);
         else
            return (FALSE);
      }
   }
   
   return (FALSE);
}

/* ************************************************************* */
/* *************************** Fault *************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadFaultOptions (OBJECT *object)
#else
loadFaultOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, GMA_GEOMETRY);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Translation");
      xvt_list_add (cwin , 1, "Rotation");
      xvt_list_add (cwin , 2, "Elliptical");
      xvt_list_add (cwin , 3, "Curved");
      xvt_list_add (cwin , 4, "Ring");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->geometry, TRUE);

      cwin = xvt_win_get_ctl (win, GMA_MOVEMENT);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Hanging Wall");
      xvt_list_add (cwin , 1, "Foot Wall");
      xvt_list_add (cwin , 2, "Both");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->movement, TRUE);

      cwin = xvt_win_get_ctl (win, GMA_ALTERATION);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "None");
      xvt_list_add (cwin , 1, "Hanging Wall");
      xvt_list_add (cwin , 2, "Foot Wall");
      xvt_list_add (cwin , 3, "Both");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->alterationZones, TRUE);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, GMA_ALTERATION_EDIT),
                         (BOOLEAN) options->alterationZones);
   }
   
   if (win = tabs[1].win)
   {
      cwin = xvt_win_get_ctl (win, SURFACE_TYPE_LIST);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Flat Plane");
      xvt_list_add (cwin , 1, "DXF Surface");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->surfaceType, TRUE);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_VIEW),
                         (BOOLEAN) options->alterationZones);
      cwin = updateFloatTextField (win, SURFACE_XDIM, options->surfaceXDim, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SURFACE_YDIM, options->surfaceYDim, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SURFACE_ZDIM, options->surfaceZDim, 1);
      setControlFont (cwin);

      updateScrollAndText(win, SURFACE_XDIM_BAR, SURFACE_XDIM, FALSE);
      updateScrollAndText(win, SURFACE_YDIM_BAR, SURFACE_YDIM, FALSE);
      updateScrollAndText(win, SURFACE_ZDIM_BAR, SURFACE_ZDIM, FALSE);
   }
   
   if (win = tabs[2].win)
   {
      cwin = updateFloatTextField (win, XYZ_X, options->positionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Y, options->positionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Z, options->positionZ, 1);
      setControlFont (cwin);
      
      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }
   
   if (win = tabs[3].win)
   {
      cwin = updateFloatTextField (win, DDP_DDIR, options->dipDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_DIP, options->dip, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_PITCH, options->pitch, 1);
      setControlFont (cwin);

      updateScrollAndText(win, DDP_DDIR_BAR,  DDP_DDIR,  FALSE);
      updateScrollAndText(win, DDP_DIP_BAR,   DDP_DIP,   FALSE);
      updateScrollAndText(win, DDP_PITCH_BAR, DDP_PITCH, FALSE);
   }

   if (win = tabs[5].win)
   {
      cwin = updateFloatTextField (win, FSCL_ROTATION, options->rotation, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FSCL_SLIP, options->slip, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FSCL_AMPLITUDE, options->amplitude, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FSCL_RADIUS, options->radius, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FSCL_CYL, options->cylindricalIndex, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, FSCL_PP, options->profilePitch, 1);
      setControlFont (cwin);

      updateScrollAndText(win, FSCL_ROTATION_BAR,  FSCL_ROTATION, FALSE);
      updateScrollAndText(win, FSCL_SLIP_BAR,      FSCL_SLIP,     FALSE);
      updateScrollAndText(win, FSCL_AMPLITUDE_BAR, FSCL_AMPLITUDE,FALSE);
      updateScrollAndText(win, FSCL_RADIUS_BAR,    FSCL_RADIUS,   FALSE);
      updateScrollAndText(win, FSCL_CYL_BAR,       FSCL_CYL,      FALSE);
      updateScrollAndText(win, FSCL_PP_BAR,        FSCL_PP,       FALSE);
   }

   if (win = tabs[6].win)
   {
      cwin = updateFloatTextField (win, AXIS_X, options->xAxis, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, AXIS_Y, options->yAxis, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, AXIS_Z, options->zAxis, 1);
      setControlFont (cwin);

      updateScrollAndText(win, AXIS_X_BAR, AXIS_X, FALSE);
      updateScrollAndText(win, AXIS_Y_BAR, AXIS_Y, FALSE);
      updateScrollAndText(win, AXIS_Z_BAR, AXIS_Z, FALSE);
   }

   updateFaultOptions (object);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveFaultOptions (OBJECT *object, int previewOnly)
#else
saveFaultOptions (object, previewOnly)
OBJECT *object;
int previewOnly;
#endif
{
   WINDOW win, cwin;
   FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;
   double pitch, dip;
   int result = TRUE;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, GMA_GEOMETRY);
      options->geometry = xvt_list_get_sel_index (cwin);

      cwin = xvt_win_get_ctl (win, GMA_MOVEMENT);
      options->movement = xvt_list_get_sel_index (cwin);

      cwin = xvt_win_get_ctl (win, GMA_ALTERATION);
      options->alterationZones = xvt_list_get_sel_index (cwin);
   }
   
   if (win = tabs[1].win)
   {
      cwin = xvt_win_get_ctl (win, SURFACE_TYPE_LIST);
      options->surfaceType = xvt_list_get_sel_index (cwin);
      options->surfaceXDim = getFloatTextFieldValue (win, SURFACE_XDIM);
      options->surfaceYDim = getFloatTextFieldValue (win, SURFACE_YDIM);
      options->surfaceZDim = getFloatTextFieldValue (win, SURFACE_ZDIM);
   }

   if (win = tabs[2].win)
   {
      options->positionX = getFloatTextFieldValue (win, XYZ_X);
      options->positionY = getFloatTextFieldValue (win, XYZ_Y);
      options->positionZ = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[3].win)
   {
      options->dipDirection = getFloatTextFieldValue (win, DDP_DDIR);
      options->dip = getFloatTextFieldValue (win, DDP_DIP);
      options->pitch = getFloatTextFieldValue (win, DDP_PITCH);
   }

   if (win = tabs[5].win)
   {
      options->rotation = getFloatTextFieldValue (win, FSCL_ROTATION);
      options->slip = getFloatTextFieldValue (win, FSCL_SLIP);
      options->amplitude = getFloatTextFieldValue (win, FSCL_AMPLITUDE);
      options->radius = getFloatTextFieldValue (win, FSCL_RADIUS);
      options->cylindricalIndex = getFloatTextFieldValue (win, FSCL_CYL);
      options->profilePitch = getFloatTextFieldValue (win, FSCL_PP);
   }

   if (win = tabs[6].win)
   {
      options->xAxis = getFloatTextFieldValue (win, AXIS_X);
      options->yAxis = getFloatTextFieldValue (win, AXIS_Y);
      options->zAxis = getFloatTextFieldValue (win, AXIS_Z);
   }

   if ((options->geometry == ELLIPTICAL)
                          || (options->geometry == CURVED))
   {
      if (options->slip > 0.45*options->xAxis)
      {
         if (previewOnly)
         {
            result = FALSE;
         }
         else
         {
            xvt_dm_post_error ("Warning! Using a Slip that is greater than 45 percent of the X-Axis may produce unexpected results.");
            result = FALSE;
         }
      }
   }
                      /* calculate and store the forward and
                      ** reverse rotation matricies */
   if (options->geometry == ROTATION)
      pitch = 0.0;
   else
      pitch = options->pitch;

   if (options->dip == 90.0)
      dip = 89.999;
   else
      dip = options->dip;

   convrt(dip, options->dipDirection, pitch, TRUE);
   rotset(dip, options->dipDirection, pitch,
               options->rotationMatrix.forward,
               options->rotationMatrix.reverse);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);

   return (result);
}

/* ======================================================================
FUNCTION        setFaultValues
DESCRIPTION
        set the values that are not NULL on the windows
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
setFaultValues (double *xPos, double *yPos, double *zPos,
                              double *dipDir, double *dip)
#else
setFaultValues (xPos, yPos, zPos, dipDir, dip)
double *xPos, *yPos, *zPos, *dipDir, *dip;
#endif
{
   WINDOW win;

   if (win = tabs[2].win)
   {
      if (xPos)
         updateFloatTextField (win, XYZ_X, *xPos, 1);
      if (yPos)
         updateFloatTextField (win, XYZ_Y, *yPos, 1);
      if (zPos)
         updateFloatTextField (win, XYZ_Z, *zPos, 1);

      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }

   if (win = tabs[3].win)
   {
      if (dipDir)
         updateFloatTextField (win, DDP_DDIR, *dipDir, 1);
      if (dip)
         updateFloatTextField (win, DDP_DIP, *dip, 1);

      updateScrollAndText(win, DDP_DDIR_BAR,  DDP_DDIR,  FALSE);
      updateScrollAndText(win, DDP_DIP_BAR,   DDP_DIP,   FALSE);
   }
}


/* ======================================================================
FUNCTION        updateFaultOptions
DESCRIPTION
        make sure the correct options are enabled
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
updateFaultOptions (OBJECT *object)
#else
updateFaultOptions (object)
OBJECT *object;
#endif
{
   char option[100];
   WINDOW win, cwin;
   BOOLEAN pitch, rot, slip, amp, radius, ellipse, cylIdx, pp;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, GMA_GEOMETRY);
      if (xvt_list_get_first_sel (cwin, option, 100))
      {
         if (strcmp (option, "Translation") == 0)
         {
            pitch = TRUE;   rot   = FALSE;  slip    = TRUE;
            amp   = FALSE;  radius= FALSE;  ellipse = FALSE;
            cylIdx= FALSE;  pp    = FALSE;
         }
         if (strcmp (option, "Rotation") == 0)
         {
            pitch = FALSE;  rot   = TRUE;   slip    = FALSE;
            amp   = FALSE;  radius= FALSE;  ellipse = FALSE;
            cylIdx= FALSE;  pp    = FALSE;
         }
         if (strcmp (option, "Elliptical") == 0)
         {
            pitch = TRUE;   rot   = FALSE;  slip    = TRUE;
            amp   = FALSE;  radius= FALSE;  ellipse = TRUE;
            cylIdx= FALSE;  pp    = FALSE;
         }
         if (strcmp (option, "Curved") == 0)
         {
            pitch = TRUE;   rot   = FALSE;  slip    = TRUE;
            amp   = TRUE;   radius= FALSE;  ellipse = TRUE;
            cylIdx= TRUE;   pp    = TRUE;
         }
         if (strcmp (option, "Ring") == 0)
         {
            pitch = FALSE;  rot   = FALSE;  slip    = TRUE;
            amp   = FALSE;  radius= TRUE;   ellipse = FALSE;
            cylIdx= FALSE;  pp    = FALSE;
         }
      }

      cwin = xvt_win_get_ctl (win, GMA_ALTERATION);
      if (xvt_list_get_sel_index (cwin) == 0)
         xvt_vobj_set_enabled (xvt_win_get_ctl (win, GMA_ALTERATION_EDIT), FALSE);
      else
         xvt_vobj_set_enabled (xvt_win_get_ctl (win, GMA_ALTERATION_EDIT), TRUE);

                                                         /* Show Profile Button */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, GMA_PROFILE_BUTTON), pp);
   }
   
   if (win = tabs[1].win)
   {                                                      /* Surface Type */
      int dxfSurf = xvt_list_get_sel_index (xvt_win_get_ctl (win, SURFACE_TYPE_LIST));
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_VIEW), (BOOLEAN) dxfSurf);
      if (!dxfSurf)  /* Free the data if its not selected */
      {
         DXFHANDLE *dxfData;
         if (dxfData = getDxfSurfaceStructure (object))
            DXFfree(dxfData);
      }
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM), (BOOLEAN) dxfSurf);
   }
   
   if (win = tabs[3].win)
   {                                                      /* Pitch */
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, DDP_PITCH_LABEL), (BOOLEAN) pitch);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, DDP_PITCH_BAR), (BOOLEAN) pitch);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, DDP_PITCH), (BOOLEAN) pitch);
   }

   if (win = tabs[5].win)
   {                                                      /* Rotation */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_ROTATION_LABEL), (BOOLEAN) rot);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_ROTATION_BAR), (BOOLEAN) rot);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_ROTATION), (BOOLEAN) rot);
                                                          /* Slip */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_SLIP_LABEL), (BOOLEAN) slip);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_SLIP_BAR), (BOOLEAN) slip);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_SLIP), (BOOLEAN) slip);
                                                          /* Amplitude */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_AMPLITUDE_LABEL), (BOOLEAN) amp);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_AMPLITUDE_BAR), (BOOLEAN) amp);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_AMPLITUDE), (BOOLEAN) amp);
                                                          /* Radius */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_RADIUS_LABEL),(BOOLEAN) radius);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_RADIUS_BAR), (BOOLEAN) radius);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_RADIUS), (BOOLEAN) radius);
                                                         /* cylIdx */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_CYL_LABEL), (BOOLEAN) cylIdx);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_CYL_BAR), (BOOLEAN) cylIdx);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_CYL), (BOOLEAN) cylIdx);
                                                          /* Profile Pitch */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_PP_LABEL), (BOOLEAN) pp);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_PP_BAR), (BOOLEAN) pp);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, FSCL_PP), (BOOLEAN) pp);
   }

   if (win = tabs[6].win)
   {                                                      /* Ellipse X */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_X_LABEL), (BOOLEAN) ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_X_BAR), (BOOLEAN) ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_X), (BOOLEAN) ellipse);
                                                         /* Ellipse Y */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Y_LABEL), (BOOLEAN) ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Y_BAR), (BOOLEAN) ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Y), (BOOLEAN) ellipse);
                                                         /* Ellipse Z */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Z_LABEL), (BOOLEAN) ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Z_BAR), (BOOLEAN) ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Z), (BOOLEAN) ellipse);
   }
}

int
#if XVT_CC_PROTO
getFaultPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getFaultPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;

   if (win = tabs[2].win)
   {
      *x = getFloatTextFieldValue (win, XYZ_X);
      *y = getFloatTextFieldValue (win, XYZ_Y);
      *z = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[3].win)
   {
      *dip = getFloatTextFieldValue (win, DDP_DIP);
      *dipDir = getFloatTextFieldValue (win, DDP_DDIR);
      *pitch = getFloatTextFieldValue (win, DDP_PITCH);
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* *********************** Unconformity ************************ */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadUnconformityOptions (OBJECT *object)
#else
loadUnconformityOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;
   STRATIGRAPHY_OPTIONS *stratOptions = getStratigraphyOptionsStructure (object);
   
   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, ALT_ALTERATION);
      xvt_list_set_sel (cwin, options->alterationZones, TRUE);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, ALT_ALTERATION_EDIT),
                            (BOOLEAN) options->alterationZones);
   }

   if (win = tabs[1].win)
   {
      cwin = xvt_win_get_ctl (win, SURFACE_TYPE_LIST);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Flat Plane");
      xvt_list_add (cwin , 1, "DXF Surface");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->surfaceType, TRUE);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_VIEW),
                         (BOOLEAN) options->alterationZones);
      cwin = updateFloatTextField (win, SURFACE_XDIM, options->surfaceXDim, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SURFACE_YDIM, options->surfaceYDim, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SURFACE_ZDIM, options->surfaceZDim, 1);
      setControlFont (cwin);

      updateScrollAndText(win, SURFACE_XDIM_BAR, SURFACE_XDIM, FALSE);
      updateScrollAndText(win, SURFACE_YDIM_BAR, SURFACE_YDIM, FALSE);
      updateScrollAndText(win, SURFACE_ZDIM_BAR, SURFACE_ZDIM, FALSE);
   }
   
   if (win = tabs[2].win)
   {
      cwin = updateFloatTextField (win, XYZ_X, options->positionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Y, options->positionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Z, options->positionZ, 1);
      setControlFont (cwin);
      
      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }
   
   if (win = tabs[3].win)
   {
      cwin = updateFloatTextField (win, DDD_DDIR, options->dipDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDD_DIP, options->dip, 1);
      setControlFont (cwin);

      updateScrollAndText(win, DDD_DDIR_BAR, DDD_DDIR, FALSE);
      updateScrollAndText(win, DDD_DIP_BAR,  DDD_DIP,  FALSE);
   }
   
   updateUnconformityOptions (object);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveUnconformityOptions (OBJECT *object)
#else
saveUnconformityOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;
   STRATIGRAPHY_OPTIONS *stratOptions = getStratigraphyOptionsStructure (object);
   
   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, ALT_ALTERATION);
      options->alterationZones = xvt_list_get_sel_index (cwin);
   }
   
   if (win = tabs[1].win)
   {
      cwin = xvt_win_get_ctl (win, SURFACE_TYPE_LIST);
      options->surfaceType = xvt_list_get_sel_index (cwin);
      options->surfaceXDim = getFloatTextFieldValue (win, SURFACE_XDIM);
      options->surfaceYDim = getFloatTextFieldValue (win, SURFACE_YDIM);
      options->surfaceZDim = getFloatTextFieldValue (win, SURFACE_ZDIM);
   }
   
   if (win = tabs[2].win)
   {
      options->positionX = getFloatTextFieldValue (win, XYZ_X);
      options->positionY = getFloatTextFieldValue (win, XYZ_Y);
      options->positionZ = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[3].win)
   {
      options->dipDirection = getFloatTextFieldValue (win, DDD_DDIR);
      options->dip = getFloatTextFieldValue (win, DDD_DIP);
   }
   
   if (win = tabs[4].win)
   {
      cwin = xvt_win_get_ctl (win, STRATIGRAPHY_LIST);
      stratOptions->numLayers = xvt_list_count_all (cwin);
   }

                      /* calculate and store the forward and
                      ** reverse rotation matricies */
   convrt(options->dip, options->dipDirection, 0.0, TRUE);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   rotset(options->dip, options->dipDirection, 0.0,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);

   return (TRUE);
}

/* ======================================================================
FUNCTION        updateUnconformityOptions
DESCRIPTION
        make sure the correct options are enabled
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
updateUnconformityOptions (OBJECT *object)
#else
updateUnconformityOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   UNCONFORMITY_OPTIONS *unconOptions = (UNCONFORMITY_OPTIONS *) object->options;
   STRATIGRAPHY_OPTIONS *options = getStratigraphyOptionsStructure (object);
   int numLayers = 0, layer;
   int selIndex;

   options = &(unconOptions->stratigraphyDetails);

   if (win = tabs[1].win)
   {                                                      /* Surface Type */
      int dxfSurf = xvt_list_get_sel_index (xvt_win_get_ctl (win, SURFACE_TYPE_LIST));
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_VIEW), (BOOLEAN) dxfSurf);
      if (!dxfSurf)  /* Free the data if its not selected */
      {
         DXFHANDLE *dxfData;
         if (dxfData = getDxfSurfaceStructure (object))
            DXFfree(dxfData);
      }
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM), (BOOLEAN) dxfSurf);
   }

   if (win = tabs[4].win)
   {
      cwin = xvt_win_get_ctl (win, STRATIGRAPHY_LIST);
      selIndex = xvt_list_get_sel_index (cwin);  /* get so it wont change */
      xvt_list_suspend (cwin);
      xvt_list_clear (cwin);
      for (layer = options->numLayers-1; layer >= 0; layer--)
      {
         xvt_list_add (cwin, -1, options->properties[layer].unitName);
      }
      xvt_list_resume (cwin);

      xvt_list_set_sel (cwin, selIndex, TRUE); /* set so it wont change */
   }
   
   update3dPreview (NULL_WIN, object);
}

int
#if XVT_CC_PROTO
getUnconformityPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getUnconformityPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;

   if (win = tabs[2].win)
   {
      *x = getFloatTextFieldValue (win, XYZ_X);
      *y = getFloatTextFieldValue (win, XYZ_Y);
      *z = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[3].win)
   {
      *dip = getFloatTextFieldValue (win, DDD_DIP);
      *dipDir = getFloatTextFieldValue (win, DDD_DDIR);
      *pitch = 90.0;
   }
   
   if (object->optionGroup == 2) /* show strat and not plane */
      return (FALSE);
      
   return (TRUE);
}

/* ************************************************************* */
/* *************************** Shear *************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadShearOptions (OBJECT *object)
#else
loadShearOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, TGMA_TYPE);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Shear Zone");
      xvt_list_add (cwin , 1, "Kink");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->type-SHEAR_ZONE_EVENT, TRUE);

      cwin = xvt_win_get_ctl (win, TGMA_GEOMETRY);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Translation");
      xvt_list_add (cwin , 1, "Rotation");
      xvt_list_add (cwin , 2, "Elliptical");
      xvt_list_add (cwin , 3, "Curved");
      xvt_list_add (cwin , 4, "Ring");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->geometry, TRUE);

      cwin = xvt_win_get_ctl (win, TGMA_MOVEMENT);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Hanging Wall");
      xvt_list_add (cwin , 1, "Foot Wall");
      xvt_list_add (cwin , 2, "Both");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->movement, TRUE);

      cwin = xvt_win_get_ctl (win, TGMA_ALTERATION);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "None");
      xvt_list_add (cwin , 1, "Hanging Wall");
      xvt_list_add (cwin , 2, "Foot Wall");
      xvt_list_add (cwin , 3, "Both");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->alterationZones, TRUE);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, GMA_ALTERATION_EDIT),
                         (BOOLEAN) options->alterationZones);
   }

   if (win = tabs[1].win)
   {
      cwin = xvt_win_get_ctl (win, SURFACE_TYPE_LIST);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Flat Plane");
      xvt_list_add (cwin , 1, "DXF Surface");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->surfaceType, TRUE);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_VIEW),
                         (BOOLEAN) options->alterationZones);
      cwin = updateFloatTextField (win, SURFACE_XDIM, options->surfaceXDim, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SURFACE_YDIM, options->surfaceYDim, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SURFACE_ZDIM, options->surfaceZDim, 1);
      setControlFont (cwin);

      updateScrollAndText(win, SURFACE_XDIM_BAR, SURFACE_XDIM, FALSE);
      updateScrollAndText(win, SURFACE_YDIM_BAR, SURFACE_YDIM, FALSE);
      updateScrollAndText(win, SURFACE_ZDIM_BAR, SURFACE_ZDIM, FALSE);
   }
    
   if (win = tabs[2].win)
   {
      cwin = updateFloatTextField (win, XYZ_X, options->positionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Y, options->positionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Z, options->positionZ, 1);
      setControlFont (cwin);
      
      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }

   if (win = tabs[3].win)
   {
      cwin = updateFloatTextField (win, DDP_DDIR, options->dipDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_DIP, options->dip, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_PITCH, options->pitch, 1);
      setControlFont (cwin);

      updateScrollAndText(win, DDP_DDIR_BAR,  DDP_DDIR,  FALSE);
      updateScrollAndText(win, DDP_DIP_BAR,   DDP_DIP,   FALSE);
      updateScrollAndText(win, DDP_PITCH_BAR, DDP_PITCH, FALSE);
   }

   if (win = tabs[5].win)
   {
      cwin = updateFloatTextField (win, SSCL_ROTATION, options->rotation, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SSCL_SLIP, options->slip, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SSCL_AMPLITUDE, options->amplitude, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SSCL_WIDTH, options->width, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SSCL_RADIUS, options->radius, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SSCL_CYL, options->cylindricalIndex, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SSCL_PP, options->profilePitch, 1);
      setControlFont (cwin);

      updateScrollAndText(win, SSCL_ROTATION_BAR,  SSCL_ROTATION, FALSE);
      updateScrollAndText(win, SSCL_SLIP_BAR,      SSCL_SLIP,     FALSE);
      updateScrollAndText(win, SSCL_AMPLITUDE_BAR, SSCL_AMPLITUDE,FALSE);
      updateScrollAndText(win, SSCL_WIDTH_BAR,     SSCL_WIDTH,    FALSE);
      updateScrollAndText(win, SSCL_RADIUS_BAR,    SSCL_RADIUS,   FALSE);
      updateScrollAndText(win, SSCL_CYL_BAR,       SSCL_CYL,      FALSE);
      updateScrollAndText(win, SSCL_PP_BAR,        SSCL_PP,       FALSE);
   }

   if (win = tabs[6].win)
   {
      cwin = updateFloatTextField (win, AXIS_X, options->xAxis, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, AXIS_Y, options->yAxis, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, AXIS_Z, options->zAxis, 1);
      setControlFont (cwin);

      updateScrollAndText(win, AXIS_X_BAR, AXIS_X, FALSE);
      updateScrollAndText(win, AXIS_Y_BAR, AXIS_Y, FALSE);
      updateScrollAndText(win, AXIS_Z_BAR, AXIS_Z, FALSE);
   }

   updateShearOptions (object);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveShearOptions (OBJECT *object)
#else
saveShearOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;
   double pitch, dip;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, TGMA_TYPE);
      options->type = xvt_list_get_sel_index (cwin) + SHEAR_ZONE_EVENT;

      cwin = xvt_win_get_ctl (win, TGMA_GEOMETRY);
      options->geometry = xvt_list_get_sel_index (cwin);

      cwin = xvt_win_get_ctl (win, TGMA_MOVEMENT);
      options->movement = xvt_list_get_sel_index (cwin);

      cwin = xvt_win_get_ctl (win, TGMA_ALTERATION);
      options->alterationZones = xvt_list_get_sel_index (cwin);
   }
   
   if (win = tabs[1].win)
   {
      cwin = xvt_win_get_ctl (win, SURFACE_TYPE_LIST);
      options->surfaceType = xvt_list_get_sel_index (cwin);
      options->surfaceXDim = getFloatTextFieldValue (win, SURFACE_XDIM);
      options->surfaceYDim = getFloatTextFieldValue (win, SURFACE_YDIM);
      options->surfaceZDim = getFloatTextFieldValue (win, SURFACE_ZDIM);
   }
   
   if (win = tabs[2].win)
   {
      options->positionX = getFloatTextFieldValue (win, XYZ_X);
      options->positionY = getFloatTextFieldValue (win, XYZ_Y);
      options->positionZ = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[3].win)
   {
      options->dipDirection = getFloatTextFieldValue (win, DDP_DDIR);
      options->dip = getFloatTextFieldValue (win, DDP_DIP);
      options->pitch = getFloatTextFieldValue (win, DDP_PITCH);
   }

   if (win = tabs[5].win)
   {
      options->rotation = getFloatTextFieldValue (win, SSCL_ROTATION);
      options->slip = getFloatTextFieldValue (win, SSCL_SLIP);
      options->amplitude = getFloatTextFieldValue (win, SSCL_AMPLITUDE);
      options->width = getFloatTextFieldValue (win, SSCL_WIDTH);
      options->radius = getFloatTextFieldValue (win, SSCL_RADIUS);
      options->cylindricalIndex = getFloatTextFieldValue (win, SSCL_CYL);
      options->profilePitch = getFloatTextFieldValue (win, SSCL_PP);
   }

   if (win = tabs[6].win)
   {
      options->xAxis = getFloatTextFieldValue (win, AXIS_X);
      options->yAxis = getFloatTextFieldValue (win, AXIS_Y);
      options->zAxis = getFloatTextFieldValue (win, AXIS_Z);
   }


   if ((options->geometry == ELLIPTICAL)
                          || (options->geometry == CURVED))
   {
      if (options->slip > 0.45*options->xAxis)
         xvt_dm_post_error ("Warning! Using a Slip that is greater than 45 percent of the X-Axis may produce unexpected results.");
   }
                      /* calculate and store the forward and
                      ** reverse rotation matricies */
   if (options->geometry == ROTATION)
      pitch = 0.0;
   else
      pitch = options->pitch;

   if (options->dip == 90.0)
      dip = 89.999;
   else
      dip = options->dip;

   convrt(dip, options->dipDirection, pitch, TRUE);
   rotset(dip, options->dipDirection, pitch,
               options->rotationMatrix.forward,
               options->rotationMatrix.reverse);
   plane(&options->plane.a, &options->plane.b,
         &options->plane.c, &options->plane.d,
         options->positionX, options->positionY, options->positionZ);
   getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);

   return (TRUE);
}

/* ======================================================================
FUNCTION        setShearValues
DESCRIPTION
        set the values that are not NULL on the windows
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
setShearValues (double *xPos, double *yPos, double *zPos,
                              double *dipDir, double *dip)
#else
setShearValues (xPos, yPos, zPos, dipDir, dip)
double *xPos, *yPos, *zPos, *dipDir, *dip;
#endif
{
   WINDOW win;

   if (win = tabs[2].win)
   {
      if (xPos)
         updateFloatTextField (win, XYZ_X, *xPos, 1);
      if (yPos)
         updateFloatTextField (win, XYZ_Y, *yPos, 1);
      if (zPos)
         updateFloatTextField (win, XYZ_Z, *zPos, 1);

      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }

   if (win = tabs[3].win)
   {
      if (dipDir)
         updateFloatTextField (win, DDP_DDIR, *dipDir, 1);
      if (dip)
         updateFloatTextField (win, DDP_DIP, *dip, 1);

      updateScrollAndText(win, DDP_DDIR_BAR,  DDP_DDIR,  FALSE);
      updateScrollAndText(win, DDP_DIP_BAR,   DDP_DIP,   FALSE);
   }
}

/* ======================================================================
FUNCTION        updateShearOptions
DESCRIPTION
        make sure the correct options are enabled
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
updateShearOptions (OBJECT *object)
#else
updateShearOptions (object)
OBJECT *object;
#endif
{
   char option[100];
   WINDOW win, cwin;
   BOOLEAN pitch, rot, slip, amp, radius, ellipse, cylIdx, pp;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, TGMA_GEOMETRY);
      if (xvt_list_get_first_sel (cwin, option, 100))
      {
         if (strcmp (option, "Translation") == 0)
         {
            pitch = TRUE;   rot   = FALSE;  slip    = TRUE;
            amp   = FALSE;  radius= FALSE;  ellipse = FALSE;
            cylIdx= FALSE;  pp    = FALSE;
         }
         if (strcmp (option, "Rotation") == 0)
         {
            pitch = FALSE;  rot   = TRUE;   slip    = FALSE;
            amp   = FALSE;  radius= FALSE;  ellipse = FALSE;
            cylIdx= FALSE;  pp    = FALSE;
         }
         if (strcmp (option, "Elliptical") == 0)
         {
            pitch = TRUE;   rot   = FALSE;  slip    = TRUE;
            amp   = FALSE;  radius= FALSE;  ellipse = TRUE;
            cylIdx= FALSE;  pp    = FALSE;
         }
         if (strcmp (option, "Curved") == 0)
         {
            pitch = TRUE;   rot   = FALSE;  slip    = TRUE;
            amp   = TRUE;   radius= FALSE;  ellipse = TRUE;
            cylIdx= TRUE;   pp    = TRUE;
         }
         if (strcmp (option, "Ring") == 0)
         {
            pitch = FALSE;  rot   = FALSE;  slip    = TRUE;
            amp   = FALSE;  radius= TRUE;   ellipse = FALSE;
            cylIdx= FALSE;  pp    = FALSE;
         }
      }

      cwin = xvt_win_get_ctl (win, TGMA_ALTERATION);
      if (xvt_list_get_sel_index (cwin) == 0)
         xvt_vobj_set_enabled (xvt_win_get_ctl (win, TGMA_ALTERATION_EDIT), FALSE);
      else
         xvt_vobj_set_enabled (xvt_win_get_ctl (win, TGMA_ALTERATION_EDIT), TRUE);

                                                         /* Show Profile Button */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, TGMA_PROFILE_BUTTON), pp);
   }
   
   if (win = tabs[1].win)
   {                                                      /* Surface Type */
      int dxfSurf = xvt_list_get_sel_index (xvt_win_get_ctl (win, SURFACE_TYPE_LIST));
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_VIEW), (BOOLEAN) dxfSurf);
      if (!dxfSurf)  /* Free the data if its not selected */
      {
         DXFHANDLE *dxfData;
         if (dxfData = getDxfSurfaceStructure (object))
            DXFfree(dxfData);
      }
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_XDIM), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_YDIM), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM_LABEL), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM_BAR), (BOOLEAN) dxfSurf);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SURFACE_ZDIM), (BOOLEAN) dxfSurf);
   }
   
   if (win = tabs[3].win)
   {                                                      /* Pitch */
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, DDP_PITCH_LABEL), pitch);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, DDP_PITCH_BAR), pitch);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, DDP_PITCH), pitch);
   }

   if (win = tabs[5].win)
   {                                                      /* Rotation */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_ROTATION_LABEL), rot);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_ROTATION_BAR), rot);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_ROTATION), rot);
                                                          /* Slip */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_SLIP_LABEL), slip);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_SLIP_BAR), slip);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_SLIP), slip);
                                                          /* Amplitude */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_AMPLITUDE_LABEL), amp);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_AMPLITUDE_BAR), amp);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_AMPLITUDE), amp);
                                                          /* Radius */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_RADIUS_LABEL),radius);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_RADIUS_BAR), radius);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_RADIUS), radius);
                                                         /* cylIdx */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_CYL_LABEL), cylIdx);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_CYL_BAR), cylIdx);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_CYL), cylIdx);
                                                          /* Profile Pitch */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_PP_LABEL), pp);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_PP_BAR), pp);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, SSCL_PP), pp);
   }

   if (win = tabs[6].win)
   {                                                      /* Ellipse X */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_X_LABEL), ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_X_BAR), ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_X), ellipse);
                                                         /* Ellipse Y */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Y_LABEL), ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Y_BAR), ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Y), ellipse);
                                                         /* Ellipse Z */
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Z_LABEL), ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Z_BAR), ellipse);
      xvt_vobj_set_enabled (xvt_win_get_ctl(win, AXIS_Z), ellipse);
   }
}

int
#if XVT_CC_PROTO
getShearPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getShearPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;

   if (win = tabs[2].win)
   {
      *x = getFloatTextFieldValue (win, XYZ_X);
      *y = getFloatTextFieldValue (win, XYZ_Y);
      *z = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[3].win)
   {
      *dip = getFloatTextFieldValue (win, DDP_DIP);
      *dipDir = getFloatTextFieldValue (win, DDP_DDIR);
      *pitch = getFloatTextFieldValue (win, DDP_PITCH);
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* *************************** Dyke **************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadDykeOptions (OBJECT *object)
#else
loadDykeOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   DYKE_OPTIONS *options = (DYKE_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
         /* put the options into the list selection */
      cwin = xvt_win_get_ctl (win, FDYKE_TYPE);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin, 0, "Dilation");
      xvt_list_add (cwin, 1, "Stope");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->type, TRUE);

      cwin = xvt_win_get_ctl (win, FDYKE_ALTERATION);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "None");
      xvt_list_add (cwin , 1, "Hanging Wall");
      xvt_list_add (cwin , 2, "Foot Wall");
      xvt_list_add (cwin , 3, "Both");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->alterationZones, TRUE);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, FDYKE_ALTERATION_EDIT),
                         (BOOLEAN) options->alterationZones);

      cwin = xvt_win_get_ctl (win, FDYKE_MERGE);
      xvt_ctl_set_checked(cwin, (BOOLEAN) options->mergeEvents);
   }
   
   if (win = tabs[1].win)
   {
      cwin = updateFloatTextField (win, XYZ_X, options->positionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Y, options->positionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Z, options->positionZ, 1);
      setControlFont (cwin);
      
      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }

   if (win = tabs[2].win)
   {
      cwin = updateFloatTextField (win, DDP_DDIR, options->dipDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_DIP, options->dip, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_PITCH, options->slipPitch, 1);
      setControlFont (cwin);

      updateScrollAndText(win, DDP_DDIR_BAR,  DDP_DDIR,  FALSE);
      updateScrollAndText(win, DDP_DIP_BAR,   DDP_DIP,   FALSE);
      updateScrollAndText(win, DDP_PITCH_BAR, DDP_PITCH, FALSE);
   }

   if (win = tabs[3].win)
   {
      cwin = updateFloatTextField (win, SW_SLIP, options->slipLength, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, SW_WIDTH, options->width, 1);
      setControlFont (cwin);

      updateScrollAndText(win, SW_SLIP_BAR,  SW_SLIP,  FALSE);
      updateScrollAndText(win, SW_WIDTH_BAR, SW_WIDTH, FALSE);
   }

   updateDykeOptions (NULL_WIN);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveDykeOptions (OBJECT *object)
#else
saveDykeOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   DYKE_OPTIONS *options = (DYKE_OPTIONS *) object->options;
   double dipDirection;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, FDYKE_TYPE);
      options->type = xvt_list_get_sel_index (cwin);

      cwin = xvt_win_get_ctl (win, FDYKE_ALTERATION);
      options->alterationZones = xvt_list_get_sel_index (cwin);

      cwin = xvt_win_get_ctl (win, FDYKE_MERGE);
      options->mergeEvents = xvt_ctl_is_checked (cwin);
   }
   
   
   if (win = tabs[1].win)
   {
      options->positionX = getFloatTextFieldValue (win, XYZ_X);
      options->positionY = getFloatTextFieldValue (win, XYZ_Y);
      options->positionZ = getFloatTextFieldValue (win, XYZ_Z);
   }

   if (win = tabs[2].win)
   {
      options->dipDirection = getFloatTextFieldValue (win, DDP_DDIR);
      options->dip = getFloatTextFieldValue (win, DDP_DIP);
      options->slipPitch = getFloatTextFieldValue (win, DDP_PITCH);
   }

   if (win = tabs[3].win)
   {
      options->slipLength = getFloatTextFieldValue (win, SW_SLIP);
      options->width = getFloatTextFieldValue (win, SW_WIDTH);
   }

                      /* calculate and store the forward and
                      ** reverse rotation matricies */
   if (options->dip == 90.0)
      dipDirection = options->dipDirection + 180.0;
   else
      dipDirection = options->dipDirection;

   convrt(options->dip, dipDirection, 90.0, TRUE);
   rotset(options->dip, dipDirection, 90.0,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);

   return (TRUE);
}

/* ======================================================================
FUNCTION        updateDykeOptions
DESCRIPTION
        make sure the correct options are enabled
INPUT

OUTPUT

RETURNED
====================================================================== */
void 
#if XVT_CC_PROTO
updateDykeOptions (WINDOW parentWin)
#else
updateDykeOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win, cwin;
   int pitch, length;
   int type;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, FDYKE_TYPE);
      type = xvt_list_get_sel_index (cwin);
      switch (type)
      {
         case (DILATION_DYKE):
            pitch = TRUE;
            length = TRUE;
            break;
         case (STOPE_DYKE):
            pitch = FALSE;
            length = TRUE;
            break;
      }
                /* allow edit button to be pressed with alterations */
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, FDYKE_ALTERATION_EDIT),
         (BOOLEAN) xvt_list_get_sel_index(xvt_win_get_ctl(win, FDYKE_ALTERATION)));
   }

   if (win = tabs[3].win)
   {                                              /* slip Pitch */
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SW_SLIP_LABEL), (BOOLEAN) pitch);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SW_SLIP_BAR), (BOOLEAN) pitch);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SW_SLIP), (BOOLEAN) pitch);
                                                  /* width */
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SW_WIDTH_LABEL), (BOOLEAN) length);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SW_WIDTH_BAR), (BOOLEAN) length);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, SW_WIDTH), (BOOLEAN) length);
   }
}

int
#if XVT_CC_PROTO
getDykePlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getDykePlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   DYKE_OPTIONS *options = (DYKE_OPTIONS *) object->options;

   if (win = tabs[1].win)
   {
      *x = getFloatTextFieldValue (win, XYZ_X);
      *y = getFloatTextFieldValue (win, XYZ_Y);
      *z = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[2].win)
   {
      *dip = getFloatTextFieldValue (win, DDP_DIP);
      *dipDir = getFloatTextFieldValue (win, DDP_DDIR);
      *pitch = getFloatTextFieldValue (win, DDP_PITCH);
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* *************************** Plug **************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadPlugOptions (OBJECT *object)
#else
loadPlugOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   PLUG_OPTIONS *options = (PLUG_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
         /* put the options into the list selection */
      cwin = xvt_win_get_ctl (win, FPLUG_GEOMETRY);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "Cylindrical");
      xvt_list_add (cwin , 1, "Cone");
      xvt_list_add (cwin , 2, "Parabolic");
      xvt_list_add (cwin , 3, "Ellipsoidal");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->type, TRUE);

      cwin = xvt_win_get_ctl (win, FPLUG_ALTERATION);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "None");
      xvt_list_add (cwin , 1, "Top");
      xvt_list_add (cwin , 2, "Bottom");
      xvt_list_add (cwin , 3, "Both");
      xvt_list_resume (cwin);
      xvt_list_set_sel (cwin, options->alterationZones, TRUE);
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, FPLUG_ALTERATION_EDIT),
                         (BOOLEAN) options->alterationZones);

      cwin = xvt_win_get_ctl (win, FPLUG_MERGE);
      xvt_ctl_set_checked(cwin, (BOOLEAN) options->mergeEvents);
   }
   
   if (win = tabs[1].win)
   {
      cwin = updateFloatTextField (win, XYZ_X, options->positionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Y, options->positionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Z, options->positionZ, 1);
      setControlFont (cwin);
      
      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }
   
   if (win = tabs[2].win)
   {
      cwin = updateFloatTextField (win, DDP_DDIR, options->dipDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_DIP, options->dip, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDP_PITCH, options->axisPitch, 1);
      setControlFont (cwin);

      updateScrollAndText(win, DDP_DDIR_BAR,  DDP_DDIR,  FALSE);
      updateScrollAndText(win, DDP_DIP_BAR,   DDP_DIP,   FALSE);
      updateScrollAndText(win, DDP_PITCH_BAR, DDP_PITCH, FALSE);
   }

   if (win = tabs[3].win)
   {
      cwin = updateFloatTextField (win, PSCL_RADIUS, options->radius, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, PSCL_ANGLE, options->apicalAngle, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, PSCL_BVALUE, options->BValue, 1);
      setControlFont (cwin);

      updateScrollAndText(win, PSCL_RADIUS_BAR, PSCL_RADIUS, FALSE);
      updateScrollAndText(win, PSCL_ANGLE_BAR,  PSCL_ANGLE,  FALSE);
      updateScrollAndText(win, PSCL_BVALUE_BAR, PSCL_BVALUE, FALSE);
   }

   if (win = tabs[4].win)
   {
      cwin = updateFloatTextField (win, AXIS_X, options->axisA, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, AXIS_Y, options->axisB, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, AXIS_Z, options->axisC, 1);
      setControlFont (cwin);

      updateScrollAndText(win, AXIS_X_BAR, AXIS_X, FALSE);
      updateScrollAndText(win, AXIS_Y_BAR, AXIS_Y, FALSE);
      updateScrollAndText(win, AXIS_Z_BAR, AXIS_Z, FALSE);
   }

   updatePlugOptions (NULL_WIN);

   return (TRUE);
}

static int
#if XVT_CC_PROTO
savePlugOptions (OBJECT *object)
#else
savePlugOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   PLUG_OPTIONS *options = (PLUG_OPTIONS *) object->options;
   double pitch;
   
   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, FPLUG_GEOMETRY);
      options->type = xvt_list_get_sel_index (cwin);

      cwin = xvt_win_get_ctl (win, FPLUG_ALTERATION);
      options->alterationZones = xvt_list_get_sel_index (cwin);

      cwin = xvt_win_get_ctl (win, FPLUG_MERGE);
      options->mergeEvents = xvt_ctl_is_checked (cwin);
   }
   
   if (win = tabs[1].win)
   {
      options->positionX = getFloatTextFieldValue (win, XYZ_X);
      options->positionY = getFloatTextFieldValue (win, XYZ_Y);
      options->positionZ = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[2].win)
   {
      options->dipDirection = getFloatTextFieldValue (win, DDP_DDIR);
      options->dip = getFloatTextFieldValue (win, DDP_DIP);
      options->axisPitch = getFloatTextFieldValue (win, DDP_PITCH);
   }

   if (win = tabs[3].win)
   {
      options->radius = getFloatTextFieldValue (win, PSCL_RADIUS);
      options->apicalAngle = getFloatTextFieldValue (win, PSCL_ANGLE);
      options->BValue = getFloatTextFieldValue (win, PSCL_BVALUE);
   }

   if (win = tabs[4].win)
   {
      options->axisA = getFloatTextFieldValue (win, AXIS_X);
      options->axisB = getFloatTextFieldValue (win, AXIS_Y);
      options->axisC = getFloatTextFieldValue (win, AXIS_Z);
   }

                      /* calculate and store the forward and
                      ** reverse rotation matricies */
   if (options->type != ELLIPSOIDAL_PLUG)
      pitch = 0.0;
   else
      pitch = options->axisPitch;
   convrt(options->dip-90.0, options->dipDirection, pitch, TRUE);
   rotset(options->dip-90.0, options->dipDirection, pitch,
                        options->rotationMatrix.forward,
                        options->rotationMatrix.reverse);

   return (TRUE);
}

void 
#if XVT_CC_PROTO
updatePlugOptions (WINDOW parentWin)
#else
updatePlugOptions (parentWin)
WINDOW parentWin;
#endif
{
   WINDOW win, cwin;
   int radius, angle, bValue, axisPitch, axisPos;
   int geometry;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, FPLUG_GEOMETRY);
      geometry = xvt_list_get_sel_index (cwin);
      switch (geometry)
      {
         case (CYLINDRICAL_PLUG):
            radius = TRUE;     angle = FALSE;   bValue = FALSE;
            axisPitch = FALSE; axisPos = FALSE;
            break;
         case (CONE_PLUG):
            radius = FALSE;    angle = TRUE;    bValue = FALSE;
            axisPitch = FALSE; axisPos = FALSE;
            break;
         case (PARABOLIC_PLUG):
            radius = FALSE;    angle = FALSE;   bValue = TRUE;
            axisPitch = FALSE; axisPos = FALSE;
            break;
         case (ELLIPSOIDAL_PLUG):
            radius = FALSE;    angle = FALSE;   bValue = FALSE;
            axisPitch = TRUE;  axisPos = TRUE;
            break;
      }
                /* allow edit button to be pressed with alterations */
      xvt_vobj_set_enabled (xvt_win_get_ctl (win, FPLUG_ALTERATION_EDIT),
         (BOOLEAN) xvt_list_get_sel_index(xvt_win_get_ctl(win, FPLUG_ALTERATION)));
   }

   if (win = tabs[2].win)
   {                                                  /* axis Pitch */
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, DDP_PITCH_LABEL), (BOOLEAN) axisPitch);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, DDP_PITCH_BAR), (BOOLEAN) axisPitch);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, DDP_PITCH), (BOOLEAN) axisPitch);
   }
   
   if (win = tabs[3].win)
   {
                                                          /* radius */
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_RADIUS_LABEL), (BOOLEAN) radius);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_RADIUS_BAR), (BOOLEAN) radius);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_RADIUS), (BOOLEAN) radius);
                                                          /* Apical Angle */
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_ANGLE_LABEL), (BOOLEAN) angle);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_ANGLE_BAR), (BOOLEAN) angle);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_ANGLE), (BOOLEAN) angle);
                                                          /* B Value */
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_BVALUE_LABEL), (BOOLEAN) bValue);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_BVALUE_BAR), (BOOLEAN) bValue);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, PSCL_BVALUE), (BOOLEAN) bValue);
   }

   if (win = tabs[4].win)
   {                                                      /* axis Pos A */
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_X_LABEL), (BOOLEAN) axisPos);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_X_BAR), (BOOLEAN) axisPos);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_X), (BOOLEAN) axisPos);
                                                          /* axis Pos B */
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_Y_LABEL), (BOOLEAN) axisPos);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_Y_BAR), (BOOLEAN) axisPos);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_Y), (BOOLEAN) axisPos);
                                                          /* axis Pos C */
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_Z_LABEL), (BOOLEAN) axisPos);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_Z_BAR), (BOOLEAN) axisPos);
      xvt_vobj_set_enabled(xvt_win_get_ctl(win, AXIS_Z), (BOOLEAN) axisPos);
   }
}

int
#if XVT_CC_PROTO
getPlugPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getPlugPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   PLUG_OPTIONS *options = (PLUG_OPTIONS *) object->options;

   if (win = tabs[1].win)
   {
      *x = getFloatTextFieldValue (win, XYZ_X);
      *y = getFloatTextFieldValue (win, XYZ_Y);
      *z = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[2].win)
   {
      *dip = getFloatTextFieldValue (win, DDP_DIP);
      *dipDir = getFloatTextFieldValue (win, DDP_DDIR);
      *pitch = getFloatTextFieldValue (win, DDP_PITCH);
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* *************************** Tilt **************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadTiltOptions (OBJECT *object)
#else
loadTiltOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   TILT_OPTIONS *options = (TILT_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = updateFloatTextField (win, XYZ_X, options->positionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Y, options->positionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Z, options->positionZ, 1);
      setControlFont (cwin);
      
      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }
   
   if (win = tabs[1].win)
   {
      cwin = updateFloatTextField (win, PP_PDIR, options->plungeDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, PP_PLUNGE, options->plunge, 1);
      setControlFont (cwin);

      updateScrollAndText(win, PP_PDIR_BAR,   PP_PDIR,   FALSE);
      updateScrollAndText(win, PP_PLUNGE_BAR, PP_PLUNGE, FALSE);
   }
   
   if (win = tabs[2].win)
   {
      cwin = updateFloatTextField (win, ROTATION_ROT, options->rotation, 1);
      setControlFont (cwin);

      updateScrollAndText(win, ROTATION_ROT_BAR, ROTATION_ROT, FALSE);
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveTiltOptions (OBJECT *object)
#else
saveTiltOptions (object)
OBJECT *object;
#endif
{
   WINDOW win;
   TILT_OPTIONS *options = (TILT_OPTIONS *) object->options;
   double angle, axis[2][4], mata[4][4];
   int i, j;

   if (win = tabs[0].win)
   {
      options->positionX = getFloatTextFieldValue (win, XYZ_X);
      options->positionY = getFloatTextFieldValue (win, XYZ_Y);
      options->positionZ = getFloatTextFieldValue (win, XYZ_Z);
   }      
   
   if (win = tabs[1].win)
   {
      options->plungeDirection = getFloatTextFieldValue (win, PP_PDIR);
      options->plunge = getFloatTextFieldValue (win, PP_PLUNGE);
   }

   if (win = tabs[2].win)
   {
      options->rotation = getFloatTextFieldValue (win, ROTATION_ROT);
   }

               /* calculate and store the rotation matricies */
   angle = options->rotation * 0.0174532;
   paxis (options->plunge, options->plungeDirection, axis);
   irotate(mata, axis[1][1], axis[1][2], axis[1][3], angle);
   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         options->rotationMatrix[i][j] = mata[i+1][j+1];
   
   return (TRUE);
}

int
#if XVT_CC_PROTO
getTiltPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getTiltPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   TILT_OPTIONS *options = (TILT_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      *x = getFloatTextFieldValue (win, XYZ_X);
      *y = getFloatTextFieldValue (win, XYZ_Y);
      *z = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   if (win = tabs[1].win)
   {
      *dip = getFloatTextFieldValue (win, DDD_DIP);
      *dipDir = getFloatTextFieldValue (win, DDD_DDIR);
      *pitch = 90.0;
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* ************************** Strain *************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadStrainOptions (OBJECT *object)
#else
loadStrainOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   STRAIN_OPTIONS *options = (STRAIN_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = updateFloatTextField (win, MATRIX_TOP_LEFT, options->tensor[0][0], 2);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, MATRIX_TOP_CENTER, options->tensor[1][0], 2);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, MATRIX_TOP_RIGHT, options->tensor[2][0], 2);
      setControlFont (cwin);
   
      cwin = updateFloatTextField (win, MATRIX_CENTER_LEFT, options->tensor[0][1], 2);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, MATRIX_CENTER_CENTER, options->tensor[1][1], 2);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, MATRIX_CENTER_RIGHT, options->tensor[2][1], 2);
      setControlFont (cwin);
   
      cwin = updateFloatTextField (win, MATRIX_BOTTOM_LEFT, options->tensor[0][2], 2);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, MATRIX_BOTTOM_CENTER, options->tensor[1][2], 2);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, MATRIX_BOTTOM_RIGHT, options->tensor[2][2], 2);
      setControlFont (cwin);
   }
   
   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveStrainOptions (OBJECT *object)
#else
saveStrainOptions (object)
OBJECT *object;
#endif
{
   WINDOW win;
   STRAIN_OPTIONS *options = (STRAIN_OPTIONS *) object->options;
   int i, j;
   double tensor[4][4], inverseTensor[4][4];

   if (win = tabs[0].win)
   {
      options->tensor[0][0] = getFloatTextFieldValue (win, MATRIX_TOP_LEFT);
      options->tensor[1][0] = getFloatTextFieldValue (win, MATRIX_TOP_CENTER);
      options->tensor[2][0] = getFloatTextFieldValue (win, MATRIX_TOP_RIGHT);
   
      options->tensor[0][1] = getFloatTextFieldValue (win, MATRIX_CENTER_LEFT);
      options->tensor[1][1] = getFloatTextFieldValue (win, MATRIX_CENTER_CENTER);
      options->tensor[2][1] = getFloatTextFieldValue (win, MATRIX_CENTER_RIGHT);
   
      options->tensor[0][2] = getFloatTextFieldValue (win, MATRIX_BOTTOM_LEFT);
      options->tensor[1][2] = getFloatTextFieldValue (win, MATRIX_BOTTOM_CENTER);
      options->tensor[2][2] = getFloatTextFieldValue (win, MATRIX_BOTTOM_RIGHT);
   }

       /* calculate and store the inverse tensor matrix */
   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         tensor[j+1][i+1] = options->tensor[i][j];

   if (!f01aaf(tensor, inverseTensor)) /* matrix inversion */
      return (FALSE);

   for (i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
          options->inverseTensor[i][j] = inverseTensor[i+1][j+1];

   return (TRUE);
}

int
#if XVT_CC_PROTO
getStrainPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getStrainPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   return (FALSE);
}

/* ************************************************************* */
/* ************************* Foliation ************************* */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadFoliationOptions (OBJECT *object)
#else
loadFoliationOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   FOLIATION_OPTIONS *options = (FOLIATION_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = updateFloatTextField (win, DDD_DDIR, options->dipDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, DDD_DIP, options->dip, 1);
      setControlFont (cwin);

      updateScrollAndText(win, DDD_DDIR_BAR, DDD_DDIR, FALSE);
      updateScrollAndText(win, DDD_DIP_BAR,  DDD_DIP, FALSE);
   }
   
   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveFoliationOptions (OBJECT *object)
#else
saveFoliationOptions (object)
OBJECT *object;
#endif
{
   WINDOW win;
   FOLIATION_OPTIONS *options = (FOLIATION_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      options->dipDirection = getFloatTextFieldValue (win, DDD_DDIR);
      options->dip = getFloatTextFieldValue (win, DDD_DIP);
   }

   convrt(options->dip, options->dipDirection, 0.0, TRUE);
   rotset(options->dip, options->dipDirection, 0.0,
                                           options->rotationMatrix.forward,
                                           options->rotationMatrix.reverse);

   return (TRUE);
}

int
#if XVT_CC_PROTO
getFoliationPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getFoliationPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   FOLIATION_OPTIONS *options = (FOLIATION_OPTIONS *) object->options;

   *x = BLOCK_DIAGRAM_SIZE_X / 2;
   *y = BLOCK_DIAGRAM_SIZE_Y / 2;
   *z = BLOCK_DIAGRAM_SIZE_Z / 2;
   if (win = tabs[0].win)
   {
      *dip = getFloatTextFieldValue (win, DDD_DIP);
      *dipDir = getFloatTextFieldValue (win, DDD_DDIR);
      *pitch = 90.0;
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* ************************* Lineation ************************* */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadLineationOptions (OBJECT *object)
#else
loadLineationOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   LINEATION_OPTIONS *options = (LINEATION_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = updateFloatTextField (win, PP_PDIR, options->plungeDirection, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, PP_PLUNGE, options->plunge, 1);
      setControlFont (cwin);

      updateScrollAndText(win, PP_PDIR_BAR,   PP_PDIR,   FALSE);
      updateScrollAndText(win, PP_PLUNGE_BAR, PP_PLUNGE, FALSE);
   }
   
   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveLineationOptions (OBJECT *object)
#else
saveLineationOptions (object)
OBJECT *object;
#endif
{
   WINDOW win;
   LINEATION_OPTIONS *options = (LINEATION_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      options->plungeDirection = getFloatTextFieldValue (win, PP_PDIR);
      options->plunge = getFloatTextFieldValue (win, PP_PLUNGE);
   }

   convrt(90.0-options->plunge, 180.0+options->plungeDirection, 0.0, TRUE);
   rotset(90.0-options->plunge, 180.0+options->plungeDirection, 0.0,
                                           options->rotationMatrix.forward,
                                           options->rotationMatrix.reverse);

   return (TRUE);
}

int
#if XVT_CC_PROTO
getLineationPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getLineationPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   LINEATION_OPTIONS *options = (LINEATION_OPTIONS *) object->options;

   *x = BLOCK_DIAGRAM_SIZE_X / 2;
   *y = BLOCK_DIAGRAM_SIZE_Y / 2;
   *z = BLOCK_DIAGRAM_SIZE_Z / 2;
   if (win = tabs[0].win)
   {
      *dip = getFloatTextFieldValue (win, DDD_DIP);
      *dipDir = getFloatTextFieldValue (win, DDD_DDIR);
      *pitch = 90.0;
   }
   
   return (TRUE);
}

/* ************************************************************* */
/* ************************** Import *************************** */
/* ************************************************************* */
int
#if XVT_CC_PROTO
loadImportOptions (OBJECT *object)
#else
loadImportOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   IMPORT_OPTIONS *options = (IMPORT_OPTIONS *) object->options;
   int buttonIndex, result = FALSE;
   WINDOW radioButtons[5];

   if (!strlen(options->importFile.name))
      if (result = getImportFile (&(options->importFile)))
         if (!loadImportFile (options))
            xvt_dm_post_error ("Error, Could not load Import File");

   if (win = tabs[0].win)
   {
      buttonIndex = 0;
      radioButtons[buttonIndex++] = xvt_win_get_ctl (win, CORNER_CENTER);
      radioButtons[buttonIndex++] = xvt_win_get_ctl (win, CORNER_SWUPPER);
      xvt_ctl_check_radio_button (radioButtons[options->positionBy], radioButtons, buttonIndex);
   }

   if (win = tabs[1].win)
   {
      cwin = updateFloatTextField (win, XYZ_X, options->positionX, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Y, options->positionY, 1);
      setControlFont (cwin);
      cwin = updateFloatTextField (win, XYZ_Z, options->positionZ, 1);
      setControlFont (cwin);
      
      updateScrollAndText(win, XYZ_X_BAR, XYZ_X, FALSE);
      updateScrollAndText(win, XYZ_Y_BAR, XYZ_Y, FALSE);
      updateScrollAndText(win, XYZ_Z_BAR, XYZ_Z, FALSE);
   }

   if (win = tabs[2].win)
   {
      cwin = updateFloatTextField (win, ISCL_BSIZE, options->blockSize, 1);
      setControlFont (cwin);
   }

   if (win = tabs[3].win)
   {
      cwin = xvt_win_get_ctl (win, IPROP_PROPERTIES);
      xvt_list_suspend (cwin);
      xvt_list_add (cwin , 0, "- No Block Loaded -");
      xvt_list_resume (cwin);

      cwin = updateNumericTextField (win, IPROP_TRANSPARENT, options->transparentLayer);
      setControlFont (cwin);
   }

   return (result);
}

static int
#if XVT_CC_PROTO
saveImportOptions (OBJECT *object)
#else
saveImportOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   IMPORT_OPTIONS *options = (IMPORT_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = xvt_win_get_ctl (win, CORNER_CENTER);
      if (xvt_ctl_is_checked (cwin)) options->positionBy = POS_BY_CENTER;
      cwin = xvt_win_get_ctl (win, CORNER_SWUPPER);
      if (xvt_ctl_is_checked (cwin)) options->positionBy = POS_BY_CORNER;
   }

   if (win = tabs[1].win)
   {
      options->positionX = getFloatTextFieldValue (win, XYZ_X);
      options->positionY = getFloatTextFieldValue (win, XYZ_Y);
      options->positionZ = getFloatTextFieldValue (win, XYZ_Z);
   }

   if (win = tabs[2].win)
   {
      options->blockSize = getFloatTextFieldValue (win, ISCL_BSIZE);
   }

   if (win = tabs[3].win)
   {
      options->transparentLayer = getIntegerTextFieldValue (win, IPROP_TRANSPARENT);
   }

   return (TRUE);
}

void 
#if XVT_CC_PROTO
updateImportOptions (WINDOW win, OBJECT *object)
#else
updateImportOptions (win, object)
WINDOW win;
OBJECT *object;
#endif
{
   WINDOW cwin;
   IMPORT_OPTIONS *options;
   char text[100];
   int selIndex, layer;

   if (!object)
      object = (OBJECT *) xvt_vobj_get_data (win);
      
   options = (IMPORT_OPTIONS *) object->options;
   
   strcpy (text, "Import - ");
   if (options->blockData)
      strcat (text, options->importFile.name);
   else
      strcat (text, "NO FILE LOADED");
   xvt_vobj_set_title (win, text);


   if (win = tabs[3].win)
   {
      cwin = xvt_win_get_ctl (win, IPROP_PROPERTIES);
      selIndex = xvt_list_get_sel_index (cwin);  /* get so it wont change */
      xvt_list_suspend (cwin);
      xvt_list_clear (cwin);
      if (options->numProperties == 0)
      {
         xvt_list_add (cwin , 0, "- No Block Loaded -");
         xvt_vobj_set_enabled (cwin, FALSE);
      }
      else
      {
         for (layer = 0; layer < options->numProperties; layer++)
         {
            xvt_list_add (cwin, -1, options->properties[layer].unitName);
         }
         xvt_vobj_set_enabled (cwin, TRUE);
      }
      xvt_list_resume (cwin);

      if (selIndex >= 0)
         xvt_list_set_sel (cwin, selIndex, TRUE); /* set so it wont change */
   }

}

int
#if XVT_CC_PROTO
getImportPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getImportPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   WINDOW win;
   IMPORT_OPTIONS *options = (IMPORT_OPTIONS *) object->options;

   if (win = tabs[1].win)
   {
      *x = getFloatTextFieldValue (win, XYZ_X);
      *y = getFloatTextFieldValue (win, XYZ_Y);
      *z = getFloatTextFieldValue (win, XYZ_Z);
   }
   
   *dip = 0.0;
   *dipDir = 0.0;
   *pitch = 0.0;

   return (TRUE);
}

/* ************************************************************* */
/* ************************** Generic ************************** */
/* ************************************************************* */
static int
#if XVT_CC_PROTO
loadGenericOptions (OBJECT *object)
#else
loadGenericOptions (object)
OBJECT *object;
#endif
{
   WINDOW win, cwin;
   GENERIC_OPTIONS *options = (GENERIC_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      cwin = updateFloatTextField (win, GENERIC_1, options->param1, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_2, options->param2, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_3, options->param3, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_4, options->param4, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_5, options->param5, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_6, options->param6, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_7, options->param7, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_8, options->param8, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_9, options->param9, 1);
      setControlFont (win);
      cwin = updateFloatTextField (win, GENERIC_10, options->param10, 1);
      setControlFont (win);
   }

   return (TRUE);
}

static int
#if XVT_CC_PROTO
saveGenericOptions (OBJECT *object)
#else
saveGenericOptions (object)
OBJECT *object;
#endif
{
   WINDOW win;
   GENERIC_OPTIONS *options = (GENERIC_OPTIONS *) object->options;

   if (win = tabs[0].win)
   {
      options->param1 = getFloatTextFieldValue (win, GENERIC_1);
      options->param2 = getFloatTextFieldValue (win, GENERIC_2);
      options->param3 = getFloatTextFieldValue (win, GENERIC_3);
      options->param4 = getFloatTextFieldValue (win, GENERIC_4);
      options->param5 = getFloatTextFieldValue (win, GENERIC_5);
      options->param6 = getFloatTextFieldValue (win, GENERIC_6);
      options->param7 = getFloatTextFieldValue (win, GENERIC_7);
      options->param8 = getFloatTextFieldValue (win, GENERIC_8);
      options->param9 = getFloatTextFieldValue (win, GENERIC_9);
      options->param10= getFloatTextFieldValue (win, GENERIC_10);
   }

              /* Write Out a particular licene file */
   if (options->param10 > 0.0)
      createLicence ((int) options->param10);
/*
*/
   return (TRUE);
}

int
#if XVT_CC_PROTO
getGenericPlane (OBJECT *object,
               double *x,   double *y,      double *z,
               double *dip, double *dipDir, double *pitch)
#else
getGenericPlane (object, x, y, z, dip, dipDir, pitch)
OBJECT *object;
double *x, *y, *z, *dip, *dipDir, *pitch;
#endif
{
   return (FALSE);
}



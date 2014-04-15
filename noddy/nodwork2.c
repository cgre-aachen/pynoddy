/*
*/
#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include "scales.h"
#include "status.h"

#define DEBUG(X)    
#define DEG_TO_RAD(X)   (((double) (X))*0.01745329)

                              /* Line Map Menu items */
#define SYMBOL_OPTIONS_BASE   2100
#define EVENT1_OPTIONS_BASE   3100
#define EVENT2_OPTIONS_BASE   4100
#define DEFINE_OPTIONS_BASE   5100

#define RATIO_TOLERANCE       0.0200
#define VERSION_TOLERANCE     0.0100
#define TOLERANCE             0.0100


extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;

extern BOOLEAN processingLongJob;
extern WINDOW lastActiveWindow;

extern WINDOW blockDiagramWindow;
extern WINDOW boreHoleWindow;
extern WINDOW boreHole3DWindow;
extern WINDOW sectionWindow;
extern WINDOW lineMapWindow;
extern WINDOW lineMapPlotWindow;
extern WINDOW stratColumnsWindow;
extern WINDOW topoWindow;
extern WINDOW topo3dWindow;
extern WINDOW strat3dWindow;
extern WINDOW gravityReferenceWindow;
extern WINDOW magneticsReferenceWindow;

extern WINDOW statusBar;

extern FILE_SPEC topoFileSpec;
extern int TopoRow, TopoCol; 
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE, minTopoValue, maxTopoValue;
extern double **topographyMap;

                                /* Functions external to this file */
#if XVT_CC_PROTO
extern void xyToLineMapCoords();
extern void lineMapCoordsToXY();
extern int mapLocation ();
extern int refreshSectionSymbols (WINDOW, BLOCK_SURFACE_DATA *, RCT *,
                           double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS], int);
extern int previewBlockDiagram (BLOCK_DIAGRAM_DATA *);
extern int overlayAnomGrid (WINDOW, ANOMIMAGE_DATA *);
#else
extern void xyToLineMapCoords();
extern void lineMapCoordsToXY();
extern int mapLocation ();
extern int refreshSectionSymbols ();
extern int previewBlockDiagram ();
extern int overlayAnomGrid ();
#endif


#if XVT_CC_PROTO
extern int HandleContents (int, int, int, int, int, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS], int *, int, int);
extern int UpdateMapCoords(int,int, int, int);
extern int UpdateEventCoords(double, double, double, double, double);
extern int refreshContents (double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS], int, int, int);
extern int updateProfileImage (ANOMIMAGE_DATA *, PNT , PNT , double, double);
extern int RecallFourierCoeffs (double [2][11]);
extern int TempSymbol(int,int, OBJECTS);
extern int MoreSymbol(OBJECTS);
extern int profile_finished (double [2][11], OBJECTS);
extern void update3dSceen (THREED_IMAGE_DATA *);
extern void preview3dSceen (THREED_IMAGE_DATA *, int);
extern int getSectionLocOfPoint (BLOCK_SURFACE_DATA *, PNT, RCT *, double *, double *, double *);
extern int getSectionPointAtLoc (BLOCK_SURFACE_DATA *, double, double, double, RCT *, short *, short *);
#else
extern int HandleContents ();
extern int UpdateMapCoords();
extern int UpdateEventCoords();
extern int refreshContents ();
extern int updateProfileImage ();
extern int RecallFourierCoeffs ();
extern int TempSymbol();
extern int MoreSymbol();
extern int profile_finished ();
extern void update3dSceen ();
extern void preview3dSceen ();
extern int getSectionLocOfPoint ();
extern int getSectionPointAtLoc ();
#endif
                                /* Functions in this file */
#if XVT_CC_PROTO
long lineMapEventHandler (WINDOW, EVENT *);
long sectionEventHandler (WINDOW, EVENT *);
long wellLogEventHandler (WINDOW, EVENT *);
long blockDiagramEventHandler (WINDOW, EVENT *);
long movieEventHandler (WINDOW, EVENT *);
long traceEventHandler (WINDOW win, EVENT *ep);
long pictureEventHandler (WINDOW, EVENT *);
long anomImageEventHandler (WINDOW, EVENT *);
int drawProfileLabel (WINDOW, int, PNT, PNT, double);
long profileEventHandler (WINDOW, EVENT *);
long threedEventHandler (WINDOW, EVENT *);
void readGeophys (FILE *, unsigned char **, double **, int, int,
                          double *, double *, BOOLEAN, int);
int readGeophysComment (FILE *, char *, char *, int, int);
int readGeophysHeader (FILE *, int *, int *, int *, int *, int *,
          float [3], float [6], float *, float *, int *, int *, int *);
FILE *getTopoFile ();
int interpolateEvent (OBJECT *, int, int);
int updateMovieTitle (WINDOW win);
#else
long lineMapEventHandler ();
long sectionEventHandler ();
long wellLogEventHandler ();
long blockDiagramEventHandler ();
long movieEventHandler ();
long traceEventHandler ();
long pictureEventHandler ();
long anomImageEventHandler ();
int drawProfileLabel ();
long profileEventHandler ();
long threedEventHandler ();
void readGeophys ();
int readGeophysComment ();
int readGeophysHeader ();
FILE *getTopoFile ();
int interpolateEvent ();
int updateMovieTitle ();
#endif

/* ======================================================================
FUNCTION        lineMapEventHandler

DESCRIPTION
        well log event handler

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
lineMapEventHandler (WINDOW win, EVENT *ep)
#else
lineMapEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   SECTION_DATA *lineMapData;
   WINDOW oldCurrentDrawingWindow;
   int event1, event2, symType;
   int xcur, ycur;
   static double proposedEventX, proposedEventY, proposedEventZ;
   static int eventPosX, eventPosY;
   static short buttonDown = FALSE;
   static OBJECT *objectBeingPositioned = NULL;
   RCT position;

   switch (ep->type)
   {
      case E_CREATE:
           LINE_MAP_WINDOW_eh (win, ep);   /* do the xvt event handler stuff */
           break;
      case E_DESTROY:
                             /* make sure current window is not invalid */
           if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;

           if (win == lineMapWindow)
           {
              lineMapWindow = NULL_WIN;
              setCurrentDrawingWindow (NULL_WIN);
           }
           lineMapData = (SECTION_DATA *) xvt_vobj_get_data (win);
           xvt_pmap_destroy (lineMapData->pixmap);
           xvt_mem_free ((char *) lineMapData);
           xvt_vobj_set_data (win, 0L);

           updateMenuOptions (TASK_MENUBAR, win);
           break;
      case E_CLOSE:
           xvt_vobj_destroy (win);
           break;
      case E_COMMAND:
           if (!(lineMapData = (SECTION_DATA *) xvt_vobj_get_data (win)))
              return (0L);
                            /* get the current Settings */
           event1 = lineMapData->event1;
           event2 = lineMapData->event2;
           symType = lineMapData->symbol;
           if ((ep->v.cmd.tag > DEFINE_OPTIONS_BASE)
                                 && (ep->v.cmd.tag < DEFINE_OPTIONS_BASE+500))
           {                  /* Move the position of Event 1 */
              OBJECT *object;
              double x, y, z;
              
              if (win == lineMapWindow)
              {
                 object = (OBJECT *) nthObject (NULL_WIN, event1);
              
                 if (getEventPosition (object, &x, &y, &z))
                 {                                              
                    proposedEventX = x;
                    proposedEventY = y;
                    proposedEventZ = z;
                    lineMapCoordsToXY (x, y, &eventPosX, &eventPosY); 
                    xvt_dwin_set_fore_color (win, COLOR_BLACK);
                    xvt_dwin_set_back_color (win, COLOR_WHITE);
                    xvt_dwin_draw_icon (win, eventPosX, eventPosY,
                                                        EVENT_POSITION_ICON);
                    objectBeingPositioned = object;
                 }
                 else
                 {
                    xvt_dm_post_error ("Error, '%s' does not have a Position");
                 }
              }
              else
              {
                 if (lineMapWindow)
                    xvt_dm_post_error("Only the last Line Map is active.");
                 else
                    xvt_dm_post_error("Error, Event History Changed - Create another Line Map.");
              }
           }
           else if ((ep->v.cmd.tag > EVENT2_OPTIONS_BASE)
                                 && (ep->v.cmd.tag < EVENT2_OPTIONS_BASE+500))
           {
                   /* uncheck what was selected get event number of event  
                   ** then check what the new option */
              xvt_menu_set_item_checked (win, (MENU_TAG) (event2 + EVENT2_OPTIONS_BASE), (BOOLEAN) FALSE);
              event2 = ep->v.cmd.tag - EVENT2_OPTIONS_BASE;
              xvt_menu_set_item_checked (win, ep->v.cmd.tag, (BOOLEAN) TRUE);
           }
           else if ((ep->v.cmd.tag > EVENT1_OPTIONS_BASE)

                                 && (ep->v.cmd.tag < EVENT1_OPTIONS_BASE+500))
           {
                   /* uncheck what was selected get event number of event  **
                   ** then check what the new option */
              xvt_menu_set_item_checked (win, (MENU_TAG) (event1 + EVENT1_OPTIONS_BASE), (BOOLEAN) FALSE);
              event1 = ep->v.cmd.tag - EVENT1_OPTIONS_BASE;
              xvt_menu_set_item_checked (win, ep->v.cmd.tag, TRUE);
           }
           else if ((ep->v.cmd.tag > SYMBOL_OPTIONS_BASE)
                                 && (ep->v.cmd.tag < SYMBOL_OPTIONS_BASE+500))
           {
                   /* uncheck what was selected get event number of event  **
                   ** then check what the new option */
              xvt_menu_set_item_checked (win, (MENU_TAG) (symType + SYMBOL_OPTIONS_BASE), FALSE);
              symType = ep->v.cmd.tag - SYMBOL_OPTIONS_BASE;
              xvt_menu_set_item_checked (win, ep->v.cmd.tag, TRUE);
           }
           else
              do_TASK_MENUBAR(win, ep);
                            /* store the new Settings */
           lineMapData->symbol = symType;
           lineMapData->event1 = event1;
           lineMapData->event2 = event2;
           break;
      case E_HSCROLL:
      case E_VSCROLL:
                         /* did some strange things temporaryly removed */
           /* win_119_eh (win, ep);   do the xvt event handler stuff */
           break;
      case E_MOUSE_DOWN:
           if (objectBeingPositioned && (win == lineMapWindow))
           {
              BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
              buttonDown = TRUE;
                          /* the object is positioned at the level of the line map */
              proposedEventZ = viewOptions->originZ - viewOptions->lengthZ;
           }
           break;
      case E_MOUSE_DBL:
           break;
      case E_MOUSE_UP:
            
           if (win == lineMapWindow) /* only the latest window is live */
           {
              xcur = ep->v.mouse.where.h;
              ycur = ep->v.mouse.where.v;
              if (objectBeingPositioned)  /* position event */
              {
                 double x, y, z;
                 ASK_RESPONSE response;
                 
                 buttonDown = FALSE;
                
                 getEventPosition (objectBeingPositioned,
                                                  &x, &y, &z);
                 xyToLineMapCoords (xcur, ycur, &x, &y); 
                // response = xvt_dm_post_ask ("Yes", "No", NULL,
                          //  "Are you sure you want to move '%s' to (%.1f, %.1f)",
                                         //      objectBeingPositioned->text, x, y);
                 if (response == RESP_DEFAULT) /* Set if Sure */
                 {
                    setEventPosition (objectBeingPositioned, x, y, z);
                    invalidateCurrentLineMap ();
                 }
                 objectBeingPositioned = NULL;
                 xvt_dwin_invalidate_rect (win, NULL);
                 xvt_dwin_update (win);
              }
              else  /* mark a point in the linemap */
              {
                 oldCurrentDrawingWindow = getCurrentDrawingWindow ();
                 setCurrentDrawingWindow (win);
                      /* item - OPTIONS_BASE gets back to the number of
                      ** the item in the menu structure eg. Bedding = 1 */
                 if (lineMapData = (SECTION_DATA *) xvt_vobj_get_data (win))
                 {
                    if (lineMapData->count < (MAX_LINEMAP_EVENTS-2))
                    {
                       HandleContents (xcur, ycur,
                          lineMapData->symbol + 2, lineMapData->event1,
                          lineMapData->event2, lineMapData->orientations,
                          &(lineMapData->count), 0, 0);
                    }
                 }
                 setCurrentDrawingWindow (oldCurrentDrawingWindow);
              }
           }
           else
           {
              if (lineMapWindow)
                 xvt_dm_post_error("Only the last Line Map is active.");
              else
                 xvt_dm_post_error("Error, Event History Changed - Create another Line Map.");
           }
           break;
      case E_MOUSE_MOVE:
           if (win == lineMapWindow) /* only the latest window is live */
           {
              xcur = ep->v.mouse.where.h;
              ycur = ep->v.mouse.where.v;
              oldCurrentDrawingWindow = getCurrentDrawingWindow ();
              setCurrentDrawingWindow (win);
              UpdateMapCoords (xcur, ycur, 0, 0);
                                  /* when positining an event also show its position */
              if (objectBeingPositioned)
              {
                 if (buttonDown)
                 {
                    xyToLineMapCoords (xcur, ycur, &proposedEventX, &proposedEventY); 
                    xvt_rect_set (&position, (short) eventPosX, (short) eventPosY,
                                 (short) (eventPosX + ICON_SIZE), (short) (eventPosY + ICON_SIZE));
                    eventPosX = xcur;
                    eventPosY = ycur;             
                    xvt_dwin_invalidate_rect (win, &position);
                    xvt_dwin_update (win);
                 }
                 UpdateEventCoords (proposedEventX, proposedEventY, proposedEventZ, 0, 0);
              }
              setCurrentDrawingWindow (oldCurrentDrawingWindow);
           }
           break;
      case E_SIZE:
           if (!xvt_vobj_get_data (win))
              break;
           xvt_dwin_invalidate_rect (win, NULL);
           break;
      case E_UPDATE:
           if (!xvt_vobj_get_data (win))
              break;

           if (lineMapData = (SECTION_DATA *) xvt_vobj_get_data (win))
           {                    
              RCT pixmapSize;
              xvt_vobj_get_client_rect (lineMapData->pixmap, &pixmapSize);
              position = pixmapSize;
              xvt_dwin_draw_pmap (win, lineMapData->pixmap,
                                                  &position, &pixmapSize);
              oldCurrentDrawingWindow = getCurrentDrawingWindow ();
              setCurrentDrawingWindow (lineMapData->pixmap);
              refreshContents (lineMapData->orientations,
                               lineMapData->count, 0, 0);
              setCurrentDrawingWindow (oldCurrentDrawingWindow);
              if (objectBeingPositioned)
                 xvt_dwin_draw_icon(win, eventPosX, eventPosY,
                                         EVENT_POSITION_ICON);
           }
           break;
      case E_FOCUS:
         lastActiveWindow = win;
         break;
   }
   return 0L;
}

/* ======================================================================
FUNCTION        sectionEventHandler

DESCRIPTION
        well log event handler

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
sectionEventHandler (WINDOW win, EVENT *ep)
#else
sectionEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   int width, height, realWidth, realHeight;
   double xScale, yScale;
   int event1, event2, symType;
   XVT_PIXMAP diagram;
   RCT position, realSize;
   SECTION_DATA *sectionData;
   char label[80], message[40];
   double xLoc, yLoc, zLoc;
                 /* Variables for moving event positions */
   static short eventPosX, eventPosY;
   static short buttonDown = FALSE;
   static OBJECT *objectBeingPositioned = NULL;

   
   switch (ep->type)
   {
      case E_CREATE:
         updateMenuOptions (TASK_MENUBAR, win);
         break;
      case E_DESTROY:
                          /* make sure current window is not invalid */
         if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
           setCurrentDrawingWindow (NULL_WIN);
         if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
           lastActiveWindow = NULL_WIN;
         
         if (win == sectionWindow)
           sectionWindow = NULL_WIN;
         if (win == lineMapWindow)
           lineMapWindow = NULL_WIN;
         if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
         {
            xvt_pmap_destroy (sectionData->pixmap);
            if (sectionData->surfaceData)
               freeSurfaceData(sectionData->surfaceData);
         }
         xvt_vobj_set_data (win, 0L);
         
         updateMenuOptions (TASK_MENUBAR, win);
         break;
      case E_CLOSE:
         xvt_vobj_destroy (win);
         break;
      case E_COMMAND:
         if (!(sectionData = (SECTION_DATA *) xvt_vobj_get_data (win)))
           return (0L);
                         /* get the current Settings */
         event1 = sectionData->event1;
         event2 = sectionData->event2;
         symType = sectionData->symbol;
 
         if ((ep->v.cmd.tag > DEFINE_OPTIONS_BASE)
                               && (ep->v.cmd.tag < DEFINE_OPTIONS_BASE+500))
         {                  /* Move the position of Event 1 */
            OBJECT *object;
            double x, y, z;
              
            if ((win == lineMapWindow) || (win == sectionWindow))
            {
               object = (OBJECT *) nthObject (NULL_WIN, event1);
              
               if (getEventPosition (object, &x, &y, &z))
               {                                              
                  xvt_vobj_get_client_rect (win, &position);
                  getSectionPointAtLoc (sectionData->surfaceData, x, y, z, &position, &eventPosX, &eventPosY);

                  xvt_dwin_set_fore_color (win, COLOR_BLACK);
                  xvt_dwin_set_back_color (win, COLOR_WHITE);
                  xvt_dwin_draw_icon (win, (int) eventPosX, (int) eventPosY,
                                                      EVENT_POSITION_ICON);
                  objectBeingPositioned = object;
               }
               else
               {
                  xvt_dm_post_error ("Error, '%s' does not have a Position");
               }
            }
            else
            {
               if (lineMapWindow)
                  xvt_dm_post_error("Only the last Line Map is active.");
               else
                  xvt_dm_post_error("Error, Event History Changed - Create another Line Map.");
            }
         }
         else if ((ep->v.cmd.tag > EVENT2_OPTIONS_BASE)
                              && (ep->v.cmd.tag < EVENT2_OPTIONS_BASE+500))
         {
                /* uncheck what was selected get event number of event  
                ** then check what the new option */
           xvt_menu_set_item_checked (win, (MENU_TAG) (event2 + EVENT2_OPTIONS_BASE), FALSE);
           event2 = ep->v.cmd.tag - EVENT2_OPTIONS_BASE;
           xvt_menu_set_item_checked (win, ep->v.cmd.tag, TRUE);
         }
         else if ((ep->v.cmd.tag > EVENT1_OPTIONS_BASE)
         
                              && (ep->v.cmd.tag < EVENT1_OPTIONS_BASE+500))
         {
                /* uncheck what was selected get event number of event  **
                ** then check what the new option */
           xvt_menu_set_item_checked (win, (MENU_TAG) (event1 + EVENT1_OPTIONS_BASE), FALSE);
           event1 = ep->v.cmd.tag - EVENT1_OPTIONS_BASE;
           xvt_menu_set_item_checked (win, ep->v.cmd.tag, TRUE);
         }
         else if ((ep->v.cmd.tag > SYMBOL_OPTIONS_BASE)
                              && (ep->v.cmd.tag < SYMBOL_OPTIONS_BASE+500))
         {
                /* uncheck what was selected get event number of event  **
                ** then check what the new option */
           xvt_menu_set_item_checked (win, (MENU_TAG) (symType + SYMBOL_OPTIONS_BASE), FALSE);
           symType = ep->v.cmd.tag - SYMBOL_OPTIONS_BASE;
           xvt_menu_set_item_checked (win, ep->v.cmd.tag, TRUE);
         }
         else
           do_TASK_MENUBAR(win, ep);
                         /* store the new Settings */
         sectionData->symbol = symType;
         sectionData->event1 = event1;
         sectionData->event2 = event2;
         break;
      case E_HSCROLL:
      case E_VSCROLL:
      case E_MOUSE_DOWN:
           if (objectBeingPositioned && ((win == lineMapWindow) || (win == sectionWindow)))
           {
              buttonDown = TRUE;
           }
           break;
      case E_MOUSE_DBL:
         break;
      case E_MOUSE_MOVE:
         if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
         {
            if (objectBeingPositioned)
            {
               strcpy (message, "Press to relocate event.");
            }
            else
            {
               switch (sectionData->symbol)
               {
                  case (BEDDING_SYM):
                     strcpy (message, "Press to locate Bedding");
                     break;
                  case (FOLIATION_SYM):
                     strcpy (message, "Press to locate Foliation");
                     break;
                  case (LINEATION_SYM):
                     strcpy (message, "Press to locate Lineation");
                     break;
                  case (BD_CL_SYM):
                     strcpy (message, "Press to locate Bedding/Clevage");
                     break;
                  case (CL_CL_SYM):
                     strcpy (message, "Press to locate Cleavage/Cleavage");
                     break;
                  default:
                     strcpy (message, "");
               }
            }
            if (sectionData->surfaceData)  /* Work out our position */
            {
               xvt_vobj_get_client_rect (win, &position);
               if (!getSectionLocOfPoint (sectionData->surfaceData,
                       ep->v.mouse.where, &position, &xLoc, &yLoc, &zLoc))
               {
                  xLoc = yLoc = zLoc = 0.0;
               }
            }
            sprintf (label,"%s\tX:%.0lf\tY:%.0lf\tZ:%.0lf",message, xLoc, yLoc, zLoc);
            statbar_set_title (statusBar, label);
            
                                /* when positining an event also show its position */
            if (objectBeingPositioned)
            {
               if (buttonDown)
               {
                  eventPosX = ep->v.mouse.where.h;
                  eventPosY = ep->v.mouse.where.v;
                  xvt_dwin_invalidate_rect (win, NULL);
                  xvt_dwin_update (win);
               }
            }
         }
         break;
      case E_MOUSE_UP:
         if ((win == sectionWindow) || (win == lineMapWindow)) /* only the latest window is live */
         {
            if (objectBeingPositioned)  /* position event */
            {
               double x, y, z;
               ASK_RESPONSE response;
                          
               if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
               {
                  getEventPosition (objectBeingPositioned, &x, &y, &z);
   
                  xvt_vobj_get_client_rect (win, &position);
                  if (getSectionLocOfPoint (sectionData->surfaceData,
                          ep->v.mouse.where, &position, &xLoc, &yLoc, &zLoc))
                  {
                  //   response = xvt_dm_post_ask ("Yes", "No", NULL,
                              // "Are you sure you want to move '%s' from (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f)",
                                             //     objectBeingPositioned->text, x, y, z, xLoc, yLoc, zLoc);
                     if (response == RESP_DEFAULT) /* Set if Sure */
                     {
                        setEventPosition (objectBeingPositioned, xLoc, yLoc, zLoc);
                        invalidateCurrentLineMap ();
                     }
                  }
               }
               objectBeingPositioned = NULL;
               xvt_dwin_invalidate_rect (win, NULL);
               xvt_dwin_update (win);
            }
            else  /* mark a point in the section */
            {
               setCurrentDrawingWindow (win);
                   /* item - OPTIONS_BASE gets back to the number of
                   ** the item in the menu structure eg. Bedding = 1 */
               if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
               {
                  if (sectionData->count < (MAX_LINEMAP_EVENTS-2))
                  {
                     xvt_vobj_get_client_rect (win, &position);
                     if (getSectionLocOfPoint (sectionData->surfaceData,
                             ep->v.mouse.where, &position, &xLoc, &yLoc, &zLoc))
                     {
                        if (mapLocation (xLoc, yLoc, zLoc,
                               sectionData->symbol + 2, sectionData->event1,
                               sectionData->event2, sectionData->orientations,
                               &(sectionData->count)))
                           xvt_dwin_invalidate_rect (win, NULL);
								updateLegendData(win, SECTION_LEGENDS);
                     }
                  }
               }
            }
         }
         else
         {
            if ((sectionWindow) || (lineMapWindow))
               xvt_dm_post_error("Only the last Section is active.");
            else
               xvt_dm_post_error("Error, Event History Changed - Create another Line Map.");
         }
         bringWindowToFront(win);
         buttonDown = FALSE;
         break;
      case E_SIZE:
         if (!xvt_vobj_get_data (win))
           break;
         xvt_vobj_get_client_rect (win, &position);
                     /* keep the XY ratio constant when resizing window
                     ** so the image does not distory with scaling */
         width = xvt_rect_get_width (&position);
         height = xvt_rect_get_height (&position);
         
         if (!(diagram = getWindowPixmap (win)))
         {
           xvt_dwin_invalidate_rect (win, NULL);
           xvt_dwin_update (win);
           break;
         }
         xvt_vobj_get_client_rect (diagram, &realSize);
         realWidth = xvt_rect_get_width (&realSize);
         realHeight = xvt_rect_get_height (&realSize);
         
         xScale = (double) width/realWidth;
         yScale = (double) height/realHeight;
         
         if (fabs(xScale - yScale) > RATIO_TOLERANCE)
         {
           if (yScale < xScale)
           {
              position.right = position.left + (short) (yScale * realWidth);
              position.bottom = position.top + (short) (yScale * realHeight);
           }
           else
           {
              position.bottom = position.top + (short) (xScale * realHeight);
              position.right = position.left + (short) (xScale * realWidth);
           }
           CORRECT_WIN_RESIZE(win, position)
         
           xvt_vobj_move (win, &position);
         }
         xvt_dwin_invalidate_rect (win, NULL);
		   updateLegendData(win, SECTION_LEGENDS);
         break;
      case E_UPDATE:
         if (!xvt_vobj_get_data (win))
           break;
         
         xvt_vobj_get_client_rect (win, &position);
         if (sectionData = (SECTION_DATA *) xvt_vobj_get_data (win))
         {                    
            RCT pixmapSize;
                       
            xvt_vobj_get_client_rect (sectionData->pixmap, &pixmapSize);
            xvt_dwin_draw_pmap (win, sectionData->pixmap,
                                               &position, &pixmapSize);
            refreshSectionSymbols (win, sectionData->surfaceData, &position,
                   sectionData->orientations, sectionData->count);

            if (objectBeingPositioned)
            {
               xvt_dwin_set_fore_color (win, COLOR_BLACK);
               xvt_dwin_set_back_color (win, COLOR_WHITE);
               xvt_dwin_draw_icon(win, (int) eventPosX, (int) eventPosY, EVENT_POSITION_ICON);
            }
         }
         break;
      case E_FOCUS:
         lastActiveWindow = win;
			updateLegendData(win, SECTION_LEGENDS);
         break;
   }
   return 0L;
}
/* ======================================================================
FUNCTION        wellLogEventHandler

DESCRIPTION
        well log event handler

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
wellLogEventHandler (WINDOW win, EVENT *ep)
#else
wellLogEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   int width, height, realWidth, realHeight;
   double xScale, yScale;
   XVT_PIXMAP diagram;
   RCT position, realSize;

   switch (ep->type)
   {
      case E_CREATE:
           updateMenuOptions (TASK_MENUBAR, NULL_WIN);
           break;
      case E_DESTROY:
                             /* make sure current window is not invalid */
           if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;

           if (win == boreHoleWindow)
              boreHoleWindow = NULL_WIN;
           diagram = (XVT_PIXMAP) xvt_vobj_get_data (win);
           xvt_pmap_destroy (diagram);
           xvt_vobj_set_data (win, 0L);

           updateMenuOptions (TASK_MENUBAR, win);
           break;
      case E_CLOSE:
           xvt_vobj_destroy (win);
           break;
      case E_COMMAND:
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
           do_TASK_MENUBAR(win, ep);
#endif
           break;
      case E_HSCROLL:
      case E_VSCROLL:
      case E_MOUSE_DOWN:
      case E_MOUSE_DBL:
      case E_MOUSE_MOVE:
           break;
      case E_MOUSE_UP:
           bringWindowToFront(win);
           break;
      case E_SIZE:
           if (!xvt_vobj_get_data (win))
              break;
           xvt_vobj_get_client_rect (win, &position);
                        /* keep the XY ratio constant when resizing window
                        ** so the image does not distory with scaling */
           width = xvt_rect_get_width (&position);
           height = xvt_rect_get_height (&position);

           if (!(diagram = getWindowPixmap (win)))
           {
              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
              break;
           }
           xvt_vobj_get_client_rect (diagram, &realSize);
           realWidth = xvt_rect_get_width (&realSize);
           realHeight = xvt_rect_get_height (&realSize);

           xScale = (double) width/realWidth;
           yScale = (double) height/realHeight;

           if (fabs(xScale - yScale) > RATIO_TOLERANCE)
           {
              if (yScale < xScale)
              {
                 position.right = position.left + (short) (yScale * realWidth);
                 position.bottom = position.top + (short) (yScale * realHeight);
              }
              else
              {
                 position.bottom = position.top + (short) (xScale * realHeight);
                 position.right = position.left + (short) (xScale * realWidth);
              }
              CORRECT_WIN_RESIZE(win, position)

              xvt_vobj_move (win, &position);
           }
           xvt_dwin_invalidate_rect (win, NULL);
           break;
      case E_UPDATE:
           if (!xvt_vobj_get_data (win))
              break;

           xvt_vobj_get_client_rect (win, &position);
           if (diagram = (XVT_PIXMAP) xvt_vobj_get_data (win))
           {
              RCT pixmapSize;
              xvt_vobj_get_client_rect (diagram, &pixmapSize);
              xvt_dwin_draw_pmap (win, diagram, &position, &pixmapSize);
           }
           break;
      case E_FOCUS:
         lastActiveWindow = win;
         break;
  }
   return 0L;
}


/* ======================================================================
FUNCTION        blockDiagramEventHandler

DESCRIPTION
        block diagram event handler

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
blockDiagramEventHandler (WINDOW win, EVENT *ep)
#else
blockDiagramEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
#define MIN_SPIN_MOVEMENT  3.0   /* Number of pixels to move in a direction to spin block */
   static PNT startMove = { 0, 0 };
	static short shiftDown = FALSE, zoomButtonDown = FALSE;
   static short previewDone = FALSE, updatingImage = FALSE;
   double declination, azimuth, scale;
   PNT endMove;
   int width, height;
   double decInc, eleInc, scaleInc;
   double xScale, yScale;
   BLOCK_DIAGRAM_DATA *diagram;
   BLOCK_SURFACE_DATA *surface;
   XVT_PIXMAP pixmap;
   RCT position;
   XVT_PIXMAP newPixmap;
   char message[100];

   switch (ep->type)
   {
      case E_CREATE:
      /*     updateMenuOptions (TASK_MENUBAR, NULL_WIN); */
           break;
      case E_DESTROY:
                             /* make sure current window is not invalid */
           if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;

           if (win == blockDiagramWindow)
              blockDiagramWindow = NULL_WIN;
           if (win == boreHole3DWindow)
              boreHole3DWindow = NULL_WIN;
           diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win);
           
                           /* Free memory used for block Data */
           if (diagram->threedData.pixmap)
              xvt_pmap_destroy (diagram->threedData.pixmap);
           diagram = (BLOCK_DIAGRAM_DATA *) freeBlockDiagramData (diagram, TRUE);
           xvt_vobj_set_data (win, 0L);

           updateMenuOptions (TASK_MENUBAR, win);
           break;
      case E_CLOSE:
           xvt_vobj_destroy (win);
           break;
      case E_COMMAND:
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
           do_TASK_MENUBAR(win, ep);
#endif
           break;
      case E_HSCROLL:
      case E_VSCROLL:
      case E_MOUSE_DOWN:
           if (ep->v.mouse.button == 2)  /* Zoom */
				  zoomButtonDown = TRUE;
           if (ep->v.mouse.button == 0 || ep->v.mouse.button == 2)
           {
              if (processingLongJob || updatingImage)
                 break;
              if (ep->v.mouse.shift)  /* Zoom */
					  shiftDown = TRUE;
				  else
					  shiftDown = FALSE;
              startMove = ep->v.mouse.where;
              previewDone = FALSE;
              xvt_win_trap_pointer (win);
           }
           else  /* put up popup Menu */
           {
				  if (diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win))
				  {
					  if (diagram->boreHoles)   /* Dont popup menu if it is a borehole display */
				        break;
	           }

              lastActiveWindow = win;
              ep->v.mouse.where.h -= 10;
              ep->v.mouse.where.v -= 10;
              createPopupWindow (win, ep->v.mouse.where,
                          LAYER_DISPLAY_WINDOW, LAYER_DISPLAY_WINDOW_eh,
                          (long) xvt_vobj_get_data (win));
           }
           break;
      case E_MOUSE_DBL:
           if (diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win))
           {
              if (surface = (BLOCK_SURFACE_DATA *) findSurfaceInImage (
                                            ep->v.mouse.where, diagram))
                 updateSection (TRUE, surface);
           }
           startMove.h = 0; startMove.v = 0;
           break;
      case E_MOUSE_UP:
           if (ep->v.mouse.button == 2)  /* Zoom */
				  zoomButtonDown = FALSE;
           if (processingLongJob || updatingImage)
              break;
           if (ep->v.mouse.button != 0)
              break;

           endMove = ep->v.mouse.where;
           decInc = (double) (endMove.h-startMove.h);
           eleInc = (double) (endMove.v-startMove.v);
           scaleInc = (double) (endMove.v-startMove.v);
                               /* raise window if just a click in the window */
           if ((fabs(decInc) < MIN_SPIN_MOVEMENT) && (fabs(eleInc) < MIN_SPIN_MOVEMENT)
                                                  && !previewDone)
           {
              bringWindowToFront(win);
              startMove.h = 0; startMove.v = 0;
           }
           else if (diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win))
           {                   /* move the position of the threed view */
              updatingImage = TRUE;
              startMove.h = 0; startMove.v = 0;
              previewDone = FALSE;
              getCameraPosition (&(diagram->threedData),
                                 &declination, &azimuth, &scale);

              if (ep->v.mouse.shift || shiftDown || zoomButtonDown)  /* Zoom */
              {
                 scaleInc = (scale/100.0)*scaleInc;
                 setCameraPosition (&(diagram->threedData),
                     declination, azimuth, scale + scaleInc);
              }
              else  /* Pan */
              {
                 decInc = decInc/4.0;
                 eleInc = eleInc/4.0;
                 setCameraPosition (&(diagram->threedData),
                     declination + decInc, azimuth + eleInc, scale);
              }

#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
              xvt_win_set_cursor (TASK_WIN, CURSOR_WAIT);
#endif

              updateBlockDiagram (win);

#if (XVTWS != MTFWS) && (XVTWS != XOLWS)
              xvt_win_set_cursor (TASK_WIN, CURSOR_ARROW);
#endif
              updatingImage = FALSE;
           }
           xvt_win_release_pointer ();
           break;
      case E_MOUSE_MOVE:
           if (startMove.h || startMove.v)
           {
              if (diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win))
              {
                 endMove = ep->v.mouse.where;
                 decInc = (double) (endMove.h-startMove.h);
                 eleInc = (double) (endMove.v-startMove.v);
                 scaleInc = (double) (endMove.v-startMove.v);
                                              
                 if ((fabs(decInc) < MIN_SPIN_MOVEMENT) && (fabs(eleInc) < MIN_SPIN_MOVEMENT))
                    break;     /* Dont move unless worth it */
                                                               
                 getCameraPosition (&(diagram->threedData),
                                    &declination, &azimuth, &scale);
                 
                 if (ep->v.mouse.shift || shiftDown || zoomButtonDown)  /* Zoom */
                 {
                    scaleInc = (scale/100.0)*scaleInc;
                    setCameraPosition (&(diagram->threedData),
                        declination, azimuth, scale + scaleInc);
                    sprintf (message, "%s\t%.0lf Rot.\t%.0lf Az.\t%.0lf %%", STATUS_BLOCK_HELP, declination, azimuth, (scale + scaleInc)*100.0);
                 }
                 else  /* Pan */
                 {
                    decInc = decInc/4.0;
                    eleInc = eleInc/4.0;
                    setCameraPosition (&(diagram->threedData),
                        declination + decInc, azimuth + eleInc, scale);
                    sprintf (message, "%s\t%.0lf Rot.\t%.0lf Az.\t%.0lf %%", STATUS_BLOCK_HELP, declination - decInc, azimuth + eleInc, scale*100.0);
                 }
                 statbar_set_title (statusBar, message);
   
                 previewBlockDiagram (diagram);

                 xvt_dwin_invalidate_rect (win, NULL);
                 xvt_dwin_update (win);

                 startMove = endMove;
                 previewDone = TRUE;
              }
           }
           break;
      case E_SIZE:
           if (!(diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win)))
              break;
           getCameraPosition (&(diagram->threedData), &declination, &azimuth, &scale);

                                /* Size of the window */
           xvt_vobj_get_client_rect (win, &position);
           width = xvt_rect_get_width (&position);
           height = xvt_rect_get_height (&position);

           if (!(pixmap = diagram->threedData.pixmap))
           {
              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
              break;
           }

           if (newPixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT, (short) width, (short) height, NULL))
           {                 /* Get the old size of the pixmap */
              RCT pixmapSize;
              xvt_vobj_get_client_rect (pixmap, &pixmapSize);    
                            /* Draw the image stretched to the new window size */
              xvt_dwin_draw_pmap (newPixmap, pixmap, &position, &pixmapSize);
                            /* Adjust scale to roughly match the image size that is now drawn */
              xScale = (double) ((double) width)/((double) xvt_rect_get_width(&pixmapSize));
              yScale = (double) ((double) height)/((double) xvt_rect_get_height(&pixmapSize));
              scale = scale * MIN(xScale,yScale);
              setCameraPosition (&(diagram->threedData), declination, azimuth, scale);
              
              diagram->threedData.pixmap = newPixmap;
              xvt_pmap_destroy (pixmap);
           }

           xvt_dwin_invalidate_rect (win, NULL);
           break;
      case E_UPDATE:
           if (!xvt_vobj_get_data (win))
              break;

           xvt_vobj_get_client_rect (win, &position);
           if (diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win))
           {
              RCT pixmapSize;
              xvt_vobj_get_client_rect (diagram->threedData.pixmap, &pixmapSize);
              xvt_dwin_draw_pmap (win, diagram->threedData.pixmap, &position, &pixmapSize);
           }
           break;
      case E_FOCUS:
         if (ep->v.active)
         {
            if (diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win))
            {
               getCameraPosition (&(diagram->threedData), &declination, &azimuth, &scale);
               sprintf (message, "%s\t%.0lf Rot.\t%.0lf Az.\t%.0lf %%", STATUS_BLOCK_HELP,
                                 declination, azimuth, scale*100.0);
               statbar_set_title (statusBar, message);
            }
            lastActiveWindow = win;
				updateLegendData(win, BLOCK_DIAGRAM_LEGENDS);
         }
         break;
   }
   return 0L;
}

/* ======================================================================
FUNCTION        movieEventHandler

DESCRIPTION
        movie event handler

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
movieEventHandler (WINDOW win, EVENT *ep)
#else
movieEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   int width, height, realWidth, realHeight;
   double xScale, yScale;
   XVT_PIXMAP diagram;
   MOVIE_DATA *movieData, *oldMovieData;
   RCT position, realSize;
	static short forwardPlay = TRUE;

   switch (ep->type)
   {
      case E_CREATE:
           updateMenuOptions (TASK_MENUBAR, NULL_WIN);
           updateMovieTitle (win);
           break;
      case E_DESTROY:
                             /* make sure current window is not invalid */
           if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;

           movieData = (MOVIE_DATA *) xvt_vobj_get_data (win);
           while (movieData->prev)    /* move to start of movie */
              movieData = movieData->prev;
           while (movieData)
           {
              oldMovieData = movieData;
              movieData = oldMovieData->next;
              xvt_pmap_destroy (oldMovieData->pixmap);
              xvt_mem_free ((char *) oldMovieData);
           }
           xvt_vobj_set_data (win, 0L);

           updateMenuOptions (TASK_MENUBAR, win);
           break;
      case E_CLOSE:
           xvt_vobj_destroy (win);
           break;
      case E_COMMAND:
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
           do_TASK_MENUBAR(win, ep);
#endif
           break;
      case E_HSCROLL:
      case E_VSCROLL:
      case E_MOUSE_DOWN:
           break;
      case E_MOUSE_DBL:
           break;
      case E_MOUSE_MOVE:
           break;
      case E_TIMER:
           if (!(movieData = (MOVIE_DATA *) xvt_vobj_get_data (win)))
              xvt_timer_destroy (ep->v.timer.id);
			  
			  if (forwardPlay)
			  {
				  if (movieData->next)  /* move along to the next frame */
				  {
					  movieData = movieData->next;
					  xvt_vobj_set_data (win, (long) movieData);
					  updateMovieTitle (win);
					  xvt_dwin_invalidate_rect (win, NULL);
					  xvt_dwin_update (win);
				  }
				  else  /* at end so remove the timer */
					  xvt_timer_destroy (ep->v.timer.id);
			  }
			  else
			  {
				  if (movieData->prev)  /* move along to the prev frame */
				  {
					  movieData = movieData->prev;
					  xvt_vobj_set_data (win, (long) movieData);
					  updateMovieTitle (win);
					  xvt_dwin_invalidate_rect (win, NULL);
					  xvt_dwin_update (win);
				  }
				  else  /* at end so remove the timer */
					  xvt_timer_destroy (ep->v.timer.id);
			  }
              
           break;
      case E_MOUSE_UP:
           if (!(movieData = (MOVIE_DATA *) xvt_vobj_get_data (win)))
              return 0L;
                                 /* Play movie from start */
           if (ep->v.mouse.shift || ep->v.mouse.control)
           {              
              if (ep->v.mouse.button == 0)  /* Forward */
              {
					  while (movieData->prev)    /* move to start of movie */
						  movieData = movieData->prev;
					  forwardPlay = TRUE;
				  }
				  else
				  {
					  while (movieData->next)    /* move to end of movie */
						  movieData = movieData->next;
					  forwardPlay = FALSE;
				  }

              if (projectOptions.moviePlaySpeed)
              {
                 xvt_vobj_set_data (win, (long) movieData);
					  xvt_dwin_invalidate_rect (win, NULL);
                 updateMovieTitle (win);
					  xvt_dwin_update (win);
                 xvt_timer_create(win,
                                  (long) projectOptions.moviePlaySpeed*100);
              }
              else  /* just play it as fast as we can */
              {
					  if (forwardPlay)
					  {
						  do     /* play the movie */
						  {
							  xvt_vobj_set_data (win, (long) movieData);
							  xvt_dwin_invalidate_rect (win, NULL);
							  updateMovieTitle (win);
							  xvt_dwin_update (win);
							  movieData = movieData->next;
						  } while (movieData);       
					  }
					  else
					  {
						  do     /* play the movie backwards */
						  {
							  xvt_vobj_set_data (win, (long) movieData);
							  xvt_dwin_invalidate_rect (win, NULL);
							  updateMovieTitle (win);
							  xvt_dwin_update (win);
							  movieData = movieData->prev;
						  } while (movieData);       
					  }
              }
           }
           else   /* Move through ONE frame at a time */
           {
              if (ep->v.mouse.button == 0)  /* Forward */
              {
                 if (movieData->next)
                    movieData = movieData->next;
                 else
                 {
                    while (movieData->prev)  /* move to start of movie */
                       movieData = movieData->prev;
                 }
              }
              else                          /* Backward */
              {
                 if (movieData->prev)
                    movieData = movieData->prev;
                 else
                 {
                    while (movieData->next)  /* move to end of movie */
                       movieData = movieData->next;
                 }
              }
              
              if (movieData)
                 xvt_vobj_set_data (win, (long) movieData);
           
              updateMovieTitle (win);
              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
           }
           break;
      case E_SIZE:
           if (!xvt_vobj_get_data (win))
              break;
           xvt_vobj_get_client_rect (win, &position);
                        /* keep the XY ratio constant when resizing window
                        ** so the image does not distory with scaling */
           width = xvt_rect_get_width (&position);
           height = xvt_rect_get_height (&position);

           if (!(diagram = getWindowPixmap (win)))
           {
              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
              break;
           }
           xvt_vobj_get_client_rect (diagram, &realSize);
           realWidth = xvt_rect_get_width (&realSize);
           realHeight = xvt_rect_get_height (&realSize);

           xScale = (double) width/realWidth;
           yScale = (double) height/realHeight;

           if (fabs(xScale - yScale) > RATIO_TOLERANCE)
           {
              if (yScale < xScale)
              {
                 position.right = position.left + (short) (yScale * realWidth);
                 position.bottom = position.top + (short) (yScale * realHeight);
              }
              else
              {
                 position.bottom = position.top + (short) (xScale * realHeight);
                 position.right = position.left + (short) (xScale * realWidth);
              }
              CORRECT_WIN_RESIZE(win, position)

              xvt_vobj_move (win, &position);
           }
           xvt_dwin_invalidate_rect (win, NULL);
           break;
      case E_UPDATE:
           xvt_vobj_get_client_rect (win, &position);
           if (movieData = (MOVIE_DATA *) xvt_vobj_get_data (win))
           {
              RCT pixmapSize;

              if (!movieData->pixmap)
              {
                 if (movieData->prev)
                    movieData = movieData->prev;
              }
                            
              if (movieData->pixmap)
              {
                 xvt_vobj_get_client_rect (movieData->pixmap, &pixmapSize);
                 xvt_dwin_draw_pmap (win, movieData->pixmap, &position, &pixmapSize);
              }
           }
           break;
      case E_FOCUS:
         lastActiveWindow = win;
         break;
   }
   return 0L;
}

/* ======================================================================
FUNCTION        traceEventHandler

DESCRIPTION
        trace event handlers

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
traceEventHandler (WINDOW win, EVENT *ep)
#else
traceEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   int numPoints, deletePoint;
   RCT position, dragRect;
   TRACE_DATA *traceData;
   PROFILE_POINT *point, *prevPoint;
   static PROFILE_POINT *dragPoint = NULL;
   
   switch (ep->type)
   {
      case E_CREATE:
           updateMenuOptions (TASK_MENUBAR, NULL_WIN);
           traceData = (TRACE_DATA *) xvt_vobj_get_data (win);
           xvt_vobj_get_outer_rect (win, &position);
           traceData->imageWindow = win;
           switch (traceData->object->shape)
           {
              case (FOLD):
                 traceData->menuWindow = createPositionedWindow (FOLD_TRACE_WINDOW,
                                         TASK_WIN, position.left/4, position.top,
                                         EM_ALL, FOLD_TRACE_WINDOW_eh,
                                         (long) traceData, (RCT *) NULL);
                 break;
              case (FAULT):
                 traceData->menuWindow = createPositionedWindow (FAULT_TRACE_WINDOW,
                                         TASK_WIN, position.left/4, position.top,
                                         EM_ALL, FAULT_TRACE_WINDOW_eh,
                                         (long) traceData, (RCT *) NULL);
                 break;
              case (SHEAR_ZONE):
                 if (traceData->menuWindow = createPositionedWindow (FAULT_TRACE_WINDOW,
                                         TASK_WIN, position.left/4, position.top,
                                         EM_ALL, FAULT_TRACE_WINDOW_eh,
                                         (long) traceData, (RCT *) NULL))
                    xvt_vobj_set_title (traceData->menuWindow, "Shear Trace Control");                        
                 break;
           }
           if (!traceData->menuWindow)
           {
              xvt_dm_post_error ("Error, Can't create Control Window.");
              xvt_vobj_destroy (win);
           }
           else
           {
              xvt_vobj_set_visible (traceData->menuWindow, TRUE);
              bringWindowToFront (traceData->menuWindow);
           }
           break;
      case E_DESTROY:
                             /* make sure current window is not invalid */
           if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;

           traceData = (TRACE_DATA *) xvt_vobj_get_data (win);
           if (traceData->pixmap)
           {
              PROFILE_POINT *point, *nextPoint;
              
              if (traceData->pixmap)
                 xvt_pmap_destroy (traceData->pixmap);
              if (traceData->menuWindow)
                 xvt_vobj_destroy (traceData->menuWindow);
              for (point = traceData->points; point; point = nextPoint)
              {
                 nextPoint = point->next;
                 xvt_mem_free ((char *) point);
              }
              xvt_mem_free ((char *) traceData);
              xvt_vobj_set_data (win, 0L);
           }

           updateMenuOptions (TASK_MENUBAR, win);
           break;
      case E_CLOSE:
           xvt_vobj_destroy (win);
           break;
      case E_COMMAND:
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
           do_TASK_MENUBAR(win, ep);
#endif
           break;
      case E_HSCROLL:
      case E_VSCROLL:
      case E_MOUSE_DOWN:
           if (!(traceData = (TRACE_DATA *) xvt_vobj_get_data (win)))
              break;
           
           if (traceData->object->shape == FOLD)
              deletePoint = xvt_ctl_is_checked(xvt_win_get_ctl(traceData->menuWindow,
                                               FOLD_TRACE_DELETE_POINT));
           else
              deletePoint = FALSE;
                   
                   /* Delete unwanted points and count how many we have */
           numPoints = 0; dragPoint = NULL; prevPoint = NULL;
           for (point = traceData->points; point; point = point->next)
           {
              getDragRect (&(point->point), &dragRect);
              if (xvt_rect_has_point(&dragRect, ep->v.mouse.where))
              {
                 dragPoint = point;
                 xvt_win_trap_pointer (win);
                 if (deletePoint)
                 {
                    if (prevPoint)
                       prevPoint->next = dragPoint->next;
                    else
                       traceData->points = dragPoint->next;
                    xvt_mem_free ((char *) dragPoint);
						  dragPoint = NULL;
                 }
                 break;
              }
              numPoints++;
              prevPoint = point;
           }
           
                    /* if draging an existing point or deleting go no further */
           if (dragPoint || deletePoint)
              break;
                    
                        /* FAULT profiles can only be staight (2 points) */
           if (((traceData->object->shape == FAULT)
                   || (traceData->object->shape == SHEAR_ZONE)) && (numPoints >= 2))
              break;
              
                             /* Add a point at this location */
           if (!(dragPoint = (PROFILE_POINT *) xvt_mem_zalloc (sizeof (PROFILE_POINT))))
              break;
           dragPoint->point = ep->v.mouse.where;
              
           if ((traceData->points) &&  /* New first Point in list */
               (traceData->points->point.h > dragPoint->point.h))
           {
              dragPoint->next = traceData->points;
              traceData->points = dragPoint;
           }
           else  /* Insert Point in list */
           {
              for (point = traceData->points; point; point = point->next)
              {
                 if ((point->point.h < dragPoint->point.h)
                       && (point->next && (point->next->point.h >= dragPoint->point.h)))
                    break;
                 
                 if (!point->next)  /* end before (point==NULL) to add at end */
                    break;
              }
              if (!point)  /* This is the first point */
                 traceData->points = dragPoint;
              else  /* add after the point location found */
              {
                 dragPoint->next = point->next;
                 point->next = dragPoint;
              }
           }
           break;
      case E_MOUSE_DBL:
           break;
      case E_MOUSE_MOVE:
           if (dragPoint)
           {
              dragPoint->point = ep->v.mouse.where;
              xvt_dwin_invalidate_rect (win, NULL);
           }
           break;
      case E_MOUSE_UP:
           bringWindowToFront(win);
           dragPoint = NULL;
           xvt_win_release_pointer ();
           xvt_dwin_invalidate_rect (win, NULL);
           break;
      case E_SIZE:
           xvt_dwin_invalidate_rect (win, NULL);
           break;
      case E_UPDATE:
           if (!xvt_vobj_get_data (win))
              break;

           xvt_vobj_get_client_rect (win, &position);
           if (traceData = (TRACE_DATA *) xvt_vobj_get_data (win))
           {
              RCT pixmapSize;
              int first = TRUE;
              DRAW_CTOOLS tools;
              
              xvt_vobj_get_client_rect (traceData->pixmap, &pixmapSize);
              xvt_dwin_draw_pmap (win, traceData->pixmap, &pixmapSize, &pixmapSize);
              xvt_dwin_get_draw_ctools (win, &tools);
              tools.pen.width = 1;
              tools.pen.color = COLOR_RED;
              tools.brush.color = COLOR_RED;
              xvt_dwin_set_draw_ctools (win, &tools);
              for (point = traceData->points; point; point = point->next)
              {
                 if (first)
                 {
                    xvt_dwin_draw_set_pos (win, point->point);
                    first = FALSE;
                 }
                 else
                    xvt_dwin_draw_line (win, point->point);
                    
                 getDragRect (&(point->point), &dragRect);
                 xvt_dwin_draw_rect (win, &dragRect);
              }
           }
           break;
      case E_FOCUS:
         lastActiveWindow = win;
         break;
   }
   return 0L;
}

/* ======================================================================
FUNCTION        pictureEventHandler

DESCRIPTION
        picture event handlers

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
pictureEventHandler (WINDOW win, EVENT *ep)
#else
pictureEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   int width, height;
   double xScale, yScale;
   XVT_PIXMAP diagram;
   RCT position, realSize;
   int realWidth, realHeight;

   switch (ep->type)
   {
      case E_CREATE:
           updateMenuOptions (TASK_MENUBAR, NULL_WIN);
           break;
      case E_DESTROY:
                             /* make sure current window is not invalid */
           if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;

           diagram = (XVT_PIXMAP) xvt_vobj_get_data (win);
           if (diagram)
           {
              xvt_pmap_destroy (diagram);
              xvt_vobj_set_data (win, 0L);
           }

           if (win == lineMapPlotWindow)
              lineMapPlotWindow = NULL_WIN;
           else if (win == stratColumnsWindow)
              stratColumnsWindow = NULL_WIN;
           else if (win == topoWindow)
              topoWindow = NULL_WIN;

           updateMenuOptions (TASK_MENUBAR, win);

           break;
      case E_CLOSE:
           xvt_vobj_destroy (win);
           break;
      case E_COMMAND:
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
           do_TASK_MENUBAR(win, ep);
#endif
           break;
      case E_HSCROLL:
      case E_VSCROLL:
      case E_MOUSE_DOWN:
      case E_MOUSE_DBL:
      case E_MOUSE_MOVE:
           break;
      case E_MOUSE_UP:
           bringWindowToFront(win);
           break;
      case E_SIZE:
           if (!xvt_vobj_get_data (win))
              break;
           xvt_vobj_get_client_rect (win, &position);
                        /* keep the XY ratio constant when resizing window
                        ** so the image does not distory with scaling */
           width = xvt_rect_get_width (&position);
           height = xvt_rect_get_height (&position);

           if (!(diagram = getWindowPixmap (win)))
           {
              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
              break;
           }
           xvt_vobj_get_client_rect (diagram, &realSize);
           realWidth = xvt_rect_get_width (&realSize);
           realHeight = xvt_rect_get_height (&realSize);

           xScale = (double) width/realWidth;
           yScale = (double) height/realHeight;

           if (fabs(xScale - yScale) > RATIO_TOLERANCE)
           {
              if (yScale < xScale)
              {
                 position.right = position.left + (short) (yScale * realWidth);
                 position.bottom = position.top + (short) (yScale * realHeight);
              }
              else
              {
                 position.bottom = position.top + (short) (xScale * realHeight);
                 position.right = position.left + (short) (xScale * realWidth);
              }
              CORRECT_WIN_RESIZE(win, position)

              xvt_vobj_move (win, &position);
           }
           xvt_dwin_invalidate_rect (win, NULL);
           break;
      case E_UPDATE:
           if (!xvt_vobj_get_data (win))
              break;

           xvt_vobj_get_client_rect (win, &position);
           if (diagram = (XVT_PIXMAP) xvt_vobj_get_data (win))
           {
              RCT pixmapSize;
              xvt_vobj_get_client_rect (diagram, &pixmapSize);
              xvt_dwin_draw_pmap (win, diagram, &position, &pixmapSize);
           }
           break;
      case E_FOCUS:
         lastActiveWindow = win;
         break;
   }
   return 0L;
}

/* ======================================================================
FUNCTION        anomImageEventHandler

DESCRIPTION
        anom image event handlers

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
anomImageEventHandler (WINDOW win, EVENT *ep)
#else
anomImageEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   int width, height;
   double xScale, yScale, winScale;
   double xLoc, yLoc, value;
   RCT position, realSize;
   int realWidth, realHeight, profNum;
   ANOMIMAGE_DATA *anomImageData;
   static PNT startLine = { 0, 0 };
   static PNT endLine   = { 0, 0 };
   static short mouseDown = FALSE;
   char message[200];
   
   switch (ep->type)
   {
      case E_CREATE:
           updateMenuOptions (TASK_MENUBAR, NULL_WIN);
			  ep->type = E_SIZE;
			  anomImageEventHandler (win, ep);
           break;
      case E_DESTROY:
                             /* make sure current window is not invalid */
           if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;

           anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win);
           if (anomImageData)
           {
              xvt_pmap_destroy (anomImageData->pixmap);
              anomImageData->pixmap = NULL_WIN;
				  if (anomImageData->imageDisplay.lut)
				  {
					  xvt_mem_free((char *) anomImageData->imageDisplay.lut);
				     anomImageData->imageDisplay.lutSize = 0;
				  }
              if ((win != gravityReferenceWindow) && (win != magneticsReferenceWindow))
              {
                 destroy2DArray ((char **) anomImageData->imageData,
                                 anomImageData->dataYSize,
                                 anomImageData->dataXSize);
                 xvt_mem_free ((char *) anomImageData);
              }
              xvt_vobj_set_data (win, 0L);
           }

           if (win == gravityReferenceWindow)  /* make sure gravityReferenceWindow is valid */
              gravityReferenceWindow = NULL_WIN;
           if (win == magneticsReferenceWindow)  /* make sure magneticsReferenceWindow is valid */
              magneticsReferenceWindow = NULL_WIN;

           updateMenuOptions (TASK_MENUBAR, win);
           break;
      case E_CLOSE:
           xvt_vobj_destroy (win);
           break;
      case E_COMMAND:
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
           do_TASK_MENUBAR(win, ep);
#endif
           break;
      case E_HSCROLL:
      case E_VSCROLL:
           break;
      case E_MOUSE_DOWN:
           if (ep->v.mouse.button != 0)
           {
              if (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
              {       /* only allow options window on images, not profiles */
                 if (anomImageData->dataYSize > 1)
                 {
                    lastActiveWindow = win;
                    createPopupWindow (win, ep->v.mouse.where,
                                IMAGE_OPTION_WINDOW, IMAGE_OPTION_WINDOW_eh,
                                (long) &(anomImageData->imageDisplay));
                 }
              }
           }
           else
           {
              if (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
              {       /* only allow profiles on map images, not profiles */
                 if (anomImageData->dataYSize > 1)
                 {
                    mouseDown = TRUE;
                    startLine.h = ep->v.mouse.where.h;
                    startLine.v = ep->v.mouse.where.v;
                    endLine.h = ep->v.mouse.where.h;
                    endLine.v = ep->v.mouse.where.v;
   
                    xvt_win_trap_pointer(win);
                 }
              }  
           }
           break;
     case E_MOUSE_MOVE:
           if ((anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
                                          && (anomImageData->dataYSize > 1))
           {
              if (mouseDown)
              {
                 xvt_dwin_set_draw_mode (win, M_XOR);
                 xvt_dwin_set_std_cpen (win, TL_PEN_RUBBER);
                                   /* erase last Line */
                 if ((startLine.h != endLine.h) || (startLine.v != endLine.v))
                 {
                    xvt_dwin_draw_set_pos (win, startLine);
                    xvt_dwin_draw_line (win, endLine);
                 }
                 endLine.h = ep->v.mouse.where.h;
                 endLine.v = ep->v.mouse.where.v;
                 xvt_dwin_draw_set_pos (win, startLine);
                 xvt_dwin_draw_line (win, endLine);
                 xvt_dwin_set_draw_mode (win, M_COPY);
              }
              
              xvt_vobj_get_client_rect (win, &position);
              width = xvt_rect_get_width (&position);
              winScale = ((double) width)/((double) (anomImageData->dataXSize-1));

              if (getAnomImageValue (anomImageData, (int) ep->v.mouse.where.h, (int) ep->v.mouse.where.v,
                                     winScale, &xLoc, &yLoc, &value))
              {
                 if (fabs(value) > TOLERANCE)
                    sprintf (message, "%s\tX:%.0lf\tY:%.0lf\tVal:%.2lf", STATUS_ANOMIMAGE_HELP, xLoc, yLoc, value);
                 else
                    sprintf (message, "%s\tX:%.0lf\tY:%.0lf\tVal:%e", STATUS_ANOMIMAGE_HELP, xLoc, yLoc, value);
              }
              else
                 sprintf (message, "%s", STATUS_ANOMIMAGE_HELP);
              statbar_set_title (statusBar, message);
           }
           break;
     case E_MOUSE_UP:
           if (ep->v.mouse.button != 0)
              break; /* make sure mouse up of right button does nothing */
              
           if ((anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
                                          && (anomImageData->dataYSize > 1))
           {
              if (mouseDown && ((startLine.h != endLine.h) ||
                                (startLine.v != endLine.v)))
              {
                                       /* erase last Line */
                 xvt_dwin_set_draw_mode (win, M_XOR);
                 xvt_dwin_set_std_cpen (win, TL_PEN_RUBBER);
                 xvt_dwin_draw_set_pos (win, startLine);
                 xvt_dwin_draw_line (win, endLine);
                 xvt_dwin_set_draw_mode (win, M_COPY);
   
                 xvt_win_release_pointer();
                 endLine.h = ep->v.mouse.where.h;
                 endLine.v = ep->v.mouse.where.v;
                 
                                         /* display the profile */
                 if (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
                 {
                    xvt_vobj_get_client_rect (win, &position);
                    width = xvt_rect_get_width (&position);
   
                    xvt_vobj_get_client_rect (anomImageData->pixmap, &realSize);
                    realWidth = xvt_rect_get_width (&realSize);
   
                    winScale = (double) width/realWidth;
                    if (startLine.h < endLine.h)
                       profNum = updateProfileImage (anomImageData, startLine, endLine, winScale,
												(double) ((double) width)/((double) (anomImageData->dataXSize-1)));
                    else
                       profNum = updateProfileImage (anomImageData, endLine, startLine, winScale,
												(double) ((double) width)/((double) (anomImageData->dataXSize-1)));

                               /* Draw profile to image */
                    if (ep->v.mouse.shift || ep->v.mouse.control)
                    {
                       drawProfileLabel (anomImageData->pixmap,
                                  profNum, startLine, endLine, winScale);
                       xvt_dwin_invalidate_rect (win, NULL);
                       xvt_dwin_update (win);
                    }
                 }
              } 
              else
                 bringWindowToFront(win);
           }
           else
              bringWindowToFront(win);
           
           mouseDown = FALSE;
           xvt_win_release_pointer();
           startLine.h = startLine.v= endLine.h = endLine.v = 0;
           break;
      case E_MOUSE_DBL:
           break;
      case E_SIZE:
           if (!(anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win)))
           {
              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
              break;
           }
           if (!anomImageData->pixmap)
           {
              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
              break;
           }
                        /* keep the XY ratio constant when resizing window
                        ** so the image does not distory with scaling */
           xvt_vobj_get_client_rect (win, &position);
           width = xvt_rect_get_width (&position);
           height = xvt_rect_get_height (&position);

           xvt_vobj_get_client_rect (anomImageData->pixmap, &realSize);
           realWidth = xvt_rect_get_width (&realSize);
           realHeight = xvt_rect_get_height (&realSize);

           xScale = (double) width/realWidth;
           yScale = (double) height/realHeight;

           if (fabs(xScale - yScale) > RATIO_TOLERANCE)
           {
              if (yScale < xScale)
              {
                 position.right = position.left + (short) (yScale * realWidth);
                 position.bottom = position.top + (short) (yScale * realHeight);
              }
              else
              {
                 position.bottom = position.top + (short) (xScale * realHeight);
                 position.right = position.left + (short) (xScale * realWidth);
              }
              CORRECT_WIN_RESIZE(win, position)

              xvt_vobj_move (win, &position);
           }
           xvt_dwin_invalidate_rect (win, NULL);
			  updateLegendData(win, MAG_IMAGE_LEGENDS);
           break;
      case E_UPDATE:
           if (!xvt_vobj_get_data (win))
              break;
                                  
           xvt_vobj_get_client_rect (win, &position);
           if (anomImageData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win))
           {
              RCT pixmapSize;
              
              if (!anomImageData->pixmap)
                 break;
                 
              xvt_vobj_get_client_rect (anomImageData->pixmap, &pixmapSize);
              xvt_dwin_draw_pmap (win, anomImageData->pixmap, &position, &pixmapSize);
              if (anomImageData->dataYSize > 1) /* Dont draw grid on profile Images */
					  overlayAnomGrid (win, anomImageData);
           }
           break;
      case E_FOCUS:
         lastActiveWindow = win;
			updateLegendData(win, MAG_IMAGE_LEGENDS);
         sprintf (message, "%s", STATUS_ANOMIMAGE_HELP);
         statbar_set_title (statusBar, message);
         break;
   }
   return 0L;
}

/* ======================================================================
FUNCTION        drawProfileLabel

DESCRIPTION
        draw the labeling line of a profile

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
drawProfileLabel (WINDOW win, int profNum,
                  PNT startLine, PNT endLine, double scale)
#else
drawProfileLabel (win, profNum, startLine, endLine, scale)
WINDOW win;
int profNum;
PNT startLine;
PNT endLine;
double scale;
#endif
{
   CPEN pen;
   char label[10];
   
   startLine.h = (short) (startLine.h / scale);
   startLine.v = (short) (startLine.v / scale);
   endLine.h = (short) (endLine.h / scale);
   endLine.v = (short) (endLine.v / scale);
        /* leave a label showing the profile pos */
   xvt_dwin_set_draw_mode (win, M_COPY);
   pen.width = 1;        pen.pat = PAT_NONE;
   pen.style = P_SOLID;  pen.color = COLOR_RED;
   xvt_dwin_set_cpen (win, &pen);
   xvt_dwin_set_fore_color (win, COLOR_RED);
   
   xvt_dwin_draw_set_pos (win, startLine);
   xvt_dwin_draw_line (win, endLine);
   sprintf (label, "%d", profNum);
   if (startLine.h < endLine.h)
     xvt_dwin_draw_text (win,
           startLine.h-10, startLine.v, label, -1);
   else
     xvt_dwin_draw_text (win,
           endLine.h-10, endLine.v, label, -1);

   return (TRUE);
}

/* ======================================================================
FUNCTION        profileEventHandler

DESCRIPTION
        block diagram event handler

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
profileEventHandler (WINDOW win, EVENT *ep)
#else
profileEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   switch (ep->type)
   {
      case E_CREATE:
           break;
      case E_DESTROY:
           xvt_vobj_set_data (win, 0L);
                             /* make sure current window is not invalid */
           if (win == getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;
           break;
      case E_MOUSE_UP:
           break;
   }
                /* now perform any action associated with the normal **
                ** non editable profile window */
   return (win_105_eh (win, ep));
}

/* ======================================================================
FUNCTION        threedEventHandler

DESCRIPTION
        threed picture event handlers

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
threedEventHandler (WINDOW win, EVENT *ep)
#else
threedEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   double xScale, yScale;
   static PNT startMove = { 0, 0 };
   static short previewDone = FALSE, updatingImage = FALSE;
   PNT endMove;
   THREED_IMAGE_DATA *threedData;
   int width, height;
   XVT_PIXMAP diagram, newPixmap;
   RCT position;
   double declination, azimuth, scale, decInc, eleInc, scaleInc;
   char message[100];

   switch (ep->type)
   {
      case E_CREATE:
           updateMenuOptions (TASK_MENUBAR, NULL_WIN);
           break;
      case E_DESTROY:
                             /* make sure current window is not invalid */
           if (getWindowPixmap(win) == (XVT_PIXMAP) getCurrentDrawingWindow ()) 
              setCurrentDrawingWindow (NULL_WIN);
           if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
              lastActiveWindow = NULL_WIN;

           threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (win);
           if (threedData)
           {
              if (threedData->pixmap)
                 xvt_pmap_destroy (threedData->pixmap);
               
              if (threedData->surface)
                 xvt_mem_hfree ((char __huge *) threedData->surface);

              xvt_mem_free ((char *) threedData);
              xvt_vobj_set_data (win, 0L);
           }

           if (win == topo3dWindow)
              topo3dWindow = NULL_WIN;
           else if (win == strat3dWindow)
              strat3dWindow = NULL_WIN;

           updateMenuOptions (TASK_MENUBAR, win);
           break;
      case E_CLOSE:
           xvt_vobj_destroy (win);
           break;
      case E_COMMAND:
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
           do_TASK_MENUBAR(win, ep);
#endif
           break;
      case E_HSCROLL:
      case E_VSCROLL:
      case E_MOUSE_DOWN:
           if (processingLongJob || updatingImage)
              break;
           startMove = ep->v.mouse.where;
           previewDone = FALSE;
           xvt_win_trap_pointer (win);
           break;
      case E_MOUSE_DBL:
           break;
      case E_MOUSE_UP:
           if (processingLongJob || updatingImage)
              break;

           endMove = ep->v.mouse.where;
           decInc = (endMove.h-startMove.h);
           eleInc = (endMove.v-startMove.v);
                               /* raise window if just a click in the window */
           if ((decInc == 0) && (eleInc == 0) && !previewDone)
           {
              bringWindowToFront(win);
              startMove.h = 0; startMove.v = 0;
           }
           else if (threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (win))
           {                   /* move the position of the threed view */
              updatingImage = TRUE;
              startMove.h = 0; startMove.v = 0;
              previewDone = FALSE;
              getCameraPosition (threedData,
                                 &declination, &azimuth, &scale);
              decInc = decInc/4;
              eleInc = eleInc/4;
              setCameraPosition (threedData,
                 declination + decInc, azimuth + eleInc, scale);

              update3dSceen (threedData);

              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
              updatingImage = FALSE;
           }
           xvt_win_release_pointer ();
           break;
      case E_MOUSE_MOVE:
           if (startMove.h || startMove.v)
           {
              if (threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (win))
              {
                 endMove = ep->v.mouse.where;
                 decInc = (double) (endMove.h-startMove.h);
                 eleInc = (double) (endMove.v-startMove.v);
                 scaleInc = (double) (endMove.v-startMove.v);
                                              
                 if ((fabs(decInc) < 2) && (fabs(eleInc) < 2))
                    return 0L;
                                                               
                 getCameraPosition (threedData,
                                    &declination, &azimuth, &scale);
   
                 if (ep->v.mouse.shift)  /* Zoom */
                 {
                    scaleInc = (scale/100.0)*scaleInc;
                    setCameraPosition (threedData,
                        declination, azimuth, scale + scaleInc);
                    if (threedData->imageType == STRAT_3D)
                       sprintf (message, "%s\t%.0lf Rot.\t%.0lf Az.\t%.0lf %%", STATUS_3DSTRAT_HELP,
                                   (declination<180.0)?(180.0-declination):180+(360-declination), azimuth,
                                   (scale + scaleInc)*100.0);
                    else
                       sprintf (message, "%s\t%.0lf Rot.\t%.0lf Az.\t%.0lf %%", STATUS_3DSTRAT_HELP, declination, azimuth, (scale + scaleInc)*100.0);
                 }
                 else  /* Pan */
                 {
                    decInc = decInc/4.0;
                    eleInc = eleInc/4.0;
                    setCameraPosition (threedData,
                        declination + decInc, azimuth + eleInc, scale);
                    if (threedData->imageType == STRAT_3D)
                       sprintf (message, "%s\t%.0lf Rot.\t%.0lf Az.\t%.0lf %%", STATUS_3DSTRAT_HELP,
                                   (declination<180.0)?(180.0+declination+decInc):(declination+decInc)-180.0,
                                   (azimuth + eleInc), scale*100.0);
                    else
                       sprintf (message, "%s\t%.0lf Rot.\t%.0lf Az.\t%.0lf %%", STATUS_3DSTRAT_HELP, declination + decInc, azimuth + eleInc, scale*100.0);
                 }
                 statbar_set_title (statusBar, message);

                 preview3dSceen (threedData, 4);

                 xvt_dwin_invalidate_rect (win, NULL);
                 xvt_dwin_update (win);

                 startMove = endMove;
                 previewDone = TRUE;
              }
           }
           break;
      case E_SIZE:
           if (!(threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (win)))
              break;
           getCameraPosition (threedData, &declination, &azimuth, &scale);

                                /* Size of the window */
           xvt_vobj_get_client_rect (win, &position);
           width = xvt_rect_get_width (&position);
           height = xvt_rect_get_height (&position);

           if (!(diagram = threedData->pixmap))
           {
              xvt_dwin_invalidate_rect (win, NULL);
              xvt_dwin_update (win);
              break;
           }

           if (newPixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT, (short) width, (short) height, NULL))
           {                 /* Get the old size of the pixmap */
              RCT pixmapSize;
              xvt_vobj_get_client_rect (diagram, &pixmapSize);    
                            /* Draw the image stretched to the new window size */
              xvt_dwin_draw_pmap (newPixmap, diagram, &position, &pixmapSize);
                            /* Adjust scale to roughly match the image size that is now drawn */
              xScale = (double) ((double) width)/((double) xvt_rect_get_width(&pixmapSize));
              yScale = (double) ((double) height)/((double) xvt_rect_get_height(&pixmapSize));
              scale = scale * MIN(xScale,yScale);
              setCameraPosition (threedData, declination, azimuth, scale);
              
              threedData->pixmap = newPixmap;
              xvt_pmap_destroy (diagram);
           }
           xvt_dwin_invalidate_rect (win, NULL);
           break;
      case E_UPDATE:
           xvt_vobj_get_client_rect (win, &position);
           if (threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (win))
           {
              RCT pixmapSize;
              xvt_vobj_get_client_rect (threedData->pixmap, &pixmapSize);
              xvt_dwin_draw_pmap (win, threedData->pixmap, &position, &pixmapSize);
           }
           break;
      case E_FOCUS:
         lastActiveWindow = win;
			updateLegendData(win, THREED_LEGENDS);
        break;
   }
   return 0L;
}


/* ======================================================================
FUNCTION        readGeophys

DESCRIPTION
        read the image part of the anomilies file

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
readGeophys (FILE *in, unsigned char **charData, double **doubleData,
             int maxx, int maxy, double *maxValuePtr, double *minValuePtr,
             BOOLEAN transpose, int binary)
#else
readGeophys (in, charData, doubleData, maxx, maxy,
                           maxValuePtr, minValuePtr, transpose, binary)
FILE *in;
unsigned char **charData;
double **doubleData;
int maxx, maxy;
double *maxValuePtr, *minValuePtr;
BOOLEAN transpose;
int binary;
#endif
{
   long position;
   register int j, k, xPos, yPos, linePos;
   double value, minValue = 999999.0, maxValue = -999999.0 , scale;
   unsigned char ivalue;
   PIXEL pixel;
   int pixelSize;

   if (binary)
      pixelSize = sizeof (PIXEL);

   if (charData || (minValuePtr && maxValuePtr))
   {
      position = ftell (in);  /* get out position in th file */
                     /* first pick out the range of the data */
      for (j = 0; j < maxy; j++)
      {
         incrementLongJob (INCREMENT_JOB);
         for(k = 0; k < maxx; k++)
         {
            if (binary)
            {
               fread (&pixel, pixelSize, 1, in);
               value = pixel.value;
#if (XVTWS == XOLWS)
               DOUBLE_SWAP(value)
#endif
            }
            else
            {
               fscanf(in,"%lf", &value);
            }
            if (value < minValue)
               minValue = value;
            else if (value > maxValue)
               maxValue = value;
         }
      }
      if (maxValue == minValue)
         scale = 1.0;
      else
         scale = 255.0 / (maxValue - minValue);
      if (minValuePtr)
         *minValuePtr = minValue;
      if (maxValuePtr)
         *maxValuePtr = maxValue;
      fseek (in, position, 0);  /* move back again */
   }
DEBUG(printf("\nMin VAlue = %f, Max Value = %f\n",minValue, maxValue);)
DEBUG(printf("\nBINARY = %d\n",binary);)


   if (transpose)
   {
      for (j = 0; j < maxy; j++)
      {
         incrementLongJob (INCREMENT_JOB);
         for(k = 0; k < maxx; k++)
         {
            if (binary)
            {
               fread (&pixel, pixelSize, 1, in);
               value = pixel.value;
               xPos = pixel.x;
               yPos = pixel.y;
#if (XVTWS == XOLWS)
               SHORT_SWAP(xPos)
               SHORT_SWAP(yPos)
               DOUBLE_SWAP(value)
#endif
            }
            else
            {
               fscanf(in,"%lf", &value);
               xPos = k; yPos = j;
            }

DEBUG(if (xPos == 0))
DEBUG(printf("\n X = %d, y = %d, Val = %f",xPos, yPos, value);)

            linePos = xPos*maxy;

            if (doubleData)
            {
               doubleData[xPos+1][maxy - (yPos+1)] = value;
            }
            if (charData)
            {
               value = (value - minValue) * scale;  /* make it from 0 - 255 */
               if (value > 255)
                  ivalue = 255;
               else if (value < 0)
                  ivalue = 0;
               else
                  ivalue = (unsigned char) value;
             
               charData[xPos][yPos] = ivalue;
            }
         }
      }
   }
   else
   {
      for (j = 0; j < maxy; j++)
      {
         incrementLongJob (INCREMENT_JOB);
         for(k = 0; k < maxx; k++)
         {
            if (binary)
            {
               fread (&pixel, pixelSize, 1, in);
               value = pixel.value;
               xPos = pixel.x;
               yPos = pixel.y;
#if (XVTWS == XOLWS)
               SHORT_SWAP(xPos)
               SHORT_SWAP(yPos)
               DOUBLE_SWAP(value)
#endif
            }
            else
            {
               fscanf(in,"%lf", &value);
               xPos = k; yPos = j;
            }

DEBUG(if (xPos == 0))
DEBUG(printf("\n X = %d, y = %d, Val = %f",xPos, yPos, value);)

            linePos = yPos*maxx;

            if (doubleData)
            {
               doubleData[yPos][xPos] = value;
            }
            if (charData)
            {
               value = (value - minValue) * scale;  /* make it from 0 - 255 */
               if (value > 255)
                  ivalue = 255;
               else if (value < 0)
                  ivalue = 0;
               else
                  ivalue = (unsigned char) value;
             
               charData[yPos][xPos] = ivalue;
            }
         }
      }
   }
}

/* ======================================================================
FUNCTION        readGeophysComment

DESCRIPTION
        read the image part of the anomilies file

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
readGeophysComment (FILE *in, char *tag, char *strValue, int valueLength, int fromStart)
#else
readGeophysComment (in, tag, strValue, valueLength, fromStart)
FILE *in;
char *tag, *strValue;
int valueLength, fromStart;
#endif
{
   char lineIn[255], *gotLine, *ptr;
	int found = FALSE;

			/* Can't find what we dont know or if we dont have somewhere to put result */
	if (!in || !tag || !strValue || (valueLength < 1))
		return (FALSE);

	if (fromStart)
		rewind(in); /* Start from the start each time */

	do {  /* looking for a matching line */
		gotLine = fgets (lineIn, 255, in);
	} while (gotLine && (!(ptr = strstr(lineIn, tag))));

	if (gotLine && ptr)
	{
		found = TRUE;
		strncpy(strValue, ptr+strlen(tag), valueLength);
		if (ptr = strchr(strValue,'\n'))
			*ptr = '\0';  /* Get rid of any CR */
	}

   return (found);
}

/* ======================================================================
FUNCTION        readGeophysHeader

DESCRIPTION
        read the image part of the anomilies file

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
readGeophysHeader (FILE *in, int *fileId, int *csize,
                   int *xmax, int *ymax, int *zmax,
                   float dat[3], float position[6],
                   float *grid, float *airgap,
                   int *sizeX, int *sizeY, int *binary)
#else
readGeophysHeader (in, fileId, csize, xmax, ymax, zmax, dat, position,
                   grid, airgap, sizeX, sizeY, binary)
FILE *in;
int *fileId, *csize, *xmax, *ymax, *zmax;
float dat[3], position[6], *grid, *airgap;
int *sizeX, *sizeY;
int *binary;
#endif
{
   char lineIn[255];

   readGoodLine(in, lineIn);
   if (strstr(lineIn, "BINARY"))
      *binary = TRUE;
   else
      *binary = FALSE;

   sscanf(lineIn,"%d", fileId);
   if ((*fileId != 444 && *fileId != 333)
                               && (*fileId != 555 && *fileId != 666))
   {
      return (FALSE);
   }
                      /* read in the header */
   readGoodLine(in, lineIn);
   sscanf(lineIn,"%d %d %d %d\n", csize, xmax, ymax, zmax);
   readGoodLine(in, lineIn);
   sscanf(lineIn,"%f %f %f", &dat[0],&dat[1],&dat[2]);
   readGoodLine(in, lineIn);
   sscanf(lineIn,"%f %f %f", &position[0], &position[1], &position[2]);
   readGoodLine(in, lineIn);
   sscanf(lineIn,"%f %f %f", &position[3], &position[4], &position[5]);
   readGoodLine(in, lineIn);
   sscanf(lineIn,"%f %f", grid, airgap);

                      /* size of the image we are reading in */
   if ((*fileId == 444) || (*fileId == 333))   /* anomily */
      *sizeX = *xmax - (2 * (*csize)) - 1;
   else if ((*fileId == 555) || (*fileId == 666))  /* profile */
      *sizeX = 1;
   *sizeY = *ymax - (2 * (*csize)) - 1;

            /* get rid of the rest of the last line of the header */
   /* Note needed now we are using readGoodLine to skip comments - fgets (lineIn, 80, in); */

   return (TRUE);
}

/* ======================================================================
FUNCTION        getTopoFile

DESCRIPTION
        get the file pointer to read the Topography file

RETURNED
====================================================================== */
FILE *
#if XVT_CC_PROTO
getTopoFile ()
#else
getTopoFile ()
#endif
{
   FILE *topoFile = NULL;

   xvt_fsys_set_dir (&topoFileSpec.dir);
   topoFile = (FILE *) fopen (topoFileSpec.name, "r");

   return (topoFile);
}


/* ======================================================================
FUNCTION        interpolateEvent

DESCRIPTION
        change the values in an event to perform a smooth transition
        from the state before the event to the state after in a set
        number of frames

RETURNED    TRUE if it was interpolated, FALSE if nothing changed
====================================================================== */
int
#if XVT_CC_PROTO
interpolateEvent (OBJECT *object, int frame, int framesPerEvent)
#else
interpolateEvent (object, frame, framesPerEvent)
OBJECT *object;
int frame, framesPerEvent;
#endif
{
   static double increment1 = 0.0, increment2 = 0.0, increment3 = 0.0,
                 increment4 = 0.0, increment5 = 0.0, increment6 = 0.0,
                 increment7 = 0.0, increment8 = 0.0, increment9 = 0.0;
   int interpolated = TRUE;

   switch (object->shape)
   {
      case STRATIGRAPHY:
         interpolated = FALSE;
         break;
      case FOLD:
         {
            FOLD_OPTIONS *options = (FOLD_OPTIONS *) object->options;
            
            if (frame == 0)  /* init values to increment by on the first frame */
            {
               increment1 = options->amplitude / (double) framesPerEvent;
               options->amplitude = 0.0;
            }
            
            options->amplitude = options->amplitude + increment1;
         }
         break;
      case FAULT:
         {
            FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;
            
            if (options->geometry == ROTATION)
            {
               if (frame == 0)  /* init values to increment by on the first frame */
               {
                  increment1 = options->rotation / (double) framesPerEvent;
                  options->rotation = 0.0;
               }
               options->rotation = options->rotation + increment1;
            }
            else
            {
               if (frame == 0)  /* init values to increment by on the first frame */
               {
                  increment1 = options->slip / (double) framesPerEvent;
                  options->slip = 0.0;
               }
               options->slip = options->slip + increment1;
            }

				if (options->alterationZones != NONE_ALTERATION)
				{
					PROFILE_OPTIONS *profile;

					profile = options->alterationFuntions;
					while (profile)
					{
						if (frame == 0)
						{          /* Temporarly store max val in unused min Val to restore after interpolation */
							profile->minX = profile->maxX;
							profile->maxX = 0.0 + profile->minX / (double) framesPerEvent;
						}
						else if (frame == framesPerEvent-1)  /* Last step so put back to what we had */
						{
							profile->maxX = profile->minX;
							profile->minX = 0.0;
						}
						else
							profile->maxX = profile->maxX + profile->minX / (double) framesPerEvent;
						profile = profile->next;
					}
				}
         }
         break;
      case UNCONFORMITY:
         {
            UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;

				if (options->alterationZones != NONE_ALTERATION)
				{
					PROFILE_OPTIONS *profile;

					profile = options->alterationFuntions;
					while (profile)
					{
						if (frame == 0)
						{          /* Temporarly store max val in unused min Val to restore after interpolation */
							profile->minX = profile->maxX;
							profile->maxX = 0.0;
						}
						else if (frame == framesPerEvent-1)  /* Last step so put back to what we had */
						{
							profile->maxX = profile->minX;
							profile->minX = 0.0;
						}
						else
							profile->maxX = profile->maxX + profile->minX / (double) framesPerEvent;
						profile = profile->next;
					}
				}
				else
					interpolated = FALSE;
			}
			break;
      case SHEAR_ZONE:
         {
            SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;
            
            if (options->geometry == ROTATION)
            {
               if (frame == 0)  /* init values to increment by on the first frame */
               {
                  increment1 = options->rotation / (double) framesPerEvent;
                  options->rotation = 0.0;
               }
               options->rotation = options->rotation + increment1;
            }
            else
            {
               if (frame == 0)  /* init values to increment by on the first frame */
               {
                  increment1 = options->slip / (double) framesPerEvent;
                  options->slip = 0.0;
               }
               options->slip = options->slip + increment1;
            }

            if (frame == 0)  /* init values to increment by on the first frame */
            {
               increment2 = options->width / (double) framesPerEvent;
               options->width = 0.0;
            }
            options->width = options->width + increment2;

				if (options->alterationZones != NONE_ALTERATION)
				{
					PROFILE_OPTIONS *profile;

					profile = options->alterationFuntions;
					while (profile)
					{
						if (frame == 0)
						{          /* Temporarly store max val in unused min Val to restore after interpolation */
							profile->minX = profile->maxX;
							profile->maxX = 0.0;
						}
						else if (frame == framesPerEvent-1)  /* Last step so put back to what we had */
						{
							profile->maxX = profile->minX;
							profile->minX = 0.0;
						}
						else
							profile->maxX = profile->maxX + profile->minX / (double) framesPerEvent;
						profile = profile->next;
					}
				}
         }
         break;
      case DYKE:
         { 
            DYKE_OPTIONS *options = (DYKE_OPTIONS *) object->options;
            
            if (frame == 0)  /* init values to increment by on the first frame */
            {
               increment1 = options->slipLength / (double) framesPerEvent;
               increment2 = options->width / (double) framesPerEvent;
               options->slipLength = 0.0;
               options->width = 0.0;
            }
            
            options->slipLength = options->slipLength + increment1;
            options->width = options->width + increment2;

				if (options->alterationZones != NONE_ALTERATION)
				{
					PROFILE_OPTIONS *profile;

					profile = options->alterationFuntions;
					while (profile)
					{
						if (frame == 0)
						{          /* Temporarly store max val in unused min Val to restore after interpolation */
							profile->minX = profile->maxX;
							profile->maxX = 0.0;
						}
						else if (frame == framesPerEvent-1)  /* Last step so put back to what we had */
						{
							profile->maxX = profile->minX;
							profile->minX = 0.0;
						}
						else
							profile->maxX = profile->maxX + profile->minX / (double) framesPerEvent;
						profile = profile->next;
					}
				}
         }
         break;
      case PLUG:
         {
            PLUG_OPTIONS *options = (PLUG_OPTIONS *) object->options;
   
            if (frame == 0)  /* init values to increment by on the first frame */
            {
               if (options->type == CYLINDRICAL_PLUG)
               {
                  increment1 = options->radius / (double) framesPerEvent;
                  options->radius = 0.0;
               }
               else if (options->type == CONE_PLUG)
               {
                  increment1 = options->apicalAngle / (double) framesPerEvent;
                  options->apicalAngle = 0.0;
               }
               else if (options->type == PARABOLIC_PLUG)
               {
                  increment1 = options->BValue / (double) framesPerEvent;
                  options->BValue = 0.0;
               }
               else
               {
                  increment1 = options->axisA / (double) framesPerEvent;
                  increment2 = options->axisB / (double) framesPerEvent;
                  increment3 = options->axisC / (double) framesPerEvent;
                  options->axisA = 0.0;
                  options->axisB = 0.0;
                  options->axisC = 0.0;
               }
            }
            
            if (options->type == CYLINDRICAL_PLUG)
               options->radius = options->radius + increment1;
            else if (options->type == CONE_PLUG)
               options->apicalAngle = options->apicalAngle + increment1;
            else if (options->type == PARABOLIC_PLUG)
               options->BValue = options->BValue + increment1;
            else
            {
               options->axisA = options->axisA + increment1;
               options->axisB = options->axisB + increment2;
               options->axisC = options->axisC + increment3;
            }

				if (options->alterationZones != NONE_ALTERATION)
				{
					PROFILE_OPTIONS *profile;

					profile = options->alterationFuntions;
					while (profile)
					{
						if (frame == 0)
						{          /* Temporarly store max val in unused min Val to restore after interpolation */
							profile->minX = profile->maxX;
							profile->maxX = 0.0;
						}
						else if (frame == framesPerEvent-1)  /* Last step so put back to what we had */
						{
							profile->maxX = profile->minX;
							profile->minX = 0.0;
						}
						else
							profile->maxX = profile->maxX + profile->minX / (double) framesPerEvent;
						profile = profile->next;
					}
				}
         }
         break;
      case STRAIN:
         {
            STRAIN_OPTIONS *options = (STRAIN_OPTIONS *) object->options;
            double tensor[4][4], inverseTensor[4][4];
            int i, j;
            
            if (frame == 0)  /* init values to increment by on the first frame */
            {
               increment1 = (options->tensor[0][0]-1.0) / (double) framesPerEvent;
               increment2 = options->tensor[0][1] / (double) framesPerEvent;
               increment3 = options->tensor[0][2] / (double) framesPerEvent;
               increment4 = options->tensor[1][0] / (double) framesPerEvent;
               increment5 = (options->tensor[1][1]-1.0) / (double) framesPerEvent;
               increment6 = options->tensor[1][2] / (double) framesPerEvent;
               increment7 = options->tensor[2][0] / (double) framesPerEvent;
               increment8 = options->tensor[2][1] / (double) framesPerEvent;
               increment9 = (options->tensor[2][2]-1.0) / (double) framesPerEvent;
               options->tensor[0][0] = 1.0;
               options->tensor[0][1] = 0.0;
               options->tensor[0][2] = 0.0;
               options->tensor[1][0] = 0.0;
               options->tensor[1][1] = 1.0;
               options->tensor[1][2] = 0.0;
               options->tensor[2][0] = 0.0;
               options->tensor[2][1] = 0.0;
               options->tensor[2][2] = 1.0;
            }
            
            options->tensor[0][0] = options->tensor[0][0] + increment1;
            options->tensor[0][1] = options->tensor[0][1] + increment2;
            options->tensor[0][2] = options->tensor[0][2] + increment3;
            options->tensor[1][0] = options->tensor[1][0] + increment4;
            options->tensor[1][1] = options->tensor[1][1] + increment5;
            options->tensor[1][2] = options->tensor[1][2] + increment6;
            options->tensor[2][0] = options->tensor[2][0] + increment7;
            options->tensor[2][1] = options->tensor[2][1] + increment8;
            options->tensor[2][2] = options->tensor[2][2] + increment9;
                               /* calculate and store the inverse tensor
                               ** matrix */
            for (i = 0; i < 3; i++)
            {
               for (j = 0; j < 3; j++)
               {
                  tensor[j+1][i+1] = options->tensor[i][j];
               }
            }
            if (!f01aaf(tensor, inverseTensor)) /* matrix inversion */
               return (FALSE);
            for (i = 0; i < 3; i++)
            {
               for (j = 0; j < 3; j++)
               {
                   options->inverseTensor[i][j] = inverseTensor[i+1][j+1];
               }
            }
         }
         break;
      case TILT:
         {
            TILT_OPTIONS *options = (TILT_OPTIONS *) object->options;
            double angle, axis[2][4], mata[4][4];
            int i, j;
            
            if (frame == 0)  /* init values to increment by on the first frame */
            {
               increment1 = options->rotation / (double) framesPerEvent;
               options->rotation = 0.0;
            }
            
            options->rotation = options->rotation + increment1;
            
                               /* calculate and store the rotation matricies */
            angle = options->rotation * 0.0174532;
            paxis (options->plunge, options->plungeDirection, axis);
            irotate(mata, axis[1][1], axis[1][2], axis[1][3], angle);

            for (i = 0; i < 3; i++)
            {
               for (j = 0; j < 3; j++)
               {
                  options->rotationMatrix[i][j] = mata[i+1][j+1];
               }
            }
         }
         break;
      case FOLIATION:
         interpolated = FALSE;
         break;
      case LINEATION:
         interpolated = FALSE;
         break;
      default:
         interpolated = FALSE;
   }
   
   return (interpolated);
}

/* ======================================================================
FUNCTION        updateMovieTitle

DESCRIPTION
        update the window name to reflect which frame of the movie
        is showing

RETURNED    TRUE
====================================================================== */
int
#if XVT_CC_PROTO
updateMovieTitle (WINDOW win)
#else
updateMovieTitle (win)
WINDOW win;
#endif
{
   char title[50], title2[50], *ptr;
   MOVIE_DATA *movieData;
   int numFrames, frame;
   
   if (!win)
      return (FALSE);
      
   xvt_vobj_get_title (win, title, 50);
   for (ptr = title; (*ptr != ' ') && (*ptr != '\0'); ptr++)
      ;
   *ptr = '\0';  /* cut of rest of title */

                   /* Work out position in movie */
   if (movieData = (MOVIE_DATA *) xvt_vobj_get_data (win))
   {                             /* current Position */
      for (frame = 0; movieData->prev; frame++)
         movieData = movieData->prev;
                                 /* Total Frames */
      for (numFrames = 0; movieData->next; numFrames++)
         movieData = movieData->next;

      sprintf (title2, " (%d of %d)", frame+1, numFrames+1);

      strcat (title, title2);
      xvt_vobj_set_title (win, title);
   }
   
   return (TRUE);
}



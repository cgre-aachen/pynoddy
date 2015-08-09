#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include "status.h"
#include "titles.h"

#define DEBUG(X)    X
#define DEBUG2(X)   

#define MENU1_ROWS      NUM_MENU_OPTIONS
#define MENU1_COLS      1
#define MENU2_ROWS      6
#define MENU2_COLS      3
#define MENU3_ROWS      1
#define MENU3_COLS      NUM_MENU_OPTIONS

#define NUM_MENU_OPTIONS        14  /* num options in floating menu */

char *commandLine = 0;

static int menuItems[NUM_MENU_OPTIONS] = {
          POINTER_ICON,
          STRATIGRAPHY_ICON,
          FOLD_ICON,
          FAULT_ICON,
          UNCONFORMITY_ICON,
          SHEAR_ICON,
          DYKE_ICON,
          PLUG_ICON,
          STRAIN_ICON,
          TILT_ICON,
          FOLIATION_ICON,
          LINEATION_ICON,
          IMPORT_ICON,
          STOP_ICON   };
static RCT menuItemPositions[NUM_MENU_OPTIONS];

static char *statusBarProps[] = {
   "TASK_WIN Status Bar", /* title */
   /* Property List */
   "DEBUG=FALSE",
   "DEFAULT_TEXT=\"Status Bar\"",
   "FIELD_OFFSET=16",
   "FILL_COLOR=LTGRAY",
   "FONTSIZE=12",
   "LEFT_OFFSET=8",
   "PLAIN=FALSE",
   "TASK_WIN=TRUE",
   "TEXT_COLOR=BLACK",
   /* ACE Instance Data */
   "AUTOSIZED",
   NULL /* NULL-terminate the array */
};
static int numStatusBarProps = 11;
WINDOW statusBar = NULL_WIN;


extern double **topographyMap;
extern int TopoCol, TopoRow;
extern BOOLEAN processingLongJob;
extern int batchExecution;
extern WINDOW_INFO batchWindowInfo;
extern ANOMIMAGE_DATA *gravityReferenceData, *magneticsReferenceData;
extern WINDOW currentPreviewWindow;

static WINDOW menuWindow = NULL_WIN;
static WINDOW historyWindow = NULL_WIN;
static BOOLEAN creatingObject = FALSE;
static FILE_SPEC currentFileName;

                              /* ************************ */
                              /* Buffers for Undo Command */
#define DELETE_UNDO_POSITONS  -1
                 /* linked list of object + data of delete objects */
static OBJECT *lastRemovedObjects = (OBJECT *) NULL; 
       /* array pointers to objects in the linked list that were last added */
static int numLastAddedObjects = 0;
static OBJECT **lastAddedObjects = (OBJECT **) NULL;
        /* linked list of the objects (with no data) showing last positions */
static OBJECT *lastObjectPositions = (OBJECT *) NULL;

/*
     All drawing, selecting, etc., is in terms of logical coordinates.
     Following macros convert between logical and physical coordinates.
*/
#define X_LOG_TO_PHYS(wip, x) ((x) - wip->origin.h)
#define Y_LOG_TO_PHYS(wip, y) ((y) - wip->origin.v)
#define X_PHYS_TO_LOG(wip, x) ((x) + wip->origin.h)
#define Y_PHYS_TO_LOG(wip, y) ((y) + wip->origin.v)

/*
     Geometrical constants.
*/
#define PAGE_WIDTH       2000 /* width of page in pixels */
#define PAGE_HEIGHT      4000 /* height of page in pixels */
#define PAGE_INCR        25        /* "line" increment for scrolling */
#define MIN_SIZE         20        /* min. size for object */
#define MIN_MOVE         4         /* min. distance for moves */


/*
     Prototypes and declarations for static functions.
*/
#if XVT_CC_PROTO
int checkOverlap (WINDOW, RCT, OBJECT *);
int pointBetweenObjects (WINDOW, PNT, OBJECT **, OBJECT **);
int makeRoomBetweenObjects (WINDOW, OBJECT **, OBJECT **, OBJECT **,
                                      BOOLEAN, int *, int *);
static int validateStructure (WINDOW, RCT, OBJECTS, BOOLEAN);
void reorderObjects (WINDOW);
OBJECT * addObject (WINDOW, OBJECTS, RCT, char *, BOOLEAN);
void deleteSelectedObjects (WINDOW, BOOLEAN);
void deleteObject (WINDOW, OBJECT *);
int freeImportEventMemory (IMPORT_OPTIONS *);
int freeProfileEventMemory (OBJECT *);
static void obj_update(WINDOW);
static void obj_select(OBJECT *, BOOLEAN);
static void obj_select_enclosed(WINDOW, RCT *);
static void obj_draw_selection(OBJECT *, BOOLEAN);
static OBJECT *findObject (WINDOW, PNT);
static void obj_clear_selections(WINDOW);
static RCT *obj_bound_selected(WINDOW);
static int objMove(OBJECT *, int, int);
static int objMoveSelected(WINDOW, int, int);
static void setObjectBound (OBJECT *, RCT *);
static RCT *phys_rect(WINDOW, RCT *);
static void log_draw_rect(WINDOW, RCT *);
static void log_draw_oval(WINDOW, RCT *);
static void log_draw_roundrect(WINDOW, RCT *, int, int);
static void drawCenteredText(WINDOW, int, int, char *, int);
static void drawIcon_log (WINDOW, RCT *, int);
static void drawIcon (WINDOW, RCT *, int);
static BOOLEAN log_needs_update(WINDOW, RCT *);
static void log_set_clip(WINDOW, RCT *);
void log_invalidate_rect(WINDOW, RCT *);
/* ####################### General Utility Functions ######################## */
WINDOW createPreviewWindow (WINDOW, WINDOW);
WINDOW createEventWindow (OBJECT *);
WINDOW createPositionedWindow (int, WINDOW, int, int, EVENT_MASK, EVENT_HANDLER, long, RCT *);
WINDOW createCenteredWindow (int, WINDOW, EVENT_MASK, EVENT_HANDLER, long);
WINDOW createCenteredDialog (int, EVENT_MASK, EVENT_HANDLER, long);
void tidyObjects (WINDOW);
void copyUndoPositions (WINDOW_INFO *, int);
void copyUndoAdditions (OBJECT *, int);
void copyUndoDeletions (OBJECT *, int);
int undoChanges (WINDOW);
OBJECT *nthObject (WINDOW, int);
int totalObjects (WINDOW);
int countObjects (WINDOW);
int sizeofOptions (OBJECTS);
WINDOW getEventDrawingWindow ();
WINDOW getFloatingMenuWindow ();
void gridCorrectRect (RCT *grid_rctp, RCT *rctp);
static void normalizeRect (RCT *, RCT *);
static void yToRow (WINDOW, int, int *);
static void xToColumn (WINDOW, int, int *);
static void rowToY (WINDOW, int, int *, int *);
static void columnToX (WINDOW, int, int *, int *);
void editObjectParams (OBJECT *);
static void createObject(WINDOW, RCT);
static BOOLEAN moveObject(WINDOW, OBJECT *, PNT, PNT, int);
static void drawArrowBetween (WINDOW win, OBJECT *, OBJECT *);
static void drawObjectLinks (WINDOW win);
static void rubberRect (WINDOW, RCT *);
static void shift_view(WINDOW, int, int, PNT *);
static void autoscroll(WINDOW, EVENT *);
BOOLEAN doMouse(WINDOW, EVENT *);
static void updateDrawingWindow (WINDOW);
void updateFloatingMenu (WINDOW, OBJECT *, TOOL);
int setFloatingMenuShape (WINDOW, int, int);
int getFloatingMenuShape (WINDOW, int *, int *);
static void setCurrentObject (WINDOW, MENU_TAG);
static void doScroll (WINDOW, EVENT *);
static void scroll_sync(WINDOW);
static void setupWindow(WINDOW);
void setCurrentFileName (FILE_SPEC *);
void getCurrentFileName (FILE_SPEC *);
long windowEventHandler (WINDOW, EVENT *);
static long menuEventHandler (WINDOW, EVENT *);
void selectAllObjects ();
WINDOW_INFO *get_win_info(WINDOW win);
#else
int checkOverlap ();
int pointBetweenObjects ();
int makeRoomBetweenObjects ();
static int validateStructure ();
void reorderObjects ();
OBJECT * addObject ();
void deleteSelectedObjects ();
void deleteObject ();
int freeImportEventMemory ();
int freeProfileEventMemory ();
static void obj_update();
static void obj_select();
static void obj_select_enclosed();
static void obj_draw_selection();
static OBJECT *findObject ();
static void obj_clear_selections();
static RCT *obj_bound_selected();
static int objMove();
static int objMoveSelected();
static void setObjectBound ();
static RCT *phys_rect();
static void log_draw_rect();
static void log_draw_oval();
static void log_draw_roundrect();
static void drawCenteredText();
static void drawIcon_log ();
static void drawIcon ();
static BOOLEAN log_needs_update();
static void log_set_clip();
void log_invalidate_rect();
/* ####################### General Utility Functions ######################## */
WINDOW createPreviewWindow ();
WINDOW createEventWindow ();
WINDOW createPositionedWindow ();
WINDOW createCenteredWindow ();
WINDOW createCenteredDialog ();
void tidyObjects ();
void copyUndoPositions ();
void copyUndoAdditions ();
void copyUndoDeletions ();
int undoChanges ();
OBJECT *nthObject ();
int totalObjects ();
int countObjects ();
int sizeofOptions ();
WINDOW getEventDrawingWindow ();
WINDOW getFloatingMenuWindow ();
void gridCorrectRect ();
static void normalizeRect ();
static void yToRow ();
static void xToColumn ();
static void rowToY ();
static void columnToX ();
void editObjectParams ();
static void createObject();
static BOOLEAN moveObject();
static void drawArrowBetween ();
static void drawObjectLinks ();
static void rubberRect();
static void shift_view();
static void autoscroll();
BOOLEAN doMouse();
static void updateDrawingWindow ();
void updateFloatingMenu ();
int setFloatingMenuShape ();
int getFloatingMenuShape ();
static void setCurrentObject ();
static void doScroll ();
static void scroll_sync();
static void setupWindow();
void setCurrentFileName ();
void getCurrentFileName ();
long windowEventHandler ();
static long menuEventHandler ();
void selectAllObjects ();
WINDOW_INFO *get_win_info();
#endif


/*
**   Following group of functions (obj_*) manipulate objects with access
**   to the internals of the OBJECT data structure.
*/
/*
     Function to set the bounds and handles of an object.
*/
static void
#if XVT_CC_PROTO
setObjectBound (OBJECT *p, RCT *bound)
#else
setObjectBound (p, bound)
register OBJECT *p;
RCT *bound;
#endif
{
     p->bound = *bound;
}

/*
     Function to draw (or clear) selection handles.
*/
static void
#if XVT_CC_PROTO
obj_draw_selection(OBJECT *p, BOOLEAN selected)
#else
obj_draw_selection(p, selected)
register OBJECT *p;
BOOLEAN selected;
#endif
{
   DRAW_CTOOLS t;
   RCT boundingRectangle;

   if (!selected)
   {
      log_invalidate_rect (historyWindow, &(p->bound));
      return;
   }
     
   xvt_app_get_default_ctools(&t);
   t.mode = M_COPY;
   t.pen.width = 2;
   t.pen.pat = PAT_SOLID;
   t.pen.color = COLOR_RED;
   t.brush.pat = PAT_HOLLOW;
   xvt_dwin_set_draw_ctools(historyWindow, &t);

               /* draw a bounding rectangle around the selected object */
   boundingRectangle.top = p->bound.top;
   boundingRectangle.left = p->bound.left;
   boundingRectangle.bottom = p->bound.bottom;
   boundingRectangle.right = p->bound.right;

   log_draw_rect(historyWindow, &boundingRectangle);

}

/*
     Function to select or deselect an object.
*/
static void
#if XVT_CC_PROTO
obj_select(OBJECT *p, BOOLEAN select)
#else
obj_select(p, select)
register OBJECT *p;
BOOLEAN select;
#endif
{
     if (p->selected != select) {
          obj_draw_selection(p, select);
          p->selected = select;
     }
}

/*
     Function to select all objects
*/
void
#if XVT_CC_PROTO
selectAllObjects ()
#else
selectAllObjects ()
#endif
{
   OBJECT *object;
   WINDOW eventDrawWindow = getEventDrawingWindow ();
   WINDOW_INFO *wip;

   wip = get_win_info(eventDrawWindow);

   for (object = wip->head; object != NULL; object = object->next)
      obj_select (object, TRUE);
}

/*
     Get pointer to window info data
*/
WINDOW_INFO *
#if XVT_CC_PROTO
get_win_info(WINDOW win)
#else
get_win_info(win)
WINDOW win;
#endif
{
     WINDOW_INFO *wip;

     if (batchExecution)
        return (&batchWindowInfo);

     if (win)
        wip = (WINDOW_INFO *) xvt_vobj_get_data(win);
     else
        wip = (WINDOW_INFO *) NULL;

     return wip;
}

/*
     Function to select objects enclosed by a given rectangle. If the rectangle
     pointer is NULL, all objects are selected.
*/
static void
#if XVT_CC_PROTO
obj_select_enclosed(WINDOW win, RCT *rctp)
#else
obj_select_enclosed(win, rctp)
WINDOW win;
RCT *rctp;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     register OBJECT *p;

     for (p = wip->head; p != NULL; p = p->next)
          if (rctp == NULL || (p->bound.left >= rctp->left &&
            p->bound.top >= rctp->top && p->bound.right <= rctp->right &&
            p->bound.bottom <= rctp->bottom))
               obj_select(p, TRUE);
}

/*
     Function to deselect all selected object.
*/
static void
#if XVT_CC_PROTO
obj_clear_selections(WINDOW win)
#else
obj_clear_selections(win)
WINDOW win;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     register OBJECT *p;

     for (p = wip->head; p != NULL; p = p->next)
          if (p->selected) {
               obj_draw_selection(p, FALSE);
               p->selected = FALSE;
          }
}

/*
     Function to determine bounding box that encloses all selected objects.
*/
static RCT *
#if XVT_CC_PROTO
obj_bound_selected(WINDOW win)
#else
obj_bound_selected(win)
WINDOW win;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     register OBJECT *p;
     static RCT rct;
     BOOLEAN first = TRUE;

     for (p = wip->head; p != NULL; p = p->next)
          if (p->selected) {
               if (first) {
                    first = FALSE;
                    rct = p->bound;
               }
               rct.left = MIN(rct.left, p->bound.left);
               rct.top = MIN(rct.top, p->bound.top);
               rct.right = MAX(rct.right, p->bound.right);
               rct.bottom = MAX(rct.bottom, p->bound.bottom);
          }
     return(&rct); /* may be empty rect */
}

/*
     Function to reorder the objects in the list according to their position
*/
int
#if XVT_CC_PROTO
checkOverlap (WINDOW win, RCT bound, OBJECT *p)
#else
checkOverlap (win, bound, p)
WINDOW win;
RCT bound;
OBJECT *p;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   register OBJECT *current;
   int overlap = FALSE;

   for (current = wip->head; current != NULL; current = current->next)
   {
      if (p != current)
      {
         if (!(p && p->selected && current->selected))
         {
            if ((bound.left == current->bound.left)
              && (bound.right == current->bound.right)
              && (bound.top == current->bound.top)
              && (bound.bottom == current->bound.bottom))
            {
               overlap = TRUE;
               break;
            }
         }
      }
   }

   if (overlap)
   {
      if (batchExecution)
         fprintf (stderr, "You cannot place an object on top of an existing object");
      else
         xvt_dm_post_error("You cannot place an object on top of an existing object");
   }

   return (overlap);
}

/* ======================================================================
FUNCTION  makeRoomBetweenObjects

DESCRIPTION
     make enough room between object1, and object 2 to be able
        to place objectIgnore or the current selection (depending on
        the value of selected)
CHANGED
   The spacing of the icons
   offsetX, offsetY - the new offsets needed to place the object(s) into
                      the space provided.

RETURNED
   0 - if no room needed to be made
   1 - if room was made
     (or atleast the closest position to it)

====================================================================== */
int
#if XVT_CC_PROTO
makeRoomBetweenObjects (WINDOW win, OBJECT **object1, OBJECT **object2,
                        OBJECT **objectIgnore, BOOLEAN selected,
                        int *offsetX, int *offsetY)
#else
makeRoomBetweenObjects (win, object1, object2, objectIgnore, selected,
                        offsetX, offsetY)
WINDOW win;
OBJECT **object1, **object2, **objectIgnore;
BOOLEAN selected;
int *offsetX, *offsetY;
#endif
{
   int sameRow;
   int spaceNeededX = 0, spaceNeededY = 0;
   int actualSpaceX = 0, actualSpaceY = 0;
   PNT topLeftStart, topLeftEnd;
   RCT selectionBound;
   int spaceAdded = FALSE;
   OBJECT *p;

   if (((*object1)->row == (*object2)->row) && ((*object1)->row == 1))
      sameRow = TRUE;
   else if ((*object1)->column == (*object2)->column)
      sameRow = FALSE;
   else
      return (spaceAdded); /* should never happen as cannot be between them */


       /* we need to make room for whatever is being placed to fit */
   if (sameRow)
   {   /* everything needs to be shifted in the X direction after object2 */
      if (selected)
      {
         selectionBound = *obj_bound_selected (win);
         spaceNeededX = (selectionBound.right - selectionBound.left)
                                              + (GRID_WIDTH - ICON_SIZE)*2;
         topLeftStart.v = selectionBound.top;
         topLeftStart.h = selectionBound.left;
      }
      else
      {
         spaceNeededX = GRID_WIDTH + (GRID_WIDTH - ICON_SIZE);

         if (objectIgnore)
         {
            topLeftStart.v = (*objectIgnore)->bound.top;
            topLeftStart.h = (*objectIgnore)->bound.left;
         }
      }

      topLeftEnd.v = (short) ((*object1)->bound.top);
      topLeftEnd.h = (short) ((*object1)->bound.left + GRID_WIDTH);

      actualSpaceX = (*object2)->bound.left - (*object1)->bound.right;

             /* move everything along  in X direction */
      if (actualSpaceX < spaceNeededX)
      {
         for (p = *object2; p != NULL; p = p->next)
         {
            if (objectIgnore)
            {
                   /* only offset ones not involved in the actual move */
               if (((!selected) && (p != *objectIgnore))
                                  || ((selected) && (!(p->selected))))
               {
                  p->bound.left += spaceNeededX - actualSpaceX;
                  p->bound.right += spaceNeededX - actualSpaceX;
                  gridCorrectRect (&(p->bound), &(p->bound));
                  setObjectBound (p, &(p->bound));
                  spaceAdded = TRUE;
               }
            }
            else
            {
                   /* only offset ones not involved in the actual move */
               if ((!selected) || ((selected) && (!(p->selected))))
               {
                  p->bound.left += spaceNeededX - actualSpaceX;
                  p->bound.right += spaceNeededX - actualSpaceX;
                  gridCorrectRect (&(p->bound), &(p->bound));
                  setObjectBound (p, &(p->bound));
                  spaceAdded = TRUE;
               }
            }
         }
      }
   }
   else /* same Column */
   {   /* everything in this column needs to be shifted in the Y direction */
      if (selected)
      {
         selectionBound = *obj_bound_selected (win);
         spaceNeededY = (selectionBound.bottom - selectionBound.top)
                                              + (GRID_HEIGHT - ICON_SIZE)*2;
         topLeftStart.v = selectionBound.top;
         topLeftStart.h = selectionBound.left;
      }
      else
      {
         spaceNeededY = GRID_HEIGHT + (GRID_HEIGHT - ICON_SIZE);

         if (objectIgnore)
         {
            topLeftStart.v = (*objectIgnore)->bound.top;
            topLeftStart.h = (*objectIgnore)->bound.left;
         }
      }

      topLeftEnd.v = (*object1)->bound.top + GRID_HEIGHT;
      topLeftEnd.h = (*object1)->bound.left;
           /* in a selection object1 and object2 may not be int he same
           ** column as the top left corner so the offset will need to be
           ** offset by the position of the movement within the selection */
      if (selected && objectIgnore)
      {
         topLeftEnd.v += selectionBound.top - (*objectIgnore)->bound.top;
         topLeftEnd.h += selectionBound.left - (*objectIgnore)->bound.left;
      }

      actualSpaceY = (*object2)->bound.top - (*object1)->bound.bottom;

             /* move everything in same column along in Y direction */
      if (actualSpaceY < spaceNeededY)
      {
         for (p = *object2; (p != NULL) && (p->column == (*object2)->column);
                            p = p->next)
         {
            if (objectIgnore)
            {
                   /* only offset ones not involved in the actual move */
               if (((!selected) && (p != *objectIgnore))
                                  || ((selected) && (!(p->selected))))
               {
                  p->bound.top += spaceNeededY - actualSpaceY;
                  p->bound.bottom += spaceNeededY - actualSpaceY;
                  gridCorrectRect (&(p->bound), &(p->bound));
                  setObjectBound (p, &(p->bound));
                  spaceAdded = TRUE;
               }
            }
            else
            {
                   /* only offset ones not involved in the actual move */
               if ((!selected) || ((selected) && (!(p->selected))))
               {
                  p->bound.top += spaceNeededY - actualSpaceY;
                  p->bound.bottom += spaceNeededY - actualSpaceY;
                  gridCorrectRect (&(p->bound), &(p->bound));
                  setObjectBound (p, &(p->bound));
                  spaceAdded = TRUE;
               }
            }
         }
      }
   }

               /* only assign the offsets if we have made space
               ** and the pointers are not NULL */
   if (spaceAdded && offsetX && offsetY)
   {
      *offsetX = topLeftEnd.h - topLeftStart.h;
      *offsetY = topLeftEnd.v - topLeftStart.v;
   }

   return (spaceAdded);
}

/*
     Check to see if a point is between to objects
*/
int
#if XVT_CC_PROTO
pointBetweenObjects (WINDOW win, PNT point, OBJECT **object1, OBJECT **object2)
#else
pointBetweenObjects (win, point, object1, object2)
WINDOW win;
PNT point;
OBJECT **object1, **object2;
#endif
{
   int betweenObjects = FALSE;
   WINDOW_INFO *wip = get_win_info(win);
   OBJECT *p, *next;
   int allDone = FALSE;
     
                /* check the horizontal connections first */
   for (p = wip->head; (p != NULL) && (p->next != NULL); )
   {
      next = p->next;
      
                    /* skip over the ones in columns */
      while (p->bound.left == next->bound.left)
      {
         if (next->next)
            next = next->next;
         else
         {
            allDone = TRUE;
            break;
         }
      }

      if (allDone)
         break;

                /* check to see if point is between these two */
      if ((point.v > p->bound.top) && (point.v < p->bound.bottom) &&
          (point.h > p->bound.right) && (point.h < next->bound.left))
      {
         betweenObjects = TRUE;
         *object1 = p;
         *object2 = next;

         return (betweenObjects);
      }

      p = next;
   }

                /* now draw in the vertical connections */
   for (p = wip->head; (p != NULL) && (p->next != NULL); p = p->next)
   {
      if (p->bound.left == p->next->bound.left)
      {
                /* check to see if point is between these two */
         if ((point.h > p->bound.left) && (point.h < p->bound.right) &&
             (point.v > p->bound.bottom) && (point.v < p->next->bound.top))
         {
            betweenObjects = TRUE;
            *object1 = p;
            *object2 = p->next;
            return (betweenObjects);
         }
      }
   }


   return (betweenObjects);
}

/*
     Function to make sure that a valid noddy structure will be producted
*/
static int
#if XVT_CC_PROTO
validateStructure (WINDOW win, RCT pos, OBJECTS object, BOOLEAN creating)
#else
validateStructure (win, pos, object, creating)
WINDOW win;
RCT pos;
OBJECTS object;
BOOLEAN creating;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   register OBJECT *current;
   int validOrder = TRUE;
   int validObject = TRUE;


   current = wip->head;

   if ((current != NULL) && (current->shape == STRATIGRAPHY)
                         && (object == STRATIGRAPHY))
   {
                        /* we can only have 1 stratigraphy */
      if (creating)
         validObject = FALSE;
      else
      {  /* also cannot move the stratigraphy down lower than the top line **
         ** or further right than the right edge of events */
         for (current = wip->head->next;
                              current != NULL; current = current->next)
         {
            if (pos.left > current->bound.left)
            {
               validOrder = FALSE;
               break;
            }
            if (pos.top > current->bound.top)
            {
               validOrder = FALSE;
               break;
            }
         }
      }
   }

             /* decide if the current head is the one to check or
             ** the item being created or moved */
   current = wip->head;
   if ((current != NULL) && 
      ((current->bound.left < pos.left) ||
       ((current->bound.left == pos.left)
                        && (current->bound.top < pos.top))))
   {            /* current is the first item */
      if (current->shape != STRATIGRAPHY)
         validOrder = FALSE;
   }
   else  /* the new one is going to be first */
   {
      if (object != STRATIGRAPHY)
         validOrder = FALSE;
   }

             /* make sure the first one is always a statrigraphy */
   if (!validOrder)
   {
      if (batchExecution)
         fprintf (stderr, "A Stratigraphy must always come first");
      else
         xvt_dm_post_error("A Stratigraphy must always come first");
   }

             /* make sure there is only one stratigraphy */
   if (!validObject)
   {
      if (batchExecution)
         fprintf (stderr, "You may only have one Stratigraphy");
      else
         xvt_dm_post_error("You may only have one Stratigraphy");
   }

   if ((!validOrder) || (!validObject))
      return (0);
   else
      return (1);
}


/*
     Function to reorder the objects in the list according to their position
*/
void
#if XVT_CC_PROTO
reorderObjects (WINDOW win)
#else
reorderObjects (win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   register OBJECT *first, *current;
   register OBJECT *startNewList = NULL, *newList;
   register OBJECT *beforeFirst, *beforeCurrent;
   int row, column, difference;

               /* no list to order so just exit */
   if (wip->head == NULL)
      return;  
       /* Dont order anything with an invisable event in the history */
   for (current = wip->head; current; current = current->next)
      if (!current->drawEvent)
         return;

   while (wip->head != NULL)
   {
                      /* assume first element is the smallest */
      for (first = wip->head, current = first->next,
           beforeFirst = NULL, beforeCurrent = first;
                        current != NULL;
                              beforeCurrent = current, current = current->next)
      {
                                   /* we are ordering from left to right */
         if (current->bound.left < first->bound.left)  
         {
            first = current;
            beforeFirst = beforeCurrent;
         }
         else if (current->bound.left == first->bound.left)  
         {      /* when in the same X position the top one takes precedence */
            if (current->bound.top < first->bound.top)
            {
               first = current;
               beforeFirst = beforeCurrent;
            }
         }
      }

      if (startNewList == NULL)
      {
             /* remove element from wip list */
         if (beforeFirst == NULL)
            wip->head = first->next;
         else
            beforeFirst->next = first->next;

             /* add it to our new list */
         startNewList = first;  /* initialise the new list to this first one */
         newList = first;    /* the end of the list is the same as the start */
      }
      else
      {
             /* remove element from wip list */
         if (beforeFirst == NULL)
            wip->head = first->next;
         else
            beforeFirst->next = first->next;

             /* add it to our new list */
         newList->next = first;  /* append next smallest to end of new list */
         newList = first;  /* keep newList pointing to last element in list */
      }
     
   }

   newList->next = NULL;     /* the last appended is the last in the list */

   wip->head = startNewList;  /* make the wip list use our sorted one */

              /* make sure the Row and column fields are right */
   row = 1; column = 1;
   for (current = wip->head; current != NULL; current = current->next)
   {
      current->row = row;
      current->column = column;

             /* make sure that the top layer is always in a line */
      if (current->row == 1)
      {
         if (current->bound.top != wip->head->bound.top)
         {
            difference = wip->head->bound.top - current->bound.top;
            current->bound.top += difference;
            current->bound.bottom += difference;
            setObjectBound (current, &current->bound);
         }
      }

      if ((current->next)
                && (current->bound.left == current->next->bound.left))
      {
         row++;  /* next one is a row down in this column */
      }
      else  /* moving to the next column and therefore to the top of a row */
      {
         row = 1;
         column++;
      }
   }

}

/*
     Function to add an object to a window's list.
*/
OBJECT *
#if XVT_CC_PROTO
addObject (WINDOW win, OBJECTS shape, RCT bound, char *name, BOOLEAN selected)
#else
addObject (win, shape, bound, name, selected)
WINDOW win;
OBJECTS shape;
RCT bound;
char *name;
BOOLEAN selected;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   register OBJECT *p;
   char *optionsStructure;
   static int count;
   char buf[50];

                     /* malloc the basic structure */
   p = (OBJECT *) xvt_mem_alloc(sizeof(OBJECT));
   xvt_errmsg_sig_if(!(p != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory.");
   memset((char *)p, 0, sizeof(OBJECT));
                    /* malloc the options structure */
   switch (shape)
   {
      case STRATIGRAPHY:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (STRATIGRAPHY_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case FOLD:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (FOLD_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case FAULT:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (FAULT_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case UNCONFORMITY:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (UNCONFORMITY_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case SHEAR_ZONE:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (SHEAR_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case DYKE:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (DYKE_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case PLUG:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (PLUG_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case STRAIN:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (STRAIN_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case TILT:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (TILT_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case FOLIATION:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (FOLIATION_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case LINEATION:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (LINEATION_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case IMPORT:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (IMPORT_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      case GENERIC:
         optionsStructure = (char *) xvt_mem_zalloc(sizeof (GENERIC_OPTIONS));
         xvt_errmsg_sig_if(!(optionsStructure != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"107",
          107, "addObject:  Out of memory for Options.");
         break;
      default:
         optionsStructure = NULL;
   }

                    /* assign the items in these stuctures */
   p->options = optionsStructure;
   p->drawEvent = TRUE;
   p->shape = shape;
   if (name == NULL)
   {
      sprintf(buf, "Event %d", ++count);
      setObjectName (p, buf);
   }
   else
      setObjectName (p, name);
   setObjectBound (p, &bound);
   p->selected = selected;
   setDefaultOptions (p);
   p->newObject = TRUE;

   p->next = wip->head;
   wip->head = p;

   reorderObjects (win);  /* make sure they are in the current order */

   if (!batchExecution)
		log_invalidate_rect(win, &(p->bound));
   copyUndoAdditions ((OBJECT *) NULL, FALSE);
   copyUndoAdditions (p, FALSE);
   editObjectParams (p);

   invalidateCurrentLineMap ();

	return (p);
}

/*
     Function to destroy a window's selected objects, or all its objects.
*/
void
#if XVT_CC_PROTO
deleteSelectedObjects (WINDOW win, BOOLEAN all)
#else
deleteSelectedObjects (win, all)
WINDOW win;
BOOLEAN all;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   OBJECT *p, *prev, *next;
   BOOLEAN allSelected = TRUE;

                      /* first check to see if everything is selected */
   if (!all)
   {
      for (p = wip->head; p != NULL; p = p->next)
      {                  /* look until it finds one that is not selected */
         if (!(p->selected))
         {
            allSelected = FALSE;
            break;
         }
      }
      all = allSelected;
   }

   prev = NULL; /* suppress gratuitous warning from MPW C */
   for (p = wip->head; p != NULL; p = next)
   {
      next = p->next;
      if (all || p->selected)
      {
         if ((!all) && (p->shape == STRATIGRAPHY) && (p == wip->head))
         {
            if (batchExecution)
               fprintf (stderr, "You must always have a Stratigraphy as the first event");
            else
               xvt_dm_post_error("You must always have a Stratigraphy as the first event");
            prev = p;
         }
         else
         {
            copyUndoDeletions (p, FALSE);
            if (p == wip->head)
               wip->head = next;
            else
              prev->next = next;

                /* note we dont free any malloced memory in the event as
                ** that is needed in the undo procedure */
            if (p->options)
               xvt_mem_free((char *)p->options);
            xvt_mem_free((char *)p);
         }
      }
      else
         prev = p;
   }

   copyUndoPositions(wip, TRUE);

   reorderObjects (win);  /* make sure they are in the correct order */
   log_invalidate_rect(win, NULL);
   invalidateCurrentLineMap ();
   updateMenuOptions (TASK_MENUBAR, NULL_WIN);
   updateFloatingMenu (menuWindow, wip->head, wip->tool);
}

/*
     Function to destroy a window's selected objects, or all its objects.
*/
void
#if XVT_CC_PROTO
deleteObject (WINDOW win, OBJECT *object)
#else
deleteObject (win, object)
WINDOW win;
OBJECT *object;
#endif
{
   WINDOW_INFO *wip;
   OBJECT *p, *prev, *next;

   if (!win)
      win = getEventDrawingWindow ();
   wip = get_win_info(win);

   prev = NULL; /* suppress gratuitous warning from MPW C */
   for (p = wip->head; p != NULL; p = next)
   {
      next = p->next;
      if (p == object)
      {
         copyUndoDeletions (object, FALSE);

         if (p == wip->head)
            wip->head = next;
         else
           prev->next = next;

                /* note we dont free any malloced memory in the event as
                ** that is needed in the undo procedure */
         if (p->options)
            xvt_mem_free((char *)p->options);
         xvt_mem_free((char *)p);
      }
      else
         prev = p;
   }

   copyUndoPositions(wip, TRUE);

   reorderObjects (win);  /* make sure they are in the correct order */
   log_invalidate_rect(win, NULL);
   updateMenuOptions (TASK_MENUBAR, NULL_WIN);
   updateFloatingMenu (menuWindow, wip->head, wip->tool);
}

/*
     Free the memory that can be malloced with the options of an import event
*/
int
#if XVT_CC_PROTO
freeImportEventMemory (IMPORT_OPTIONS *importOptions)
#else
freeImportEventMemory (importOptions)
IMPORT_OPTIONS *importOptions;
#endif
{
   if (importOptions->properties)
   {
      xvt_mem_free ((char *) importOptions->properties);
      importOptions->properties = NULL;
      importOptions->numProperties = 0;
   }
   if (importOptions->blockData)
   {
      destroy3DIregArray((char ***) importOptions->blockData,
                          importOptions->numLayers,
                          importOptions->layerDimensions);
      importOptions->blockData = NULL;
   }                                         
   if (importOptions->layerDimensions)
   {
      destroy2DArray((char **) importOptions->layerDimensions,
                     importOptions->numLayers, 2);
      importOptions->layerDimensions = NULL;
      importOptions->numLayers = 0;
   }
   return (TRUE);
}

/*
     Free the memory that can be malloced with the options 
     that has an alteration zone or profile
*/
int
#if XVT_CC_PROTO
freeProfileEventMemory (OBJECT *object)
#else
freeProfileEventMemory (object)
OBJECT *object;
#endif
{
   PROFILE_OPTIONS *profOptions = NULL,
                   *altOptions = NULL;
   
   switch (object->shape)
   {
      case (UNCONFORMITY):
         {
            UNCONFORMITY_OPTIONS *unconOptions;
            unconOptions = (UNCONFORMITY_OPTIONS *) object->options;
            altOptions = unconOptions->alterationFuntions;
         }
         break;
      case (FOLD):
         {
            FOLD_OPTIONS *foldOptions;
            foldOptions = (FOLD_OPTIONS *) object->options;
            profOptions = &(foldOptions->profile);
         }
         break;
      case (FAULT):
         {
            FAULT_OPTIONS *faultOptions;
            faultOptions = (FAULT_OPTIONS *) object->options;
            altOptions = faultOptions->alterationFuntions;
            profOptions = &(faultOptions->profile);
         }
         break;
      case (SHEAR_ZONE):
         {
            SHEAR_OPTIONS *shearOptions;
            shearOptions = (SHEAR_OPTIONS *) object->options;
            altOptions = shearOptions->alterationFuntions;
            profOptions = &(shearOptions->profile);
         }
         break;
      case (DYKE):
         {
            DYKE_OPTIONS *dykeOptions;
            dykeOptions = (DYKE_OPTIONS *) object->options;
            altOptions = dykeOptions->alterationFuntions;
         }
         break;
      case (PLUG):
         {
            PLUG_OPTIONS *plugOptions;
            plugOptions = (PLUG_OPTIONS *) object->options;
            altOptions = plugOptions->alterationFuntions;
         }
         break;
   }


   if (profOptions)
   {
      profilePointsFree (profOptions->points);
      freeArrayForProfile (profOptions->array);
   }
   
   if (altOptions)
   {
      for (profOptions = altOptions; profOptions; profOptions = altOptions)
      {
         altOptions = profOptions->next;
         profilePointsFree (profOptions->points);
         freeArrayForProfile (profOptions->array);
         xvt_mem_free ((char *) profOptions);
      }
   }

   return (TRUE);
}

int
#if XVT_CC_PROTO
freeObjectMemory (OBJECT *object)
#else
freeObjectMemory (object)
OBJECT *object;
#endif
{
   if (!object)
      return (FALSE);

   freeProfileEventMemory(object);

   switch (object->shape)
   {
      case STRATIGRAPHY:
         {
            STRATIGRAPHY_OPTIONS *options = getStratigraphyOptionsStructure(object);
            
            if (options->properties)
               xvt_mem_free ((char *) options->properties);
         }
         break;
      case FOLD:
         break;
      case FAULT:
         {
            FAULT_OPTIONS *options = (FAULT_OPTIONS *) object->options;
            DXFfree(&(options->dxfData));
         }
         break;
      case UNCONFORMITY:
         {
            UNCONFORMITY_OPTIONS *options = (UNCONFORMITY_OPTIONS *) object->options;
            STRATIGRAPHY_OPTIONS *stratOptions = getStratigraphyOptionsStructure(object);
            
            DXFfree(&(options->dxfData));
            if (stratOptions->properties)
               xvt_mem_free ((char *) stratOptions->properties);
         }
         break;
      case SHEAR_ZONE:
         {
            SHEAR_OPTIONS *options = (SHEAR_OPTIONS *) object->options;
            DXFfree(&(options->dxfData));
         }
         break;
      case DYKE:
         break;
      case PLUG:
         break;
      case STRAIN:
         break;
      case TILT:
         break;
      case FOLIATION:
         break;
      case LINEATION:
         break;
      case IMPORT:
         {
            freeImportEventMemory ((IMPORT_OPTIONS *) object->options);
         }
         break;
      case GENERIC:
         break;
      case STOP:
         break;
   }

   if (object->options)
      xvt_mem_free ((char *) object->options);
   xvt_mem_free ((char *) object);
   
   return (TRUE);
}

/*
     Function to update an object.
*/
static void
#if XVT_CC_PROTO
obj_update(WINDOW win)
#else
obj_update(win)
WINDOW win;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     register OBJECT *p;
     DRAW_CTOOLS t;
     int ascent, decent, textPos;
     char label[40];
     char word1[40], word2[40];

     if (batchExecution)
        return;

     xvt_app_get_default_ctools(&t);
     t.pen.pat = PAT_SOLID;
     t.opaque_text = TRUE;
     xvt_dwin_set_draw_ctools(win, &t);
     xvt_dwin_get_font_metrics(win, NULL, &ascent, &decent);

     for (p = wip->head; p != NULL; p = p->next)
     {
           switch (p->shape)
           {
            case STRATIGRAPHY:
               drawIcon_log (win, &p->bound, STRATIGRAPHY_ICON);
               break;
            case FOLD:
               drawIcon_log (win, &p->bound, FOLD_ICON);
               break;
            case FAULT:
               drawIcon_log (win, &p->bound, FAULT_ICON);
               break;
            case UNCONFORMITY:
               drawIcon_log (win, &p->bound, UNCONFORMITY_ICON);
               break;
            case SHEAR_ZONE:
               drawIcon_log (win, &p->bound, SHEAR_ICON);
               break;
            case DYKE:
               drawIcon_log (win, &p->bound, DYKE_ICON);
               break;
            case PLUG:
               drawIcon_log (win, &p->bound, PLUG_ICON);
               break;
            case STRAIN:
               drawIcon_log (win, &p->bound, STRAIN_ICON);
               break;
            case TILT:
               drawIcon_log (win, &p->bound, TILT_ICON);
               break;
            case FOLIATION:
               drawIcon_log (win, &p->bound, FOLIATION_ICON);
               break;
            case LINEATION:
               drawIcon_log (win, &p->bound, LINEATION_ICON);
               break;
            case IMPORT:
               drawIcon_log (win, &p->bound, IMPORT_ICON);
               break;
            case GENERIC:
               drawIcon_log (win, &p->bound, GENERIC_ICON);
               break;
            case STOP:
               if (p->drawEvent) /* only draw event entered by user */
               {
                  xvt_dwin_set_fore_color (win, COLOR_RED);
                  drawIcon_log (win, &p->bound, STOP_ICON);
                  xvt_dwin_set_fore_color (win, COLOR_BLACK);
               }
               else
                  return;  /* we are makeing a move so forget rest */
               break;
           }
           textPos = p->bound.left + ((p->bound.right - p->bound.left)/2);
           strcpy (word1, ""); strcpy (word2, "");
           sscanf (p->text, "%s %s",word1, word2);
           strncpy (label, word1, 10);
           label[10] = '\0';
           drawCenteredText (win, textPos ,
                           p->bound.bottom + ascent + 2,
                        label, -1);

           strncpy (label, word2, 10);
           label[10] = '\0';
           drawCenteredText (win, textPos ,
                           p->bound.bottom + ascent + decent + ascent + 2,
                        label, -1);
     }

     drawObjectLinks (win);

                                    /* update the screen */
     for (p = wip->head; p != NULL; p = p->next)
        if (p->selected && log_needs_update(win, &p->bound))
           obj_draw_selection(p, TRUE);
}



/*
     Function like xvt_dwin_is_update_needed, but in logical coordinates.
*/
static BOOLEAN
#if XVT_CC_PROTO
log_needs_update(WINDOW win, RCT *rctp)
#else
log_needs_update(win, rctp)
WINDOW win;
RCT *rctp;
#endif
{
     return(xvt_dwin_is_update_needed(win, phys_rect(win, rctp)));
}

/*
     Function like xvt_dwin_set_clip, but in logical coordinates.
*/
static void
#if XVT_CC_PROTO
log_set_clip(WINDOW win, RCT *rctp)
#else
log_set_clip(win, rctp)
WINDOW win;
RCT *rctp;
#endif
{
     xvt_dwin_set_clip(win, rctp == NULL ? NULL : phys_rect(win, rctp));
}

/*
     Function like xvt_dwin_invalidate_rect, but in logical coordinates.
*/
void
#if XVT_CC_PROTO
log_invalidate_rect(WINDOW win, RCT *rctp)
#else
log_invalidate_rect(win, rctp)
WINDOW win;
RCT *rctp;
#endif
{
   RCT refreshArea;
   int widthd2, heightd2;

	if (batchExecution)
		return;

   if (rctp == NULL)
      xvt_dwin_invalidate_rect(win, NULL);
   else
   {
             /* refresh an area a bit bigger than just the object */
      widthd2 = (rctp->right - rctp->left)/2;
      heightd2 = (rctp->bottom - rctp->top)/2;
      refreshArea.top = rctp->top - 3;
      refreshArea.bottom = rctp->bottom + heightd2;
      refreshArea.left = rctp->left - 3;
      refreshArea.right = rctp->right + 3;

      xvt_dwin_invalidate_rect(win, phys_rect(win, &refreshArea));
   }
}

/*
     Function to synchronize scroll bars with position of drawing.
*/
static void
#if XVT_CC_PROTO
scroll_sync(WINDOW win)
#else
scroll_sync(win)
WINDOW win;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     RCT rct;

     xvt_vobj_get_client_rect(win, &rct);
     xvt_sbar_set_range(win, HSCROLL, 0, wip->range.h = PAGE_WIDTH - rct.right);
     xvt_sbar_set_range(win, VSCROLL, 0, wip->range.v = PAGE_HEIGHT - rct.bottom);
     xvt_sbar_set_pos(win, HSCROLL, wip->origin.h);
     xvt_sbar_set_pos(win, VSCROLL, wip->origin.v);
}


/* ###########################################################################
############################### Setup Functions ##############################
############################################################################## */

/*
     Function to create and initialize a new window's info structure.
*/
static void
#if XVT_CC_PROTO
setupWindow(WINDOW win)
#else
setupWindow(win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip;

   if (!batchExecution)
		xvt_vobj_set_title (win, HISTORY_TITLE);
   setCurrentFileName (NULL);

   wip = (WINDOW_INFO *)xvt_mem_alloc(sizeof(WINDOW_INFO));
   xvt_errmsg_sig_if(!(wip != NULL), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"108",
          108, "setupWindow: Out of memory.");
   memset((char *)wip, 0, sizeof(WINDOW_INFO));
   xvt_vobj_set_data(win, PTR_LONG(wip));
   setCurrentObject (win, (MENU_TAG) M_TOOL_POINTER);
        /* fudge factor for MS-WINDOWS positioning with diff font sizes */
   scroll_sync(win);
}

/*
** set the name of the currently loaded file
*/
void
#if XVT_CC_PROTO
setCurrentFileName (FILE_SPEC *filename)
#else
setCurrentFileName (filename)
FILE_SPEC *filename;
#endif
{
   char title[100];

   if (filename)
      memcpy (&currentFileName, filename, sizeof(FILE_SPEC));
   else
   {
      xvt_fsys_convert_str_to_dir (".", &(currentFileName.dir));
      strcpy (currentFileName.name, "");
   }

   if (strlen (currentFileName.name))
      sprintf (title, "%s - %s", HISTORY_TITLE, currentFileName.name);
   else
      sprintf (title, HISTORY_TITLE);

   if (!batchExecution)
		xvt_vobj_set_title (historyWindow, title);
}

void
#if XVT_CC_PROTO
getCurrentFileName (FILE_SPEC *filename)
#else
getCurrentFileName (filename)
FILE_SPEC *filename;
#endif
{
   memcpy (filename, &currentFileName, sizeof(FILE_SPEC));
}


/* ###########################################################################
######################### General Utility Functions ##########################
############################################################################## */

/* ======================================================================
FUNCTION  createEventWindow

DESCRIPTION
     create the event edit window
====================================================================== */
WINDOW
#if XVT_CC_PROTO
createPreviewWindow (WINDOW parentWin, WINDOW previewPlace)
#else
createPreviewWindow (parentWin, previewPlace)
WINDOW parentWin, previewPlace;
#endif
{
#define PREVIEW_UPDATE_SIZE       40
#define PREVIEW_TYPE_SIZE         130
#define PREVIEW_TYPE_OPTION_SIZE  80
   RCT listRect, previewRect;
   WINDOW listWin, previewWin = NULL_WIN;
      
   xvt_vobj_get_outer_rect (previewPlace, &previewRect);
      
   if (previewWin = (WINDOW) xvt_win_create (W_PLAIN, &previewRect, "Preview",
                               0, parentWin, WSF_NO_MENUBAR,
                               EM_ALL, PREVIEW_WINDOW_eh, (long) 0L))
   {      
      xvt_vobj_get_client_rect (previewWin, &listRect);
      listRect.top = listRect.bottom - 20;  /* place at bottom of window */
      listRect.left = listRect.right - PREVIEW_UPDATE_SIZE;
      listWin = xvt_ctl_create (WC_CHECKBOX, &listRect, "On",
                                previewWin, CTL_FLAG_CHECKED, 0L, PREVIEW_UPDATE);

      xvt_vobj_get_client_rect (previewWin, &listRect);
      listRect.top = listRect.bottom - 20;  /* place at bottom of window */
      listRect.bottom += 100;
      listRect.right = listRect.right - PREVIEW_UPDATE_SIZE;
      listRect.left = listRect.right - PREVIEW_TYPE_SIZE;
      if (listWin = xvt_ctl_create (WC_LISTBUTTON, &listRect, "Preview Type",
                                    previewWin, 0L, 0L, PREVIEW_TYPE))
      {
         xvt_list_suspend (listWin);
         xvt_list_add (listWin, 0, "Plane");
         xvt_list_add (listWin, 1, "Block");
         xvt_list_add (listWin, 2, "Gravity");
         xvt_list_add (listWin, 3, "Magnetic");
         xvt_list_add (listWin, 4, "Gravity + Map");
         xvt_list_add (listWin, 5, "Magnetic + Map");
         xvt_list_add (listWin, 6, "Gravity Diff");
         xvt_list_add (listWin, 7, "Magnetic Diff");
         xvt_list_resume (listWin);
      }

      xvt_vobj_get_client_rect (previewWin, &listRect);
      listRect.top = listRect.bottom - 20;  /* place at bottom of window */
      listRect.bottom += 100;
      listRect.right = listRect.right - PREVIEW_UPDATE_SIZE - PREVIEW_TYPE_SIZE;
      listRect.left = listRect.right - PREVIEW_TYPE_OPTION_SIZE;
      if (listWin = xvt_ctl_create (WC_LISTBUTTON, &listRect, "Preview Type Options",
                                    previewWin, CTL_FLAG_INVISIBLE, 0L, PREVIEW_TYPE_OPTIONS))
      {
         xvt_list_suspend (listWin);
         xvt_list_add (listWin, 0, "Event");
         xvt_list_add (listWin, 1, "History");
         xvt_list_add (listWin, 2, "Full");
         xvt_list_resume (listWin);
      }

      xvt_vobj_set_visible (previewWin, TRUE);

      currentPreviewWindow =  previewWin;  /* Store window globally */
   }
    
   return (previewWin);
}

/* ======================================================================
FUNCTION  createEventWindow

DESCRIPTION
     create the event edit window
====================================================================== */
WINDOW
#if XVT_CC_PROTO
createEventWindow (OBJECT *object)
#else
createEventWindow (object)
OBJECT *object;
#endif
{
   WINDOW eventWin = NULL_WIN, previewPosWin;
   int screenWidth, screenHeight;
   int winWidth, winHeight;
   WIN_DEF *windowDefinition;                                     
   RCT winRect;
	double winPositionScale;

   if (batchExecution)
		return (eventWin);

                   /* Create the window centered on the screen */
#if (XVTWS == WINWS) || (XVTWS == WIN32WS) || (XVTWS == WIN16WIN)
   winPositionScale = (double) 8.0/6.0;

   xvt_vobj_get_client_rect (TASK_WIN, &winRect);
   screenWidth = (int) (((double) (winRect.right - winRect.left))/winPositionScale);
   screenHeight= (int) (((double) (winRect.bottom - winRect.top))/winPositionScale);
#else
   screenWidth = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_WIDTH);
   screenHeight = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_HEIGHT);
#endif

   windowDefinition = xvt_res_get_win_def (EVENT_WINDOW);
   winWidth = windowDefinition[0].rct.right - windowDefinition[0].rct.left;
   winHeight = windowDefinition[0].rct.bottom - windowDefinition[0].rct.top;

   windowDefinition[0].rct.left = (screenWidth - winWidth)/2;
   windowDefinition[0].rct.right = windowDefinition[0].rct.left + winWidth;
   windowDefinition[0].rct.top = (screenHeight - winHeight)/2;
   windowDefinition[0].rct.bottom = windowDefinition[0].rct.top + winHeight;

   object->previewWin = NULL_WIN;  /* make sure no old preview windows are left */
   eventWin = (WINDOW) xvt_win_create_def (windowDefinition, TASK_WIN,
                           EM_ALL, EVENT_WINDOW_eh, (long) object);
   xvt_res_free_win_def (windowDefinition);

                         /* Add in the preview window a the correct pos */
   if (previewPosWin = xvt_win_get_ctl (eventWin, EVENT_PREVIEW))
      object->previewWin = createPreviewWindow(eventWin, previewPosWin);
   else
      object->previewWin = NULL_WIN;


                    /* Show all that we have done */
   if (eventWin && object->previewWin)
   {
      xvt_vobj_set_visible (eventWin, TRUE);
      bringWindowToFront(eventWin);
      bringWindowToFront(object->previewWin);
   }
   else
   {
      if (eventWin)
         xvt_vobj_destroy (eventWin);
      eventWin = NULL_WIN;
   }

   return (eventWin);
}

/* ======================================================================
FUNCTION  createPositionedWindow
                                                              
INPUT
OUTPUT
   RCT *posRect - returned bounding rectangle of the window created
                                                                 
DESCRIPTION
     create a resource file window and put it in a location
====================================================================== */
WINDOW
#if XVT_CC_PROTO
createPositionedWindow (int windowId, WINDOW parent, int posX, int posY,
          EVENT_MASK mask, EVENT_HANDLER eventHandler, long appData,
          RCT *posRect)
#else
createPositionedWindow (windowId, parent, posX, posY, mask,
                        eventHandler, appData, posRect)
int windowId;
WINDOW parent;
int posX, posY;
EVENT_MASK mask;
EVENT_HANDLER eventHandler;
long appData;
RCT *posRect;
#endif
{
   WINDOW popupWin = NULL_WIN;
   int winWidth, winHeight;
   WIN_DEF *windowDefinition;                                     

   windowDefinition = xvt_res_get_win_def (windowId);
   winWidth = windowDefinition[0].rct.right - windowDefinition[0].rct.left;
   winHeight = windowDefinition[0].rct.bottom - windowDefinition[0].rct.top;

   windowDefinition[0].rct.left = posX;
   windowDefinition[0].rct.right = windowDefinition[0].rct.left + winWidth;
   windowDefinition[0].rct.top = posY;
   windowDefinition[0].rct.bottom = windowDefinition[0].rct.top + winHeight;

   if (posRect)
   {
      posRect->top = windowDefinition[0].rct.top;
      posRect->bottom = windowDefinition[0].rct.bottom;
      posRect->left = windowDefinition[0].rct.left;
      posRect->right = windowDefinition[0].rct.right;
   }
                       /* give child windows a set style */
   if ((parent != TASK_WIN) && (parent != SCREEN_WIN))
      windowDefinition[0].wtype = W_NO_BORDER;
   
   popupWin = (WINDOW) xvt_win_create_def (windowDefinition,
                       parent, mask, eventHandler, appData);
   xvt_res_free_win_def (windowDefinition);

   return (popupWin);
}

/* ======================================================================
FUNCTION  createCenteredWindow

DESCRIPTION
     create a resource file window and center it on the display
====================================================================== */
WINDOW
#if XVT_CC_PROTO
createCenteredWindow (int windowId, WINDOW parent, EVENT_MASK mask,
                      EVENT_HANDLER eventHandler, long appData)
#else
createCenteredWindow (windowId, parent, mask, eventHandler, appData)
int windowId;
WINDOW parent;
EVENT_MASK mask;
EVENT_HANDLER eventHandler;
long appData;
#endif
{
   WINDOW popupWin = NULL_WIN;
   int screenWidth, screenHeight;
   int winWidth, winHeight;
   WIN_DEF *windowDefinition;                                     
   RCT winRect;
	double winPositionScale;

/*
#if (XVTWS == WINWS) || (XVTWS == WIN32WS) || (XVTWS == WIN16WIN)
   winPositionScale = (double) 1.0;//8.0/6.0;

   xvt_vobj_get_client_rect (TASK_WIN, &winRect);
   screenWidth = (int) (((double) (winRect.right - winRect.left))/winPositionScale);
   screenHeight= (int) (((double) (winRect.bottom - winRect.top))/winPositionScale);
#else
*/
   screenWidth = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_WIDTH);
   screenHeight = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_HEIGHT);
/*
#endif
*/

   windowDefinition = xvt_res_get_win_def (windowId);
   winWidth = windowDefinition[0].rct.right - windowDefinition[0].rct.left;
   winHeight = windowDefinition[0].rct.bottom - windowDefinition[0].rct.top;

   windowDefinition[0].rct.left = (screenWidth - winWidth)/10;
   windowDefinition[0].rct.right = windowDefinition[0].rct.left + winWidth;
   windowDefinition[0].rct.top = (screenHeight - winHeight)/10;
   windowDefinition[0].rct.bottom = windowDefinition[0].rct.top + winHeight;

                       /* give child windows a set style */
   if ((parent != TASK_WIN) && (parent != SCREEN_WIN))
      windowDefinition[0].wtype = W_PLAIN;
   
   popupWin = (WINDOW) xvt_win_create_def (windowDefinition, parent,
                           mask, eventHandler, appData);
   xvt_res_free_win_def (windowDefinition);

   if (popupWin)
   {
      xvt_vobj_set_visible (popupWin, TRUE);
      bringWindowToFront(popupWin);
   }

   return (popupWin);
}

/* ======================================================================
FUNCTION  createCenteredDialog

DESCRIPTION
     create a resource file dialog and center it on the display
====================================================================== */
WINDOW
#if XVT_CC_PROTO
createCenteredDialog (int windowId, EVENT_MASK mask,
                      EVENT_HANDLER eventHandler, long appData)
#else
createCenteredDialog (windowId, mask, eventHandler, appData)
int windowId;
EVENT_MASK mask;
EVENT_HANDLER eventHandler;
long appData;
#endif
{
   WINDOW popupWin = NULL_WIN;
   int screenWidth, screenHeight;
   int winWidth, winHeight;
   WIN_DEF *windowDefinition;
   RCT winRect;

#if (XVTWS == WINWS) || (XVTWS == WIN32WS) || (XVTWS == WIN16WIN)
   xvt_vobj_get_client_rect (TASK_WIN, &winRect);
   screenWidth = (int) (((double) (winRect.right - winRect.left))/2.5);
   screenHeight= (int) (((double) (winRect.bottom - winRect.top))/2.5);
#else
   screenWidth = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_WIDTH);
   screenHeight = (int) xvt_vobj_get_attr (NULL_WIN, ATTR_SCREEN_HEIGHT);
#endif

   windowDefinition = xvt_res_get_dlg_def (windowId);
   winWidth = windowDefinition[0].rct.right - windowDefinition[0].rct.left;
   winHeight = windowDefinition[0].rct.bottom - windowDefinition[0].rct.top;

   windowDefinition[0].rct.left = (screenWidth - winWidth)/2;
   windowDefinition[0].rct.right = windowDefinition[0].rct.left + winWidth;
   windowDefinition[0].rct.top = (screenHeight - winHeight)/2;
   windowDefinition[0].rct.bottom = windowDefinition[0].rct.top + winHeight;

   popupWin = (WINDOW) xvt_dlg_create_def (windowDefinition,
                           mask, eventHandler, appData);
   xvt_res_free_win_def (windowDefinition);

   if (popupWin)
   {
      xvt_vobj_set_visible (popupWin, TRUE);
      bringWindowToFront (popupWin);
   }

   return (popupWin);
}

/* ======================================================================
FUNCTION  tidyObjects 

DESCRIPTION
     collect the object into the tighest representation
====================================================================== */
void
#if XVT_CC_PROTO
tidyObjects (WINDOW win)
#else
tidyObjects (win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   register OBJECT *p;
   register int top, left;

   copyUndoPositions (wip, FALSE);

   for (p = wip->head; p != NULL; p = p->next)
   {
      left = (p->column-1) * GRID_WIDTH;
      top = (p->row-1) * GRID_HEIGHT;
      xvt_rect_set (&(p->bound), (short) left, (short) top,
                                 (short) (left + ICON_SIZE + 2),
							     (short) (top + ICON_SIZE + 2));
      gridCorrectRect (&(p->bound), &(p->bound));
   }

   log_invalidate_rect(win, NULL);
}

/* ======================================================================
FUNCTION  copyUndoPositions

DESCRIPTION
     take copy of icon positions for undo buffer
====================================================================== */
void
#if XVT_CC_PROTO
copyUndoPositions (WINDOW_INFO *wip, int preserveObjects)
#else
copyUndoPositions (wip, preserveObjects)
WINDOW_INFO *wip;
int preserveObjects;
#endif
{
   register OBJECT *p, *tempObject, *lastTempObject = (OBJECT *) NULL;
   int count = 0;

	if (batchExecution)
		return;

   if (!wip)
      wip = get_win_info(getEventDrawingWindow ());

                            /* ************************* */
                            /* free positions being kept */
   for (p = lastObjectPositions; p != NULL; p = tempObject)
   {
      tempObject = p->next;
      xvt_mem_free ((char *) p);
   }
   lastObjectPositions = (OBJECT *) NULL;

   if (preserveObjects == DELETE_UNDO_POSITONS)
      return;

                            /* **************************** */
                            /* take new copy of positioning */
   for (p = wip->head; p != NULL; p = p->next, count++)
   {
      if (!(tempObject = (OBJECT *) xvt_mem_alloc (sizeof(OBJECT))))
      {                      /* if we fail, free what is done so far */
         while (count--)
         {
            tempObject = lastObjectPositions;
            lastObjectPositions = lastObjectPositions->next;
            xvt_mem_free ((char *) lastObjectPositions);
         }
         lastObjectPositions = NULL;
         break;
      }
                      /* take a copy of the present object */
      memcpy (tempObject, p, sizeof(OBJECT));
      tempObject->next = (OBJECT *) NULL;

                      /* add this object to the list of objects */
      if (lastTempObject)
         lastTempObject->next = tempObject;

      if (!lastObjectPositions)
         lastObjectPositions = tempObject;

      lastTempObject = tempObject;
   }

   if (!preserveObjects)
   {
      copyUndoAdditions ((OBJECT *) NULL, TRUE);
      copyUndoDeletions ((OBJECT *) NULL, TRUE);
   }
}
/* ======================================================================
FUNCTION  copyUndoAdditions

DESCRIPTION
     take copy of icon insertions (Added) for undo Buffer
====================================================================== */
void
#if XVT_CC_PROTO
copyUndoAdditions (OBJECT *saveObject, int preserveObjects)
#else
copyUndoAdditions (saveObject, preserveObjects)
OBJECT *saveObject;
int preserveObjects;
#endif
{
   int optionsSize = 0;

	if (batchExecution)
		return;

                /* ************************************* */
                /* free the objects that were being kept */
   if (!saveObject)
   {
      if (lastAddedObjects)
      {
         xvt_mem_free ((char *) lastAddedObjects);
         lastAddedObjects = (OBJECT **) NULL;
         numLastAddedObjects = 0;
      }
   }
                    /* ******************************************** */
                    /* Take a copy of a pointer to the object added */
   else
   {  
      if (!lastAddedObjects)
      {
         lastAddedObjects = (OBJECT **) xvt_mem_alloc(sizeof(OBJECT*));
         numLastAddedObjects = 1;
      }
      else
      {
         numLastAddedObjects++;
         lastAddedObjects = (OBJECT **) xvt_mem_realloc(
               (char *) lastAddedObjects, sizeof(OBJECT*)*numLastAddedObjects);
      }

      if (!lastAddedObjects)
         numLastAddedObjects = 0;
      else
         lastAddedObjects[numLastAddedObjects-1] = saveObject;

      if (!preserveObjects)
         copyUndoDeletions((OBJECT *) NULL, TRUE);
   }
}
/* ======================================================================
FUNCTION  copyUndoDeletions

DESCRIPTION
     take copy of deleted object and data for undo buffer
====================================================================== */
void
#if XVT_CC_PROTO
copyUndoDeletions (OBJECT *saveObject, int preserveObjects)
#else
copyUndoDeletions (saveObject, preserveObjects)
OBJECT *saveObject;
int preserveObjects;
#endif
{
   register OBJECT *p, *tempObject;
   int optionsSize = 0;

                /* ************************************* */
                /* free the objects that were being kept */
   if (!saveObject)
   {
      for (p = lastRemovedObjects; p != NULL; p = tempObject)
      {
         tempObject = p->next;
         freeObjectMemory (p);
      }
      lastRemovedObjects = (OBJECT *) NULL;
   }
                    /* ****************************************** */
                    /* Take a copy of object before it is removed */
   else
   {  
      optionsSize = sizeofOptions (saveObject->shape);

                                          /* *********************** */
                                          /* copy the object to save */
      if (tempObject = (OBJECT *) xvt_mem_alloc (sizeof(OBJECT)))
      {
         memcpy (tempObject, saveObject, sizeof(OBJECT));

         if (optionsSize)
         {
            if (tempObject->options = xvt_mem_alloc (optionsSize))
            {
               memcpy (tempObject->options, saveObject->options, optionsSize);
            }
            else
            {
               xvt_mem_free ((char *) tempObject);
               tempObject = (OBJECT *) NULL;
            }
         }
         else
            tempObject->options = (char *) NULL;
      }

      if (tempObject)       /* add it to the list of stored objects */
      {
         tempObject->next = lastRemovedObjects;
         lastRemovedObjects = tempObject;
      }

      if (!preserveObjects)
         copyUndoAdditions((OBJECT *) NULL, TRUE);
   }
}

/* ======================================================================
FUNCTION  undoChanges 

DESCRIPTION
     undo the last change to the history window by replacing the
     current linked list of object positions with the copy of the
     the previous state.
RETURN
   TRUE - if the undo was performed sucessfully
   FALSE - otherwise
====================================================================== */
int
#if XVT_CC_PROTO
undoChanges (WINDOW win)
#else
undoChanges (win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip;
   register OBJECT *p, *tempObject, *previous;
   int count, error = TRUE;

   if (!win)
      win = getEventDrawingWindow();

   wip = (WINDOW_INFO *) get_win_info(win);

                  /* ***************************************************** */
                  /* *** Restore object positions before the last move *** */
   if (lastObjectPositions)
   {
      tempObject = wip->head;
      wip->head = lastObjectPositions;

               /* restore the origional list if Undo is called again */
      lastObjectPositions = tempObject;
      error = FALSE;
   }

                  /* ********************************************** */
                  /* *** Put back the objects that were removed *** */
   if (lastRemovedObjects)
   {
      for (p = wip->head; (p != NULL) && (p->next != NULL); p = p->next)
         ; /* find last in list */

      if (p)
         p->next = lastRemovedObjects;
      else
         wip->head = lastRemovedObjects;

            /* Store removed objects so they can be re-added with next UNDO */
      copyUndoAdditions ((OBJECT *) NULL, TRUE);
      for (p = lastRemovedObjects; (p != NULL); p = p->next)
         copyUndoAdditions (p, TRUE);

      lastRemovedObjects = (OBJECT *) NULL;

      error = FALSE;
   }
                  /* ************************************************** */
                  /* *** Remove the objects that were last added in *** */
   else if (lastAddedObjects)
   {
      copyUndoDeletions (NULL, TRUE);  /* clear undo buffer for deletions */
      for (count = 0; count < numLastAddedObjects; count++)
      {
         tempObject = lastAddedObjects[count];

                  /* remove the added ones */
         for (previous = NULL, p = wip->head; p != NULL; p = p->next)
         {
            if (p == tempObject)
            {
               if (previous)
                  previous->next = p->next;
               else
                  wip->head = p->next;

               copyUndoDeletions (p, TRUE);  /* add to deletions undo buffer */
               if (p->options)
                  xvt_mem_free (p->options);
               xvt_mem_free ((char *) p);
            }
            else
               previous = p;
         }
      }
            /* Clear Undo buffer for storing last additions */
      copyUndoAdditions ((OBJECT *) NULL, TRUE);
      error = FALSE;
   }

    /* in case we added the first or removed the last in history */
   updateMenuOptions (TASK_MENUBAR, NULL_WIN);

   if (error)
   {
      if (batchExecution)
         fprintf (stderr, "Sorry, There is Nothing to Undo.");
      else
         xvt_dm_post_error("Sorry, There is Nothing to Undo.");
   }

   reorderObjects (win);

   log_invalidate_rect(win, NULL);
   invalidateCurrentLineMap ();

   return (!error);
}

/* ======================================================================
FUNCTION  nthObject

DESCRIPTION
     get the nth object in the list or NULL if out of bounds 
====================================================================== */
OBJECT *
#if XVT_CC_PROTO
nthObject (WINDOW win, int itemCount)
#else
nthObject (win, itemCount)
WINDOW win;
int itemCount;
#endif
{
   WINDOW_INFO *wip;
   register OBJECT *p;
   register int count;

   if (win) 
      wip = get_win_info (win);
   else
      wip = get_win_info (getEventDrawingWindow ());

   if (!wip)
      return (0);

   count = 0;
   for (p = wip->head; p != NULL; p = p->next, count++)
   {
      if (count == itemCount)
         break;
   }

   return (p);
}

/* ======================================================================
FUNCTION  totalObjects

DESCRIPTION
     get the number of objects in the list
====================================================================== */
int
#if XVT_CC_PROTO
totalObjects (WINDOW win)
#else
totalObjects (win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip;
   register OBJECT *p;
   register int count;

   if (win) 
      wip = get_win_info (win);
   else
      wip = get_win_info (getEventDrawingWindow ());

   if (!wip)
      return (0);

   count = 0;
   for (p = wip->head; p != NULL; p = p->next)
   {
      count++;
   }

   return (count);
}
/* ======================================================================
FUNCTION  countObjects

DESCRIPTION
     get the number of objects in the list up to a stop
====================================================================== */
int
#if XVT_CC_PROTO
countObjects (WINDOW win)
#else
countObjects (win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip;
   register OBJECT *p;
   register int count;

   if (win) 
      wip = get_win_info (win);
   else
      wip = get_win_info (getEventDrawingWindow ());

   count = 0;
   for (p = wip->head; p != NULL; p = p->next)
   {
      if (p->shape == STOP)
         break;
      count++;
   }

   return (count);
}

/* ======================================================================
FUNCTION  sizeofOptions

DESCRIPTION
     return the size of the options structure for that event
====================================================================== */
int
#if XVT_CC_PROTO
sizeofOptions (OBJECTS object)
#else
sizeofOptions (object)
OBJECTS object;
#endif
{
   int optionsSize;

   switch (object)
   {
      case STRATIGRAPHY:
         optionsSize = sizeof (STRATIGRAPHY_OPTIONS); break;
      case FOLD:
         optionsSize = sizeof (FOLD_OPTIONS); break;
      case FAULT:
         optionsSize = sizeof (FAULT_OPTIONS); break;
      case UNCONFORMITY:
         optionsSize = sizeof (UNCONFORMITY_OPTIONS); break;
      case SHEAR_ZONE:
         optionsSize = sizeof (SHEAR_OPTIONS); break;
      case DYKE:
         optionsSize = sizeof (DYKE_OPTIONS); break;
      case PLUG:
         optionsSize = sizeof (PLUG_OPTIONS); break;
      case STRAIN:
         optionsSize = sizeof (STRAIN_OPTIONS); break;
      case TILT:
         optionsSize = sizeof (TILT_OPTIONS); break;
      case FOLIATION:
         optionsSize = sizeof (FOLIATION_OPTIONS); break;
      case LINEATION:
         optionsSize = sizeof (LINEATION_OPTIONS); break;
      case IMPORT:
         optionsSize = sizeof (IMPORT_OPTIONS); break;
      case GENERIC:
         optionsSize = sizeof (GENERIC_OPTIONS); break;
      default:
         optionsSize = 0;
   }
      
   return (optionsSize);
}

/* ======================================================================
FUNCTION  getEventDrawingWindow

DESCRIPTION
     return the window that the events are drawn in, this is needed
        to retrieve the linked list of structures that describes the
        events
====================================================================== */
WINDOW
#if XVT_CC_PROTO
getEventDrawingWindow ()
#else
getEventDrawingWindow ()
#endif
{
   return ((WINDOW) historyWindow);
}

/* ======================================================================
FUNCTION  getFloatingMenuWindow

DESCRIPTION
     return the window that has the Floating menu in
====================================================================== */
WINDOW
#if XVT_CC_PROTO
getFloatingMenuWindow ()
#else
getFloatingMenuWindow ()
#endif
{
   return ((WINDOW) menuWindow);
}

/* ======================================================================
FUNCTION  gridCorrectRect

DESCRIPTION
     make sure the rectangle lays on an invisable grid
     (or atleast the closest position to it)

====================================================================== */
void
#if XVT_CC_PROTO
gridCorrectRect (RCT *grid_rctp, RCT *rctp)
#else
gridCorrectRect (grid_rctp, rctp)
RCT *grid_rctp;
RCT *rctp;
#endif
{
   int top, bottom, left, right;
   int difference;

   normalizeRect (grid_rctp, rctp);
   top    = grid_rctp->top;
   bottom = grid_rctp->bottom;
   left   = grid_rctp->left;
   right  = grid_rctp->right;

   if (left < GRID_WIDTH/2)   /* cant have things in the border */
   {
      difference = GRID_WIDTH/2 - left;
      left += difference;
      right += difference;
   }
   if (top < GRID_HEIGHT/2)   /* cant have things in the border */
   {
      difference = GRID_HEIGHT/2 - top;
      top += difference;
      bottom += difference;
   }

                 /* snap the box to the closest grid corner */
   if (difference = (left+GRID_WIDTH/2)%GRID_WIDTH)
   {
      if (difference < (GRID_WIDTH/2))
      {
         left = left - difference;
         right = right - difference;
      }
      else
      {
         left = left + (GRID_WIDTH - difference);
         right = right + (GRID_WIDTH - difference);
      }   
   }

   if (difference = (top+GRID_HEIGHT/2)%GRID_HEIGHT)
   {
      if (difference < (GRID_HEIGHT/2))
      {
         top = top - difference;
         bottom = bottom - difference;
      }
      else
      {
         top = top + (GRID_HEIGHT - difference);
         bottom = bottom + (GRID_HEIGHT - difference);
      }   
   }

   xvt_rect_set(grid_rctp, (short) left, (short) top , (short) right, (short) bottom);
}

/* ======================================================================
FUNCTION  normalizeRect

DESCRIPTION
     Function to transform a rectangle so that the point <left, top>
   is above and to the left of <right, bottom>.

====================================================================== */
static void
#if XVT_CC_PROTO
normalizeRect (RCT *norm_rctp, RCT *rctp)
#else
normalizeRect (norm_rctp, rctp)
RCT *norm_rctp;
RCT *rctp;
#endif
{
     xvt_rect_set(norm_rctp, (short) MIN(rctp->left, rctp->right),
                             (short) MIN(rctp->top, rctp->bottom),
							 (short) MAX(rctp->left, rctp->right),
                             (short) MAX(rctp->top, rctp->bottom));
}

/* ======================================================================
FUNCTION  yToRow

DESCRIPTION

====================================================================== */
static void
#if XVT_CC_PROTO
yToRow (WINDOW win, int yPos, int *row)
#else
yToRow (win, yPos, row)
WINDOW win;
int yPos, *row;
#endif
{
}

/* ======================================================================
FUNCTION  columnToX

DESCRIPTION
     change a column address in to a X pixel locations that span that
        row.

====================================================================== */
static void
#if XVT_CC_PROTO
xToColumn (WINDOW win, int xPos, int *column)
#else
xToColumn (win, xPos, column)
WINDOW win;
int xPos, *column;
#endif
{
}



/* ======================================================================
FUNCTION  rowToY

DESCRIPTION
     change a row address in to a Y pixel locations that span that
        row.

====================================================================== */
static void
#if XVT_CC_PROTO
rowToY (WINDOW win, int row, int *startY, int *endY)
#else
rowToY (win, row, startY, endY)
WINDOW win;
int row, *startY, *endY;
#endif
{
}

/* ======================================================================
FUNCTION  columnToX

DESCRIPTION
     change a column address in to a X pixel locations that span that
        row.

====================================================================== */
static void
#if XVT_CC_PROTO
columnToX (WINDOW win, int row, int *startX, int *endX)
#else
columnToX (win, row, startX, endX)
WINDOW win;
int row, *startX, *endX;
#endif
{
}


/*
     Function to find an object that contains a given point. Selected objects
     are given preference over deselected ones.
*/
static OBJECT *
#if XVT_CC_PROTO
findObject(WINDOW win, PNT pnt)
#else
findObject (win, pnt)
WINDOW win;
PNT pnt;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     register OBJECT *p;

     for (p = wip->head; p != NULL; p = p->next)
          if (p->selected && xvt_rect_has_point(&p->bound, pnt))
               return(p);
     for (p = wip->head; p != NULL; p = p->next)
          if (!p->selected && xvt_rect_has_point(&p->bound, pnt))
               return(p);
     return(NULL);
}

/*
     Function to convert a logical rectangle to a physical one.
*/
static RCT *
#if XVT_CC_PROTO
phys_rect(WINDOW win, RCT *rctp)
#else
phys_rect(win, rctp)
WINDOW win;
RCT *rctp;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     static RCT rct;

     rct.left = X_LOG_TO_PHYS(wip, rctp->left);
     rct.top = Y_LOG_TO_PHYS(wip, rctp->top);
     rct.right = X_LOG_TO_PHYS(wip, rctp->right);
     rct.bottom = Y_LOG_TO_PHYS(wip, rctp->bottom);
     return(&rct);
}

/*
     Function to move an object.
*/
static int
#if XVT_CC_PROTO
objMove(OBJECT *p, int dx, int dy)
#else
objMove(p, dx, dy)
register OBJECT *p;
int dx;
int dy;
#endif
{
   RCT newBound;


   newBound.left = p->bound.left;
   newBound.right = p->bound.right;
   newBound.top = p->bound.top;
   newBound.bottom = p->bound.bottom;

   xvt_rect_offset(&newBound, (short) dx, (short) dy);
   gridCorrectRect (&newBound, &newBound);

   if (checkOverlap (historyWindow, newBound, p))
      return (FALSE);
   if (!validateStructure (historyWindow, newBound, p->shape, FALSE))
      return (FALSE);

              /* repaint the position it came from */
   log_invalidate_rect(historyWindow, &p->bound);

   p->bound.left = newBound.left;
   p->bound.right = newBound.right;
   p->bound.top = newBound.top;
   p->bound.bottom = newBound.bottom;

   setObjectBound (p, &p->bound);

              /* repaint the position it is placed in */
   log_invalidate_rect(historyWindow, &p->bound);

   return (TRUE);
}

/*
     Function to move all selected objects.
*/
static int
#if XVT_CC_PROTO
objMoveSelected(WINDOW win, int dx, int dy)
#else
objMoveSelected(win, dx, dy)
WINDOW win;
int dx;
int dy;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   register OBJECT *p;
   int result;

   for (p = wip->head; p != NULL; p = p->next)
   {
      if (p->selected)
      {
         result = objMove(p, dx, dy);
      }
   }

   return (result);
}


/* ###########################################################################
######################### Graphic Utility Functions ##########################
############################################################################## */

/* ======================================================================
FUNCTION  createObject

DESCRIPTION
   create the current object in the rectangle rctp

====================================================================== */
static void
#if XVT_CC_PROTO
createObject(WINDOW win, RCT position)
#else
createObject(win, position)
WINDOW win;
RCT position;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   int width, height;

   width = position.right - position.left;
   height = position.bottom - position.top;
   if ((width < MIN_SIZE) || (height < MIN_SIZE)) /* too small */
      return;

   if (checkOverlap (win, position, NULL))
      return;
   if (!validateStructure (win, position, (OBJECTS) (wip->tool - 1), TRUE))
      return;

   switch (wip->tool)
   {
      case (TL_STRATIGRAPHY):
         addObject (win, STRATIGRAPHY, position, "Strat", FALSE);
         break;
      case (TL_FOLD):
         addObject (win, FOLD, position, "Fold", FALSE);
         break;
      case (TL_FAULT):
         addObject (win, FAULT, position, "Fault", FALSE);
         break;
      case (TL_UNCONFORMITY):
         addObject (win, UNCONFORMITY, position, "U/C", FALSE);
         break;
      case (TL_SHEAR):
         addObject (win, SHEAR_ZONE, position, "Shear Zone", FALSE);
         break;
      case (TL_DYKE):
         addObject (win, DYKE, position, "Dyke", FALSE);
         break;
      case (TL_PLUG):
         addObject (win, PLUG, position, "Plug", FALSE);
         break;
      case (TL_STRAIN):
         addObject (win, STRAIN, position, "Strain", FALSE);
         break;
      case (TL_TILT):
         addObject (win, TILT, position, "Tilt", FALSE);
         break;
      case (TL_FOLIATION):
         addObject (win, FOLIATION, position, "Foliation", FALSE);
         break;
      case (TL_LINEATION):
         addObject (win, LINEATION , position, "Lineation", FALSE);
         break;
      case (TL_IMPORT):
         addObject (win, IMPORT , position, "Import", FALSE);
         break;
      case (TL_GENERIC):
         addObject (win, GENERIC , position, "Generic", FALSE);
         break;
      case (TL_STOP):
         addObject (win, STOP , position, "", FALSE);
         break;
   }

   log_invalidate_rect(win, NULL);
   drawObjectLinks (win);

}

/* ======================================================================
FUNCTION  moveObject

DESCRIPTION
     Function to move an object.

====================================================================== */
static BOOLEAN
#if XVT_CC_PROTO
moveObject (WINDOW win, OBJECT *p, PNT p1, PNT p2, int pasteAction)
#else
moveObject (win, p, p1, p2, pasteAction)
WINDOW win;
OBJECT *p;
PNT p1, p2;
int pasteAction;
#endif
{
   int dx, dy;
   int result;
   OBJECT *object1, *object2;

   dx = p2.h - p1.h;
   dy = p2.v - p1.v;

   if (abs(dx) < MIN_MOVE && abs(dy) < MIN_MOVE)
      return(FALSE); /* negligible move; must have been click to select */

              /* don't store the positions of icons if we are pasteing */
    if (!pasteAction)
       copyUndoPositions((WINDOW_INFO *) NULL, FALSE);


   if (pointBetweenObjects (win, p2, &object1, &object2))
   {
      makeRoomBetweenObjects (win, &object1, &object2, &p, p->selected,
                                  &dx, &dy);
   }

     /* if the object being moved is selected then move the whole
        ** selection */
   if (p->selected)
      result = objMoveSelected(win, dx, dy);
   else
      result = objMove(p, dx, dy);

   reorderObjects (win);

   log_invalidate_rect(win, NULL);
   drawObjectLinks (win);

   invalidateCurrentLineMap ();

   return (result);
}

/* ======================================================================
FUNCTION  drawArrowBetween

DESCRIPTION
     Function to draw an arrow going from 1 object to another

====================================================================== */
static void
#if XVT_CC_PROTO
drawArrowBetween (WINDOW win, OBJECT *object1, OBJECT *object2)
#else
drawArrowBetween (win, object1, object2)
WINDOW win;
OBJECT *object1;
OBJECT *object2;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   PNT start, end;
   char word1[40], word2[40];
   int halfHeight, halfWidth;
   int ascent, decent;
   
   if (!object1 || !object2)
      return;
      
                  /* vertical attachment so just used a vertical line */
   if (object1->bound.left == object2->bound.left)
   {
      xvt_dwin_get_font_metrics(win, NULL, &ascent, &decent);
      strcpy (word1, ""); strcpy (word2, "");
      sscanf (object1->text, "%s %s",word1, word2);
      
      halfWidth = (object1->bound.right - object1->bound.left)/2;

      start.v = object1->bound.bottom + ascent + decent + 2;
      start.h = object1->bound.left + halfWidth;
      end.v = object2->bound.top - 2;
      end.h = object2->bound.left + halfWidth;

      if (strlen (word2))  /* if there is a 2nd word leave room for it */
         start.v += (ascent + decent);

      start.h = X_LOG_TO_PHYS(wip, start.h);
      start.v = Y_LOG_TO_PHYS(wip, start.v);
      end.h = X_LOG_TO_PHYS(wip, end.h);
      end.v = Y_LOG_TO_PHYS(wip, end.v);

      if (((start.v > 0) && (start.h > 0)) || ((end.v > 0) && (end.h > 0)))
      {
         xvt_dwin_draw_set_pos (win, start);
         xvt_dwin_draw_line (win, end);
      }
   }
   else  /* horizontal attachment so use an arrow head */
   {
      halfHeight = (object1->bound.bottom - object1->bound.top)/2;

      start.h = object1->bound.right + 2;
      start.v = object1->bound.top + halfHeight;
      end.h = object2->bound.left - 2;
      end.v = object2->bound.top + halfHeight;

      start.h = X_LOG_TO_PHYS(wip, start.h);
      start.v = Y_LOG_TO_PHYS(wip, start.v);
      end.h = X_LOG_TO_PHYS(wip, end.h);
      end.v = Y_LOG_TO_PHYS(wip, end.v);

      if (((start.v > 0) && (start.h > 0)) || ((end.v > 0) && (end.h > 0)))
      {
         xvt_dwin_draw_set_pos (win, start);
         xvt_dwin_draw_aline (win, end, FALSE, TRUE);
      }
   }
}

/* ======================================================================
FUNCTION  drawObjectLinks

DESCRIPTION
     Function to draw an arrow going from through all the 
   displayed objects

====================================================================== */
static void
#if XVT_CC_PROTO
drawObjectLinks (WINDOW win)
#else
drawObjectLinks (win)
WINDOW win;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   register OBJECT *p, *next;
   DRAW_CTOOLS t;
   int allDone = FALSE;
     
   xvt_app_get_default_ctools(&t);
   t.pen.width = 2;
   t.pen.pat = PAT_SOLID;
   t.pen.color = COLOR_BLACK;
   xvt_dwin_set_draw_ctools(win, &t);

                /* draw in the horizontal connections first */
   for (p = wip->head; (p != NULL) && (p->next != NULL); )
   {
      next = p->next;
      
      if (!next->drawEvent)     /* skip items not really in history */
         if (!(next = next->next))
            break;
      
                       /* skip over the ones in columns */
      while (p->bound.left == next->bound.left)
      {
         if (next->next)
            next = next->next;
         else
         {
            allDone = TRUE;
            break;
         }
      }

      if (allDone)
         break;

      drawArrowBetween (win, p, next);

      p = next;
   }

                /* now draw in the vertical connections */
   for (p = wip->head; (p != NULL) && (p->next != NULL); p = p->next)
   {
      if (p->bound.left == p->next->bound.left)
         drawArrowBetween (win, p, p->next);
   }

}

/* ======================================================================
FUNCTION  rubberRect

DESCRIPTION
     Function to draw a rubber-band rectangle.

====================================================================== */
static void
#if XVT_CC_PROTO
rubberRect (WINDOW win, RCT *rctp)
#else
rubberRect (win, rctp)
WINDOW win;
RCT *rctp;
#endif
{
     RCT rct;
     DRAW_CTOOLS t;

     xvt_app_get_default_ctools(&t);
     t.pen.pat = PAT_RUBBER;
     t.brush.pat = PAT_HOLLOW;
     t.mode = M_XOR;
     xvt_dwin_set_draw_ctools(win, &t);
     normalizeRect (&rct, rctp);
     log_draw_rect(win, &rct);
}

/*
     Function to shift the view after scrolling.
*/
static void
#if XVT_CC_PROTO
shift_view(WINDOW win, int dx, int dy, PNT *pntp)
#else
shift_view(win, dx, dy, pntp)
WINDOW win;
int dx, dy;
PNT *pntp;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     RCT rct;

     dx = (dx >= 0 ? 1 : -1) * ((abs(dx) + 7) / 8) * 8; /* force aligned pats */
     dy = (dy >= 0 ? 1 : -1) * ((abs(dy) + 7) / 8) * 8;
     xvt_dwin_update(win);
     if (dx != 0) {
          if (dx > 0)
               dx = MIN(dx, wip->origin.h);
          else
               dx = MAX(dx, wip->origin.h - wip->range.h);
          wip->origin.h -= dx;
          xvt_sbar_set_pos(win, HSCROLL, wip->origin.h);
     }
     if (dy != 0) {
          if (dy > 0)
               dy = MIN(dy, wip->origin.v);
          else
               dy = MAX(dy, wip->origin.v - wip->range.v);
          wip->origin.v -= dy;
          xvt_sbar_set_pos(win, VSCROLL, wip->origin.v);
     }
     xvt_vobj_get_client_rect(win, &rct);
     xvt_dwin_scroll_rect(win, &rct, dx, dy);
     if (pntp != NULL) {
          pntp->h += dx;
          pntp->v += dy;
     }
}

/*
     Function to scroll automatically when the mouse goes outside the window's
     client area.
*/
static void
#if XVT_CC_PROTO
autoscroll(WINDOW win, EVENT *ep)
#else
autoscroll(win, ep)
WINDOW win;
EVENT *ep;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     RCT rct;
     PNT phys_where;
     int dx = 0, dy = 0;

     phys_where.h = X_LOG_TO_PHYS(wip, ep->v.mouse.where.h);
     phys_where.v = Y_LOG_TO_PHYS(wip, ep->v.mouse.where.v);
     xvt_vobj_get_client_rect(win, &rct);
     if (phys_where.h > rct.right)
          dx = rct.right - phys_where.h; /* dx negative */
     else if (phys_where.h < rct.left) {
          dx = rct.left - phys_where.h; /* dx positive */
          if (wip->origin.h < dx)
               dx = 0; /* cant move left anymore */
     }
     if (phys_where.v > rct.bottom)
          dy = rct.bottom - phys_where.v; /* dy negative */
     else if (phys_where.v < rct.top) {
          dy = rct.top - phys_where.v; /* dy positive */
          if (wip->origin.v < dy)
               dy = 0; /* cant move up anymore */
     }
     if (dx != 0 || dy != 0) {
          shift_view(win, dx, dy, &ep->v.mouse.where);
     }
}

/* ###########################################################################
######################### Graphic Support Functions ##########################
############################################################################## */
/*
     Function to put up the attributes dialog box.
*/
void
#if XVT_CC_PROTO
editObjectParams (OBJECT *p)
#else
editObjectParams (p)
register OBJECT *p;
#endif
{
   WINDOW popupWin;

   if (p->shape != STOP)
      popupWin = createEventWindow (p);

               /* needed in case it is first object */
   updateMenuOptions (TASK_MENUBAR, popupWin);
}

/*
     Function to update a window.
*/
/* ======================================================================
FUNCTION  updateDrawingWindow

DESCRIPTION
     Function to set a window's drawing tool and set the Tool menu check
     marks. If the "cmd" argument is zero only the check marks are set.

RETURNED
====================================================================== */
static void
#if XVT_CC_PROTO
updateDrawingWindow (WINDOW win)
#else
updateDrawingWindow (win)
WINDOW win;
#endif
{
   if (batchExecution || !win)
      return;

	xvt_dwin_clear(win, COLOR_WHITE);

   obj_update(win);
}

/* ======================================================================
FUNCTION  updateFloatingMenu

DESCRIPTION
     draw all the menu options into the floating menu

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
updateFloatingMenu (WINDOW menuWindow, OBJECT *firstObject, TOOL tool)
#else
updateFloatingMenu (menuWindow, firstObject, tool)
WINDOW menuWindow;
OBJECT *firstObject;
TOOL tool;
#endif
{
   DRAW_CTOOLS t;
   RCT iconPosition;
   int item;
   BOOLEAN stratigraphyPresent = FALSE;
   int iconSpacing = ICON_SIZE + 2;
   RCT winSize;
     
   if (batchExecution)
      return;

   if (!menuWindow) return;

   xvt_vobj_get_client_rect (menuWindow, &winSize);
   xvt_app_get_default_ctools(&t);
   t.pen.pat = PAT_SOLID;
   t.opaque_text = TRUE;
   t.back_color = COLOR_WHITE;
   t.fore_color = COLOR_BLACK;
   xvt_dwin_set_draw_ctools(menuWindow, &t);

   if (firstObject && (firstObject->shape == STRATIGRAPHY))
      stratigraphyPresent = TRUE; /* if stratigraphy is in the list it
                                  ** cannot be selected from menu again */
   else
      stratigraphyPresent = FALSE;

   xvt_dwin_clear(menuWindow, COLOR_WHITE);

   iconPosition.left = iconPosition.top = 0;
   iconPosition.right = iconPosition.bottom = iconSpacing;
   for (item = 0; item < NUM_MENU_OPTIONS; item++)
   {
                 /* only allow first two to be selected if no Strat */
      if (stratigraphyPresent)
      {
         if (item == 1)  /* Cant have another Strat */
         {
            xvt_dwin_set_fore_color (menuWindow, COLOR_LTGRAY);
            drawIcon (menuWindow, &iconPosition, menuItems[item]);
            menuItemPositions[item].top = 0;
            menuItemPositions[item].left = 0;
            menuItemPositions[item].right = 0;
            menuItemPositions[item].bottom = 0;
         }
         else
         {
            xvt_dwin_set_fore_color (menuWindow, COLOR_BLACK);
            drawIcon (menuWindow, &iconPosition, menuItems[item]);
            menuItemPositions[item].top = iconPosition.top;
            menuItemPositions[item].left = iconPosition.left;
            menuItemPositions[item].right = iconPosition.right;
            menuItemPositions[item].bottom = iconPosition.bottom;
         }
      }
      else
      {
         if (item >= 2)  /* only Pointer and Strat Valid */
         {
            xvt_dwin_set_fore_color (menuWindow, COLOR_LTGRAY);
            drawIcon (menuWindow, &iconPosition, menuItems[item]);
            menuItemPositions[item].top = 0;
            menuItemPositions[item].left = 0;
            menuItemPositions[item].right = 0;
            menuItemPositions[item].bottom = 0;
         }
         else
         {
            xvt_dwin_set_fore_color (menuWindow, COLOR_BLACK);
            drawIcon (menuWindow, &iconPosition, menuItems[item]);
            menuItemPositions[item].top = iconPosition.top;
            menuItemPositions[item].left = iconPosition.left;
            menuItemPositions[item].right = iconPosition.right;
            menuItemPositions[item].bottom = iconPosition.bottom;
         }
      }
                 /* only highlight if the item was a valid option */
      if (tool == (TOOL) item)
      {
         xvt_app_get_default_ctools(&t);
         t.mode = M_XOR;
         t.pen.width = 1;
         t.pen.pat = PAT_HOLLOW;
         t.brush.pat = PAT_SOLID;
         t.brush.color = COLOR_BLACK,
         xvt_dwin_set_draw_ctools(menuWindow, &t);
         xvt_dwin_draw_rect (menuWindow, &iconPosition);

         t.mode = M_COPY;    /* Restore the brush */
         xvt_dwin_set_draw_ctools(menuWindow, &t);
      }

                                   /* the position of the next Icon */
      iconPosition.top += iconSpacing;
      iconPosition.bottom += iconSpacing;                             
      if (iconPosition.bottom > winSize.bottom) /* at bottom make new row */
      {
         iconPosition.left += iconSpacing;
         iconPosition.right += iconSpacing;
         iconPosition.top = 0;
         iconPosition.bottom = iconSpacing;
      }
   }
}

/* ======================================================================
FUNCTION  setFloatingMenuShape

DESCRIPTION
     set the shape of the Floating menu in Rows / Cols on icons

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
setFloatingMenuShape (WINDOW win, int rows, int cols)
#else
setFloatingMenuShape (win, rows, cols)
WINDOW win;
int rows, cols;
#endif
{
   RCT winPosition;
                 
   if (!win)
      win = menuWindow;
   
   xvt_vobj_get_client_rect (win, &winPosition);
                                                                         /* its across */
                           /* Calc new Menu Size */
   winPosition.bottom = winPosition.top + rows*(ICON_SIZE+2) + 2;
   winPosition.right = winPosition.left + cols*(ICON_SIZE+2) + 2;
   
   CORRECT_WIN_RESIZE(win, winPosition)

   xvt_vobj_move (win, &winPosition);

   return (TRUE);
}

/* ======================================================================
FUNCTION  getFloatingMenuShape

DESCRIPTION
     get the shape of the Floating menu in Rows / Cols on icons

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
getFloatingMenuShape (WINDOW win, int *rows, int *cols)
#else
getFloatingMenuShape (win, rows, cols)
WINDOW win;
int *rows, *cols;
#endif
{
   RCT winPosition;

	if (batchExecution)
	{
		if (rows)
			*rows = 5;
		if (cols)
			*cols = 5;
	   return ((int) NUM_MENU_OPTIONS);
	}
                 
   if (!win)
      win = menuWindow;
   
   xvt_vobj_get_client_rect (win, &winPosition);
                           /* Calc current Menu Size */
   if (rows)
      *rows = (winPosition.bottom - winPosition.top) / ICON_SIZE;
   if (cols)
      *cols = (winPosition.right - winPosition.left) / ICON_SIZE;
   
   return ((int) NUM_MENU_OPTIONS);
}

/* ======================================================================
FUNCTION  setCurrentObject

DESCRIPTION
     Function to set a window's drawing tool and set the Tool menu check
     marks. If the "cmd" argument is zero only the check marks are set.

RETURNED
====================================================================== */
static void
#if XVT_CC_PROTO
setCurrentObject (WINDOW win, MENU_TAG cmd)
#else
setCurrentObject (win, cmd)
WINDOW win;
MENU_TAG cmd;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);

   switch (cmd)
   {
     case M_TOOL_POINTER:
          wip->tool = TL_POINTER;
          break;
     case M_HISTORY_STRATIGRAPHY:
          wip->tool = TL_STRATIGRAPHY;
          break;
     case M_HISTORY_FOLD:
          wip->tool = TL_FOLD;
          break;
     case M_HISTORY_FAULT:
          wip->tool = TL_FAULT;
          break;
     case M_HISTORY_UNCONFORMITY:
          wip->tool = TL_UNCONFORMITY;
          break;
     case M_HISTORY_SHEAR:
          wip->tool = TL_SHEAR;
          break;
     case M_HISTORY_DYKE:
          wip->tool = TL_DYKE;
          break;
     case M_HISTORY_PLUG:
          wip->tool = TL_PLUG;
          break;
     case M_HISTORY_STRAIN:
          wip->tool = TL_STRAIN;
          break;
     case M_HISTORY_TILT:
          wip->tool = TL_TILT;
          break;
     case M_HISTORY_FOLIATION:
          wip->tool = TL_FOLIATION;
          break;
     case M_HISTORY_LINEATION:
          wip->tool = TL_LINEATION;
          break;
     case M_HISTORY_IMPORT:
          wip->tool = TL_IMPORT;
          break;
     case M_HISTORY_GENERIC:
          wip->tool = TL_GENERIC;
          break;
     case M_HISTORY_STOP:
          wip->tool = TL_STOP;
          break;
   }
   xvt_win_set_cursor(win, (CURSOR) (wip->tool == TL_POINTER ? CURSOR_ARROW : CURSOR_CROSS));

   if (cmd == M_TOOL_POINTER)
      creatingObject = FALSE;
   else
      creatingObject = TRUE;

   updateFloatingMenu (menuWindow, wip->head, wip->tool);
}




/*
     Function to draw a rectangle in logical coordinates.
*/
static void
#if XVT_CC_PROTO
log_draw_rect(WINDOW win, RCT *rctp)
#else
log_draw_rect(win, rctp)
WINDOW win;
RCT *rctp;
#endif
{
     xvt_dwin_draw_rect(win, phys_rect(win, rctp));
}


/*
     Function to draw a rounded rectangle in logical coordinates.
*/
static void
#if XVT_CC_PROTO
log_draw_roundrect(WINDOW win, RCT *rctp, int oval_width, int oval_height)
#else
log_draw_roundrect(win, rctp, oval_width, oval_height)
WINDOW win;
RCT *rctp;
int oval_width;
int oval_height;
#endif
{
     xvt_dwin_draw_roundrect(win, phys_rect(win, rctp), oval_width, oval_height);
}

/*
     Function to draw text in logical coordinates.
*/
static void
#if XVT_CC_PROTO
drawCenteredText(WINDOW win, int x, int y, char *s, int len)
#else
drawCenteredText(win, x, y, s, len)
WINDOW win;
int x;
int y;
char *s;
int len;
#endif
{
   RCT rct, *rctp;
   int textWidth;

   textWidth = xvt_dwin_get_text_width (win, s, len);
   rct.left = rct.right = x - (textWidth/2);
   rct.top = rct.bottom = y;
   rctp = phys_rect(win, &rct);
   xvt_dwin_draw_text(win, rctp->left, rctp->top, s, len);
}

/* ======================================================================
FUNCTION  drawIcon_log

DESCRIPTION
     draw a given icon at a given position 

RETURNED
====================================================================== */
static void
#if XVT_CC_PROTO
drawIcon_log (WINDOW win, RCT *rctp, int theIcon)
#else
drawIcon_log (win, rctp, theIcon)
WINDOW win;
RCT *rctp;
int theIcon;
#endif
{    
   drawIcon (win, phys_rect(win, rctp), theIcon);
}
/* ======================================================================
FUNCTION  drawIcon

DESCRIPTION
     draw a given icon at a given position 

RETURNED
====================================================================== */
static void
#if XVT_CC_PROTO
drawIcon (WINDOW win, RCT *rctp, int theIcon)
#else
drawIcon (win, rctp, theIcon)
WINDOW win;
RCT *rctp;
int theIcon;
#endif
{    
   RCT trect;
     
   trect=*rctp;
   xvt_dwin_draw_icon(win, trect.left + 1, trect.top + 1, theIcon);
}

/* ###########################################################################
################################ Event Functions #############################
############################################################################## */

/* ======================================================================
FUNCTION  doMouse

DESCRIPTION
   Main function to process mouse events for creating, or selecting
   objects.

RETURNED
====================================================================== */
BOOLEAN
#if XVT_CC_PROTO
doMouse(WINDOW win, EVENT *ep)
#else
doMouse(win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   WINDOW_INFO *wip = get_win_info(win);
   static BOOLEAN deleteIfInvalid = FALSE;
   static PNT last_pnt, down_pnt;
   static enum {DR_MOVE, DR_SIZE, DR_OUTLINE, DR_NONE, DR_CREATE} drag_type = DR_NONE;
   static OBJECT *drag_objp;
   static BOOLEAN first_move;
   static RCT outline;
   OBJECT *object1, *object2;
   RCT objectPos;
   PNT point;


   point.h = X_PHYS_TO_LOG(wip, ep->v.mouse.where.h);
   point.v = Y_PHYS_TO_LOG(wip, ep->v.mouse.where.v);
   switch (ep->type)
   {
     case E_MOUSE_DBL:
        if ((drag_objp = findObject (win, point)) != NULL)
        {
           editObjectParams (drag_objp);
           return(TRUE);
        }
		break;
/*
**        else if (ep->v.mouse.control)
**        {
**           createLicence (0);
**        }
*/
     case E_MOUSE_DOWN:
                                  /* already moving from a paste event so */
        if (drag_type == DR_MOVE) /* ignore this button down. */
        {                         /* NB signifies a paste event is happening */
           deleteIfInvalid = TRUE;
           return(FALSE);
        }
                    /* if we are in pointer mode then make sure we are
                    ** pointing to an object or an object resize corner */
        if ((wip->tool == TL_POINTER) &&
            (drag_objp = findObject (win, point)) != NULL)
        {
           drag_type = DR_MOVE;

           if (!drag_objp->selected)
              outline = drag_objp->bound;
           else
              outline = *obj_bound_selected(win);
        }
        else if (drag_type != DR_CREATE)
        {
                    /* we are about to draw the outline as we move the mouse */
           drag_type = DR_OUTLINE;
           outline.left = outline.right = point.h;
           outline.top = outline.bottom = point.v;
        }

                         /* take note of the area being selected */
        down_pnt = last_pnt = point;
        first_move = TRUE;

        xvt_win_trap_pointer(win);
        break;
     case E_MOUSE_MOVE:
		if (creatingObject && (drag_type != DR_CREATE))
        {
           drag_type = DR_CREATE;
           first_move = TRUE;
           xvt_win_trap_pointer(win);
        }
                /* make sure it is turned off if the pointer is re selected */
        if ((drag_type == DR_CREATE) && (!creatingObject))
        {
           rubberRect (win, &outline);  /* erase the rectangle */
           drag_type = DR_NONE;
           xvt_win_release_pointer();
        }

        if (drag_type == DR_NONE)
           return(FALSE);
                    /* dont draw an outline if we are
                    ** creating an object */
        if ((wip->tool != TL_POINTER) && (drag_type == DR_OUTLINE))
            return (TRUE);

        if ((last_pnt.h == point.h) &&
            (last_pnt.v == point.v))
           return(TRUE); /* didn't really move */

        if (!first_move)
           rubberRect (win, &outline);

        first_move = FALSE;

        autoscroll(win, ep);

        if (drag_type == DR_CREATE)
        {
           outline.top = point.v - (ICON_SIZE/2);
           outline.left = point.h - (ICON_SIZE/2);
           outline.right = point.h + (ICON_SIZE/2);
           outline.bottom = point.v + (ICON_SIZE/2);
        }
        else if (drag_type == DR_MOVE)
           xvt_rect_offset(&outline, (short) (point.h - last_pnt.h),
                                     (short) (point.v - last_pnt.v));
        else
        {
           outline.right = point.h;
           outline.bottom = point.v;
        }
        last_pnt = point;
        rubberRect (win, &outline);
        break;
     case E_MOUSE_UP:
        xvt_win_release_pointer();

        if (drag_type == DR_NONE)
           return(FALSE);

               /* update outline to where the area was selected */
        if ((wip->tool == TL_POINTER) && (!first_move))
           rubberRect (win, &outline);

        if (drag_type == DR_CREATE)
        {
           if (wip->tool != TL_POINTER)
           {
              rubberRect (win, &outline);  /* erase the rectangle */
                          /* create the current Object in the middle of
                          ** where the pointer finished up */
              objectPos.top = point.v - (ICON_SIZE/2) - 1;
              objectPos.left = point.h - (ICON_SIZE/2) - 1;
              objectPos.right = point.h + (ICON_SIZE/2) + 1;
              objectPos.bottom = point.v + (ICON_SIZE/2) + 1;

              copyUndoPositions (wip, FALSE);

              if (pointBetweenObjects (win, point,
                                                  &object1, &object2))
              {
                 makeRoomBetweenObjects (win, &object1, &object2,
                                         NULL, FALSE, NULL, NULL);
                    
                    /* as things get moved down and to the right when making room
                    ** giving this extra offset the the object position means that
                    ** it has a looser tolerance to fit into the whole just made for it */                     
                 objectPos.top = objectPos.top + (ICON_SIZE/4);
                 objectPos.left = objectPos.left + (ICON_SIZE/4);
                 objectPos.right = objectPos.right + (ICON_SIZE/4);
                 objectPos.bottom = objectPos.bottom + (ICON_SIZE/4);
              }
              gridCorrectRect (&objectPos, &objectPos);
              createObject(win, objectPos);
              setCurrentObject (win, (MENU_TAG) M_TOOL_POINTER);
           }
        }
        else if (drag_type == DR_OUTLINE)
        {
           {
              normalizeRect (&outline, &outline);
              if (!ep->v.mouse.shift)
                 obj_clear_selections(win);
              obj_select_enclosed(win, &outline);
           }
        }
        else
        {
                 /* If an obect is draged off the top or left then delete it */
           if ((point.h < 0) || (point.v < 0))
           {
              if (drag_objp && drag_objp->selected) /* delete whole selection */
                 deleteSelectedObjects (win, FALSE);
              else
                 deleteObject(win, drag_objp);  /* delete single object */
           }
              
           else if (!moveObject(win, drag_objp, down_pnt, point,
                                                             deleteIfInvalid))
           {
                   /* when a paste event fail we want to delete the object **
                   ** that was being pasted */
              if (deleteIfInvalid)
                 deleteSelectedObjects (win, FALSE);
              else
              {   /* if they didn't move it then they were just selecting it **
                  ** so we clear what was selected and select what we thought
                  ** the move event was on */
                 if (!ep->v.mouse.shift)
                    obj_clear_selections(win);
                 obj_select(drag_objp, TRUE);
              }
           }
           if (deleteIfInvalid) /* just handled any paste event with the move */
              deleteIfInvalid = FALSE;
        }
        drag_type = DR_NONE;
        break;
   }
   return(TRUE);
}

/* ======================================================================
FUNCTION  doScroll

DESCRIPTION
     Function to handle scroll-bar events.

RETURNED
====================================================================== */
static void
#if XVT_CC_PROTO
doScroll (WINDOW win, EVENT *ep)
#else
doScroll (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
     WINDOW_INFO *wip = get_win_info(win);
     int n;
     RCT rct;

     xvt_vobj_get_client_rect(win, &rct);
     switch (ep->type) {
     case E_HSCROLL:
          switch (ep->v.scroll.what) {
          case SC_LINE_UP:
               n = MAX(0, wip->origin.h - PAGE_INCR);
               shift_view(win, wip->origin.h - n, 0, NULL);
               break;
          case SC_LINE_DOWN:
               /* avoid rightward jump */
               if (wip->origin.h < wip->range.h) {
                    n = MIN(wip->range.h,wip->origin.h + PAGE_INCR);
                    shift_view(win, wip->origin.h - n, 0, NULL);
               }
               break;
          case SC_PAGE_UP:
               n = MAX(0, wip->origin.h - (rct.right - rct.left));
               shift_view(win, wip->origin.h - n, 0, NULL);
               break;
          case SC_PAGE_DOWN:
               n = MIN(wip->range.h, wip->origin.h + (rct.right - rct.left));
               shift_view(win, wip->origin.h - n, 0, NULL);
               break;
          case SC_THUMB:
               shift_view(win, wip->origin.h - ep->v.scroll.pos, 0, NULL);
					break;
          }
          break;
     case E_VSCROLL:
          switch (ep->v.scroll.what) {
          case SC_LINE_UP:
               n = MAX(0, wip->origin.v - PAGE_INCR);
               shift_view(win, 0, wip->origin.v - n, NULL);
               break;
          case SC_LINE_DOWN:
               /* avoid downward jump */
               if (wip->origin.v < wip->range.v) {
                    n = MIN(wip->range.v, wip->origin.v + PAGE_INCR);
                    shift_view(win, 0, wip->origin.v - n, NULL);
               }
               break;
          case SC_PAGE_UP:
               n = MAX(0, wip->origin.v - (rct.bottom - rct.top));
               shift_view(win, 0, wip->origin.v - n, NULL);
               break;
          case SC_PAGE_DOWN:
               n = MIN(wip->range.v, wip->origin.v + (rct.bottom - rct.top));
               shift_view(win, 0, wip->origin.v - n, NULL);
               break;
          case SC_THUMB:
               shift_view(win, 0, wip->origin.v - ep->v.scroll.pos, NULL);
               break;
          }
     }
}



/* ======================================================================
FUNCTION  windowEventHandler

DESCRIPTION
     Window event handler

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
windowEventHandler (WINDOW win, EVENT *ep)
#else
windowEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
        RCT menuSize, outerRect;
        WINDOW_INFO *wip;
        BOOLEAN printFiles;
        int fileCount;
        FILE_SPEC *fileToLoad;
		  char filename[100];
        OBJECT *object, *nextObject;
        BLOCK_VIEW_OPTIONS *blockView;
   
#if (XVTWS == MACWS)
        if (processingLongJob)
           return 0L;
#endif

        switch (ep->type) {
        case E_CREATE:
                historyWindow = win;  /* store as a global */
                setupWindow(win);
                scroll_sync(win);

                xvt_vobj_get_outer_rect (win, &outerRect);
                getWindowStartPosition (TOOLBAR_TITLE, &menuSize.left, &menuSize.top, NULL, NULL, NULL, NULL);
                menuSize.right = menuSize.left + NUM_MENU_OPTIONS * (ICON_SIZE+2) + 2;
                menuSize.bottom = menuSize.top + (ICON_SIZE+4);
                menuWindow = xvt_win_create(W_DOC, &menuSize, TOOLBAR_TITLE,
                                  0, TASK_WIN, WSF_NO_MENUBAR,
                                  EM_ALL, menuEventHandler, (long) win);
                xvt_errmsg_sig_if(!(menuWindow != NULL_WIN), NULL_WIN, SEV_FATAL, ERR_ASSERT_4,"106",
                                             106, "Cannot create menu window");
                                             

                statusBar = statbar_create(STATUS_BAR, 0, 100, 100, 200, 
                                    numStatusBarProps, (char **) statusBarProps,
                                    TASK_WIN, (int) TASK_WIN, 0x810L, "");
                statbar_set_default_title (statusBar, STATUS_DEFAULT);
                statbar_set_title (statusBar, STATUS_BUILD_HISTORY);

                updateMenuOptions (TASK_MENUBAR, NULL_WIN);
                xvt_fsys_set_dir_startup ();

                        /* put this window in the created heirachy */
                addWinToStack (win);

                        /* check if a history needs to be loaded */
                if (commandLine)
						 fileCount = 1;
					 else
						 fileCount = 0;

                if (fileCount)
                {
						 strcpy (filename, commandLine);
						 if ((strstr(filename, ".his")) || (strstr(filename, ".HIS")))
	                   loadHistoryFile (filename);
						 else if ((strstr(filename, ".mag")) || (strstr(filename, ".MAG")))
						 {
							 loadHistoryFile (NODDY_DEFAULT_SETUP_FILE);
	 						 setCurrentFileName (NULL);
	 						 displayAnomaliesImage (filename, (ANOMIMAGE_DATA *) NULL);
						 }
						 else if ((strstr(filename, ".grv")) || (strstr(filename, ".GRV")))
						 {
							 loadHistoryFile (NODDY_DEFAULT_SETUP_FILE);
							 setCurrentFileName (NULL);
							 displayAnomaliesImage (filename, (ANOMIMAGE_DATA *) NULL);
						 }
                }
                else  /* try and load a default setup if it exists */
                {
                   loadHistoryFile (NODDY_DEFAULT_SETUP_FILE);
                   setCurrentFileName (NULL);
                }

                readRockPropDatabase ();
                break;
        case E_DESTROY:
                        /* take this window out of the created heirachy */
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
                             /* Free memory associated with UNDO command */
                copyUndoAdditions ((OBJECT *) NULL, FALSE);
                copyUndoDeletions ((OBJECT *) NULL, FALSE);

                memManagerFreeAll ();
                takeWinFromStack (win);

                              /* Free Memory used to perform cancel on Windows */
                copyOptions (0, (OBJECT *) NULL, OPTIONS_FREE_MEMORY);
                
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

                }   
                                   /* Free window info structure */
                if (wip = (WINDOW_INFO *) get_win_info(win))
                {
                   xvt_mem_free ((char *) wip);
                   xvt_vobj_set_data (win, 0L);
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

					 destroyLegendWindow ();
                
                menuWindow = NULL_WIN;
                historyWindow = NULL_WIN;
                break;
        case E_CLOSE:
                if (is_quit_OK())
                {
                   deleteSelectedObjects (win, TRUE);
                   xvt_app_destroy ();
                }
                break;
        case E_COMMAND:
                do_TASK_MENUBAR(win, ep);
                break;
        case E_HSCROLL:
        case E_VSCROLL:
                doScroll (win, ep);
                break;
        case E_MOUSE_DOWN:
        case E_MOUSE_DBL:
        case E_MOUSE_UP:
        case E_MOUSE_MOVE:
                doMouse (win, ep);
                break;
        case E_UPDATE:
                updateDrawingWindow (win);
                break;
        case E_FOCUS:
                statbar_set_title (statusBar, STATUS_BUILD_HISTORY);
                break;
        }
        return 0L;
}


/* ======================================================================
FUNCTION        menuEventHandler

DESCRIPTION
        floating menu event handler

RETURNED
====================================================================== */
static long
#if XVT_CC_PROTO
menuEventHandler (WINDOW win, EVENT *ep)
#else
menuEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{
   WINDOW_INFO *wip;
   int iconSpacing = ICON_SIZE + 2;
   static int tools[NUM_MENU_OPTIONS] = {
        M_TOOL_POINTER,
        M_HISTORY_STRATIGRAPHY,
        M_HISTORY_FOLD,
        M_HISTORY_FAULT,
        M_HISTORY_UNCONFORMITY,
        M_HISTORY_SHEAR,
        M_HISTORY_DYKE,
        M_HISTORY_PLUG,
        M_HISTORY_STRAIN,
        M_HISTORY_TILT,
        M_HISTORY_FOLIATION,
        M_HISTORY_LINEATION,
        M_HISTORY_IMPORT,
        M_HISTORY_STOP };
   int selectedTool;
   WINDOW drawingWindow;

   switch (ep->type)
   {
        case E_CREATE:
              drawingWindow = (WINDOW) xvt_vobj_get_data(win);
              wip = get_win_info(drawingWindow);
              xvt_vobj_set_title (win, TOOLBAR_TITLE);
              updateFloatingMenu (win, wip->head, TL_POINTER);
              break;
        case E_DESTROY:
                break;
        case E_CLOSE:
                deleteSelectedObjects (win, TRUE);
                xvt_vobj_destroy(win);
                break;
        case E_COMMAND:
                break;
        case E_HSCROLL:
        case E_VSCROLL:
                doScroll (win, ep);
                break;
        case E_MOUSE_DOWN:
              if (ep->v.mouse.shift || ep->v.mouse.control) /* Toggle Menu shape */
              {
                 int rows, cols;
                 
                 getFloatingMenuShape (win, &rows, &cols);
                 if (cols == MENU1_COLS)
                    setFloatingMenuShape (win, MENU2_ROWS, MENU2_COLS);
                 else if (cols == MENU2_COLS)
                    setFloatingMenuShape (win, MENU3_ROWS, MENU3_COLS);
                 else
                    setFloatingMenuShape (win, MENU1_ROWS, MENU1_COLS);
                 
              }          
				  else  /* Select Event */
				  {
					  drawingWindow = (WINDOW) xvt_vobj_get_data(win);
										  /* order from the TOOL type (+1 to skip pointer) */
					  for (selectedTool = 0; selectedTool < NUM_MENU_OPTIONS; selectedTool++)
					  {
						  if (xvt_rect_has_point(&menuItemPositions[selectedTool],
														 ep->v.mouse.where))
							  break;
					  }
					  if ((selectedTool >= 0) && (selectedTool < NUM_MENU_OPTIONS))
						  setCurrentObject (drawingWindow,(MENU_TAG)tools[selectedTool]);
				  }
              break;
        case E_MOUSE_DBL:
              if (ep->v.mouse.shift && ep->v.mouse.control)  /* Add hidden event */
              {
                 drawingWindow = (WINDOW) xvt_vobj_get_data(win);
                 setCurrentObject (drawingWindow,(MENU_TAG) M_HISTORY_GENERIC);
              }
              else /* just add the event to end of history quickly */
              {/*
                 gridCorrectRect (&objectPos, &objectPos);
                 createObject(win, objectPos);
                 setCurrentObject (win, (MENU_TAG) M_TOOL_POINTER); */
              }
              break;
        case E_MOUSE_UP:
              break;
        case E_MOUSE_MOVE:
              break;
        case E_UPDATE: case E_SIZE:
              drawingWindow = (WINDOW) xvt_vobj_get_data(win);
              wip = get_win_info(drawingWindow);
              updateFloatingMenu (win, wip->head, wip->tool);
              break;
        case E_FOCUS:
              statbar_set_title (statusBar, STATUS_EVENT_MENU_HELP);
              break;
   }
   return 0L;
}


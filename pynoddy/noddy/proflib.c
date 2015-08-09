/*
*/
#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include "scales.h"

#define DEBUG(X)    
#define TOLERANCE  0.0001
                              /* Extern veriables */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern PROJECT_OPTIONS projectOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;

#if XVT_CC_PROTO
extern PROFILE_OPTIONS *getCurrentProfileOptions (WINDOW, PROFILE_OPTIONS *);
#else
extern PROFILE_OPTIONS *getCurrentProfileOptions ();
#endif
                                /* Functions in this file */
#if XVT_CC_PROTO
int drawProfileBackground (WINDOW, PROFILE_OPTIONS *, COLOR);
int drawProfileLabels (WINDOW, PROFILE_OPTIONS *, int *);
int DrawProfileFrame (WINDOW, PROFILE_OPTIONS *);
int DrawProfileGraph (WINDOW, PROFILE_OPTIONS *);
PROFILE_POINT *profilePointSelect (WINDOW, PNT *);
int profilePointMove (WINDOW, PROFILE_POINT *, PNT *);
int profilePointDrawMoving (WINDOW, PROFILE_POINT *);
int profilePointAdd (WINDOW, PNT *);
int profilePointDelete (PROFILE_OPTIONS *, PROFILE_POINT *);
int profilePointDeleteAll (PROFILE_OPTIONS *);
PROFILE_POINT *profilePointsDuplicate (PROFILE_POINT *);
int profilePointsFree (PROFILE_POINT *points);
int canvasPntToArray (RCT *, PNT *, PNT *, PROFILE_OPTIONS *);
int arrayPntToCanvas (RCT *, PNT *, PNT *, PROFILE_OPTIONS *);
int getDragRect (PNT *, RCT *);
float *getArrayForProfile (PROFILE_OPTIONS *);
int freeArrayForProfile (float *);
double calcProfile (float *, double);
static short larangeInterp (PROFILE_POINT *, short);
static Point2d *BezierInit(int, PROFILE_POINT *);
static int BezierFinish(Point2d *);
static int BezierCurve(int, Point2d *, Point2d *, float);
#else
int drawProfileBackground ();
int drawProfileLabels ();
int DrawProfileFrame ();
int DrawProfileGraph ();
PROFILE_POINT *profilePointSelect ();
int profilePointMove ();
int profilePointDrawMoving ();
int profilePointAdd ();
int profilePointDelete ();
int profilePointDeleteAll ();
PROFILE_POINT *profilePointsDuplicate ();
int profilePointsFree ();
int canvasPntToArray ();
int arrayPntToCanvas ();
int getDragRect ();
float *getArrayForProfile ();
int freeArrayForProfile ();
double calcProfile ();
static short larangeInterp ();
static Point2d *BezierInit();
static int BezierFinish();
static int BezierCurve();
#endif

/* ======================================================================
FUNCTION        drawProfileBackground

DESCRIPTION
        Draw a profile color for the current graph in the background
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
drawProfileBackground (WINDOW win, PROFILE_OPTIONS *profOptions, COLOR color)
#else
drawProfileBackground (win, profOptions, color)
WINDOW win;
PROFILE_OPTIONS *profOptions;
COLOR color;
#endif
{
   float *profArray = NULL;
   int state, newState, pos;
   RCT rect, border;
   DRAW_CTOOLS tools;
   PROFILE_OPTIONS *overlayProfOptions;

   overlayProfOptions = (PROFILE_OPTIONS *) xvt_vobj_get_data(win);
   if ((profOptions->type == ANI_PROFILE)
                         || (profOptions->type == SUS_X_PROFILE)
                         || (profOptions->type == SUS_Y_PROFILE)
                         || (profOptions->type == SUS_Z_PROFILE)
                         || (profOptions->type == SUS_DIP_PROFILE)
                         || (profOptions->type == SUS_DDIR_PROFILE)
                         || (profOptions->type == SUS_PITCH_PROFILE))
   {
      while (overlayProfOptions && overlayProfOptions->type != ANI_PROFILE)
         overlayProfOptions = overlayProfOptions->next;
   }
   else if ((profOptions->type == REM_PROFILE)
                         || (profOptions->type == REM_DEC_PROFILE)
                         || (profOptions->type == REM_INC_PROFILE)
                         || (profOptions->type == REM_STR_PROFILE))
   {
      while (overlayProfOptions && overlayProfOptions->type != REM_PROFILE)
         overlayProfOptions = overlayProfOptions->next;
   }
   else
      return (FALSE);
      
   if (!overlayProfOptions)
      return (FALSE);
   
   if (!(profArray = (float *) getArrayForProfile (overlayProfOptions)))
      return (FALSE);
      
   xvt_vobj_get_client_rect (win, &border);

   xvt_dwin_get_draw_ctools (win, &tools);
   tools.brush.color = color;
   tools.brush.pat = PAT_SOLID;
   tools.pen.color = color;
   tools.pen.pat = PAT_SOLID;
   xvt_dwin_set_draw_ctools (win, &tools);
                                       
   rect.top = 0;
   rect.bottom = border.bottom;
   if (state = (fabs((double) profArray[0]) >= 1.0 ))
      rect.left = 0;
         
   for (pos = 1; pos < X_PROFILE_RESOLUTION; pos++)
   {
      newState = (fabs((double) profArray[pos]) >= 1.0 );
      if (((newState == 0) && (state != 0)) || ((newState != 0) && (state == 0)))
      {
         if (newState != 0) /* Turning On */
            rect.left = (short) (((double) pos)/((double) X_PROFILE_RESOLUTION)
                                           * (double) border.right);
         else  /* Turning Off  so draw rect */
         {
            rect.right =(short) (((double) pos)/((double) X_PROFILE_RESOLUTION)
                                           * (double) border.right);
            xvt_dwin_draw_rect (win, &rect);
         }
         
         state = newState;
      }
   }
   
   if (state)  /* draw the last rect if it was on */
   {
      rect.right = border.right;
      xvt_dwin_draw_rect (win, &rect);
   }

/*  Not needed as a pointer to the array is kept in the
**  profOptions structure.
**   freeArrayForProfile (profArray);
*/
   return (TRUE);
}


/* ======================================================================
FUNCTION        DrawProfileLabels

DESCRIPTION
        Draw a profile labels
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
drawProfileLabels (WINDOW win, PROFILE_OPTIONS *profOptions, int *logScale)
#else
drawProfileLabels (win, profOptions, logScale)
WINDOW win;
PROFILE_OPTIONS *profOptions;
int *logScale;
#endif
{
   int textWidth, textAscent, textDescent;
   char labelTop[30], labelBot[30];
   RCT border;
   int drawAniBackground = FALSE, drawRemBackground = FALSE;

   xvt_vobj_get_client_rect (win, &border);

   *logScale = FALSE;
                                /* Add Labels if necessary */
   if (profOptions->joinType == SQUARE_SEGMENTS)
   {
      switch (profOptions->type)
      {
         case (DENSITY_PROFILE):
            sprintf (labelTop, "%.0f", profOptions->maxYReplace);
            sprintf (labelBot, "%.0f", profOptions->minYReplace);
            break;
         case (ANI_PROFILE):
            strcpy (labelTop, "0 = OFF, else ON");
            strcpy (labelBot, "0 = OFF, else ON");
            break;
         case (REM_PROFILE):
            strcpy (labelTop, "0 = OFF, else ON");
            strcpy (labelBot, "0 = OFF, else ON");
            break;
         case (SUS_X_PROFILE): case (SUS_Y_PROFILE):
         case (SUS_Z_PROFILE):
            sprintf (labelTop, "%.0e", pow(10.0,profOptions->maxYReplace));
            sprintf (labelBot, "%.0e", pow(10.0,profOptions->minYReplace));
            *logScale = TRUE;
            break;
         case (REM_STR_PROFILE):
            sprintf (labelTop, "%.0e", pow(10.0,profOptions->maxYReplace));
            sprintf (labelBot, "%.0e", pow(10.0,profOptions->minYReplace));
            *logScale = TRUE;
            break;
         case (SUS_DDIR_PROFILE): case (SUS_PITCH_PROFILE):
         case (SUS_DIP_PROFILE):
            sprintf (labelTop, "%.1f deg", profOptions->maxYReplace);
            sprintf (labelBot, "%.1f deg", profOptions->minYReplace);
            break;
         case (REM_INC_PROFILE): case (REM_DEC_PROFILE):
            sprintf (labelTop, "%.1f deg", profOptions->maxYReplace);
            sprintf (labelBot, "%.1f deg", profOptions->minYReplace);
            break;
         default:
            sprintf (labelTop, "%.2f", profOptions->maxYReplace);
            sprintf (labelBot, "%.2f", profOptions->minYReplace);
      }
   }
   else
   {
      switch (profOptions->type)
      {
         case (DENSITY_PROFILE):
            sprintf (labelTop, "%.0f %%", profOptions->maxYScale*100.0);
            sprintf (labelBot, "%.0f %%", profOptions->minYScale*100.0);
            break;
         case (ANI_PROFILE):
            strcpy (labelTop, "0 = OFF, else ON");
            strcpy (labelBot, "0 = OFF, else ON");
            break;
         case (REM_PROFILE):
            strcpy (labelTop, "0 = OFF, else ON");
            strcpy (labelBot, "0 = OFF, else ON");
            break;
         case (SUS_X_PROFILE): case (SUS_Y_PROFILE):
         case (SUS_Z_PROFILE):
            sprintf (labelTop, "%.0e", pow(10.0,profOptions->maxYScale));
            sprintf (labelBot, "%.0e", pow(10.0,profOptions->minYScale));
            *logScale = TRUE;
            break;
         case (REM_STR_PROFILE):
            sprintf (labelTop, "%.0e", pow(10.0,profOptions->maxYScale));
            sprintf (labelBot, "%.0e", pow(10.0,profOptions->minYScale));
            *logScale = TRUE;
            break;
         case (SUS_DDIR_PROFILE): case (SUS_PITCH_PROFILE):
         case (SUS_DIP_PROFILE):
            sprintf (labelTop, "%.1f deg", profOptions->maxYScale);
            sprintf (labelBot, "%.1f deg", profOptions->minYScale);
            break;
         case (REM_INC_PROFILE): case (REM_DEC_PROFILE):
            sprintf (labelTop, "%.1f deg", profOptions->maxYScale);
            sprintf (labelBot, "%.1f deg", profOptions->minYScale);
            break;
         default:
            sprintf (labelTop, "%.2f", profOptions->maxYScale);
            sprintf (labelBot, "%.2f", profOptions->minYScale);
      }
   }
   
   xvt_dwin_set_fore_color (win, COLOR_WHITE);
   xvt_dwin_get_font_metrics (win, NULL, &textAscent, &textDescent);
   textWidth = xvt_dwin_get_text_width(win, labelTop, -1);
   xvt_dwin_draw_text (win, border.right - textWidth - 2,
                            border.top + textAscent + 2,
                            labelTop, -1);

   textWidth = xvt_dwin_get_text_width(win, labelBot, -1);
   xvt_dwin_draw_text (win, border.right - textWidth - 2,
                            border.bottom - textDescent - 2,
                            labelBot, -1);

   return (TRUE);
}


/* ======================================================================
FUNCTION        DrawProfileFrame

DESCRIPTION
        Draw a profile background
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
DrawProfileFrame (WINDOW win, PROFILE_OPTIONS *profOptions)
#else
DrawProfileFrame (win, profOptions)
WINDOW win;
PROFILE_OPTIONS *profOptions;
#endif
{
   DRAW_CTOOLS tools;
   RCT border;
   PNT line[2];
   double increment;   
   int i, logScale, height, width;
   int divisionX, divisionY, hardEveryX, hardEveryY;
   
                       /* setup the for the background */
   xvt_dwin_clear (win, COLOR_BLACK);
   xvt_dwin_get_draw_ctools (win, &tools);
   xvt_vobj_get_client_rect (win, &border);
   height = border.bottom - border.top;
   width = border.right - border.left;

                       /* horizontal lines */
   tools.pen.style = P_SOLID;
   tools.pen.pat = PAT_SOLID;
   tools.pen.width = 1;
   tools.mode = M_COPY;
   xvt_dwin_set_draw_ctools (win, &tools);

   drawProfileBackground (win, profOptions, COLOR_GREEN);
   drawProfileLabels (win, profOptions, &logScale);

   divisionY = 10;
   hardEveryY = 5;
   divisionX = 20;
   hardEveryX = 5;
   increment = (double) (border.bottom - border.top)/((double) divisionY);
   
   for (i = 1; i < divisionY; i++)
   {
      line[0].h = border.left;
      line[0].v = border.top + ((short) (i*increment));
      line[1].h = border.right;
      line[1].v = line[0].v;

      if ((i%hardEveryY == 0) && (!logScale))
      {
         tools.pen.color = COLOR_WHITE;
         tools.pen.style = P_SOLID;
         xvt_dwin_set_draw_ctools (win, &tools);
      }
      else
      {
         tools.pen.color = COLOR_LTGRAY;
         tools.pen.style = P_DOT;
         xvt_dwin_set_draw_ctools (win, &tools);
      }

      xvt_dwin_draw_polyline (win, line, 2);  /* Center */
   }

   increment = (double) (border.right - border.left)/((double) divisionX);
   for (i = 1; i < divisionX; i++)
   {
      line[0].h = border.left + ((short) (increment*i));
      line[0].v = border.top;
      line[1].h = line[0].h;
      line[1].v = border.bottom;

      if (i%hardEveryX == 0)
      {
         tools.pen.color = COLOR_WHITE;
         tools.pen.style = P_SOLID;
         xvt_dwin_set_draw_ctools (win, &tools);
      }
      else
      {
         tools.pen.color = COLOR_LTGRAY;
         tools.pen.style = P_DOT;
         xvt_dwin_set_draw_ctools (win, &tools);
      }

      xvt_dwin_draw_polyline (win, line, 2);  /* Divisions */
   }
   
   
   return (TRUE);
}


/* ======================================================================
FUNCTION        DrawProfileGraph

DESCRIPTION
        Draw a profile Graph with drag points
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
DrawProfileGraph (WINDOW win, PROFILE_OPTIONS *profOptions)
#else
DrawProfileGraph (win, profOptions)
WINDOW win;
PROFILE_OPTIONS *profOptions;
#endif
{
   PROFILE_POINT *point, *lastPoint;
   BOOLEAN first = TRUE;
   PNT canvasPoint, lastCanvasPoint, temp, temp2;
   RCT dragRect;
   DRAW_CTOOLS tools;
   RCT bound;
   short x;
   Point2d *bezierCoeff, tempPt;
   int numPoints = 0;
   float bezierPos;
   
   if (!profOptions)
      return (FALSE);
                 
                           /* Count the points first */
   for (point = profOptions->points; (point); point = point->next)
      numPoints++;
   
   if (profOptions->joinType == CURVE_SEGMENTS)
      if (!(bezierCoeff = BezierInit(numPoints, profOptions->points)))
         return (FALSE);
         
   xvt_vobj_get_client_rect (win, &bound);
   xvt_dwin_get_draw_ctools (win, &tools);
   tools.pen.style = P_SOLID;
   tools.pen.pat = PAT_SOLID;
   tools.pen.width = 1;
   tools.pen.color = COLOR_MAGENTA;
   tools.brush.pat = PAT_SOLID;
   tools.mode = M_COPY;
   xvt_dwin_set_draw_ctools (win, &tools);
   
   point = profOptions->points;
   
   while (point)
   {
      arrayPntToCanvas (&bound, &(point->point), &canvasPoint, profOptions);
      
      switch (profOptions->joinType)
      {
         case (LINE_SEGMENTS):
            if (first)
            {
               xvt_dwin_draw_set_pos (win, canvasPoint);
               first = FALSE;
            }
            else
               xvt_dwin_draw_line (win, canvasPoint);
            break;
         case (CURVE_SEGMENTS):
            if (first)
            {
               xvt_dwin_draw_set_pos (win, canvasPoint);
               first = FALSE;
            }
            else
            {
               for (x = lastPoint->point.h; x < point->point.h; x+=4)
               {
                  /* larange Curve
                  ** temp.h = x;
                  ** temp.v = larangeInterp (profOptions->points, (short) x);
                  */
                  bezierPos = ((float) x) / ((float) X_PROFILE_RESOLUTION);
                  BezierCurve(numPoints, bezierCoeff, &tempPt, bezierPos);

                  temp.h = (short) tempPt.x;
                  temp.v = (short) tempPt.y;
                  arrayPntToCanvas (&bound, &temp, &temp2, profOptions);
                  xvt_dwin_draw_line (win, temp2);
               }
               /* bezier cures dont go through points 
               ** xvt_dwin_draw_line (win, canvasPoint);
               */
            }
            break;
         case (SQUARE_SEGMENTS):
            if (first)
            {
               xvt_dwin_draw_set_pos (win, canvasPoint);
               first = FALSE;
            }
            else
            {
               PNT cornerPoint;
               cornerPoint.h = canvasPoint.h;
               cornerPoint.v = lastCanvasPoint.v;
               xvt_dwin_draw_line (win, cornerPoint);
               xvt_dwin_draw_line (win, canvasPoint);
            }
            break;
      }
      
      if (point == profOptions->currentPoint)
         tools.brush.color = COLOR_RED;
      else
         tools.brush.color = COLOR_MAGENTA;
      
      xvt_dwin_set_draw_ctools (win, &tools);
      
      getDragRect (&canvasPoint, &dragRect);
      xvt_dwin_draw_rect (win, &dragRect);
      
      lastCanvasPoint = canvasPoint;
      lastPoint = point;
      point = point->next;
   }

   if (profOptions->joinType == CURVE_SEGMENTS)
      BezierFinish(bezierCoeff);

   return (TRUE);
}                     

/* ======================================================================
FUNCTION        profilePointSelect

DESCRIPTION
        select a point a given location for draging
                                         
RETURNED
====================================================================== */
PROFILE_POINT *
#if XVT_CC_PROTO
profilePointSelect (WINDOW win, PNT *canvasLoc)
#else
profilePointSelect (win, canvasLoc)
WINDOW win;
PNT *canvasLoc;
#endif
{
   PROFILE_OPTIONS *profOptions;
   PROFILE_POINT *point;
   PNT canvasPoint;
   RCT bound, dragRect;

   profOptions = getCurrentProfileOptions (win,
                  (PROFILE_OPTIONS *) xvt_vobj_get_data (win));

   xvt_vobj_get_client_rect (win, &bound);
   point = profOptions->points;
   
   while (point)
   {
      arrayPntToCanvas (&bound, &(point->point), &canvasPoint, profOptions);
      getDragRect (&canvasPoint, &dragRect);
      
      if (xvt_rect_has_point (&dragRect, *canvasLoc))
      {
         profOptions->currentPoint = point;
         return (point);
      }
         
      point = point->next;
   }
   
   profOptions->currentPoint = point;
   return ((PROFILE_POINT *) point);
}

/* ======================================================================
FUNCTION        profilePointMove

DESCRIPTION
        Move the position of a point
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
profilePointMove (WINDOW win, PROFILE_POINT *point, PNT *canvasLoc)
#else
profilePointMove (win, point, canvasLoc)
WINDOW win;
PROFILE_POINT *point;
PNT *canvasLoc;
#endif
{
   PROFILE_OPTIONS *profOptions;
   PROFILE_POINT *pointBefore, *pointAfter;
   RCT bound;
   PNT newPosition;
   
   profOptions = getCurrentProfileOptions (win,
                  (PROFILE_OPTIONS *) xvt_vobj_get_data (win));

   xvt_vobj_get_client_rect (win, &bound);
   
   pointAfter = point->next;
   pointBefore = profOptions->points;
   while (pointBefore && (pointBefore->next != point))
   {
      pointBefore = pointBefore->next;
   }

   canvasPntToArray (&bound, canvasLoc, &newPosition, profOptions);
   
   if (pointBefore)  /* dont move past the point before */
   {
      if (newPosition.h <= pointBefore->point.h)
         newPosition.h = pointBefore->point.h + 1;
   }
   else
      newPosition.h = 0;
   
   if (pointAfter)  /* dont move past the point After */
   {
      if (newPosition.h >= pointAfter->point.h)
         newPosition.h = pointAfter->point.h - 1;
   }
   else
      newPosition.h = X_PROFILE_RESOLUTION;

   point->point.h = newPosition.h;
   point->point.v = newPosition.v;
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        profilePointDrawMoving

DESCRIPTION
        Draw only the point given with drag lines either side
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
profilePointDrawMoving (WINDOW win, PROFILE_POINT *point)
#else
profilePointDrawMoving (win, point)
WINDOW win;
PROFILE_POINT *point;
#endif
{
   PROFILE_OPTIONS *profOptions;
   PROFILE_POINT *pointBefore, *pointAfter;
   PNT canvasPoint;
   RCT dragRect;
   DRAW_CTOOLS tools;
   RCT bound;

   profOptions = getCurrentProfileOptions (win,
                  (PROFILE_OPTIONS *) xvt_vobj_get_data (win));

   xvt_vobj_get_client_rect (win, &bound);
   xvt_dwin_get_draw_ctools (win, &tools);
   tools.pen.style = P_SOLID;
   tools.pen.pat = PAT_SOLID;
   tools.pen.width = 1;
   tools.pen.color = COLOR_YELLOW;
   tools.brush.pat = PAT_SOLID;
   tools.brush.color = COLOR_YELLOW;
   tools.mode = M_XOR;
   xvt_dwin_set_draw_ctools (win, &tools);
   
    /* find points before and after to draw line segs */
   pointAfter = point->next;
   pointBefore = profOptions->points;
   while (pointBefore && (pointBefore->next != point))
   {
      pointBefore = pointBefore->next;
   }
   
   if (pointBefore)  /* draw point before */
   {
      arrayPntToCanvas (&bound, &(pointBefore->point), &canvasPoint, profOptions);
      xvt_dwin_draw_set_pos (win, canvasPoint);
   }

   if (point)   /* the the middle point (one requested) */
   {
      arrayPntToCanvas (&bound, &(point->point), &canvasPoint, profOptions);

      if (pointBefore)
         xvt_dwin_draw_line (win, canvasPoint);
      else
         xvt_dwin_draw_set_pos (win, canvasPoint);

      getDragRect (&canvasPoint, &dragRect);
      xvt_dwin_draw_rect (win, &dragRect);
   }
   
   if (pointAfter)   /* the conection to the point after */
   {
      arrayPntToCanvas (&bound, &(pointAfter->point), &canvasPoint, profOptions);
      xvt_dwin_draw_line (win, canvasPoint);
   }
      
   return (TRUE);
}

/* ======================================================================
FUNCTION        profilePointAdd

DESCRIPTION
        Add a point at a given location
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
profilePointAdd (WINDOW win, PNT *canvasLoc)
#else
profilePointAdd (win, canvasLoc)
WINDOW win;
PNT *canvasLoc;
#endif
{
   PROFILE_OPTIONS *profOptions;
   PROFILE_POINT *point, *lastPoint, *newPoint;
   PNT newArrayPoint;
   RCT bound;
   BOOLEAN sucess = FALSE;

   profOptions = getCurrentProfileOptions (win,
                  (PROFILE_OPTIONS *) xvt_vobj_get_data (win));

   xvt_vobj_get_client_rect (win, &bound);

   canvasPntToArray (&bound, canvasLoc, &newArrayPoint, profOptions);
   if ((newArrayPoint.h < 0) || (newArrayPoint.h > X_PROFILE_RESOLUTION))
      return (FALSE);
   if (abs(newArrayPoint.v) > Y_PROFILE_RESOLUTION)
      return (FALSE);

   if (point = profOptions->points)
   {   
      if (newArrayPoint.h < point->point.h)
      {      /* comes before all so insert at start */
         if (newPoint = (PROFILE_POINT *) xvt_mem_zalloc (sizeof(PROFILE_POINT)))
         {
            newPoint->point.h = newArrayPoint.h;
            newPoint->point.v = newArrayPoint.v;
            newPoint->next = point;
            profOptions->points = newPoint;
            sucess = TRUE;
         }
      }
      else if (newArrayPoint.h == point->point.h)
      {                              /* move the point */
         point->point.h = newArrayPoint.h;
         point->point.v = newArrayPoint.v;
         sucess = TRUE;
      }
      else
      {
         lastPoint = point;
         point = point->next;
         while (point && !sucess)
         {
            if (newArrayPoint.h < point->point.h)
            {          /* between last and current point */
               if (newPoint = (PROFILE_POINT *) xvt_mem_zalloc (sizeof(PROFILE_POINT)))
               {
                  newPoint->point.h = newArrayPoint.h;
                  newPoint->point.v = newArrayPoint.v;
                  newPoint->next = point;
                  lastPoint->next = newPoint;
                  sucess = TRUE;
               }
            }
            else if (newArrayPoint.h == point->point.h)
            {                        /* move the point */
               point->point.h = newArrayPoint.h;
               point->point.v = newArrayPoint.v;
               sucess = TRUE;
            }
            
            lastPoint = point;
            point = point->next;
         }
         
         if (!sucess)
         {
                      /* after them all so add to end */
            if (newPoint = (PROFILE_POINT *) xvt_mem_zalloc (sizeof(PROFILE_POINT)))
            {
               newPoint->point.h = newArrayPoint.h;
               newPoint->point.v = newArrayPoint.v;
               newPoint->next = (PROFILE_POINT *) NULL;
               lastPoint->next = newPoint;
               sucess = TRUE;
            }
         }
      }
   }
      
   return (sucess);
}

/* ======================================================================
FUNCTION        profilePointDelete

DESCRIPTION
        REmove a given point or the current point if none given
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
profilePointDelete (PROFILE_OPTIONS *profOptions, PROFILE_POINT *point)
#else
profilePointDelete (profOptions, point)
PROFILE_OPTIONS *profOptions;
PROFILE_POINT *point;
#endif
{
   PROFILE_POINT *beforePoint, *searchPoint, *afterPoint;
   
   if (point)
      searchPoint = point;
   else
   {
      searchPoint = profOptions->currentPoint;
      profOptions->currentPoint = (PROFILE_POINT *) NULL;
   }

     /* cant find the point to delete so do nothing */
   if (!searchPoint)
      return (FALSE);
   
   afterPoint = searchPoint->next;
   beforePoint = profOptions->points;
   while (beforePoint && (beforePoint->next != searchPoint))
      beforePoint = beforePoint->next;
   
   if (beforePoint)
      beforePoint->next = afterPoint;
   else   
   {      /* nothing before so move point to start */
      profOptions->points = afterPoint;
      afterPoint->point.h = 0;
   }
      
   if (!afterPoint)  /* nothing after so move point to end */
      beforePoint->point.h = X_PROFILE_RESOLUTION;
      
   xvt_mem_free ((char *) searchPoint);
      
   return (TRUE);
}

/* ======================================================================
FUNCTION        profilePointDeleteAll

DESCRIPTION
        REmove a given point or the current point if none given
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
profilePointDeleteAll (PROFILE_OPTIONS *profOptions)
#else
profilePointDeleteAll (profOptions)
PROFILE_OPTIONS *profOptions;
#endif
{
   if (profilePointsFree (profOptions->points))
      profOptions->points = NULL;
   profOptions->points = initGraphPoints (profOptions->points,
                             2, STRAIGHT_PROFILE, profOptions);
   return (TRUE);
}


/* ======================================================================
FUNCTION        profilePointsDuplicate

DESCRIPTION
        Produce a copy of the point list and return it
                                         
RETURNED
====================================================================== */
PROFILE_POINT *
#if XVT_CC_PROTO        
profilePointsDuplicate (PROFILE_POINT *points)
#else
profilePointsDuplicate (points)
PROFILE_POINT *points;
#endif
{
   PROFILE_POINT *copyPoints = NULL, *lastCopy = NULL,
                 *currentPoint,      *currentCopy;
                                 
   currentPoint = points; /* first in the list */
   while (currentPoint)
   {                            /* mem for point copy */
      if (!(currentCopy = (PROFILE_POINT *) xvt_mem_zalloc (sizeof (PROFILE_POINT))))
         return ((PROFILE_POINT *) NULL);
         
      memcpy (currentCopy, currentPoint, sizeof(PROFILE_POINT));
      currentCopy->next = NULL;  /* make sure we dont poitn to orig list */

      if (!copyPoints)  /* store the start of the copys */
         copyPoints = currentCopy;
         
      if (lastCopy)     /* add to end of points already copied */
         lastCopy->next = currentCopy;
         
      lastCopy = currentCopy;

      currentPoint = currentPoint->next; /* do next */
   }
   
   return (copyPoints);
}

/* ======================================================================
FUNCTION        profilePointsFree

DESCRIPTION
        Free the entire list of points
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO        
profilePointsFree (PROFILE_POINT *points)
#else
profilePointsFree (points)
PROFILE_POINT *points;
#endif
{
   PROFILE_POINT *point, *pointToFree;
                                 
   point = points; /* first in the list */
   while (point)
   {
      pointToFree = point;
      point = point->next;
      xvt_mem_free ((char *) pointToFree);
   }
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        updateProfileLocation

DESCRIPTION
        Update the display of the location of the mouse
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO        
updateProfileLocation (WINDOW win, PROFILE_OPTIONS *profOption, PNT *loc)
#else
updateProfileLocation (win, profOption, loc)
WINDOW win;
PROFILE_OPTIONS *profOption;
PNT *loc;
#endif
{
   WINDOW xLocWin, yLocWin, squareWin;
   char xLabel[40], yLabel[40];
   RCT border;
   double widthRatio, heightRatio, xLoc, yLoc;
   int replaceValue;
   
   if (!(xLocWin = xvt_win_get_ctl (win, PROFILE_XPOS)))
      xLocWin = xvt_win_get_ctl (xvt_vobj_get_parent(win), PROFILE_XPOS);

   if (!(yLocWin = xvt_win_get_ctl (win, PROFILE_YPOS)))
      yLocWin = xvt_win_get_ctl (xvt_vobj_get_parent(win), PROFILE_YPOS);

   if (!(squareWin = xvt_win_get_ctl (win, PROFILE_SQUARE_JOIN)))
      squareWin = xvt_win_get_ctl (xvt_vobj_get_parent(win), PROFILE_SQUARE_JOIN);
      
   if (squareWin)
      replaceValue = xvt_ctl_is_checked (squareWin);
   else
      replaceValue = (profOption->joinType == SQUARE_SEGMENTS);

   xvt_vobj_get_client_rect (win, &border);
   border.bottom--;
   
   if (xLocWin && yLocWin)
   {
      widthRatio = (profOption->maxX-profOption->minX)
                          / ((double) (border.right - border.left));
      xLoc = profOption->minX + ((double) loc->h) * widthRatio;
      if (replaceValue)
      {
         heightRatio= (profOption->maxYReplace-profOption->minYReplace)
                          / ((double) (border.bottom - border.top));
         yLoc = profOption->maxYReplace - ((double) loc->v) * heightRatio;
      }
      else
      {
         heightRatio= (profOption->maxYScale-profOption->minYScale)
                          / ((double) (border.bottom - border.top));
         yLoc = profOption->maxYScale - ((double) loc->v) * heightRatio;
      }

      switch (profOption->type)
      {
         case (DENSITY_PROFILE):
            sprintf (xLabel, "X: %.3lf", xLoc);
            if (replaceValue)
               sprintf (yLabel, "Y: %.3lf", yLoc);
            else
               sprintf (yLabel, "Y: %.3lf", yLoc*100.0);
            break;
         case (ANI_PROFILE): case (REM_PROFILE):
            sprintf (xLabel, "X: %.3lf", xLoc);
            if (fabs(yLoc) < 0.5)
               sprintf (yLabel, "Y: Off");
            else
               sprintf (yLabel, "Y: On");
            break;
         case (SUS_X_PROFILE): case (SUS_Y_PROFILE):
         case (SUS_Z_PROFILE): case (REM_STR_PROFILE):
            sprintf (xLabel, "X: %.3lf", xLoc);
            sprintf (yLabel, "Y: %.2e", pow(10.0, yLoc));
            break;
         default:
            sprintf (xLabel, "X: %.3lf", xLoc);
            sprintf (yLabel, "Y: %.3lf", yLoc);
      }

      xvt_vobj_set_title(xLocWin, xLabel);
      xvt_vobj_set_title(yLocWin, yLabel);
   }
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        getDragRect

DESCRIPTION
        Get the drag rect for a point on the canvas
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
getDragRect (PNT *canvasPoint, RCT *dragRect)
#else
getDragRect (canvasPoint, dragRect)
PNT *canvasPoint;
RCT *dragRect;
#endif
{
   int increment = PROFILE_DRAGPOINT_SIZE/2;
   
   dragRect->top = canvasPoint->v - increment;
   dragRect->bottom = canvasPoint->v + increment;
   
   dragRect->left = canvasPoint->h - increment;
   dragRect->right = canvasPoint->h + increment;
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        canvasPntToArray

DESCRIPTION
        convert a canvas PNT to Array coords
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
canvasPntToArray (RCT *bound, PNT *canvasPoint, PNT *arrayPoint,
                  PROFILE_OPTIONS *profOptions)
#else
canvasPntToArray (bound, canvasPoint, arrayPoint, profOptions)
RCT *bound;
PNT *canvasPoint;
PNT *arrayPoint;
PROFILE_OPTIONS *profOptions;
#endif
{
   double hPos, vPos;

   hPos = ((double) canvasPoint->h/(double) bound->right)
                                 * (double) X_PROFILE_RESOLUTION;
   vPos = ((double) canvasPoint->v/(double) bound->bottom)
                                 * (double) Y_PROFILE_RESOLUTION*2;

          /* bottom = Y_PROFILE_RESOLUTION -> 2*YPROFILE_RESOLUTION */
          /* becomes 0 -> -Y_PROFILE_RESOLUTION */
   if (vPos > Y_PROFILE_RESOLUTION)
      vPos = -(vPos - Y_PROFILE_RESOLUTION);
   else   /* top = 0 -> Y_PROFILE_RESOLUTION */
          /* becomes Y_PROFILE_RESOLUTION -> 0 */
      vPos = (Y_PROFILE_RESOLUTION - vPos);

   if (vPos > Y_PROFILE_RESOLUTION)
      vPos = Y_PROFILE_RESOLUTION;
   if (vPos < -Y_PROFILE_RESOLUTION)
      vPos = -Y_PROFILE_RESOLUTION;

   arrayPoint->h = (short) hPos;
   arrayPoint->v = (short) vPos;

   return (TRUE);
}

/* ======================================================================
FUNCTION        arrayPntToCanvas

DESCRIPTION
        convert a array PNT to Canvas coords
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
arrayPntToCanvas (RCT *bound, PNT *arrayPoint, PNT *canvasPoint,
                  PROFILE_OPTIONS *profOptions)
#else
arrayPntToCanvas (bound, arrayPoint, canvasPoint, profOptions)
RCT *bound;
PNT *arrayPoint;
PNT *canvasPoint;
PROFILE_OPTIONS *profOptions;
#endif
{
   double hPos, vPos;

   hPos = ((double) arrayPoint->h/(double) X_PROFILE_RESOLUTION)
                                * (double) bound->right;
   vPos = ((double) arrayPoint->v/(double) Y_PROFILE_RESOLUTION)
                                * (double) bound->bottom/2.0;

          /* bottom 0 -> -bound->bottom/2.0 */
          /* becomes = bound->bottom/2.0 -> bound->bottom */
   if (vPos < 0.0)
      vPos = bound->bottom/2.0 - vPos;
   else   /* top bound->bottom/2.0 -> 0 */
          /* becomes = 0 -> bound->bottom/2.0 */
      vPos = bound->bottom/2.0 - vPos;

   if (vPos > bound->bottom)
      vPos = bound->bottom;
   if (vPos < 0)
      vPos = 0;

   canvasPoint->h = (short) hPos;
   canvasPoint->v = (short) vPos;

   return (TRUE);
}

/* ======================================================================
FUNCTION        getArrayForProfile

DESCRIPTION
        allocate an array of numBins and get a rasterised
        version of the profile lines.
                                         
RETURNED
   malloced memory (short array of numBins)
====================================================================== */
float *
#if XVT_CC_PROTO
getArrayForProfile (PROFILE_OPTIONS *profOptions)
#else
getArrayForProfile (profOptions)
PROFILE_OPTIONS *profOptions;
#endif
{
   float *array = NULL;
   PROFILE_POINT *point, *nextPoint = NULL;
   int bin, numBins, numBinsPlusOne;
   short startX, endX;
   double startValue, endValue, valueDiff;
   double xPercent, value;
   Point2d *bezierCoeff = NULL, temp;
   int numPoints = 0;
   float bezierPos;
                           /* Count the points first */
   for (point = profOptions->points; (point); point = point->next)
      numPoints++;
   
   if (profOptions->joinType == CURVE_SEGMENTS)
      if (!(bezierCoeff = BezierInit(numPoints, profOptions->points)))
         return (FALSE);

                   /* cant do other resolution yet */
   numBins = X_PROFILE_RESOLUTION;
   numBinsPlusOne = numBins+1;
   
   if (profOptions->array)
      array = profOptions->array;
   else
      if (!(array = (float *) create1DArray (numBins+2, sizeof (float))))
      {
         if (bezierCoeff)
            BezierFinish (bezierCoeff);
         return (array);
      }
      
   if (!profOptions->points)   /* No points so set to zero */
   {
      memset (array, 0, (numBins+2)*sizeof(float));
      return (array);
   }
      /* fill the array with values to mimic the profile */
   point = profOptions->points;
   nextPoint = point->next;
   while (point && nextPoint)
   {
      startX = point->point.h;
      endX = nextPoint->point.h;
      startValue = (double) point->point.v;
      endValue = (double) nextPoint->point.v;
      if ((startX > numBins) || (endX > numBins))
      {
         if (bezierCoeff)
            BezierFinish (bezierCoeff);
         destroy1DArray ((char *) array);
         return ((float *) NULL);
      }
      
      array[startX] = (float) startValue; /* the values we have */
      array[endX] = (float) endValue;
      valueDiff = endValue - startValue;
      for (bin = startX; bin <= endX; bin++)
      {
         switch (profOptions->joinType)
         {
            case (LINE_SEGMENTS):
               xPercent = (double) ((double) (bin-startX))
                                 / ((double) (endX-startX));
               value = startValue + xPercent*valueDiff;
               array[bin] = (float) value;
               break;
            case (CURVE_SEGMENTS):
               /* larange Curve
               ** array[bin] = larangeInterp (profOptions->points, (short) bin);
               */
               bezierPos = ((float) bin) / ((float) X_PROFILE_RESOLUTION);
               BezierCurve(numPoints, bezierCoeff, &temp, bezierPos);
               array[bin] = (float) temp.y;
               break;
            case (SQUARE_SEGMENTS):
               array[bin] = (float) startValue;
               break;
         }
      }
                /* only of point After was not NULL */
      point = nextPoint;
      nextPoint = nextPoint->next;
   }   
   
   if (profOptions->joinType == CURVE_SEGMENTS)
      BezierFinish(bezierCoeff);

   return (array);
}
/* ======================================================================
FUNCTION        freeArrayForProfile

DESCRIPTION
        free the memory that was allocated for the profile
        in array format
                                         
RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
freeArrayForProfile (float *profileArray)
#else
freeArrayForProfile (profileArray)
float *profileArray;
#endif
{
   destroy1DArray ((char *) profileArray);
   return (TRUE);
}

/* ======================================================================
FUNCTION        calcProfile

DESCRIPTION
        calc the position in the profile
        Uses a simple linear interpolation between descrete
        values in the array
                                         
RETURNED
====================================================================== */
double
#if XVT_CC_PROTO
calcProfile (float *profileArray, double position)
#else
calcProfile (profileArray, position)
float *profileArray;
double position;
#endif
{
   double value, value1, value2, diff, percentDiff, dIndex; 
   int index1, index2;
/*
{
FILE *fo;
   
if (!(fo = (FILE *) fopen ("data.tmp", "a")))
{
   fo = (FILE *) fopen ("data.tmp","w");
   fprintf (fo, "POS\tINDEX1\tINDEX2\tVAL1\tVAL2\tDIFF\tVALUE\n", position);
}

fprintf (fo, "%f\t", position);
*/
   if (position < 0.0)
   {
      position = fabs(position);
      dIndex = (double) ((position/6.2831853)*((double) X_PROFILE_RESOLUTION));
      percentDiff = dIndex - ((double) floor(dIndex));
      index1 = X_PROFILE_RESOLUTION - ((int) floor(dIndex))%X_PROFILE_RESOLUTION;
      index2 = X_PROFILE_RESOLUTION - ((int)  ceil(dIndex))%X_PROFILE_RESOLUTION;
   }
   else
   {
      dIndex = (double) ((position/6.2831853)*((double) X_PROFILE_RESOLUTION));
      percentDiff = dIndex - ((double) floor(dIndex));
      index1 = ((int) floor(dIndex))%X_PROFILE_RESOLUTION;
      index2 = ((int)  ceil(dIndex))%X_PROFILE_RESOLUTION;
   }
              /* make extra sure values are within ranges */
   if (index1 < 0)
      index1 = 0;
   else if (index1 >= X_PROFILE_RESOLUTION)
      index1 = X_PROFILE_RESOLUTION-1;
   if (index2 < 0)
      index2 = 0;
   else if (index2 >= X_PROFILE_RESOLUTION)
      index2 = X_PROFILE_RESOLUTION-1;
   
                    /* interpolate between these two values */
   value1 = ((double) profileArray[index1])
                           / ((double) Y_PROFILE_RESOLUTION);
   value2 = ((double) profileArray[index2])
                           / ((double) Y_PROFILE_RESOLUTION);


   diff = value2 - value1;
   value = value1 + percentDiff*diff;

   return (value);
}

/* ======================================================================
FUNCTION        larangeInterp

DESCRIPTION
        return the y value for a given x value to draw of smooth
        polygon through the passed array of points
                                         
RETURNED
   short y value
====================================================================== */
static short
#if XVT_CC_PROTO
larangeInterp (PROFILE_POINT *points, short xLoc)
#else
larangeInterp (points, xLoc)
PROFILE_POINT *points;
short xLoc;
#endif
{
   double y, x, Ltop, Lbot;
   PROFILE_POINT *point1, *point2;
   
             /* Larange's interpolation formula */
   x = (double) xLoc;
   y = 0.0;
   
   for (point1 = points; point1 != NULL; point1 = point1->next)
   {
      Ltop = 1.0;
      Lbot = 1.0;
      for (point2 = points; point2 != NULL; point2 = point2->next)
      {
         if (point2 != point1)
         {
            Ltop = Ltop * (double) ((double) x 
                             - ((double) point2->point.h));
            Lbot = Lbot * (double) (((double) point1->point.h)
                             - ((double)point2->point.h));
         }
      }
      y = y + (double) (Ltop/Lbot * ((double) point1->point.v));
   }
   
   return ((short) floor(y + 0.5));
}

/* ======================================================================
FUNCTION        BezierInit

DESCRIPTION
   Setup Bezier coefficient array once for each control polygon.
                                         
RETURNED
   pointer to the coefficients 
====================================================================== */
static Point2d *
#if XVT_CC_PROTO
BezierInit(int numPoints, PROFILE_POINT *points)
#else
BezierInit(numPoints, points)
int numPoints;
PROFILE_POINT *points;
#endif
{
   int k;
   long n, choose;
   PROFILE_POINT *point;
   Point2d *coeff;
   float x, y;
   
   
   if (!(coeff = (Point2d *) xvt_mem_zalloc (numPoints*sizeof(Point2d))))
      return ((Point2d *) NULL);

   n = numPoints - 1;
   for(k = 0, point = points;
              (k <= n) && (point);
                           k++, point = point->next)
   {
     if (k == 0)
        choose = 1;
     else if (k == 1)
        choose = n;
     else
        choose = choose * (n-k+1)/k;
        
     x = ((float) point->point.h)/((float) X_PROFILE_RESOLUTION);
     y = ((float) point->point.v)/((float) Y_PROFILE_RESOLUTION);
     coeff[k].x = (float) (x * choose);
     coeff[k].y = (float) (y * choose);
     
   }
   
   return (coeff);
}

/* ======================================================================
FUNCTION        BezierFinish

DESCRIPTION
   Destroy mem associated with coeff
                                         
RETURNED
====================================================================== */
static int
#if XVT_CC_PROTO
BezierFinish(Point2d *coeff)
#else
BezierFinish(coeff)
Point2d *coeff;
#endif
{
   xvt_mem_free ((char *) coeff);
   
   return (TRUE);
}


/* ======================================================================
FUNCTION        BezierCurve

DESCRIPTION
    Return Point pt(t), t <= 0 <= 1 from C, given the number
    of Points in control polygon. BezierForm must be called
    once for any given control polygon. 
                                         
RETURNED
   short y value
====================================================================== */
static int
#if XVT_CC_PROTO
BezierCurve(int numPoints, Point2d *coeff, Point2d *pt, float pos)
#else
BezierCurve(numPoints, coeff, pt, pos)
int numPoints;
Point2d *coeff;
Point2d *pt;
float pos;
#endif
{
   int k, n;
   double t1, tt, u;
   Point2d *b;
   double x, y;
   
   if (!(b = (Point2d *) xvt_mem_alloc (numPoints*sizeof(Point2d))))
      return (FALSE);
   
   n = numPoints - 1;
   u = pos;
   b[0].x = coeff[0].x;
   b[0].y = coeff[0].y;
   /* b[0].z = c[0].z; */      /* for 3-D curves */
   for(k = 1; k <=n; k++)
   {
      b[k].x = coeff[k].x * u;
      b[k].y = coeff[k].y * u;
      /* b[k].z = c[k].z *u */  /* for 3-D curves */
      u =u*pos;
   };
   
   x = b[n].x;
   y = b[n].y;
   t1 = 1-pos;
   tt = t1;
   for(k =n-1; k >=0; k--)
   {
      x += b[k].x*tt;
      y += b[k].y*tt;
      /* (*pt).z += b[k].z *tt;  */   /* Again, 3-D */
      tt =tt*t1;
   }
   
   pt->x = x*((double) X_PROFILE_RESOLUTION);
   pt->y = y*((double) Y_PROFILE_RESOLUTION);
   xvt_mem_free ((char *) b);
   return (TRUE);
}

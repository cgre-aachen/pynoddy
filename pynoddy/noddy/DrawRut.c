#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include <math.h>
#include "scales.h"

#define DEBUG(X)    

                 /* ************************* */
                 /* Globals used in this file */
static WINDOW currentDrawingWindow = (WINDOW) 0;
static int ixcur = 0, iycur = 0;
static int pointSizeX = 1, pointSizeY = 1;


void setCurrentDrawingWindow (win)
WINDOW win;
{
   DRAW_CTOOLS tools;

   currentDrawingWindow = win;

   if (win)
   {
      xvt_app_get_default_ctools (&tools);
      tools.pen.pat = PAT_SOLID;
      tools.brush.pat = PAT_SOLID;
      xvt_dwin_set_draw_ctools(currentDrawingWindow, &tools);
   }
}

WINDOW getCurrentDrawingWindow ()
{
   return (currentDrawingWindow);
}

/* ************************************************************************
** set the clip rectangle for the window
************************************************************************ */
void
#if XVT_CC_PROTO
setClipRect (RCT *clipRect, BOOLEAN clipOn)
#else
setClipRect (clipRect, clipOn)
RCT *clipRect;
BOOLEAN clipOn;
#endif
{
   if (clipOn)
      xvt_dwin_set_clip (currentDrawingWindow, clipRect);
   else
      xvt_dwin_set_clip (currentDrawingWindow, (RCT *) NULL);
}

/* ************************************************************************
** set the size of points drawn with DrawPoint
************************************************************************ */
void
#if XVT_CC_PROTO
SetColor (COLOR color)
#else
SetColor (color)
COLOR color;
#endif
{
   DRAW_CTOOLS tools;

   xvt_dwin_get_draw_ctools(currentDrawingWindow, &tools);
   tools.pen.color = color;
   tools.brush.color = color;
   xvt_dwin_set_draw_ctools(currentDrawingWindow, &tools);
}

/* ************************************************************************
** set the size of points drawn with DrawPoint
************************************************************************ */
void
#if XVT_CC_PROTO
SetPointSize (int size)
#else
SetPointSize (size)
int size;
#endif
{
   pointSizeX = size;
   pointSizeY = size;
}

/* ************************************************************************
** set the size of points drawn with DrawPoint
************************************************************************ */
void
#if XVT_CC_PROTO
SetRectPointSize (int sizeX, int sizeY)
#else
SetRectPointSize (sizeX, sizeY)
int sizeX, sizeY;
#endif
{
   pointSizeX = sizeX;
   pointSizeY = sizeY;
}

/* ************************************************************************
** place the pen a position ix, iy 
************************************************************************ */
void
#if XVT_CC_PROTO
DrawPoint (int ix, int iy)
#else
DrawPoint (ix,iy)
int ix, iy;
#endif
{
   RCT point;

   point.bottom = iy;
   point.top = point.bottom - pointSizeY;
   point.left = ix;
   point.right = point.left + pointSizeX;

   xvt_dwin_draw_rect (currentDrawingWindow, &point);
   ixcur = ix; iycur = iy;
}

/* ************************************************************************
** place the pen a position ix, iy 
************************************************************************ */
void
#if XVT_CC_PROTO
moveTo (int ix, int iy)
#else
moveTo (ix,iy)
int ix,iy;
#endif
{
   PNT point;

   point.h = ix;
   point.v = iy;

   xvt_dwin_draw_set_pos (currentDrawingWindow, point);
   ixcur = ix; iycur = iy;
}

/* ************************************************************************
** draw a line from the current pen position to ix, iy, leaving the
** pen at position ix, iy
************************************************************************ */
void lineTo(ix,iy)
int ix,iy;
{   
   PNT point;

   point.h = ix;
   point.v = iy;

   xvt_dwin_draw_line (currentDrawingWindow, point);
   xvt_dwin_draw_set_pos (currentDrawingWindow, point);
   ixcur = ix; iycur = iy;
}

/*
Function that draw a point at a location.
It is used to make lines actually draw on the start Point and the end point
*/
int pointAt(ix,iy)
int ix,iy;
{   
#if (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   PNT point;

   point.h = ix;
   point.v = iy;
   xvt_dwin_draw_set_pos (currentDrawingWindow, point);
   point.h = ix-1;
   xvt_dwin_draw_line (currentDrawingWindow, point);
   ixcur = ix; iycur = iy;
#endif
	return (TRUE);
}

/* ************************************************************************
** draw a line from the current pen position to a relative position of
** currentPositionX + ix and currentPositionY + iy 
************************************************************************ */
void line(ix,iy)
int ix,iy;
{
   PNT point;

   point.h = ixcur + ix;
   point.v = iycur + iy;

   xvt_dwin_draw_line (currentDrawingWindow, point);
   xvt_dwin_draw_set_pos (currentDrawingWindow, point);
   ixcur += ix; iycur += iy;
}

/* ************************************************************************
** write a string at the current pen position
************************************************************************ */
void drawString(ix, iy, temp)
int ix, iy;
char *temp;
{
   xvt_dwin_draw_text (currentDrawingWindow, ix, iy, temp, -1);
}

/* ************************************************************************
************************************************************************ */
void penSize(iw,ih)
int iw,ih;
{
   DRAW_CTOOLS tools;
     
   xvt_app_get_default_ctools(&tools);
   tools.pen.width = iw;
   tools.pen.pat = PAT_SOLID;

   xvt_dwin_set_draw_ctools(currentDrawingWindow, &tools);
}


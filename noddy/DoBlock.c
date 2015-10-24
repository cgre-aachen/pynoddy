#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include "3dHier.h"
#include "3dCamera.h"
#include "3dMatPat.h"
#include "3dXFDraw.h"
#include "3dGrid.h"
#include "3dDraw.h"
#include "3dFMath.h"
#if (XVTWS != MACWS)
#include <malloc.h>
#endif

#define DEBUG(X)        
#define TOLERANCE   0.0001

                 /* ********************************** */
                 /* External Globals used in this file */
#if XVT_CC_PROTO                 
extern LAYER_PROPERTIES ***calcBlockSection(double, double, double, double,
                        double, double, double, int *, int *);
extern int sortBlockSurfaces (BLOCK_DIAGRAM_DATA *);
extern int drawBlockSurfaces (BLOCK_DIAGRAM_DATA *);
extern int drawDirectionIndicator (WINDOW, PNT);
#else
extern LAYER_PROPERTIES ***calcBlockSection();
extern int sortBlockSurfaces ();
extern int drawBlockSurfaces ();
extern int drawDirectionIndicator ();
#endif
                 
extern BLOCK_VIEW_OPTIONS *blockViewOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern double iscale; /* scaling factor for geology */
extern BOOLEAN parallel;
extern Fixed zoom; 
extern COLOR backgroundColor;

#if XVT_CC_PROTO
void qSortPoints(THREED_POINT_INFO __huge *, int, int, int);
#else
void qSortPoints();
#endif


                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
int blockFrame(WINDOW, double, double, double, double, double, double);
int drawBlockDiagram(BLOCK_DIAGRAM_DATA *);
int drawBlockSection(WINDOW, COLOR **, int, int, double,
                 double, double, double, double, double, double,
                 double, double, double, double, double, double);
int drawBlockDiagramFrame (Point3d, Point3d, Point3d, int, int, int *, int *, int *);
int draw3dRect (WINDOW, Point3d [4]);
int draw3dLine (WINDOW, Point3d [2]);
int draw3dPoint (WINDOW, Point3d);
int renderBlockDiagram (BLOCK_DIAGRAM_DATA *);
int drawBlockBoreHoles (BLOCK_DIAGRAM_DATA *);
#else
int blockFrame();
int drawBlockDiagram();
int drawBlockSection();
int drawBlockDiagramFrame ();
int draw3dRect ();
int draw3dLine ();
int draw3dPoint ();
int renderBlockDiagram ();
int drawBlockBoreHoles ();
#endif


/***********************************************************************
*                                                                      *
*    void DoBlockDiagram()                                             *
*                                                                      *
*                                                                      *
*    DoBlockDiagram function displays the block diagram of the present *
*    geology in dotmap form for three faces of block and draws         *
*    frame for block                                                   *
*                                                                      *
*    DoBlockDiagram() takes no arguments                               *
*    DoBlockDiagram() returns no value                                 *
*                                                                      *
************************************************************************/
void DoBlockDiagram(blockDiagram, resolution)
BLOCK_DIAGRAM_DATA *blockDiagram;
int resolution;
{
   WINDOW win;
   int totalJob = 0, numEvents = (int) countObjects(NULL_WIN);
   BLOCK_SURFACE_DATA *surface;
   double minX, maxX;
   int displayStatusBox = FALSE;
   char memLabel[20];
   RCT border;
   PNT directionPos;
   
   win = blockDiagram->threedData.pixmap;
   for (surface = blockDiagram->surfaces; surface; surface = surface->next)
   {               /* only display calculation stat window if it is a big one */
      if ((!surface->surfaceData) || (blockDiagram->blockData))
      {
         displayStatusBox = TRUE;
         break;
      }
   }
      
   if (displayStatusBox)
   {
      strcpy (memLabel, "Block");
      if (blockDiagram->blockData)
         totalJob = blockDiagram->nz;
      else
         totalJob = blockDiagram->nz*numEvents + blockDiagram->nx*numEvents
                                               + blockDiagram->ny*numEvents;
      initLongJob (0, totalJob, "Calculating Block Diagram...", memLabel);
   }

   minX = blockDiagram->minXLoc;
   maxX = minX + blockDiagram->nx*blockDiagram->blockSize;
                               
                               /* Adjust focus to center of block */
   if ((blockDiagram->blockData) || (blockDiagram->boreHoles))
      blockDiagram->threedData.focus.z += minX;

   initBlockDiagram3d(&(blockDiagram->threedData)); 
                                            
                               /* Put back the focus to where it was */
   if ((blockDiagram->blockData) || (blockDiagram->boreHoles))
      blockDiagram->threedData.focus.z -= minX;

   xvt_dwin_clear (blockDiagram->threedData.pixmap, backgroundColor);
  
   sortBlockSurfaces (blockDiagram);

   if (blockDiagram->blockData)
      renderBlockDiagram (blockDiagram);
   else if (blockDiagram->boreHoles)
      drawBlockBoreHoles (blockDiagram);
   else
   {   
      drawBlockSurfaces (blockDiagram);
   
      xvt_vobj_get_client_rect (win, &border);
      directionPos.h = 35;
      directionPos.v = border.bottom - 35;
      drawDirectionIndicator (win, directionPos);
   }

/*            Calculation keeping block in memory
**
**   if (blockDiagram->blockData = calcBlockDiagram(blockDiagram->minXLoc,
**                           blockDiagram->minYLoc, blockDiagram->minZLoc,
**                                                  nx, ny, nz, cubeSize))
**   {
**      updateBlockDiagramPoints (blockDiagram);
**      drawBlockDiagram (blockDiagram);
**   }
**
*/
   
   if (displayStatusBox)
      finishLongJob();

   return;
}

/* ********************************************************************* *
initBlockDiagram3d                                                      

Setup the 3d libraries for drawing the block diagrams

** ********************************************************************* */
int initBlockDiagram3d(threedData)
THREED_IMAGE_DATA *threedData;
{
   setup3dDrawing (threedData);
   parallel = TRUE;
   zoom = threedData->scale*DEFAULT_BLOCK_SCALING;
/*
   if (resolution == BLOCK_PREVIEW)
      zoom = 0.022/((maxX-minX)/((double) BLOCK_DIAGRAM_SIZE_X));
   else
      zoom = 0.03/((maxX-minX)/((double) BLOCK_DIAGRAM_SIZE_X));
*/
   
   return (TRUE);
}

/* ********************************************************************* *
findSurfaceInImage                                                      

return the surface that was clicked in

** ********************************************************************* */
BLOCK_SURFACE_DATA *findSurfaceInImage  (location, diagram)
PNT location;
BLOCK_DIAGRAM_DATA *diagram;
{
   BLOCK_SURFACE_DATA *surface, *chosenSurface = NULL;
   Point3d origPoint, transPoint;
   Point2d startPoint, endPoint;
   double blockXStart, blockXEnd;
   RCT surfaceRect;
   
   initBlockDiagram3d(&(diagram->threedData));

   sortBlockSurfaces (diagram);
   
   blockXStart = diagram->minXLoc;
   blockXEnd = diagram->nx*diagram->blockSize + blockXStart;

   for (surface = diagram->surfaces; surface; surface = surface->next)
   {
      origPoint.z = blockXEnd - (surface->xStart - blockXStart);
      origPoint.x = surface->yStart;
      origPoint.y = surface->zStart;
      ApplyTransform(&origPoint, &transPoint, &xFormViewer);
      Project(&transPoint, &startPoint);

      origPoint.z = blockXEnd - (surface->xEnd - blockXStart);
      origPoint.x = surface->yEnd;
      origPoint.y = surface->zEnd;
      ApplyTransform(&origPoint, &transPoint, &xFormViewer);
      Project(&transPoint, &endPoint);
      
      surfaceRect.left = (short) MIN(startPoint.x,endPoint.x);
      surfaceRect.right = (short) MAX(startPoint.x,endPoint.x);
      surfaceRect.top = (short) MIN(startPoint.y,endPoint.y);
      surfaceRect.bottom = (short) MAX(startPoint.y,endPoint.y);

             /* As surfaces are ordered from furthest to closest it is the 
             ** last one with the point in that is closest and therefore probably
             ** the one a person wants */                                   
      if (xvt_rect_has_point(&surfaceRect, location))
      {
         chosenSurface = surface;  /* This Point is in the Surface */
      }
   }
   
   return (chosenSurface);
}


/************************************************************************
*                                                                       *
*    void DoBlockFrame()                                                *
*                                                                       *
*                                                                       *
*    DoBlockFrame function draws frame for block                        *
*                                                                       *
*    DoBlockFrame() takes no arguments                                  *
*    DoBlockFrame() returns no value                                    *
*                                                                       *
************************************************************************/
int blockFrame(win, startX, startY, startZ, endX, endY, endZ)
WINDOW win;
double startX, startY, startZ;
double endX, endY, endZ;
{
   DRAW_CTOOLS tools;
   
   xvt_dwin_get_draw_ctools (win, &tools);
   tools.pen.width = 0;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_BLACK;
   xvt_dwin_set_draw_ctools (win, &tools);
   
   xfMoveto3d (endY,   endZ,   endX,   &xFormViewer);

   xfLineto3d (startY, endZ,   endX,   &xFormViewer);

   xfLineto3d (startY, endZ,   startX, &xFormViewer);

   xfMoveto3d (startY, endZ,   endX,   &xFormViewer);

   xfLineto3d (startY, startZ, endX, &xFormViewer);

   return (TRUE);
}


/***********************************************************************
*                                                                      *
*    int drawBlockSection()                                           *
*                                                                      *
*                                                                      *
*    drawBlockSection function displays the block diagram of a         *
*    specific face
*                                                                      *
*                                                                      *
************************************************************************/
int
#if XVT_CC_PROTO
drawBlockSection(WINDOW win, COLOR **blockSection,
                 int dim1, int dim2, double cubeSize,
                 double xLocStart, double yLocStart, double zLocStart,
                 double xLocEnd,   double yLocEnd,   double zLocEnd,
                 double xBlkStart, double yBlkStart, double zBlkStart,
                 double xBlkEnd,   double yBlkEnd,   double zBlkEnd)
#else
drawBlockSection(win, blockSection, dim1, dim2, cubeSize,
                      xLocStart, yLocStart, zLocStart,
                      xLocEnd,   yLocEnd,   zLocEnd,
                      xBlkStart, yBlkStart, zBlkStart,
                      xBlkEnd,   yBlkEnd,   zBlkEnd)
WINDOW win;
COLOR **blockSection;
int dim1, dim2;
double cubeSize;
double xLocStart, yLocStart, zLocStart;
double xLocEnd,   yLocEnd,   zLocEnd;
double xBlkStart, yBlkStart, zBlkStart;
double xBlkEnd,   yBlkEnd,   zBlkEnd;
#endif
{
   DRAW_CTOOLS tools;
   Point3d Rect3D[4], rect3dPoint;
   Point2d rect2dPoint;
   PNT drawRect[4];
   COLOR *colorPtr, *brushColor;
   double xInc, yInc, zInc, halfXInc, halfYInc, halfZInc;
   double diffX, diffY, diffZ;
   double xOff, yOff, zOff, xLoc, yLoc, zLoc;
   Matrix3D *matrixPtr;
   register int i, j, point, zIsConstant;

                         /* Switch the X Drawing direction */
   xLocStart = xBlkEnd - xLocStart;
   xLocEnd = xBlkEnd - xLocEnd;

   xOff = xLocStart;
   yOff = yLocStart;
   zOff = zLocStart;

   diffX = (xLocEnd - xLocStart);
   diffY = (yLocEnd - yLocStart);
   diffZ = (zLocEnd - zLocStart);
   
   if (fabs(diffZ) < TOLERANCE)
   {
      zIsConstant = TRUE;
   
      xInc = diffX/((double) dim1);
      yInc = diffY/((double) dim2);
      zInc = 0.0;
   }
   else
   {
      zIsConstant = FALSE;

      xInc = diffX/((double) dim1);
      yInc = diffY/((double) dim1);
      zInc = diffZ/((double) dim2);
   }

   halfXInc = xInc/2.0;
   halfYInc = yInc/2.0;
   halfZInc = zInc/2.0;
   
   xvt_dwin_get_draw_ctools (win, &tools);
   tools.pen.width = 0;
   tools.pen.pat = PAT_HOLLOW;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_WHITE;
   tools.brush.color = COLOR_BLACK; 
   tools.brush.pat = PAT_SOLID;
   xvt_dwin_set_draw_ctools (win, &tools);

   brushColor = &(tools.brush.color); 
   matrixPtr = &xFormViewer;
   
   for (i = 0; i < dim1; i++)
   {
      colorPtr = blockSection[i]; 
      incrementLongJob (INCREMENT_JOB);

                /* the values that only vary with i */
      if (zIsConstant)
      {
         xLoc = i*xInc + xOff;
         zLoc = zOff;

         Rect3D[0].y = Rect3D[1].y =
         Rect3D[2].y = Rect3D[3].y = zLoc;

         Rect3D[0].z = xLoc;      Rect3D[1].z = xLoc+xInc;
         Rect3D[2].z = xLoc+xInc; Rect3D[3].z = xLoc;
      }
      else
      {
         xLoc = i*xInc + xOff;
         yLoc = i*yInc + yOff;

         Rect3D[0].z = xLoc;      Rect3D[1].z = xLoc;
         Rect3D[2].z = xLoc+xInc; Rect3D[3].z = xLoc+xInc;
   
         Rect3D[0].x = yLoc;      Rect3D[1].x = yLoc;
         Rect3D[2].x = yLoc+yInc; Rect3D[3].x = yLoc+yInc;
      }

      for (j = 0; j < dim2; j++)
      {
         *brushColor = *colorPtr++;
         xvt_dwin_set_draw_ctools (win, &tools);

                /* the values that only vary with j */
         if (zIsConstant)
         {
            yLoc = j*yInc + yOff;

            Rect3D[0].x = yLoc;      Rect3D[1].x = yLoc;
            Rect3D[2].x = yLoc+yInc; Rect3D[3].x = yLoc+yInc;
         }
         else
         {
            zLoc = j*zInc + zOff;

            Rect3D[0].y = zLoc;      Rect3D[1].y = zLoc+zInc;
            Rect3D[2].y = zLoc+zInc; Rect3D[3].y = zLoc;
         }
                /* this point projection is here as you need
                ** the full xyz location before projecting */
         for (point = 0; point < 4; point++)
         {
            ApplyTransform(&(Rect3D[point]), &rect3dPoint, matrixPtr);
            Project(&rect3dPoint, &rect2dPoint);
            drawRect[point].h = (short) rect2dPoint.x;
            drawRect[point].v = (short) rect2dPoint.y;
         }
         
         xvt_dwin_draw_polygon (win, drawRect, 4);
      }
   }

         /* Draw a black border around the surface we just drew */
   tools.pen.width = 1;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_BLACK;
   xvt_dwin_set_draw_ctools (win, &tools);
   
   if (zIsConstant)
   {
      xfMoveto3d(yLocEnd,   zLocStart, xLocEnd,   &xFormViewer);
      xfLineto3d(yLocEnd,   zLocEnd,   xLocStart, &xFormViewer);
      xfLineto3d(yLocStart, zLocEnd,   xLocStart, &xFormViewer);
      xfLineto3d(yLocStart, zLocStart, xLocEnd,   &xFormViewer);
   }
   else
   {
      xfMoveto3d(yLocEnd,   zLocStart, xLocEnd,   &xFormViewer);
      xfLineto3d(yLocEnd,   zLocEnd,   xLocEnd,   &xFormViewer);
      xfLineto3d(yLocStart, zLocEnd,   xLocStart, &xFormViewer);
      xfLineto3d(yLocStart, zLocStart, xLocStart, &xFormViewer);
   }
                                /* Join back to the start */
   xfLineto3d(yLocEnd,   zLocStart, xLocEnd,   &xFormViewer);
   return (TRUE);
}


/***********************************************************************
*                                                                      *
*    int drawBlockSection()                                           *
*                                                                      *
*                                                                      *
*    drawBlockSection function displays the block diagram of a         *
*    specific face
*                                                                      *
*                                                                      *
************************************************************************/
int
#if XVT_CC_PROTO
drawBlockDiagram(BLOCK_DIAGRAM_DATA *blockDiagram)
#else
drawBlockDiagram(blockDiagram)
BLOCK_DIAGRAM_DATA *blockDiagram;
#endif
{
   WINDOW win;
   DRAW_CTOOLS tools;
   THREED_POINT_INFO __huge *pointPtr;
   int numPoints, point, i;
   Point3d Rect3D[4];
   Point2d Rect2D[4];
   PNT drawRect[4];

   win = blockDiagram->threedData.pixmap;
   xvt_dwin_get_draw_ctools (win, &tools);
   tools.pen.width = 0;
   tools.pen.pat = PAT_HOLLOW;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_WHITE;
   tools.brush.color = COLOR_BLACK; 
   tools.brush.pat = PAT_SOLID;
   xvt_dwin_set_draw_ctools (win, &tools);
   
   if (pointPtr = blockDiagram->threedData.surface)
   {
      numPoints = blockDiagram->threedData.surfaceWidth;
      qSortPoints (pointPtr, 0, numPoints-2, 2);
      
      for (point = 0; point < numPoints; point+=2, pointPtr+=2)
      {
         tools.brush.color = pointPtr->color; 
         xvt_dwin_set_draw_ctools (win, &tools);
         
         Rect3D[0].y = pointPtr[0].point.z + pointPtr[1].point.z;
         Rect3D[1].y = pointPtr[0].point.z - pointPtr[1].point.z;
         Rect3D[2].y = pointPtr[0].point.z - pointPtr[1].point.z;
         Rect3D[3].y = pointPtr[0].point.z + pointPtr[1].point.z;
      
         Rect3D[0].z = pointPtr[0].point.x - pointPtr[1].point.x;
         Rect3D[1].z = pointPtr[0].point.x + pointPtr[1].point.x;
         Rect3D[2].z = pointPtr[0].point.x + pointPtr[1].point.x;
         Rect3D[3].z = pointPtr[0].point.x - pointPtr[1].point.x;
      
         Rect3D[0].x = pointPtr[0].point.y - pointPtr[1].point.y;
         Rect3D[1].x = pointPtr[0].point.y - pointPtr[1].point.y;
         Rect3D[2].x = pointPtr[0].point.y + pointPtr[1].point.y;
         Rect3D[3].x = pointPtr[0].point.y + pointPtr[1].point.y;
         
         for (i = 0; i < 4; i++)  /* transform to something drawable */
         {
            ApplyTransform(&(Rect3D[i]), &(Rect3D[i]), &xFormViewer);
            Project(&(Rect3D[i]), &(Rect2D[i]));
            drawRect[i].h = (short) Rect2D[i].x;
            drawRect[i].v = (short) Rect2D[i].y;
         }
         xvt_dwin_draw_polygon (win, drawRect, 4);
      }
   }

   return (TRUE);
}

/*
int
#if XVT_CC_PROTO
drawSectionThroughBlock(WINDOW win, LAYER_PROPERTIES ***blockSection,
                 int dim1, int dim2, double cubeSize,
                 double xLocStart, double yLocStart, double zLocStart,
                 double xLocEnd,   double yLocEnd,   double zLocEnd)
#else
#endif
{
   return (TRUE);
}

*/
/* ======================================================================
FUNCTION        previewBlockDiagram
DESCRIPTION
     update the Preview display of the block Diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
previewBlockDiagram (BLOCK_DIAGRAM_DATA *blockDiagram)
#else
previewBlockDiagram (blockDiagram)
BLOCK_DIAGRAM_DATA *blockDiagram;
#endif
{
   WINDOW win;
   THREED_IMAGE_DATA *threedData;
   Point3d start;
   Point3d end;
   double blockSize, minX, maxX;
   DRAW_CTOOLS tools;
   PNT directionPos;
   RCT border;

   if (!(threedData = &(blockDiagram->threedData)))
      return (FALSE);
      
   win = blockDiagram->threedData.pixmap;
   blockSize = blockDiagram->blockSize;
   start.x = blockDiagram->minYLoc; end.x = start.x + blockDiagram->ny*blockSize;
   start.y = blockDiagram->minZLoc; end.y = start.y + blockDiagram->nz*blockSize;
   start.z = /*blockDiagram->minXLoc*/0.0; end.z = start.z + blockDiagram->nx*blockSize;
   
   minX = start.z;
   maxX = end.z;

   setup3dDrawing (threedData);
   parallel = TRUE;
   /* zoom = 0.03/((maxX-minX)/((double) BLOCK_DIAGRAM_SIZE_X)); */

   zoom = blockDiagram->threedData.scale*DEFAULT_BLOCK_SCALING;

   xvt_dwin_clear (threedData->pixmap, backgroundColor);

   xvt_dwin_get_draw_ctools (threedData->pixmap, &tools);
   tools.pen.width = 0;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_LTGRAY;
   tools.brush.color = COLOR_BLACK; 
   tools.brush.pat = PAT_SOLID;
   xvt_dwin_set_draw_ctools (threedData->pixmap, &tools);

   xfMoveto3d(start.x, start.y, end.z,   &xFormViewer);
   xfLineto3d(start.x, end.y,   end.z,   &xFormViewer);
   xfLineto3d(start.x, end.y,   start.z, &xFormViewer);
   xfLineto3d(start.x, start.y, start.z, &xFormViewer);
   xfLineto3d(start.x, start.y, end.z,   &xFormViewer);

   xfMoveto3d(end.x,   start.y, end.z,   &xFormViewer);
   xfLineto3d(end.x,   end.y,   end.z,   &xFormViewer);
   xfLineto3d(end.x,   end.y,   start.z, &xFormViewer);
   xfLineto3d(end.x,   start.y, start.z, &xFormViewer);
   xfLineto3d(end.x,   start.y, end.z,   &xFormViewer);

   xfMoveto3d(start.x, start.y, end.z,   &xFormViewer);
   xfLineto3d(end.x,   start.y, end.z,   &xFormViewer);
   xfLineto3d(end.x,   start.y, start.z, &xFormViewer);
   xfLineto3d(start.x, start.y, start.z, &xFormViewer);
   xfLineto3d(start.x, start.y, end.z,   &xFormViewer);

   xfMoveto3d(start.x, end.y,   end.z,   &xFormViewer);
   xfLineto3d(end.x,   end.y,   end.z,   &xFormViewer);
   xfLineto3d(end.x,   end.y,   start.z, &xFormViewer);
   xfLineto3d(start.x, end.y,   start.z, &xFormViewer);
   xfLineto3d(start.x, end.y,   end.z,   &xFormViewer);
   
   xvt_vobj_get_client_rect (win, &border);
   directionPos.h = 35;
   directionPos.v = border.bottom - 35;
   drawDirectionIndicator (win, directionPos);


   return (TRUE);
}


/* ======================================================================
FUNCTION        renderBlockDiagram
DESCRIPTION
     update the Preview display of the block Diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
renderBlockDiagram (BLOCK_DIAGRAM_DATA *blockDiagram)
#else
renderBlockDiagram (blockDiagram)
BLOCK_DIAGRAM_DATA *blockDiagram;
#endif
{
#define SHADE_1   1.2  /* 0.9 */
#define SHADE_2   0.8  /* 0.8 */
   WINDOW win;
   THREED_IMAGE_DATA *threedData;
   Point3d start;
   Point3d end;
   double blockSize, minX, maxX, minY, maxY;
   DRAW_CTOOLS tools;
   PNT directionPos;
   RCT border;
   register int x, y, z, nx, ny, nz, xIdx, yIdx, side;
   int xInc, yInc, zInc;
   int reverseX = FALSE , reverseY = FALSE;
   double xPos, yPos, zPos, xLoc, yLoc;
   Point3d Rect3D[4];
   COLOR ***blockData, currentColor;
   CUBE_SIDES drawSideOrder[6];
                       /* *********************************** */
                       /* Make sure we have something to draw */
   if (!(threedData = &(blockDiagram->threedData)))
      return (FALSE);
   if (!(blockData = blockDiagram->blockData))
      return (FALSE);
                       /* ********************************* */
                       /* Someuseful things to have locally */
   win = blockDiagram->threedData.pixmap;
   blockSize = blockDiagram->blockSize;
   nx = blockDiagram->nx;
   ny = blockDiagram->ny;
   nz = blockDiagram->nz;
   start.x = blockDiagram->minYLoc; end.x = start.x + ny*blockSize;
   start.y = blockDiagram->minZLoc; end.y = start.y + nz*blockSize;
   start.z = blockDiagram->minXLoc; end.z = start.z + nx*blockSize;
   minX = blockDiagram->minXLoc;
   maxX = blockDiagram->minXLoc + nx*blockSize;
   minY = blockDiagram->minYLoc;
   maxY = blockDiagram->minYLoc + ny*blockSize;

                      /* *************************** */
                      /* Initialise the 3d libraries */   
   xvt_dwin_clear (threedData->pixmap, backgroundColor);

                      /* ************************************************** */
                      /* Setup the brushes and pens we will be drawing with */
   xvt_dwin_get_draw_ctools (threedData->pixmap, &tools);

   tools.pen.width = 0;          /* Draw the back (hidden) lines */
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_LTGRAY;
   xvt_dwin_set_draw_ctools (win, &tools);
   drawBlockDiagramFrame (start, end, threedData->camera, FALSE, TRUE, 
                          &xInc, &yInc, &zInc);
               /* Setup drawing of diagram so everything is Z-buffered */
   drawSideOrder[0] = BOTTOM_SIDE;
   if (xInc == 1)
   {
      reverseX = TRUE;
      drawSideOrder[1] = EAST_SIDE;
      drawSideOrder[3] = WEST_SIDE;
   }
   else if (xInc == -1)
   {
      reverseX = FALSE;
      drawSideOrder[1] = WEST_SIDE;
      drawSideOrder[3] = EAST_SIDE;
   }
   
   if (yInc == 1)
   {
      reverseY = FALSE;
      drawSideOrder[2] = SOUTH_SIDE;
      drawSideOrder[4] = NORTH_SIDE;
   }
   else if (yInc == -1)
   {
      reverseY = TRUE;
      drawSideOrder[2] = NORTH_SIDE;
      drawSideOrder[4] = SOUTH_SIDE;
   }
   drawSideOrder[5] = TOP_SIDE;
      
                  /* Setup Brushes and pens for the drawing */
   tools.pen.width = 0;
   tools.pen.pat = PAT_HOLLOW; /* PAT_NONE; */
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_BLACK;
   tools.brush.color = COLOR_BLACK; 
   tools.brush.pat = PAT_SOLID;
   xvt_dwin_set_draw_ctools (threedData->pixmap, &tools);

                      /* *************** */
                      /* Initialise Rect */
   for (x = 0; x < 4; x++)
   {
      Rect3D[x].x = Rect3D[x].y = Rect3D[x].z = 0.0;
   }
                      /* ********************* */
                      /* Draw the actual block */
   for (z = 0, zPos = blockDiagram->minZLoc; (z < nz); z++, zPos+=blockSize)
   {
      incrementLongJob (INCREMENT_JOB);
      for (xIdx = 0, xLoc = maxX; (xIdx < nx); xIdx++, xLoc-=blockSize)
      {                                 
         if (reverseX)
         {
            x = nx - xIdx - 1;
            xPos = minX + (maxX - xLoc) + blockSize; 
         }
         else
         {
            x = xIdx;
            xPos = xLoc;
         }
         
         for (yIdx = 0, yLoc = minY; (yIdx < ny); yIdx++, yLoc+=blockSize)
         {
            if (reverseY)
            {
               y = ny - yIdx - 1;
               yPos = minY + (maxY - yLoc) - blockSize;
            }
            else
            {
               y = yIdx;
               yPos = yLoc;
            }
               
            if ((currentColor = blockData[z][x][y]) != COLOR_INVALID)
            {
               for (side = 0; side < 6; side++)
               {
                  switch (drawSideOrder[side])
                  {
                     case (BOTTOM_SIDE):
                        if ((z == 0) || (blockData[z-1][x][y] == COLOR_INVALID))
                        {                              /* Reached Bottom or Nothing below */
                           Rect3D[0].z = xPos;
                           Rect3D[1].z = xPos;
                           Rect3D[2].z = xPos - blockSize;
                           Rect3D[3].z = xPos - blockSize;
            
                           Rect3D[0].x = yPos + blockSize;
                           Rect3D[1].x = yPos;
                           Rect3D[2].x = yPos;
                           Rect3D[3].x = yPos + blockSize;
                        
                           Rect3D[0].y = Rect3D[1].y = Rect3D[2].y = Rect3D[3].y = zPos;
                     
                           tools.brush.color = currentColor; 
                           xvt_dwin_set_draw_ctools (win, &tools);
                           draw3dRect (win, Rect3D);
                        }
                        break;
                     case (WEST_SIDE):
                        if ((x == 0) || (blockData[z][x-1][y] == COLOR_INVALID))
                        {                              /* Reached Left Edge or Nothing left */
                           Rect3D[0].z = Rect3D[1].z = Rect3D[2].z = Rect3D[3].z = xPos;
            
                           Rect3D[0].x = yPos + blockSize;
                           Rect3D[1].x = yPos;
                           Rect3D[2].x = yPos;
                           Rect3D[3].x = yPos + blockSize;
                        
                           Rect3D[0].y = zPos;
                           Rect3D[1].y = zPos;
                           Rect3D[2].y = zPos + blockSize;
                           Rect3D[3].y = zPos + blockSize;
         
                           tools.brush.color = XVT_MAKE_COLOR(XVT_COLOR_GET_RED(currentColor)*SHADE_2,
                                                              XVT_COLOR_GET_GREEN(currentColor)*SHADE_2,
                                                              XVT_COLOR_GET_BLUE(currentColor)*SHADE_2);
                           xvt_dwin_set_draw_ctools (win, &tools);
                           draw3dRect (win, Rect3D);
                        }
                        break;
                     case (EAST_SIDE):
                        if ((x == nx-1) || (blockData[z][x+1][y] == COLOR_INVALID))
                        {                              /* Reached Right Edge or Nothing right */
                           Rect3D[0].z = Rect3D[1].z = Rect3D[2].z = Rect3D[3].z = xPos - blockSize;
            
                           Rect3D[0].x = yPos + blockSize;
                           Rect3D[1].x = yPos;
                           Rect3D[2].x = yPos;
                           Rect3D[3].x = yPos + blockSize;
                        
                           Rect3D[0].y = zPos;
                           Rect3D[1].y = zPos;
                           Rect3D[2].y = zPos + blockSize;
                           Rect3D[3].y = zPos + blockSize;
                     
                           tools.brush.color = XVT_MAKE_COLOR(XVT_COLOR_GET_RED(currentColor)*SHADE_2,
                                                              XVT_COLOR_GET_GREEN(currentColor)*SHADE_2,
                                                              XVT_COLOR_GET_BLUE(currentColor)*SHADE_2);
                           xvt_dwin_set_draw_ctools (win, &tools);
                           draw3dRect (win, Rect3D);
                        }
                        break;
                     case (SOUTH_SIDE):
                        if ((y == 0) || (blockData[z][x][y-1] == COLOR_INVALID))
                        {                              /* Reached Front or Nothing forward */
                           Rect3D[0].z = xPos;
                           Rect3D[1].z = xPos;
                           Rect3D[2].z = xPos - blockSize;
                           Rect3D[3].z = xPos - blockSize;
            
                           Rect3D[0].x = Rect3D[1].x = Rect3D[2].x = Rect3D[3].x = yPos;
                        
                           Rect3D[0].y = zPos;
                           Rect3D[1].y = zPos + blockSize;
                           Rect3D[2].y = zPos + blockSize;
                           Rect3D[3].y = zPos;
                     
                           tools.brush.color = XVT_MAKE_COLOR(XVT_COLOR_GET_RED(currentColor)*SHADE_1,
                                                              XVT_COLOR_GET_GREEN(currentColor)*SHADE_1,
                                                              XVT_COLOR_GET_BLUE(currentColor)*SHADE_1);
                           xvt_dwin_set_draw_ctools (win, &tools);
                           draw3dRect (win, Rect3D);
                        }
                        break;
                     case (NORTH_SIDE):
                        if ((y == ny-1) || (blockData[z][x][y+1] == COLOR_INVALID))
                        {                              /* Reached Back or Nothing backwards */
                           Rect3D[0].z = xPos;
                           Rect3D[1].z = xPos;
                           Rect3D[2].z = xPos - blockSize;
                           Rect3D[3].z = xPos - blockSize;
            
                           Rect3D[0].x = Rect3D[1].x = Rect3D[2].x = Rect3D[3].x = yPos + blockSize;
                        
                           Rect3D[0].y = zPos;
                           Rect3D[1].y = zPos + blockSize;
                           Rect3D[2].y = zPos + blockSize;
                           Rect3D[3].y = zPos;
                     
                           tools.brush.color = XVT_MAKE_COLOR(XVT_COLOR_GET_RED(currentColor)*SHADE_1,
                                                              XVT_COLOR_GET_GREEN(currentColor)*SHADE_1,
                                                              XVT_COLOR_GET_BLUE(currentColor)*SHADE_1);
                           xvt_dwin_set_draw_ctools (win, &tools);
                           draw3dRect (win, Rect3D);
                        }
                        break;
                     case (TOP_SIDE):
                        if ((z == nz-1) || (blockData[z+1][x][y] == COLOR_INVALID))
                        {                              /* Reached Top or Nothing above */
                           Rect3D[0].z = xPos;
                           Rect3D[1].z = xPos;
                           Rect3D[2].z = xPos - blockSize;
                           Rect3D[3].z = xPos - blockSize;
            
                           Rect3D[0].x = yPos + blockSize;
                           Rect3D[1].x = yPos;
                           Rect3D[2].x = yPos;
                           Rect3D[3].x = yPos + blockSize;
                        
                           Rect3D[0].y = Rect3D[1].y = Rect3D[2].y = Rect3D[3].y = zPos + blockSize;
                     
                           tools.brush.color = currentColor; 
                           xvt_dwin_set_draw_ctools (win, &tools);
                           draw3dRect (win, Rect3D);
                        }
                        break;
                  }
               }
            }
         }
      }
   }
                           /* *************************************** */
                           /* Draw the border and direction indicator */

   tools.pen.width = 0;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_LTGRAY;
   xvt_dwin_set_draw_ctools (win, &tools);
   drawBlockDiagramFrame (start, end, threedData->camera, TRUE, FALSE,
                          (int *) NULL, (int *) NULL, (int *) NULL);

   xvt_vobj_get_client_rect (win, &border);
   directionPos.h = 35;
   directionPos.v = border.bottom - 35;
   drawDirectionIndicator (win, directionPos);

   return (TRUE);
}


/* ======================================================================
FUNCTION        drawBlockBoreHoles
DESCRIPTION
     update the Preview display of the block Diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
drawBlockBoreHoles (BLOCK_DIAGRAM_DATA *blockDiagram)
#else
drawBlockBoreHoles (blockDiagram)
BLOCK_DIAGRAM_DATA *blockDiagram;
#endif
{
   WINDOW win;
   THREED_IMAGE_DATA *threedData;
   LAYER_PROPERTIES *layerProp;
   Point3d start, end, line3d[2];
   double blockSize, minX, maxX, minY, maxY;
   DRAW_CTOOLS tools;
   PNT directionPos;
   RCT border;
   int nx, ny, nz;
   double ***boreHoleData;
   register int hole, point, numHoles, numPoints;
   double ***xyzLoc;
   STORY **histoire;
   int eventIndex;
   int numEvents = countObjects(NULL_WIN);
   unsigned int rockType;

                       /* *********************************** */
                       /* Make sure we have something to draw */
   if (!(threedData = &(blockDiagram->threedData)))
      return (FALSE);
   if (!(boreHoleData = blockDiagram->boreHoles))
      return (FALSE);
                       /* ********************************* */
                       /* Someuseful things to have locally */
   win = blockDiagram->threedData.pixmap;
   blockSize = blockDiagram->blockSize;
   nx = blockDiagram->nx;
   ny = blockDiagram->ny;
   nz = blockDiagram->nz;
   start.x = blockDiagram->minYLoc; end.x = start.x + ny*blockSize;
   start.y = blockDiagram->minZLoc; end.y = start.y + nz*blockSize;
   start.z = blockDiagram->minXLoc; end.z = start.z + nx*blockSize;
   minX = blockDiagram->minXLoc;
   maxX = blockDiagram->minXLoc + nx*blockSize;
   minY = blockDiagram->minYLoc;
   maxY = blockDiagram->minYLoc + ny*blockSize;

                      /* *************************** */
                      /* Initialise the 3d libraries */   
   xvt_dwin_clear (threedData->pixmap, backgroundColor);

                      /* ************************************************** */
                      /* Setup the brushes and pens we will be drawing with */
   xvt_dwin_get_draw_ctools (threedData->pixmap, &tools);

   tools.pen.width = 0;          /* Draw the back (hidden) lines */
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_LTGRAY;
   xvt_dwin_set_draw_ctools (win, &tools);
   drawBlockDiagramFrame (start, end, threedData->camera, FALSE, TRUE, 
                          (int *) NULL, (int *) NULL, (int *) NULL);
      
                  /* Setup Brushes and pens for the drawing */
   tools.pen.width = 3;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_BLACK;
   tools.brush.color = COLOR_BLACK; 
   tools.brush.pat = PAT_SOLID;
   xvt_dwin_set_draw_ctools (threedData->pixmap, &tools);

                      /* ************************* */
                      /* Draw the actual BoreHoles */
   numHoles = blockDiagram->numBoreHoles;
   xyzLoc = (double ***) create3DArray (2, 2, 4, sizeof(double));
   histoire = (STORY **) create2DArray (2, 2, sizeof(STORY));

   for (hole = 0; hole < numHoles; hole++)
   {
      numPoints = blockDiagram->numPointInBoreHoles[hole];
      for (point = 0; point < numPoints; point++)
      {      
         xyzLoc[1][1][1] = boreHoleData[hole][point][0];
         xyzLoc[1][1][2] = boreHoleData[hole][point][1];
         xyzLoc[1][1][3] = boreHoleData[hole][point][2];
         histoire[1][1].again = TRUE;
         izero(histoire[1][1].sequence);

         reverseEvents (xyzLoc, histoire, 1, 1);
         taste(numEvents, histoire[1][1].sequence, &rockType, &eventIndex);

         if (layerProp = whichLayer(eventIndex, xyzLoc[1][1][1], xyzLoc[1][1][2], xyzLoc[1][1][3]))
         {
            tools.pen.color = XVT_MAKE_COLOR (layerProp->color.red, layerProp->color.green,
                                                                    layerProp->color.blue);
            xvt_dwin_set_draw_ctools (win, &tools);
            line3d[1].z = maxX - (boreHoleData[hole][point][0] - minX);
            line3d[1].x = boreHoleData[hole][point][1];
            line3d[1].y = boreHoleData[hole][point][2];
            if (numPoints == 1)
               draw3dPoint (win, line3d[1]);
            else if (point > 0)
               draw3dLine (win, line3d); 

            memcpy (&(line3d[0]), &(line3d[1]), sizeof(Point3d));
         }
      }
   }
   destroy3DArray ((char ***) xyzLoc, 2, 2, 4);
   destroy2DArray ((char **) histoire, 2, 2);
                           /* *************************************** */
                           /* Draw the border and direction indicator */
   tools.pen.width = 0;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_LTGRAY;
   xvt_dwin_set_draw_ctools (win, &tools);
   drawBlockDiagramFrame (start, end, threedData->camera, TRUE, FALSE,
                          (int *) NULL, (int *) NULL, (int *) NULL);

   xvt_vobj_get_client_rect (win, &border);
   directionPos.h = 35;
   directionPos.v = border.bottom - 35;
   drawDirectionIndicator (win, directionPos);

   return (TRUE);
}


/* ======================================================================
FUNCTION        drawBlockDiagramFrame
DESCRIPTION
     Draw the frame of the block diagram

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
drawBlockDiagramFrame (Point3d start, Point3d end, Point3d position,
               int front, int back, int *xInc, int *yInc, int *zInc)
#else
drawBlockDiagramFrame (start, end, position, front, back, xInc, yInc, zInc)
Point3d start, end, position;
int front, back;
int *xInc, *yInc, *zInc;
#endif
{
   Point3d vertex[8];
   Point3d *lines[12][2], lineMiddle, distVector;
   int furthestLine, closestLine;
   double lineDist[12], largestDist, smallestDist;
   double cornerDist[4];
   int count, line;
   
   if (xInc)        /* +ve - move away from block origin */
      *xInc = 0;     /* -ve - move towards block origin */
   if (yInc)
      *yInc = 0;
   if (zInc)
      *zInc = 0;
   
                   /* Eight vertexs that make the cube */
   count = 0;
   vertex[count].x = start.x;
   vertex[count].y = start.y;
   vertex[count].z = end.z;
   count++;   
   vertex[count].x = start.x;
   vertex[count].y = end.y;
   vertex[count].z = end.z;
   count++;   
   vertex[count].x = start.x;
   vertex[count].y = end.y;
   vertex[count].z = start.z;
   count++;   
   vertex[count].x = start.x;
   vertex[count].y = start.y;
   vertex[count].z = start.z;

   count++;   
   vertex[count].x = end.x;
   vertex[count].y = start.y;
   vertex[count].z = end.z;
   count++;   
   vertex[count].x = end.x;
   vertex[count].y = end.y;
   vertex[count].z = end.z;
   count++;   
   vertex[count].x = end.x;
   vertex[count].y = end.y;
   vertex[count].z = start.z;
   count++;   
   vertex[count].x = end.x;
   vertex[count].y = start.y;
   vertex[count].z = start.z;
   
   count = 0;
   lines[count][0] = &vertex[0];
   lines[count][1] = &vertex[1];
   count++;
   lines[count][0] = &vertex[1];
   lines[count][1] = &vertex[2];
   count++;
   lines[count][0] = &vertex[2];
   lines[count][1] = &vertex[3];
   count++;
   lines[count][0] = &vertex[3];
   lines[count][1] = &vertex[0];
   
   count++;
   lines[count][0] = &vertex[4];
   lines[count][1] = &vertex[5];
   count++;
   lines[count][0] = &vertex[5];
   lines[count][1] = &vertex[6];
   count++;
   lines[count][0] = &vertex[6];
   lines[count][1] = &vertex[7];
   count++;
   lines[count][0] = &vertex[7];
   lines[count][1] = &vertex[4];

   count++;
   lines[count][0] = &vertex[0];
   lines[count][1] = &vertex[4];
   count++;
   lines[count][0] = &vertex[1];
   lines[count][1] = &vertex[5];
   count++;
   lines[count][0] = &vertex[2];
   lines[count][1] = &vertex[6];
   count++;
   lines[count][0] = &vertex[3];
   lines[count][1] = &vertex[7];

   for (count = 0; count < 12; count++)   /* Work out the line distances */
   {
      avg(lines[count][0], lines[count][1], &lineMiddle);
      SubPt3d(&lineMiddle, &position, &distVector);
      lineDist[count] = fabs(distVector.x) + fabs(distVector.y) + fabs(distVector.z);
   }

   if (front)
   {
      for (count = 0; count < 9; count++)  /* 9 closest Lines */
      {
         closestLine = -1;
         for (line = 0; line < 12; line++)  /* Find Closest */
         {
            if (lines[line][0] && lines[line][1])
            {
               if ((closestLine == -1) || (lineDist[line] < smallestDist))
               {
                  closestLine = line;
                  smallestDist = lineDist[line];
               }
            }
         }
                                               /* Draw the line */
         if ((closestLine != -1) && lines[closestLine][0] && lines[closestLine][1])
         {
            xfDraw3dLine(lines[closestLine][0]->x, lines[closestLine][0]->y, lines[closestLine][0]->z,
                         lines[closestLine][1]->x, lines[closestLine][1]->y, lines[closestLine][1]->z, &xFormViewer);
            lines[closestLine][0] = NULL;  /* Make sure they cannot be chosen again */
            lines[closestLine][1] = NULL;
         }
      }
   }


   if (back)
   {
      for (count = 0; count < 3; count++)  /* 3 Furthest Lines */
      {
         furthestLine = -1;
         for (line = 0; line < 12; line++)  /* Find Furthest */
         {
            if (lines[line][0] && lines[line][1])
            {
               if ((furthestLine == -1) || (lineDist[line] > largestDist))
               {
                  furthestLine = line;
                  largestDist = lineDist[line];
               }
            }
         }
                                               /* Draw the line */
         if ((furthestLine != -1) && lines[furthestLine][0] && lines[furthestLine][1])
         {
            xfDraw3dLine(lines[furthestLine][0]->x, lines[furthestLine][0]->y, lines[furthestLine][0]->z,
                         lines[furthestLine][1]->x, lines[furthestLine][1]->y, lines[furthestLine][1]->z, &xFormViewer);
            lines[furthestLine][0] = NULL;  /* Make sure they cannot be chosen again */
            lines[furthestLine][1] = NULL;
         }
      }
   }


                              /* +ve - move away from block origin */
                              /* -ve - move towards block origin */
   if (xInc || yInc || zInc)  /* Which way to draw for z buffering */
   {
             /* get the Closest corner and make ordering dependent on that */
      SubPt3d(&(vertex[2]), &position, &distVector);
      cornerDist[0] = fabs(distVector.x) + fabs(distVector.y) + fabs(distVector.z);
      SubPt3d(&(vertex[6]), &position, &distVector);
      cornerDist[1] = fabs(distVector.x) + fabs(distVector.y) + fabs(distVector.z);
      SubPt3d(&(vertex[1]), &position, &distVector);
      cornerDist[2] = fabs(distVector.x) + fabs(distVector.y) + fabs(distVector.z);
      SubPt3d(&(vertex[5]), &position, &distVector);
      cornerDist[3] = fabs(distVector.x) + fabs(distVector.y) + fabs(distVector.z);

      smallestDist = cornerDist[0];  /* Find the closest */
      closestLine = 0;
      for (count = 1; count < 4; count++)
      {
         if (cornerDist[count] < smallestDist)
         {
            smallestDist = cornerDist[count];
            closestLine = count;
         } 
      }

      switch (closestLine)
      {
         case (0):  /* MinX, MinY, MaxZ */
            if (xInc)
               *xInc = -1;
            if (yInc)
               *yInc = -1;
            if (zInc)
               *zInc = -1;
            break;
         case (1):  /* MinX, MaxY, MaxZ */
            if (xInc)
               *xInc = -1;
            if (yInc)
               *yInc = 1;
            if (zInc)
               *zInc = -1;
            break;
         case (2):  /* MaxX, MinY, MaxZ */
            if (xInc)
               *xInc = 1;
            if (yInc)
               *yInc = -1;
            if (zInc)
               *zInc = -1;
            break;
         case (3):  /* MaxX, MaxY, MaxZ */
            if (xInc)
               *xInc = 1;
            if (yInc)
               *yInc = 1;
            if (zInc)
               *zInc = -1;
            break;
      }                 
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION        draw3dRect
DESCRIPTION
     Draw 3d Rect

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
draw3dRect (WINDOW win, Point3d Rect3D[4])
#else
draw3dRect (win, Rect3D)
WINDOW win;
Point3d Rect3D[4];
#endif
{
   Point2d Rect2D[4];
   PNT drawRect[4];
   register int i;
   
   for (i = 0; i < 4; i++)  /* transform to something drawable */
   {
      ApplyTransform(&(Rect3D[i]), &(Rect3D[i]), &xFormViewer);
      Project(&(Rect3D[i]), &(Rect2D[i]));
      drawRect[i].h = (short) Rect2D[i].x;
      drawRect[i].v = (short) Rect2D[i].y;
   }
   xvt_dwin_draw_polygon (win, drawRect, 4);
   
   return (TRUE);
}

/* ======================================================================
FUNCTION        draw3dLine
DESCRIPTION
     Draw 3d Line

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
draw3dLine (WINDOW win, Point3d origLine3D[2])
#else
draw3dLine (win, origLine3D)
WINDOW win;
Point3d origLine3D[2];
#endif
{
   Point3d line3D[2];
   Point2d line2D[2];
   PNT drawLine[2];
   register int i;
   
   memcpy (line3D, origLine3D, sizeof(Point3d)*2);
   for (i = 0; i < 2; i++)  /* transform to something drawable */
   {
      ApplyTransform(&(line3D[i]), &(line3D[i]), &xFormViewer);
      Project(&(line3D[i]), &(line2D[i]));
      drawLine[i].h = (short) line2D[i].x;
      drawLine[i].v = (short) line2D[i].y;
   }
   xvt_dwin_draw_polygon (win, drawLine, 2);
   
   return (TRUE);
}


/* ======================================================================
FUNCTION        draw3dPoint
DESCRIPTION
     Draw a 3d Point

INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
draw3dPoint (WINDOW win, Point3d point3d)
#else
draw3dPoint (win, point3d)
WINDOW win;
Point3d point3d;
#endif
{
   Point2d point2d;
   PNT point;
   
                 /* transform to something drawable */
   ApplyTransform(&point3d, &point3d, &xFormViewer);
   Project(&point3d, &point2d);
   point.h = (short) point2d.x;
   point.v = (short) point2d.y;

   xvt_dwin_draw_set_pos (win, point);
   point.h++;
   xvt_dwin_draw_line (win, point);
   
   return (TRUE);
}
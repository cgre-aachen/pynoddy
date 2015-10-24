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

#if XVT_CC_PROTO
extern int drawBlockSection(WINDOW, COLOR **, int, int, double, double, double, double,
          double, double, double, double, double, double, double, double, double);
#else
extern int drawBlockSection();
#endif

#if XVT_CC_PROTO
static void create3dTopoMap ();
static void create3dStratMap ();
static int drawPointInRange (double, double, double, double, double, double,
                            int *, Point3d *, int *);
static int drawPlungeLine(double, double, double, double, double,
                          double, double, Point3d *, int, int *);
int comparePoints (THREED_POINT_INFO *, THREED_POINT_INFO *, int);
int swapPoints (THREED_POINT_INFO *, THREED_POINT_INFO *);
void qSortPoints(THREED_POINT_INFO __huge *, int, int, int);
extern void allSurface(THREED_IMAGE_DATA *, STORY ***, double ***,
                       int, int, int, double, char *);
extern void ShadeXYZ(double [4][3], COLOR *, double, double, double);
extern void ShadeLambert(double [4][3], COLOR *);
int drawDirectionIndicator (WINDOW, PNT);
#else
static void create3dTopoMap ();
static void create3dStratMap ();
static int drawPointInRange ();
static int drawPlungeLine();
int comparePoints ();
int swapPoints ();
void qSortPoints();
extern void allSurface();
extern void ShadeXYZ();
extern void ShadeLambert();
int drawDirectionIndicator ();
#endif

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern double iscale; /* scaling factor for geology */
extern int TopoRow, TopoCol;
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE;
extern double minTopoValue, maxTopoValue;
extern double **topographyMap;
extern int subdivisions; /* 1,2, or 4 - for xfMatrix2Patch */
extern COLOR backgroundColor;
extern Fixed zoom; 
extern int batchExecution;

                 /* ************************* */
                 /* Globals used in this file */
#define VIEW_DISTANCE         100000.0
#define TOLERANCE             0.0001

Point3d *cameraPos;

/* ======================================================================
FUNCTION  setup3dDrawing
DESCRIPTION
     perform all the standard initilisation to draw a 3d image

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
void setup3dDrawing (threedData)
THREED_IMAGE_DATA *threedData;
{
   DRAW_CTOOLS tools;
   Point3d focus;
   double lens = 20.0;
   LtSource LS;
   pLighting pL;       /* lighting information structure */
   RendAttr RA;        /* working copy of rendering attributes */

   if (!(threedData->pixmap))
      return;
   
   focus.x = focus.y = focus.z = 0.0;
   focus.x = threedData->focus.x;
   focus.y = threedData->focus.y;
   focus.z = threedData->focus.z;

   xvt_dwin_get_draw_ctools (threedData->pixmap, &tools);
   tools.pen.width = 0;
   tools.pen.color = COLOR_BLUE;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.brush.color = COLOR_WHITE;
   tools.brush.pat = PAT_HOLLOW;
   xvt_dwin_set_draw_ctools (threedData->pixmap, &tools);

   set3dOutputWindow (threedData->pixmap);

   Init3d ();

   subdivisions = 1;        /* to keep the size down some */
   hither = 8.0;            /* near clipping plane close to the viewer */

   AimCamera (threedData->camera, focus, lens, 0.00);
   cameraPos = &(threedData->camera);

   /*
   ** setup rendering for the image
   */
   RA.R = 130;
   RA.G = 130;
   RA.B = 130;
   RA.frameColor = XVT_MAKE_COLOR (10, 10, 255);
   RA.lightSource = FALSE;
   RA.distanceEffect = FALSE;
   RA.ambientEffect = TRUE;
   RA.normalVisibility = TRUE;
   RA.framed = FALSE;
   RA.patterned = FALSE;
   RA.grayScaled = FALSE;
   RA.texture = TX_NONE;
   RA.diffusion = 32767;
   RA.specIndex = 40;
   RA.specularity = 16384;
   RA.ambience = 255;
   RA.absorption = 0;
   RA.translucence = 0;
   RA.refraction = 0;
   RA.transparency = 0;


   /*
   ** Setup the Lighting for the image
   */
                                     /* create a point light source */
   Set3dLtSource (&LS, 255, 255, 255, 0.250, -128.0, 16.0, -128.0);
                             /* set ambient light and store point source */
   pL = NewLighting(80, 80, 80, 0.90, 1, &LS);
                             /* transform its coordinates into the viewer's
                             ** (camera's) reference frame */
   TransformLighting(pL, &xFormViewer);
   xvt_mem_free ((char *) pL);

   /* xvt_dwin_clear (threedData->pixmap, COLOR_BLACK); */
}

/* ======================================================================
FUNCTION  init3dDrawing
DESCRIPTION
     perform all the standard initilisation to draw a 3d image
     same as setup3dDrawing but it has the xyz axis setup like 
     Noddy does and not confused

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
int init3dDrawing (threedData)
THREED_IMAGE_DATA *threedData;
{
   DRAW_CTOOLS tools;
   Point3d focus;
   double lens = 20.0;
   LtSource LS;
   pLighting pL;       /* lighting information structure */
   RendAttr RA;        /* working copy of rendering attributes */

   if (!(threedData->pixmap))
      return (FALSE);
   
   focus.x = focus.y = focus.z = 0.0;
   focus.x = threedData->focus.x;
   focus.y = threedData->focus.y;
   focus.z = threedData->focus.z;

   xvt_dwin_get_draw_ctools (threedData->pixmap, &tools);
   tools.pen.width = 0;
   tools.pen.color = COLOR_BLUE;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.brush.color = COLOR_WHITE;
   tools.brush.pat = PAT_HOLLOW;
   xvt_dwin_set_draw_ctools (threedData->pixmap, &tools);

   set3dOutputWindow (threedData->pixmap);

   Init3d ();

   subdivisions = 1;        /* to keep the size down some */
   hither = 8.0;            /* near clipping plane close to the viewer */

   AimCamera (threedData->camera, focus, lens, 0.00);
   cameraPos = &(threedData->camera);

   /*
   ** setup rendering for the image
   */
   RA.R = 130;
   RA.G = 130;
   RA.B = 130;
   RA.frameColor = XVT_MAKE_COLOR (10, 10, 255);
   RA.lightSource = FALSE;
   RA.distanceEffect = FALSE;
   RA.ambientEffect = TRUE;
   RA.normalVisibility = TRUE;
   RA.framed = FALSE;
   RA.patterned = FALSE;
   RA.grayScaled = FALSE;
   RA.texture = TX_NONE;
   RA.diffusion = 32767;
   RA.specIndex = 40;
   RA.specularity = 16384;
   RA.ambience = 255;
   RA.absorption = 0;
   RA.translucence = 0;
   RA.refraction = 0;
   RA.transparency = 0;


   /*
   ** Setup the Lighting for the image
   */
                                     /* create a point light source */
   Set3dLtSource (&LS, 255, 255, 255, 0.250, -128.0, 16.0, -128.0);
                             /* set ambient light and store point source */
   pL = NewLighting(80, 80, 80, 0.90, 1, &LS);
                             /* transform its coordinates into the viewer's
                             ** (camera's) reference frame */
   TransformLighting(pL, &xFormViewer);
   xvt_mem_free ((char *) pL);

   /* xvt_dwin_clear (threedData->pixmap, COLOR_BLACK); */
   return (TRUE);
}


/* ======================================================================
FUNCTION  setCameraPosition
DESCRIPTION
     set the camera position in XYZ with from an orientation in
     declinatin, azimuth, and scale.

INPUT
     Point3d *camera  - the final position set
     double declination, azimuth, scale;

OUTPUT

RETURNED
====================================================================== */
void setCameraPosition (threedData, declination, azimuth, scale)
THREED_IMAGE_DATA *threedData;
double declination, azimuth, scale;
{
   double angleFromPlan, rotation;
   
   if (declination < 0)
      declination = declination + 360;
   else if (declination > 360)
      declination = declination - 360;
      
   if (azimuth > 90.0)
      azimuth = 90.0;
   else if (azimuth < 0.0)
      azimuth = 0.0;
      
   threedData->declination = declination;
   threedData->azimuth = azimuth;
   threedData->scale = scale;
   
   angleFromPlan = 90.0 - azimuth;
   if (angleFromPlan < 0.0) angleFromPlan = 0.0;
   if (angleFromPlan > 90.0) angleFromPlan = 90.0;
   angleFromPlan = Rad(angleFromPlan);
   
   rotation = Rad(360.0 - declination);
   threedData->camera.x = VIEW_DISTANCE * sin(angleFromPlan) * cos(rotation);
   threedData->camera.z = VIEW_DISTANCE * sin(angleFromPlan) * sin(rotation);
   threedData->camera.y = VIEW_DISTANCE * cos(angleFromPlan);

   threedData->camera.x += threedData->focus.x;
   threedData->camera.z += threedData->focus.y;
   threedData->camera.y += threedData->focus.z;
}

void getCameraPosition (threedData, declination, azimuth, scale)
THREED_IMAGE_DATA *threedData;
double *declination, *azimuth, *scale;
{
   *scale = threedData->scale;
   *declination = threedData->declination;
   *azimuth = threedData->azimuth;
}

/* ======================================================================
FUNCTION  update3dSceen
DESCRIPTION
     redraw an already created sceen at the new aspect

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
void update3dSceen (threedData)
THREED_IMAGE_DATA *threedData;
{                            
   register XVT_PIXMAP pixmap;
   register THREED_POINT_INFO __huge *pointPtr;
   CPEN pen;
   register CPEN *penPtr;
   COLOR shading, lineShade;
   double conlist[4][3];
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   setup3dDrawing (threedData);
   xvt_dwin_clear (threedData->pixmap, backgroundColor);

   pen.width = 0;
   pen.pat = PAT_SOLID;
   pen.style = P_SOLID;
   penPtr = &pen;
   pixmap = threedData->pixmap;

   if (threedData->imageType == STRAT_3D)
   {   
      register int point, vertex, polySize = 3;
      register int numPoints, mm;
      Point3d point3D;
      Point2d point2D;
      PNT thePoly[4];
      DRAW_CTOOLS tools;

      parallel = TRUE;
      zoom = threedData->scale/3.0;

                           /* hardwired for black filled triangles */
      xvt_dwin_get_draw_ctools (pixmap, &tools);
      tools.pen.width = 0;
      tools.pen.pat = PAT_SOLID;
      tools.brush.color = backgroundColor; 
      tools.brush.pat = PAT_SOLID;
      xvt_dwin_set_draw_ctools (pixmap, &tools);

      if (!threedData->surface)
      {
         create3dStratMap (threedData, (char *) NULL);
         finishLongJob ();
      }
      else
      {
         pointPtr = threedData->surface;
         numPoints = (int) threedData->surfaceWidth;

         qSortPoints (pointPtr, 0, numPoints-3, 3);
         gridBase (threedData, &xFormViewer, viewOptions->lengthX, viewOptions->lengthY, viewOptions->lengthZ);

         for (point = 0; point < numPoints; point += 3)
         {               /* change color as needed */
            switch (threedViewOptions.fillType)
            {
               case XYZ_COLOR_FILL:
                  for (mm = 0; mm < 3; mm++)
                  {
                     conlist[mm][1] = pointPtr[mm].point.x;
                     conlist[mm][2] = pointPtr[mm].point.y;
                     conlist[mm][0] = pointPtr[mm].point.z;
                  }

                  ShadeXYZ(conlist, &shading, viewOptions->lengthX, viewOptions->lengthY, viewOptions->lengthZ);
                  lineShade= XVT_MAKE_COLOR(
                        (XVT_COLOR_GET_RED(shading)  +((255-XVT_COLOR_GET_RED(shading))/2)),
                        (XVT_COLOR_GET_GREEN(shading)+((255-XVT_COLOR_GET_GREEN(shading))/2)),
                        (XVT_COLOR_GET_BLUE(shading) +((255-XVT_COLOR_GET_BLUE(shading))/2)));
                  tools.pen.color = lineShade; 
                  tools.brush.color = shading; 
                  xvt_dwin_set_draw_ctools (pixmap, &tools);
                  break;
               case SHADING_FILL:
                  for (mm = 0; mm < 3; mm++)
                  {
                     conlist[mm][1] = pointPtr[mm].point.x;
                     conlist[mm][2] = pointPtr[mm].point.y;
                     conlist[mm][0] = pointPtr[mm].point.z;
                  }

                  ShadeLambert(conlist, &shading);
                  tools.pen.color = shading; 
                  tools.brush.color = shading; 
                  xvt_dwin_set_draw_ctools (pixmap, &tools);
                  break;
               default: /* LINES_FILL */
                  if (pen.color != pointPtr->color)
                  {
                     pen.color = pointPtr->color;
                     xvt_dwin_set_cpen (pixmap, penPtr);
                  }
            }
                        /* Put triangle in polygon so it can be shaded */
            for (vertex = 0; vertex < 3; vertex++)
            {
               point3D = pointPtr->point;
               ApplyTransform(&point3D, &point3D, &xFormViewer);
               Project(&point3D, &point2D);
               thePoly[vertex].h = (short) point2D.x;
               thePoly[vertex].v = (short) point2D.y;
               pointPtr++;
            }
            
            xvt_dwin_draw_polygon (pixmap, thePoly, polySize);
         }         
      }
   }
   else if (threedData->imageType == TOPO_3D)
   {   
      register int row, col, rowp1;
      register int width, height;

      parallel = TRUE;
      zoom = threedData->scale/3.0;

      pointPtr = threedData->surface;
      if (!pointPtr)
         return;

      width = (int) threedData->surfaceWidth;
      height = (int) threedData->surfaceHeight;
      for (row = 0; row < width ; row++)
      {
         rowp1 = row + 1;
         incrementLongJob (INCREMENT_JOB);
         vtMovetoPt (&(pointPtr->point));
         for (col = 0; col < height; col++)
         {
            pen.color = pointPtr->color;
            xvt_dwin_set_cpen (pixmap, penPtr);
            vtLinetoPt (&(pointPtr->point));
            if (rowp1 < width)
            {
               pen.color = (pointPtr+height)->color;
               xvt_dwin_set_cpen (pixmap, penPtr);
               vtLinetoPt (&((pointPtr+height)->point));
            }
            vtMovetoPt (&(pointPtr->point));
      
            pointPtr++;
         }     
      }
   }
}
/* ======================================================================
FUNCTION  preview3dSceen
DESCRIPTION
     preview an already created sceen at the new aspect

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image
     int level - preview level (1 - greatest)
     
OUTPUT

RETURNED
====================================================================== */
void preview3dSceen (threedData, previewLevel)
THREED_IMAGE_DATA *threedData;
int previewLevel;
{                            
   register XVT_PIXMAP pixmap;
   register THREED_POINT_INFO __huge *pointPtr;
   CPEN pen;
   register CPEN *penPtr;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   setup3dDrawing (threedData);
   xvt_dwin_clear (threedData->pixmap, backgroundColor);

   pen.width = 0;
   pen.pat = PAT_SOLID;
   pen.style = P_SOLID;
   penPtr = &pen;
   pixmap = threedData->pixmap;

   if (threedData->imageType == STRAT_3D)
   {   
      DRAW_CTOOLS tools;

      parallel = TRUE;
      zoom = threedData->scale/3.0;

      xvt_dwin_get_draw_ctools (pixmap, &tools);
      tools.pen.width = 0;
      tools.pen.pat = PAT_SOLID;
      tools.brush.color = COLOR_BLACK; 
      tools.brush.pat = PAT_SOLID;
      xvt_dwin_set_draw_ctools (pixmap, &tools);

      gridBase (threedData, &xFormViewer, viewOptions->lengthX, viewOptions->lengthY, viewOptions->lengthZ);
   }
   else if (threedData->imageType == TOPO_3D)
   {
      register int row, col, rowp1;
      register int width, height;

      parallel = TRUE;
      zoom = threedData->scale/3.0;

      pointPtr = threedData->surface;
      if (!pointPtr)
         return;

      pen.color = COLOR_LTGRAY;
      xvt_dwin_set_cpen (pixmap, penPtr);

      width = (int) threedData->surfaceWidth;
      height = (int) threedData->surfaceHeight;
      for (row = 0; row < width ; row++)
      {
         rowp1 = row + 1;
         vtMovetoPt (&(pointPtr->point));
         for (col = 0; col < height; col++)
         {
            if ((col == 0) || (col+1 == height)
             || (row == 0) || (rowp1 == width))
            {
               vtLinetoPt (&(pointPtr->point));

               if ((rowp1 < width) && (row != 0))
                  vtLinetoPt (&((pointPtr+height)->point));
            }
            vtMovetoPt (&(pointPtr->point));
      
            pointPtr++;
         }     
      }
   }
}

/* ======================================================================
FUNCTION  do3dTopoMap
DESCRIPTION
     draw the topography map using the passed in paramaters in 3d

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
void do3dTopoMap (threedData)
THREED_IMAGE_DATA *threedData;
{

   if (!(geologyOptions.useTopography && topographyMap))
   {
      if (batchExecution)
         fprintf (stderr, "Error, You must first load a topography from Geology Options");
      else
         xvt_dm_post_error("Error, You must first load a topography from Geology Options");
      return;
   }

   /*
   ** convert to a solid object
   */
   if ((!threedData->surface) || (threedData->surfaceWidth == 0)
                              || (threedData->surfaceHeight == 0))
   {
      initLongJob (0, TopoRow*6, "Calculating 3D Topography", NULL);
      create3dTopoMap (threedData);
   }
                               
   /*
   ** draw the surface
   */
   update3dSceen(threedData);

   finishLongJob ();
}
/* ======================================================================
FUNCTION  create3dTopoMap
DESCRIPTION
     create the topography map using the passed in paramaters in 3d

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
static void create3dTopoMap (threedData)
THREED_IMAGE_DATA *threedData;
{
   register THREED_POINT_INFO __huge *pointsPtr;
   register THREED_POINT_INFO __huge *points;
   OBJECT *object;
   NODDY_COLOUR *colorOptions;
   COLOR color;
   register double ***dots;
   register STORY **histoire;
   int nV;
   register int row, col;
   int index, numEvents = (int) countObjects(NULL_WIN);
   double blockSize;
   unsigned int flavor;
 
   setCurrentDrawingWindow (threedData->pixmap);
   threedData->imageType = TOPO_3D;
   
                                       /* array to hold point list */
   nV = TopoRow * TopoCol;
   blockSize = (TopomapXE - TopomapXW)/TopoCol;
   points = (THREED_POINT_INFO __huge *) xvt_mem_halloc(nV,sizeof(THREED_POINT_INFO));
   if (!points)
   {
      if (batchExecution)
         fprintf (stderr, "Error, Not enough memory for 3D-Topography");
      else
         xvt_dm_post_error("Error, Not enough memory for 3D-Topography");
      return;
   }
   pointsPtr = points;

   if ((dots = (double ***) qdtrimat(0,TopoRow,0,TopoCol,0,3))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Error, Not enough Memory");
      else
         xvt_dm_post_error("Error, Not enough Memory");
      xvt_mem_hfree ((char __huge *) points);
      return;
   }
   if ((histoire = (struct story **) strstomat(0,TopoRow,0,TopoCol))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         xvt_dm_post_error("Not enough memory, try closing some windows");
      freeqdtrimat(dots,0,TopoRow,0,TopoCol,0,3);
      xvt_mem_hfree ((char __huge *) points);
      return;
   }
   for (row = 0; row < TopoRow; row++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (col = 0; col < TopoCol; col++)
      {
         /* dots[row+1][TopoCol-col] */
         dots[row+1][TopoCol-col][1] = col*blockSize + TopomapXW;
         dots[row+1][TopoCol-col][2] = row*blockSize + TopomapYW;
         dots[row+1][TopoCol-col][3] = topographyMap[col+1][TopoRow-row]; /* topographyMap[col+1][row+1]; */

         histoire[row+1][col+1].again=1;
         izero(histoire[row+1][col+1].sequence);
      }
   }
   reverseEvents (dots, histoire, TopoRow, TopoCol);

   for (row = 0; row < TopoRow; row++)
   {
      incrementLongJob (INCREMENT_JOB);
      for (col = 0; col < TopoCol; col++)
      {
         pointsPtr->point.x = (double) (row - (TopoRow/2)) * 20;
/*         pointsPtr->point.y = (topographyMap[col+1][row+1] - minTopoValue) */
         pointsPtr->point.y = (topographyMap[TopoCol-col][TopoRow-row] - minTopoValue)
                                                  / (blockSize/20);
         pointsPtr->point.z = (double) (col - (TopoCol/2)) * 20;

/*         taste(numEvents, histoire[row+1][col+1].sequence, &flavor, &index); */
         taste(numEvents, histoire[row+1][col+1].sequence, &flavor, &index); /* TopoCol-col */
         if (flavor == IGNEOUS_STRAT)
         {
            if (!(object = (OBJECT *) nthObject (NULL_WIN, index)))
               object = (OBJECT *) nthObject (NULL_WIN, 0);
            if (object && (colorOptions = (NODDY_COLOUR *)
                                 getNoddyColourStructure (object)))
               color = XVT_MAKE_COLOR(colorOptions->red,
                             colorOptions->green, colorOptions->blue);
            else
               color = COLOR_BLACK;

            pointsPtr->color = color;
         }
         else
         {
            pointsPtr->color = (COLOR) which (-1.0, -1.0, index,
                                   dots[row+1][col+1][1],
                                   dots[row+1][col+1][2],
                                   dots[row+1][col+1][3], flavor);
         }
         
         pointsPtr++;
      }
   }
   threedData->surface = points;
   threedData->surfaceWidth = TopoRow;
   threedData->surfaceHeight = TopoCol;

   freeqdtrimat(dots,0,TopoRow,0,TopoCol,0,3);
   free_strstomat(histoire,0,TopoRow,0,TopoCol);

}

/* ======================================================================
FUNCTION  do3dStratMap
DESCRIPTION
     draw the stratigraphy using the passed in paramaters in 3d

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
void do3dStratMap (threedData, filename)
THREED_IMAGE_DATA *threedData;
char *filename;
{
   /*
   ** convert to a solid object
   */
   if (threedData)
   {
      setup3dDrawing (threedData);
      parallel = TRUE;
      zoom = threedData->scale/3.0; /* * DEFAULT_BLOCK_SCALING; */
      xvt_dwin_clear (threedData->pixmap, backgroundColor);
   }
      
   if ((!threedData) ||
       (!threedData->surface) || (threedData->surfaceWidth == 0)
                              || (threedData->surfaceHeight == 0))
   {
      create3dStratMap (threedData, filename);
   }
                               
   /*
   ** draw the surface
   */
   /* update3dSceen(threedData); */

   finishLongJob ();
}
/* ======================================================================
FUNCTION  create3dStartMap
DESCRIPTION
     create the Strat map using the passed in paramaters in 3d

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
static void create3dStratMap (threedData, filename)
THREED_IMAGE_DATA *threedData;
char *filename;
{
   register double ***dots, ***dots3D;
   register STORY ***histoire;
   register int x, y, z;
   int xMax, yMax, zMax;
   int numEvents = (int) countObjects(NULL_WIN);
   double height;
   double blockSize;
   double xLoc, yLoc, zLoc;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   int bbox=0;
   
   xLoc = viewOptions->originX;
   yLoc = viewOptions->originY;
   zLoc = viewOptions->originZ - viewOptions->lengthZ;

   if (threedData)
   {
      setCurrentDrawingWindow (threedData->pixmap);
      threedData->imageType = STRAT_3D;
      threedData->surfaceHeight = 0;
      threedData->surfaceWidth = 0;
   }
   
   blockSize = viewOptions->geologyCubeSize;

/*
   if (geologyOptions.cubeResolution == BLOCK_HIGH)
      blockSize = 200;
   else if (geologyOptions.cubeResolution == BLOCK_MEDHIGH)
      blockSize = 300;
   else if (geologyOptions.cubeResolution == BLOCK_MEDIUM)
      blockSize = 500;
   else if (geologyOptions.cubeResolution == BLOCK_MEDLOW)
      blockSize = 700;
   else if (geologyOptions.cubeResolution == BLOCK_LOW)
      blockSize = 1000;     
*/
      
/*
   xMax = (int) (BLOCK_DIAGRAM_SIZE_X / blockSize) + 1;
   yMax = (int) (BLOCK_DIAGRAM_SIZE_Y / blockSize) + 1;
   zMax = (int) (BLOCK_DIAGRAM_SIZE_Z / blockSize) + 1;
*/
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
      return;
   }
   if ((histoire = (STORY ***) qdtristrsmat(0,zMax,0,yMax,0,xMax))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Not enough memory, try closing some windows");
      else
         xvt_dm_post_error("Not enough memory, try closing some windows");
      freeqdtrimat(dots,0,zMax,0,yMax,0,xMax);
      return;
   }
   if ((dots3D = (double ***) qdtrimat(0,zMax,0,yMax,0,xMax))==0L)
   {
      if (batchExecution)
         fprintf (stderr, "Error, Not enough Memory");
      else
         xvt_dm_post_error("Error, Not enough Memory");
      freeqdtrimat(dots,0,zMax,0,yMax,0,xMax);
      free_qdtristrsmat(histoire,0,zMax,0,yMax,0,xMax);
      return;
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

			for (x = 0; x < xMax; x++)
				if(( x==0 || y==0 || z==0 || x==xMax-1 || y==yMax-1 || z==zMax -1 ) && bbox==1)
				{
					dots[y+1][x+1][3] = -99999999999999999999999999.0; /* to provide bounding box triangles mwj_hack*/
					histoire[z+1][y+1][x+1].again=1;
					histoire[z+1][y+1][x+1].sequence[0]=(unsigned char)3;
				}
 

      for (y = 0; y < yMax; y++)
      {
         for (x = 0; x < xMax; x++)
         {
            dots3D[z+1][y+1][x+1] = dots[y+1][x+1][3]; /* Z location */
         }
      }
   }
   freeqdtrimat(dots,0,yMax,0,xMax,0,3);

           /* Calculate the surface with correct joins */
   gridBase (threedData, &xFormViewer, viewOptions->lengthX, viewOptions->lengthY, viewOptions->lengthZ);
   allSurface(threedData, histoire, dots3D,
              xMax, yMax, zMax, blockSize, filename);


   free_qdtristrsmat(histoire,0,zMax,0,yMax,0,xMax);
   freeqdtrimat(dots3D,0,zMax,0,yMax,0,xMax);
}

/* ======================================================================
FUNCTION  do3dPreview
DESCRIPTION
     draw the preview using the passed in paramaters in 3d

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
int do3dPreview (threedData, blockView, object,
                 posX, posY, posZ, dip, dipDir, pitch, drawPitch)
THREED_IMAGE_DATA *threedData;
BLOCK_VIEW_OPTIONS *blockView;
OBJECT *object;
double posX, posY, posZ;
double dip, dipDir, pitch;
int drawPitch;
{
   Point3d start, end, step;
   Point3d pointArray[20];
   int firstPointInFace[7], face;
   int numPoints = 0;
   GridOptions options;
   DRAW_CTOOLS tools;
   double posScale, coeffA, coeffB, coeffC, coeffD, alpha, beta, gamma;
   double newPosX, newPosY, newPosZ;
   double x, y, z, termA, termB, termC;
   int firstPoint = TRUE;
   RCT border;
   PNT directionPos;

   setup3dDrawing (threedData);
   parallel = TRUE;
   zoom = threedData->scale;

   xvt_dwin_clear (threedData->pixmap, backgroundColor);

                                 /* Set the drawing paramaters */
   xvt_dwin_get_draw_ctools (threedData->pixmap, &tools);
   tools.pen.width = 0;
   tools.pen.pat = PAT_SOLID;
   tools.pen.color = COLOR_WHITE;
   tools.brush.color = COLOR_BLACK; 
   tools.brush.pat = PAT_SOLID;
   xvt_dwin_set_draw_ctools (threedData->pixmap, &tools);
   
                                 /* Draw a bounding box */
   options.left = options.right = options.top = TRUE;
   options.bottom = options.back = options.front = TRUE;
   options.gridPat = GP_LTGRAY;
   options.connectX = options.connectY = options.connectZ = FALSE;
   options.connectPat = GP_LTGRAY;

   if ((blockView->lengthX < 1) || (blockView->lengthY < 1) || (blockView->lengthZ < 1))
      return (FALSE);

   start.x = -(blockView->lengthY/1000.0)*15.0; end.x = -start.x;  step.x = 1;
   start.y = -(blockView->lengthZ/1000.0)*15.0; end.y = -start.y;  step.y = 1;
   start.z = -(blockView->lengthX/1000.0)*15.0; end.z = -start.z;  step.z = 1;

   
   xfGrid (start, end, step, options, &xFormViewer);
   
                      /* Draw an arrow pointing North */   
   xvt_vobj_get_client_rect (threedData->pixmap, &border);
   directionPos.h = 35;
   directionPos.v = border.bottom - 35;
   drawDirectionIndicator (threedData->pixmap, directionPos);

   if (!object)
      return (TRUE);
      
                    /* Draw plane with correct dip, dipDir, pitch */
                    /* First scale the positions to fit the diagram */
   if (posX == blockView->originX) posX += 1.0;
   if (posX == blockView->originX+blockView->lengthX) posX -= 1.0;

   if (posY == blockView->originY) posY += 1.0;
   if (posY == blockView->originY+blockView->lengthY) posY -= 1.0;

   if (posZ == blockView->originZ) posZ -= 1.0;
   if (posZ == blockView->originZ-blockView->lengthZ) posZ += 1.0;
   
   posScale = (end.z - start.z)/blockView->lengthX;
   newPosX = (end.z - posX*posScale)   + blockView->originX*posScale;
   newPosY = (posY*posScale + start.x) - blockView->originY*posScale;
   newPosZ = (posZ*posScale + start.y) + blockView->lengthZ*posScale - blockView->originZ*posScale;
                    /* Calculate the coeff for the plane */
   convrt(dip, 360.0-dipDir, 90.0-pitch, (BOOLEAN) drawPitch);
   plane(&coeffA, &coeffB, &coeffC, &coeffD, newPosX, newPosY, newPosZ);
   if (drawPitch)
      getAlphaBetaGamma (&alpha, &beta, &gamma);
      
   if (fabs(coeffA) < TOLERANCE) coeffA = TOLERANCE;
   if (fabs(coeffB) < TOLERANCE) coeffB = TOLERANCE;
   if (fabs(coeffC) < TOLERANCE) coeffC = TOLERANCE;
   
   
              /*  0 = coeffA*x + coeffB*y + coeffC*z + coeffD */
   tools.pen.width = 0;
   tools.pen.color = COLOR_YELLOW;
   xvt_dwin_set_draw_ctools (threedData->pixmap, &tools);
   face = 0;

   firstPoint = TRUE;
   firstPointInFace[face++] = numPoints;
   x = start.z;  termA = coeffA*x;   /* Face at x = start.x */
   y = start.x;  termB = coeffB*y;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);
   z = end.y;    termC = coeffC*z;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   y = end.x;    termB = coeffB*y;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);
   z = start.y;  termC = coeffC*z;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   y = start.x;  termB = coeffB*y;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);


   firstPoint = TRUE;
   firstPointInFace[face++] = numPoints;
   x = end.z;    termA = coeffA*x;   /* Face at x = end.x */
   y = start.x;  termB = coeffB*y;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);
   z = end.y;    termC = coeffC*z;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   y = end.x;    termB = coeffB*y;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);
   z = start.y;  termC = coeffC*z;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   y = start.x;  termB = coeffB*y;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);


   firstPoint = TRUE;
   firstPointInFace[face++] = numPoints;
   y = end.x;    termB = coeffB*y;   /* Face at y = end.y */
   x = start.z;  termA = coeffA*x;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);
   z = end.y;    termC = coeffC*z;
   x = -((termB + termC + coeffD)/coeffA);
   drawPointInRange (x, x, y, z, start.z, end.z, &firstPoint,
                                                     pointArray, &numPoints);
   x = end.z;    termA = coeffA*x;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);
   z = start.y;  termC = coeffC*z;
   x = -((termB + termC + coeffD)/coeffA);
   drawPointInRange (x, x, y, z, start.z, end.z, &firstPoint,
                                                     pointArray, &numPoints);
   x = start.z;  termA = coeffA*x;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);


   firstPoint = TRUE;
   firstPointInFace[face++] = numPoints;
   y = start.x;  termB = coeffB*y;   /* Face at y = start.y */
   x = start.z;  termA = coeffA*x;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);
   z = end.y;    termC = coeffC*z;
   x = -((termB + termC + coeffD)/coeffA);
   drawPointInRange (x, x, y, z, start.z, end.z, &firstPoint,
                                                     pointArray, &numPoints);
   x = end.z;    termA = coeffA*x;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);
   z = start.y;  termC = coeffC*z;
   x = -((termB + termC + coeffD)/coeffA);
   drawPointInRange (x, x, y, z, start.z, end.z, &firstPoint,
                                                     pointArray, &numPoints);
   x = start.z;  termA = coeffA*x;
   z = -((termA + termB + coeffD)/coeffC);
   drawPointInRange (z, x, y, z, start.y, end.y, &firstPoint,
                                                     pointArray, &numPoints);


   firstPoint = TRUE;
   firstPointInFace[face++] = numPoints;
   z = start.y;  termC = coeffC*z;   /* Face at z = start.z */
   x = start.z;  termA = coeffA*x;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   y = end.x;    termB = coeffB*y;
   x = -((termB + termC + coeffD)/coeffA);
   drawPointInRange (x, x, y, z, start.z, end.z, &firstPoint,
                                                     pointArray, &numPoints);
   x = end.z;    termA = coeffA*x;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   y = start.x;  termB = coeffB*y;
   x = -((termB + termC + coeffD)/coeffA);
   drawPointInRange (x, x, y, z, start.z, end.z, &firstPoint,
                                                     pointArray, &numPoints);
   x = start.z;  termA = coeffA*x;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   

   firstPoint = TRUE;
   firstPointInFace[face++] = numPoints;
   z = end.y;    termC = coeffC*z;   /* Face at z = end.z */
   x = start.z;  termA = coeffA*x;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   y = end.x;    termB = coeffB*y;
   x = -((termB + termC + coeffD)/coeffA);
   drawPointInRange (x, x, y, z, start.z, end.z, &firstPoint,
                                                     pointArray, &numPoints);
   x = end.z;    termA = coeffA*x;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   y = start.x;  termB = coeffB*y;
   x = -((termB + termC + coeffD)/coeffA);
   drawPointInRange (x, x, y, z, start.z, end.z, &firstPoint,
                                                     pointArray, &numPoints);
   x = start.z;  termA = coeffA*x;
   y = -((termA + termC + coeffD)/coeffB);
   drawPointInRange (y, x, y, z, start.x, end.x, &firstPoint,
                                                     pointArray, &numPoints);
   firstPointInFace[face] = numPoints;

                           /* Draw a line showing the pitch in the plane */
   if (drawPitch)
   {
      tools.pen.color = COLOR_RED;
      xvt_dwin_set_draw_ctools (threedData->pixmap, &tools);

      drawPlungeLine(coeffA, coeffB, coeffC, coeffD,
                     alpha, beta, gamma, pointArray,
                     numPoints, firstPointInFace);
   }


/*  Old Arrow Drawing
   xfMoveto3d (start.x, start.y,
               end.z+40, &xFormViewer);
   xfLine3d(100.0, 0.0,   0.0, &xFormViewer);
   xfLine3d(-40.0, 0.0,  15.0, &xFormViewer);
   xfLine3d(  0.0, 0.0, -30.0, &xFormViewer);
   xfLine3d( 40.0, 0.0,  15.0, &xFormViewer);
*/                                                     
   return (TRUE);      
}

static int drawPointInRange (value, x, y, z, startRange, endRange, firstPoint,
                                                         pointArray, numPoints)
double value, x, y, z, startRange, endRange;
int *firstPoint;
Point3d *pointArray;
int *numPoints;
{
   if ((value >= startRange) && (value <= endRange))
   {
      if (*firstPoint)
      {
         xfMoveto3d (y, z, x, &xFormViewer);
         *firstPoint = FALSE;
      }
      else
         xfLineto3d (y, z, x, &xFormViewer);
         
      pointArray[*numPoints].x = x;
      pointArray[*numPoints].y = y;
      pointArray[*numPoints].z = z;
      (*numPoints)++;
   }
         
   return (TRUE);
}

static int drawPlungeLine(coeffAPlane, coeffBPlane,
                 coeffCPlane, coeffDPlane, coeffA, coeffB, coeffC,
                 pointArray, numPoints, firstPointInFace)
double coeffAPlane, coeffBPlane, coeffCPlane, coeffDPlane;
double coeffA, coeffB, coeffC;
Point3d *pointArray;
int numPoints, *firstPointInFace;
{
   double coeffD;
   double xDiff, yDiff, zDiff, xInc, yInc, zInc;
   double dist1, dist2, dist, result1, result2;
   double percentMove, x, y, z;
   int point, point1, point2, line, numLines;
   Point3d borderPoint, planePosPoint, *pointPtr;
   int face;
   
   if (numPoints < 4)
      return (FALSE);
    
   point1 = 0;
   dist2 = 0;
   pointPtr = pointArray;  /* the point the furest away from this one */
   x = pointPtr->x; y = pointPtr->y; z = pointPtr->z;
   coeffD = -((coeffA*x) + (coeffB*y) + (coeffC*z));
   point1 = point2 = 0;
   dist1 = dist2 = 0;
   for (pointPtr++, point = 1; point < numPoints; point++, pointPtr++)
   {           /* dont need sqrt as only interested in ratios */
      dist = coeffA*pointPtr->x + coeffB*pointPtr->y
                                + coeffC*pointPtr->z + coeffD;
      if (dist < dist1)
      {
         dist1 = dist;
         point1 = point;
      }
      else if (dist > dist2)
      {
         dist2 = dist;
         point2 = point;
      }
   }
   
   dist = dist2 - dist1;
   numLines = (int) ((dist / 30) - 1);
   if (numLines == 0) numLines = 1;
   xDiff = pointArray[point2].x - pointArray[point1].x;
   yDiff = pointArray[point2].y - pointArray[point1].y;
   zDiff = pointArray[point2].z - pointArray[point1].z;
   xInc = xDiff / (numLines+1);
   yInc = yDiff / (numLines+1);
   zInc = zDiff / (numLines+1);
      
   planePosPoint = pointArray[point1];
   for (line = 0; line < numLines; line++)
   {   
      planePosPoint.x += xInc;
      planePosPoint.y += yInc;
      planePosPoint.z += zInc;
      coeffD = -((coeffA*planePosPoint.x) + (coeffB*planePosPoint.y)
                                          + (coeffC*planePosPoint.z));
   
      for (face = 0; face < 6; face++)
      {                      /* there is a line on this face */
         if (firstPointInFace[face] != firstPointInFace[face+1])
         {
            point1 = firstPointInFace[face];
            result1 = coeffA*pointArray[point1].x + coeffB*pointArray[point1].y
                                                  + coeffC*pointArray[point1].z + coeffD;
            point2 = point1 + 1;
            result2 = coeffA*pointArray[point2].x + coeffB*pointArray[point2].y
                                                  + coeffC*pointArray[point2].z + coeffD;
            if (result1 == 0.0)  /* point 1 on pitch Plane */
            {
               borderPoint = pointArray[point1];
               xfDraw3dLine(planePosPoint.y, planePosPoint.z, planePosPoint.x,
                            borderPoint.y, borderPoint.z, borderPoint.x, &xFormViewer);
            }
            else if (result2 == 0.0)  /* point 2 on pitch Plane */
            {
               borderPoint = pointArray[point2];
               xfDraw3dLine(planePosPoint.y, planePosPoint.z, planePosPoint.x,
                            borderPoint.y, borderPoint.z, borderPoint.x, &xFormViewer);
            }                     /* either side of Pitch Plane */
            else if (((result1 < 0.0) && (result2 > 0.0))
                          || ((result1 > 0.0) && (result2 < 0.0)))
            {
               xDiff = pointArray[point2].x - pointArray[point1].x;
               yDiff = pointArray[point2].y - pointArray[point1].y;
               zDiff = pointArray[point2].z - pointArray[point1].z;

               dist1 = fabs(result1)/sqrt(coeffA*coeffA + coeffB*coeffB + coeffC*coeffC);
               dist2 = fabs(result2)/sqrt(coeffA*coeffA + coeffB*coeffB + coeffC*coeffC);
               percentMove = dist1/(dist1+dist2);

               borderPoint.x = pointArray[point1].x + percentMove*xDiff;
               borderPoint.y = pointArray[point1].y + percentMove*yDiff;
               borderPoint.z = pointArray[point1].z + percentMove*zDiff;
            
               xfDraw3dLine(planePosPoint.y, planePosPoint.z, planePosPoint.x,
                            borderPoint.y, borderPoint.z, borderPoint.x, &xFormViewer);
            }
         }
      }
   }

   return (TRUE);
}


/* ======================================================================
FUNCTION  gridBase
DESCRIPTION
     draw a grid on the base

INPUT
     THREED_IMAGE_DATA *threedData - all we need to describe the image

OUTPUT

RETURNED
====================================================================== */
void gridBase (threedData, xf, lengthX, lengthY, lengthZ)
THREED_IMAGE_DATA *threedData;
Matrix3D *xf;
double lengthX, lengthY, lengthZ;
{
   Point3d start;
   Point3d end;
   Point3d step;
   GridOptions options;
   
   if (!threedData)
      return;

   if (threedData->imageType == STRAT_3D)
   {
/*
      start.x = -350.0; end.x = 350.0;
      start.y = -250.0; end.y = 250.0;
      start.z = -500.0; end.z = 500.0;
*/
      start.x = -(lengthY/10)/2.0; end.x = -start.x;
      start.y = -(lengthZ/10)/2.0; end.y = -start.y;
      start.z = -(lengthX/10)/2.0; end.z = -start.z;

      step.x = lengthY/1000.0;
      step.y = lengthZ/1000.0;
      step.z = lengthX/1000.0;

      if ((threedData->declination > 90)
                               && (threedData->declination < 270))
      {
         options.left = FALSE;
         options.right = TRUE;
      }
      else
      {
         options.left = TRUE;
         options.right = FALSE;
      }
      if (threedData->declination < 180)
      {
         options.back = TRUE;
         options.front = FALSE;
      }
      else
      {
         options.back = FALSE;
         options.front = TRUE;
      }
      options.top = FALSE;
      options.bottom = TRUE;
      options.gridPat = GP_LTGRAY;
      options.connectX = FALSE;
      options.connectY = FALSE;
      options.connectZ = FALSE;
      options.connectPat = GP_LTGRAY;
   }
   else
   {
      start.x = -((TopoRow/2) * 20);  end.x = start.x + (TopoRow*20);
      start.y = 0.0;                  end.y = 300.0;
      start.z =  -((TopoCol/2) * 20); end.z = start.z + (TopoCol*20);

      step.x = 100.0;
      step.y = 50.0;
      step.z = 400.0;

      options.left = FALSE;
      options.right = TRUE;
      options.top = FALSE;
      options.bottom = TRUE;
      options.back = TRUE;
      options.front = FALSE;
      options.gridPat = GP_LTGRAY;
      options.connectX = FALSE;
      options.connectY = FALSE;
      options.connectZ = FALSE;
      options.connectPat = GP_LTGRAY;
   }
   xfGrid (start, end, step, options, xf);
}


/* ======================================================================
FUNCTION  updateBlockDiagramPoints
DESCRIPTION
     Update the list of 3D Block diagram points that cover the surfaces defined
     over the surfaces

INPUT
     BLOCK_DIAGRAM_DATA *blockDiagramData;

OUTPUT

RETURNED    TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
updateBlockDiagramPoints (BLOCK_DIAGRAM_DATA *blockDiagramData)
#else
updateBlockDiagramPoints (blockDiagramData)
BLOCK_DIAGRAM_DATA *blockDiagramData;
#endif
{
   BLOCK_SURFACE_DATA *surface;
   int x, y, z, xOff, yOff, zOff, nx, ny, nz, newSize;
   THREED_POINT_INFO __huge *newPoints, *point;
   THREED_IMAGE_DATA *threedDataPtr;
   COLOR ***blockData;
   double xPos, yPos, zPos, xStart, yStart, zStart, xEnd, yEnd, zEnd;
   double xInc, yInc, zInc, halfXInc, halfYInc, halfZInc;
   double blockSize = blockDiagramData->blockSize;
   int index, surfSize;

   blockData = blockDiagramData->blockData;
   threedDataPtr = &(blockDiagramData->threedData);
   if (threedDataPtr->surface)
   {
      xvt_mem_hfree ((char __huge *) threedDataPtr->surface);
      threedDataPtr->surface = (THREED_POINT_INFO __huge *) NULL;
      threedDataPtr->surfaceWidth = 0;
      threedDataPtr->surfaceHeight = 0;
   }


                      /* Add each surface to the list of 3d points that define
                      ** the center of each cube on the surface */
   for (surface = blockDiagramData->surfaces; surface; surface = surface->next)
   {
      xStart = MIN(surface->xStart, surface->xEnd);
      yStart = MIN(surface->yStart, surface->yEnd);
      zStart = MIN(surface->zStart, surface->zEnd);
      xEnd   = MAX(surface->xStart, surface->xEnd);
      yEnd   = MAX(surface->yStart, surface->yEnd);
      zEnd   = MAX(surface->zStart, surface->zEnd);
      xInc = yInc = zInc = blockSize;
      nx = (int) floor ((xEnd - xStart)/blockSize + 0.5);
      ny = (int) floor ((yEnd - yStart)/blockSize + 0.5);
      nz = (int) floor ((zEnd - zStart)/blockSize + 0.5);
      if (!nx) nx = 1;
      if (!ny) ny = 1;
      if (!nz) nz = 1;
      xInc = (xEnd - xStart)/((double) nx); halfXInc = xInc/2.0;
      yInc = (yEnd - yStart)/((double) ny); halfYInc = yInc/2.0;
      zInc = (zEnd - zStart)/((double) nz); halfZInc = zInc/2.0;
      
      index = (int) threedDataPtr->surfaceWidth;
      surfSize = nx*ny*nz*2;
      if (index + surfSize > threedDataPtr->surfaceHeight)
      {                             /* Expand the array if needed */
         newSize = (int) threedDataPtr->surfaceHeight + surfSize; /* the next hunk of memory */
         newPoints = (THREED_POINT_INFO __huge *) xvt_mem_halloc(newSize, sizeof(THREED_POINT_INFO));
         if (newPoints)
         {
            if (threedDataPtr->surface)
            {
               hmemcpy (newPoints, threedDataPtr->surface,
                        threedDataPtr->surfaceHeight*sizeof(THREED_POINT_INFO));
               xvt_mem_hfree ((char __huge *) threedDataPtr->surface);
            }
            threedDataPtr->surfaceHeight = newSize;
            threedDataPtr->surface = newPoints;
         }
         else   /* make sure we dont try and fill it */
         {
            if (threedDataPtr->surface)
            {
               xvt_mem_hfree ((char __huge *) threedDataPtr->surface);
               threedDataPtr->surface = NULL;
               threedDataPtr->surfaceWidth = 0;
               threedDataPtr->surfaceHeight = 0;
               return (FALSE);
            }
         }
      }

      xOff = (int) floor((xStart - blockDiagramData->minXLoc)/blockSize + 0.5);
      yOff = (int) floor((yStart - blockDiagramData->minYLoc)/blockSize + 0.5);
      zOff = (int) floor((zStart - blockDiagramData->minZLoc)/blockSize + 0.5);

      if (xOff >= blockDiagramData->nx) xOff = blockDiagramData->nx-1;
      else if (xOff < 0) xOff = 0;
      if (yOff >= blockDiagramData->ny) yOff = blockDiagramData->ny-1;
      else if (yOff < 0) yOff = 0;
      if (zOff >= blockDiagramData->nz) zOff = blockDiagramData->nz-1;
      else if (zOff < 0) zOff = 0;

      for (z = 0, zPos = zStart; z < nz; z++, zPos += zInc)
      {
         for (x = 0, xPos = xStart; x < nx; x++, xPos += xInc)
         {
            for (y = 0, yPos = yStart; y < ny; y++, yPos += yInc)
            {
               point = &(threedDataPtr->surface[index++]);
               point->point.x = xPos;
               point->point.y = yPos;
               point->point.z = zPos;
               point->color = blockData[z+zOff][x+xOff][y+yOff];

               point = &(threedDataPtr->surface[index++]);
               point->point.x = halfXInc;
               point->point.y = halfYInc;
               point->point.z = halfZInc;
               point->color = blockData[z+zOff][x+xOff][y+yOff];
            }
         }
      }
      threedDataPtr->surfaceWidth = index;  /* keep how many points we have */
   }

   return (TRUE);
}


/* ======================================================================
FUNCTION  comparePoints
DESCRIPTION
     Compare the average of a number of points and see which average
     is greater

INPUT
     BLOCK_DIAGRAM_DATA *blockDiagramData;

OUTPUT

RETURNED    1 - greater, 0 - equal, -1 - less than
====================================================================== */
int
#if XVT_CC_PROTO
comparePoints (THREED_POINT_INFO *point1, THREED_POINT_INFO *point2, int increment)
#else
comparePoints (point1, point2, increment)
THREED_POINT_INFO *point1, *point2;
int increment;
#endif
{
   register int i;
   double mag1, mag2;
   Point3d midA, midB, pointA, pointB;

             /* only use first point when conparing mid + blocksize (2 points) */   
   if (increment == 2)
      increment == 1;

   midA.x = midA.y = midA.z = 0.0;
   midB.x = midB.y = midB.z = 0.0;
   for (i = 0; i < increment; i++)
   {
      midA.x = midA.x + point1[i].point.x;
      midA.y = midA.y + point1[i].point.y;
      midA.z = midA.z + point1[i].point.z;
      midB.x = midB.x + point2[i].point.x;
      midB.y = midB.y + point2[i].point.y;
      midB.z = midB.z + point2[i].point.z;
   }
   midA.x = midA.x / increment;
   midA.y = midA.y / increment;
   midA.z = midA.z / increment;
   midB.x = midB.x / increment;
   midB.y = midB.y / increment;
   midB.z = midB.z / increment;
   
   
   SubPt3d (cameraPos, &midA, &pointA);
   SubPt3d (cameraPos, &midB, &pointB);

   mag1 = (pointA.x*pointA.x) + (pointA.y*pointA.y)
                              + (pointA.z*pointA.z);
   mag2 = (pointB.x*pointB.x) + (pointB.y*pointB.y)
                              + (pointB.z*pointB.z);
               
   if (mag1 < mag2)
      return (1);
   else if (mag1 > mag2)
      return (-1) ;
   else 
      return 0;
}

/* ======================================================================
FUNCTION  swapPoints
DESCRIPTION
     Swap two 3d Points

INPUT

OUTPUT

RETURNED    TRUE - sucess
====================================================================== */
int
#if XVT_CC_PROTO
swapPoints (THREED_POINT_INFO *point1, THREED_POINT_INFO *point2)
#else
swapPoints (point1, point2)
THREED_POINT_INFO *point1, *point2;
#endif
{
   THREED_POINT_INFO tempPoint;

   memcpy (&tempPoint, point1, sizeof(THREED_POINT_INFO));
   memcpy (point1, point2, sizeof (THREED_POINT_INFO));
   memcpy (point2, &tempPoint, sizeof (THREED_POINT_INFO));
   
   return (TRUE);
}


/* ======================================================================
FUNCTION  qSortPoints
DESCRIPTION
     Quick sort algorithm over a number of 3d points

INPUT

OUTPUT

RETURNED 
====================================================================== */
void
#if XVT_CC_PROTO
qSortPoints(THREED_POINT_INFO __huge *points,
            int first, int last, int increment)
#else
qSortPoints(points, first, last, increment)
THREED_POINT_INFO __huge *points;
int first, last, increment;
#endif
{
   int i;
   int j;
 
   while (last - first > 1)
   {
      i = first;
      j = last;
      for (;;)
      {
         i += increment;
         while ((i < last) &&
                     comparePoints(&(points[i]), &(points[first]), increment) < 0)
         {
            i += increment;
         }

         j -= increment;
         while ((j > first) &&
                     comparePoints(&(points[j]), &(points[first]), increment) > 0)
         {
            j -= increment;
         }
         
         if (i >= j)
            break;
            
         {
            int temp;
            
            for (temp = 0; temp < increment; temp++)
               swapPoints(&(points[i+temp]), &(points[j+temp]));
         }
      }
      if (j == first)
      {
         first += increment;
         continue;
      }

      {
         int temp;
            
         for (temp = 0; temp < increment; temp++)
            swapPoints(&(points[first+temp]), &(points[j+temp]));
      }
      if (j - first < last - (j + increment))
      {
         qSortPoints(points, first, j, increment);
         first = j + increment;  /*  qsort1(j + 1, last);  */
      }
      else
      {
         qSortPoints(points, j + increment, last, increment);
         last = j;      /*  qsort1(first, j);  */
      }
   }
}

/* ======================================================================
FUNCTION  sortBlockSurfaces
DESCRIPTION
     sort the surfaces that describe a block diagram so that the
     furthest comes first and the closest last

INPUT

OUTPUT

RETURNED TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
sortBlockSurfaces (BLOCK_DIAGRAM_DATA *blockDiagram)
#else
sortBlockSurfaces (blockDiagram)
BLOCK_DIAGRAM_DATA *blockDiagram;
#endif
{
   BLOCK_SURFACE_DATA *tempSurface, *surface,
                      *prevSurface, *firstSurface,
                      *prevFirstSurface;
   Point3d avgPt, *camera, distPt;
   double dist, longestDist;
   double blockXStart, blockXEnd;
   double depth;

   camera = &(blockDiagram->threedData.camera);
   blockXStart = blockDiagram->minXLoc;
   blockXEnd = blockDiagram->nx*blockDiagram->blockSize
                                            + blockXStart;

   prevFirstSurface = NULL;
   for (firstSurface = blockDiagram->surfaces; firstSurface; firstSurface = firstSurface->next)
   {
      longestDist = 0.0;
      prevSurface = NULL;
      for (surface = firstSurface; surface; surface = surface->next)
      {
         depth = (double) surface->depth;
/*         xStart = blockXStart + (blockXEnd - surface->xStart);
**         xEnd   = blockXStart + (blockXEnd - surface->xEnd); */
         avgPt.x = (surface->yStart + surface->yEnd) / 2.0;
         avgPt.y = (surface->zStart + surface->zEnd) / 2.0;
         avgPt.z = (surface->xStart + surface->xEnd) / 2.0;
/*         avgPt.z = (xStart + xEnd) / 2.0; */

         distPt.x = (camera->x - avgPt.x)/1000.0;
         distPt.y = (camera->y - avgPt.y)/1000.0;
         distPt.z = (camera->z - avgPt.z)/1000.0;

           /* Dont need the sqrt for dist as we are only interested
           ** in the ratio not the actual value */
         dist = distPt.x*distPt.x + distPt.y*distPt.y
                                  + distPt.z*distPt.z;
         dist /= depth;
            
         if (dist > longestDist)   /* Put at start */
         {
            if (prevSurface)  /* remove surface from current pos */
            {
                         /* take surface out of current pos */
               prevSurface->next = surface->next;
                         /* Put it in at the start */
               tempSurface = firstSurface;
               firstSurface = surface;
               firstSurface->next = tempSurface;
                   /* cont from where we were up too 
                   ** NOTE surface will go to next leaving
                   ** prevSurface currect */
               surface = prevSurface;
            }
            longestDist = dist;
         }
         else
            prevSurface = surface;
      }
      if (prevFirstSurface)
         prevFirstSurface->next = firstSurface;
      else
         blockDiagram->surfaces = firstSurface; 
      prevFirstSurface = firstSurface;
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION  drawBlockSurfaces
DESCRIPTION
     draw the block as a number of surfaces

INPUT

OUTPUT

RETURNED TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
drawBlockSurfaces (BLOCK_DIAGRAM_DATA *blockDiagram)
#else
drawBlockSurfaces (blockDiagram)
BLOCK_DIAGRAM_DATA *blockDiagram;
#endif
{
   double xLocStart, yLocStart, zLocStart, xLocEnd, yLocEnd, zLocEnd;
   double xBlkStart, yBlkStart, zBlkStart, xBlkEnd, yBlkEnd, zBlkEnd;
   double xCalStart, yCalStart, zCalStart, xCalEnd, yCalEnd, zCalEnd;
   BLOCK_SURFACE_DATA *surface;
   GEOLOGY_OPTIONS geolOptions;
   double cubeSize, halfCubeSize, dist;
   double coeffA, coeffB, coeffC, coeffD;
   int surfNum;
   Point3d *camera;

   memcpy (&geolOptions, &geologyOptions, sizeof(GEOLOGY_OPTIONS));
   cubeSize = blockDiagram->blockSize;
	halfCubeSize = cubeSize / (double) 2;
   camera = &(blockDiagram->threedData.camera);

   xBlkStart = blockDiagram->minXLoc;
   yBlkStart = blockDiagram->minYLoc;
   zBlkStart = blockDiagram->minZLoc;
   xBlkEnd = xBlkStart + blockDiagram->nx*cubeSize;
   yBlkEnd = yBlkStart + blockDiagram->ny*cubeSize;
   zBlkEnd = zBlkStart + blockDiagram->nz*cubeSize;

                       /* Loop throgh surfaces drawing them */
   surfNum = 0;
   for (surface = blockDiagram->surfaces; surface; surface = surface->next)
   {    
      xLocStart = MIN(surface->xStart, surface->xEnd);
      yLocStart = MIN(surface->yStart, surface->yEnd);
      zLocStart = MIN(surface->zStart, surface->zEnd);
      xLocEnd   = MAX(surface->xStart, surface->xEnd);
      yLocEnd   = MAX(surface->yStart, surface->yEnd);
      zLocEnd   = MAX(surface->zStart, surface->zEnd);
      
      if (!surface->surfaceData)    /* Create the surface if not already created */
      {			    /* make sure calculated data is through cube centres
		       	    ** even through it will be drawn at the edges */
			xCalStart = xLocStart; yCalStart = yLocStart; zCalStart = zLocStart;
			xCalEnd = xLocEnd;     yCalEnd = yLocEnd;     zCalEnd = zLocEnd;
/*
			if (xCalStart == xBlkStart) xCalStart += halfCubeSize;
			if (yCalStart == yBlkStart) yCalStart += halfCubeSize;
			if (zCalStart == zBlkStart) zCalStart += halfCubeSize;
			if (xCalStart == xBlkEnd) xCalStart -= halfCubeSize;
			if (yCalStart == yBlkEnd) yCalStart -= halfCubeSize;
			if (zCalStart == zBlkEnd) zCalStart -= halfCubeSize;
			if (xCalEnd == xBlkEnd) xCalEnd -= halfCubeSize;
			if (yCalEnd == yBlkEnd) yCalEnd -= halfCubeSize;
			if (zCalEnd == zBlkEnd) zCalEnd -= halfCubeSize;
			if (xCalEnd == xBlkStart) xCalEnd += halfCubeSize;
			if (yCalEnd == yBlkStart) yCalEnd += halfCubeSize;
			if (zCalEnd == zBlkStart) zCalEnd += halfCubeSize;
*/
         surface->surfaceData = calcBlockSectionColors(
                               xCalStart, yCalStart, zCalStart,
                               xCalEnd,   yCalEnd,   zCalEnd,
                               cubeSize, &surface->dataDim1,
                                         &surface->dataDim2);
      }
      
      convrt(surface->dip, 360.0-surface->dipDirection, 90.0, FALSE);
      plane (&coeffA, &coeffB, &coeffC, &coeffD,
             surface->xStart, surface->yStart, surface->zStart);

      dist = coeffA*camera->y + coeffB*camera->z
                              + coeffC*camera->x + coeffD;

                /* Dist > 0.0 means the surface is facing us */                              
      if (surface->surfaceData &&
          (((surface->forwardCheck == FORWARD_GREATER) && (dist > 0.0)) ||
           ((surface->forwardCheck == FORWARD_LESS) && (dist < 0.0))))
      {
         drawBlockSection(blockDiagram->threedData.pixmap,
             surface->surfaceData, surface->dataDim1, surface->dataDim2, cubeSize,
             xLocStart, yLocStart, zLocStart, xLocEnd, yLocEnd, zLocEnd,
             xBlkStart, yBlkStart, zBlkStart, xBlkEnd, yBlkEnd, zBlkEnd);
      }
      surfNum++;
   }

   return (TRUE);
}

/* ======================================================================
FUNCTION  drawDirectionIndicator
DESCRIPTION
     draw the north indicator at the positon given

INPUT

OUTPUT

RETURNED TRUE - sucess, FALSE - error
====================================================================== */
int
#if XVT_CC_PROTO
drawDirectionIndicator (WINDOW win, PNT startPos)
#else
drawDirectionIndicator (win, startPos)
WINDOW win;
PNT startPos;
#endif
{
   int textX, textY;
   Point3d start3d, end3d;
   Point2d start2d, end2d, diff2d;
   PNT endPos;
   double length2d;
   int length = 30;
   DRAW_CTOOLS tools;

   xvt_dwin_get_draw_ctools (win, &tools);
   tools.pen.width = 1;
   tools.pen.pat = PAT_SOLID;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_LTGRAY;
   xvt_dwin_set_draw_ctools (win, &tools);
   
   start3d.x = 0.0;
   start3d.y = 0.0;
   start3d.z = 0.0;
   end3d.x = 10000.0;
   end3d.y = 0.0;
   end3d.z = 0.0;
   
   ApplyTransform(&start3d, &start3d, &xFormViewer);
   ApplyTransform(&end3d,   &end3d,   &xFormViewer);
   
   Project(&start3d, &start2d);
   Project(&end3d,   &end2d);
   
   diff2d.x = end2d.x - start2d.x;
   diff2d.y = end2d.y - start2d.y;

   length2d = sqrt(diff2d.x*diff2d.x + diff2d.y*diff2d.y);

   endPos.h = startPos.h + (short) (length*(diff2d.x/length2d));
   endPos.v = startPos.v + (short) (length*(diff2d.y/length2d));
   
   xvt_dwin_draw_set_pos (win, startPos);
   xvt_dwin_draw_aline (win, endPos, FALSE, TRUE);
   
   textX = endPos.h + 2;
   textY = endPos.v - 2;
   xvt_dwin_set_fore_color (win, COLOR_LTGRAY);
   xvt_dwin_draw_text (win, textX, textY, "N", -1);
   
   return (TRUE);
}


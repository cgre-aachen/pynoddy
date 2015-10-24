#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include "scales.h"
#include <math.h>


#define DEBUG(X)    
#define TOLERANCE 0.0001

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern double iscale; /* scaling factor for geology */
extern COLOR backgroundColor;
extern double **topographyMap;
extern int TopoCol, TopoRow;
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE, minTopoValue, maxTopoValue;


#if XVT_CC_PROTO
extern int coutourImageFromDoubleData (XVT_PIXMAP , double **, int, int, int, int, int, int, int,
              double *, int, double, int, int, COLOR *);
#else
extern int coutourImageFromDoubleData ();
#endif
                 /* ************************* */
                 /* Globals used in this file */

/* ******************************************************************** *
CreateSolidSection
   Creates pixmap and draws a section in that pixmap to reflect the
   surface Data
   
RETURNS
   XVT_PIXMAP  - pixmap with the section drawing in it
   NULL_PIXMAP - Error
*********************************************************************** */
XVT_PIXMAP createSolidSection (parentWin, surface, penSize)
WINDOW parentWin;
BLOCK_SURFACE_DATA *surface;
int penSize;
{
   double xLocStart, yLocStart, zLocStart, xLocEnd, yLocEnd, zLocEnd;
	double **topoOverlay;
   XVT_PIXMAP pixmap = NULL_PIXMAP;
   DRAW_CTOOLS tools;
   COLOR *colorPtr, *brushColor;
   BLOCK_VIEW_OPTIONS *blockView = getViewOptions ();
   int rectSize, x, y, topoFlag = FALSE;
   RCT rect;
   PNT pos;
   
   if (!surface)
      return (pixmap);
   
   if (penSize < 1)  /* Size to draw the rectangles */
      rectSize = 1;
   else
      rectSize = penSize;

   xLocStart = MIN(surface->xStart, surface->xEnd);
   yLocStart = MIN(surface->yStart, surface->yEnd);
   zLocStart = MIN(surface->zStart, surface->zEnd);
   xLocEnd   = MAX(surface->xStart, surface->xEnd);
   yLocEnd   = MAX(surface->yStart, surface->yEnd);
   zLocEnd   = MAX(surface->zStart, surface->zEnd);
      
   if (!surface->surfaceData)
   {                 /* Create the surface if not already created */
		if ((fabs(zLocStart-zLocEnd) > TOLERANCE) || (!geologyOptions.useTopography))
		{
			surface->surfaceData = calcBlockSectionColors(
							xLocStart, yLocStart, zLocStart, xLocEnd,   yLocEnd,   zLocEnd,
							blockView->geologyCubeSize, &surface->dataDim1, &surface->dataDim2);
		}
		else  /* Calculate a map taking into account topo effects */
		{
			LAYER_PROPERTIES *currentLayer;
			int nx, ny, red, green, blue, error;
			double xLoc, yLoc, topoHeight;

			topoFlag = TRUE;
			nx = (int) floor ((xLocEnd-xLocStart)/blockView->geologyCubeSize);
			ny = (int) floor ((yLocEnd-yLocStart)/blockView->geologyCubeSize);
			if (!(surface->surfaceData = (COLOR **) create2DArray(nx, ny, sizeof(COLOR))))
			{
				return (pixmap);
			}
			surface->dataDim1 = nx;
			surface->dataDim2 = ny;

			topoOverlay = (double **) create2DArray (nx+1, ny+1, sizeof(double));
			overlayTopoOnArray (topographyMap, TopoCol, TopoRow,
									  TopomapYW, TopomapYE, TopomapXW, TopomapXE, 
									  topoOverlay, nx, ny, yLocStart, yLocEnd, xLocStart, xLocEnd);
/*									  blockView->originY, blockView->originY + ny*blockView->geologyCubeSize, */
/*									  blockView->originX, blockView->originX + nx*blockView->geologyCubeSize); */

			for (x = 0; x < nx; x++)
			{
				xLoc = xLocStart + x*blockView->geologyCubeSize + blockView->geologyCubeSize/2.0;
				for (y = 0; y < ny; y++)
				{
					yLoc = yLocStart + (ny-y-1)*blockView->geologyCubeSize + blockView->geologyCubeSize/2.0;           /* yLoc = yLocStart + y*blockView->geologyCubeSize + blockView->geologyCubeSize/2.0;  */
/*
					topoHeight = getTopoValueAtLoc (topographyMap, TopoCol, TopoRow,
											  TopomapYW, TopomapYE, TopomapXW, TopomapXE,
											  xLoc, yLoc, &error);
					if (error) topoHeight = zLocEnd;
*/
					topoHeight = topoOverlay[x+1][ny-y];

					yLoc = yLocStart + y*blockView->geologyCubeSize + blockView->geologyCubeSize/2.0;
					if (currentLayer = calcBlockPixel(xLoc, yLoc, topoHeight, FALSE,
						                FALSE, (float *) NULL, (float *) NULL,
											 FALSE, (float *) NULL, (float *) NULL, (float *) NULL))
					{
						red = currentLayer->color.red;
						green = currentLayer->color.green;
						blue = currentLayer->color.blue;
						surface->surfaceData[x][y] = XVT_MAKE_COLOR(red, green, blue);
					}
					else
						surface->surfaceData[x][y] = COLOR_BLACK;
				}
			}
			destroy2DArray ((char **) topoOverlay, nx+1, ny+1);
		}
   }


   if (!(pixmap = xvt_pmap_create (parentWin, XVT_PIXMAP_DEFAULT,
                  (short) (surface->dataDim1*rectSize), (short) (surface->dataDim2*rectSize), 0L)))
      return (pixmap);
   xvt_dwin_clear (pixmap, backgroundColor);

   xvt_dwin_get_draw_ctools (pixmap, &tools);
   tools.pen.width = 0;
   tools.pen.pat = PAT_HOLLOW;
   tools.pen.style = P_SOLID;
   tools.pen.color = COLOR_WHITE;
   tools.brush.color = COLOR_BLACK; 
   tools.brush.pat = PAT_SOLID;
   xvt_dwin_set_draw_ctools (pixmap, &tools);

   brushColor = &(tools.brush.color); 
   
   xvt_rect_set(&rect, (short) 0, (short) 0, (short) rectSize, (short) rectSize);
   for (x = 0; x < surface->dataDim1; x++)
   {
      colorPtr = surface->surfaceData[x]; 
      pos.h = x*rectSize;
      for (y = 0; y < surface->dataDim2; y++)
      {                    /* Set the color of the point to draw */
         *brushColor = colorPtr[surface->dataDim2-y-1];
         xvt_dwin_set_draw_ctools (pixmap, &tools);
                           /* Position the rect we are about to draw */
         pos.v = y*rectSize;
         xvt_rect_set_pos (&rect, pos);
         xvt_dwin_draw_rect (pixmap, &rect);
      }
   }

	if (topoFlag) /* Draw the topography over the section */
	{
		double *contourLevelPtr = NULL, interpolate;
		double **topoOverlay = NULL;
		COLOR *layerColors = NULL;
		int numContour = 10, contour;
		int xStart, yStart, xSize, ySize;

		interpolate = (double) penSize;
		xStart = 1;
		yStart = 1;
		xSize = (surface->dataDim1-1)-xStart;
		ySize = (surface->dataDim2-1)-yStart;

      topoOverlay = (double **) create2DArray (surface->dataDim1+1, surface->dataDim2+1, sizeof(double));
      overlayTopoOnArray (topographyMap, TopoCol, TopoRow,
								  TopomapYW, TopomapYE, TopomapXW, TopomapXE, 
                          topoOverlay, surface->dataDim1, surface->dataDim2,
                          yLocStart, yLocEnd, xLocStart, xLocEnd);

		contourLevelPtr = (double *) create1DArray(numContour, sizeof(double));
		layerColors = (COLOR *) create1DArray(numContour, sizeof(COLOR));
		if (contourLevelPtr && layerColors)
		{
			for (contour = 0; contour < numContour; contour++)
			{
				contourLevelPtr[contour] = geologyOptions.topoLowContour + contour*geologyOptions.topoContourInterval;
				layerColors[contour] = COLOR_LTGRAY;
			}
			coutourImageFromDoubleData(pixmap, topoOverlay, xSize, ySize, xStart, yStart,
						 TRUE, FALSE, FALSE, (double *) contourLevelPtr, (int) numContour, interpolate, 
						 (int) 0, (int) 0, (COLOR *) layerColors);
		}
      destroy2DArray ((char **) topoOverlay, surface->dataDim1+1, surface->dataDim2+1);
		destroy1DArray((char *) contourLevelPtr);
		destroy1DArray((char *) layerColors);
	}

   return (pixmap);
}

/* ******************************************************************** *
CreateLineSection
   Creates pixmap and draws a section in that pixmap to reflect the
   surface Data
   
RETURNS
   XVT_PIXMAP  - pixmap with the section drawing in it
   NULL_PIXMAP - Error
*********************************************************************** */
XVT_PIXMAP createLineSection (parentWin, surface, blockCubeSize, scale, overlayImage)
WINDOW parentWin;
BLOCK_SURFACE_DATA *surface;
double blockCubeSize;
int scale;
WINDOW overlayImage;
{
   double xLocStart, yLocStart, zLocStart, xLocEnd, yLocEnd, zLocEnd;
	double **topoOverlay;
   register int m;
   OBJECT *event;
   STRATIGRAPHY_OPTIONS *stratOptions = NULL;
   XVT_PIXMAP pixmap = NULL_PIXMAP;
   COLOR layerColors[55];
   int nx, ny, cyphno = 0, index1, topoFlag = FALSE;
   int ncon, j, k, index, l, i, red, green, blue;
   unsigned int flavor, flavor1;
   unsigned char cypher[55][ARRAY_LENGTH_OF_STRAT_CODE];
   double cval[55];  /* contour values*/
   int numEvents = (int) countObjects(NULL_WIN);
   double SectionDec = geologyOptions.welllogDeclination;
   int iign;  /* iign TRUE if DYKE or PLUG */
   double ***dots, **map, xo, yo;
   STORY **histoire;
   BOOLEAN found;
   double cubeSize;
   RCT imageRect;
 
   ncon = 7;

   xLocStart = MIN(surface->xStart, surface->xEnd);
   yLocStart = MIN(surface->yStart, surface->yEnd);
   zLocStart = MIN(surface->zStart, surface->zEnd);
   xLocEnd   = MAX(surface->xStart, surface->xEnd);
   yLocEnd   = MAX(surface->yStart, surface->yEnd);
   zLocEnd   = MAX(surface->zStart, surface->zEnd);
   if (overlayImage)
   {
      xvt_vobj_get_client_rect (overlayImage, &imageRect);
      cubeSize = (xLocEnd - xLocStart)
                          / (((double) xvt_rect_get_width(&imageRect))/5.3); /* 6.0 */
   }
   else
      cubeSize = blockCubeSize;

	if ((fabs(zLocStart-zLocEnd) > TOLERANCE) || (!geologyOptions.useTopography))
	{
		if (!calcReverseSection(xLocStart, yLocStart, zLocStart, xLocEnd, yLocEnd, zLocEnd,
										cubeSize, &dots, &histoire, &nx, &ny))
			return (pixmap);                      
	}
	else
	{
		int x, y, error;
		double xLoc, yLoc, topoHeight;

		topoFlag = TRUE;
		nx = (int) floor ((xLocEnd-xLocStart)/cubeSize);
		ny = (int) floor ((yLocEnd-yLocStart)/cubeSize);
		dots = (double ***) create3DArray (nx+1, ny+1, 4, sizeof(double));
		histoire = (STORY **) create2DArray (nx+1, ny+1, sizeof(STORY));
		if (!dots || !histoire)
		{
			destroy3DArray ((char ***) dots, nx+1, ny+1, 4);
			destroy2DArray ((char **) histoire, nx+1, ny+1);
			return (pixmap);                      
		}

		topoOverlay = (double **) create2DArray (nx+1, ny+1, sizeof(double));
		overlayTopoOnArray (topographyMap, TopoCol, TopoRow,
								  TopomapYW, TopomapYE, TopomapXW, TopomapXE, 
								  topoOverlay, nx, ny,
								  yLocStart, yLocEnd, xLocStart, xLocEnd);

		for (x = 0; x < nx; x++)
		{
			xLoc = xLocStart + x*cubeSize + cubeSize/2.0;
			for (y = 0; y < ny; y++)
			{
				yLoc = yLocStart + (ny-y-1)*cubeSize + cubeSize/2.0;           /* yLoc = yLocStart + y*cubeSize + cubeSize/2.0;  */
/*
				topoHeight = getTopoValueAtLoc (topographyMap, TopoCol, TopoRow,
										  TopomapYW, TopomapYE, TopomapXW, TopomapXE,
										  xLoc, yLoc, &error);
				if (error) topoHeight = zLocEnd;
*/
				topoHeight = topoOverlay[x+1][ny-y];

            dots[x+1][y+1][1] = xLoc;
            dots[x+1][ny-y][2] = yLoc;
            dots[x+1][y+1][3] = topoHeight;
            histoire[x+1][y+1].again = TRUE;
            izero(histoire[x+1][y+1].sequence);
         }
      }
		destroy2DArray ((char **) topoOverlay, nx+1, ny+1);
		reverseEvents (dots, histoire, nx, ny);
	}

   if (overlayImage)
   {
      pixmap = overlayImage;

      xo = ((double) xvt_rect_get_height(&imageRect) - MAC_V)/-MAC_SCALE;    /* Really Y */
      yo = 0.0;                                                              /* Really X */

      xo -= 0.6; /* 0.6; */                                                  /* Really Y */
      yo -= 0.6;                                                             /* Really X */
   }
   else
   {
      if (!(pixmap = xvt_pmap_create (parentWin, XVT_PIXMAP_DEFAULT,
                                      (short) (nx*scale), (short) (ny*scale), 0L)))
      {
         destroy2DArray((char **) histoire, nx+1, ny+1);
         destroy3DArray((char ***) dots, nx+1, ny+1, 4);
         return (pixmap);
      }
      xvt_dwin_clear (pixmap, backgroundColor);

      xo = 0.0 - ((ny+1)*(scale/10.0)-35)*0.35;
      yo = 0.0;
   }
   

   if (!(map = (double **) create2DArray(nx+1, ny+1, sizeof(double))))
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      destroy2DArray((char **) histoire, nx+1, ny+1);
      destroy3DArray((char ***) dots, nx+1, ny+1, 4);
      return (pixmap);
   }
   else
   {
                /* make sure all the drawing goes into this window */
      setCurrentDrawingWindow (pixmap);

      initLongJob (0, nx*(countObjects(NULL_WIN)+1), "Calculating Line Map...", (char *) NULL);

/*
      ScaleTopoOld (dots, map, cval, &ncon, nx, ny);

      if (geologyOptions.useTopography)
      {
         mcontr(topographyMap, nx-1, ny-1, cval, ncon+1, 1.0e6, 0.2, 1.3, 1.0,
                               TRUE, BASE_STRAT, FALSE, NULL);
         mcontrlabel(topographyMap, nx-1, ny-1, cval, ncon+1, 1.0e6, 0.2, 1.3, 1.0);
      }
*/
      
                        /* **************************************** */
                        /* Count the number of different rock types */
      cyphno = 1;
      iequal(cypher[1],histoire[0][0].sequence);
      for (k = 1; k <= nx; k++) 
      {                                                             
         for (l = 1; l <= ny; l++) 
         {                                                          
            found = FALSE;   /* Search for this rock */
            for (j = 1; j <= cyphno; j++) 
               if (coincide(histoire[k][l].sequence, &(cypher[j][0])))
                  found = TRUE;
   
            if (!found)   /* Add it if we could not find it */
            {
               cyphno++;
               iequal(&(cypher[cyphno][0]), histoire[k][l].sequence);
            }
         }
      }

      mplot(4.0, 3.3, -3);
      for (j = 1; j <= cyphno; j++) 
      {
         iign=0;
 
         taste (numEvents, cypher[j], &flavor, &index);
         if (j < cyphno)
         {
            taste (numEvents, cypher[j+1], &flavor1, &index1);
         }
         else
            flavor1 = flavor;

         if (flavor != IGNEOUS_STRAT)
         {                                  
            for (k = 1; k <= ny; k++)
            {
               for (l = 1; l <= nx; l++)
               {
                  if (coincide(histoire[l][k].sequence, cypher[j]))
                     map[l][k] = dots[l][k][3];
                  else
                     map[l][k] = 1.0e8;
               }                                                             
            }

            if (flavor == BASE_STRAT)
               index = 0;

              /* if the current event does not have a stratigraphy
              ** associated with it then use the base stratigraphy */
            if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
               event = (OBJECT *) nthObject (NULL_WIN, 0);
            if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
                                   getStratigraphyOptionsStructure (event)))
            {
               if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
                  getStratigraphyOptionsStructure((OBJECT *) nthObject (NULL_WIN, 0))))
                  return (pixmap);  /* not even a base strat is defined yet */
            }

            for (m = 0; m < 10; m++)
               cval[m] = -99999.0;  /* initialise off the edge */
                            /* assign cval the height of each layer but first */
            for (m = 1; m <= (stratOptions->numLayers-1); m++)
               cval[m] = stratOptions->properties[m].height;

            for (m = 0; m < stratOptions->numLayers; m++)
            {
               red = stratOptions->properties[m].color.red;
               green = stratOptions->properties[m].color.green;
               blue = stratOptions->properties[m].color.blue;
               layerColors[m] = (COLOR) XVT_MAKE_COLOR(red, green, blue);
            }

            ncon = stratOptions->numLayers;
            mcontr(map, nx-1, ny-1, cval, ncon, 1.0e6, 0.2, xo, yo, FALSE,
                   flavor, iign, layerColors);
         }

         penSize(1,1);

         for (i = 0; i < (nx/cyphno); i++)
            incrementLongJob (INCREMENT_JOB);
      }
    
      contour_boundaries (histoire, nx-1, ny-1, xo, yo);

      destroy2DArray((char **) histoire, nx+1, ny+1);
      destroy3DArray((char ***) dots, nx+1, ny+1, 4);
      destroy2DArray((char **) map, nx+1, ny+1);

      finishLongJob ();
   }

	if (topoFlag) /* Draw the topography over the section */
	{
		double *contourLevelPtr = NULL, interpolate;
		double **topoOverlay = NULL;
		COLOR *layerColors = NULL;
		int numContour = 10, contour;
		int xStart, yStart, xSize, ySize;

		interpolate = (double) 1.0;
		xStart = 1;
		yStart = 1;
		xSize = (surface->dataDim1-1)-xStart;
		ySize = (surface->dataDim2-1)-yStart;

      topoOverlay = (double **) create2DArray (surface->dataDim1+1, surface->dataDim2+1, sizeof(double));
      overlayTopoOnArray (topographyMap, TopoCol, TopoRow,
								  TopomapYW, TopomapYE, TopomapXW, TopomapXE, 
                          topoOverlay, surface->dataDim1, surface->dataDim2,
                          yLocStart, yLocEnd, xLocStart, xLocEnd);

		contourLevelPtr = (double *) create1DArray(numContour, sizeof(double));
		layerColors = (COLOR *) create1DArray(numContour, sizeof(COLOR));
		if (contourLevelPtr && layerColors)
		{
			for (contour = 0; contour < numContour; contour++)
			{
				contourLevelPtr[contour] = geologyOptions.topoLowContour + contour*geologyOptions.topoContourInterval;
				layerColors[contour] = COLOR_LTGRAY;
			}
			coutourImageFromDoubleData(pixmap, topoOverlay, xSize, ySize, xStart, yStart,
						 TRUE, FALSE, FALSE, (double *) contourLevelPtr, (int) numContour, interpolate, 
						 (int) 0, (int) 0, (COLOR *) layerColors);
		}
      destroy2DArray ((char **) topoOverlay, surface->dataDim1+1, surface->dataDim2+1);
		destroy1DArray((char *) contourLevelPtr);
		destroy1DArray((char *) layerColors);
	}
   
   return (pixmap);
}


/* ***********************************************************************

    void DoSection()


    DoSection function calculates contact map for current section

    DoSection() takes no arguments
    DoSection() returns no value

** ********************************************************************* */
void DoSectionOld (win, resolution)
WINDOW win;
int resolution;
{
   int nx,ny;
   int nx1,ny1;
   double SectionDec = geologyOptions.welllogDeclination;
   double ***dots;
   struct story **histoire;
   double  BSize;        /* the block size to use for a pixel */
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;
    
   DEBUG(printf("\nDOSECTION: start");)
   BSize = setBlockSize (resolution);

   nx = 71;                                                                     
   ny = 36;                                                                     

   nx1 = (int) (25.0/BSize);
   ny1 = (int) (12.0/BSize);
     
   if ((histoire = (struct story **) strstomat(0,nx1,0,ny1))==0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }

   if ((dots = (double ***) qdtrimat(0,nx1,0,ny1,0,3))==0L)
   {  
      xvt_dm_post_error("Not enough memory, try closing some windows");
      free_strstomat(histoire,0,nx1,0,ny1);
      return;
   }

                /* make sure all the drawing goes into this window */
   setCurrentDrawingWindow (win);

   initLongJob (0, nx1*(countObjects(NULL_WIN)+1), "Calculating Section...", (char *) NULL);

                            /* create the dot map for the section */
   ldotmap(dots, histoire, SECTION, BSize, nx1, ny1);
      
   DEBUG(printf("\nDOSECTION: drawing border");)
   mplot(1.0-.125,1.3,3);
   mplot(1.0+.125,1.3,2);
   mplot(1.0,1.3-.125,3);
   mplot(1.0,1.3+.125,2);

   mplot(1.0+nx*0.2-.125,1.3+ny*0.2,3);
   mplot(1.0+nx*0.2+.125,1.3+ny*0.2,2);
   mplot(1.0+nx*0.2,1.3+ny*0.2-.125,3);
   mplot(1.0+nx*0.2,1.3+ny*0.2+.125,2);

   PlotSectionLegend (40, 20, absx, absy, absz, SectionDec);
   freeqdtrimat (dots,0,nx1,0,ny1,0,3);
   free_strstomat (histoire,0,nx1,0,ny1);

   finishLongJob ();

   DEBUG(printf("\nDOSECTION: end");)
}

/************************************************************************
*                                            *
*    void DoWellLog()                             *
*                                            *
*                                            *
*    DoWellLog function calculates well log for current geology  *
*                                            *
*    DoWellLog() takes no arguments                         *
*    DoWellLog() returns no value                      *
*                                            *
************************************************************************/
void DoWellLog (win, xyzPoints, numPoints)
WINDOW win;
double **xyzPoints;
int numPoints;
{
   OBJECT *object = NULL;
   NODDY_COLOUR *colorOptions = NULL;
   RCT rect;
   COLOR color;
   register int m;
   int nx, ny, index, ninc;
   unsigned int flavor;
   double xinc, yinc, xcord, ycord;
   double dx, dy, dz, dzs;
   int intx, inty, j, k;
   char ttt[255];
   int numEvents = (int) countObjects (NULL_WIN);
   double ***dots;
   struct story **histoire;
   double WellAngleZ = geologyOptions.boreholeDip;
   double SectionDec = geologyOptions.boreholeDecl;
   double WellDepth = geologyOptions.boreholeLength;
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;
    
   DEBUG(printf("\nDOWELLLOG: start");)

   nx=2;
   ny=2;
   
   if (xyzPoints && numPoints)
      ninc = numPoints-1;
   else
      ninc = (int) (WellDepth/100);

   yinc=15/MAC_SCALE;
   xinc=20/MAC_SCALE;

   if ((histoire = (struct story **) strstomat(0,nx,0,ny)) == 0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }

   if ((dots = (double ***) qdtrimat(0,nx,0,ny,0,3)) == 0L)
   { 
      xvt_dm_post_error("    Not enough memory, try closing some windows");
      free_strstomat(histoire,0,nx,0,ny);
      return;
   }

   initLongJob (0, ninc*(countObjects(NULL_WIN)-1), "Calculating Well Log...", NULL);

                /* make sure all the drawing goes into this window */
   setCurrentDrawingWindow (win);
   
   dx = sin(SectionDec*3.14159/180.0);
   dy = cos(SectionDec*3.14159/180.0);
   dz = cos(WellAngleZ*3.14159/180.0);
   dzs= sin(WellAngleZ*3.14159/180.0);

   for (m = 0; m <= ninc; m++) 
   {                                                            
      if (xyzPoints && numPoints)
      {                         /* Use Points passed in */
         dots[1][1][1] = xyzPoints[m][0];
         dots[1][1][2] = xyzPoints[m][1];
         dots[1][1][3] = xyzPoints[m][2];
      }
      else
      {                         /* Generate Poitns if none are given */
         dots[1][1][1] = absx + (100*m)*dzs*dx;                            
         dots[1][1][2] = absy + (100*m)*dzs*dy;;                        
         dots[1][1][3] = (absz+ (500*iscale))-(100*m)*dz;
      }

      histoire[1][1].again=1;
      izero(histoire[1][1].sequence);
        
                            /* ************************** */
                            /* Perform the inverse events */
      j = 1; k = 1;
      reverseEvents (dots, histoire, j, k);

                   /* ************************************************** */
                   /* draw labels at end of each section of the well log */
      xcord = (double)40+((m/20)*100);
      ycord = (double)((m%20)*15)+40;
      xcord = (xcord-5)/MAC_SCALE;
      ycord = (MAC_V-ycord)/MAC_SCALE;
      if (xyzPoints && numPoints)
		{
			if (m%20 == 0)
				  number(xcord+1.5*xinc, ycord+yinc, 0.15, (double)xyzPoints[0][2]+(xyzPoints[1][2]-xyzPoints[0][2])*m, 0.0, -1);
			if (m == ninc)
				  number(xcord+1.5*xinc, ycord+yinc, 0.15, (double)xyzPoints[0][2]+(xyzPoints[1][2]-xyzPoints[0][2])*m, 0.0, -1);
		}
		else
		{
			if (m%20 == 0)
				  number(xcord+1.5*xinc, ycord+yinc, 0.15, (double)(100*m), 0.0, -1);
			if (m == ninc)
				  number(xcord+1.5*xinc, ycord+yinc, 0.15, (double)(100*m), 0.0, -1);
		}

      taste(numEvents, histoire[j][k].sequence, &flavor, &index);

      if (flavor == IGNEOUS_STRAT)
      {                                               
         intx=(int)(MAC_SCALE*(xcord+xinc/2))+5;
         inty=(int)(MAC_V-(MAC_SCALE*(ycord+yinc/2)));

         if (!(object = (OBJECT *) nthObject (NULL_WIN, index)))
         {
            object = (OBJECT *) nthObject (NULL_WIN, 0);
         }
         if (object && (colorOptions = (NODDY_COLOUR *)
                                       getNoddyColourStructure (object)))
         {
            color = XVT_MAKE_COLOR (colorOptions->red, colorOptions->green,
                                colorOptions->blue);
         }
         else
         {
            color = COLOR_BLACK;
         }

         SetColor (color);

         rect.left = intx - 9;
         rect.top = inty - 7;
         rect.right = rect.left + 19;
         rect.bottom = rect.top + 15;
         xvt_dwin_draw_rect (win, &rect);
      }
      else                                                                   
         lwhich (xcord, ycord, index, dots[j][k][1],
                 dots[j][k][2], dots[j][k][3], flavor, WELLLOG);

           /* draw the black border to the well */
      SetColor (COLOR_BLACK);
      mplot(xcord,ycord,3);
      mplot(xcord,ycord+yinc,2);
      mplot(xcord+xinc,ycord,3);
      mplot(xcord+xinc,ycord+yinc,2);
   }

   SetColor (COLOR_BLACK);

   sprintf(ttt,"WellLog Declination = %.1lf",SectionDec);
   drawString (15, 350, ttt);

   sprintf(ttt,"The Angle between WellLog and Z axis = %.1lf",WellAngleZ);
   drawString (15, 370, ttt);

   sprintf(ttt, "Top of WellLog:  x = %.1lf, y = %.1lf, z = %.1lf",
						     xyzPoints[0][0], xyzPoints[0][1], xyzPoints[0][2]);
   drawString (15, 390, ttt);

   sprintf(ttt,"Bottom of WellLog:  x = %.1lf, y = %.1lf, z = %.1lf",
           xyzPoints[numPoints-1][0], xyzPoints[numPoints-1][1], xyzPoints[numPoints-1][2]);
   drawString (15, 410, ttt);

   freeqdtrimat (dots, 0, nx, 0, ny, 0, 3);  
   free_strstomat (histoire, 0, nx, 0, ny);

   finishLongJob ();

   DEBUG(printf("\nDOWELLLOG: end");)
}

int PlotSectionLegend (tlh, tlv, absx, absy, absz, SectionDec)
int tlh,tlv;
double absx, absy, absz;
double SectionDec;
{
   int vinc=20,htab1=15,htab2=75;
   double length,dx,dy;
   char number[40];                                 

   tlv += 10;

   DEBUG(printf("\nPLOTSECTIONLEGEND: start");)
   length=1000.01*iscale;
   dx=length*sin(SectionDec*3.14159/180.0);
   dy=length*cos(SectionDec*3.14159/180.0);

   penSize(2,2);

   moveTo (tlh, tlv); lineTo (tlh+220,tlv);
   moveTo (tlh+220,tlv); lineTo (tlh+220,tlv+45);
   moveTo (tlh+220,tlv+45); lineTo (tlh,tlv+45);
   moveTo (tlh,tlv+45); lineTo (tlh,tlv);
     
   drawString(tlh+htab1,tlv+vinc, (char *) "Up-R:");

   sprintf (number,"%.0f",absx+dx);
   drawString(tlh+htab2, tlv+vinc, number);

   sprintf (number,"%.0f",absy+dy);
   drawString(tlh+htab2+50, tlv+vinc, number);

   sprintf (number,"%.0f",absz+(500*iscale));
   drawString(tlh+htab2+100, tlv+vinc, number);

   vinc+=15;
   drawString(tlh+htab1,tlv+vinc, (char *) "Low-L:");

   sprintf (number,"%.0f",absx);
   drawString(tlh+htab2, tlv+vinc, number);

   sprintf (number,"%.0f",absy);
   drawString(tlh+htab2+50, tlv+vinc, number);

   sprintf (number,"%.0f",absz);
   drawString(tlh+htab2+100, tlv+vinc, number);

   penSize(1,1);
   DEBUG(printf("\nPLOTSECTIONLEGEND: end");)
   
   return (TRUE);
}

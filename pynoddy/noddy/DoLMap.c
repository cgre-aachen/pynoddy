#include "xvt.h"
#include "noddy.h"
#include "scales.h"
#include <math.h>


#define DEBUG(X)    

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern double iscale; /* scaling factor for geology */
extern int TopoRow, TopoCol;
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE;
extern double **topographyMap;

                 /* ************************* */
                 /* Globals used in this file */


/************************************************************************
*                                            *
*    void DoLineMap()                             *
*                                            *
*                                            *
*    DoLineMap function calculates contact map for current geology    *
*                                            *
*    DoLineMap() takes no arguments                         *
*    DoLineMap() returns no value                      *
*                                            *
************************************************************************/
void DoLineMap(win, resolution)
WINDOW win;
int resolution;
{
   int nx, ny;
   int bar=0, ncon;
   int nx1, ny1;
   double koeff;
   double absx, absy, absz;
   double SectionDec = geologyOptions.welllogDeclination;
   double LinemapXE, LinemapYE;
   double ***dots;
   struct story **histoire;
   double  BSize;        /* the block size to use for a pixel */
   unsigned int flavor = 0;
   double cval[16];
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;
   
   LinemapXE = absx;
   LinemapYE = absy;
   
   DEBUG(printf("\nDOLINEMAP - SOLID: start");)
   BSize = setBlockSize (resolution);

   nx=71;                                                                     
   ny=51;                                                                     
   ncon=7;

   nx1 = (int) (25/BSize);
   ny1 = (int) (17/BSize);

   if (LinemapXE > 1000*iscale)
      nx1 = (int) ((LinemapXE/(1000*iscale))*(25/BSize)-1);

   if (LinemapYE > 700*iscale)
      ny1 = (int) ((LinemapYE/(700*iscale))*(17/BSize)-1);

   if (nx1 > 210)
      koeff=1000*nx1/210;

   if (ny1 > 146)
      koeff=800*ny1/146; 

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
   else
   {
                /* make sure all the drawing goes into this window */
      setCurrentDrawingWindow (win);

      initLongJob (0, nx1*(countObjects(NULL_WIN)+1),"Calculating Line Map...", (char *) NULL);

      ScaleTopo(cval, &ncon);

      ldotmap(dots, histoire, LINE, BSize, nx1, ny1);

      koeff=MAC_V_LINE/MAC_SCALE;

      mplot(1.0-.125,1.3-koeff,3);

      if (geologyOptions.useTopography)
      {
         mcontr(topographyMap, nx-1,ny-1,cval,ncon+1,1.0e6,0.2,1.3,1.0,TRUE,
                flavor, FALSE, NULL);
         mcontrlabel(topographyMap, nx-1,ny-1,cval,ncon+1,1.0e6,0.2,1.3,1.0);
      }

      /*PlotMapLegend(20,300);*/
      freeqdtrimat(dots,0,nx1,0,ny1,0,3);
      free_strstomat(histoire,0,nx1,0,ny1);

      finishLongJob ();
   }

   DEBUG(printf("\nDOLINEMAP - SOLID: end");)
}

/************************************************************************
*                                            *
*    void ScaleTopo()                             *
*                                            *
*                                            *
*    ScaleTopo function scales topography according to block scale    *
*                                            *
*    ScaleTopo() takes no arguments                         *
*    ScaleTopo() returns no value                      *
*                                            *
************************************************************************/
int ScaleTopo(cval, ncon)
double *cval;
int *ncon;
{
   register int k;
                                                  
   if (geologyOptions.useTopography && topographyMap) 
   {
      *ncon = 7;
      for(k=1;k<=*ncon;k++)                                     
         cval[k] = geologyOptions.topoLowContour
                           + (geologyOptions.topoContourInterval*(k-1));  

   } /* if (geologyOptions.useTopography) */
   
   return (TRUE);
}


int ScaleTopoOld (dots, map, cval, ncon, nx, ny)
double ***dots;
double **map;
double *cval;
int *ncon;
int nx, ny;
{
   register int k,jk,kl;
   double absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   absz = viewOptions->originZ;
              
   if (geologyOptions.useTopography)
   {
      *ncon = 7;
      for (k = 1; k <= *ncon; k++)                                     
         cval[k] = geologyOptions.topoLowContour
                           + (geologyOptions.topoContourInterval*(k-1));  
              
      for (jk = 1; jk <= ny; jk++)
      {
         for (kl = 1; kl <= nx; kl++)
         {
            if ((jk <= TopoRow) && (kl <= TopoCol))
               map[kl][jk] = (double) topographyMap[kl][jk];
            else
               map[kl][jk] = 0.0;
         }
      }
   }
        
   for (jk = 1; jk <= ny; jk++)
   {
      for (kl = 1; kl <= nx; kl++)
      {
         if (geologyOptions.useTopography)
            dots[kl][jk][3] = map[kl][jk];
         else
            dots[kl][jk][3] = absz;
      }
   } 
   return (TRUE);
}

/************************************************************************
*                                            *
*    void ScaleOneTopo(Boolean ready,int xcol, yrow)             *
*                                            *
*    Boolean ready; flag to see if map already read in      *
*    int xcol, yrow; position of mouse click                *
*    ScaleOneTopo function scales topography according to block scale*
*         same as ScaleTopo except that its for symbol plotting  *
*                                            *
*    ScaleOneTopo() takes 3 arguments                  *
*    ScaleOneTopo() returns no value                        *
*                                            *
************************************************************************/
int ScaleOneTopo(dots,xcol,yrow)
double ***dots;
int xcol, yrow;
{
   double valueMapTopo;
                                                  
                       /*if(geologyOptions.useTopography && !ready) */
   if (geologyOptions.useTopography && topographyMap)
   {
      ApproxDot(xcol, yrow, &valueMapTopo);
      dots[1][1][3] = valueMapTopo;
   }

   return (TRUE);
}

/************************************************************************
*                                            *
*    void DoTopoMap()                             *
*                                            *
*                                            *
*    DoTopoMap function draws contour map of topography          *
*                                            *
*    DoTopoMap() takes no arguments                         *
*    DoTopoMap() returns no value                      *
*                                            *
************************************************************************/
void DoTopoMap (drawWin)
WINDOW drawWin;
{
   int nx, ny, x, y;
   double cval[16], **array;
   int ncon;
   unsigned int flavor = 0;
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;

   if (!(geologyOptions.useTopography && topographyMap))
   {
      xvt_dm_post_error("Error, You must first load a topography from Geology Options");
      return;
   }

   setCurrentDrawingWindow (drawWin);
   penSize (2,2);

   nx=71;
   ny=51;
   ncon=7;

   ScaleTopo(cval, &ncon);
DEBUG(for (nx = 0; nx < ncon+1; nx++))
DEBUG(printf("\n cval[%d] = %f",nx,cval[nx]);)

   if (!(array = (double **) create2DArray(TopoCol+1, TopoRow+1, sizeof(double))))
	   return ;
	for (x = 1; x <= TopoCol; x++)
	   for (y = 1; y <= TopoRow; y++)
		    array[x][y] = topographyMap[x][TopoRow-y+1];
   mcontr(array, TopoCol-1,TopoRow-1,cval,ncon+1,1.0e6,0.2,1.3,1.0,TRUE,
                                              flavor, FALSE, (COLOR *) NULL);
	destroy2DArray((char **) array, TopoCol+1, TopoRow+1);
   mcontrlabel(topographyMap,TopoCol-1,TopoRow-1,cval,ncon+1,1.0e6,0.2,1.3,1.0);
     
   PlotMapLegend(20, 300, absx, absy, absz);
}

int PlotMapLegend (tlh, tlv, absx, absy, absz)
int tlh,tlv;
double absx, absy, absz;
{
   int vinc=20,htab1=15,htab2=65;
   char temp[50]; 

   tlv+=10;
   penSize(2,2);

   moveTo (tlh, tlv); lineTo (tlh+200, tlv);
   moveTo (tlh+200, tlv); lineTo (tlh+200, tlv+60);
   moveTo (tlh+200, tlv+60); lineTo (tlh, tlv+60);
   moveTo (tlh, tlv+60); lineTo (tlh, tlv);

   drawString (tlh+htab1, tlv+vinc, "SW:");
   sprintf(temp, "%d", (int) absy);
   drawString (tlh+htab2, tlv+vinc, temp);
   sprintf(temp, "%d", (int) absx);
   drawString (tlh+htab2+60, tlv+vinc, temp);
     
   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "NE:");
   sprintf(temp, "%d", (int) (absy+(BLOCK_DIAGRAM_SIZE_Y)));
   drawString (tlh+htab2, tlv+vinc, temp);
   sprintf(temp, "%d", (int) (absx+(BLOCK_DIAGRAM_SIZE_X)));
   drawString (tlh+htab2+60, tlv+vinc, temp);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Z:");
   sprintf(temp, "%d", (int) (absz+(BLOCK_DIAGRAM_SIZE_Z)));
   drawString (tlh+htab2, tlv+vinc, temp);
   penSize(1,1);
   
   return (TRUE);
}


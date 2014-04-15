#include <math.h>
#include "xvt.h"
#include "noddy.h"

#define DEBUG(X)

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern double iscale;         /* scaling factor for geology */
extern int TopoRow, TopoCol;
extern double TopomapXE, TopomapYE;
extern double TopomapXW, TopomapYW;
extern double **topographyMap;

                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
extern int lblock(double ***, struct story **, double, int, int, int);
#else
extern int lblock();
#endif


/************************************************************************
*                                            *
*    void ldotmap(maptype)                             *
*                                            *
*                                            *
*    dotmap function generates coordinate arrays for block       * 
*   diagrams and calls inverse function routines                      *
*   in reverse chronological order.                                   *       
*                                            *
*    dotmap() takes one arguments                      *
*    dotmap() returns no arguments                     *
*                                            *
************************************************************************/
void ldotmap(dots, histoire, maptype, BSize, numX, numY)
double ***dots;
struct story **histoire;
int maptype; 
double BSize;
int numX, numY;
{
/*
c 
c   ldotmap generates coordinate arrays for Linemap
c   and calls inverse function routines
c   in reverse chronological lh[15][3],order.
*/
   register int m,n;
   int bar=0;
   double zz,dx,dy;
   double **maptopo;
   int mmax, nmax, j, k, numEvents = (int) countObjects(NULL_WIN);
   double SectionDec = geologyOptions.welllogDeclination;
   double totalScale;
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;

   totalScale = BSize * iscale;

   mmax = numX;
   nmax = numY;

   if (maptype == LINE)
      zz = BLOCK_DIAGRAM_SIZE_Z+absz;
   else if (maptype == SECTION)
      zz = absz;

   j = mmax;
   k = nmax;
 
   if (maptype == LINE)
   {
      if (!geologyOptions.useTopography)
      {
         for (m=1;m<=mmax;m++)
         {                                                        
            incrementLongJob (INCREMENT_JOB);
            for (n=1;n<=nmax;n++)
            {                                                            
            dots[m][n][1]=((m-1)*40.0*totalScale)+absx;
            dots[m][n][2]=((n-1)*41.17647*totalScale)+absy;
               dots[m][n][3]=zz;

               histoire[m][n].again=1;
               izero(histoire[m][n].sequence);
            }
         }
      }
      else
      {
         if ((maptopo= (double **) dmatrix(0,mmax+1,0,nmax+1))==0L)
         {
            xvt_dm_post_error("Not enough memory, try closing some windows");
            return;
         }
         else
         {
            calculMaptopo(maptopo,mmax,nmax,BSize,0); /* x: mmax,y: nmaxx*/

            for (m=1;m<=mmax;m++)
            {           
               incrementLongJob (INCREMENT_JOB);
               for (n=1;n<=nmax;n++)
               {          
                  dots[m][n][1]=((m-1)*40.0*totalScale)+absx;
                  dots[m][n][2]=((n-1)*41.17647*totalScale)+absy;
                  dots[m][n][3]=maptopo[m][n]; 

                  histoire[m][n].again=1;
                  izero(histoire[m][n].sequence);
               }
            }
            free_dmatrix(maptopo,0,mmax+1,0,nmax+1);
         } 
      }  /* geologyOptions.useTopography */

   } /* maptype */
   else if (maptype == SECTION)
   {
      dx=sin(SectionDec*3.14159/180.0);
      dy=cos(SectionDec*3.14159/180.0);

      for (m=1;m<=mmax;m++)
      {           
         incrementLongJob (INCREMENT_JOB);
         for (n=1;n<=nmax;n++)
         {          
            dots[m][n][1]=(m-1)*40.0*totalScale*dx+absx;
            dots[m][n][2]=(m-1)*40.0*totalScale*dy+absy;
            dots[m][n][3]=(n-1)*40.0*totalScale+zz;

            histoire[m][n].again=1;
            izero(histoire[m][n].sequence);
         }
      }
   }

   bar=0;
   if ((numEvents-1) > 0)
   {        
      reverseEvents (dots, histoire, j, k);
   }                                                      

   if (maptype == LINE)
      lblock (dots, histoire, BSize, mmax, nmax, LINE);
   else if (maptype == SECTION)
      lblock (dots, histoire, BSize, mmax, nmax, SECTION);
}

/*
 interpolation for Linemap  from topofile  51*71 to nmax*mmax
              for Anomalies  --'--               to ny*nx
*/

int calculMaptopo (maptopo, mmax, nmax, BSize, iLineAnom)
double **maptopo;
int mmax,nmax;
double BSize;
int iLineAnom;
{
   /* iLineAnom: 0 - Linemap, 1 - Anomalies for Map, 2 - Profile*/
   int startX,startY; /* Col & Row of TopoMap for MapXW & MapYW */
   double stepXline,stepYline,stepXtopo,stepYtopo;
   double MapXE,MapYE,MapXW,MapYW;
   double cosDeclin,sinDeclin;
   double LinemapXE, LinemapYE;
   double gblock, msize;
   double gx1, gy1, gz1, gx2, gy2, gz2, trx1, try1, trz1, trz2;
   double Length = 10000.0;
   double Declination = 90.0;
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;
   gblock = viewOptions->geophysicsCubeSize;
   msize = floor(geophysicsOptions.calculationRange/gblock);
   
   LinemapXE = absx;
   LinemapYE = absy;
    
   gx1 = viewOptions->originX;
   gy1 = viewOptions->originY;
   gz1 = viewOptions->originZ - viewOptions->lengthZ;
   gx2 = viewOptions->originX + viewOptions->lengthX;
   gy2 = viewOptions->originY + viewOptions->lengthY;
   gz2 = viewOptions->originZ;
   trx1 = gx2;
   try1 = gy2;
   trz1 = gz1;
   trz2 = gz2;


   stepXtopo=(TopomapXE-TopomapXW) / (TopoCol - 1) ;
   stepYtopo=(TopomapYE-TopomapYW) / (TopoRow - 1) ;

                          /*     maptopo[1][1]=      map[1][1];
                                 maptopo[mmax][nmax]=map[71][51];
                                 maptopo[1][nmax]=   map[1][51];
                                 maptopo[mmax][1]=   map[71][1]; */

   if (iLineAnom == 0)  /*Linemap */
   {
                                 /*  stepXtopo=14.285714*iscale;
                                     stepYtopo=14.0*iscale; */
      stepXline=40.0*iscale*BSize;
      stepYline=41.17647*iscale*BSize;

      MapXE=LinemapXE;
      MapYE=LinemapYE;
      MapXW=absx;
      MapYW=absy;
   }
   else if (iLineAnom == 1) /* anomalies */
   {                             
      double mapTopoTop, mapTopoBottom, mapTopoLeft, mapTopoRight;
      
      mapTopoTop = gy1-(gblock*msize);
      mapTopoBottom = gy2+(gblock*msize);
      mapTopoLeft = gx1-(gblock*msize); 
      mapTopoRight = gx2+(gblock*msize); 
      
      overlayTopoOnArray (topographyMap, TopoCol, TopoRow,
                          TopomapYW, TopomapYE, TopomapXW, TopomapXE,
                          maptopo, mmax, nmax,
                          mapTopoTop, mapTopoBottom, mapTopoLeft, mapTopoRight);
      return (TRUE);
   }
   else if (iLineAnom == 2) /* Profiles */
   {
      cosDeclin=cos((double)Declination*3.14159/180.0);
      sinDeclin=sin((double)Declination*3.14159/180.0);

      stepXline=gblock*sinDeclin;
      stepYline=gblock*fabs(cosDeclin);

      MapXW=trx1-msize*gblock*sinDeclin;  /* ? */
      MapXE=trx1 + (Length+msize*gblock)*sinDeclin;

      if (cosDeclin >= 0.0)
      {
         MapYW=try1-msize*gblock*cosDeclin;
         MapYE=try1 + (Length+msize*gblock)*cosDeclin;
      }
      else
      {
         MapYE=try1-msize*gblock*cosDeclin;
         MapYW=try1 + (Length+msize*gblock)*cosDeclin;
      }
   }

   startX = 1 + (int) ((MapXW-TopomapXW)/stepXtopo);
   startY = 1 + (int) ((MapYW-TopomapYW)/stepYtopo);

                                   /* topofile covers whole area */
   if (TopomapXE > MapXE && TopomapYE > MapYE)
   {
      ApproxXY(1,1,mmax,nmax,maptopo,stepXtopo,stepYtopo,
                                     stepXline,stepYline,startX,startY);
   }
   else if (TopomapXE > MapXE && TopomapYE == MapYE )
   {
      ApproxXY(1,1,mmax,nmax-1,maptopo,stepXtopo,stepYtopo,
                                       stepXline,stepYline,startX,startY);
      ApproxX(1,mmax,nmax,maptopo,stepXtopo,stepXline,startX,startY);
   }
   else if ( TopomapXE == MapXE && TopomapYE > MapYE )
   {
      ApproxXY(1,1,mmax-1,nmax,maptopo,stepXtopo,stepYtopo,
                                       stepXline,stepYline,startX,startY);
      ApproxY(1,mmax,nmax,maptopo,stepYtopo,stepYline,startX,startY);
   }
   else if ( TopomapXE == MapXE && TopomapYE == MapYE )
   {
      ApproxXY(1,1,mmax-1,nmax-1,maptopo,stepXtopo,stepYtopo,
                                         stepXline,stepYline,startX,startY);
      maptopo[mmax][nmax]=topographyMap[TopoCol][TopoRow];
      ApproxX(1,mmax-1,nmax,maptopo,stepXtopo,stepXline,startX,startY);
      ApproxY(1,mmax,nmax-1,maptopo,stepYtopo,stepYline,startX,startY);
   }
   return (TRUE);
}

int coordX (in,stX,x)
int in;
double stX,*x;
{
   *x=(in-1)*stX;
   return (TRUE);
}


int ApproxXY (istart,jstart,mmax,nmax,maptopo,
          stepXtopo,stepYtopo,stepXline,stepYline,startX,startY)
int istart,jstart,mmax,nmax;
double **maptopo;
double stepXtopo,stepYtopo,stepXline,stepYline;
int startX,startY;
{
   int jm,jn;
   int bar=0;
   int mless,mmore,nless,nmore;
   double xless,xmore,yless,ymore,x,y;

   for(jm=istart;jm<=mmax;jm++)
   {
      for(jn=jstart;jn<=nmax;jn++)
      {
         mless = startX + (int) ((jm-1)*stepXline/stepXtopo);
                                       /*40.0*BSize/14.285714;*/
         mmore = 1 + mless;
         nless = startY + (int) ((jn-1)*stepYline/stepYtopo);
                                         /*41.176447*BSize/14.0;*/
         nmore=nless+1;
         coordX(jm,stepXline,&x);
         coordX(mless,stepXtopo,&xless);
         coordX(mmore,stepXtopo,&xmore);
         coordX(jn,stepYline,&y);
         coordX(nless,stepYtopo,&yless);
         coordX(nmore,stepYtopo,&ymore);
 
          maptopo[jm][jn]=( (y-yless)*(x-xless)*topographyMap[mmore][nmore]+
                            (y-yless)*(xmore-x)*topographyMap[mless][nmore]+
                            (ymore-y)*(x-xless)*topographyMap[mmore][nless]+
                            (ymore-y)*(xmore-x)*topographyMap[mless][nless])/
                            ((ymore-yless)*(xmore-xless));
      }
   }
   return (TRUE);
}

int ApproxX(istart,mmax,nmax,maptopo,stepXtopo,stepXline,startX,startY)
int istart,mmax,nmax;
double **maptopo,stepXtopo,stepXline;
int startX,startY;
{
   int jm;
   int mless,mmore;
   double xless,xmore,x;

   for (jm=istart;jm<=mmax;jm++)
   {
                      /*mless=1+(jm-1)*40.0*BSize/14.285714;*/
      mless = (int) (startX+(jm-1)*stepXline/stepXtopo);
      mmore = 1+mless;
      coordX(jm,stepXline,&x);
      coordX(mless,stepXtopo,&xless);
      coordX(mmore,stepXtopo,&xmore);

      maptopo[jm][nmax]=( (x-xless)*topographyMap[mmore][TopoRow] +
                          (xmore-x)*topographyMap[mless][TopoRow])
                                                      /(xmore-xless);
   }
   return (TRUE);
}

int ApproxY(jstart,mmax,nmax,maptopo,stepYtopo,stepYline,startX,startY)
int jstart,mmax,nmax;
double **maptopo,stepYtopo,stepYline;
int startX,startY;
{
   int jn;
   int nless,nmore;
   double yless,ymore,y;

   for (jn=jstart;jn<=nmax;jn++)
   {
                        /*nless=1+(jn-1)*41.176447*BSize/14.0;*/
      nless = (int) (startY+(jn-1)*stepYline/stepYtopo);
      nmore=nless+1;
      coordX(jn,stepYline,&y);
      coordX(nless,stepYtopo,&yless);
      coordX(nmore,stepYtopo,&ymore);

      maptopo[mmax][jn]=( (y-yless)*topographyMap[TopoCol][nmore] +
                          (ymore-y)*topographyMap[TopoCol][nless])
                                                      /(ymore-yless);
   }
   return (TRUE);
}

/*
* Calculating valueMapTopo at a single point 
*/
int ApproxDot(xcol,yrow,pvalueMapTopo)
int xcol,yrow;
double *pvalueMapTopo;
{
   int mless,mmore,nless,nmore;
   double xless,xmore,yless,ymore,x,y;
   double stepXtopo,stepYtopo;
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;

   x=((xcol-1)*14.285714*iscale)+absx;
   y=(((yrow-1))*14.0*iscale)+absy;

   stepXtopo=(TopomapXE-TopomapXW) / (TopoCol - 1) ;
   stepYtopo=(TopomapYE-TopomapYW) / (TopoRow - 1) ;

   mless = (int) (1+(x-TopomapXW)/stepXtopo);

   if (mless == TopoCol)
      mless--;

   mmore=1+mless;

   nless = (int) (1+(y-TopomapYW)/stepYtopo);
   if (nless == TopoRow)
      nless--;

   nmore=nless+1;

   coordX(mless,stepXtopo,&xless);
   coordX(mmore,stepXtopo,&xmore);
   coordX(nless,stepYtopo,&yless);
   coordX(nmore,stepYtopo,&ymore);

   *pvalueMapTopo=( (y-yless)*(x-xless)*topographyMap[mmore][nmore]+
                    (y-yless)*(xmore-x)*topographyMap[mless][nmore]+
                    (ymore-y)*(x-xless)*topographyMap[mmore][nless]+
                    (ymore-y)*(xmore-x)*topographyMap[mless][nless])/
                              ((ymore-yless)*(xmore-xless));
   return (TRUE);
}

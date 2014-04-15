/*****************************************************************************
 * File:    3dGrid.c
 *
 * Purpose: General purpose 3d wireframe gridwork, non hidden.
 *
 * ©1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include "3dHier.h"
#include "3dGrid.h"
#include "3dXFDraw.h"
#include "3dFMath.h"

/*****************************************************************************
 *
 * Function:   xfGrid(É)
 *
 * Purpose: General purpose 3d background gridwork.  Options provide for
 *          size limits, connector lines, line patterns, color, etc.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
xfGrid(Point3d mmin,Point3d mmax,Point3d steps,GridOptions options,Matrix3D *xf)
#else
xfGrid(mmin, mmax, steps, options, xf)
Point3d mmin, mmax, steps;
GridOptions options;
Matrix3D *xf;
#endif
{
   Point3d step;
   Fixed f,x,y,z;
   CPEN  penState;
   DRAW_CTOOLS tools;
   
   xvt_dwin_get_draw_ctools (currentWin, &tools);
   penState = tools.pen;

   switch (options.gridPat)
   {
      case GP_WHITE:
         tools.pen.color = COLOR_WHITE;
         break;
      case GP_LTGRAY:
         tools.pen.color = COLOR_LTGRAY;
         break;
      case GP_GRAY:
         tools.pen.color = COLOR_GRAY;
         break;
      case GP_DKGRAY:
         tools.pen.color = COLOR_DKGRAY;
         break;
      case GP_BLACK:
         tools.pen.color = COLOR_BLACK;
         break;
      default:
         tools.pen.color = COLOR_WHITE;
         break;
   }
   xvt_dwin_set_draw_ctools (currentWin, &tools);
   step.x = fixdiv(mmax.x-mmin.x,steps.x);
   step.y = fixdiv(mmax.y-mmin.y,steps.y);
   step.z = fixdiv(mmax.z-mmin.z,steps.z);
   if (options.bottom||options.back||options.top||options.front)
      for(f=mmin.x;f<=mmax.x;f+=step.x)
      {
         if (options.bottom)
            xfDraw3dLine(f,mmin.y,mmin.z,f,mmin.y,mmax.z,xf); /* y const */
         if (options.back)
            xfDraw3dLine(f,mmin.y,mmax.z,f,mmax.y,mmax.z,xf); /* z const */
         if (options.top)
            xfDraw3dLine(f,mmax.y,mmin.z,f,mmax.y,mmax.z,xf); /* y const */
         if (options.front)
            xfDraw3dLine(f,mmin.y,mmin.z,f,mmax.y,mmin.z,xf); /* z const */
      }
   if (options.left||options.back||options.right||options.front)
      for(f=mmin.y;f<=mmax.y;f+=step.y)
      {
         if (options.left)
            xfDraw3dLine(mmin.x,f,mmin.z,mmin.x,f,mmax.z,xf); /* x const */
         if (options.back)
            xfDraw3dLine(mmin.x,f,mmax.z,mmax.x,f,mmax.z,xf); /* z const */
         if (options.right)
            xfDraw3dLine(mmax.x,f,mmin.z,mmax.x,f,mmax.z,xf); /* x const */
         if (options.front)
            xfDraw3dLine(mmin.x,f,mmin.z,mmax.x,f,mmin.z,xf); /* z const */
      }
   if (options.left||options.bottom||options.right||options.top)
      for(f=mmin.z;f<=mmax.z;f+=step.z)
      {  
         if (options.left)
            xfDraw3dLine(mmin.x,mmin.y,f,mmin.x,mmax.y,f,xf); /* x const */
         if (options.bottom)
            xfDraw3dLine(mmin.x,mmin.y,f,mmax.x,mmin.y,f,xf); /* y const */
         if (options.right)
            xfDraw3dLine(mmax.x,mmin.y,f,mmax.x,mmax.y,f,xf); /* x const */
         if (options.top)
            xfDraw3dLine(mmin.x,mmax.y,f,mmax.x,mmax.y,f,xf); /* y const */
      }
   if(options.connectX||options.connectY||options.connectZ)
   {
      switch (options.connectPat)
      {
         case GP_WHITE:
            tools.pen.color = COLOR_WHITE;
            break;
         case GP_LTGRAY:
            tools.pen.color = COLOR_LTGRAY;
            break;
         case GP_GRAY:
            tools.pen.color = COLOR_GRAY;
            break;
         case GP_DKGRAY:
            tools.pen.color = COLOR_DKGRAY;
            break;
         case GP_BLACK:
            tools.pen.color = COLOR_BLACK;
            break;
      }
      xvt_dwin_set_draw_ctools (currentWin, &tools);
   }
   if(options.connectX||options.connectZ)
   {
      for(y=mmin.y+step.y;y<mmax.y;y+=step.y)
      {
         if(options.connectX)
            for(z=mmin.z+step.z;z<mmax.z;z+=step.z)
               xfDraw3dLine(mmin.x,y,z,mmax.x,y,z,xf);
         if(options.connectZ)
            for(x=mmin.x+step.x;x<mmax.x;x+=step.x)
               xfDraw3dLine(x,y,mmin.z,x,y,mmax.z,xf);
      }
   }
   if (options.connectY)
      for(x=mmin.x+step.x;x<mmax.x;x+=step.x)
         for(z=mmin.z+step.z;z<mmax.z;z+=step.z)
            xfDraw3dLine(x,mmin.y,z,x,mmax.y,z,xf);
   
   tools.pen = penState;
   xvt_dwin_set_draw_ctools (currentWin, &tools);
}

#define  FACETLINEX(a,b,c,n,s) \
   {  register int   i = (n);\
      ix[0] = ixV++;\
      pV[ix[0]].p.x = (a);\
      pV[ix[0]].p.y = (b);\
      pV[ix[0]].p.z = (c);\
      pV[ix[0]].impure = FALSE;\
      while( i-- )\
      {  ix[1] = ixV++;\
         pV[ix[1]] = pV[ix[0]];\
         pV[ix[1]].p.x += (s);\
         MakePolyFacet(pF,ixF++,2,ix,pV);\
         ix[0] = ix[1];\
      }\
   }
#define  FACETLINEY(a,b,c,n,s) \
   {  register int   i = (n);\
      ix[0] = ixV++;\
      pV[ix[0]].p.x = (a);\
      pV[ix[0]].p.y = (b);\
      pV[ix[0]].p.z = (c);\
      pV[ix[0]].impure = FALSE;\
      while( i-- )\
      {  ix[1] = ixV++;\
         pV[ix[1]] = pV[ix[0]];\
         pV[ix[1]].p.y += (s);\
         MakePolyFacet(pF,ixF++,2,ix,pV);\
         ix[0] = ix[1];\
      }\
   }
#define  FACETLINEZ(a,b,c,n,s) \
   {  register int   i = (n);\
      ix[0] = ixV++;\
      pV[ix[0]].p.x = (a);\
      pV[ix[0]].p.y = (b);\
      pV[ix[0]].p.z = (c);\
      pV[ix[0]].impure = FALSE;\
      while( i-- )\
      {  ix[1] = ixV++;\
         pV[ix[1]] = pV[ix[0]];\
         pV[ix[1]].p.z += (s);\
         MakePolyFacet(pF,ixF++,2,ix,pV);\
         ix[0] = ix[1];\
      }\
   }


/*****************************************************************************
 *
 * Function:   xfGrid2Patch(É)
 *
 * Purpose: General purpose 3d background gridwork.  Options provide for
 *          size limits, connector lines, line patterns, color, etc.
 *
 *****************************************************************************
 */
int xfGrid2Patch(pG,ixP,mmin,mmax,steps,options,raOptions,xf)
pGroup pG;
int ixP;
Point3d mmin,mmax,steps;
GridOptions options;
RendAttr raOptions;
Matrix3D *xf;
{
   Point3d step;
   Fixed f,x,y,z;
   Vrtx _huge *pV;               /* Vertex working area           */
   Facet _huge *pF;              /* Facet Working Array           */
   int nF=0,nV=0,       /* Number of facets and vertices */
       ixF=0,ixV=0;     /* Current facet and vertex indices */
   int ix[2];           /* Index array for MakePolyFacet */
   int nSX,nSY,nSZ;     /* steps per axis integer        */
   int nX,nY,nZ;
   int err;
   
   raOptions.normalVisibility = FALSE;       /* not practical for lines */
   if( raOptions.R || raOptions.G || raOptions.B )
   {
         /* inherit frame color from whatever was specified */
         /* as the primary color    */
      raOptions.frameColor = XVT_MAKE_COLOR (raOptions.R, raOptions.G,
                               raOptions.B);
   }
   nX = nSX = Fix2Int( steps.x );
   nY = nSY = Fix2Int( steps.y );
   nZ = nSZ = Fix2Int( steps.z );
   steps.x  = Int2Fix( nSX );
   steps.y  = Int2Fix( nSY );
   steps.z  = Int2Fix( nSZ );
   nX++;
   nY++;
   nZ++;
   if( options.top )
      nF += nX*nSZ + nSX*nZ;
   if( options.bottom )
      nF += nX*nSZ + nSX*nZ;
   if( options.left )
      nF += nY*nSZ + nSY*nZ;
   if( options.right )
      nF += nY*nSZ + nSY*nZ;
   if( options.front )
      nF += nX*nSY + nSX*nY;
   if( options.back )
      nF += nX*nSY + nSX*nY;
   nX-=2;
   nY-=2;
   nZ-=2;
   if( options.connectX )
      nF += nSX*nY*nZ;
   if( options.connectY )
      nF += nX*nSY*nZ;
   if( options.connectZ )
      nF += nX*nY*nSZ;
   nV = nF*2;
   pF = (Facet _huge *) xvt_mem_halloc( nF, sizeof(Facet));
   if( !pF )
      return MEMORY_ALLOC_ERROR;
   pV = (Vrtx _huge *) xvt_mem_halloc( nV, sizeof(Vrtx));
   if( !pV )
   {
      xvt_mem_hfree( (char _huge *)pF );
      return MEMORY_ALLOC_ERROR;
   }
   step.x = fixdiv( mmax.x-mmin.x, steps.x );
   step.y = fixdiv( mmax.y-mmin.y, steps.y );
   step.z = fixdiv( mmax.z-mmin.z, steps.z );
   if( options.bottom || options.back || options.top || options.front )
      for( f=mmin.x;f<=mmax.x;f+=step.x )
      {
         if( options.bottom )
            FACETLINEZ( f, mmin.y, mmin.z, nSZ, step.z );
         if( options.top )
            FACETLINEZ( f, mmax.y, mmin.z, nSZ, step.z );
         if( options.back )
            FACETLINEY( f, mmin.y, mmax.z, nSY, step.y );
         if( options.front )
            FACETLINEY( f, mmin.y, mmin.z, nSY, step.y );
      }
   if( options.left || options.back || options.right || options.front )
      for( f=mmin.y;f<=mmax.y;f+=step.y )
      {
         if( options.left )
            FACETLINEZ( mmin.x, f, mmin.z, nSZ, step.z );
         if( options.right )
            FACETLINEZ( mmax.x, f, mmin.z, nSZ, step.z );
         if( options.back )
            FACETLINEX( mmin.x, f, mmax.z, nSX, step.x );
         if( options.front )
            FACETLINEX( mmin.x, f, mmin.z, nSX, step.x );
      }
   if( options.left || options.bottom || options.right || options.top )
      for( f=mmin.z;f<=mmax.z;f+=step.z )
      {  
         if( options.left )
            FACETLINEY( mmin.x, mmin.y, f, nSY, step.y );
         if( options.right )
            FACETLINEY( mmax.x, mmin.y, f, nSY, step.y );
         if( options.bottom )
            FACETLINEX( mmin.x, mmin.y, f, nSX, step.x );
         if( options.top )
            FACETLINEX( mmin.x, mmax.y, f, nSX, step.x );
      }
   if( options.connectX || options.connectZ )
   {
      for( y = mmin.y + step.y; y < mmax.y; y += step.y )
      {
         if( options.connectX )
            for( z = mmin.z + step.z; z < mmax.z; z += step.z )
               FACETLINEX( mmin.x, y, z, nSX, step.x );
         if( options.connectZ )
            for( x = mmin.x + step.x; x < mmax.x; x += step.x )
               FACETLINEZ( x, y, mmin.z, nSZ, step.z );
      }
   }
   if( options.connectY )
      for( x = mmin.x + step.x; x < mmax.x; x += step.x )
         for( z = mmin.z + step.z; z< mmax.z; z += step.z )
            FACETLINEY( x, mmin.y, z, nSY, step.y );
   
   err = NewPatch( pG, ixP, ixV, pV, ixF, pF, raOptions, xf );
   xvt_mem_hfree( (char _huge *)pF );
   xvt_mem_hfree( (char _huge *)pV );
   return err;
}

/*****************************************************************************
 *
 * Function:   xfLines2Patch(É)
 *
 * Purpose: General purpose 3d Line storage at patch/facet level.
 *
 * Returns: INVALID_REQUEST      if odd number of points and not disjoint.
 *          MEMORY_ALLOC_ERROR   if any trouble allocating memory occurs.
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
xfLines2Patch(pGroup pG, int ixP, BOOLEAN disjoint, int nPts,
      Point3d *pPt, RendAttr raOptions, Matrix3D *xf)
#else
xfLines2Patch(pG, ixP, disjoint, nPts, pPt, raOptions, xf)
pGroup pG;
int ixP;
BOOLEAN disjoint;
int nPts;
Point3d *pPt;
RendAttr raOptions;
Matrix3D *xf;
#endif
{
   Vrtx _huge *pV;               /* Vertex working area           */
   Facet _huge *pF;              /* Facet Working Array           */
   int nF=0,nV=0,       /* Number of facets and vertices */
      ixF=0,ixV=0;      /* Current facet and vertex indices */
   int ix[2];           /* Index array for MakePolyFacet */
   int err;
   
   if( (nPts % 2) && !disjoint )
      return   INVALID_REQUEST;

   raOptions.normalVisibility = FALSE;       /* not practical for lines */
   if( raOptions.R || raOptions.G || raOptions.B )
   {
         /* inherit frame color from whatever was specified */
         /* as the primary color    */
      raOptions.frameColor = XVT_MAKE_COLOR (raOptions.R, raOptions.G,
                               raOptions.B);
   }
   
   nF = (disjoint)? nPts/2 : nPts;
   
   nV = nPts;
   pF = (Facet _huge *) xvt_mem_halloc(nF, sizeof(Facet));
   if( !pF )
      return MEMORY_ALLOC_ERROR;
   pV = (Vrtx _huge *) xvt_mem_halloc(nV, sizeof(Vrtx));
   if( !pV )
   {
      xvt_mem_hfree( (char _huge *)pF );
      return MEMORY_ALLOC_ERROR;
   }
   ixV = 0;
   while(ixV<nPts)
   {
      pV[ixV].p = pPt[ixV];
      pV[ixV].impure = FALSE;
      ixV++;
   }
   
   if(disjoint)
   {
      ixV = 0;
      while(ixV<nPts)
      {
         ix[0] = ixV++;
         ix[1] = ixV++;
         MakePolyFacet(pF, ixF++,2,ix,pV);
      }
   }
   else
   {
      ixV = 0;
      ix[0] = ixV++;
      while(ixV<nPts)
      {
         ix[1] = ixV++;
         MakePolyFacet(pF ,ixF++,2,ix,pV);
         ix[0] = ix[1];
      }
      ix[1] = 0;
      MakePolyFacet(pF, ixF++,2,ix,pV);
   }
   
   err = NewPatch( pG, ixP, ixV, pV, ixF, pF, raOptions, xf );
   xvt_mem_hfree( (char _huge  *)pF );
   xvt_mem_hfree( (char _huge *)pV );
   return err;
}

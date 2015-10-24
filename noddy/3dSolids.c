/*****************************************************************************
 * File:    Solids.c
 *
 * Purpose: Renders hierarchical collections of faceted solids
 *
 * © 1989 Mark M. Owen. All rights reserved.
 *
 *****************************************************************************
 */

#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include "3dHier.h"
#include "3dSolids.h"
#include "3dClip.h"
#include "3dFMath.h"


typedef  struct
{
   pPatch pP;  /* handle to surface patch    */
   int ixP;    /* index into patch array     */
   int ixF;    /* facet index hP[ixP] relative  */
   int nIx;    /* facet geometry (# of indices) */
   Fixed dist; /* distance, centroid to viewer  */
   int xfIx;      /* index of instance matrix      */
} SortFacet, *pSortFacet, **hSortFacet;

/*
 * Explanation of the visibility checking macros:
 * ----------- -- --- ---------- -------- -------
 *
 * given 3 co-planar points a, b and c the two macros below
 * are derived from the following sequence of operations:
 * 
 * using point a as the origin calculate two vectors in the
 * plane thereby represented:
 * 
 * ¶x = b.x - a.x;
 * ¶y = b.y - a.y;
 * ¶z = b.z - a.z;
 * 
 * ¶X = c.x - a.x;
 * ¶Y = c.y - a.y;
 * ¶Z = c.z - a.z;
 * 
 * the plane's normal vector N would be:
 * 
 * N.x = ¶y*¶Z - ¶Y*¶z;
 * N.y = ¶X*¶z - ¶x*¶Z;
 * N.z = ¶x*¶Y - ¶X*¶y;
 * 
 * however, since the viewer normal V is [0,0,-1], we
 * can simplify the above to ¶x*¶y - ¶X*¶Y because:
 * 
 *    0   = N.x * V.x
 *    0   = N.y * V.y
 *    -N.z = N.z * V.z
 *
 * and the dot product of V¥N is equal to -N, the sum
 * of the above components.
 * 
 * since our viewer normal V is [0,0,-1] rather than
 * a line of sight from the facet to the viewer, the
 * a facet is visible if the dot product ² 0, and is
 * invisible otherwise.
 */
#define  Visible(a,b,c)\
      (  (  fixmul( ( (b).x-(a).x ) , ( (c).y-(a).y ) )\
         -  fixmul( ( (c).x-(a).x ) , ( (b).y-(a).y ) )\
         )\
         <= 0.00\
      )
#define  Invisible(a,b,c) !Visible((a),(b),(c))  

              
#if XVT_CC_PROTO
int getRegionBounds (PNT *, RCT *);
BOOLEAN isRectInRegion (RCT *, PNT *);
int regionSize (PNT *);
static void DepthSort (SortFacet __huge *, int);
static SortFacet __huge *allocPSF (int, SortFacet __huge *, int);
static Matrix3D *allocXF (int, Matrix3D *, int);
static void RenderPolyFacet (Matrix3D*,pSortFacet,pLighting,BOOLEAN,WINDOW);
static void RenderRectFacet (Matrix3D*,pSortFacet,pLighting,BOOLEAN,WINDOW);
static BOOLEAN InterruptEvent (void);
#else
int getRegionBounds ();
BOOLEAN isRectInRegion ();
int regionSize ();
static void DepthSort ();
static SortFacet __huge *allocPSF ();
static Matrix3D *allocXF ();
static void RenderPolyFacet ();
static void RenderRectFacet ();
static BOOLEAN InterruptEvent ();
#endif

/*****************************************************************************
 *
 * Function:   getRegionBounds
 *
 * Purpose: get a bounding box for a region
 *
 * Returns: the num points in the region, bounding box in rect
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
getRegionBounds (PNT *region, RCT *regionBound)
#else
getRegionBounds (region, regionBound)
PNT *region;
RCT *regionBound;
#endif
{
   int pnt, size = regionSize (region);

   xvt_rect_set (regionBound, region[0].h, region[0].v, region[0].h, region[0].v);
   for (pnt = 1; pnt < size; pnt++)
   {
      if (region[pnt].h < regionBound->left)
         regionBound->left = region[pnt].h;
      else if (region[pnt].h > regionBound->right)
         regionBound->right = region[pnt].h;

      if (region[pnt].v < regionBound->top)
         regionBound->top = region[pnt].v;
      else if (region[pnt].v > regionBound->bottom)
         regionBound->bottom = region[pnt].v;
   }
   return (size);
}                   

/*****************************************************************************
 *
 * Function:   isRectInRegion
 *
 * Purpose: see if the rectange and the region intersect at any point
 *
 * Returns: TRUE if they intersect, FALSE otherwize
 *
 *****************************************************************************
 */
BOOLEAN
#if XVT_CC_PROTO
isRectInRegion (RCT *rect, PNT *region)
#else
isRectInRegion (rect, region)
RCT *rect;
PNT *region;
#endif
{
   int pnt, size = regionSize (region);
   RCT regionBound;                                 

   xvt_rect_set (&regionBound, region[0].h, region[0].v, region[0].h, region[0].v);
   for (pnt = 1; pnt < size; pnt++)
   {
      if (region[pnt].h < regionBound.left)
         regionBound.left = region[pnt].h;
      else if (region[pnt].h > regionBound.right)
         regionBound.right = region[pnt].h;

      if (region[pnt].v < regionBound.top)
         regionBound.top = region[pnt].v;
      else if (region[pnt].v > regionBound.bottom)
         regionBound.bottom = region[pnt].v;
   }
   regionBound.bottom -= 1;
   regionBound.top += 1;
   regionBound.left += 1;
   regionBound.right -= 1;
   
   return (xvt_rect_intersect (NULL, rect, &regionBound));
}                   

/*****************************************************************************
 *
 * Function:   regionSize
 *
 * Purpose: work out how many points are in a region structure
 *
 * Returns: the number of points
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
regionSize (PNT *region)
#else
regionSize (region)
PNT *region;
#endif
{
   int size;                                 
   
   if (!region)
      return (0);
   
   for (size = 1;
         ((region[size].h != region[0].h) || (region[size].v != region[0].v));
                                                            size++)
      ; /* do nothing */
                         
   return (size+1);
}                   

/*****************************************************************************
 *
 * Function:   InterruptEvent()
 *
 * Purpose: Polls SystemTask and checks for any event in the designated
 *          INTERRUPT_MASK at periodic intervals.
 *
 * Returns: TRUE  if an event of interest is available
 *          FALSE if either not time to check, or nothing of interest
 *
 * Warnings:   Private Function
 *
 *****************************************************************************
 */
static BOOLEAN    InterruptEvent()
{
   xvt_app_process_pending_events ();
   return FALSE;
}                   

/*****************************************************************************
 *
 * Function:   DepthSort(É)
 *
 * Purpose: Sorts an array of facets into ascending distance order.
 *
 * Methods: Adaptation of Shell-Metzner algorithm
 *
 * Warnings:   Private Function
 *
 *****************************************************************************
 */
static void DepthSort(pSF, n)
register SortFacet __huge *pSF;
register int n;
{
   register int      i,jump;
   register BOOLEAN  done;
   SortFacet         temp;
   register pSortFacet  pSFi,pSFj;

   jump = n;
   done = FALSE;
   while( jump )
   {
      jump>>=1;
      do
      {  done = TRUE;
         for( i = 0, pSFi = pSF; i < (n-jump); i++, pSFi++ )
         {
            pSFj = &pSF[i + jump];
            if(    pSFi->dist > pSFj->dist )
            {  temp  = *pSFi;
               *pSFi = *pSFj;
               *pSFj = temp;
               done  = FALSE;
            }
         }
      } while( !done );
   }
}

/****************************************************************************
 *
 * Function:   Distance(É)
 *
 * Purpose: Calculates the magnitude of a vector.
 *
 * Returns: Fixed Point magnitude of vector.
 *
 *****************************************************************************
 */
Fixed Distance(c)
Point3d c;
{
   register FPType x, y, z;
   FPType d;

   x = c.x; /*0.0000152587890625 is 1 / 65535 */
   y = c.y;
   z = c.z;
   d = sqrt(x*x+y*y+z*z);
   return (d);
}

/*****************************************************************************
 *
 * Function:   allocPSF(É)
 *
 * Purpose: Allocates storage for SortFacet array entries.
 *
 * Returns: Handle to block or nil if unsuccessfull
 *
 * Warnings:   Private Function
 *          On failure to allocate, any prior allocations are disposed of.
 *          
 *****************************************************************************
 */
static SortFacet __huge *allocPSF(need, pSF, nInUse)
int need;
SortFacet __huge *pSF;
int nInUse;
{
   static long nAlloc;
   int avail;
   SortFacet __huge *pSFtemp;
   
   if ( pSF )  /* accumulation in progress - grow the block if necessary */
   {
      avail = (int) (nAlloc - nInUse);
      need = (int) (avail < need)? need - avail : 0;
      if( need )
      {
         nAlloc += need;
         pSFtemp = (SortFacet _huge *) xvt_mem_halloc (nAlloc, sizeof(SortFacet));
         if( pSFtemp )
         {
            hmemcpy((char __huge *) pSFtemp, (char __huge *) pSF, nInUse*sizeof(SortFacet));
            xvt_mem_hfree( (char __huge *) pSF );
         }
      }
      else
         pSFtemp = pSF;
   }
   else
   {
      pSFtemp = (SortFacet __huge *) xvt_mem_halloc (need, sizeof(SortFacet));
      nAlloc = need;
   }
   return pSFtemp;
}

/*****************************************************************************
 *
 * Function:   allocXF(É)
 *
 * Purpose: Allocates storage for a transform matrix array.
 *
 * Returns: Pointer to block or nil if unsuccessfull
 *
 * Warnings:   Private Function
 *          On failure to allocate, any prior allocations are disposed of.
 *          
 *****************************************************************************
 */
static Matrix3D  *allocXF(need,pMat,nInUse)
int need;
Matrix3D *pMat;
int nInUse;
{
   static long nAllocated;
   int size, avail;
   Matrix3D *pMatTemp;
   
   if( pMat )  /* accumulation in progress - grow the block if necessary */
   {
      avail = (int) (nAllocated - nInUse);
      need = (int) (avail < need)? need - avail : 0;
      if( need )
      {
         nAllocated += need;
         size = (int) (nAllocated * sizeof(Matrix3D));
         pMatTemp = (Matrix3D*)xvt_mem_alloc( (size_t) size );
         if( pMatTemp )
         {
            memcpy((char *)pMatTemp, (char *)pMat, nInUse*sizeof(Matrix3D) );
            xvt_mem_free( (char *) pMat );
         }
      }
      else
         pMatTemp = pMat;
   }
   else
   {
      pMatTemp = (Matrix3D*)xvt_mem_alloc( sizeof(Matrix3D) * need );
      nAllocated  = need;
   }
   return pMatTemp;
}

/*****************************************************************************
 *
 * Function:   RenderPolyFacet(É)
 *
 * Purpose: Controls the process of rendering a polygonal facet.  Wire
 *          frame only, points and lines are drawn here.  Area filled
 *          facets are drawn by ColorRgn.
 *
 * Warnings:   Private Function
 *          
 *****************************************************************************
 */
static void
#if XVT_CC_PROTO
RenderPolyFacet(Matrix3D *pXF, pSortFacet pSF, pLighting pL,
            BOOLEAN bWireFrameOnly, WINDOW currentPort)
#else
RenderPolyFacet(pXF, pSF, pL, bWireFrameOnly, currentPort)
Matrix3D *pXF;
pSortFacet pSF;
pLighting pL;
BOOLEAN bWireFrameOnly;
WINDOW currentPort;
#endif
{
   pPatch pP;
   pVrtx pV,pcV;
   pFacet pF;
   int ixP,i,nIx,cix,ix0,ix1;
   Point3d *p,*pt;
   Point3d a,b,N;
   Point2d *s;
   PNT *S;
   PNT *rhFacet;
   Point3d centroid;
   BOOLEAN incomplete = FALSE;
   RendAttr *raOptions;

   incrementLongJob (-1);

   /* Lock and dereference the various handles for simplicity
    * Patch Handle, Vertex Handle and Facet Handle
    */
   ixP = pSF->ixP;
   pP = pSF->pP;
   pV = pP[ixP].pV;
   pF = pP[ixP].pF;
   raOptions = &pP[ixP].raOptions;
   
   i  = nIx = pF[pSF->ixF].nIx;
   
   p  = (Point3d*)xvt_mem_alloc( sizeof(Point3d)*i ); /* 3d Transformed Points(Fixed)  */
   s  = (Point2d*)xvt_mem_alloc( sizeof(Point2d)*i ); /* 2d Projection Points (Fixed)  */
   S  = (PNT *) xvt_mem_alloc( sizeof(PNT)*i ); /* 2d Projection Points (Int) */

   if( !p || !s || !S )
   {
      /* Problems allocating enough space, so clean up and
       * exit quietly.
       */
      if ( p )
         xvt_mem_free( (char *)p );
      if ( s )
         xvt_mem_free( (char *)s );
      if ( S )
         xvt_mem_free( (char *)S );
      return;
   }

   while(i--)
   {
      /* Isolate the vertex index from the SortFacet entry
       */
      cix = pF[pSF->ixF].ixVrtx[i];
      if( i == 0 )
         ix0 = cix;
      else
      if( i == 1 )
         ix1 = cix;
      pcV = &pV[cix];
      if( !pcV->impure )
      {  /*
          * This vertex has not yet been transformed, do it now.
          */
         ApplyTransform( &pcV->p, &pcV->p, &pXF[pSF->xfIx] );
         pcV->impure = TRUE;
      }
      
      /* Isolate the vertex
       */
      p[i] = pcV->p;
      
      /* On other than the last vertex in the polygon, check for
       * clipped visibility and terminate if not visible.
       */
      if(i < nIx-1)  /* not last */
         if ( (incomplete = ( ClipLine( &p[i], &p[i+1] ) == isnt_visible ) ) )
            break;

      /* Project to 2d space and convert to screen coordinates
       */
      Project( &p[i], &s[i] );
      S[i].h = Fix2Int( s[i].x );
      S[i].v = Fix2Int( s[i].y );
   }

   /* Clean up memory allocations no longer needed
    */
   xvt_mem_free ( (char *)s );

   /* Final visibility check in 2d space since perspective
    * projection will eliminate some facets from view.
    */
   if(   incomplete
      || (  ( nIx > 2 ) /* more complex than point or line */
         && ( raOptions->normalVisibility?
               Invisible( s[0], s[1], s[2] )
            :  FALSE
            )
         )
     )
   {
      xvt_mem_free ( (char *)S );
      xvt_mem_free ( (char *)p );
      return;
   }

   if (  nIx < 3 )                  /* point or line only */
   {
      COLOR theForeColor;
      DRAW_CTOOLS tools;

      if ( ColorPresent )
      {
         xvt_dwin_get_draw_ctools (currentWin, &tools);
         theForeColor = tools.pen.color;
         tools.pen.color = raOptions->frameColor;
         tools.brush.color = raOptions->frameColor;
      }
      MoveTo( S[0].h, S[0].v );
      i = 0;
      while( ++i < nIx )
         LineTo( S[i].h, S[i].v );
      LineTo( S[0].h, S[0].v );
      if ( ColorPresent )
      {
         tools.pen.color = theForeColor;
         tools.brush.color = theForeColor;
      }
      xvt_mem_free ( (char *)S);
      xvt_mem_free ( (char *)p);
      return;
   }


   /* Create a region from the projected and clipped facet's
    * line segments, if possible.
    */
   if ((nIx > 2) &&
         (rhFacet = (PNT *) xvt_mem_alloc ( sizeof (PNT) * (nIx+1))))
   {
      for (i = 0; i < nIx; i++)
      {
         rhFacet[i].h = S[i].h;
         rhFacet[i].v = S[i].v;
      }
      rhFacet[i].h = S[0].h;  /* Close in the Region */
      rhFacet[i].v = S[0].v;
   }
   else
      rhFacet = NULL;

      
   /* Check for a legitimate, complete region
    */
   if ( rhFacet )
   {       
      RCT frame;
      
      xvt_vobj_get_client_rect (currentWin, &frame);
      if ( !isRectInRegion( &frame, rhFacet) )
      {  /*
          * nowhere near visible, so forget itÉ
          */
         xvt_mem_free ( (char *) rhFacet );
         xvt_mem_free ( (char *)S);
         xvt_mem_free ( (char *)p);
         return;
      }

      /* If light sources are present and WireFrameOnly drawing has
       * not been requested we'll do the complete rendering.  
       * Otherwise, we may just draw the wire frame (see below).
       */
      if ( pL && !bWireFrameOnly )
      {
         /* Transform the facet's centroid to viewer space
          */
         ApplyTransform( &pF[pSF->ixF].centroid, &centroid, &pXF[pSF->xfIx] );

         /* Compute the facet's normal vector at the centroid
          */
         pt = &pV[ix1].p;
         a.x = pt->x - centroid.x;
         a.y = pt->y - centroid.y;
         a.z = pt->z - centroid.z;
         pt = &pV[ix0].p;
         b.x = pt->x - centroid.x;
         b.y = pt->y - centroid.y;
         b.z = pt->z - centroid.z;

         N.x   =  fixmul(a.y,b.z)-fixmul(b.y,a.z);
         N.y   = -fixmul(a.x,b.z)+fixmul(b.x,a.z);
         N.z   =  fixmul(a.x,b.y)-fixmul(b.x,a.y);

         /* Call ColorRgn to do the real work of evaluating the
          * lighting effects on the facet.
          */
         ColorRgn(centroid,N,pSF->dist,raOptions,rhFacet,pL);
      }
      else    /* Erase the Region */
      {
         DRAW_CTOOLS tools;
         COLOR theForeColor;
         
         xvt_dwin_get_draw_ctools (currentWin, &tools);
         theForeColor = tools.pen.color;
         tools.pen.color = COLOR_BLACK;
         tools.brush.color = COLOR_BLACK;
         xvt_dwin_set_draw_ctools (currentWin, &tools);
         
         xvt_dwin_draw_polygon (currentWin, rhFacet, regionSize(rhFacet));

         tools.pen.color = theForeColor;
         tools.brush.color = theForeColor;
         xvt_dwin_set_draw_ctools (currentWin, &tools);
      }

      /* If no light sources, or facet framing is requested (either via
       * the rendering attributes (raOptions) or WireFrameOnly we will
       * draw the region's outline here (a bit faster than FrameRgn).
       */
      if( raOptions->framed || bWireFrameOnly )
      {
         COLOR theForeColor;
         DRAW_CTOOLS tools;
         
         if( ColorPresent )
         {
            xvt_dwin_get_draw_ctools (currentWin, &tools);
            theForeColor = tools.pen.color;
            tools.pen.color = raOptions->frameColor;
            tools.brush.color = raOptions->frameColor;
            xvt_dwin_set_draw_ctools (currentWin, &tools);
         }
         MoveTo(S[0].h,S[0].v);
         i=0;
         while(++i < nIx)
            LineTo(S[i].h,S[i].v);
         LineTo(S[0].h,S[0].v);
         if( ColorPresent )
         {
            tools.pen.color = theForeColor;
            tools.brush.color = theForeColor;
            xvt_dwin_set_draw_ctools (currentWin, &tools);
         }
      }
   }
   /* Clean up remaining memory allocations now
    */
   xvt_mem_free ( (char *)rhFacet);
   xvt_mem_free ( (char *)S);
   xvt_mem_free ( (char *)p);
}

/*****************************************************************************
 *
 * Function:   RenderRectFacet(É)
 *
 * Purpose: Controls the process of rendering a rectangular facet.  Wire
 *          frame only drawing occurs here, if enabled.  Area fills are
 *          done in ColorRgn.
 *
 * Warnings:   Private Function
 *          
 *****************************************************************************
 */
static void
#if XVT_CC_PROTO
RenderRectFacet(Matrix3D *pXF, pSortFacet pSF, pLighting pL,
                  BOOLEAN bWireFrameOnly, WINDOW currentPort)
#else
RenderRectFacet(pXF, pSF, pL, bWireFrameOnly, currentPort)
Matrix3D *pXF;
pSortFacet pSF;
pLighting pL;
BOOLEAN bWireFrameOnly;
WINDOW currentPort;
#endif
{
   pPatch pP;
   pVrtx pV;
   pFacet pF;
   Facet F;
   int ixP;
   Point3d a,b,N;
   Point2d s0,s1,s2,s3;
   PNT S0,S1,S2,S3;
   PNT *rhFacet;
   RendAttr *raOptions;
   RCT frame;
      

   register pVrtx    pv0,pv1,pv2,pv3;
   register Matrix3D*  pxf;
   

   incrementLongJob (-1);

   /* Lock and dereference the various handles for simplicity
    * Patch Handle, Vertex Handle and Facet Handle
    */
   ixP   = pSF->ixP;
   pP = pSF->pP;
   pV = pP[ixP].pV;
   pF = pP[ixP].pF;
   F = pF[pSF->ixF];
   raOptions = &(pP[ixP].raOptions);

   /* Isolate the vertices via the Facet and SortFacet 
    */
   pv0 = &pV[F.ixVrtx[0]];
   pv1 = &pV[F.ixVrtx[1]];
   pv2 = &pV[F.ixVrtx[2]];
   pv3 = &pV[F.ixVrtx[3]];
   pxf = &pXF[pSF->xfIx];

   /* Check for any vertices as yet untransformed, and transform
    * them now.
    */
   if( !pv0->impure )
   {
      ApplyTransform( &pv0->p, &pv0->p, pxf );
      pv0->impure = TRUE;
   }
   if( !pv1->impure )
   {
      ApplyTransform( &pv1->p, &pv1->p, pxf );
      pv1->impure = TRUE;
   }
   if( !pv2->impure )
   {
      ApplyTransform( &pv2->p, &pv2->p, pxf );
      pv2->impure = TRUE;
   }
   if( !pv3->impure )
   {
      ApplyTransform( &pv3->p, &pv3->p, pxf );
      pv3->impure = TRUE;
   }
   
   /* If either of the two primary line segments are not visible
    * after clipping, we can't draw this correctly, so exit here.
    */
   if( ( ClipLine(&pv0->p,&pv1->p) == isnt_visible ) ||
       ( ClipLine(&pv2->p,&pv3->p) == isnt_visible ) )
   {
      return;
   }
   
   /* Project to 2d Space and screen coordinates
    */
   Project( &pv0->p, &s0 );
   Project( &pv1->p, &s1 );
   Project( &pv2->p, &s2 );
   Project( &pv3->p, &s3 );

   /* final visibility check in 2d space since perspective
    * projection may eliminate some facets from view.
    */
   if( ( raOptions->normalVisibility? Invisible( s0, s2, s3 ) : FALSE ) )
   {
      return;
   }

   /* Convert from Fixed to integer representation
    */
   S0.h = Fix2Int( s0.x );
   S0.v = Fix2Int( s0.y );
   S1.h = Fix2Int( s1.x );
   S1.v = Fix2Int( s1.y );
   S2.h = Fix2Int( s2.x );
   S2.v = Fix2Int( s2.y );
   S3.h = Fix2Int( s3.x );
   S3.v = Fix2Int( s3.y );

   /* Create a region from the projected and clipped facet's
    * line segments, if possible.
    */
   if (rhFacet = (PNT *) xvt_mem_alloc ( sizeof(PNT) * 5))
   {
      rhFacet[0].h = S0.h; rhFacet[0].v = S0.v;
      rhFacet[1].h = S1.h; rhFacet[1].v = S1.v;
      rhFacet[2].h = S2.h; rhFacet[2].v = S2.v;
      rhFacet[3].h = S3.h; rhFacet[3].v = S3.v;
      rhFacet[4].h = S0.h; rhFacet[4].v = S0.v;
   }
   else
      rhFacet = NULL;
        
   xvt_vobj_get_client_rect (currentWin, &frame);
      if ( !isRectInRegion( &frame, rhFacet) )
   {  /*
       * nowhere near visible, so forget itÉ
       */
      xvt_mem_free ( (char *) rhFacet );
      return;
   }


   /* Check for a legitimate, complete region
    */
   if ( rhFacet )
   {
      /* If light sources are present and WireFrameOnly drawing has
       * not been requested we'll do the complete rendering.  
       * Otherwise, we may just draw the wire frame (see below).
       */
      if ( pL && !bWireFrameOnly )
      {
         /* Transform the facet's centroid to viewer space
          */
         ApplyTransform( &F.centroid, &F.centroid, pxf );

         /* Compute the facet's normal vector at the centroid
          */
         a.x = pv3->p.x - F.centroid.x;
         a.y = pv3->p.y - F.centroid.y;
         a.z = pv3->p.z - F.centroid.z;
         
         b.x = pv2->p.x - F.centroid.x;
         b.y = pv2->p.y - F.centroid.y;
         b.z = pv2->p.z - F.centroid.z;

         N.x   =  fixmul(a.y,b.z)-fixmul(b.y,a.z);
         N.y   = -fixmul(a.x,b.z)+fixmul(b.x,a.z);
         N.z   =  fixmul(a.x,b.y)-fixmul(b.x,a.y);
         
         /* Call ColorRgn to do the real work of evaluating the
          * lighting effects on the facet.
          */
         ColorRgn( F.centroid, N, pSF->dist, raOptions, rhFacet, pL );
      }
      else    /* Erase the Region */
      {
         DRAW_CTOOLS tools, copyTools;
         
         xvt_dwin_get_draw_ctools (currentWin, &tools);
         copyTools = tools;
         tools.pen.pat = PAT_HOLLOW;
         tools.brush.color = COLOR_BLACK;
         xvt_dwin_set_draw_ctools (currentWin, &tools);
         
         xvt_dwin_draw_polygon (currentWin, rhFacet, regionSize(rhFacet));

         xvt_dwin_set_draw_ctools (currentWin, &copyTools);
      }

      /* If no light sources, or facet framing is requested (either via
       * the rendering attributes (raOptions) or WireFrameOnly we will
       * draw the region's outline here (a bit faster than FrameRgn).
       */
      if( raOptions->framed || bWireFrameOnly )
      {
         DRAW_CTOOLS tools, copyTools;
         
         if( ColorPresent )
         {
            xvt_dwin_get_draw_ctools (currentWin, &tools);
            copyTools = tools;
            tools.pen.color = raOptions->frameColor;
            xvt_dwin_set_draw_ctools(currentWin, &tools);
         }
         MoveTo( S0.h, S0.v );
         LineTo( S1.h, S1.v );
         LineTo( S2.h, S2.v );
         LineTo( S3.h, S3.v );
         LineTo( S0.h, S0.v );
         if( ColorPresent )
            xvt_dwin_set_draw_ctools (currentWin, &copyTools);
      }
   }
   /* Dispose of the region's memory allocation
    */
   xvt_mem_free ( (char *) rhFacet );
}

/*****************************************************************************
 *
 * Function:   xfRenderGroup(É)
 *
 * Purpose: Controls the process of rendering a group of surface patches
 *          comprised of facets.
 *
 * Returns: INVALID_REQUEST      if nil group handle or transform matrix
 *          MEMORY_ALLOC_ERROR   if problems allocating memory
 *          noErr          if successful or interrupted by events
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
xfRenderGroup(pGroup pG, Matrix3D *xf, pLighting pL, BOOLEAN bWireFrameOnly)
#else
xfRenderGroup(pG, xf, pL, bWireFrameOnly)
pGroup pG;
Matrix3D *xf;
pLighting pL;
BOOLEAN bWireFrameOnly;
#endif
{
   pPatch pP;
   Facet __huge *pF;
   int nS=0;
   int nP,nF;
   SortFacet __huge *pSF = NULL;
   SortFacet   SF;
   Point3d c;
   int currentXF = 0;
   Matrix3D *pXF = 0L;
   int nXF=0;
   Point2d s;
   PNT S;
   RCT roughClip;
            

   if( !pG || ! xf )
      return INVALID_REQUEST;

   xvt_vobj_get_client_rect (currentWin, &roughClip);
   
   nP = pG->nP;
   if(!( pXF = allocXF( nP+1, pXF, nXF+1 ) ) )
   {
      return   MEMORY_ALLOC_ERROR;
   }

   pXF[0] = *xf;  /* set up the supplied transform */

   while( nP-- /* && !InterruptEvent() */ )
   {
      pP = pG->pP;
      if( !pP )
         continue;
      nF = pP[nP].nF;
      if( !( pSF = allocPSF( nF, pSF, nS ) ) )
      {
         return   MEMORY_ALLOC_ERROR;
      }
      pF = pP[nP].pF;
      if( !pF || !pP[nP].pV )
         continue;

      currentXF = 0;
      if(pP[nP].instanced )
      {
         mMult4x4( &pP[nP].instanceXf, xf, &pXF[++nXF] );
         currentXF = nXF;
      }

      while( nF-- /* && !InterruptEvent() */ )
      {
         c = pF[nF].centroid;
         ApplyTransform( &c, &c, &pXF[currentXF] );

         {  /*
             * Initial clipping (is centroid near the portRect?
             * If not, skip this facet completely.
             */
            Project( &c, &s );
            S.h = Fix2Int(s.x);
            S.v = Fix2Int(s.y);
            if( !xvt_rect_has_point(&roughClip, S ) )
               continue;
         }
         if( ClipPt( &c ) == is_visible )
         {
            SF.dist = Distance( c );
            SF.pP = pP;
            SF.ixP = nP;
            SF.ixF = nF;
            SF.nIx = pF[nF].nIx;
            SF.xfIx = currentXF;
            pSF[nS++]   = SF;
         }
      }
   }
   DepthSort( pSF, nS );
   while( nS-- /* && !InterruptEvent() */ ) /* draw back to front */
      if( pSF[nS].nIx == 4 )
         RenderRectFacet(pXF, &pSF[nS], pL, bWireFrameOnly, currentWin);
      else
         RenderPolyFacet(pXF, &pSF[nS], pL, bWireFrameOnly, currentWin);
   xvt_mem_hfree( (char *) pSF );
   xvt_mem_free( (char *) pXF );

   return TRUE;
}

/*****************************************************************************
 *
 * Function:   xfRenderCollection(É)
 *
 * Purpose: Controls the process of rendering a collection of groups of
 *          surface patches comprised of facets.
 *
 * Returns: INVALID_REQUEST      if nil collection handle or matrix
 *          MEMORY_ALLOC_ERROR   if problems allocating memory
 *          noErr          if successfull
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
xfRenderCollection(pCollection pC, Matrix3D *xf,
                        pLighting pL, BOOLEAN bWireFrameOnly)
#else
xfRenderCollection(pC, xf, pL, bWireFrameOnly)
pCollection pC;
Matrix3D *xf;
pLighting pL;
BOOLEAN bWireFrameOnly;
#endif
{
   pGroup pG;
   pPatch pP;
   Facet __huge *pF;
   int nS = 0;
   int nG, nP, nF;
   SortFacet __huge *pSF = NULL;
   SortFacet   SF;
   Point3d c;
   int currentXF=0;
   Matrix3D *pXF=0L;
   Point2d s;
   PNT S;
   RCT roughClip;
               
   int nXF=0;
   
   if( !pC || !xf )
      return INVALID_REQUEST;
   
   xvt_vobj_get_client_rect (currentWin , &roughClip);

   nG = pC->nG;
   while( nG-- /* && !InterruptEvent() */ )
   {
      pG = (pC->pG)[nG];
      if( !pG )
         continue;
      nP = pG->nP;
      if(!( pXF = allocXF( nP+1, pXF, nXF+1 ) ) )
      {
         if( pSF )
            xvt_mem_hfree ( (char *) pSF );
         return   MEMORY_ALLOC_ERROR;
      }

      pXF[0] = *xf;  /* set up the supplied transform */

      while( nP-- /* && !InterruptEvent() */ )
      {
         pP = pG->pP;
         if( !pP )
            continue;
         nF = pP[nP].nF;
         if( !( pSF = allocPSF( nF, pSF, nS ) ) )
         {                    
            return MEMORY_ALLOC_ERROR;
         }
         pF = pP[nP].pF;
         if( !pF || !pP[nP].pV )
            continue;

         currentXF = 0;
         if(pP[nP].instanced )
         {
            mMult4x4( &pP[nP].instanceXf, xf, &pXF[++nXF] );
            currentXF = nXF;
         }

         while( nF-- /* && !InterruptEvent() */ )
         {
            c = pF[nF].centroid;
            ApplyTransform( &c, &c, &pXF[currentXF] );

            {  /*
                * Initial clipping (is centroid near the portRect?
                * If not, skip this facet completely.
                */
               Project( &c, &s );
               S.h = Fix2Int(s.x);
               S.v = Fix2Int(s.y);
               if ( !xvt_rect_has_point (&roughClip, S) )
                  continue;
            }

            if( ClipPt( &c ) == is_visible )
            {
               SF.dist     = Distance( c );
               SF.pP       = pP;
               SF.ixP      = nP;
               SF.ixF      = nF;
               SF.nIx      = pF[nF].nIx;
               SF.xfIx     = currentXF;
               pSF[nS++]   = SF;
            }
         }
      }
   }
   DepthSort( pSF, nS );
   while( nS-- /* && !InterruptEvent() */ ) /* draw back to front */
      if( pSF[nS].nIx == 4 )
         RenderRectFacet( pXF, &pSF[nS], pL, bWireFrameOnly, currentWin );
      else
         RenderPolyFacet( pXF, &pSF[nS], pL, bWireFrameOnly, currentWin );
   xvt_mem_hfree ( (char *) pSF );
   xvt_mem_free ( (char *) pXF );

   return TRUE;
}


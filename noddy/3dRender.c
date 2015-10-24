/*****************************************************************************
 * File:    Render.c
 *
 * Purpose: ZBuffer Rendering Functions.
 *
 * © 1991 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#if (XVTWS == MAXWS)
#else
#include <limits.h>
#endif
#include <math.h>
#include "3dHier.h"
#include "3dTextur.h"
#include "3dAntiA.h"
#include "3dFMath.h"

#define  CRSR_ZEROPCT   128

#ifdef PAUL
typedef struct edgeBox
{
   int x;            /* horizontal screen coordinate  */
   FPPoint3d N,         /* normal vector           */
         P,       /* viewer transformed point      */
         R;       /* raw (instanced only) point    */
   struct edgeBox *prior;  /* prior list element, if not nil   */
} edgeBox, *pEdgeBox;

typedef struct
{
   FPRGBColor color;          /* object color            */
   FPRGBColor amb;            /* ambient light color        */
   FPType vd;              /* distance to view point     */
   FPType Ks;              /* specular reflection coefficient  */
   FPType Kd;              /* diffusion coefficient         */
   FPType specIndex;          /* specular reflection index     */
   FPType transparency;       /* transparency coefficient      */
   enum textures  texture;       /* texture mapping types, if any */
   FPType arg[TXLIMIT][TXARGS];     /*texture mapping parameters     */
   pLighting pL;              /* ambient and point light sources  */
   
} LightInfo, *pLightInfo;


typedef struct polyList
{
   PNT *pp;             /* polygon with data extension   */
   struct polyList *prior;    /* prior list element, if not nil   */

} polyList, *pPolyList;

short nPolys   = 0;
pPolyList pPL = 0L;     /* list of active polygons */

static BOOLEAN shadows = FALSE;
static BOOLEAN antiAliasing = FALSE;
static long MINH,MAXH,MINV,MAXV;
static PNT *phCurrent;

#define NewP(q,kind)       (kind*)xvt_mem_alloc((long)(q)*sizeof(kind))
#define NewPClr(q,kind)       (kind*)NewPtrClr((long)(q)*sizeof(kind))
#define DisposeP(p)           if((p))xvt_mem_free((char *) (p))
#define abs(x)             (((x)<0)?-x:x)
#define INC(a,b)           {(a).x+=(b).x;(a).y+=(b).y;(a).z+=(b).z;}
#define delta(d,a,b,r)        (d)=((b)-(a))*(r);
#define DELTA(d,a,b,r)        {delta((d).x,(a).x,(b).x,(r));delta((d).y,(a).y,(b).y,(r));delta((d).z,(a).z,(b).z,(r));}
#define FPFix(fx)          ((fx)*0.0000152587890625)
#define P3dToFP3d(p3d,fp3d)      {(fp3d).x=FPFix((p3d).x);(fp3d).y=FPFix((p3d).y);(fp3d).z=FPFix((p3d).z);}
#define distance(c)           sqrt((c).x*(c).x+(c).y*(c).y+(c).z*(c).z)
#define DotProduct(a,b)       (a).x*(b).x+(a).y*(b).y+(a).z*(b).z
#define ClipColor(c)       (((c)>255.0)?255:(((c)<0)?0:(colorFactor)(c)))
#define Visible(a,b,c)        ((fixmul(((b).x-(a).x),((c).y-(a).y))-fixmul(((c).x-(a).x),((b).y-(a).y))) <= 0.0)
#define Invisible(a,b,c)      !Visible((a),(b),(c))
#define GetPixel(a,b,c)       GetCPixel((a),(b),(c))
#define SetPixel(a,b,c)       SetCPixel((a),(b),(c))
#define BoxMinMax(c,box,a,b)  if((a).##c<(b).##c){(box).min.c=(a).##c;(box).max.c=(b).##c;}else{(box).min.c=(b).##c;(box).max.c=(a).##c;}
#define ReflectColor(v,c,r,d) (c).##v = (r).##v*(c).##v*(d)
#define BoxIntersects(a,b)\
(  max((a).min.x,(b).min.x)<=min((a).max.x,(b).max.x)\
&& max((a).min.y,(b).min.y)<=min((a).max.y,(b).max.y)\
&& max((a).min.z,(b).min.z)<=min((a).max.z,(b).max.z)\
)
#define  PointInBox(p,b)\
(  (p).x>=(b).min.x && (p).y>=(b).min.y && (p).z>=(b).min.z\
&& (p).x<=(b).max.x && (p).y<=(b).max.y && (p).z<=(b).max.z\
)



static char *NewPtrClr (long size);
static int AddEdge (pPolyExt pPE,pEdgeBox *pEL,long yMin,long yMax,PNT p0,pVN vnA,PNT p1,pVN vnB);
static void DisposePEB (pEdgeBox pEB);
static void RenderSegment (int y,pEdgeBox pb1,pEdgeBox pb2,pLighting pL,pRendAttr pRA,pZBuffer pZB);
static void AntialiasEdge (int y,pEdgeBox pb1,pEdgeBox pb2,pZBuffer pZB);
static FPRGBColor EvaluateLight (pPolyList pPL,FPPoint3d P,FPPoint3d N,pLightInfo pLI,FPRGBColor *rgb);
static BOOLEAN Adjacent (pPolyExt pPEA,pPolyExt pPEB);
static BOOLEAN InShadow (pPolyList pPL,FPPoint3d P,FPType ld,pLtSource pLS,FPType *transmission);



/**********************************************************************************
 *
 * Function:   NewPtrClr
 *
 * Purpose: Allocates a zero filled, non-relocatable block of memory.
 *
 * Returns: pointer to the memory allocated or nil if the memory could not be
 *          allocated.  Call MemError() for further info in the nil case.
 *
 **********************************************************************************
 */
static char *NewPtrClr(long size)
{
   return xvt_mem_alloc( (size_t) size );
}

/**********************************************************************************
 *
 * Function:   SetShadows
 *
 * Purpose: Sets the shadows global variable to indicate if shadow rendering is
 *          to be performed. Shadow rendering must be selected prior to calling
 *          xfRenderCollectionPgn if shadows are to be generated.
 *
 **********************************************************************************
 */
void  SetShadows(BOOLEAN state)
{
   shadows = state;
}

/**********************************************************************************
 *
 * Function:   Shadows
 *
 * Returns: the state of the shadows global variable.
 *
 **********************************************************************************
 */
BOOLEAN  Shadows()
{
   return   shadows;
}

/**********************************************************************************
 *
 * Function:   SetAntiAliasing
 *
 * Purpose: Sets the antiAliasing global variable to indicate if edge pixels
 *          should be blended in with the background as they are drawn.
 *
 **********************************************************************************
 */
void  SetAntiAliasing(BOOLEAN state)
{
   antiAliasing = state;
}

/**********************************************************************************
 *
 * Function:   AntiAliasing
 *
 * Returns: the state of the antiAliasing global variable.
 *
 **********************************************************************************
 */
BOOLEAN  AntiAliasing()
{
   return antiAliasing;
}

/**********************************************************************************
 *
 * Function:   AddToPolyList
 *
 * Purpose: Adds a polygon with data extension to the list of active polygons,
 *          provided memory can be allocated.
 *
 **********************************************************************************
 */
void  AddToPolyList(PNT *pp)
{
   pPolyList ppl = (pPolyList) xvt_mem_alloc( sizeof(polyList) );
   if( !ppl )
      return;
   ppl->pp = pp;
   ppl->prior = pPL;
   pPL = ppl;
   
   nPolys++;
}

/**********************************************************************************
 *
 * Function:   DisposePolyList
 *
 * Purpose: Disposes of all polygon with data extensions in the list of active
 *          polygons.
 *
 **********************************************************************************
 */
void  DisposePolyList()
{
   pPolyList ppl = pPL,pplt;

   while( ppl )
   {
      KillPolygon( ppl->pp );
      pplt = ppl;
      ppl = ppl->prior;
      xvt_mem_free( (char *)pplt );
   }
   pPL = 0L;
   
   nPolys = 0;
}

/**********************************************************************************
 *
 * Function:   RenderPolyList
 *
 * Purpose: Renders all polygons with data extensions in the active list.
 *
 **********************************************************************************
 */
void  RenderPolyList(BOOLEAN edgesOnly)
{
   pPolyList ppl = pPL;

/*
** if( AntiAliasing() && edgesOnly )
**    HideCursor();  .* assure we don't blend the cursor into the scene *.
*/
   {
      short crsId = CRSR_ZEROPCT;
      short inc = nPolys/8;
      short count = 0;
      
      while( ppl )
      {
         RenderPolygon( ppl->pp, edgesOnly );
         ppl = ppl->prior;

      }
   }
/*
** if( AntiAliasing() && edgesOnly )   
**    ShowCursor();
*/
}

/**********************************************************************************
 *
 * Function:   NewPolygon
 *
 * Purpose: Creates a polygon structure with an extension appended to it which
 *          carries the data necessary to render the polygon using Z buffered
 *          Phong style shading, texture mapping, etc.  The additional data
 *          includes: lighting, rendering attributes, geometric elements such 
 *          as viewer distance, centroid, normal vector, etc. and a pointer to
 *          a Z buffer for hidden surface removal and/or compositing purposes.
 *
 * Returns: Handle to the extended polygon or nil if memory allocation problems
 *          occur (in which case MemError() may be called for more information).
 *
 **********************************************************************************
 */

PNT *NewPolygon
   (  int nSrcPts,      /* number of projected screen points      */
      PNT *srcPts,      /* array of projected screen points    */
      pLighting   pL,      /* ambient and point light sources        */
      pRendAttr   pRA,     /* rendering attributes             */
      FPType dist,      /* viewer distance (the Z depth)       */
      FPPoint3d   *centroid,/* center of the polygon (3d)            */
      FPPoint3d   *normal, /* vector normal to the polygon        */
      FPPoint3d   *V,      /* array of viewer transformed vertices      */
      FPPoint3d   *N,      /* array of vertex normal vectors         */
      FPPoint3d   *R,      /* array of raw (instanced only) vertices */
      pZBuffer pZB      /* destination Z buffer for hidden surfaces  */
   )
{
   register int n = nSrcPts,i = 0;
   register long sizePoly,sizePolyExt;
   register PNT src;
   register PNT *pS=srcPts,*pD;
   register pVN pvn;
   FPPoint3d *pv=V;
   FPPoint3d *pn=N;
   FPPoint3d *pr=R;

   PNT *h;
   
   RCT box = {INT_MAX,INT_MAX,INT_MIN,INT_MIN};
   FPPoint3d   minPt = {LONG_MAX,LONG_MAX,LONG_MAX},
         maxPt = {LONG_MIN,LONG_MIN,LONG_MIN};

   sizePoly = sizeof(PNT)*(n+1);
   sizePolyExt = sizeof(VN)*(n+1) + sizeof(polyExt);
   h = (PNT *) xvt_mem_alloc(sizePoly+sizePolyExt);
   if( h )
   {
      pPolyExt pPE;
      
      /* Locate the data extension
       */
      pPE = (pPolyExt)((char *)(h) + sizePoly);
      pPE->pZB = pZB;
      pPE->pL = pL;
      pPE->RA = *pRA;
      pPE->d = dist;
      pPE->c = *centroid;
      pPE->n = *normal;
      pPE->np = n+1;
      pD = h;
      pvn = pPE->vn;
      while(n--)
      {
         src = *pS++;
         /* Accumulate the 2d bounding box
          */
         if( src.h<box.left ) box.left = src.h;
         if( src.h>box.right )   box.right   = src.h;
         if( src.v<box.top )     box.top     = src.v;
         if( src.v>box.bottom )  box.bottom  = src.v;
         *pD++ = src;
         pvn->v = *pv;
         pvn->n = *pn;
         pvn->r = *pr;
         if( i++ )
         {
            (pvn-1)->d.x = pvn->v.x - (pvn-1)->v.x;
            (pvn-1)->d.y = pvn->v.y - (pvn-1)->v.y;
            (pvn-1)->d.z = pvn->v.z - (pvn-1)->v.z;
         }
         pv++;
         pn++;
         pr++;
         /* Accumulate the 3d bounding box
          */
         if( pvn->v.x<minPt.x ) minPt.x = pvn->v.x;
         if( pvn->v.x>maxPt.x ) maxPt.x = pvn->v.x;
         if( pvn->v.y<minPt.y ) minPt.y = pvn->v.y;
         if( pvn->v.y>maxPt.y ) maxPt.y = pvn->v.y;
         if( pvn->v.z<minPt.z ) minPt.z = pvn->v.z;
         if( pvn->v.z>maxPt.z ) maxPt.z = pvn->v.z;
         pvn++;
      }
      *pD = *srcPts;
      pvn->v = *V;
      pvn->n = *N;
      pvn->r = *R;
      (pvn-1)->d.x = pvn->v.x - (pvn-1)->v.x;
      (pvn-1)->d.y = pvn->v.y - (pvn-1)->v.y;
      (pvn-1)->d.z = pvn->v.z - (pvn-1)->v.z;
      pvn->d = pPE->vn->d;
      pPE->bounds.min = minPt;
      pPE->bounds.max = maxPt;
   }
   return h;
}

/**********************************************************************************
 *
 * Function:   KillPolygon
 *
 * Purpose: Disposes of an extended polygon structure.
 *
 **********************************************************************************
 */
void KillPolygon(PNT *ph)
{
   xvt_mem_free((char *) ph);
}

/**********************************************************************************
 *
 * Function:   RenderPolygon
 *
 * Purpose: Determines if an extended polygon is visible, and if so sets up any
 *          texture mapping parameters and reduces the polygon to an edge list.
 *          The edge list is then processed, rendering the pixels of each scan
 *          line.  Throughout the process, the handle to the extended polygon
 *          remains locked.  It is then unlocked just prior to exit from this
 *          function.
 *
 * Returns: noErr if successfull; or MemError() if a memory problem occurs.
 *
 **********************************************************************************
 */
int RenderPolygon(PNT *ph, BOOLEAN edgesOnly)
{

   register long y, ix;
   register pVN pvn;
   register PNT *pp;
   register pEdgeBox *pEL;
   int regionSize;
   RCT regionBounds;
   
   pPolyExt pPE;
   pLighting pL;
   pRendAttr pRA;
   pZBuffer pZB;
   int err;

   if( !ph )
      return FALSE;
   
   pPE   = (pPolyExt) (ph + 1);
   pZB = pPE->pZB;
   pL = pPE->pL;
   pRA   = &pPE->RA;
   pvn   = pPE->vn;
   pp = ph;
   
   /* Intersect Z buffer and polygon bounding boxes
    */                                  
   regionSize = getRegionBounds(ph, &regionBounds);
   MINV= max(regionBounds.top, pZB->yMn );
   MAXV= min(regionBounds.bottom, pZB->yMx );
   MINH= max(regionBounds.left, pZB->xMn );
   MAXH= min(regionBounds.right, pZB->xMx );
   if( MAXV <= MINV || MAXH <= MINH )
   {  /* clipped out of existance, omit itÉ
      */
      return TRUE;
   }

   if( pRA->normalVisibility  )
   {
      Point2d  s[3];
      s[0].x = Int2Fix(pp[0].h);
      s[0].y = Int2Fix(pp[0].v);
      s[1].x = Int2Fix(pp[1].h);
      s[1].y = Int2Fix(pp[1].v);
      s[2].x = Int2Fix(pp[2].h);
      s[2].y = Int2Fix(pp[2].v);
      if( Invisible( s[0], s[1], s[2] ) )
      {  /* projected normal points away from the viewer, omit itÉ
         */
         return TRUE;
      }
   }
   
   phCurrent = ph;

   if( !(AntiAliasing() && edgesOnly) )   /* not needed if just antialiasing */
   {
      if( pRA->texture & TX_WOOD )
         WoodTextureTransform( pRA->tx[TXI_WOOD].txRotations, pRA->tx[TXI_WOOD].txScalars, pRA->tx[TXI_WOOD].txTranslations );
      if( pRA->texture & TX_ROCK )
         RockTextureTransform( pRA->tx[TXI_ROCK].txRotations, pRA->tx[TXI_ROCK].txScalars, pRA->tx[TXI_ROCK].txTranslations );
      if( pRA->texture & TX_RGB )
         RGBTextureTransform( pRA->tx[TXI_RGB].txRotations, pRA->tx[TXI_RGB].txScalars, pRA->tx[TXI_RGB].txTranslations );
      if( pRA->texture & TX_BUMPS )
         BumpsTextureTransform( pRA->tx[TXI_BUMPS].txRotations, pRA->tx[TXI_BUMPS].txScalars, pRA->tx[TXI_BUMPS].txTranslations );
      if( pRA->texture & TX_DEPTH )
         DepthTextureTransform( pRA->tx[TXI_DEPTH].txRotations, pRA->tx[TXI_DEPTH].txScalars, pRA->tx[TXI_DEPTH].txTranslations );
      if( pRA->texture & TX_PICT )
      {
         PICTinit( FPFix(pRA->tx[TXI_PICT].txArgument[0]), &pPE->bounds );
         if(      pRA->tx[TXI_PICT].txScalars.x
            && pRA->tx[TXI_PICT].txScalars.y
            && pRA->tx[TXI_PICT].txScalars.z
           )
            PICTTextureTransform
               (  pRA->tx[TXI_PICT].txRotations,
                  pRA->tx[TXI_PICT].txScalars,
                  pRA->tx[TXI_PICT].txTranslations
               );
      }
   }

   /* Allocate scan line arrayÉ
    */
   if(!(pEL = NewPClr(MAXV-MINV+1,pEdgeBox)) )
      return FALSE;
   
   /* Add all of the edgesÉ
    */
   ix = pPE->np - 1;
   while(ix--)
   {
      if( (err = AddEdge(pPE,pEL,MINV,MAXV,*pp,pvn,*(pp+1),pvn+1)) )
      {
         for( y = MINV; y<=MAXV; y++ )
            DisposePEB(pEL[y-MINV]);
         DisposeP(pEL);
         return   err;
      }
      pvn++;
      pp++;
   }
   
   /* Add the last edge alsoÉ (closure to initial point)
    */
   if( (err = AddEdge(pPE,pEL,MINV,MAXV,*pp,pvn,*ph,pPE->vn)) )
   {
      for( y = MINV; y<=MAXV; y++ )
         DisposePEB(pEL[y-MINV]);
      DisposeP(pEL);
      return   err;
   }
   
   /* Draw the scan lines
    */
   if( AntiAliasing() && edgesOnly )
   {
      for( y = MINV; y<=MAXV; y++ )
         if(pEL[y-MINV])
            AntialiasEdge(y,pEL[y-MINV],pEL[y-MINV]->prior,pZB);
   }
   else
   {
      for( y = MINV; y<=MAXV; y++ )
         if(pEL[y-MINV])
            RenderSegment(y,pEL[y-MINV],pEL[y-MINV]->prior,pL,pRA,pZB);
   }
      
   /* Clean up our memory mess
    */
   for( y = MINV; y<=MAXV; y++ )
      DisposePEB(pEL[y-MINV]);
   DisposeP(pEL);

   if( !(AntiAliasing() && edgesOnly) )   /* not needed if just antialiasing */
      if( pRA->texture & TX_PICT )
         PICTterm();

   return TRUE;
}

/**********************************************************************************
 *
 * Function:   AddEdge
 *
 * Purpose: Builds a list of edge endpoints for an extended polygon, linked in
 *          a manner which allows it to be processed by scan line (Y). The edge
 *          endpoints are clipped here to the vertical extents of the Z buffer.
 *
 * Returns: noErr if successfull; or MemError() if a memory problem occurs.
 *
 **********************************************************************************
 */
static int AddEdge(pPolyExt pPE, pEdgeBox *pEL, long yMin, long yMax,
                        PNT p0, pVN vnA, PNT p1, pVN vnB)
{
   register FPType r,x,dx;
   register int Iy1,Iy2,y,yMn,oy;
   register pEdgeBox pEB;

   FPPoint3d   A,Na,Ra;
   FPPoint3d   B,Nb,Rb;
   FPPoint3d   N,dN;
   FPPoint3d   P,dP; 
   FPPoint3d   R,dR;

   if( p0.v == p1.v )
      return TRUE;

   A  = vnA->v;
   Na = vnA->n;
   Ra = vnA->r;
   B  = vnB->v;
   Nb = vnB->n;
   Rb = vnB->r;
   
   if( p0.v > p1.v )
   {  /* swap */
      PNT p;
      p  = p0; P = A;   N  = Na; R  = Ra;
      p0 = p1; A = B;   Na = Nb; Ra = Rb;
      p1 = p;  B = P;   Nb = N;  Rb = R;
   }
   
   if( pPE->RA.specIndex == 0 )
   {
      Na = Nb = pPE->n; /* not specular so make it flat */
   }

   Iy1   = p0.v;
   Iy2   = p1.v;

   r = 1.0 / (Iy2 - Iy1);
   N = Na;
   P = A;
   R = Ra;
   x = p0.h;
   DELTA(dN,N,Nb,r);
   DELTA(dP,P,B,r);
   DELTA(dR,R,Rb,r);
   delta(dx,x,p1.h,r);
   y = max(Iy1,yMin);   /* forcing -y to zero here */
   oy = abs(y-Iy1);     /* how much offset, if any?   */
   x += dx* oy;         /* adjust items as needed. */
   N.x += dN.x * oy;
   N.y += dN.y * oy;
   N.z += dN.z * oy;
   P.x += dP.x * oy;
   P.y += dP.y * oy;
   P.z += dP.z * oy;
   R.x += dR.x * oy;
   R.y += dR.y * oy;
   R.z += dR.z * oy;
   yMn   = yMin;
   Iy2 = min(Iy2,yMax);
   while( y<Iy2 )
   {
      if(! (pEB = NewP(1,edgeBox)) )
         return FALSE;
      pEB->x      = (int)(x+0.5);
      pEB->N      = N;
      pEB->P      = P;
      pEB->R      = R;
      pEB->prior  = pEL[y-yMn];
      pEL[y-yMn]  = pEB;
      y++;
      x += dx;
      INC(N,dN);
      INC(P,dP);
      INC(R,dR);
   }
   return TRUE;
}

/**********************************************************************************
 *
 * Function:   DisposePEB
 *
 * Purpose: Disposes of edge list elements.
 *
 **********************************************************************************
 */
static void DisposePEB(pEdgeBox pEB)
{
   register pEdgeBox peb;

   while(pEB)
   {
      peb = pEB->prior;
      DisposeP(pEB);
      pEB = peb;
   }
}           

/**********************************************************************************
 *
 * Function:   RenderSegment
 *
 * Purpose: Draws a series of adjacent pixels on a scan line.  Drawing proceeds
 *          left to right.  Lighting effects and rendering attributes are 
 *          evaluated to determine the color of each pixel, through calls to
 *          appropriate subordinate functions.
 *
 **********************************************************************************
 */
static void RenderSegment
   (  int y,         /* current scan line (vertical coordinate)      */
      pEdgeBox pb1,     /* scan line horizontal end point information   */
      pEdgeBox pb2,     /* scan line horizontal end point information   */
      pLighting   pL,      /* ambient and point light source information   */
      pRendAttr   pRA,     /* rendering attributes of the polygon    */
      pZBuffer pZB      /* destination Z buffer             */
   )
{
   register int Ix1,Ix2,x,xMx,ox;
   register pEdgeBox peb1 = pb1,
               peb2 = pb2;
   register FPType r;

   FPPoint3d   dN,N,dP,P,R,dR,n;
   LightInfo   LI;
   COLOR rgb;
   FPRGBColor C;
   BOOLEAN edgePixel;
   

   if( !peb1 || !peb2 )
      return;
   if( peb1->x == peb2->x )
      return;

   if( peb1->x > peb2->x )
   {
      pEdgeBox pb = peb1;
      peb1 = peb2;
      peb2 = pb;
   }
   if( peb1->x > pZB->xMx || peb2->x < pZB->xMn )  /* out of window bounds */
      return;

   LI.Kd       = FPFix(pRA->diffusion);
   LI.Ks       = FPFix(pRA->specularity);
   LI.specIndex   = pRA->specIndex;
   LI.pL       = pL;

   if( pRA->ambientEffect )
   {
      register FPType I = (pL->level*pRA->ambience)*3.552713677E-15;
      LI.amb.red  = I * XVT_COLOR_GET_RED(pL->color);
      LI.amb.green   = I * XVT_COLOR_GET_GREEN(pL->color);
      LI.amb.blue = I * XVT_COLOR_GET_BLUE(pL->color);
   }
   else
   {
      LI.amb.red = 0.0;
      LI.amb.green = 0.0;
      LI.amb.blue = 0.0;
   }
   
   LI.texture = pRA->texture;
   LI.transparency = FPFix(pRA->transparency);

   if( pRA->texture )
   {  int t;
   
      for(t=0;t<TXLIMIT;t++)
         for(x=0; x<TXARGS; x++)
            LI.arg[t][x]   = FPFix(pRA->tx[t].txArgument[x]);
   }
   
   Ix1 = peb1->x;
   Ix2 = peb2->x;

   r  = 1.0 / (peb2->x - peb1->x);
   N = peb1->N;
   P = peb1->P;
   R = peb1->R;
   DELTA(dN,N,peb2->N,r);
   DELTA(dP,P,peb2->P,r);
   DELTA(dR,R,peb2->R,r);

   x = max(Ix1,pZB->xMn);  /* forcing -x to zero here */
   ox = abs(x-Ix1);     /* how much offset, if any?   */
   N.x += dN.x * ox;    /* adjust items as needed. */
   N.y += dN.y * ox;
   N.z += dN.z * ox;
   P.x += dP.x * ox;
   P.y += dP.y * ox;
   P.z += dP.z * ox;
   R.x += dR.x * ox;
   R.y += dR.y * ox;
   R.z += dR.z * ox;
   
   xMx = min(Ix2,pZB->xMx);

   while( x < xMx )
   {
      edgePixel =    (x == Ix1)
               || (x == Ix2)
               || (y == MINV && MINV != pZB->yMn)  /* don't do edges generated by clipping */
               || (y == MAXV && MAXV != pZB->yMx);/* to the window boundaries          */
      if( P.z > 0.0 )
      {
         LI.vd = distance( P );
         if( SetZB( pZB, x, y, (long)(65536.0*LI.vd) ) )
         {
            LI.vd = (pRA->distanceEffect)? 1.0/LI.vd:1.0;

            if( pRA->framed && edgePixel )
            {
               LI.color.red   = XVT_COLOR_GET_RED(pRA->frameColor);
               LI.color.green = XVT_COLOR_GET_GREEN(pRA->frameColor);
               LI.color.blue  = XVT_COLOR_GET_BLUE(pRA->frameColor);
            }
            else
            {
               LI.color.red   = pRA->R;
               LI.color.green = pRA->G;
               LI.color.blue  = pRA->B;
            }

            C.red = C.green = C.blue = 0;

            if( LI.texture & TX_BUMPS )
               n = PerturbNormal( P.x, P.y, P.z, N, LI.arg[TXI_BUMPS][0], LI.arg[TXI_BUMPS][1], LI.arg[TXI_BUMPS][2] );
            else
               n = N;

            C = EvaluateLight( pPL, P, n, &LI, &C );

            if( LI.texture )
            {
               if( LI.texture & TX_WOOD )
                  LI.color = Wood(R.x,R.y,R.z,LI.color,LI.arg[TXI_WOOD][0],LI.arg[TXI_WOOD][1],LI.arg[TXI_WOOD][2]);
               if( LI.texture & TX_ROCK )
                  LI.color = Rock(R.x,R.y,R.z,LI.color,LI.arg[TXI_ROCK][0],LI.arg[TXI_ROCK][1],LI.arg[TXI_ROCK][2],LI.arg[TXI_ROCK][3]);
               if( LI.texture & TX_PICT )
                  LI.color = PICT(R.x,R.y,R.z,LI.color,LI.arg[TXI_PICT][1]);
               if( LI.texture & TX_RGB )
                  LI.color = R_G_B(R.x,R.y,R.z,LI.color,LI.arg[TXI_RGB][0]);
               if( LI.texture & TX_DEPTH )
                  LI.color = Depth(R.x,R.y,R.z,LI.color,LI.arg[TXI_DEPTH][0],LI.arg[TXI_DEPTH][1]);
            }

            ReflectColor(red,  C,LI.color,LI.vd);
            ReflectColor(green,C,LI.color,LI.vd);
            ReflectColor(blue, C,LI.color,LI.vd);

            C.red =(C.red  + LI.amb.red  )*255.0;
            C.green  =(C.green+ LI.amb.green)*255.0;
            C.blue   =(C.blue + LI.amb.blue )*255.0;
            
            if( pRA->transparency > 0.0 )
            {
               GetPixel( x, y, &rgb );
               C = Transparency( &rgb, &C, FPFix(pRA->transparency) );
            }

            rgb = XVT_MAKE_COLOR(ClipColor(C.red), ClipColor(C.green),
                                 ClipColor(C.blue));

            SetPixel( x, y, &rgb );
         }
      }
      x++;
      INC(N,dN);
      INC(P,dP);
      INC(R,dR);
   }
}

/**********************************************************************************
 *
 * Function:   AntialiasEdge
 *
 * Purpose: Blends the edge pixels into their surroundings.
 *
 **********************************************************************************
 */
static void AntialiasEdge
   (
      int         y,       /* current scan line (vertical coordinate)      */
      pEdgeBox pb1,     /* scan line horizontal end point information   */
      pEdgeBox pb2,     /* scan line horizontal end point information   */
      pZBuffer pZB         /* destination Z buffer                */
   )
{
   register int Ix1,Ix2,x,xMx,ox;
   register pEdgeBox peb1 = pb1,
               peb2 = pb2;
   register FPType   r;
   
   FPPoint3d   dP,P;
   COLOR rgb,c;
   

   if( !peb1 || !peb2 )
      return;
   if( peb1->x == peb2->x )
      return;

   if( peb1->x > peb2->x )
   {
      pEdgeBox pb = peb1;
      peb1 = peb2;
      peb2 = pb;
   }
   if( peb1->x > pZB->xMx || peb2->x < pZB->xMn )  /* out of window bounds */
      return;
   
   Ix1 = peb1->x;
   Ix2 = peb2->x;

   r  = 1.0 / (peb2->x - peb1->x);
   P = peb1->P;
   DELTA(dP,P,peb2->P,r);

   x = max(Ix1,pZB->xMn);  /* forcing -x to zero here */
   ox = abs(x-Ix1);     /* how much offset, if any?   */
   P.x += dP.x * ox;
   P.y += dP.y * ox;
   P.z += dP.z * ox;
   
   xMx = min(Ix2,pZB->xMx);

   while( x < xMx )
   {
      if(      (P.z > 0.0)
         && (     (x == Ix1)
               || (x >= Ix2-1)
               || (y == MINV&&MINV!=pZB->yMn)
               || (y >= MAXV-1&&MAXV!=pZB->yMx)
            )
        )
         if( SetZB( pZB, x, y, (long)(65536.0*distance(P)) ) )
         {
            GetQueuePixel( x,y, &c );
            rgb = ConvolutionColor(x-1,y,W1,W2,W3);   /* left */
            SetQueuePixel( x-1, y, &rgb );
            rgb = ConvolutionColor(x+1,y,W1,W2,W3);   /* right */
            SetQueuePixel( x+1, y, &rgb );
            ConvoluteColorPixel(x,y,W1,W2,W3, c);  /* this */
         }
      x++;
      INC(P,dP);
   }
}

/**********************************************************************************
 *
 * Function:   EvaluateLight
 *
 * Purpose: Determines the effect of multiple light sources and the rendering
 *          attributes at a given point.  If shadow rendering is in process,
 *          the active polygon list is processed to determine if the point is
 *          in shadow relative to each light source, and if so the transmission
 *          factor (transparency) of the intersected polygon is used to reduce
 *          the effect of the light source.  The basic light model used here is
 *          the Phong model, extended to include Warn's method of modeling light
 *          sources of variable geometry; colored light sources and colored
 *          specular hilites.  The light source distance intensity function used
 *          is 1/d rather than 1/(d*d) since the latter tends to produce very
 *          dark images and is difficult to control.
 *
 * Returns: Reflected light color at the specified point.
 *
 **********************************************************************************
 */
static FPRGBColor EvaluateLight
   (
      pPolyList   pPL,  /* active polygon list                       */
      FPPoint3d   P,    /* current transformed 3d point                 */
      FPPoint3d   N,    /* current normal vector                     */
      pLightInfo  pLI,  /* lighting and attribute information           */
      FPRGBColor  *rgb  /* initial pixel color                       */
   )
{
   register int nS;
   register pLtSource   pLS;
   register pLightInfo  pli;
   register FPType I = 0.0, LN,ld,rld,Spec,Shape;
   
   FPType transmission;
   FPPoint3d lv;
   FPRGBColor C;
   
   pli= pLI;
   nS = pli->pL->nSources;
   pLS= pli->pL->source;
   
   C.red = pli->amb.red + rgb->red;
   C.green  = pli->amb.green+ rgb->green;
   C.blue   = pli->amb.blue   + rgb->blue;

   while(nS--)
   {
      if( !pLS->level )
      {
         pLS++;
         continue;
      }
      lv.x = P.x - pLS->x;
      lv.y = P.y - pLS->y;
      lv.z = P.z - pLS->z;
      ld = distance(lv);
      rld = (ld==0.0)? 0.0: (1.0 / ld);
      lv.x *= rld;
      lv.y *= rld;
      lv.z *= rld;
      LN = DotProduct(lv,N);
      if( LN > 0.0 )
      {
         I = 1.0;
         if( shadows )
            if( InShadow( pPL,P,ld,pLS,&transmission ) )
               if( transmission == 0.0 )
               {  pLS++;
                  continue;
               }
               else
                  I = transmission;
         Shape = (pLS->shape==0.0)? 1.0
            : pow(pLS->nx*-lv.x+pLS->ny*-lv.y+pLS->nz*-lv.z,pLS->shape);
         Shape = (Shape<0.0)? -Shape : Shape;
         Spec = (pli->specIndex==0.0)? 0.0
            : pli->Ks * pow((N.x*(N.x+lv.x)+N.y*(N.y+lv.y)+N.z*(N.z+lv.z))*0.5,pli->specIndex);
         I *= ( FPFix(pLS->level) * Shape * ( pli->Kd * LN + Spec ) ) * rld;
         I *= 0.0000152590218966;
         C.red  += I * XVT_COLOR_GET_RED(pLS->color);
         C.green+= I * XVT_COLOR_GET_GREEN(pLS->color);
         C.blue += I * XVT_COLOR_GET_BLUE(pLS->color);
      }
      pLS++;
   }
   return C;
}

/**********************************************************************************
 *
 * Function:   Adjacent
 *
 * Purpose: Determines if two polygons have a vertex in common.
 *
 * Returns: TRUE if there is a vertex in common; FALSE otherwise.
 *
 **********************************************************************************
 */
static BOOLEAN Adjacent( pPolyExt pPEA, pPolyExt pPEB )
{
   register pVN   pvnA;
   register pVN   pvnB;
   register int   i,j;

   i = pPEA->np;
   pvnA = pPEA->vn;
   while(i--)
   {
      j = pPEB->np;
      pvnB = pPEB->vn;
      while( j-- )
      {
         if(      pvnA->v.x == pvnB->v.x
            && pvnA->v.y == pvnB->v.y
            && pvnA->v.z == pvnB->v.z
           )
            return TRUE;
         pvnB++;
      }
      pvnA++;
   }
   return FALSE;
}

/**********************************************************************************
 *
 * Function:   InShadow
 *
 * Purpose: Intersects a ray from a point to a light source with each polygon
 *          in a polygon list to determine if any obscuration occurs.
 *
 * Returns: TRUE if any polygon obscures the light ray; otherwise FALSE.
 *
 **********************************************************************************
 */
static BOOLEAN InShadow(pPolyList pPL, FPPoint3d P, FPType ld,
                        pLtSource pLS, FPType *transmission)
{
   register int i;
   register FPType t,ldx,ldy,ldz;
   register pPolyList ppl = pPL;
   register pPolyExt pPE;
   register pVN pvn;
            
   FPPoint3d iP,LS,V;
   Box RayBox;
   pPolyExt pPEcurrent;
   
   LS.x = pLS->x;
   LS.y = pLS->y;
   LS.z = pLS->z;
   pPE   = (pPolyExt) (phCurrent + (*phCurrent).p.polySize);
   
   pPEcurrent = pPE;
   
   BoxMinMax(x,RayBox,P,LS);
   BoxMinMax(y,RayBox,P,LS);
   BoxMinMax(z,RayBox,P,LS);

   ldx = LS.x-P.x;
   ldy = LS.y-P.y;
   ldz = LS.z-P.z;

   while(ppl)
   {
      if( ppl->pp!=phCurrent )
      {
         pPE   = (pPolyExt) (ppl->pp + (*ppl->pp).p.polySize);

         if(BoxIntersects(pPE->bounds, RayBox))
         {
            if( !Adjacent( pPEcurrent, pPE ) )
            {
               t = -
               (   pPE->n.x*(P.x-pPE->c.x)+pPE->n.y*(P.y-pPE->c.y)+pPE->n.z*(P.z-pPE->c.z)   )
               / /*-----------------------------------------------------------------------*/
               (              pPE->n.x*ldx+pPE->n.y*ldy+pPE->n.z*ldz             );
               iP.x= P.x+ldx*t;
               iP.y= P.y+ldy*t;
               iP.z= P.z+ldz*t;

               if( PointInBox(iP, pPE->bounds))
               {           
                  V.x = LS.x - iP.x;
                  V.y = LS.y - iP.y;
                  V.z = LS.z - iP.z;
                  if( distance( V ) < ld )
                  {                    
                     pvn = pPE->vn;
                     i = pPE->np;
                     while( --i )
                     {
                        V.x = iP.x - pvn->v.x;
                        V.y = iP.y - pvn->v.y;
                        V.z = iP.z - pvn->v.z;
                        if( DotProduct( pvn->d, V ) < 0 )
                           break;
                        pvn++;
                     }
                     if( !i )
                     {
                        *transmission = FPFix( pPE->RA.transparency );
                        return TRUE;
                     }
                  }
               }
            }
         }
      }
      ppl = ppl->prior;
   }            
   return FALSE;
}


#endif

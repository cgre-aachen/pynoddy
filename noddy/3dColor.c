/*****************************************************************************
 * File:    3dColor.c
 *
 * Purpose: Color rendering of solids.
 *
 * © 1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include <math.h>
#include "3dColor.h"


extern BOOLEAN ColorPresent;  /* from Init3d() call of SysEnvirons in 3d.c */

#if XVT_CC_PROTO
extern int regionSize (PNT *);
#else
extern int regionSize ();
#endif

/*****************************************************************************
 *
 * Function:   rgbToHsv
 *
 * Purpose: Convert RGB value cordinated to HSV
 *    hue,         Fraction of circle, red at 0 
 *      saturation,   0 - 1, 0 for gray, 1 for pure color 
 *      value;     0 - 1, 0 for black, 1 for max intensity
 *
 * Returns: TRUE if sucessful , FALSE on error
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
rgbToHsv (int r, int g, int b, HSV_COLOR *hsv)
#else
rgbToHsv (r, g, b, hsv)
int r, g, b;
HSV_COLOR *hsv;
#endif
{
   float redDist, greenDist, blueDist;
   float fHue, fValue, fRange;
   int iValue, iMin;
   
   iValue = MAX(r, MAX(g, b));
   iMin = MIN(r, MIN(g, b));   
   if (iValue)                 
   {                
      fValue = (float) iValue;
      if ((fRange = (float) (fValue - iMin)) < 1.0)
         fRange = (float) 1.0;
      
      redDist  = (float) (iValue - r) / fRange; /* dist of color from red */
      greenDist= (float) (iValue - g) / fRange; /* dist of color from green */
      blueDist = (float) (iValue - b) / fRange; /* dist of color from blue */
      
      if (iValue == r)
         if (iMin == g)
            fHue = 5 + blueDist;
         else
            fHue = 2 - greenDist;
      else if (iValue == g)
         if (iMin == b)
            fHue = 1 + redDist;
         else
            fHue = 3 - blueDist;
      else if (iMin == r)
         fHue = 3 + greenDist;
      else
         fHue = 5 - redDist;
         
      hsv->hue = fHue / 6; /* convert to range from 0 - 1 with red at 0 */
      hsv->saturation = fRange / fValue;
      hsv->value = fValue / 255;      
      
      return (TRUE);
   }       
   else  /* if the max is 0 then the conversion fails */
   { 
      hsv->hue = (float) 0.0;
      hsv->saturation = (float) 0.0;
      hsv->value = (float) 0.0;
      
      return (FALSE);
   }
}

/*****************************************************************************
 *
 * Function:   hsvToRgb
 *
 * Purpose: Convert HSV value cordinated to RGB
 *    hue,         Fraction of circle, red at 0 
 *      saturation,   0 - 1, 0 for gray, 1 for pure color 
 *      value;     0 - 1, 0 for black, 1 for max intensity
 *
 * Returns: TRUE if sucessful , FALSE on error
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
hsvToRgb (HSV_COLOR *hsv, int *r, int *g, int *b)
#else
hsvToRgb (hsv, r, g, b)
HSV_COLOR *hsv;
int *r, *g, *b;
#endif
{
   int value1, value2, value3;
   float fractHue, fHue, fValue;  
   int iHueSector;
   
   if (hsv->hue >= 1.0)
      fHue = (hsv->hue - (float) floor ((double) hsv->hue)) * 6;
   else
      fHue = hsv->hue * 6;
      
   iHueSector = (int) floor ((double) fHue);  /* which part of circle its in */
   fractHue = fHue - iHueSector;  /* fractional part of the Hue */
   fValue = hsv->value * 255;
   
   value1 = (int) (float) (fValue * (1.0 - hsv->saturation));
   value2 = (int) (float) (fValue * (1.0 - (hsv->saturation * fractHue)));
   value3 = (int) (float) (fValue * (1.0 - (hsv->saturation * (1.0 - fractHue))));
   
   switch (iHueSector)
   {
      case (0):
         *r = (int) fValue;
         *b = value3;
         *g = value1;
         break;
      case (1):
         *r = value2;
         *b = (int) fValue;
         *g = value1;
         break;
      case (2):
         *r = value1;
         *b = (int) fValue;
         *g = value3;
         break;
      case (3):
         *r = value1;
         *b = value2;
         *g = (int) fValue;
         break;
      case (4):
         *r = value3;
         *b = value1;
         *g = (int) fValue;
         break;
      case (5):
         *r = (int) fValue;
         *b = value1;
         *g = value2;
         break;      
      default:      
         *r = *g = *b = 0;
         return (FALSE);
   }                    
   return (TRUE);
}
/*****************************************************************************
 *
 * Function:   NewLighting(…)
 *
 * Purpose: Allocates and sets up a lighting structure containing a number
 *          of light sources and ambient light information.
 *
 * Returns: Pointer to the lighting information if successfull
 *          nil if memory allocation fails (call MemError for reason)
 *
 *****************************************************************************
 */
pLighting
#if XVT_CC_PROTO
NewLighting (
      colorFactor R, colorFactor G, colorFactor B, /* ambient light     */
      Fixed lvl,     /* intensity scaling */
      int nS,        /* number of sources */
      pLtSource pLS     /* array of sources  */
   )
#else
NewLighting (R, G, B, lvl, nS, pLS)
colorFactor R, G, B; /* ambient light  */
Fixed lvl;     /* intensity scaling */
int nS;        /* number of sources */
pLtSource pLS;    /* array of sources  */
#endif
{
   pLighting   pL;
   
   if( nS<0 )
      nS=0;
   pL = (pLighting)xvt_mem_alloc( sizeof(Lighting) + sizeof(LtSource) * nS );
   if( pL )
   {
      pL->color = XVT_MAKE_COLOR (R, G, B);
      pL->level      = lvl;
      pL->magnitude  = (unsigned long) (R) +
                       (unsigned long) (G) +
                       (unsigned long) (B);
      pL->nSources   = nS;
           pL->source = (pLtSource) (pL + 1);
      while( nS-- )
         pL->source[nS] = pLS[nS];
   }
   return pL;
}

/*****************************************************************************
 *
 * Function:   AimLightSource(…)
 *
 * Purpose: Aims a light source at a specific point by directing the
 *          source's normal vector towards the specified point.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
AimLightSource(pLtSource pLS,Fixed x,Fixed y,Fixed z)
#else
AimLightSource(pLS, x, y, z)
pLtSource pLS;
Fixed x, y, z;
#endif
{
   register
      FPType   nx, ny, nz, r;

   pLS->focus.x   = x;
   pLS->focus.y   = y;
   pLS->focus.z   = z;
   pLS->fx        = x;
   pLS->fy        = y;
   pLS->fz        = z;
   nx          = pLS->fx - pLS->x;
   ny          = pLS->fy - pLS->y;
   nz          = pLS->fz - pLS->z;
   r = 1.0/sqrt( nx*nx+ny*ny+nz*nz );
   nx *= r;
   ny *= r;
   nz *= r;
   pLS->normal.x  = (long)(nx*(255.0));
   pLS->normal.y  = (long)(ny*(255.0));
   pLS->normal.z  = (long)(nz*(255.0));
   pLS->nx        = nx;
   pLS->ny        = ny;
   pLS->nz        = nz;
}

/*****************************************************************************
 *
 * Function:   Set3dLtSource(…)
 *
 * Purpose: Stores color and location information about a light source in
 *          a caller supplied area.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
Set3dLtSource (pLtSource pLS, colorFactor R,colorFactor G,colorFactor B,
                Fixed lvl, Fixed x, Fixed y, Fixed z )
#else
Set3dLtSource (pLS, R, G, B, lvl, x, y, z)
pLtSource pLS;
colorFactor R, G, B;
Fixed lvl, x, y, z;
#endif
{
   pLS->color = XVT_MAKE_COLOR (R, G, B);
   pLS->level = lvl;
   pLS->magnitude = (unsigned long) (R) +
                    (unsigned long) (G) +
                    (unsigned long) (B);
   pLS->location.x = x;
   pLS->location.y = y;
   pLS->location.z = z;
   pLS->x = x;
   pLS->y = y;
   pLS->z = z;
   pLS->focus.x = x;
   pLS->focus.y = y-1.00;
   pLS->focus.z = z;
   pLS->normal.x = 0.00;
   pLS->normal.y =-1.00;
   pLS->normal.z = 0.00;
   pLS->nx = 0;
   pLS->ny =-1;
   pLS->nz = 0;
   pLS->shape = 0.0;
   pLS->limitAngle = 360.0*0.017453292519943;
}

/*****************************************************************************
 *
 * Function:   TransformLighting(…)
 *
 * Purpose: Transforms the locations of a lighting set's light sources
 *          using a supplied transform matrix and converts the results to
 *          double precision.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
TransformLighting(pLighting pL, Matrix3D *xf)
#else
TransformLighting(pL, xf)
pLighting pL;
Matrix3D *xf;
#endif
{
   register int i = pL->nSources;
   register pLtSource   pLS   = pL->source;
   register FPType r;
   Point3d p;
   
   while( i-- )
   {
      ApplyTransform( &pLS->location, &p, xf );
      pLS->x = p.x;
      pLS->y = p.y;
      pLS->z = p.z;
      ApplyTransform( &pLS->focus, &p, xf );
      pLS->fx = p.x;
      pLS->fy = p.y;
      pLS->fz = p.z;
      pLS->nx = pLS->fx - pLS->x;
      pLS->ny = pLS->fy - pLS->y;
      pLS->nz = pLS->fz - pLS->z;
      r = 1.0/sqrt( pLS->nx*pLS->nx+pLS->ny*pLS->ny+pLS->nz*pLS->nz );
      pLS->nx *= r;
      pLS->ny *= r;
      pLS->nz *= r;
      pLS->normal.x = (long)(pLS->nx*(255.0));
      pLS->normal.y = (long)(pLS->ny*(255.0));
      pLS->normal.z = (long)(pLS->nz*(255.0));
      pLS++;
   }     
}

/*****************************************************************************
 *
 * Function:   ColorRgn()
 *
 * Purpose: Given a valid region, its centroid, normal vector and distance
 *          from the viewer; lighting information and rendering options
 *          ColorRgn completes the rendering of a facet.
 *
 *          Called by RenderRectFacet and RenderPolyFacet of 3dSolids.c
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
ColorRgn (Point3d C,/*  centroid of region   */
   Point3d N,/*   normal at centroid   */
   Fixed d, /* distance from viewer */
   pRendAttr   pRA,  PNT   *rh,  pLighting   pL )
#else
ColorRgn (C, N, d, pRA, rh, pL)
Point3d C;  /* centroid of region   */
Point3d N;  /* normal at centroid   */
Fixed d; /* distance from viewer */
pRendAttr pRA;
PNT *rh;
pLighting pL;
#endif
{
   BOOLEAN grayScaled, colored, patterned, distanceEffect, ambientEffect;
   FPType fNx,fNy,fNz;
   FPType fCx,fCy,fCz;
   FPType lVx,lVy,lVz;
   FPType b, L, V=0.0,  shade = 0.0, grayShade = 0.0;
   COLOR theColor;
   HSV_COLOR color;
   int grayScaleIx=0; 
   int numPoints;
   register int nS;
   register pLtSource pLS;
   register FPType R=0.0, G=0.0, B=0;
   register FPType a, c;

#define  ClipColor(c)   ( ((c)>255)?255:(((c)<0)?0:(c)))
#define  ReflectColor(c)   ((c) * pRA->c )

   /* Call validity… this function needs a valid region,
    * lighting and rendering information.  if any of the
    * these is missing we'll just exit quietly.
    */
   if( (!rh) || (!pL) || (!pRA) )
      return;
   if ((rh->h == (rh+1)->h ) && (rh->v == (rh+1)->v))  /* Empty Region */
      return;
      
   numPoints = regionSize (rh);    

   /* Determine how to apply the rendering attributes:
    * grayScaled dithering is the default if color is not 
    * available;  grayScaled dithering takes precedence 
    * over Color QuickDraw's PixPat's if both are requested.
    */
   colored = ColorPresent;
   grayScaled = !colored;
   patterned = grayScaled;
   distanceEffect = pRA->distanceEffect;
   ambientEffect  = pRA->ambientEffect;

   if( !pRA->lightSource )
   {
      /* convert region centroid and normal to floating point
       */
      fCx = C.x;
      fCy = C.y;
      fCz = C.z;
   
      fNx = N.x;
      fNy = N.y;
      fNz = N.z;
                      /* normal vector magnitude   */
      b = (double) sqrt( (double) (fNx*fNx + fNy*fNy + fNz*fNz) );
   
      /* apply all light sources to this region
       */
      nS = pL->nSources;
      pLS = &pL->source[nS];
      while( nS-- )
      {
         pLS--;
         /* determine vector from light source to centroid
          */
         lVx = fCx - pLS->x;
         lVy = fCy - pLS->y;
         lVz = fCz - pLS->z;
         
         /* determine dot product of the normal and light vectors.
          * the dot product, if positive indicates the region is
          * illuminated by the light source
          */
         a = fNx*lVx + fNy*lVy + fNz*lVz;
         if( a>=0 )
         {
            /* determine the cosine of the angle between the light
             * vector and normal (angle of incidence) which per
             * Lamberts cosine law of lighting equals the intensity
             * of the light reflected.
             */
            c = (double) sqrt( (double) lVx*lVx + lVy*lVy + lVz*lVz ); /* distance source->region  */

            if( !b || !c )
               continue;
            shade = a / ( b * c ); /* cosine of the angle of incidence */

            if(pLS->shape!=0.0)
            {
               FPType   lnlv,r;
               
               r = 1.0/c;
               lVx *= r;
               lVy *= r;
               lVz *= r;
               lnlv = pLS->nx*lVx + pLS->ny*lVy + pLS->nz*lVz;
               shade *= pow(lnlv,pLS->shape);
            }

            /* apply the inverse square rule for intensity drop off
             * with source distance yielding the light intensity (L).
             */
            L = ( (pLS->magnitude*256) / ( c * c ) ) * shade;
            
            if( grayScaled )
            {  /* if we are using dither patterns, sum the intensity
                * of all sources which illuminate the region.
                */
               grayShade += L * pLS->level;
            }
   
            /* if color is in use, take the intensity sum across all
             * light sources which illuminate the region.
             */
            if ( colored )
            {              
               int red, green, blue;
               rgbToHsv(XVT_COLOR_GET_RED(pLS->color),
                        XVT_COLOR_GET_BLUE(pLS->color),
                        XVT_COLOR_GET_GREEN(pLS->color), &color);
               V = color.value * L;
               color.value = MIN((float) 1.0, (float) V);
               if (color.value < 0.001) color.value = (float) 0.001;
               hsvToRgb(&color, &red, &green, &blue);
               red = ClipColor(red);
               green = ClipColor(green);
               blue = ClipColor(blue);
               R += (red * pLS->level );
               G += (green * pLS->level );
               B += (blue * pLS->level );
               theColor = XVT_MAKE_COLOR (red, green, blue);
            }
         }/* end is illuminated (a>0) */
      }/* end light source loop */
   }
   
   if ( pRA->lightSource )
   {
      R = G = B = 255.0;
      grayShade = pL->nSources;
   }

   if ( grayScaled )
   {
      /* average the light intensity for the number of sources
       */
      grayShade /= pL->nSources;

      /* index into the resource for the dithering patterns.  the
       * patterns are assumed to be ordered from  light to dark,
       * with 65 elements present.
       */
      grayScaleIx= (int)( ( colored? 65*grayShade : 66-65*grayShade )+0.5 );
      if (grayScaleIx>65 )
         grayScaleIx = 65;
      else
      if( grayScaleIx<1 )
         grayScaleIx = 1;
      /*
      GetIndPattern( &grayScalePat, GRAYSCALE64RESOURCEID, grayScaleIx );
      */

      if( !colored )
      {  /* monochrome display, do it now.
         */
         xvt_dwin_draw_polygon (currentWin, rh, numPoints);
         return;
      }
   }/* end grayScaled */

   if( colored )
   {
      if( distanceEffect )
      {
         /* convert the viewer distance to floating point, square it
          * and calculate the inverse (to avoid multiple divisions).
          */
         c = d/255.0;
         a = 1.0 / (c * c );
      }
      else
         a = 0.0039; /* scale down to reasonable range 1/255th */

      /* apply the reflectivity factors and viewer distance effects
       * to the accumulated light.
       */
      {
         FPType   Ra,Ga,Ba;
         Ra = (XVT_COLOR_GET_RED(pL->color)*pL->level);
         Ga = (XVT_COLOR_GET_GREEN(pL->color)*pL->level);
         Ba = (XVT_COLOR_GET_BLUE(pL->color)*pL->level);
         R = (ReflectColor(R + Ra) + Ra) * a;
         G = (ReflectColor(G + Ga) + Ga) * a;
         B = (ReflectColor(B + Ba) + Ba) * a;
         
         if( ambientEffect )
         {
            /* add the ambient light reaching the eye directly.
             */
            R = R + Ra;
            G = G + Ga;
            B = B + Ba;
         }
      }
      /* clip the finished color value into RGB space
       */
      theColor = XVT_MAKE_COLOR (ClipColor(R), ClipColor(G), ClipColor(B));

      /* now to color the region, at last…
       */
      if(patterned)
      {
/*       PAT_STYLE hPixPat; */      
         /* construct a Color QuickDraw pixel pattern which
          * approximates the desired RGB color and fill the
          * region with it.
          */
/*
**       hPixPat = NewPixPat();
**       MakeRGBPat(hPixPat,&theColor);
**       FillCRgn(rh,hPixPat);
**       DisposPixPat(hPixPat);
*/
      }
      else
      {
         DRAW_CTOOLS tools, copyTools;
                  
         /* save the current foreground color, setup the RGB
          * color calculated above, paint and/or dither the
          * region as requested.
          */
         xvt_dwin_get_draw_ctools (currentWin, &tools);
         copyTools = tools;
         tools.pen.color = theColor;
         tools.brush.color = theColor;
         xvt_dwin_set_draw_ctools (currentWin, &tools);
         
         if(grayScaled)
            xvt_dwin_draw_polygon (currentWin, rh, numPoints);
         else
            xvt_dwin_draw_polygon (currentWin, rh, numPoints);

         xvt_dwin_set_draw_ctools (currentWin, &copyTools);
      }
   }/* end colored */
}

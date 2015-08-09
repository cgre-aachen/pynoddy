/*****************************************************************************
 * file:    "3dClipping.c"
 *
 * purpose: Implements point and line canonical clipping to a perspective
 *          view volume bounded by a near (hither), far (yon) and as an
 *          option top, bottom,left and right (at 90¡ angles) planes.
 *
 * ©1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "xvt.h"
#include "3dClip.h"
#include "3dFMath.h"

BOOLEAN canonicalClip = TRUE; /* clip to 90¡ pyramid? */


#if XVT_CC_PROTO
static void PushDown (Point3d *,Point3d *);
static void PushUp (Point3d *,Point3d *);
static void PushLeft (Point3d *,Point3d *);
static void PushRight (Point3d *,Point3d *);
static void PushBeyond (Point3d *,Point3d *);
static void PushBehind (Point3d *,Point3d *);
static void Push (ClipBits *,Point3d *,Point3d *);
static void ClipEncode (Point3d *,ClipBits *);
#else
static void PushDown ();
static void PushUp ();
static void PushLeft ();
static void PushRight ();
static void PushBeyond ();
static void PushBehind ();
static void Push ();
static void ClipEncode ();
#endif


void
#if XVT_CC_PROTO
SetCanonicalClipping(BOOLEAN state)
#else
SetCanonicalClipping(state)
BOOLEAN state;
#endif
/*****************************************************************************
*  purpose: Enables or disables clipping to the top, bottom, left and
*           right planes of the 90¡ view pyramid.
*****************************************************************************/
{
   canonicalClip = state;
}

static void PushDown(S,E)  /* inverted y axis by viewer transform */
register Point3d  *S,*E;
/*****************************************************************************
*  purpose: moves the start point of a line segment to its intercept
*           with the plane y=z (the top clipping plane).
*****************************************************************************/
{  
   register Fixed    k;
   Point3d           s,e,r;

   s  = *S;
   e  = *E;
   k  = fixdiv(s.z + s.y,(s.y + s.z) - (e.y + e.z));
   r.x   = fixmul((e.x - s.x),k) + s.x;
   r.y   = fixmul((e.y - s.y),k) + s.y;
   r.z   = -r.y;
   *S = r;
}

static void PushUp(S,E) /* inverted y axis by viewer transform */
register Point3d  *S,*E;
/*****************************************************************************
*  purpose: moves the start point of a line segment to its intercept
*           with the plane y=-z (the bottom clipping plane).
*****************************************************************************/
{  
   register Fixed    k;
   Point3d           s,e,r;

   s  = *S;
   e  = *E;
   k  = fixdiv(s.z - s.y,(e.y + s.z) - (s.y + e.z));
   r.x   = fixmul((e.x - s.x),k) + s.x;
   r.y   = fixmul((e.y - s.y),k) + s.y;
   r.z   = r.y;
   *S = r;
}


static void PushLeft(S,E)
register Point3d  *S,*E;
/*****************************************************************************
*  purpose: moves the start point of a line segment to its intercept
*           with the plane x=z (the right clipping plane).
*****************************************************************************/
{  
   register Fixed    k;
   Point3d           s,e,r;

   s  = *S;
   e  = *E;
   k  = fixdiv(s.z - s.x,(e.x + s.z) - (s.x + e.z));
   r.x   = fixmul((e.x - s.x),k) + s.x;
   r.y   = fixmul((e.y - s.y),k) + s.y;
   r.z   = r.x;
   *S = r;
}

static void PushRight(S,E)
register Point3d  *S,*E;
/*****************************************************************************
*  purpose: moves the start point of a line segment to its intercept
*           with the plane x=-z (the left clipping plane).
*****************************************************************************/
{  
   register Fixed    k;
   Point3d           s,e,r;

   s  = *S;
   e  = *E;
   k  = fixdiv(s.z + s.x,(s.x + s.z) - (e.x + e.z));
   r.x   = fixmul((e.x - s.x),k) + s.x;
   r.y   = fixmul((e.y - s.y),k) + s.y;
   r.z   = -r.x;
   *S = r;
}

static void PushBeyond(S,E)
register Point3d  *S,*E;
/*****************************************************************************
*  purpose: moves the start point of a line segment to its intercept
*           with the plane hither=z (the near clipping plane).
*****************************************************************************/
{  
   register Fixed    m1,m2;
   Point3d           d,s,e;

   s  = *S;
   e  = *E;
   d.x   = e.x - s.x;
   d.y   = e.y - s.y;
   d.z   = e.z - s.z;
   m1 = fixdiv(d.y,d.z);   /* y/z slope */
   m2 = fixdiv(d.x,d.z); /* x/z slope */
   S->x= s.x+fixmul(m2,hither-s.z);
   S->y= s.y+fixmul(m1,hither-s.z);
   S->z= hither;
}

static void PushBehind(S,E)
register Point3d  *S,*E;
/*****************************************************************************
*  purpose: moves the start point of a line segment to its intercept
*           with the plane yon=z (the far clipping plane).
*****************************************************************************/
{
   register Fixed    m1,m2;
   Point3d           d,s,e;

   s  = *S;
   e  = *E;
   d.x   = e.x - s.x;
   d.y   = e.y - s.y;
   d.z   = e.z - s.z;
   m1 = fixdiv(d.y,d.z);   /* y/z slope */
   m2 = fixdiv(d.x,d.z); /* x/z slope */
   S->x= s.x+fixmul(m2,yon-s.z);
   S->y= s.y+fixmul(m1,yon-s.z);
   S->z= yon;
}

static void Push(cb, p1, p2)
register ClipBits *cb;
register Point3d *p1,*p2;
/*****************************************************************************
*  purpose: based on the start point's location pushes the point to
*           the appropriate clipping plane.
*****************************************************************************/
{  
   if (cb->behind)
      PushBeyond(p1,p2);
   else if (cb->beyond)
      PushBehind(p1,p2);
   else if (cb->above)
      PushDown(p1,p2);
   else if (cb->below)
      PushUp(p1,p2);
   else if (cb->right)
      PushLeft(p1,p2);
   else if (cb->left)
      PushRight(p1,p2);
}

static void ClipEncode(p,cb)
register Point3d  *p;
register ClipBits *cb;
/*****************************************************************************
*  purpose: classifies a point's location relative to the six clipping
*           planes (y=z,y=-z,x=z,x=-z,z=hither,z=yon) when canonical
*           clipping is enabled; or to the far and near planes (z=yon
*           and z=hither) if canonical clipping is disabled.
*
*  method:     based on Cohen-Sutherland algorithm.
*
*****************************************************************************/
{  
   register Fixed    z;

   z = FxAbs(p->z);
   if ( canonicalClip )
   {
      cb->left  = (p->x < -z);
      cb->right = (p->x >  z);
      cb->above = (p->y < -z);
      cb->below = (p->y >  z);
   }
   else
   {
      cb->left  = FALSE;
      cb->right = FALSE;
      cb->above = FALSE;
      cb->below = FALSE;
   }
   cb->behind  = (p->z < hither);
   cb->beyond  = (p->z > yon);
   cb->b7      = 
   cb->b8      = 0;
}

enum visibility ClipLine(startPt, endPt)
register Point3d *startPt, *endPt;
/*****************************************************************************
*  purpose: determines if a line segment intersects the view volume,
*           and if so clips it to the view volume.
*
*  warning: it is expected that the start and end points have been
*           transformed prior to calling "ClipLine".
*****************************************************************************/
{  
   register int   i;
   union startClipBits
   {
      ClipBits as_bits;
      char     as_char;
   }  startClipBits;
   union endClipBits
   {
      ClipBits as_bits;
      char     as_char;
   }  endClipBits;
   
   ClipEncode(startPt, &startClipBits.as_bits);
   ClipEncode(endPt, &endClipBits.as_bits);
   if(!(startClipBits.as_char | endClipBits.as_char))
      return(is_visible);
   if( (startClipBits.as_char & endClipBits.as_char))
      return(isnt_visible);
   i = 5;
   while (i--)
   {
      if (startClipBits.as_char)
      {
         Push(&startClipBits.as_bits, startPt, endPt);
         ClipEncode(startPt, &startClipBits.as_bits);
      }
      else if (endClipBits.as_char)
      {
         Push(&endClipBits.as_bits, endPt, startPt);
         ClipEncode(endPt, &endClipBits.as_bits);
      }
      
      if (!(startClipBits.as_char | endClipBits.as_char))
         return(is_visible);
      if ( (startClipBits.as_char & endClipBits.as_char))
         return(isnt_visible);
   }
   return (isnt_visible);
}

enum visibility ClipPt(Pt)
register Point3d *Pt;
/*****************************************************************************
*  purpose: determines if a points lies within the view volume.
*
*  warning: it is expected that the point has been transformed prior to
*           calling "ClipPt".
*****************************************************************************/
{  
   union ptClipBits
   {  ClipBits as_bits;
      char     as_char;
   }  ptClipBits;

   ClipEncode(Pt,&ptClipBits.as_bits);
   if(! ptClipBits.as_char )
      return(is_visible);
   return(isnt_visible);
}


/*****************************************************************************
 * file:    "3d.h"
 *
 * ©1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#ifndef _3d_
#define _3d_
              
#include "xvt.h"      

#if (XVTWS == WINWS) || (XVTWS == WIN16WS)
//#define xvt_mem_halloc(X,Y)   halloc(X,Y)
//#define xvt_mem_zhalloc(X,Y)  halloc(X,Y)
//#define xvt_mem_hfree(X)   hfree(X)
#define hmemcpy(X,Y,S)     {\
                     char __huge *ptr1;\
                     char __huge *ptr2;\
                     long ssize;\
                     ssize = (long) S;\
                     ptr1 = (char __huge *) (X);\
                     ptr2 = (char __huge *) (Y);\
                     while ((ssize--) > 0)\
                        *ptr1++ = *ptr2++;\
                  }

#else
//#define xvt_mem_halloc(X,Y)   xvt_mem_alloc((X)*(Y))
//#define xvt_mem_zhalloc(X,Y)  xvt_mem_zalloc((X)*(Y))
//#define xvt_mem_hfree(X)   xvt_mem_free(X)
#define hmemcpy(X,Y,S)     memcpy(X,Y,S)
#endif
              
#define Fixed     double
#define FPType    double

#define  QDGLOBALS   qd.

#define HITHER       64.0
#define YON      200000.0 /* 32000.0 */

typedef struct
{
   Fixed x, y;
} Point2d;

typedef struct
{
   Fixed x, y, z;
} Point3d;

typedef struct
{
   unsigned char
      above  :1,
      below  :1,
      right  :1,
      left   :1,
      behind :1,
      beyond :1,
      b7     :1,
      b8     :1;
} ClipBits;
                    
typedef struct
{
   float hue,        /* Fraction of circle, red at 0 */
         saturation, /* 0 - 1, 0 for gray, 1 for pure color */
         value;      /* 0 - 1, 0 for block, 1 for max intensity */
} HSV_COLOR;                                                       

typedef struct
{
   Fixed v[4];
} Vector;

typedef struct
{
   Fixed v[4][4];
} Matrix3D;

enum visibility   { isnt_visible, is_visible };

#define Transform(pt1,pt2)    ApplyTransform((pt1),(pt2),&xFormCombined)
#define ViewerTransform(pt1,pt2) ApplyTransform((pt1),(pt2),&xFormViewer)
#define InstanceTransform(pt1,pt2)  ApplyTransform((pt1),(pt2),&xFormInstance)
#define CombineReferenceFrames() mMult4x4(&xFormInstance,&xFormViewer,&xFormCombined)
#define MoveTo(X,Y)           { PNT pnt; pnt.h = (X); pnt.v = (Y);\
                       xvt_dwin_draw_set_pos(currentWin, pnt);}
#define LineTo(X,Y)           { PNT pnt; pnt.h = (X); pnt.v = (Y);\
                       xvt_dwin_draw_line(currentWin, pnt);}

#if XVT_CC_PROTO
void  SetPt3d (Point3d*, Fixed, Fixed, Fixed);
void  AddPt3d (Point3d*, Point3d*, Point3d*);
void  SubPt3d (Point3d*, Point3d*, Point3d*);
void  avg (Point3d*, Point3d*, Point3d*);
void  Project (Point3d*, Point2d*);
void  ApplyTransform (Point3d*, Point3d*, Matrix3D*);
void  Translate (Matrix3D*, Fixed, Fixed, Fixed);
void  Rotate (Matrix3D*, Fixed, Fixed, Fixed);
void  Scale (Matrix3D*, Fixed, Fixed, Fixed);
void  SetInstanceReferenceFrame (Fixed, Fixed, Fixed, Fixed, Fixed, Fixed, Fixed, Fixed, Fixed);
void  SetViewerReferenceFrame (Fixed, Fixed, Fixed, Fixed, Fixed, Fixed, Fixed, Fixed, Fixed, Fixed);
void  Init3d (void);
#else
void  SetPt3d ();
void  AddPt3d ();
void  SubPt3d ();
void  avg ();
void  Project ();
void  ApplyTransform ();
void  Translate ();
void  Rotate ();
void  Scale ();
void  SetInstanceReferenceFrame ();
void  SetViewerReferenceFrame ();
void  Init3d ();
#endif


#endif

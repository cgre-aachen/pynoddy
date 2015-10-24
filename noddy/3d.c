/*****************************************************************************
 * File:    "3d.c"
 *
 * Purpose: translation ,rotation and scaling routines for use in  
 *          three dimensional graphics applications.
 *
 * ©1991 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#include "xvt.h"
#include "3d.h"
#include "3dFMath.h"

WINDOW currentWin;      /* window currently being drawn into */

Point3d  penLoc,     /* location of the drawing pen   */
      viewerScale,   /* current viewer scale factors  */
      viewerLoc,     /* space location of the viewer  */
      viewerRot;     /* viewer's pitch, yaw and roll  */

Fixed hither,        /* near clipping plane        */
      yon,        /* far clipping plane         */
      zoom;       /* camera zoom factor         */

Matrix3D   Identity,      /* identity transform matrix  */
      xFormInstance, /* instancing transform matrix   */
      xFormViewer,   /* viewers transform matrix      */
      xFormCombined; /* viewer & instance combined */

Fixed centerX,    /* screen horizontal center      */
      centerY;    /* screen vertical center     */
      
BOOLEAN  parallel;      /* if true disables perspective */

BOOLEAN  FpCpuPresent;  /* Floating Point Processor?  */
BOOLEAN  ColorPresent;  /* Color Available?           */
BOOLEAN  QD32Present;   /* 32bit QuickDraw available? */

/*****************************************************************************
 *
 * Function:   set3dOutputWindow
 *
 * purpose: set the window the 3d image will be drawn into
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
set3dOutputWindow(WINDOW win)
#else
set3dOutputWindow(win)
WINDOW win;
#endif
{
   currentWin = win;
}


/*****************************************************************************
 *
 * Function:   SetPt3d
 *
 * purpose: sets a point structure from three fixed point values.
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
SetPt3d(Point3d *pt, Fixed x, Fixed y, Fixed z)
#else
SetPt3d(pt, x, y, z)
Point3d *pt;
Fixed x, y, z;
#endif
{
   pt->x = x;
   pt->y = y;
   pt->z = z;
}

/*****************************************************************************
 *
 * Function:   AddPt3d
 *
 * purpose: adds corresponding coordinates of two points producing a
 *          third point.
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
AddPt3d(Point3d *pt1, Point3d *pt2, Point3d *pt3)
#else
AddPt3d(pt1, pt2, pt3)
Point3d *pt1;
Point3d *pt2;
Point3d *pt3;
#endif
{
   pt3->x = pt1->x + pt2->x;
   pt3->y = pt1->y + pt2->y;
   pt3->z = pt1->z + pt2->z;
}

/*****************************************************************************
 *
 * Function:   SubPt3d
 *
 * purpose: adds corresponding coordinates of two points producing a
 *          third point.
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
SubPt3d(Point3d *pt1, Point3d *pt2, Point3d *pt3)
#else
SubPt3d(pt1, pt2, pt3)
Point3d *pt1, *pt2, *pt3;
#endif
{
   pt3->x = pt1->x - pt2->x;
   pt3->y = pt1->y - pt2->y;
   pt3->z = pt1->z - pt2->z;
}

/*****************************************************************************
 *
 * Function:   avg
 *
 * purpose: averages the corresponding coordinates of two points giving
 *          a third point.
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
avg(Point3d *p1, Point3d *p2, Point3d *p) 
#else
avg(p1, p2, p) 
Point3d *p1, *p2, *p; 
#endif
{  
   p->x = fixdiv(p1->x+p2->x, 2.0);
   p->y = fixdiv(p1->y+p2->y, 2.0);
   p->z = fixdiv(p1->z+p2->z, 2.0);
}

/*****************************************************************************
 *
 * Function:   Project()
 *
 * purpose: converts a three dimensional point to its two dimensional
 *          computer screen equivalent.
 *
 * methods: For a perspective projection, scales the X and Y coordinates
 *          of a 3D point by their Z coordinate and a 2d zoom factor,
 *          then applies a centering translation.
 *          
 *          For a parallel projection, only the centering translation is
 *          applied, since Z axis scaling is not required.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
Project(register Point3d *p1, register Point2d *p2)
#else
Project(p1, p2)
register Point3d *p1;
register Point2d *p2;
#endif
{
   if (parallel)
   {
      p2->x = p1->x * zoom + centerX;
      p2->y = p1->y * zoom + centerY;
   }
   else
   {
      Fixed z = 1.0/(p1->z * zoom);
      p2->x = (p1->x * z) + centerX;
      p2->y = (p1->y * z) + centerY;
   }
}

/*****************************************************************************
 *
 * Function:   ApplyTransform(É)
 *
 * purpose: applies a transformation matrix to a point, with the
 *          result returned at the address supplied for "pt2".
 *
 * methods: the source point is copied into a vector form, then it
 *          and the transformation matrix are multiplied, as follows:
 *
 *             |x,y,z|' =|x y z 1|  |a b c 0|
 *                               |d e f 0|
 *                               |g h i 0|
 *                               |j k l 1|
 *
 *          or x'=xa+yd+zg+j
 *             y'=xb+ye+zh+k
 *             z'=xc+yf+zi+l
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
ApplyTransform(Point3d *pt1, Point3d *pt2, Matrix3D *xForm)
#else
ApplyTransform(pt1, pt2, xForm)
Point3d *pt1, *pt2;
Matrix3D *xForm;
#endif
{  
   register Fixed aX, aY, aZ;

   aX = pt1->x;
   aY = pt1->y;
   aZ = pt1->z;

   pt2->x = fixmul(aX,xForm->v[0][0]) + fixmul(aY,xForm->v[1][0])
          + fixmul(aZ,xForm->v[2][0]) + xForm->v[3][0];

   pt2->y = fixmul(aX,xForm->v[0][1]) + fixmul(aY,xForm->v[1][1])
          + fixmul(aZ,xForm->v[2][1]) + xForm->v[3][1];

   pt2->z = fixmul(aX,xForm->v[0][2]) + fixmul(aY,xForm->v[1][2])
          + fixmul(aZ,xForm->v[2][2]) + xForm->v[3][2];
}

/*****************************************************************************
 *
 * Function:   Translate(É)
 *
 * purpose: inserts a translation factor into a matrix;
 *
 * methods: the matrix is modified to the following form:
 *
 *                1  0  0  0
 *                0  1  0  0
 *                0  0  1  0
 *                dx dy dz 1
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
Translate(register Matrix3D *m,Fixed dx,Fixed dy,Fixed dz)
#else
Translate(m, dx, dy, dz)
register Matrix3D *m;
Fixed dx, dy, dz;
#endif
{
   m->v[3][0] = dx;
   m->v[3][1] = dy;
   m->v[3][2] = dz;
}

/*****************************************************************************
 *
 * Function:   Rotate(É)
 *
 * purpose: concatenates the axis rotation matrices to another matrix.
 *
 * methods: three successive local matrices are constructed as follows:
 *
 *                cy 0  sy 0     yaw
 *                0  1  0  0
 *                -sy   0  cy 0
 *                0  0  0  1
 *
 *                1  0  0  0     pitch
 *                0  cp -sp   0
 *                0  sp cp 0
 *                0  0  0  1
 *
 *                cr -sr   0  0     roll
 *                sr cr 0  0
 *                0  0  1  0
 *                0  0  0  1
 *
 *          where:   cy=Cos(y),  sy=Sin(y),
 *                cp=Cos(p),  sp=Sin(p),
 *                cr=Cos(r),  sr=Sin(r).
 *
 *          these matrices are appended to the source matrix "m" and
 *          with the result being returned in "m".
 *
 * warnings:   all angles are expected to be in degrees.  conversion to
 *          radians is done here.
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
Rotate(Matrix3D *m,Fixed p,Fixed y,Fixed r)
#else
Rotate(m, p, y, r)
Matrix3D *m;
Fixed p, y, r;
#endif
{  
   Matrix3D   R;
   Fixed vRad;
   
   if( y ) {      /* yaw */
      R        = Identity;
      vRad     = Rad(y);
      R.v[0][0]   = frac2fix(fraccos(vRad));
      R.v[2][2]   = R.v[0][0];
      R.v[2][0]   = frac2fix(fracsin(vRad));
      R.v[0][2]   = -R.v[2][0];
      mMult4x4(m,&R,m);
   }
   if( p ) {      /* pitch */
      R        = Identity;
      vRad     = Rad(p);
      R.v[2][2]   = R.v[1][1] = frac2fix(fraccos(vRad));
      R.v[2][1]   = frac2fix(fracsin(vRad));
      R.v[1][2]   = -R.v[2][1];
      mMult4x4(m,&R,m);
   }
   if( r ) {      /* roll */
      R        = Identity;
      vRad     = Rad(r);
      R.v[1][1]   = R.v[0][0] = frac2fix(fraccos(vRad));
      R.v[1][0]   = frac2fix(fracsin(vRad));
      R.v[0][1]   = -R.v[1][0];
      mMult4x4(m,&R,m);
   }
}

/*****************************************************************************
 *
 * Function:   Scale(É)
 *
 * purpose: concatenates a scaling matrix to another matrix.
 *
 * methods: a local matrix is constructed of the following form:
 *
 *                sx 0  0  0
 *                0  sy 0  0
 *                0  0  sz 0
 *                0  0  0  1
 *
 *          this matrix is then appended to the source matrix "m" and
 *          the result is returned in "m".
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
Scale(Matrix3D *m,Fixed sx,Fixed sy,Fixed sz)
#else
Scale(m, sx, sy, sz)
Matrix3D *m;
Fixed sx, sy, sz;
#endif
{  
   Matrix3D S;
   
   if( (sx != 1.0) || (sy != 1.0) || (sz != 1.0) ) {
      S        = Identity;
      S.v[0][0]   = sx;
      S.v[1][1]   = sy;
      S.v[2][2]   = sz;
      mMult4x4(m,&S,m);
   }
}

/*****************************************************************************
 *
 * Function:   SetInstanceReferenceFrame(É)
 *
 * purpose: constructs the instance transformation matrix from supplied
 *          parameters (rotations,scaling, and translations).
 *
 * methods: since matrix multiplications are used to concatenate the
 *          parameters into a single instance transform matrix it is
 *          first set to an identity matrix.
 *
 *          the sequence of concatenation operations causes objects to
 *          rotated about their origins, scaled to size then translated
 *          (moved) to their final locations in space.  this assures a
 *          "common sense" orientation of the objects in space.  the
 *          individual rotations can be thought of as though the viewer
 *          were located at the object's origin, looking towards the
 *          positive z axis.  positive pitch angles cause the object's
 *          z axis to move up (the visual effect is that of tilting the
 *          the object's top towards the viewer). positive yaw angles
 *          rotate the object's z axis to the right (the visual effect
 *          is that the object rotates clockwise when seen from a top
 *          down view).  positive roll angles cause the object to roll
 *          to the right.
 *          
 * warnings:   the resulting space coordinate system is a left handed
 *          coordinate system.  its axes are shown in the diagram
 *          below.
 *
 *                         y
 *                         +  z
 *                         |  +
 *                         | /
 *                         |/
 *                     ------------ +x
 *                       / |
 *                      /  |
 *                     /   |
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
SetInstanceReferenceFrame
   (
      Fixed pitch, Fixed yaw, Fixed roll, /* rotation angles in degrees */
      Fixed sx,    Fixed sy,  Fixed sz,   /* scaling factors by axis    */
      Fixed dx,    Fixed dy,  Fixed dz /* translation values by axis */
   )
#else
SetInstanceReferenceFrame (pitch, yaw, roll, sx, sy, sz, dx, dy, dz)
Fixed pitch, yaw, roll; /* rotation angles in degrees */
Fixed sx, sy, sz; /* scaling factors by axis    */
Fixed dx, dy, dz; /* translation values by axis */
#endif
{
   xFormInstance = Identity;
   Scale(&xFormInstance, sx, sy, sz);
   Rotate(&xFormInstance, pitch, yaw, roll);
   Translate(&xFormInstance, dx, dy, dz);
}

/*****************************************************************************
 *
 * Function:   SetViewerReferenceFrame(É)
 *
 * purpose: constructs the viewer transformation matrix from supplied
 *          parameters (rotations,scaling, and translations).  copies
 *          of the supplied parameters are saved as globals for use
 *          by other routines, when required.
 *
 * methods: since matrix multiplications are used to concatenate the
 *          parameters into a single viewer transform matrix, the
 *          viewer transform matrix is first set to an identity matrix.
 *          
 *          the sequence of concatenation operations is such that space
 *          is first translated to the viewer's origin then rotated and
 *          scaled.  this assures a "common sense" orientation of the
 *          viewer in space; positive pitch angles cause the viewer to
 *          to look up; positive yaw angles cause the viewer to look to
 *          the right; positive roll angles rotate the viewer clockwise.
 *          in the case of the yaw angle (y axis rotation) the supplied
 *          parameter value is negated prior to use.
 *          
 *          z axis scaling is an unusual case, controlling the overall
 *          view angle and effective size of the image.  this parameter
 *          is most effective when supplied in the range 0<sz<1.  as
 *          such the hither and yon clipping planes are scaled by this
 *          factor and a reciprocal is computed for use elsewhere.
 *
 * warnings:   the resulting viewer coordinate system is a right handed
 *          coordinate system, the inverse of the space coordinate
 *          system.  viewer coordinate system axes are shown in the
 *          diagram below.
 *
 *                         y
 *                         -  z
 *                         |  -
 *                         | /
 *                         |/
 *                     ------------ -x
 *                         /|
 *                        /   |
 *                       / |
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
SetViewerReferenceFrame
   (
      Fixed pitch, Fixed yaw, Fixed roll, /* rotation angles in degrees */
      Fixed sx,    Fixed sy,  Fixed sz,   /* scaling factors by axis    */
      Fixed dx,    Fixed dy,  Fixed dz,   /* translation values by axis */
      Fixed z                       /* 2d zoom factor          */
   )
#else
SetViewerReferenceFrame (pitch, yaw, roll, sx, sy, sz, dx, dy, dz, z)
Fixed pitch, yaw, roll; /* rotation angles in degrees */
Fixed sx, sy, sz; /* scaling factors by axis */
Fixed dx, dy, dz; /* translation values by axis */
Fixed z;    /* 2d zoom factor    */
#endif
{
   xFormViewer = Identity;
   xFormViewer.v[1][1]=-1.0;           /* flip y axis       */
   Translate(&xFormViewer, dx, dy, dz);
   Rotate(&xFormViewer, pitch, -yaw, roll);  /* reverse yaw angle */
   Scale(&xFormViewer, sx, sy, sz);

   viewerLoc.x = dx;
   viewerLoc.y = dy;
   viewerLoc.z = dz;
   viewerRot.x = pitch;
   viewerRot.y = yaw;
   viewerRot.z = roll;
   viewerScale.x = sx;
   viewerScale.y = sy;
   viewerScale.z = sz;
   zoom = z;
}

/*****************************************************************************
 * Function:   Init3d()
 *
 * purpose: initializes the identity matrix, sets the instancing and
 *          viewer matrices to identity, and sets the pen location to
 *          the center of space (0,0,0).
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
Init3d(void)
#else
Init3d()
#endif
{
   RCT rect;

   xvt_vobj_get_client_rect (currentWin, &rect);

   mIdentity(&Identity);
   xFormInstance = Identity;
   xFormViewer = Identity;
   xFormCombined = Identity;
   penLoc.x = penLoc.y = penLoc.z = 0;
   centerX = (rect.right - rect.left) / 2;
   centerY = (rect.bottom - rect.top) / 2;
   hither = HITHER;
   yon = YON;
   parallel = FALSE;
   zoom = 1.0; 
   FpCpuPresent = TRUE;
   ColorPresent = TRUE;
   QD32Present = FALSE;
}

/*
 * =================================
 * NAME:
 *            dxfsout.c
 *
 * DESCRIPTION:
 *            Routines for producing output for the surf module, using data from a
 *            previously loaded DXF file.
 *
 * AUTHOR:
 *            Ewen Christopher
 *            (c) Ewen Christopher 1995
 *            (c) Monash University 1995
 * =================================
 */

#include "xvt.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>
#include "dxf.h"

#define PI 3.1415927   
#define TOLERANCE 0.001

#ifndef DXF_SURF
#pragma message( "This file not useable without DXF_SURF" )
                                             /*       a file that doesn't exist */
#endif


/********************** Prototypes **********************/
#if XVT_CC_PROTO
static void rasterTriangle( struct DXFtriangle *triangle, ftype *array,
            int num[2], ftype min[2], ftype step[2] );
static ftype interpolate( struct DXFtriangle *triangle, ftype x, ftype y );
#else
static void rasterTriangle( );
static ftype interpolate( );
#endif

/*
 * ===========================================
 * NAME:
 *            void
 *            DXFsurfRaster( struct DXFhandle *handle, int layerNum,
 *                                  ftype *array, int num[2], ftype min[2], ftype max[2] )
 *
 * DESCRIPTION:
 *            Converts a loaded DXF file into an array of heights.
 *            Any point in the rastor which does not have a triangle above it will
 *            have its height set to a value less than layer->min[2].
 *
 *            If a rastor point has more than one triangle above it, the height will
 *            be either of the heights.
 *
 *            The raster points are chosen to be evenly spaced, with a point on each
 *            boundary.  This routine does NOT allocate memory for the array.
 *
 * PARAMETERS:
 *            handle        Structure returned by DXFread
 *            layerNum      Layer of the file to use (usually 0)
 *            array         Used to store the heights
 *            num           The maximum array index for x,y.
 *            min           The lowest x,y values to use
 *            max           The highest x,y values to use
 *
 * RETURNS:
 *            Data is stored in the array <array>
 *
 * CALLED BY:
 *            user program
 * =======================================
 */
void
#if XVT_CC_PROTO
DXFsurfRaster( struct DXFhandle *handle, int layerNum,
                      ftype *array, int num[2], ftype min[2], ftype max[2] )
#else
DXFsurfRaster( handle, layerNum, array, num, min, max )
struct DXFhandle *handle;
int layerNum;
ftype *array;
int num[2];
ftype min[2];
ftype max[2];
#endif
{
   struct DXFlayer *layer;                   /* Layer to use */
   ftype step[2];                                /* distance between raster points */
   size_t elems;                                 /* number of elements */
   struct DXFtriangle *triangle;         /* current triangle */
   ftype notFound;                               /* Value to use when no triangle found */
   size_t i;

   assert( !(handle->resultFlags & fFATALMASK ) );
   assert( handle->conv == convSurf );
   assert( handle->numLayers > layerNum );

   layer = handle->layer + layerNum;
   elems = num[0] * num[1];

   notFound = (ftype) (handle->min[2] - (ftype) fabs( handle->min[2] ));     /* no triangle found */
   for( i=0; i<elems; i++ )
      array[i] = notFound;

   for( i=0; i<2; i++ )
      step[i] = (max[i] - min[i]) / (num[i] - 1);

   for( triangle=layer->triangles; triangle!=NULL; triangle=triangle->next )
      rasterTriangle( triangle, array, num, min, step );
}


/*
 * ===================================
 * NAME:
 *            static void
 *            rasterTriangle( struct DXFtriangle *triangle,
 *                                    ftype *array, int numX,
 *                                    ftype minX, ftype stepX,
 *                                    ftype minY, ftype stepY )
 *
 * DESCRIPTION:
 *            Determines the heights of any raster points lying within the boundary
 *            of the triangle.
 *
 * PARAMETERS:
 *            triangle   - pointer to the triangle
 *            array      - where to store the heights
 *            numX          - number of elements in a row of the array
 *            minX          - minimum X value in the plane
 *            stepX      - spacing between raster points in the X direction
 *            minY, stepY - as for X
 *
 * CALLED BY:
 *            DXFsurfRaster
 * ====================================
 */
static void
#if XVT_CC_PROTO
rasterTriangle( struct DXFtriangle *triangle, ftype *array,
                        int num[2], ftype min[2], ftype step[2] )
#else
rasterTriangle( triangle, array, num, min, step )
struct DXFtriangle *triangle;
ftype *array;
int num[2];
ftype min[2];
ftype step[2];
#endif
{
   ftype x, y;                             /* x, y of current raster point */
   int xindex,  yindex;                /* index of raster points in array */
   int ylowidx;                            /* lowest y index */
   ftype height;                        /* height of current raster point */

                                                            /* lowest index for this triangle */
   xindex = (int)((triangle->min[0] - min[0]) / step[0]);
   ylowidx = (int)((triangle->min[1] - min[1]) / step[0]);

   x = xindex * step[0] + min[0];
   while( x <= triangle->max[0] )                          /* loop on x points */
   {
      yindex = ylowidx;
      y = yindex * step[1] + min[1];
      while( y <= triangle->max[1] )                       /* loop on y points */
      {
         if( dxfInsideTriangle( triangle->corner, 0, 1, x, y ) )
         {
            height = interpolate( triangle, x, y );          /* find & store height */
            *(array + xindex * num[0] + yindex) = height;
         }
         yindex++;
         y = yindex * step[1] + min[1];
      }
      xindex++;
      x = xindex * step[0] + min[0];           /* recalculate to avoid rounding */
   }
}


/*
 * ====================================
 * NAME:
 *            static ftype
 *            interpolate( struct DXFtriangle *triangle, ftype x, ftype y )
 *
 * DESCRIPTION:
 *            Find the height at any point within a triangle.  This is achieved by
 *            finding the plane represented by the triangle and then solving for the
 *            height.  The plane is found in advance by the function minMax.
 *            No check is make to see if the point is within the triangle.
 *
 * MATHS:
 *            The plane is defined by ax + by + cz + d = 0
 *            If the corners of the triangle are A, B & C, then the normal vector of
 *            the plane is AB x AC.  The third corner is used as A.
 *
 * CALLED BY:
 *            rasterTriangle
 *            DXFsurfHeight
 * ====================================
 */
static ftype
#if XVT_CC_PROTO
interpolate( struct DXFtriangle *triangle, ftype x, ftype y )
#else
interpolate( triangle, x, y )
struct DXFtriangle *triangle;
ftype x;
ftype y;
#endif
{
   ftype height;                        /* calculated height */

   if( triangle->plane[2] == 0 )   /* beware division by zero = vertical plane */
      height = (ftype) 0;
   else
      height = -(triangle->plane[0] * x + triangle->plane[1] * y
                                                   + triangle->plane[3]) / triangle->plane[2];

   return height;
}




/*
 * =======================================
 * NAME:
 *            int
 *            DXFsurfHeight( struct DXFhandle *handle, int layerNum,
 *                            ftype x, ftype y, ftype z,
 *                            ftype *dist )
 *
 * DESCRIPTION:
 *            Find the distance from an arbitrary point (x,y,z) to the surface,
 *            along the z axis.
 *
 * PARAMETERS:
 *            handle           Structure returned by DXFread.
 *            layerNum         The layer to use (usually 0).
 *            x, y, z          The point to find the distance from.
 *            dist                Used to return the distance.
 *
 * RETURNS:
 *            Non-zero if the point if above the plane, 0 otherwise.
 *            <dist> is used to return the distance.
 *
 * CALLED BY:
 *            user program
 * =======================================
 */
int
#if XVT_CC_PROTO
DXFsurfHeight( struct DXFhandle *handle, int layerNum,
                ftype x, ftype y, ftype z,
                ftype *dist )
#else
DXFsurfHeight( handle, layerNum, x, y, z, dist )
struct DXFhandle *handle;
int layerNum;
ftype x, y, z;
ftype *dist;
#endif
{
   struct DXFlayer *layer;                /* Layer to use */
   struct DXFtriangle *tlist;               /* list of possible triangles */

   int xRegn, yRegn;                            /* region index */
   struct DXFtriangle **rlist;           /* pointer to list of regions */

   ftype height;                              /* height at the point */

   assert( !(handle->resultFlags & fFATALMASK) );
   assert( handle->conv == convSurf );
   assert( handle->numLayers > layerNum );

   layer = handle->layer + layerNum;
   assert( layer->regions != NULL );

   xRegn = (int)((x - layer->min[0]) / layer->step[0]);
   yRegn = (int)((y - layer->min[1]) / layer->step[1]);

   if(      (xRegn >= 0) && (xRegn < layer->numRegns[0]) /* check valid region */
         && (yRegn >= 0) && (yRegn < layer->numRegns[1]) )
   {
      rlist = layer->regions[ xRegn * layer->numRegns[0] + yRegn ];
      if( rlist != NULL )
      {
         while( *rlist != NULL )
         {
            if( dxfInsideTriangle( (*rlist)->corner, 0, 1, x, y ) )
            {
               height = interpolate( *rlist, x, y );
               height = height - z;
               *dist = height;
               return 1;
            }
            rlist++;
         }
      }
   }

   return 0;
}




/*
 * ==========================
 * NAME:
 *            int
 *            DXFsurfDist( struct DXFhandle *handle, int layerNum,
 *                               ftype x, ftype y, ftype z,
 *                               ftype *nx, ftype *ny, ftype *nz )
 *
 * DESCRIPTION:
 *            Converts x,y,z from original coordinates into transformed coordinates
 *            (ignoring any matrix operations).  This is to simplify use of the
 *            package when the aCentroid option is used.
 *            The z coordinate will then be altered to be the distance from the
 *            surface of that x,y,z point.
 *
 * PARAMETERS:
 *            handle        Structure returned by DXFread.
 *            layerNum      Layer to use (usually 0).
 *            x, y, z       Point to find distance from.
 *            nx, ny        Transformed x, y.
 *            nz                  Distance from point to plane.
 *
 * RETURNS:
 *            Zero is returned if the point cannot be projected onto the plane,
 *            otherwise non-zero is returned.
 *            nx, ny, nz are set to the 'new' x,y,z values
 *
 * CALLED BY:
 *            user program
 * ==========================
 */
int
#if XVT_CC_PROTO
DXFsurfDist( struct DXFhandle *handle, int layerNum,
                   ftype x, ftype y, ftype z,
                   ftype *nx, ftype *ny, ftype *nz )
#else
DXFsurfDist( handle, layerNum, x, y, z, nx, ny, nz )
struct DXFhandle *handle;
int layerNum;
ftype x, y, z, *nx, *ny, *nz;
#endif
{
   struct DXFlayer *layer;             /* Layer to use */
   ftype point[3];                         /* Mutilated point */

   assert( !(handle->resultFlags & fFATALMASK) );
   assert( handle->conv == convSurf );
   assert( layerNum < handle->numLayers );

   layer = handle->layer + layerNum;
   point[0] = x;
   point[1] = y;
   point[2] = z;

   dxfChangeOrigin( point, handle->centroid );
   dxfChangeAxis( point, handle->axis[0], handle->axis[1], handle->axis[2] );

   *nx = point[0];
   *ny = point[1];
   return DXFsurfHeight( handle, layerNum, point[0], point[1], point[2], nz );
}


/*
 * =================================
 * NAME:
 *            void
 *            DXFsurfAngle( struct DXFhandle *handle, ftype *intersect, ftype *axis )
 *
 * DESCRIPTION:
 *            Determines angles of rotation due to the arbitrary axis routine used
 *            when aCentroid is chosen.
 *
 * PARAMETERS:
 *            handle              The handle to use.
 *            intersect             Used to return the angle the line of intersection
 *                                    between the least square's plane and the original axis,
 *                                    after being rotated.
 *            axis                   Used to return the angle of the original X axis, after
 *                                    being rotated.
 *
 * RETURNS:
 *            Data is returned via the <intersect> and <axis> parameters.
 *
 * NOTE:
 *            Angles are in radians and only consider rotations in the plane (about
 *            the z axis).
 *
 * METHOD:
 *            To find the angle of the rotated line of intersection:  The line of
 *            intersection of the least squares plane and the original xy plane is
 *            found.  The least squares plane is ax+by+cz+d=0, the xy plane is z=0,
 *            giving ax+by+d=0 for the line of intersection.  Taking care to avoid
 *            horizontal planes (a=b=0) and vertical lines (b=0), the angle is found.
 *            The angle of rotation is then added.
 * ==================================
 */
void
#if XVT_CC_PROTO
DXFsurfAngle( struct DXFhandle *handle, ftype *intersect, ftype *axis )
#else
DXFsurfAngle( handle, intersect, axis )
struct DXFhandle *handle;
ftype *intersect;
ftype *axis;
#endif
{
   ftype point[2][3];                     /* 2 * two-dimensional points (Z is ignored) */
   ftype a, b, c, d;                      /* Parameters of the plane */

                                                /* Find rotation of axis */
   point[0][0] = point[0][1] = point[0][2] = (ftype) 0;       /* point (0,0,0) */
   point[1][0] = (ftype) 1;                                                /* point (1,0,0) */
   point[1][1] = point[1][2] = (ftype) 0;
   dxfChangeAxis( point[0], handle->axis[0], handle->axis[1], handle->axis[2] );
   dxfChangeAxis( point[1], handle->axis[0], handle->axis[1], handle->axis[2] );
   *axis = (ftype)atan2( point[1][1] - point[0][1], point[1][0] - point[0][0] );

                                                /* Find line of intersection */
   a = -handle->axis[2][0];
   b = -handle->axis[2][1];
   c = -handle->axis[2][2];
   d = handle->planeConst;
   if( (a < ZTOL) && (b < ZTOL) )        /* Plane is horizontal */
   {
      *intersect = (ftype) 0;
   }
   else if( b < ZTOL )           /* Plane vertical, right angles to x axis */
   {
      *intersect = (ftype)((PI / 2) + *axis);
   }
   else
   {
      point[0][0] = (ftype) 0;                        /* Choose points x=0, x=1 */
      point[0][1] = -d / b;

      point[1][0] = (ftype) 1;
      point[1][1] = -(d + a) / b;

      *intersect = (ftype)atan2( point[1][1] - point[0][1],
                                              point[1][0] - point[0][0] );
      *intersect += *axis;
   }
}

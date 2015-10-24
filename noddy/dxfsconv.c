/*
 * =============================================================
 * NAME:
 *            dxfsconv.c
 *
 * DESCRIPTION:
 *            Contains the routines for processing the data imported by surfin,
 *            with an eye to later conversion of the data to a raster of heights.
 *
 * AUTHOR:
 *            Ewen Christopher
 *            (c) Ewen Christopher 1995
 *            (c) Monash University 1995
 * =============================================================
 */

#include "xvt.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>

#include "dxf.h"

#ifndef DXF_SURF
#pragma message( "This file not useable without DXF_SURF" )
                                             /*       a file that doesn't exist */
#endif


/******************* Prototypes *******************/
#if XVT_CC_PROTO
static void alignAxis( struct DXFhandle *handle );
static int leastSquares( int numLayers, struct DXFlayer *layer,
       ftype *a, ftype *b, ftype *c, ftype *d, ftype mean[3] );
static int solveMatrix( ftype Ab[3][4], ftype x[3] );

static void surfPreCalc( struct DXFlayer *layer );

static int crossRegion( ftype corner[3][3], ftype min[2],
                                    ftype max[2] );
static void origMinMax( struct DXFhandle *handle );
#else
static void alignAxis( );
static int leastSquares( );
static int solveMatrix( );

static void surfPreCalc( );

static int crossRegion( );
static void origMinMax( );
#endif


/*
 * =======================
 * NAME:
 *            void
 *            DXFsurfConv( struct DXFhandle *handle,
 *                               enum DXFaxis axistype,
 *                               ftype matrix[4][4],
 *                               int regionalise )
 *
 * DESCRIPTION:
 *            Converts the data read in from a DXF file from the 3D WCS form to
 *            a form more suitable for producing rasters of heights.  Specifically,
 *            it:
 *                  Frees any memory from a previous conversion.
 *                  Aligns the axis as requested.
 *                  Applies any requested matrix transformations.
 *                  Divides the triangles into regions if requested.
 *
 * PRE:
 *            <handle> has been correctly set up by DXFread.
 *            No fatal errors have occured.
 *
 * CALLED BY
 *            user program
 * ========================
 */
int
#if XVT_CC_PROTO
DXFsurfConv( struct DXFhandle *handle,
        enum DXFaxis axistype,
        ftype matrix[4][4],
        int regionalise )
#else
DXFsurfConv( handle, axistype, matrix, regionalise )
struct DXFhandle *handle;
enum DXFaxis axistype;
ftype matrix[4][4];
int regionalise;
#endif
{
   int lnum;                         /* current layer */
   struct DXFlayer *layer;
   int i;
   unsigned alloc, curreg;                /* for freeing memory */
   struct DXFtriangle *triangle;

   if (!( !(handle->resultFlags & fFATALMASK) ))
      return (FALSE);
   if (!( (handle->conv == convNone) || (handle->conv == convSurf) ))
      return (FALSE);

   if( handle->conv == convNone )        /* First conversion */
   {
      handle->conv = convSurf;
      origMinMax( handle );                  /* Find original min/max */
   }

   for( lnum=0; lnum < handle->numLayers; lnum++ )
   {
      layer = handle->layer + lnum;

      assert( layer->triangles != NULL );        /* should never have empty layers */

      triangle = layer->triangles;                   /* retrieve original points */
      while( triangle != NULL )
      {
         memcpy( triangle->corner, triangle->origcor, sizeof(ftype)*9 );
         triangle = triangle->next;
      }
   }

   if( axistype == aCentroid )
      alignAxis( handle );
   else
   {                                     /* init centroid, etc as empty */
      handle->centroid[0] = handle->centroid[1] = handle->centroid[2] = 0;
      handle->axis[0][0] = handle->axis[1][1] = handle->axis[2][2] = 1;
      handle->axis[0][1] = handle->axis[0][2] = 0;
      handle->axis[1][0] = handle->axis[1][2] = 0;
      handle->axis[2][0] = handle->axis[2][1] = 0;
      handle->planeConst = 0;
   }

   if( matrix != NULL )
   {
      for( lnum=0; lnum < handle->numLayers; lnum++ )
         dxfApplyMatrix( handle->layer[lnum].triangles, matrix );
   }
   dxfMinMax( handle );

   for( lnum=0; lnum < handle->numLayers; lnum++ )
   {
      layer = handle->layer + lnum;

      surfPreCalc( layer );
      if( regionalise )
         handle->resultFlags |= dxfDivideRegions( layer, convSurf );
   }
   return (TRUE);
}




/*
 * ======================================
 * NAME:
 *            static void
 *            alignAxis( struct DXFhandle *handle )
 *
 * DESCRIPTION:
 *            Adjust axis so that the centroid lies at the origin, and the plane made
 *            by the x,y axis is the least squares plane of all the points.
 *
 * PRE:
 *            The <corner> field of each triangle has been initialised from <origcor>
 *
 * CALLED BY:
 *            DXFconvert
 * ======================================
 */
static void
#if XVT_CC_PROTO
alignAxis( struct DXFhandle *handle )
#else
alignAxis( handle )
struct DXFhandle *handle;
#endif
{
   ftype a, b, c, d;                            /* parameters of the least squares plane */
   ftype cent[3];                             /* centroid of the plane */

   ftype Xl[3], Yl[3], Zl[3];               /* least squares axis */
   ftype arb[3];                              /* an arbitrary axis */

   int layerNum;                              /* current layer number */
   struct DXFtriangle *triangle;      /* current triangle */
   int i;

   if( !leastSquares( handle->numLayers, handle->layer,
                               &a, &b, &c, &d, cent ) )
   {
      handle->resultFlags |= fAXIS;
      return;
   }


   Zl[0] = -a;                                /* Find least squares axis */
   Zl[1] = -b;
   Zl[2] = -c;
   handle->planeConst = d;
   dxfArbitraryAxis( Xl, Yl, Zl );

   for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
   {
      triangle = handle->layer[layerNum].triangles;
      while( triangle != NULL )
      {
         for( i=0; i<3; i++ )
         {
            dxfChangeOrigin( triangle->corner[i], cent );
            dxfChangeAxis( triangle->corner[i], Xl, Yl, Zl );   /* project */
         }
         triangle = triangle->next;
      }
   }

   memcpy( handle->axis[0], Xl, sizeof(ftype)*3 );       /* save axis for later */
   memcpy( handle->axis[1], Yl, sizeof(ftype)*3 );
   memcpy( handle->axis[2], Zl, sizeof(ftype)*3 );
   memcpy( handle->centroid, cent, sizeof(ftype)*3 );
}



/*
 * ==================================================
 * NAME:
 *            static int
 *            leastSquares( int numLayers, struct DXFlayer *layer,
 *                                 ftype *a, ftype *b, ftype *c, ftype *d,
 *                                 ftype mean[3] )
 *
 * DESCRIPTION:
 *            Determines the least squares plane for a set of triangles
 *            The plane is described by the equation  ax + by + cz +d = 0
 *
 * PARAMETERS:
 *            triangles          First triangle in list
 *            a, b, c, d         Used to return the parameters of the plane
 *            mean                Used to return the mean point of the points
 *
 * PRE:
 *            There are some triangles pointed to by the layer handles.
 *
 * RETURNS:
 *            0 if an error occurs or the preconditions are not met.
 *            Non-zero otherwise.
 *
 * CALLED BY:
 *            alignAxis
 * ===================================================
 */
static int
#if XVT_CC_PROTO
leastSquares( int numLayers, struct DXFlayer *layer,
                     ftype *a, ftype *b, ftype *c, ftype *d,
                     ftype mean[3] )
#else
leastSquares( numLayers, layer, a, b, c, d, mean )
int numLayers;
struct DXFlayer *layer;
ftype *a, *b, *c, *d;
ftype mean[3];
#endif
{
   struct DXFtriangle *triangle;
   int layerNum;                              /* current layer number */
   int numPoints = 0;                           /* number of triangles * 3 */
   ftype sum2[2];                             /* sum of x**2, y**2 */
   ftype byz[2];                              /* sum of x*z, y*z */
   ftype sumxy;                                  /* sum of x*y */
   ftype x, y, z;                             /* local copies of coordinates */
   int i;

   ftype coeff[3][4];                           /* matricies for finding parameters */
   ftype result[3];

   mean[0] = mean[1] = mean[2] = (ftype)0.0;          /* clear sums */
   sum2[0] = sum2[1] = (ftype)0.0;
   byz[0] = byz[1] = (ftype)0.0;
   sumxy = (ftype)0.0;

   for( layerNum=0; layerNum < numLayers; layerNum++ )
   {
      triangle = layer[layerNum].triangles;
      while( triangle != NULL )
      {
         numPoints += 3;
         for( i=0; i<3; i++ )                   /* each corner */
         {
            x = triangle->corner[i][0];
            y = triangle->corner[i][1];
            z = triangle->corner[i][2];

            mean[0] += x;
            mean[1] += y;
            mean[2] += z;

            sum2[0] += (x * x);
            sum2[1] += (y * y);

            byz[0] += (x * z);
            byz[1] += (y * z);

            sumxy += (x * y);
         }
         triangle = triangle->next;
      }
   }

   if( numPoints == 0 )
      return 0;

   coeff[0][0] = mean[0];
   coeff[0][1] = mean[1];
   coeff[0][2] = numPoints;

   coeff[1][0] = sumxy;
   coeff[1][1] = sum2[1];
   coeff[1][2] = mean[1];

   coeff[2][0] = sum2[0];
   coeff[2][1] = sumxy;
   coeff[2][2] = mean[0];

   coeff[0][3] = mean[2];               /* right-hand-side of matrix equation */
   coeff[1][3] = byz[1];
   coeff[2][3] = byz[0];

   solveMatrix( coeff, result );

   *a = result[0];
   *b = result[1];
   *c = -1;
   *d = result[2];

   mean[0] /= numPoints;                  /* convert from sums to means */
   mean[1] /= numPoints;
   mean[2] /= numPoints;

   return 1;                         /* no errors */
}


/*
 * ===========================================
 * NAME:
 *            static int
 *            solveMatrix( ftype Ab[3][4],
 *                               ftype x[3] )
 *
 * DESCRIPTION:
 *            Solve a 3x3 system of matricies of the form Ax = b for the vector x.
 *
 * PARAMETERS:
 *            Ab         Augmented matrix of A and b
 *            x          Used to return the result vector
 *
 * RETURNS:
 *            0 if the matrix is singular, non-zero otherwise.
 *
 * NOTE:
 *            The arrays Ab and x are altered by this routine.
 *
 * METHOD:
 *            Gaussian elimination with pivoting (to reduce error)
 *            See [Matthews], Algorithm 3.2
 *
 * CALLED BY:
 *            leastSquares
 * ===========================================
 */
static int
#if XVT_CC_PROTO
solveMatrix( ftype Ab[3][4],          /* Augmented matrix of A and b */
                   ftype x[3] )            /* result vector */
#else
solveMatrix( Ab, x )
ftype Ab[3][4];          /* Augmented matrix of A and b */
ftype x[3];            /* result vector */
#endif
{
   int r;                                      /* row index */
   int c;                                      /* column index */
   int temp;                                     /* swap variable */
   int p;                                      /* pivot row/column */
   int row[3];                                /* pivoting translation vector */
   ftype mult;                                /* multiplier for elements */
   ftype sum;

   for( r=0; r<3; r++ )                   /* no pivoting so far */
      row[r] = r;

   for( p=0; p<2; p++ )     /* Upper triangularisation phase */
   {
      for( r=p+1; r<3; r++ )      /* find pivot element */
      {
         if( fabs( Ab[row[r]][p] ) > fabs( Ab[row[p]][p] ) )
         {
            temp = row[p];                     /* switch pivot rows */
            row[p] = row[r];
            row[r] = temp;
         }
      }

      if( Ab[row[p]][p] == 0 )         /* singular matrix */
         return 0;

    for( r=p+1; r<3; r++ )                                  /* perform row operations */
      {
         mult = Ab[row[r]][p] / Ab[row[p]][p];
      for( c=p+1; c<4; c++ )
         {
            Ab[row[r]][c] -= (mult * Ab[row[p]][c]);
         }
      }
   }

   if( Ab[2][2] == 0 )                    /* singular matrix */
      return 0;

   x[2] = Ab[row[2]][3] / Ab[row[2]][2];         /* back substitution */
   for( r=2; r>=0; r-- )
   {
      sum = (ftype)0.0;
      for( c=r+1; c<3; c++ )
      {
         sum += Ab[row[r]][c] * x[c];
      }
      x[r] = ( Ab[row[r]][3] - sum ) / Ab[row[r]][r];
   }

   return 1;                                  /* successful completion */
}


/*
 * =====================================
 * NAME
 *            static void
 *            surfPreCalc( struct DXFlayer *layer )
 *
 * DESCRIPTION:
 *            Performs calculations whose results are used later.  Specifically, it
 *            - Marks degenerate triangles.
 *            - Finds the equations for the plane of the triangle.
 *            - Finds the equations for the bounds of the triangle.
 *            - Finds min/max values of coordinates.
 *            - Counts the number of non-degenerate triangles.
 *            Note that no operations are performed on degenerate triangles.
 *
 * METHOD:
 *            Degenerate triangles are detected by
 *            - two points with the same location, OR
 *            - two sides parallel
 *
 * CALLED BY:
 *            DXFconvert
 * ======================================
 */
static void
#if XVT_CC_PROTO
surfPreCalc( struct DXFlayer *layer )
#else
surfPreCalc( layer )
struct DXFlayer *layer;
#endif
{
   struct DXFtriangle *triangle;            /* current triangle */
   ftype vec1[3], vec2[3];
   ftype cross[3];                                  /* Cross product */
   ftype diff[2][3];                                  /* vectors AB, AC */
   int i, j;

   layer->numTriangles = 0;
   triangle = layer->triangles;

   while( triangle != NULL )
   {
      for( i=0; i<2; i++ )             /* x, y, or z */
      {
         vec1[i] = triangle->corner[1][i] - triangle->corner[0][i];
         vec2[i] = triangle->corner[2][i] - triangle->corner[0][i];
      }

      vec1[2] = vec2[2] = 0;              /* Find degenerate (2D) triangles */
      dxfCrossProduct( cross, vec1, vec2 );
      if( cross[2] == 0 )
         triangle->degenerate = 1;
      else
         triangle->degenerate = 0;

      if( !triangle->degenerate )                    /* ignore degenerate triangles */
      {
         layer->numTriangles++;

         for( i=0; i<2; i++ )                /* point B or C */
         {
            for( j=0; j<3; j++ )             /* x, y, or z */
            {
               diff[i][j] = triangle->corner[i][j] - triangle->corner[2][j];
            }
         }
                                                                  /* Find plane of triangle */
         triangle->plane[0] = diff[0][1] * diff[1][2] - diff[0][2] * diff[1][1];
         triangle->plane[1] = diff[0][2] * diff[1][0] - diff[0][0] * diff[1][2];
         triangle->plane[2] = diff[0][0] * diff[1][1] - diff[0][1] * diff[1][0];
         triangle->plane[3] = -(  triangle->plane[0] * triangle->corner[0][0]
                                 + triangle->plane[1] * triangle->corner[0][1]
                                 + triangle->plane[2] * triangle->corner[0][2] );
      }
      triangle = triangle->next;
   }
}







/*
 * ==========================================
 * NAME:
 *            static void
 *            origMinMax( struct DXFhandle *handle )
 *
 * DESCRIPTION:
 *            Calculates the min,max values of the original coordinates.      These are
 *            stored in each layer and in the handle.
 *
 * CALLED BY:
 *            DXFread
 * ==========================================
 */
static void
#if XVT_CC_PROTO
origMinMax( struct DXFhandle *handle )
#else
origMinMax( handle )
struct DXFhandle *handle;
#endif
{
   int layerNum;                                             /* current layer number */
   struct DXFlayer *layer = NULL;                    /* current layer */
   struct DXFtriangle *triangle = NULL;             /* current triangle */
   int i, j;

   for( layerNum=0; layerNum < handle->numLayers; layerNum++ )  /* each layer */
   {
      layer = handle->layer + layerNum;

      triangle = layer->triangles;                                     /* Prime layer data */
      memcpy( layer->minO, triangle->origcor[0], sizeof(ftype)*3 );
      memcpy( layer->maxO, triangle->origcor[0], sizeof(ftype)*3 );

      for( triangle=layer->triangles; triangle!=NULL; triangle=triangle->next )
      {
         for( i=0; i<3; i++ )                               /* each corner */
         {
            for( j=0; j<3; j++ )                            /* x, y, z */
            {
               if( triangle->corner[i][j] < layer->minO[j] )
                  handle->minO[j] = triangle->corner[i][j];
               if( triangle->corner[i][j] > layer->maxO[j] )
                  handle->maxO[j] = triangle->corner[i][i];
            }
         }
      }

      if( layerNum == 0 )
      {
         memcpy( handle->minO, layer->minO, sizeof(ftype)*3 );
         /* memcpy( handle->maxO, layer->minO, sizeof(ftype)*3 ); */
         memcpy( handle->maxO, layer->maxO, sizeof(ftype)*3 );
      }
      else
      {
         for( i=0; i<3; i++ )                   /* combine result with handle */
         {
            if( layer->minO[i] < handle->minO[i] )
               handle->minO[i] = layer->minO[i];
            if( layer->maxO[i] > handle->maxO[i] )
               handle->maxO[i] = layer->maxO[i];
         }
      }
   }
}

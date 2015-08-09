/*
 * ==============================
 * NAME:
 *                      dxfvconv.c
 *
 * DESCRIPTION:
 *                      Source code for applying the 'vol' conversions and pre-processing to
 *                      triangulated data which was read in by DXFread.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher 1995
 *                      (c) Monash University 1995
 * ==================================
 */

#include "xvt.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>
#include "dxf.h"

#ifndef DXF_VOL
#pragma message( "This file not useable without DXF_VOL" )
                                                                                                                        /*       a file that doesn't exist */
#endif

/*************** Prototypes **************/
#if XVT_CC_PROTO
static int orientTriangles( struct DXFtriangle **triList );
static int matchEdge( ftype corner[3][3], struct DXFtriangle *triangles,
                                int *flip );
static void preCalc( struct DXFtriangle *triangle );
static int findOrientation( struct DXFlayer *layer );
#else
static int orientTriangles( );
static int matchEdge( );
static void preCalc( );
static int findOrientation( );
#endif

/*
 * ===========================
 * NAME:
 *                      void
 *                      DXFvolOrient( struct DXFhandle *handle )
 *
 * DESCRIPTION:
 *                      Examines the triangles in each layer of a file and orients the triangles
 *                      so that the inside and outside can be determined.  This only needs to be
 *                      called once for each file loaded.
 *
 * PARAMETERS:
 *                      handle                          The handle for the file to orient.
 *
 * RETURNS:
 *                      <handle> and the triangles it points to are modified.
 * ============================
 */
int 
#if XVT_CC_PROTO
DXFvolOrient( struct DXFhandle *handle )
#else
DXFvolOrient( handle )
struct DXFhandle *handle;
#endif
{
        int layerNum;

        if (!( !(handle->resultFlags & fFATALMASK) ))
           return (FALSE);
        if (!( (handle->conv == convNone) || (handle->conv == convVol) ))
           return (FALSE);

        handle->conv = convVol;

        for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
        {
                handle->resultFlags |= orientTriangles( &(handle->layer[layerNum].triangles) );
        }
        return (TRUE);
}



/*
 * ========================
 * NAME:
 *                      void
 *                      DXFvolConv( struct volHandle *handle, ftype matrix[4][4],
 *                                                                      int orient )
 *
 * DESCRIPTION:
 *                      Control routine for
 *                      - applying a matrix transformation (if matrix != NULL)
 *                      - find the min/max values
 *                      - pre-calculate values for later use
 *                      - dividing the triangles into regions
 *
 * PARAMETERS:
 *                      handle                  Pointer to the handle for this file
 *                      matrix                  Transformation matrix, or NULL
 *                      inside                  If set, the inside and outside of the polyhedron is
 *                                                                              determined.
 * ========================
 */
int
#if XVT_CC_PROTO
DXFvolConv( struct DXFhandle *handle, ftype matrix[4][4], int inside )
#else
DXFvolConv( handle, matrix, inside )
struct DXFhandle *handle;
ftype matrix[4][4];
int inside;
#endif
{
        int layerNum;                                                                                                   /* current layer */
        struct DXFlayer *layer = NULL;

        struct DXFtriangle *triangle = NULL;            /* for copying coordinates */

        if (!( !(handle->resultFlags & fFATALMASK) ))
           return (FALSE);
        if (!( (handle->conv == convNone) || (handle->conv == convVol) ))
           return (FALSE);
        handle->conv = convVol;

                                                                                                                                                                        /* process each layer */
        for( layerNum=0; layerNum<handle->numLayers; layerNum++ )
        {
                layer = handle->layer + layerNum;

                for( triangle=layer->triangles; triangle!=NULL; triangle=triangle->next )
                {
                        memcpy( triangle->corner, triangle->origcor, sizeof(ftype)*9 );
                }

                if( matrix != NULL )
                        dxfApplyMatrix( layer->triangles, matrix );
        }

        dxfMinMax( handle );

        for( layerNum=0; layerNum<handle->numLayers; layerNum++ )
        {
                layer = handle->layer + layerNum;

                preCalc( layer->triangles );
                handle->resultFlags |= dxfDivideRegions( layer, convVol );
                if( inside )
                        handle->resultFlags |= findOrientation( layer );
        }
        return (TRUE);
}


/*
 * =================================
 * NAME:
 *                      static int
 *                      orientTriangles( struct DXFtriangle **triList )
 *
 * DESCRIPTION:
 *                      Sets the <flip> flag on each triangle so that all triangles have a
 *                      consistent orientation.  Adjacent triangles must have opposite
 *                      directions for their common edge.
 *
 * PARAMETERS:
 *                      triList:                Pointer to list of triangles to process.
 *
 * RETURNS:
 *                      Result flags.
 *                      <triList> is set to the head of the rearranged list.
 * ==================================
 */
static int
#if XVT_CC_PROTO
orientTriangles( struct DXFtriangle **triList )
#else
orientTriangles( triList )
struct DXFtriangle **triList;
#endif
{
        struct DXFtriangle *doneList = NULL;            /* oriented triangles */
        struct DXFtriangle *skipList = NULL;            /* triangles skipped until later */
        struct DXFtriangle *doList = NULL;                      /* triangles still to do */
        struct DXFtriangle *triangle = NULL;            /* current triangle */

        int progress;                                                   /* zero until progress has been made */
        int flip;                                                                       /* triangle has wrong orientation - flip it */
        ftype temp[3];                                                  /* for flipping triangles */

        if( *triList == NULL )                  /* beware empty list */
                return fNODATA;

        doneList = *triList;                            /* prime doneList with first triangle */
        doList = doneList->next;
        doneList->next = NULL;

        do
        {
                progress = 0;
                while( doList != NULL )                                 /* for each triangle in doList */
                {
                        triangle = doList;
                        doList = doList->next;

                        if( matchEdge( triangle->corner, doneList, &flip ) )
                        {
                                if( flip )                              /* wrong orientation, flip triangle */
                                {
                                        memcpy( temp, triangle->corner[0], sizeof(ftype)*3 );
                                        memcpy( triangle->corner[0], triangle->corner[1], sizeof(ftype)*3 );
                                        memcpy( triangle->corner[1], temp, sizeof(ftype)*3 );

                                        memcpy( temp, triangle->origcor[0], sizeof(ftype)*3 );
                                        memcpy( triangle->origcor[0], triangle->origcor[1], sizeof(ftype)*3 );
                                        memcpy( triangle->origcor[1], temp, sizeof(ftype)*3 );
                                }
                                triangle->next = doneList;                       /* matched - can use to match others */
                                doneList = triangle;
                                progress = 1;
                        }
                        else
                        {
                                triangle->next = skipList;                              /* no match - come back to it */
                                skipList = triangle;
                        }
                }

                if( doList == NULL )                            /* retry the skipped data */
                {
                        doList = skipList;
                        skipList = NULL;
                }
        } while( progress && (doList != NULL) );

        *triList = doneList;
        if( skipList != NULL )                                  /* could not create one large structure */
        {
                while( skipList != NULL )                       /* append to doneList */
                {
                        triangle = skipList;
                        skipList = skipList->next;
                        triangle->next = doneList;
                        doneList = triangle;
                }
                return fDISJOINT;
        }
        else
                return 0;
}


/*
 * ==========================
 * NAME:
 *                      static int
 *                      matchEdge( ftype corner[3][3], struct DXFtriangle *triangles,
 *                                                               int *flip )
 *
 * DESCRIPTION:
 *                      Searches for a match for one edge of a triangle in a given list of
 *                      triangles.
 *
 * PARAMETERS:
 *                      corner                  The corners of the triangle to match.
 *                      triangles       List of triangles to search for a match.
 *                      flip                            Used to return a flag.
 *
 * RETURNS:
 *                      Non-zero if a matching edge is found, zero otherwise.
 *                      <flip> is set if the triangle needs to be flipped.
 * ==========================
 */
static int
#if XVT_CC_PROTO
matchEdge( ftype corner[3][3], struct DXFtriangle *triangles, int *flip )
#else
matchEdge( corner, triangles, flip )
ftype corner[3][3];
struct DXFtriangle *triangles;
int *flip;
#endif
{
        int i, j;

        for( ; triangles!=NULL; triangles=triangles->next )
        {
                for( i=0; i<3; i++ )                                    /* find a corner which matches */
                {
                        for( j=0; j<3; j++ )
                        {
                                if( dxfCmpCorner( corner[j], triangles->corner[i] ) )  /* match! */
                                {                                                                                                                                                                               /* find a second corner */
                                        if( dxfCmpCorner( corner[(j+1)%3], triangles->corner[(i+1)%3] ) )
                                        {
                                                *flip = 1;
                                                return 1;
                                        }
                                        else if( dxfCmpCorner( corner[(j+1)%3], triangles->corner[(i+2)%3] ) )
                                        {
                                                *flip = 0;
                                                return 1;
                                        }
                                }
                        }
                }

        }
        return 0;
}




/*
 * ==========================
 * NAME:
 *                      static void
 *                      preCalc( struct DXFtriangle *triangle )
 *
 * DESCRIPTION:
 *                      Calculates values stored with each triangle which enable faster
 *                      operation later on.  These values are:
 *                      - Axis for a coordinate system where the triangle is horizontal.
 *                      - The corners of the triangle in this coordinate system.
 *                      - Elevation of the triangle from the x,y plane, in this system.
 *                      - Equations for the lines of the triangle, in this system.
 *
 * PRE:
 *                      No degenerate triangles exist in the input (these should have been
 *                      removed by readDxf).
 *
 * METHOD:
 *                      The plane is chosen by AutoCAD's Arbitrary Axis method.
 * ==========================
 */
static void
#if XVT_CC_PROTO
preCalc( struct DXFtriangle *triangle )
#else
preCalc( triangle )
struct DXFtriangle *triangle;
#endif
{
        ftype vec1[3], vec2[3];                         /* vectors used for finding vector of plane */
        ftype points[3][3];                                     /* points in triangle coords */
        int i, j;

        for( ; triangle!=NULL; triangle=triangle->next )
        {
                for( i=0; i<3; i++ )                            /* find Z axis from points of triangle */
                {
                        vec1[i] = triangle->corner[1][i] - triangle->corner[0][i];
                        vec2[i] = triangle->corner[2][i] - triangle->corner[0][i];
                }
                dxfCrossProduct( triangle->axis[2], vec1, vec2 );               /* z axis */
                memcpy( triangle->axis[0], vec1, sizeof(ftype)*3 );     /* x axis */
                dxfCrossProduct( triangle->axis[1], triangle->axis[2], vec1 ); /* Y=ZxX*/

                for( i=0; i<3; i++ )
                        dxfUnitVector( triangle->axis[i] );

                                                                                                                                                                                        /* project points onto axis */
                memcpy( points, triangle->corner, sizeof(ftype)*9 );
                for( i=0; i<3; i++ )
                {
                        dxfChangeAxis( points[i], triangle->axis[0],
                                                                                                                 triangle->axis[1],
                                                                                                                 triangle->axis[2] );
                }
                memcpy( triangle->points, points, sizeof(ftype)*9 );

                triangle->elevation = points[0][2]; /* all corners have same elevatn */


                for( i=0; i<3; i++ )                    /* find equations for lines in 2D */
                {
                        for( j=0; j<2; j++ )
                        {
                                triangle->line[i][j][0] = points[i][j];
                                triangle->line[i][j][1] = points[(i+1)%3][j]
                                                                                                        - points[i][j];
                        }
                }
        }
}



/*
 * ============================
 * NAME:
 *                      findOrientation
 *
 * DESCRIPTION:
 *                      Finds the orientation of the entire surface by looking at a face
 *                      from a position which must be outside the volume.
 *
 * PARAMETERS:
 *                      layer   The layer to find the orientation of.
 *
 * RETURNS:
 *                      Result flags.
 *                      Sets the <flip> flag in a DXFlayer.data structure.
 *
 * PRE:
 *                      There must be at least one triangle in the layer.
 *
 * METHOD:
 *                      Finds the "dot" value for a point known to be outside the volume.
 * ============================
 */
static int
#if XVT_CC_PROTO
findOrientation( struct DXFlayer *layer )
#else
findOrientation( layer )
struct DXFlayer *layer;
#endif
{
        ftype distance;                                                         /* Distance to the triangle */
        int inside;                                                                     /* Dot product with the triangle's vector */
        int retval = 0;                                                         /* Error flags to return */

        dxfClosestTriangle( layer,
                        layer->max[0]+10, layer->max[1]+10, layer->max[2]+10,
                        &distance, &inside );

        if( inside )
        layer->flip = 1;
        else
                layer->flip = 0;

        return retval;
}

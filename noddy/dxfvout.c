/*
 * ===================
 * NAME:
 *                      dxfvout.c
 *
 * DESCRIPTION:
 *                      Routines for producing output from the vol package.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher, 1995
 *                      (c) Monash University, 1995
 * ============================
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

/* Stack of regions yet to be examined */
struct regnStack
{
        int index[3];                                                                                           /* x,y,z index of region */
        ftype min[3];                                                                                           /* Border of region */
        ftype max[3];
        ftype distance;                                                                                         /* Distance to the point */
        struct regnStack *next;                                                         /* Next element in stack */
};


/******************** Prototypes ***************/
#if XVT_CC_PROTO
static void exploreRegion( struct DXFlayer *layer, int index[3], ftype point[3],
        struct DXFdistRes *distres );
static void addQ( struct regnStack **queue, int index[3],
        ftype min[3], ftype max[3], ftype distance );
static void getQ( struct regnStack **queue, int index[3], ftype min[3],
        ftype max[3], ftype *distance );
static ftype distRegn( ftype point[3],
        ftype min[3], ftype max[3] );
static void closestInRegn( struct DXFlayer *layer,
        struct DXFtriangle *table[],
        ftype x, ftype y, ftype z, struct DXFdistRes *distres );
static void distTriangle( struct DXFtriangle *triangle,
        ftype x, ftype y, ftype z, struct DXFdistRes *result );
static ftype distLine2D( ftype line[2][2], ftype x, ftype y,
        ftype *lparam );
static ftype distPoint2D( ftype x1, ftype y1, ftype x2, ftype y2 );
static void setRaster( int x, int y, int z, int num[3], ftype *dist,
        int *inside, ftype distance, int insideFlag );
#else
static void exploreRegion( );
static void addQ( );
static void getQ( );
static ftype distRegn( );
static void closestInRegn( );
static void distTriangle( );
static ftype distLine2D( );
static ftype distPoint2D( );
static void setRaster( );
#endif

/*************** Variables for statistics collection ****************/
#ifdef STATS
long int statClosest = 0;                               /* number of times dxfClosestTriangle called */
long int statRegions = 0;                               /* number of regions explored */
long int statTriangles = 0;                     /* number of triangles examined */
#endif

/*
 * ========================
 * NAME:
 *                      void
 *                      DXFvolDist( struct DXFhandle *handle, int layerNum,
 *                                                       ftype x, ftype y, ftype z,
 *                                                       ftype *distance, int *inside )
 *
 * DESCRIPTION:
 *                      Control routine for determining characteristics of an arbitrary point.
 *                      Specifically, how far it is from the surface, and whether it is inside
 *                      or outside.
 *
 * PARAMETERS:
 *                      handle                          Handle for the DXF file.
 *                      layerNum                        Number of the layer to use (usually 0).
 *                      x, y, z                         Point to find data for.
 *                      distance                        Used to return distance to surface.
 *                      inside                          Set to non-zero if inside, 0 if outside.
 *
 * RETURNS:
 *                      <distance> and <inside> are used to return the result.
 * =========================
 */
void
#if XVT_CC_PROTO
DXFvolDist( struct DXFhandle *handle, int layerNum,
                                 ftype x, ftype y, ftype z,
                                 ftype *distance, int *inside )
#else
DXFvolDist( handle, layerNum, x, y, z, distance, inside )
struct DXFhandle *handle;
int layerNum;
ftype x, y, z, *distance;
int *inside;
#endif
{
        assert( handle->conv == convVol );
        assert( handle->numLayers > layerNum );

        dxfClosestTriangle( handle->layer+layerNum, x, y, z, distance, inside );

        if( handle->layer[layerNum].flip )
        {
                if( *inside == iINSIDE )
                        *inside = iOUTSIDE;
                else if( *inside == iOUTSIDE )                  /* Extra test because of iUNKNOWN */
                        *inside = iINSIDE;
        }
}


/*
 * ==============================
 * NAME:
 *                      struct DXFtriangle
 *                      *dxfClosestTriangle( struct DXFlayer *layer,
 *                                                      ftype x, ftype y, ftype z, ftype *distance, int *inside )
 *
 * DESCRIPTION:
 *                      Finds the closest triangle to a point (x,y,z).
 *                      Also returns the distance from the point to the triangle, and the raw
 *                      inside/outside flag (ignoring layer.flip).
 *
 * PARAMETERS:
 *                      layer                           The layer to use.
 *                      x, y, z                         The point to find the closest triangle to.
 *                      distance                        Used to return the distance to the triangle.
 *                      inside                          Used to return the raw inside flag.
 *
 * RETURNS:
 *                      A pointer to the closest triangle.
 *                      <distance> and <inside> also return data.
 * ===============================
 */
struct DXFtriangle
#if XVT_CC_PROTO
*dxfClosestTriangle( struct DXFlayer *layer,
                                ftype x, ftype y, ftype z, ftype *distance, int *inside )
#else
*dxfClosestTriangle( layer, x, y, z, distance, inside )
struct DXFlayer *layer;
ftype x, y, z, *distance;
int *inside;
#endif
{
        struct DXFtriangle *triangle = NULL;            /* Closest triangle */
        struct DXFdistRes distres;                                                      /* Result of distance calculations */
        int index[3];                                                                                                   /* x,y indicies of current region */
        ftype point[3];                                                                                                 /* coordinates of point specified */
        int i;

#ifdef STATS
        statClosest++;
#endif

        assert( layer->regions != NULL );

        point[0] = x;
        point[1] = y;
        point[2] = z;

        for( i=0; i<3; i++ )                                                                                    /* find initial region */
        {
                index[i] = (int)((point[i] - layer->min[i]) / layer->step[i]);
                if( index[i] >= layer->numRegns[i] )
                        index[i] = layer->numRegns[i] - 1;
                if( index[i] < 0 )
                        index[i] = 0;
        }

        layer->tag++;                                                                                                           /* Need a new tag value */
        if( layer->tag == 0 )
                dxfClearTags( layer );

        distres.triangle = NULL;
        exploreRegion( layer, index, point, &distres );
        *distance = distres.distance;

        if( distres.difficult )
                *inside = iUNKNOWN;
        else if( distres.dot >= 0 )
                *inside = iINSIDE;
        else
                *inside = iOUTSIDE;

        return triangle;
}



/*
 * ======================
 * NAME:
 *                      static void
 *                      exploreRegion( struct DXFlayer *layer, int index[3], ftype point[3],
 *                                                                               struct DXFtriangle **triangle,
 *                                                                               ftype *distance, ftype *dot )
 *
 * DESCRIPTION:
 *                      Explores the regions, searching for the closest triangle to a point.
 *
 * PARAMETERS:
 *                      layer                   The layer to use.
 *                      index                   The initial region to search.
 *                      point                   The point to find a triangle closest to.
 *                      triangle                The closest triangle.
 *                      distance                Distance to the closest triangle.
 *                      dot                             Distance above/below the triangle.
 *
 * RETURNS:
 *                      Modifies <triangle>, <distance> & <dot>.
 *
 * NOTE:
 *                      This is a breadth-first-search, using an external queue.
 * =======================
 */
static void
#if XVT_CC_PROTO
exploreRegion( struct DXFlayer *layer, int index[3], ftype point[3],
                                                         struct DXFdistRes *distres )
#else
exploreRegion( layer, index, point, distres )
struct DXFlayer *layer;
int index[3];
ftype point[3];
struct DXFdistRes *distres;
#endif
{
        struct regnStack *stack = NULL;                         /* regions to examine */
        int regNum;                                     /* region number (index converted to 1 dimension) */
        int newIndex[3];                        /* Index value of an adjacent region */
        ftype rMin[3];                          /* Defining values of a region */
        ftype rMax[3];
        ftype rDist;                                    /* Distance to a region */
        int i, j, k;

        regNum = ((index[0] * layer->numRegns[1]) + index[1])
                                                                                        * layer->numRegns[2] + index[2];
        layer->regnTag[regNum] = layer->tag;                            /* mark first region */

        for( i=0; i<3; i++ )                                                                                            /* find border of region */
        {
                rMin[i] = layer->min[i] + index[i] * layer->step[i];
                rMax[i] = layer->min[i] + (index[i] + 1) * layer->step[i];
        }

        rDist = distRegn( point, rMin, rMax );
        addQ( &stack, index, rMin, rMax, rDist );

        while( stack != NULL )
        {
                getQ( &stack, index, rMin, rMax, &rDist );

                if(     ( distres->triangle != NULL )                   /* Skip region if out of range */
                         && ( rDist > distres->distance ) )
                {
                        continue;
                }

#ifdef STATS
                statRegions++;
#endif

                regNum = ((index[0] * layer->numRegns[1]) + index[1])
                                                                                                * layer->numRegns[2] + index[2];
                closestInRegn( layer, layer->regions[regNum],
                                                                         point[0], point[1], point[2], distres );

                for( i=-1; i<2; i++ )                                                   /* Investigate surrounding regions */
                {
                        newIndex[0] = index[0] + i;
                        if( (newIndex[0] < 0) || (newIndex[0] >= layer->numRegns[0]) )
                                continue;
                        rMin[0] = layer->min[0] + newIndex[0] * layer->step[0];
                        rMax[0] = layer->min[0] + (newIndex[0] + 1) * layer->step[0];

                        for( j=-1; j<2; j++ )
                        {
                                newIndex[1] = index[1] + j;
                                if( (newIndex[1] < 0) || (newIndex[1] >= layer->numRegns[1]) )
                                        continue;
                                rMin[1] = layer->min[1] + newIndex[1] * layer->step[1];
                                rMax[1] = layer->min[1] + (newIndex[1] + 1) * layer->step[1];

                                for( k=-1; k<2; k++ )
                                {
                                        if( (i==0) && (j==0) && (k==0) )        /* no use looking at this region! */
                                                continue;

                                        if( (i*j*k) != 0 )                                                              /* Don't do diagonal element */
                                                continue;

                                        newIndex[2] = index[2] + k;
                                        if( (newIndex[2] < 0) || (newIndex[2] >= layer->numRegns[2]) )
                                                continue;
                                        rMin[2] = layer->min[2] + newIndex[2] * layer->step[2];
                                        rMax[2] = layer->min[2] + (newIndex[2] + 1) * layer->step[2];

                                                                                                                                                                                /* check not already visited */
                                        regNum = ((newIndex[0] * layer->numRegns[1]) + newIndex[1])
                                                                                                                                 * layer->numRegns[2] + newIndex[2];
                                        if( layer->regnTag[regNum] != layer->tag )
                                        {
                                                                                                                                                         /* make sure not investigated again */
                                                layer->regnTag[regNum] = layer->tag;

                                                rDist = distRegn( point, rMin, rMax );
                                                if(     ( distres->triangle == NULL )
                                                         || ( rDist < distres->distance ) )
                                                {
                                                        addQ( &stack, newIndex, rMin, rMax, rDist );
                                                }
                                        }               /* end if visited */
                                }               /* end for k */
                        }               /* end for j */
                }               /* end for i */
        }               /* end while stack not empty */
}


/*
 * ==========================
 * NAME:
 *                      static void
 *                      addQ( struct regnStack **queue, int index[3], ftype min[3],
 *                                              ftype max[3], ftype distance )
 *
 * DESCRIPTION:
 *                      Add a region to the end of a queue.
 *
 * PARAMETERS:
 *                      queue                           The queue to use.
 *                      index                           The index of the region.
 *                      min, max                        The borders of the region.
 *
 * RETURNS:
 *                      <queue> is modified to contain the new data.
 * ==========================
 */
static void
#if XVT_CC_PROTO
addQ( struct regnStack **queue, int index[3], ftype min[3],
                        ftype max[3], ftype distance )
#else
addQ( queue, index, min, max, distance )
struct regnStack **queue;
int index[3];
ftype min[3], max[3], distance;
#endif
{
        struct regnStack *item = NULL;

        item = (struct regnStack *)xvt_mem_zalloc( sizeof(struct regnStack) );
        assert( item != NULL );
        if( *queue == NULL )
        {
                item->next = item;
        }
        else
        {
                item->next = (*queue)->next;
                (*queue)->next = item;
        }
        *queue = item;

        memcpy( item->index, index, sizeof(int)*3 );
        memcpy( item->min, min, sizeof(ftype)*3 );
        memcpy( item->max, max, sizeof(ftype)*3 );
        item->distance = distance;
}


/*
 * =======================
 * NAME:
 *                      static void
 *                      getQ( struct regnStack **queue, int index[3], ftype min[3], ftype max[3]
 *                                              ftype *distance )
 *
 * DESCRIPTION:
 *                      Remove the head from a queue of regions.
 *
 * PRE:
 *                      The queue is not empty (*queue != NULL).
 *
 * PARAMETERS:
 *                      queue                           The queue to use.
 *                      index                           Used to return the index of the region.
 *                      min, max                        Used to return the borders of the region.
 *
 * RETURNS:
 *                      The queue is modified.
 *                      <index>, <min> & <max> are used to return data.
 * =======================
 */
static void
#if XVT_CC_PROTO
getQ( struct regnStack **queue, int index[3], ftype min[3], ftype max[3],
                        ftype *distance )
#else
getQ( queue, index, min, max, distance )
struct regnStack **queue;
int index[3];
ftype min[3], max[3], *distance;
#endif
{
        struct regnStack *item;

        assert( *queue != NULL );
        item = (*queue)->next;
        if( item->next == item )
                *queue = NULL;
        else
                (*queue)->next = item->next;

        memcpy( index, item->index, sizeof(int)*3 );
        memcpy( min, item->min, sizeof(ftype)*3 );
        memcpy( max, item->max, sizeof(ftype)*3 );
        *distance = item->distance;

        xvt_mem_free( (char *) item );
}


/*
 * =========================
 * NAME:
 *                      static ftype
 *                      distRegn( ftype point[3],
 *                                                              ftype min[3], ftype max[3] )
 *
 * DESCRIPTION:
 *                      Determines the distance from a point to a region.
 *                      Undefined if the point is inside the region.
 *
 * PARAMETERS:
 *                      point                   The point to consider.
 *                      min, max                The borders of the region.
 *
 * RETURNS:
 *                      The distance to the region.
 *
 * METHOD:
 *                      Converts the problem to two dimensions, then uses Pythagorus to
 *                      combine this with the third dimension.
 * =========================
 */
static ftype
#if XVT_CC_PROTO
distRegn( ftype point[3], ftype min[3], ftype max[3] )
#else
distRegn( point, min, max )
ftype point[3], min[3], max[3];
#endif
{
        ftype dist[3];                                  /* Distance to the region in all three directions */
        int i;

        for( i=0; i<3; i++ )
        {
                if( point[i] > max[i] )
                        dist[i] = point[i] - max[i];
                else if( point[i] < min[i] )
                        dist[i] = min[i] - point[i];
                else                                                                                                                    /* Inside in this direction */
                        dist[i] = 0;
        }

        return (ftype)sqrt( dist[0]*dist[0] + dist[1]*dist[1] + dist[2]*dist[2] );
}



/*
 * ========================
 * NAME:
 *                      static void
 *                      closestInRegn( struct DXFlayer *layer,
 *                                                                               struct DXFtriangle *table[],
 *                                                                               ftype x, ftype y, ftype z,
 *                                                                               struct DXFtriangle **closest,
 *                                                                               ftype *distance, ftype *dot )
 *
 * DESCRIPTION:
 *                      Finds the closest triangle to a point (x,y,z), given a table of
 *                      triangles, as stored for each region.
 *
 * PARAMETERS:
 *                      layer                   The layer the triangles come from.
 *                      table                   The table of triangles to search.
 *                      x,y,z                   Coordinates of the arbitrary point.
 *                      closest                 The current closest triangle
 *                      distance                Used to return the distance to the nearest triangle.
 *                      dot                             Used to return the distance above the triangle.
 *
 * RETURNS:
 *                      <closest>, <distance> & <dot> are used to return data.
 *
 * BUGS:
 *                      When the two triangles have the same <distance>, and their <dot> values
 *                      are equal and opposite, the algorithm does not know which to use.  To
 *                      overcome this, it looks at the <dot> value from the previous call to
 *                      dxfClosestTriangle.
 * =============================
 */
static void
#if XVT_CC_PROTO
closestInRegn( struct DXFlayer *layer,
               struct DXFtriangle *table[],
               ftype x, ftype y, ftype z,
               struct DXFdistRes *distres )
#else
closestInRegn( layer, table, x, y, z, distres )
struct DXFlayer *layer;
struct DXFtriangle *table[];
ftype x, y, z;
struct DXFdistRes *distres;
#endif
{
        int pos;                                                                                                                                /* Position in list */
        struct DXFtriangle *triangle;                                   /* Current triangle */

        struct DXFdistRes tempres;                                                      /* Temporary result */
        int update;                                                                                                             /* update result from temp result */
        ftype tempangle;                                                                                                /* Angles used to choose for update */
        ftype resangle;

        if( table == NULL )                                                                             /* No triangles to examine */
                return;

        for( pos=0; table[pos]!=NULL; pos++ )
        {
                triangle = table[pos];

                if( triangle->tag != layer->tag )                               /* check not already examined */
                {
                        triangle->tag = layer->tag;
                        distTriangle( triangle, x, y, z, &tempres );

                        update = 0;
                        if( distres->triangle == NULL )
                                update = 1;
                        else if( fabs( (tempres.distance - distres->distance) /
                                (tempres.distance+1) ) < ZTOL )                                                  /* Same distance */
                        {
                                        tempangle = (ftype)fabs(PI/2 -
                                                fabs( atan2(tempres.vdist, tempres.hdist) ));
                                        resangle = (ftype)(PI/2 -
                                                fabs( atan2(distres->vdist, distres->hdist) ));
                                        if( fabs( ((tempangle - resangle) / (tempangle + 1)) ) < ZTOL )
                                                distres->difficult = 1;
                                        else if( tempangle < resangle )
                                                update = 1;                                                             /* Choose the closest to right angle */
                        }
                        else if( tempres.distance < distres->distance )
                                update = 1;

                        if( update )
                        {
                                *distres = tempres;
                        }
                }
        }
}


/*
 * =============================
 * NAME:
 *                      static ftype
 *                      distTriangle( struct DXFtriangle *triangle,
 *                                                                              ftype x, ftype y, ftype z, ftype *dot )
 *
 * DESCRIPTION:
 *                      Finds the distance from a point (x,y,z) to a specific triangle.
 *
 * PARAMETERS:
 *                      triangle                        Pointer to the triangle.
 *                      x,y,z                           The point.
 *                      dot                                     Used to return the distance above the triangle.
 *
 * PRE:
 *                      <triangle> must point to a valid triangle, not NULL.
 *
 * METHOD:
 *                      Reduces the problem to two dimensions using Pythagorus.
 *                      Determines if the point is inside the triangle, or which edges
 *                      are possible candidates for the closest point.
 * ===============================
 */
static void
#if XVT_CC_PROTO
distTriangle( struct DXFtriangle *triangle,
              ftype x, ftype y, ftype z, struct DXFdistRes *result )
#else
distTriangle( triangle, x, y, z, result )
struct DXFtriangle *triangle;
ftype x, y, z;
struct DXFdistRes *result;
#endif
{
        ftype corner[3][3];                                     /* Corners of the triangle (mutated) */
        ftype point[3];                                                         /* The actual point */

        ftype tdist;                                                                    /* temporary shortest distance */
        ftype tdist2;
        ftype lparam;                                                           /* parameter of line */

        ftype distance2;                                                        /* distance**2, also intermediate values */

        int i;

        assert( triangle != NULL );

#ifdef STATS
        statTriangles++;
#endif

        result->triangle = triangle;
        result->difficult = 0;

        memcpy( corner, triangle->points, sizeof(ftype)*3*3 );
        point[0] = x;                                                                                                    /* place point in entity coords */
        point[1] = y;
        point[2] = z;
        dxfChangeAxis( point, triangle->axis[0],
                                                                                                triangle->axis[1],
                                                                                                triangle->axis[2] );

        distance2 = point[2] - triangle->elevation;
        result->dot = distance2;
        result->vdist = (ftype)fabs( distance2 );

        if( dxfInsideTriangle( corner, 0, 1, point[0], point[1] ) )
        {
                result->distance = result->vdist;
                result->hdist = 0;
        }
        else
        {
                distance2 = distance2 * distance2;

                                                                                                                                        /* Check all corners */
                tdist = distPoint2D( point[0], point[1], corner[0][0], corner[0][1] );
                for( i=1; i<3; i++ )
                {
                        tdist2 = distPoint2D( point[0], point[1], corner[i][0], corner[i][1] );
                        if( tdist2 < tdist )
                        {
                                tdist = tdist2;
                        }
                }

                for( i=0; i<3; i++ )            /* check all edges */
                {
                        tdist2 = distLine2D( triangle->line[i],                         /* check the edge */
                                                                                                                 point[0], point[1], &lparam );
                        if( tdist2 < tdist )
                        {
                                if( (lparam > 0) && (lparam < 1) )                      /* check in correct section */
                                        tdist = tdist2;
                        }
                }

                result->distance = (ftype)sqrt( distance2 + tdist );
                result->hdist = (ftype)sqrt( tdist );
        }
}


/*
 * =======================
 * NAME:
 *                      static ftype
 *                      distLine2D( ftype line[2][2], ftype x, ftype y, ftype *lparam )
 *
 * DESCRIPTION:
 *                      Finds the square of the distance of a point from a line.
 *
 * PARAMETERS:
 *                      line                                    The equations of the line.
 *                      x, y, z                         The point in question.
 *                      lparam                          Used to return the parameter of the line.
 *
 * RETURNS:
 *                      The square of the distance to the line.
 *                      <lparam> indicates where on the line the closest point is.      If it is in
 *                      the range [0, 1], the point is within the original line segment.
 * =======================
 */
static ftype
#if XVT_CC_PROTO
distLine2D( ftype line[2][2], ftype x, ftype y, ftype *lparam )
#else
distLine2D( line, x, y, lparam )
ftype line[2][2], x, y, *lparam;
#endif
{
        ftype num;                              /* numerator */
        ftype den;                              /* denominator */

        num = line[1][1]*(x - line[0][0]) + line[0][1]*(line[1][0] - y);
        den = line[0][1]*line[0][1] + line[1][1]*line[1][1];

        *lparam = (line[0][1] * (x - line[0][0])
                                                         + line[1][1] * (y - line[1][0]) )
                                                / den;

        return (ftype)( num * num / den );
}


/*
 * =====================
 * NAME:
 *                      static ftype
 *                      distPoint2D( ftype x1, ftype y1, ftype x2, ftype y2 )
 *
 * DESCRIPTION:
 *                      Finds the square of the distance between two 2-dimensional points.
 *
 * PARAMETERS:
 *                      x1, y1                                  Coordinates of the first point.
 *                      x2, y2                                  Coordinates of the second point.
 *
 * RETURNS:
 *                      The square of the distance between the points.
 * =====================
 */
static ftype
#if XVT_CC_PROTO
distPoint2D( ftype x1, ftype y1, ftype x2, ftype y2 )
#else
distPoint2D( x1, y1, x2, y2 )
ftype x1, y1, x2, y2;
#endif
{
        ftype dx, dy;                   /* x & y offsets */

        dx = (ftype)(x2 - x1);
        dy = (ftype)(y2 - y1);
        return (ftype)(dx*dx + dy*dy);
}


/*
 * ==========================
 * NAME:
 *             void
 *             DXFvolRaster( struct DXFhandle *handle, int layerNum,
 *                           ftype *dist, int *inside,
 *                           int num[3], ftype min[3], ftype max[3] )
 *
 * DESCRIPTION:
 *             Fill two 3-dimensional arrays with
 *             - the distance of a point from the surface
 *             - whether the point is inside or outside the surface
 *             This routine does NOT allocate memory for the arrays.
 *
 * PARAMETERS:
 *             handle          Handle for the DXF file to use.
 *             layerNum        Layer to use (usually 0).
 *             dist            Array to store distance data in.
 *             inside          Array to store inside/outside flag in.
 *             num             The maximum array index in each direction.
 *             min             The lower bounds for each coordinate.
 *             max             The upper bounds for each coordinate.
 *
 * RETURNS:
 *             The arrays <dist> and <inside> are filled with data.
 * ===========================
 */
void
#if XVT_CC_PROTO
DXFvolRaster( struct DXFhandle *handle, int layerNum,
              ftype *dist, int *inside,
              int num[3], ftype min[3], ftype max[3] )
#else
DXFvolRaster( handle, layerNum, dist, inside, num, min, max )
struct DXFhandle *handle;
int layerNum;
ftype *dist;
int *inside, num[3];
ftype min[3], max[3];
#endif
{
   struct DXFlayer *layer = NULL;    /* Layer to use */
   ftype step[3];                    /* separation between raster points */
   ftype xyz[3];                     /* coordinates of current raster point */
   ftype distance;                   /* distance to closest triangle */
   int insideStat;                   /* True if inside the structure */
   int i, j, k;
   
   assert( !(handle->resultFlags & fFATALMASK) );
   assert( handle->conv == convVol );
   assert( handle->numLayers > layerNum );
   
   layer = handle->layer + layerNum;
   
   for( i = 0; i < 3; i++ )
          step[i] = (max[i] - min[i]) / (num[i] - 1);
   
   for( i = 0; i < num[0]; i++ )
   {
      xyz[0] = min[0] + i * step[0];
      for( j = 0; j < num[1]; j++ )
      {
         xyz[1] = min[1] + j * step[1];
         for( k = 0; k < num[2]; k++ )
         {
            xyz[2] = min[2] + k * step[2];
            dxfClosestTriangle( layer, xyz[0], xyz[1], xyz[2], &distance, &insideStat );
            if ( layer->flip )             /* is layer inside out? */
               insideStat = !insideStat;
                              
            setRaster( i,j,k, num, dist, inside, distance, insideStat );
         }
      }
   }
}


/*
 * ==========================
 * NAME:
 *                      static void
 *                      setRaster( int x, int y, int z, int num[3],
 *                                                               ftype *dist, int *inside,
 *                                                               ftype distance, int insideFlag )
 *
 * DESCRIPTION:
 *                      Stores data at a particular point in the arrays.
 *
 * PARAMETERS:
 *                      x, y, z                                 Index of the array location.
 *                      num                                             The maximum array index in each direction.
 *                      dist                                            The array to store distance information in.
 *                      inside                                  The array to store the inside/outside flag in.
 *                      distance                                The distance value to store.
 *                      insideFlag                      The inside/outside flag to store.
 *
 * RETURNS:
 *                      The <dist> and <inside> arrays are modified.
 * ==========================
 */
static void
#if XVT_CC_PROTO
setRaster( int x, int y, int z, int num[3],
                  ftype *dist, int *inside,
                  ftype distance, int insideFlag )
#else
setRaster( x, y, z, num, dist, inside, distance, insideFlag )
int x, y, z, num[3];
ftype *dist;
int *inside;
ftype distance;
int insideFlag;
#endif
{
        long index;                                             /* index into the raster */

        index = (x * num[1] + y) * num[2] + z;

        dist[index] = distance;
        inside[index] = insideFlag;
}



/*
 * =======================
 * NAME:
 *                      clearTags
 *
 * DESCRIPTION:
 *                      Reset all tags in a layer to avoid aliasing of tags.
 *
 * PARAMETERS:
 *                      layer           A pointer to the layer to operate on.
 *
 * RETURNS:
 *                      All tags for regions and triangles within the layer are set to 0.
 *                      The stored tag for the layer is set to 1.
 * =======================
 */
void
#if XVT_CC_PROTO
dxfClearTags( struct DXFlayer *layer )
#else
dxfClearTags( layer )
struct DXFlayer *layer;
#endif
{
        struct DXFtriangle *triangle;           /* Current triangle to clear */
        long numRegns;                                                                          /* Number of regions */
        long i;

        for( triangle=layer->triangles; triangle!=NULL; triangle=triangle->next )
                triangle->tag = 0;

        assert( layer->regnTag != NULL );
        numRegns = layer->numRegns[0] * layer->numRegns[1] * layer->numRegns[2];
        for( i=0; i<numRegns; i++ )
                layer->regnTag[i] = 0;

        layer->tag = 1;                         /* NOTE: not zero */
}


/*
 * =========================
 * NAME:
 *                      DXFfillGaps
 *
 * DESCRIPTION:
 *                      Fill gaps in an inside/outside array (those elements which are
 *                      iUNKNOWN) by examining adjacent elements.
 * ========================
 */
void
#if XVT_CC_PROTO
DXFfillGaps( int *inout, int xsize, int ysize, int zsize )
#else
DXFfillGaps( inout, xsize, ysize, zsize )
int *inout, xsize, ysize, zsize;
#endif
{
        size_t numelems;                                        /* number of elements in "inout" */
        size_t radar[6];                                        /* For finding adjacent elements */
        size_t curloc;                                          /* Current location */
        size_t peek;                                                    /* Peek location */
        int repeat;                                                     /* Repeat until no more unknowns are left */
        int numIn, numOut;                              /* Counters for determining which to choose */
        int i;

        numelems = xsize * ysize * zsize;
        radar[0] = 1;
        radar[1] = -1;
        radar[2] = -ysize;
        radar[3] = ysize;
        radar[4] = xsize * ysize;
        radar[5] = -radar[4];

        do
        {
                repeat = 0;
                for( curloc=0; curloc<numelems; curloc++ )
                {
                        if( inout[curloc] == iUNKNOWN )
                        {
                                numIn = 0;
                                numOut = 0;
                                for( i=0; i<6; i++ )
                                {
                                        peek = curloc + radar[i];
                                        if( inout[peek] == iINSIDE )
                                                numIn++;
                                        else if( inout[peek] == iOUTSIDE )
                                                numOut++;
                                } /* end for i */
                                if( numIn > numOut )
                                        inout[curloc] = iSETIN;
                                else if( numOut > numIn )
                                        inout[curloc] = iSETOUT;
                                else if( (numIn != 0) && (numIn == numOut) )
                                        inout[curloc] = iSETOUT;
                        } /* end if unknown */
                } /* end for curloc */

                for( curloc=0; curloc<numelems; curloc++ )
                {
                        switch( inout[curloc] )
                        {
                                case iSETIN:
                                        inout[curloc] = iINSIDE;
                                        break;

                                case iSETOUT:
                                        inout[curloc] = iOUTSIDE;
                                        break;

                                case iUNKNOWN:
                                        repeat = 1;
                                        break;
                        }
                } /* end for curloc */
        } while( repeat );
}

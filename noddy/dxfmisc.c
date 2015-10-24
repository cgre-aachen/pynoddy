/*
 * ================================
 * NAME:
 *            dxfmisc.c
 *
 * DESCRIPTION:
 *            Routines common to both vol and surf modules, which do not fit
 *            comfortably into dxfread.c
 *
 * AUTHOR:
 *            Ewen Christopher
 *            (c) Ewen Christopher 1995
 *            (c) Monash University 1995
 * ================================
 */

#include "xvt.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <math.h>
#include "dxf.h"

/******************* Prototypes *******************/

#if XVT_CC_PROTO
static void minMaxLayer( struct DXFlayer *layer );
static void freeRegions( struct DXFlayer *layer );
static void separateX( struct DXFtriangle *src[2], struct DXFtriangle *dest[2],
            ftype min, ftype max );
static int divideVol( struct DXFlayer *layer, struct DXFtriangle *triList,
            int index[3], int numRegn[3], ftype rMin[3], ftype rMax[3],
            ftype rMid[3], ftype step[3] );
static int divideSurf( struct DXFlayer *layer, struct DXFtriangle *triList,
            int index[3], int numRegn[3], ftype rMin[3], ftype rMax[3],
            ftype rMid[3] );
static int crossRegion( ftype corner[3][3], int xx, int yy,
            ftype min[3], ftype max[3] );
static int oneRegion( struct DXFlayer *layer );
#else
static void minMaxLayer( );
static void freeRegions( );
static void separateX( );
static int divideVol( );
static int divideSurf( );
static int crossRegion( );
static int oneRegion( );
#endif


/*
 * =========================
 * NAME:
 *            dxfMinMax
 *
 * DESCRIPTION:
 *            Find min/max values of each coordinate for an entire DXF file.
 *            If there are no layers, nothing will be done.
 *
 * PARAMETERS:
 *            handle           Handle of the file to process.
 *
 * RETURNS:
 *            The DXFhandle.min and DXFhandle.max fields are set.
 * ========================
 */
void
#if XVT_CC_PROTO
dxfMinMax( struct DXFhandle *handle )
#else
dxfMinMax( handle )
struct DXFhandle *handle;
#endif
{
   int layerNum;                              /* Number of current layer */
   struct DXFlayer *layer = NULL;     /* Pointer to current layer */
   int i;

   assert( !(handle->resultFlags & fFATALMASK) );

   for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
   {
      layer = handle->layer + layerNum;

      minMaxLayer( layer );

      if( layerNum == 0 )                          /* Coallate min/max values */
      {
         memcpy( handle->min, layer->min, sizeof(ftype)*3 );
         memcpy( handle->max, layer->max, sizeof(ftype)*3 );
      }
      else
      {
         for( i=0; i<3; i++ )
         {
            if( layer->min[i] < handle->min[i] )
               handle->min[i] = layer->min[i];
            if( layer->max[i] > handle->max[i] )
               handle->max[i] = layer->max[i];
         }
      }
   }   /* end for each layer */
}


/*
 * ===========================
 * NAME:
 *            static void
 *            minMaxLayer( struct DXFlayer *layer )
 *
 * DESCRIPTION:
 *            For each triangle in the layer, it finds the minimum and maximum values
 *            for each coordinate.    These are then coallated for the layer.
 *            The number of triangles is also counted, and stored with the layer.
 *
 * PARAMETERS:
 *            layer      The layer to take the list of triangles from, and store
 *                           the resulting min/max values in.
 *
 * PRE:
 *            <layer> may not be NULL.
 *            There must be at least one triangle in the layer.
 *
 * CALLED BY:
 *            dxfMinMax
 * ===========================
 */
static void
#if XVT_CC_PROTO
minMaxLayer( struct DXFlayer *layer )
#else
minMaxLayer( layer )
struct DXFlayer *layer;
#endif
{
   struct DXFtriangle *triangle = NULL;
   int i, j;

   assert( layer != NULL );
   assert( layer->triangles != NULL );

   triangle = layer->triangles;

   layer->numTriangles = 0;                                          /* prime layer stores */
   memcpy( layer->min, triangle->corner[0], sizeof(ftype)*3 );
   memcpy( layer->max, triangle->corner[0], sizeof(ftype)*3 );

   for( ; triangle!=NULL; triangle=triangle->next )
   {
      layer->numTriangles++;

                                                                              /* prime triangle stores */
      memcpy( triangle->min, triangle->corner[0], sizeof(ftype)*3 );
      memcpy( triangle->max, triangle->corner[0], sizeof(ftype)*3 );
      for( i=0; i<3; i++ )                         /* for each axis */
      {
         for( j=1; j<3; j++ )                      /* for each remaining corner */
         {
            if( triangle->corner[j][i] < triangle->min[i] )
               triangle->min[i] = triangle->corner[j][i];
            if( triangle->corner[j][i] > triangle->max[i] )
               triangle->max[i] = triangle->corner[j][i];
         }
         if( triangle->min[i] < layer->min[i] )           /* Coallate for layer */
            layer->min[i] = triangle->min[i];
         if( triangle->max[i] > layer->max[i] )
            layer->max[i] = triangle->max[i];
      }
   }
}

/*
 * ========================
 * NAME:
 *            static void
 *            freeRegions( struct DXFlayer *layer )
 *
 * DESCRIPTION:
 *            Frees any memory allocated to the regions of a layer.  Does not affect
 *            the triangles or other data stored in the layer.
 *
 * PARAMETERS:
 *            layer         The layer to act on.
 *
 * RETURNS:
 *            <layer> is modified.
 *
 * CALLED BY:
 *            DXFfree
 *            DXFvolConv
 *            DXfsurfConv
 * =======================
 */
static void
#if XVT_CC_PROTO
freeRegions( struct DXFlayer *layer )
#else
freeRegions( layer )
struct DXFlayer *layer;
#endif
{
   int numreg;                          /* Number of regions */
   int i;

   if( layer->regions != NULL )
   {
      numreg = layer->numRegns[0] * layer->numRegns[1] * layer->numRegns[2];
      for( i = 0; i < numreg; i++ )
      {
         if( layer->regions[i] != NULL )
         {
            xvt_mem_free( (char *) layer->regions[i] );
         }
      }
      xvt_mem_free( (char *) layer->regions );
      layer->regions = NULL;

#ifdef DXF_VOL
      if( layer->regnTag != NULL )
      {
         xvt_mem_free( (char *) layer->regnTag );
         layer->regnTag = NULL;
      }
#endif
   }
}


/*
 * ========================
 * NAME:
 *            void
 *            DXFfree( struct DXFhandle *handle )
 *
 * DESCRIPTION:
 *            Frees memory held by a loaded file.
 *            Sets any hanging pointers to NULL.
 *
 * PARAMETERS:
 *            handle           The handle for the file to free.
 *
 * RETURNS:
 *            <handle> is modified.
 * ========================
 */
void
#if XVT_CC_PROTO
DXFfree( struct DXFhandle *handle )
#else
DXFfree( handle )
struct DXFhandle *handle;
#endif
{
   struct DXFtriangle *triangle;               /* for freeing triangles */
   struct DXFtriangle *tempTri;

   struct DXFlayer *layer;                         /* current layer */
   int lnum;                                                 /* current layer number */

   for( lnum=0; lnum < handle->numLayers; lnum++ )
   {
      layer = handle->layer + lnum;

      triangle = layer->triangles;                          /* free triangles */
      while( triangle != NULL )
      {
         tempTri = triangle->next;
         xvt_mem_free( (char *) triangle );
         triangle = tempTri;
      }
      layer->triangles = NULL;

#ifdef DXF_LINE
      dxfFreePoints( layer->points );
      dxfFreeLines( layer->lines );
#endif

      freeRegions( layer );
   }

   if (handle->layer)
      xvt_mem_free( (char *) handle->layer );
   handle->layer = NULL;
   memset (handle, 0, sizeof (DXFHANDLE));
}


/*
 * ===================================
 * NAME:
 *            int
 *            dxfDivideRegions( struct DXFlayer *layer )
 *
 * DESCRIPTION:
 *            Divides the x,y plane up into rectangular regions to enable more rapid
 *            finding of the triangles near any one point.
 *
 * PARAMETERS:
 *            layer      The layer to operate on.
 *
 * RETURNS:
 *            0 if no error occurs.
 *            Otherwise returns flags to be ORed with resultFlags.
 *
 * CALLED BY:
 *            DXFvolConv
 *            DXFsurfConv
 * ===================================
 */
int
#if XVT_CC_PROTO
dxfDivideRegions( struct DXFlayer *layer, enum conversionEnum conv )
#else
dxfDivideRegions( layer, conv )
struct DXFlayer *layer;
enum conversionEnum conv;
#endif
{
   int alloc;                                             /* total number of regions */
   int numRegn[3];                                  /* Num. of regions in each direction */
   ftype step[3];                                   /* width of regions */

   ftype rMin[3];                                   /* borders of this region */
   ftype rMax[3];
   ftype rMid[3];                                   /* Midpoint of region */

   int index[3];                                    /* current region index */
   struct DXFtriangle *triangle;            /* current triangle */
   struct DXFtriangle *tempList[2];  /* Source lists */
   struct DXFtriangle *xList[2];    /* Separated lists */
   int i;

   tempList[0] = tempList[1] = NULL;
   xList[0] = xList[1] = NULL;
                                                /* check conv is consistant with defines */
#ifdef DXF_SURF
#ifdef DXF_VOL     /* DXF_SURF && DXF_VOL */
   assert( (conv == convSurf) || (conv == convVol) );
#else /* just DXF_SURF */
   assert( conv == convSurf );
#endif
#else
#ifdef DXF_VOL  /* just DXF_VOL */
   assert( conv == convVol );
#endif
#endif

   freeRegions( layer );

   numRegn[0] = numRegn[1] = (int)sqrt( layer->numTriangles / TRI_PER_REG );
   if( conv == convSurf )
      numRegn[2] = 1;
   else
      numRegn[2] = numRegn[0];

   if( numRegn[0] < 2 )                 /* don't do anything if all in one strip */
   {
      return oneRegion( layer );
   }

   for( i=0; i<3; i++ )
      step[i] = (layer->max[i] - layer->min[i]) / numRegn[i];

   memcpy( layer->numRegns, numRegn, sizeof(int)*3 );
   memcpy( layer->step, step, sizeof(ftype)*3 );

   alloc = numRegn[0] * numRegn[1] * numRegn[2];
   layer->regions = (struct DXFtriangle ***)xvt_mem_zalloc(
                                                         sizeof(struct DXFtriangle **) * alloc );
   if( layer->regions == NULL )
      return fMEM;

   for( i=0; i<alloc; i++ )                     /* initialise memory */
      layer->regions[i] = NULL;

#ifdef DXF_VOL
   if( conv == convVol )
   {
      layer->regnTag = (char *)xvt_mem_zalloc( sizeof(char) * alloc );
      if( layer->regnTag == NULL )
         return fMEM;

      for( i=0; i<alloc; i++ )
         layer->regnTag[i] = 0;
      layer->tag = 0;
   }
#endif

   tempList[0] = layer->triangles;
   for( index[0]=0; index[0]<numRegn[0]; index[0]++ )             /* for each column */
   {
      rMin[0] = layer->min[0] + index[0] * step[0];    /* find edges of region */
      rMax[0] = layer->min[0] + (index[0]+1) * step[0];
      rMid[0] = (rMin[0] + rMax[0]) / 2;

      separateX( tempList, xList, rMin[0], rMax[0] );

      for( index[1]=0; index[1]<numRegn[1]; index[1]++ )                /* for each row */
      {
         rMin[1] = layer->min[1] + index[1] * step[1];   /* find edges of region */
         rMax[1] = layer->min[1] + (index[1]+1) * step[1];
         rMid[1] = (rMin[1] + rMax[1]) / 2;

#ifdef DXF_SURF
#ifdef DXF_VOL     /* DXF_SURF && DXF_VOL */
         if( conv == convSurf )
            divideSurf( layer, xList[0], index, numRegn, rMin, rMax, rMid );
         else
            divideVol( layer, xList[0], index, numRegn, rMin, rMax, rMid, step );
#else /* just DXF_SURF */
         divideSurf( layer, xList[0], index, numRegn, rMin, rMax, rMid );
#endif
#else
#ifdef DXF_VOL  /* just DXF_VOL */
         divideVol( layer, xList[0], index, numRegn, rMin, rMax, rMid, step );
#endif
#endif
      }          /* end y loop */

      tempList[0] = xList[0];                /* Restore tempList for next x strip */
      tempList[1] = xList[1];
      xList[0] = xList[1] = NULL;
   }                               /* end x loop */

   if( tempList[0] == NULL )                      /* restore list of triangles */
   {
      layer->triangles = tempList[1];
   }
   else
   {
      if( tempList[1] != NULL )
      {
         triangle = tempList[0];                /* Append list 1 to list 0 */
         while( triangle->next != NULL )
            triangle = triangle->next;
         triangle->next = tempList[1];
      }
      layer->triangles = tempList[0];
   }

   return 0;                                                    /* successful completion */
}


/*
 * ======================
 * NAME:
 *            static void
 *            separateX( struct DXFtriangle *src[2], struct DXFtriangle *dest[2],
 *                            ftype min, ftype max )
 *
 * DESCRIPTION:
 *            Finds the triangles which are in the strip min..max (x values only).
 *            The lists src[0] and src[1] are used as the source lists.
 *            dest[0] is used to hold the triangles in the strip, and dest[1] those
 *            outside the strip.
 *
 * PARAMETERS:
 *            src           Source lists of triangles.
 *            dest          Destination lists of triangles (these are NOT cleared by
 *                           separateX).
 *            min           Minimum x value for the strip.
 *            max           Maximum x value for the strip.
 *
 * RETURNS:
 *            <src> and <dest> are modified.
 *
 * CALLED BY:
 *            dxfDivideRegions
 * =====================
 */
static void
#if XVT_CC_PROTO
separateX( struct DXFtriangle *src[2], struct DXFtriangle *dest[2],
                ftype min, ftype max )
#else
separateX( src, dest, min, max )
struct DXFtriangle *src[2];
struct DXFtriangle *dest[2];
ftype min, max;
#endif
{
   struct DXFtriangle *triangle;
   int i;

   for( i=0; i<2; i++ )
   {
      while( src[i] != NULL )          /* Find triangles in this x strip */
      {
         triangle = src[i];
         src[i] = triangle->next;
         if(     (triangle->max[0] > min)
             && (triangle->min[0] < max) )
         {
            triangle->next = dest[0];
            dest[0] = triangle;
         }
         else
         {
            triangle->next = dest[1];
            dest[1] = triangle;
         }
      }
   }
}

/*
 * ====================
 * NAME:
 *            static int
 *            storeInRegn( struct DXFlayer *layer, int regnNum,
 *                               struct DXFtriangle *inRegion[], int numInRegn )
 *
 * DESCRIPTION:
 *            Stores an array of triangles in a region.
 *
 * PARAMETERS:
 *            layer         Layer to store triangles in.
 *            regnNum       Number of the region to store triangles in.
 *            inRegion      Array of triangles in the region (the data to store).
 *            numInRegn       Number of entries in <inRegion>
 *
 * RETURNS:
 *            Error flags
 *
 * CALLED BY:
 *            divideVol
 * ====================
 */
static int
#if XVT_CC_PROTO
storeInRegn( struct DXFlayer *layer, int regnNum,
                   struct DXFtriangle *inRegion[], int numInRegn )
#else
storeInRegn( layer, regnNum, inRegion, numInRegn )
struct DXFlayer *layer;
int regnNum;
struct DXFtriangle *inRegion[];
int numInRegn;
#endif
{
   if( numInRegn > 0 )
   {
      inRegion[ numInRegn ] = NULL;                                    /* terminate list */
      numInRegn++;

      layer->regions[regnNum] = (struct DXFtriangle **)xvt_mem_zalloc(
                  sizeof(struct DXFtriangle *) * numInRegn );
      if( layer->regions[regnNum] == NULL )                         /* out of memory */
         return fMEM;

      memcpy( layer->regions[regnNum], inRegion,
                  sizeof(struct DXFtriangle *) * numInRegn );
   }
   return 0;
}


#ifdef DXF_VOL
/*
 * =======================
 * NAME:
 *            static int
 *            divideVol( struct DXFlayer *layer, struct DXFtriangle *triList,
 *            int index[3], int numRegn[3], ftype rMin[3], ftype rMax[3],
 *            ftype rMid[3], ftype step[3] )
 *
 * DESCRIPTION:
 *            Performs the final division into regions for the vol module.
 *
 * PARAMETERS:
 *            layer         The layer being operated on.
 *            triList       The list of triangles to investigate.
 *            index         The current x, y region index.
 *            numRegn       Number of regions in each direction.
 *            rMin             Lower bound of the region.
 *            rMax             Upper bound of the region.
 *            rMid             Midpoint of the region.
 *            step             Spacing between regions.
 *
 * RETURNS:
 *            Result flags.
 *            <index>, <rMin>, <rMax>, <rMid> may be altered.
 *
 * CALLED BY:
 *            dxfDivideRegions
 * =======================
 */
static int
#if XVT_CC_PROTO
divideVol( struct DXFlayer *layer, struct DXFtriangle *triList, int index[3],
                int numRegn[3], ftype rMin[3], ftype rMax[3], ftype rMid[3],
                ftype step[3] )
#else
divideVol( layer, triList, index, numRegn, rMin, rMax, rMid, step )
struct DXFlayer *layer;
struct DXFtriangle *triList;
int index[3];
int numRegn[3];
ftype rMin[3];
ftype rMax[3];
ftype rMid[3];
ftype step[3];
#endif
{
   struct DXFtriangle *inRegion[100];                     /* Triangles in region */
   int numInRegion;                                             /* Entries in inRegion */
   int regn;                                                          /* Current region number */
   struct DXFtriangle *triangle = NULL;                /* Current triangle */
   int retval = 0;                                              /* Flags to return */

                                                 /* for each triangle in this x strip */
   for( triangle=triList; triangle!=NULL; triangle=triangle->next )
   {
      if(     (triangle->max[1] > rMin[1])
          && (triangle->min[1] < rMax[1]) )
      {
          triangle->tag = -1;             /* Set a flag if in the y rectangle */
      }
      else
         triangle->tag = 0;
   }

   for( index[2]=0; index[2]<numRegn[2]; index[2]++ )                /* for each row */
   {
      regn = (index[0] * numRegn[1] + index[1]) * numRegn[2] + index[2];
      numInRegion = 0;                                       /* reset list of triangles */

      rMin[2] = layer->min[2] + index[2] * step[2];   /* find edges of region */
      rMax[2] = layer->min[2] + (index[2]+1) * step[2];
      rMid[2] = (rMin[2] + rMax[2]) / 2;

                                                          /* for each triangle in this x strip */
      for( triangle=triList; triangle!=NULL; triangle=triangle->next )
      {
         if(     crossRegion( triangle->corner, 0, 2, rMin, rMax )
             || dxfInsideTriangle( triangle->corner, 0, 2, rMid[0], rMid[2] ) )
         {
            inRegion[numInRegion] = triangle;
            numInRegion++;
            assert( numInRegion < 100 );
         }
      }

      retval |= storeInRegn( layer, regn, inRegion, numInRegion );
   }          /* end x loop */

   return retval;
}
#endif        /* defined DXF_VOL */

#ifdef DXF_SURF
/*
 * =======================
 * NAME:
 *            static int
 *            divideSurf( struct DXFlayer *layer, struct DXFtriangle *triList,
 *            int index[3], int numRegn[3], ftype rMin[3], ftype rMax[3],
 *            ftype rMid[3] )
 *
 * DESCRIPTION:
 *            Performs the final division into regions for the surf module.
 *
 * PARAMETERS:
 *            layer         The layer being operated on.
 *            triList       The list of triangles to investigate.
 *            index         The current x, y region index.
 *            numRegn       Number of regions in each direction.
 *            rMin             Lower bound of the region.
 *            rMax             Upper bound of the region.
 *            rMid             Midpoint of the region.
 *
 * RETURNS:
 *            Result flags.
 *            <index>, <rMin>, <rMax>, <rMid> may be altered.
 *
 * CALLED BY:
 *            dxfDivideRegions
 * =======================
 */
static int
#if XVT_CC_PROTO
divideSurf( struct DXFlayer *layer, struct DXFtriangle *triList, int index[3],
                  int numRegn[3], ftype rMin[3], ftype rMax[3], ftype rMid[3] )
#else
divideSurf( layer, triList, index, numRegn, rMin, rMax, rMid )
struct DXFlayer *layer;
struct DXFtriangle *triList;
int index[3];
int numRegn[3];
ftype rMin[3];
ftype rMax[3];
ftype rMid[3];
#endif
{
   struct DXFtriangle *inRegion[100];                     /* Triangles in region */
   int numInRegion = 0;                                        /* Entries in inRegion */
   int regn;                                                          /* Current region number */
   struct DXFtriangle *triangle = NULL;                /* Current triangle */

                                                 /* for each triangle in this x strip */
   for( triangle=triList; triangle!=NULL; triangle=triangle->next )
   {
      if(     crossRegion( triangle->corner, 0, 1, rMin, rMax )
          || dxfInsideTriangle( triangle->corner, 0, 1, rMid[0], rMid[1] ) )
      {
         inRegion[numInRegion] = triangle;
         numInRegion++;
         assert( numInRegion < 100 );
      }
   }

   regn = index[0] * numRegn[1] + index[1];
   return storeInRegn( layer, regn, inRegion, numInRegion );
}
#endif  /* defined( VOL_SURF ) */


/*
 * =======================================
 * NAME:
 *            static int
 *            crossRegion( ftype corner[3][3], int xx, int yy,
 *                               ftype min[3], ftype max[3] )
 *
 * DESCRIPTION:
 *            Finds the bitwise location codes for each corner of a triangle.
 *            For each point, sets following bits:
 *               bit 0 if left of region
 *               bit 1 if right of region
 *               bit 2 if below region
 *               bit 3 if above region
 *            If the bitwise AND of the codes for two corners is non-zero, then the
 *            line joining those corners does not cross the region.
 *
 * PARAMETERS:
 *            corner        the corners of the triangle
 *            min              the minimum x,y values of the region
 *            max              the maximum x,y values of the region
 *
 * RETURNS:
 *            Zero if none of the edges of the triangle cross the region, non-zero
 *            otherwise.
 *
 * SOURCE:
 *            Micrographics Techniques
 * ========================================
 */
static int
#if XVT_CC_PROTO
crossRegion(  ftype corner[3][3], int xx, int yy,
                   ftype min[3], ftype max[3] )
#else
crossRegion(  corner, xx, yy, min, max)
ftype corner[3][3];
int xx, yy;
ftype min[3], max[3];
#endif
{
   int endCodes[3];               /* binary codes of corners */
   int i;

   endCodes[0] = endCodes[1] = endCodes[2] = 0;
   
   for( i=0; i<3; i++ )
   {
      if( corner[i][xx] < min[xx] )
         endCodes[i] |= 1;
      if( corner[i][xx] > max[xx] )
         endCodes[i] |= 2;
      if( corner[i][yy] < min[yy] )
         endCodes[i] |= 4;
      if( corner[i][yy] > max[yy] )
         endCodes[i] |= 8;
   }

   if(      !(endCodes[0] & endCodes[1])
         || !(endCodes[1] & endCodes[2])
         || !(endCodes[2] & endCodes[0]) )
   {
      return 1;
   }
   else
      return 0;
}



/*
 * ======================
 * NAME:
 *            int
 *            dxfInsideTriangle( ftype corner[3][3], int xx, int yy,
 *            ftype x, ftype y )
 *
 * DESCRIPTION:
 *            Determines if a point is inside a triangle.
 *
 * PARAMETERS:
 *            corner        The corners of the triangle.
 *            xx                  The index to use as the x coordinate (usually 0).
 *            yy                  The index to use as the y coordinate (usually 1).
 *            x, y             The point to consider.
 *
 * RETURNS:
 *            Non-zero if the point (x,y) is inside the triangle, 0 otherwise.
 *
 * METHOD:
 *            If (point, c[i], c[i+1]) is clockwise for all three combinations
 *            of c, then the point is inside the triangle (likewise for
 *            anticlockwise).
 * ======================
 */
int
#if XVT_CC_PROTO
dxfInsideTriangle( ftype corner[3][3], int xx, int yy, ftype x, ftype y )
#else
dxfInsideTriangle( corner, xx, yy, x, y )
ftype corner[3][3];
int xx, yy;
ftype x, y;
#endif
{
   int i, j;
   ftype a, b, c;     /* Parameters of each line */
   ftype result[3];   /* Result of each test */
   ftype avgRes = (ftype) 0;  /* Average result */
   int above = 0;     /* Number of results indicating above */
   int below = 0;     /*       or below the line */

   for( i=0; i<3; i++ )     /* determine if inside triangle */
   {
      j = (i+1) % 3;
      a = corner[i][yy] - corner[j][yy];
      b = corner[j][xx] - corner[i][xx];
      c = -(a*corner[i][xx] + b*corner[i][yy]);
      result[i] = a*x + b*y + c;
      avgRes += (ftype) fabs( result[i] );
   }

   if( avgRes < ZTOL )
      avgRes = (ftype) 1;

   for( i=0; i<3; i++ )
   {
      if( fabs(result[i])/avgRes > ZTOL )
      {
         if( result[i] > 0 )
            above++;
         else if( result[i] < 0 )
            below++;
      }
   }

   if( (above == 0) || (below == 0) )    /* inside the triangle */
      return 1;
   else
      return 0;
}



/*
 * =====================================
 * NAME:
 *            void
 *            dxfApplyMatrix( struct DXFtriangle *triangle, ftype matrix[4][4] )
 *
 * DESCRIPTION:
 *            Applies a transformation matrix to all the points in a list of
 *            triangles.
 *
 * PRE:
 *            Matrix must not be NULL.
 *
 * CALLED BY:
 *            DXFsurfConv
 *            DXFvolConv
 * =====================================
 */
void
#if XVT_CC_PROTO
dxfApplyMatrix( struct DXFtriangle *triangle, ftype matrix[4][4] )
#else
dxfApplyMatrix( triangle, matrix )
struct DXFtriangle *triangle;
ftype matrix[4][4];
#endif
{
   int c;                    /* corner number */
   ftype orig[4];        /* original vector of coordinates */
   ftype final[4];       /* final vector of coordinates */
   int i, j;

   assert( matrix != NULL );

   while( triangle != NULL )
   {
      for( c=0; c<3; c++ )
      {
         memcpy( orig, triangle->corner[c], sizeof(ftype) * 3 );
         orig[3] = (ftype) 1;

         for( i=0; i<4; i++ )                   /* multiply matrix * orig = final */
         {
            final[i] = (ftype) 0;
            for( j=0; j<4; j++ )
            {
               final[i] += (matrix[i][j] * orig[j]);
            }
         }

                            /* some transforms alter the final element in the vector */
         triangle->corner[c][0] = final[0] / final[3];
         triangle->corner[c][1] = final[1] / final[3];
         triangle->corner[c][2] = final[2] / final[3];
      }
      triangle = triangle->next;
   }
}


/*
 * =========================
 * NAME:
 *            void
 *            dxfChangeOrigin( ftype point[3], ftype centroid[3] )
 *
 * DESCRIPTION:
 *            Moves the origin of a point.
 *
 * PARAMETERS:
 *            point         The point to move.
 *            centroid      The new origin (often the centroid of the data).
 *
 * RETURNS:
 *            <point> is modified.
 * =========================
 */
void
#if XVT_CC_PROTO
dxfChangeOrigin( ftype point[3], ftype centroid[3] )
#else
dxfChangeOrigin( point, centroid )
ftype point[3], centroid[3];
#endif
{
   point[0] -= centroid[0];
   point[1] -= centroid[1];
   point[2] -= centroid[2];
}

/*
 * ======================================
 * NAME:
 *            void
 *            dxfChangeAxis( ftype point[3],
 *                                  ftype x[3], ftype y[3], ftype z[3] )
 *
 * DESCRIPTION:
 *            Changes a point or vector from one set of cartesian axis to another.
 *
 * PARAMETERS:
 *            point      The original point/vector, also used to return the final point
 *            x,y,z      The axis vectors.
 *
 * PRE:
 *            The axis vectors must all be unit vectors.
 *
 * METHOD:
 *            This is an application of the dot product.      The projection of vector
 *            a in the direction of b is given by  a.b/|b|    It is assumed that the
 *            vector b (the axis vector) is of unit length, hence |b| = 1
 * =======================================
 */
void
#if XVT_CC_PROTO
dxfChangeAxis( ftype point[3],
                  ftype x[3], ftype y[3], ftype z[3] )
#else
dxfChangeAxis( point, x, y, z )
ftype point[3], x[3], y[3], z[3];
#endif
{
   ftype newPoint[3];

   newPoint[0] = point[0]*x[0] + point[1]*x[1] + point[2]*x[2];
   newPoint[1] = point[0]*y[0] + point[1]*y[1] + point[2]*y[2];
   newPoint[2] = point[0]*z[0] + point[1]*z[1] + point[2]*z[2];

   memcpy( point, newPoint, sizeof(ftype)*3 );
}


/*
 * ==========================
 * NAME:
 *            int
 *            dxfCmpCorner( ftype a[3], ftype b[3] )
 *
 * DESCRIPTION:
 *            Compares two corners.  Note that the return style is different from
 *            that of strcmp.  Any values within the ZTOL criterea are deemed to
 *            be the same.
 *
 * PARAMETERS:
 *            a, b          The two corners to compare.
 *
 * RETURNS:
 *            Non-zero if the corners are the same, zero otherwise.
 * =========================
 */
int
#if XVT_CC_PROTO
dxfCmpCorner( ftype a[3], ftype b[3] )
#else
dxfCmpCorner( a, b )
ftype a[3], b[3];
#endif
{
   if(     ( fabs( a[0] - b[0] ) < ZTOL )
       && ( fabs( a[1] - b[1] ) < ZTOL )
       && ( fabs( a[2] - b[2] ) < ZTOL ) )
   {
      return 1;
   }
   else
      return 0;
}



#ifdef DXF_LINE
/*
 * ========================
 * NAME:
 *            void
 *            dxfFreePoints( struct DXFpoint *points )
 *
 * DESCRIPTION:
 *            Frees a circular list of points.
 *
 * PARAMETERS:
 *            points           Pointer to the last element in the list.
 *
 * RETURNS:
 *            Nothing.
 * =========================
 */
void
#if XVT_CC_PROTO
dxfFreePoints( struct DXFpoint *points )
#else
dxfFreePoints( points )
struct DXFpoint *points;
#endif
{
   struct DXFpoint *temp;

   if( points != NULL )
   {
      temp = points;                          /* Straighten the list */
      points = points->next;
      temp->next = NULL;

      while( points != NULL )
      {
         temp = points;
         points = points->next;
         xvt_mem_free( (char *) temp );
      }
   }
}


/*
 * =====================
 * NAME:
 *            void
 *            dxfFreeLines( struct DXFline *line )
 *
 * DESCRIPTION:
 *            Free the memory held by a list of lines.
 *
 * PARAMETERS:
 *            line             List of lines to free.
 *
 * RETURNS:
 *            Nothing.
 * =====================
 */
void
#if XVT_CC_PROTO
dxfFreeLines( struct DXFline *line )
#else
dxfFreeLines( line )
struct DXFline *line;
#endif
{
   struct DXFline *temp;

   while( line != NULL )
   {
      temp = line;
      line = line->next;
      xvt_mem_free( (char *) temp );
   }
}
#endif



/*
 * ========================
 * NAME:
 *            static int
 *            oneRegion( struct DXFlayer *layer )
 *
 * DESCRIPTION:
 *            Store all the triangles in a single region.  This eliminates the
 *            special case where there may be no regions.
 *
 * PARAMETERS:
 *            layer      The layer to operate on.
 *
 * RETURNS:
 *            Error and warning flags to be ORed with the current flags.      0 indicates
 *            success.
 * ===========================
 */
static int
#if XVT_CC_PROTO
oneRegion( struct DXFlayer *layer )
#else
oneRegion( layer )
struct DXFlayer *layer;
#endif
{
   size_t numTri;                                   /* Number of triangles in layer */
   struct DXFtriangle *triangle = NULL;    /* Current triangle */
   struct DXFtriangle **table = NULL;         /* Pointer to the table of triangles */
   int i;

   numTri = 0;                                         /* Count triangles */
   for( triangle=layer->triangles; triangle!=NULL; triangle=triangle->next )
      numTri++;

                                                            /* Allocate array of regions */
   layer->numRegns[0] = layer->numRegns[1] = layer->numRegns[2] = 1;
   layer->regions = (struct DXFtriangle ***)xvt_mem_zalloc(
      sizeof(struct DXFtriangle **) );
   if( layer->regions == NULL )
      return fMEM;

                                                            /* Allocate table for this region */
   table = (struct DXFtriangle **)xvt_mem_zalloc(
      sizeof(struct DXFtriangle *) * (numTri + 1) );
   if( table == NULL )
   {
      xvt_mem_free( (char *) layer->regions );
      layer->regions = NULL;
      return fMEM;
   }
   layer->regions[0] = table;

                                                                     /* Fill table */
   for( triangle=layer->triangles; triangle!=NULL; triangle=triangle->next )
   {
      *table = triangle;
      table++;
   }
   *table = NULL;

   for( i=0; i<3; i++ )
      layer->step[i] = layer->max[i] - layer->min[i];
   layer->regnTag = (char *)xvt_mem_zalloc( sizeof(char) );
   if( layer->regnTag == NULL )
      return fMEM;

   return 0;
}

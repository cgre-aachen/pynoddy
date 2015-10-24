/*
 * ============================
 * NAME:
 *                      dxfvox.c
 *
 * DESCRIPTION:
 *                      Module for generating voxels of data from the dxf modules.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher 1995
 *                      (c) Monash University 1995
 * ==============================
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "dxfvox.h"
#include "xvt.h"

/* Stack of points for the floodfill routine */
struct fillStack
{
        int index[3];                                           /* Index of the point */
        struct fillStack *next;         /* Next item in the stack */
};

#define static


/*************** Prototypes ***************/
#if XVT_CC_PROTO
static void push( struct fillStack **stack, int index[3] );
static void pop( struct fillStack **stack, int index[3] );
static int onTriangle( struct DXFhandle *handle, ftype point[3] );
#ifdef DXF_SURF
static int fillSurf( struct DXFhandle *handle, struct DXFvoxel *voxel,
                                int index[3], vtype *numRegions );
static int canExtendSurf( struct DXFhandle *handle, struct DXFvoxel *voxel,
                                int indexA[3], int indexB[3] );
static int fillMask( struct DXFhandle *handle, char *mask, ftype x, ftype y,
                                ftype z );
#endif

#ifdef DXF_VOL
static vtype getAdjacent( struct DXFvoxel *voxel, int index[3] );
static int fillVol( struct DXFhandle *handle, struct DXFvoxel *voxel,
                                int index[3], vtype *numRegions );
static int canExtendVol( struct DXFhandle *handle, struct DXFvoxel *voxel,
                                int indexA[3], int indexB[3] );
static int intersectInRegn( struct DXFlayer *layer, int index[3],
                                ftype line[3][2] );
static int intersectInList( struct DXFtriangle *triangle, ftype line[3][2],
                                char tag );
static int intersectTriangle( struct DXFtriangle *triangle, ftype line[3][2] );
static int intersectLine( ftype lineA[2][2], ftype lineB[2][2] );
static int noTriangles( struct DXFhandle *handle, ftype point[3] );
#endif
#else
static void push( );
static void pop( );
static int onTriangle( );
#ifdef DXF_SURF
static int fillSurf( );
static int canExtendSurf( );
static int fillMask( );
#endif

#ifdef DXF_VOL
static vtype getAdjacent( );
static int fillVol( );
static int canExtendVol( );
static int intersectInRegn( );
static int intersectInList( );
static int intersectTriangle( );
static int intersectLine( );
static int noTriangles( );
#endif
#endif




/*
 * ======================
 * NAME:
 *                      int
 *                      VOXinit( struct DXFvoxel *voxel, struct DXFhandle *handle, int num[3] )
 *
 * DESCRIPTION:
 *                      Initialises a voxel structure of arbitrary size.        This includes
 *                      filling the fields in the DXFvoxel structure and allocating memory.
 *                      The allocated memory is filled with the value V_UNUSED, meaning
 *                      unused
 *
 * PARAMETERS:
 *                      voxel                   Where to store the voxel.
 *                      handle                  The data to be used with the voxel.
 *                      num                             Number of elements in each direction.
 *
 * RETURNS:
 *                      Non-zero if initialised successfully, zero otherwise.
 * ======================
 */
int
#if XVT_CC_PROTO
VOXinit( struct DXFvoxel *voxel,
         ftype min[3], ftype max[3], int num[3] )
#else
VOXinit( voxel, min, max, num )
struct DXFvoxel *voxel;
ftype min[3], max[3];
int num[3];
#endif
{
        size_t numAlloc;                                                                        /* Number of elements to allocate */
        size_t i;

        if (!( voxel != NULL ))
           return (FALSE);

        memcpy( voxel->min, min, sizeof(ftype) * 3 );   /* Fill fields in structure */
        memcpy( voxel->size, num, sizeof(int) * 3 );
        for( i=0; i<3; i++ )
                voxel->step[i] = (max[i] - min[i]) / num[i];

                                                                                                                                                                                /* Allocate and initialise memory */
        numAlloc = voxel->size[0] * voxel->size[1] * voxel->size[2];
        voxel->voxel = (vtype *)xvt_mem_zalloc( sizeof(vtype) * numAlloc );
        if( voxel->voxel != NULL )
        {
                for( i=0; i<numAlloc; i++ )
                        voxel->voxel[i] = V_UNUSED;
                return 1;
        }
        else
                return 0;
}


/*
 * ========================
 * NAME:
 *                      void
 *                      VOXfree( struct DXFvoxel *voxel )
 *
 * DESCRIPTION:
 *                      Releases the memory held by a voxel.    For safety, the pointers to the
 *                      memory in the DXFvoxel structure are set to NULL.
 *
 * PRE:
 *                      The voxel must have been initialised.
 *
 * PARAMETERS:
 *                      voxel                                   The voxel to release.
 * ========================
 */
void
#if XVT_CC_PROTO
VOXfree( struct DXFvoxel *voxel )
#else
VOXfree( voxel )
struct DXFvoxel *voxel;
#endif
{
        xvt_mem_free( (char *) voxel->voxel );
        voxel->voxel = NULL;
}


/*
 * ========================
 * NAME:
 *                      void
 *                      VOXset( struct DXFvoxel *voxel, int x, int y, int z, vtype data )
 *
 * DESCRIPTION:
 *                      Sets one location in the voxel.
 *
 * PRE:
 *                      The voxel must have been initialised.
 *
 * PARAMETERS:
 *                      voxel                           The voxel to store data in.
 *                      x, y, z                         The index of the point to set.
 *                      data                                    The data to store in the voxel.
 * ========================
 */
void
#if XVT_CC_PROTO
VOXset( struct DXFvoxel *voxel, int x, int y, int z, vtype data )
#else
VOXset( voxel, x, y, z, data )
struct DXFvoxel *voxel;
int x, y, z;
vtype data;
#endif
{
        size_t index;                                                           /* Index of the location to store the data */

        assert( voxel != NULL );
        assert( voxel->voxel != NULL );
        assert( x >= 0 );
        assert( y >= 0 );
        assert( z >= 0 );
        assert( x < voxel->size[0] );
        assert( y < voxel->size[1] );
        assert( z < voxel->size[2] );

        index = (x * voxel->size[1] + y) * voxel->size[2] + z;
        voxel->voxel[index] = data;
}



/*
 * ========================
 * NAME:
 *                      vtype
 *                      VOXget( struct DXFvoxel *voxel, int x, int y, int z )
 *
 * DESCRIPTION:
 *                      Returns the data from one location in the voxel.
 *
 * PRE:
 *                      The voxel must have been initialised.
 *
 * PARAMETERS:
 *                      voxel                           The voxel to extract data from.
 *                      x, y, z                         The location to examine.
 *
 * RETURNS:
 *                      The data from the specified location.
 * ========================
 */
vtype
#if XVT_CC_PROTO
VOXget( struct DXFvoxel *voxel, int x, int y, int z )
#else
VOXget( voxel, x, y, z )
struct DXFvoxel *voxel;
int x, y, z;
#endif
{
        size_t index;                                                           /* Index of the location to store the data */

        assert( voxel != NULL );
        assert( voxel->voxel != NULL );
        assert( x >= 0 );
        assert( y >= 0 );
        assert( z >= 0 );
        assert( x < voxel->size[0] );
        assert( y < voxel->size[1] );
        assert( z < voxel->size[2] );

        index = (x * voxel->size[1] + y) * voxel->size[2] + z;
        return voxel->voxel[index];
}


/*
 * ================================
 * NAME:
 *                      void
 *                      VOXindex2point( struct DXFvoxel *voxel, int i, int j, int k,
 *                                                                                      ftype *x, ftype *y, ftype *z )
 *
 * DESCRIPTION:
 *                      Converts an integer voxel index into floating-point coordinates.
 *                      The coordinates depend on the locations for which the voxel was
 *                      initialised and the resolution of the voxel.    This means that they
 *                      will be different for each voxel.
 *
 * PARAMETERS:
 *                      voxel                           The voxel the index relates to.
 *                      i, j, k                         Index of a location inside the voxel.
 *                      x, y, z                         Used to return the point.
 *
 * PRE:
 *                      The voxel must have been initialised.  There is no requirement for the
 *                      point to be within the bounds of the voxel.
 *
 * RETURNS:
 *                      x, y & z are used to return the floating-point coordinates.
 * ================================
 */
int
#if XVT_CC_PROTO
VOXindex2point( struct DXFvoxel *voxel, int i, int j, int k,
                ftype *x, ftype *y, ftype *z )
#else
VOXindex2point( voxel, i, j, k, x, y, z )
struct DXFvoxel *voxel;
int i, j, k;
ftype *x, *y, *z;
#endif
{
        if (!( voxel != NULL ))
           return (FALSE);                                                        /* Check voxel initialised */
        if (!( voxel->voxel != NULL ))
           return (FALSE);

        *x = i * voxel->step[0] + voxel->min[0];
        *y = j * voxel->step[1] + voxel->min[1];
        *z = k * voxel->step[2] + voxel->min[2];
        
        return (TRUE);
}


/*
 * ============================
 * NAME:
 *                      static void
 *                      push( struct fillStack **stack, int index[3] )
 *
 * DESCRIPTION:
 *                      Push a point onto a stack.
 *
 * PARAMETERS:
 *                      stack                           Pointer to the stack.
 *                      index                           Index of the point.
 *                      mask                                    Mask to store for later (an asciiz string).
 *
 * PRE:
 *                      <stack> points to a valid stack, or is NULL.
 *
 * POST:
 *                      The point is on the stack, or an error message was issued.
 * ============================
 */
static void
#if XVT_CC_PROTO
push( struct fillStack **stack, int index[3] )
#else
push( stack, index )
struct fillStack **stack;
int index[3];
#endif
{
        struct fillStack *item;

        item = (struct fillStack *)xvt_mem_zalloc( sizeof(struct fillStack) );
        assert( item != NULL );                                                                         /* out of memory */

        memcpy( item->index, index, sizeof(int) * 3 );

        item->next = *stack;
        *stack = item;
}


/*
 * ======================
 * NAME:
 *                      static void
 *                      pop( struct fillStack **stack, int index[3] )
 *
 * DESCRIPTION:
 *                      Pop an point off a stack.
 *
 * PARAMETERS:
 *                      stack                           Pointer to the stack.
 *                      index                           Used to return the index of the point.
 *                      mask                                    Used to store the mask (an asciiz string).
 *
 * PRE:
 *                      <stack> points to a non-empty stack.
 *
 * POST:
 *                      The top element has been removed from the stack, and <index> set
 *                      to hold the data from that element.  <stack> has been set to point
 *                      to the modified stack.
 * ======================
 */
static void
#if XVT_CC_PROTO
pop( struct fillStack **stack, int index[3] )
#else
pop( stack, index )
struct fillStack **stack;
int index[3];
#endif
{
        struct fillStack *item;

        assert( stack != NULL );
        assert( *stack != NULL );                                       /* Empty stack */

        item = *stack;
        *stack = item->next;

        memcpy( index, item->index, sizeof(int)*3 );
        xvt_mem_free( (char *) item );
}


#ifdef DXF_SURF
/*
 * ======================
 * NAME:
 *                      int
 *                      VOXsurfInit( struct DXFvoxel *voxel, struct DXFhandle *handle,
 *                                                                       ftype min[3], ftype max[3] )
 *
 * DESCRIPTION:
 *                      Initialises a voxel structure for use with data from the surf module.
 *                      The voxel size is determined based on the number of layers in the data,
 *                      unlike VOXinit where the size is specified by the user.  The size is
 *                      chosen so that there are handle.numLayers*STEPS_PER_LAYER vertical
 *                      elements.  The other dimensions are chosen to use the same scale factor
 *                      as for the vertical direction.
 *
 * PARAMETERS:
 *                      voxel                   Where to store the voxel.
 *                      handle                  The data to be used with the voxel.
 *                      min                             The lowest values of each coordinate for the voxel.
 *                      max                             The highest values of each coordinate for the voxel.
 *
 * RETURNS:
 *                      Non-zero if initialised successfully, zero otherwise.
 * ======================
 */
int
#if XVT_CC_PROTO
VOXsurfInit( struct DXFvoxel *voxel, struct DXFhandle *handle,
             ftype min[3], ftype max[3] )
#else
VOXsurfInit( voxel, handle, min, max )
struct DXFvoxel *voxel;
struct DXFhandle *handle;
ftype min[3], max[3];
#endif
{
        size_t numAlloc;
        size_t i;

        if (!( voxel != NULL ))
           return (FALSE);
        if (!( handle != NULL ))
           return (FALSE);
        if (!( handle->conv == convSurf ))
           return (FALSE);
        if (!( handle->layer != NULL ))
           return (FALSE);

        memcpy( voxel->min, min, sizeof(ftype) * 3 );

                                                                                /* Use the number of layers to determine the voxel size */
        voxel->size[2] = handle->numLayers * STEPS_PER_LAYER;
        voxel->step[0] = voxel->step[1] = voxel->step[2]
                                                                 = (max[2] - min[2]) / (voxel->size[2] - 1);
        voxel->size[0] = (int)((max[0] - min[0]) / voxel->step[0] + 1);
        voxel->size[1] = (int)((max[1] - min[1]) / voxel->step[1] + 1);

                                                                                                                                                                                /* Allocate and initialise memory */
        numAlloc = voxel->size[0] * voxel->size[1] * voxel->size[2];
        voxel->voxel = (vtype *)xvt_mem_zalloc( sizeof(vtype) * numAlloc );
        if( voxel->voxel != NULL )
        {
                for( i=0; i<numAlloc; i++ )
                        voxel->voxel[i] = V_UNUSED;
                return 1;
        }
        else
                return 0;
}


/*
 * ==========================
 * NAME:
 *                      int
 *                      VOXsurfLayerAbove( struct DXFhandle *handle, struct DXFvoxel *voxel )
 *
 * DESCRIPTION:
 *                      Creates and fills a voxel with the index of the layer above each point.
 *                      If the point is above the top layer, VTYPE_MAX is stored.  If the point
 *                      has no layers above or below it, 0 is stored.
 *
 * PARAMETERS:
 *                      handle                                  The handle to extract data from.
 *                      voxel                                   The voxel to store data in.
 *
 * RETURNS:
 *                      Nonzero if successful, 0 otherwise.
 *                      <voxel> is used to return data.
 *
 * NOTE:
 *                      The voxel does NOT need to be initialised before calling this routine.
 * ==========================
 */
int
#if XVT_CC_PROTO
VOXsurfLayerAbove( struct DXFhandle *handle, struct DXFvoxel *voxel )
#else
VOXsurfLayerAbove( handle, voxel )
struct DXFhandle *handle;
struct DXFvoxel *voxel;
#endif
{
        int i, j, k;                            /* Index of the voxel location */
        ftype x, y, z;                  /* Coordinates of the voxel location */
        vtype layerNum;                 /* Current layer number */
        vtype layerAbove;       /* Layer above the point */
        ftype height;                   /* Distance of the layer from the point */
        ftype tempHeight;

        if (!( voxel != NULL ))
           return (FALSE);
        if (!( voxel->voxel != NULL ))
           return (FALSE);
        if (!( handle != NULL ))
           return (FALSE);
        if (!( handle->conv == convSurf ))
           return (FALSE);
        if (!( handle->numLayers > 0 ))
           return (FALSE);

        for( i=0; i<voxel->size[0]; i++ )                                                               /* For each voxel point */
        {
                for( j=0; j<voxel->size[1]; j++ )
                {
                        for( k=0; k<voxel->size[2]; k++ )
                        {
                                if (!VOXindex2point( voxel, i, j, k, &x, &y, &z ))
                                   return (FALSE);
                                layerAbove = VTYPE_MAX;
                                for( layerNum=0; (int)layerNum < handle->numLayers; layerNum++ )
                                {
                                        if( DXFsurfHeight( handle, layerNum, x, y, z, &tempHeight ) )
                                        {                                                                                                               /* If inside the layer */
                                                if( tempHeight <= 0 )                   /* If below the layer */
                                                {
                                                                                                                                                                                /* Note that heights are -ve */
                                                        if( (layerAbove == VTYPE_MAX) || (tempHeight > height) )
                                                        {
                                                                layerAbove = layerNum;
                                                                height = tempHeight;
                                                        }
                                                }
                                        }
                                } /* end for each layer */
                                VOXset( voxel, i, j, k, layerAbove );
                        } /* end for k */
                } /* end for j */
        } /* end for i */
        return 1;
}


/*
 * =========================
 * NAME:
 *                      vtype
 *                      VOXsurfRegion( struct DXFhandle *handle, struct DXFvoxel *voxel )
 *
 * DESCRIPTION:
 *                      Creates a voxel where the data points are a region index.  Regions
 *                      are defined as a set of points surrounded by layers.    All the points
 *                      in a region are connected.      Points with no layers above or below are
 *                      set to region 0.
 *
 * PARAMETERS:
 *                      handle                                  The handle to extract data from.
 *                      voxel                                   The voxel to store data in.
 *
 * RETURNS:
 *                      The number of regions.  0 indicates error.
 *                      <voxel> is used to return data.
 *
 * PRE:
 *                      The voxel must be initialised before calling this routine.
 *
 * METHOD:
 *                      This calls a floodfill type algorithm.  Each point of the voxel is
 *                      tested, and if it is vacant, a new region is created and filled from
 *                      that point.
 * ==========================
 */
vtype
#if XVT_CC_PROTO
VOXsurfRegion( struct DXFhandle *handle, struct DXFvoxel *voxel )
#else
VOXsurfRegion( handle, voxel )
struct DXFhandle *handle;
struct DXFvoxel *voxel;
#endif
{
        vtype region = 0;                                /* Flag for current region */
        int index[3];                                                   /* Index of current point */

        if (!( voxel != NULL ))
           return (FALSE);
        if (!( voxel->voxel != NULL ))
           return (FALSE);
        if (!( handle != NULL ))
           return (FALSE);
        if (!( handle->conv == convSurf ))
           return (FALSE);
        if (!( handle->numLayers > 0 ))
           return (FALSE);

        for( index[0]=0; index[0] < voxel->size[0]; index[0]++ )
        {
                for( index[1]=0; index[1] < voxel->size[1]; index[1]++ )
                {
                        for( index[2]=0; index[2] < voxel->size[2]; index[2]++ )
                        {
                                if( VOXget( voxel, index[0], index[1], index[2] ) > V_LAST_REGN )
                                {
                                        if( !fillSurf( handle, voxel, index, &region ) )
                                                return 0;
                                }
                        } /* end for k */
                } /* end for j */
        } /* end for i */
        return region;
}


/*
 * =====================
 * NAME:
 *                      static int
 *                      fillSurf( struct DXFhandle *handle, struct DXFvoxel *voxel,
 *                                                              int index[3], vtype *numRegions )
 *
 * DESCRIPTION:
 *                      The floodfill routine for VOXsurfRegion.        Note that points with no
 *                      layers above and below are part of the special region 0.
 *
 * PARAMETERS:
 *                      handle                          The source of the data.  This must contain surf data.
 *                      voxel                           The voxel used to store the data.
 *                      index                           The voxel point at which to start the fill.
 *                      numRegions              Pointer to the current number of regions.  This is
 *                                                                              modified if a new region is created.
 *
 * RETURNS:
 *                      Zero on error (probably out of memory).  Non-zero otherwise.
 *
 * PRE:
 *                      <handle> must be initialised and contain data from the surf module.
 *                      <voxel> must be initialised.
 *                      <index> must contain a point within the voxel.
 *
 * METHOD:
 *                      An intelligent floodfill which uses the stack sparingly.        A "seed"
 *                      location is placed on the stack, but adjacent locations are inhibited
 *                      from being placed on the stack.  The inhibition is released after
 *                      passing a wall, so another seed can then be pushed onto the stack.
 * ===================
 */
static int
#if XVT_CC_PROTO
fillSurf( struct DXFhandle *handle, struct DXFvoxel *voxel,
                        int index[3], vtype *numRegions )
#else
fillSurf( handle, voxel, index, numRegions )
struct DXFhandle *handle;
struct DXFvoxel *voxel;
int index[3];
vtype *numRegions;
#endif
{
        struct fillStack *stack = NULL;         /* Stack of points to examine */
        int direction;                                                                          /* Move in the +/- x direction */
        int newIndex[3];                                                                        /* Index of a point being examined */
        char *mask;                                                                                     /* Mask of layers */
        vtype flag;                                                                                     /* Flag to fill with */
        int canSeed[3][3];                                                              /* Set when a seed may be planted */
        ftype x, y, z;                                                                          /* Acutal coordinates of a point */
        int i, j;

        mask = (char *)xvt_mem_zalloc( sizeof(char) * (handle->numLayers+1) );
        if( mask == NULL )
                return 0;

        if (!VOXindex2point( voxel, index[0], index[1], index[2], &x, &y, &z ))
           return (FALSE);
        if( fillMask( handle, mask, x, y, z ) )
                flag = 0;
        else
        {
                (*numRegions)++;
                flag = *numRegions;
                assert( flag <= V_LAST_REGN );          /* Use a larger vtype */
        }
        xvt_mem_free( (char *) mask );
        mask = NULL;


        push( &stack, index );
        while( stack != NULL )
        {
                pop( &stack, index );
                canSeed[1][0] = canSeed[1][2] = canSeed[2][0] = canSeed[2][2] = 1;
                                                                                                                                        /* ^^^ Force a seed from first location */

                for( direction=1; direction>-2; direction-=2 )  /* Move right then left */
                {
                        VOXset( voxel, index[0], index[1], index[2], flag );
                        while( (index[0] >= 0) && (index[0] < voxel->size[0] ) )
                        {
                                newIndex[0] = index[0];

                                for( i=1; i<3; i++ )                            /* y or z index */
                                {
                                        newIndex[1] = index[1];
                                        newIndex[2] = index[2];
                                        for( j=-1; j<2; j+=2 )                  /* Add or subtract one */
                                        {
                                                newIndex[i] = index[i] + j;
                                                if( (newIndex[i] < 0) || (newIndex[i] >= voxel->size[i]) )
                                                        continue;

                                                if( canExtendSurf( handle, voxel, index, newIndex ) )  /* Possible seed site? */
                                                {
                                                        if( canSeed[i][j+1] )
                                                        {
                                                                push( &stack, newIndex );               /* Sow the seed */
                                                                canSeed[i][j+1] = 0;                                                    /* Avoid sowing another */
                                                        }
                                                }
                                                else
                                                        canSeed[i][j+1] = 1;                    /* Sow seed next time it is allowed */
                                        }
                                }

                                newIndex[0] = index[0] + direction;                             /* Select next location */
                                newIndex[1] = index[1];
                                newIndex[2] = index[2];
                                if( (newIndex[0] < 0) || (newIndex[0] >= voxel->size[0]) )
                                        break;                                                                                                                                                          /* Out of bounds */
                                if( !canExtendSurf( handle, voxel, index, newIndex ) )
                                        break;                                                                                                  /* Crossed a layer, or already done */
                                index[0] += direction;                  /* Actully make the move */
                        }
                        canSeed[1][0] = canSeed[1][2] = canSeed[2][0] = canSeed[2][2] = 0;
                                                                                                                         /* ^^^^ Don't seed from first location again */

                } /* end for direction */
        } /* end while stack not empty */
        return 1;
}



/*
 * =================
 * NAME:
 *                      static int
 *                      canExtendSurf( struct DXFhandle *handle, struct DXFvoxel *voxel,
 *                                                                               int indexA[3], int indexB[3] )
 *
 * DESCRIPTION:
 *                      Tests to see if the floodfill can extend the painted area from point
 *                      A to point B.  This occurs if B has not already been painted, and if
 *                      no layers are crossed between A and B.  The distinction between A and
 *                      B is important.
 *
 * PARAMETERS:
 *                      handle                                  The handle to take the (surf) data from.
 *                      voxel                                   The voxel which is being filled.
 *                      indexA                                  The point which is known to be inside the current
 *                                                                                      region.
 *                      indexB                                  A point which may be inside the current region.
 *
 * RETURNS:
 *                      Zero if the B is not inside the current region, hence the fill cannot
 *                      be extended from A to B.        Otherwise, non-zero is returned.
 *
 * PRE:
 *                      <handle> must be initialised and contain data from the surf module.
 *                      <voxel> must be initialised.
 *                      <indexA> and <indexB> must be inside the voxel.
 * ================
 */
static int
#if XVT_CC_PROTO
canExtendSurf( struct DXFhandle *handle, struct DXFvoxel *voxel,
                                         int indexA[3], int indexB[3] )
#else
canExtendSurf( handle, voxel, indexA, indexB )
struct DXFhandle *handle;
struct DXFvoxel *voxel;
int indexA[3], indexB[3];
#endif
{
        int layerNum;                                           /* Current layer number */
        int mask[2];                                                    /* Data for comparison of layers */
        int invalid[2];         /* Set if point is invalid for all layers */
        ftype dist;                                                     /* Distance from a layer */
        ftype point[2][3];                              /* Actual points */
        int i;

   invalid[0] = invalid[1] = 1;
   
        assert( handle->conv == convSurf );

        if( VOXget( voxel, indexB[0], indexB[1], indexB[2] ) <= V_LAST_REGN )
                return 0;

        if (!VOXindex2point( voxel, indexA[0], indexA[1], indexA[2],
                        point[0]+0, point[0]+1, point[0]+2 ))
           return (FALSE);
        if (!VOXindex2point( voxel, indexB[0], indexB[1], indexB[2],
                        point[1]+0, point[1]+1, point[1]+2 ))
           return (FALSE);

        for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
        {
                for( i=0; i<2; i++ )
                {
                        if( !DXFsurfHeight( handle, layerNum, point[i][0], point[i][1],
                                                                                                        point[i][2], &dist ) )
                        {
                                mask[i] = 3;                                             /* off the edge */
                        }
                        else
                        {
                                invalid[i] = 0;
                                if( dist > 0 )
                                        mask[i] = 2;
                                else
                                        mask[i] = 1;
                        }
                }

                if( mask[0] * mask[1] == 2 )
                        return 0;
        }
        if( invalid[0] != invalid[1] )
                return 0;
        return 1;
}


/*
 * =================
 * NAME:
 *                      static void
 *                      fillMask( struct DXFhandle *handle, char *mask,
 *                                                              ftype x, ftype y, ftype z )
 *
 * DESCRIPTION:
 *                      Fills a mask with flags to show if a point is above (1) or below (2)
 *                      each layer.  If the layer does not extend as far as this point, 3 is
 *                      stored.
 *
 * NOTE:
 *                      The values stored in the mask are chosen to enable simple comparison
 *                      of masks.
 *
 * PARAMETERS:
 *                      handle                                          Handle to extract data from.
 *                      mask                                                    Buffer to store the data.
 *                      x, y, z                                         Coordinates of the current point.
 *
 * RETURNS:
 *                      Data is returned using <mask>.
 * ================
 */
static int
#if XVT_CC_PROTO
fillMask( struct DXFhandle *handle, char *mask, ftype x, ftype y, ftype z )
#else
fillMask( handle, mask, x, y, z )
struct DXFhandle *handle;
char *mask;
ftype x, y, z;
#endif
{
        int layerNum;                                                   /* Number of the current layer */
        ftype dist;                                                             /* Result from DXFsurfHeight */
        int allVoid = 1;                                                /* Set if all layers are not valid */

        assert( handle != NULL );
        assert( handle->conv == convSurf );

        for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
        {
                if( !DXFsurfHeight( handle, layerNum, x, y, z, &dist ) )
                        mask[layerNum] = 3;                                             /* off the edge */
                else if( dist > 0 )                                                     /* below */
                {
                        mask[layerNum] = 2;
                        allVoid = 0;
                }
                else                                                                                                                    /* above */
                {
                        mask[layerNum] = 1;
                        allVoid = 0;
                }
        }
        return allVoid;
}
#endif

#ifdef DXF_VOL
/*
 * =======================
 * NAME:
 *                      vtype
 *                      VOXvolRegion( struct DXFhandle *handle, struct DXFvoxel *voxel )
 *
 * DESCRIPTION:
 *                      Creates a voxel where the data points are a region index.  Regions
 *                      are defined as a set of points which may be connected without crossing
 *                      any triangles or surfaces.      Note that, unlike VOXsurfRegion, this
 *                      function does not set points off the boundary to the special region 0.
 *                      However, if any points already have regions, they will not be touched.
 *                      The region 0 is not used.
 *
 * PARAMETERS:
 *                      handle                                  The handle to extract data from.
 *                      voxel                                   The voxel to store data in.
 *
 * RETURNS:
 *                      The number of regions.  0 indicates error.
 *                      <voxel> is used to return data.
 *
 * PRE:
 *                      The voxel must be initialised before calling this routine.
 *
 * METHOD:
 *                      This calls a floodfill type algorithm.  Each point of the voxel is
 *                      tested, and if it is vacant, a new region is created and filled from
 *                      that point.
 *
 *                      A floodfill is NOT started if the point is actually on a triangle.
 *                      If this was not considered, these points may each be assigned to
 *                      different regions / colours.    The second pass of the function fills
 *                      in any gaps left, using regions adjacent to the points.
 * ==========================
 */
vtype
#if XVT_CC_PROTO
VOXvolRegion( struct DXFhandle *handle, struct DXFvoxel *voxel )
#else
VOXvolRegion( handle, voxel )
struct DXFhandle *handle;
struct DXFvoxel *voxel;
#endif
{
        vtype region = 0;                                       /* Flag for current region */
        vtype current;                                                  /* Current data at this point */
        int index[3];                                                   /* Index of current point */
        ftype point[3];                                                 /* Actual coordinates of point */
        int someFailed;                                         /* Set if some positions need filling */

        if (!( voxel != NULL ))
           return ((vtype) FALSE);
        if (!( voxel->voxel != NULL ))
           return ((vtype) FALSE);
        if (!( handle != NULL ))
           return ((vtype) FALSE);
        if (!( handle->conv == convVol ))
           return ((vtype) FALSE);
        if (!( handle->numLayers > 0 ))
           return ((vtype) FALSE);

        for( index[0]=0; index[0] < voxel->size[0]; index[0]++ )
        {
                for( index[1]=0; index[1] < voxel->size[1]; index[1]++ )
                {
                        for( index[2]=0; index[2] < voxel->size[2]; index[2]++ )
                        {
                                if( VOXget( voxel, index[0], index[1], index[2] ) == V_UNUSED )
                                {
                                        if (!VOXindex2point( voxel, index[0], index[1], index[2],
                                                               point+0, point+1, point+2 ))
                                           return ((vtype) FALSE);
                                        if( !onTriangle( handle, point ) )
                                        {
                                                if( !fillVol( handle, voxel, index, &region ) )
                                                        return 0;
                                        }
                                }
                        } /* end for k */
                } /* end for j */
        } /* end for i */

        do
        {
                someFailed = 0;
                for( index[0]=0; index[0] < voxel->size[0]; index[0]++ )
                {
                        for( index[1]=0; index[1] < voxel->size[1]; index[1]++ )
                        {
                                for( index[2]=0; index[2] < voxel->size[2]; index[2]++ )
                                {
                                        current = VOXget( voxel, index[0], index[1], index[2] );
                                        if( (current == V_UNDECIDED) || (current == V_UNUSED) )
                                        {
                                                current = getAdjacent( voxel, index );
                                                if( current > V_LAST_REGN )
                                                        someFailed = 1;
                                                VOXset( voxel, index[0], index[1], index[2], current );
                                        }
                                } /* end for k */
                        } /* end for j */
                } /* end for i */
        } while( someFailed );

        return region;
}


/*
 * =================================
 * NAME:
 *                      static vtype
 *                      getAdjacent( struct DXFvoxel *voxel, int index[3] )
 *
 * DESCRIPTION:
 *                      This routine returns the colour of one of the locations adjacent to
 *                      <index>.        It chooses the first location which has a valid colour.
 *
 * PARAMTERS:
 *                      voxel                           The voxel containing the colours.
 *                      index                           The location to find the adjacent colour for.
 *
 * RETURNS:
 *                      The selected colour, or V_UNUSED if no suitable colours were found.
 * ==================================
 */
static vtype
#if XVT_CC_PROTO
getAdjacent( struct DXFvoxel *voxel, int index[3] )
#else
getAdjacent( voxel, index )
struct DXFvoxel *voxel;
int index[3];
#endif
{
        int newIndex[3];                                        /* Location to examine */
        vtype value;                                                    /* Colour at newIndex */
        int i, j, k;

        for( i=-1; i<2; i++ )
        {
                newIndex[0] = index[0] + i;
                if( (newIndex[0] < 0) || (newIndex[0] >= voxel->size[0]) )
                        continue;
                for( j=-1; j<2; j++ )
                {
                        newIndex[1] = index[1] + j;
                        if( (newIndex[1] < 0) || (newIndex[1] >= voxel->size[1]) )
                                continue;
                        for( k=-1; k<2; k++ )
                        {
                                newIndex[2] = index[2] + k;
                                if( (newIndex[2] < 0) || (newIndex[2] >= voxel->size[2]) )
                                        continue;

                                value = VOXget( voxel, newIndex[0], newIndex[1], newIndex[2] );
                                if( value <= V_LAST_REGN )
                                        return value;
                        }
                }
        }
        return V_UNUSED;
}

/*
 * ===================
 * NAME:
 *                      static int
 *                      fillVol( struct DXFhandle *handle, struct DXFvoxel *voxel,
 *                                              int index[3], vtype *numRegions )
 *
 * DESCRIPTION:
 *                      The floodfill routine for VOXvolRegion.
 *
 * PARAMETERS:
 *                      handle                          The source of the data.  This must contain surf data.
 *                      voxel                           The voxel used to store the data.
 *                      index                           The voxel point at which to start the fill.
 *                      numRegions              Pointer to the current number of regions.  This is
 *                                                                              modified if a new region is created.
 *
 * RETURNS:
 *                      Zero on error (probably out of memory).  Non-zero otherwise.
 *
 * PRE:
 *                      <handle> must be initialised and contain data from the surf module.
 *                      <voxel> must be initialised.
 *                      <index> must contain a point within the voxel.
 *
 * METHOD:
 *                      An intelligent floodfill which uses the stack sparingly.        A "seed"
 *                      location is placed on the stack, but adjacent locations are inhibited
 *                      from being placed on the stack.  The inhibition is released after
 *                      passing a wall, so another seed can then be pushed onto the stack.
 * ===================
 */
static int
#if XVT_CC_PROTO
fillVol( struct DXFhandle *handle, struct DXFvoxel *voxel,
                        int index[3], vtype *numRegions )
#else
fillVol( handle, voxel, index, numRegions )
struct DXFhandle *handle;
struct DXFvoxel *voxel;
int index[3];
vtype *numRegions;
#endif
{
        struct fillStack *stack = NULL;         /* Stack of points to examine */
        int direction;                                                                          /* Move in the +/- x direction */
        int newIndex[3];                                                                        /* Index of a point being examined */
        char *mask;                                                                                     /* Mask of layers */
        vtype flag;                                                                                     /* Flag to fill with */
        int canSeed[3][3];                                                              /* Set when a seed may be planted */
        int i, j;

        mask = (char *)xvt_mem_zalloc( sizeof(char) * (handle->numLayers+1) );
        if( mask == NULL )
                return 0;

        (*numRegions)++;
        flag = *numRegions;
        assert( flag <= V_LAST_REGN );          /* Use a larger vtype */


        push( &stack, index );
        while( stack != NULL )
        {
                pop( &stack, index );
                canSeed[1][0] = canSeed[1][2] = canSeed[2][0] = canSeed[2][2] = 1;
                                                                                                                                        /* ^^^ Force a seed from first location */

                for( direction=1; direction>-2; direction-=2 )  /* Move right then left */
                {
                        VOXset( voxel, index[0], index[1], index[2], flag );
                        while( (index[0] >= 0) && (index[0] < voxel->size[0] ) )
                        {
                                newIndex[0] = index[0];

                                for( i=1; i<3; i++ )                            /* y or z index */
                                {
                                        newIndex[1] = index[1];
                                        newIndex[2] = index[2];
                                        for( j=-1; j<2; j+=2 )                  /* Add or subtract one */
                                        {
                                                newIndex[i] = index[i] + j;
                                                if( (newIndex[i] < 0) || (newIndex[i] >= voxel->size[i]) )
                                                        continue;

                                                if( canExtendVol( handle, voxel, index, newIndex ) )
                                                {                                                                                                                                                        /* Possible seed site? */
                                                        if( canSeed[i][j+1] )
                                                        {
                                                                push( &stack, newIndex );                               /* Sow the seed */
                                                                canSeed[i][j+1] = 0;                                                    /* Avoid sowing another */
                                                        }
                                                }
                                                else
                                                {
                                                        canSeed[i][j+1] = 1;                    /* Sow seed next time it is allowed */
                                                }
                                        }
                                }

                                newIndex[0] = index[0] + direction;                             /* Select next location */
                                newIndex[1] = index[1];
                                newIndex[2] = index[2];
                                if( (newIndex[0] < 0) || (newIndex[0] >= voxel->size[0]) )
                                        break;                                                                                                                                                          /* Out of bounds */
                                if( !canExtendVol( handle, voxel, index, newIndex ) )
                                {
                                        break;                                                                                                  /* Crossed a layer, or already done */
                                }
                                index[0] += direction;                  /* Actully make the move */
                        }
                        canSeed[1][0] = canSeed[1][2] = canSeed[2][0] = canSeed[2][2] = 0;
                                                                                                                         /* ^^^^ Don't seed from first location again */

                } /* end for direction */
        } /* end while stack not empty */
        return 1;
}



/*
 * ================
 * NAME:
 *                      static int
 *                      onTriangle( struct DXFhandle *handle, ftype point[3] )
 *
 * DESCRIPTION:
 *                      Determines if a point is on a triangle.  These points can create
 *                      ambiguities, so are avoided at certain times.
 *
 * PARAMETERS:
 *                      handle                          Holds the lists of triangles.
 *                      point                           Point which may be on a triangle.
 *
 * RETURNS:
 *                      Non-zero if the point is on a triangle, zero otherwise.
 *
 * PRE:
 *                      <handle> must contain data from the vol module.
 * ==============
 */
static int
#if XVT_CC_PROTO
onTriangle( struct DXFhandle *handle, ftype point[3] )
#else
onTriangle( handle, point )
struct DXFhandle *handle;
ftype point[3];
#endif
{
        int layerNum;                                                                                   /* Current layer */
        struct DXFlayer *layer;
        struct DXFtriangle *triangle;                   /* Current triangle */
        struct DXFtriangle **table;                             /* Table of triangles */
        int moreTri;                                                                                            /* Set if more triangles to examine */
        int index[3];                                                                                   /* Index of region to examine */
        long regNum;                                                                                            /* Current region number */
        ftype tpoint[3];                                                                                /* Temporary point */
        int i;

        if (!( handle->conv == convVol ))
           return (FALSE);

        for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
        {
                layer = handle->layer + layerNum;
                if( layer->regions == NULL )
                        triangle = layer->triangles;
                else
                {
                        moreTri = 1;
                        for( i=0; i<3; i++ )                                    /* Find region to examine */
                        {
                                index[i] = (int)((point[i] - layer->min[i]) / layer->step[i]);
                                if( (index[i] < 0) || (index[i] >= layer->numRegns[i]) )
                                        moreTri = 0;
                        }
                        if( !moreTri )                          /* Skip to next layer */
                                continue;
                        regNum = ((index[0] * layer->numRegns[1]) + index[1])
                                                                                                        * layer->numRegns[2] + index[2];
                        table = layer->regions[regNum];
                        if( table == NULL )
                                triangle = NULL;
                        else
                                triangle = *table;
                }

                while( triangle != NULL )
                {
                        memcpy( tpoint, point, sizeof(ftype)*3 );
                        dxfChangeAxis( tpoint, triangle->axis[0], triangle->axis[1],
                                                                                 triangle->axis[2] );
                        if( fabs( (tpoint[2] - triangle->elevation) / (tpoint[2] + 1) ) < ZTOL )
                        {
                                if( dxfInsideTriangle( triangle->points, 0, 1, tpoint[0], tpoint[1] ) )
                                        return 1;
                        }

                        if( layer->regions == NULL )
                                triangle = triangle->next;
                        else
                        {
                                table++;
                                triangle = *table;
                        }
                }
        } /* for each layer */
        return 0;
}


/*
 * =================
 * NAME:
 *                      static int
 *                      canExtendVol( struct DXFhandle *handle, struct DXFvoxel *voxel,
 *                                                                      int indexA[3], int indexB[3] )
 *
 * DESCRIPTION:
 *                      Tests to see if the floodfill can extend the painted region from
 *                      point A to point B.  This occurs if B is not painted, and if the line
 *                      AB does not pass through any triangles.  The distinction between A and
 *                      B is important.
 *
 * PARAMETERS:
 *                      handle                                  The handle to take the (surf) data from.
 *                      voxel                                   The voxel which is being filled.
 *                      indexA                                  The point which is known to be inside the current
 *                                                                                      region.
 *                      indexB                                  A point which may be inside the current region.
 *
 * RETURNS:
 *                      Zero if the B is not inside the current region, hence the fill cannot
 *                      be extended from A to B.        Otherwise, non-zero is returned.
 *
 * PRE:
 *                      <handle> must be initialised and contain data from the surf module.
 *                      <voxel> must be initialised.
 *                      <indexA> and <indexB> must be inside the voxel.
 * ================
 */
int
#if XVT_CC_PROTO
canExtendVol( struct DXFhandle *handle, struct DXFvoxel *voxel,
              int indexA[3], int indexB[3] )
#else
canExtendVol( handle, voxel, indexA, indexB )
struct DXFhandle *handle;
struct DXFvoxel *voxel;
int indexA[3], indexB[3];
#endif
{
        int layerNum;                                           /* Current layer number */
        struct DXFlayer *layer;
        ftype point[2][3];                              /* Actual points */
        ftype line[3][2];                               /* Parameters of the line between the points */
        ftype plane[3];                                         /* Definition of three planes which */
                                                                                                                /*                                              intersect the line */
        int planeInc[3];                                        /* Sign of the plane increments */
        ftype param[3];                                         /* Parameters of the plane/line intersection */
        int index[3];                                           /* Index of the region to examine */
        ftype prevParam;                                        /* Parameter of the previous intersection */
        int nextIntersect;                              /* Index of the next plane to intersect with */
        int i;

        if (!( handle->conv == convVol ))
           return (FALSE);

        if( VOXget( voxel, indexB[0], indexB[1], indexB[2] ) != V_UNUSED )
                return 0;

        if (!VOXindex2point( voxel, indexA[0], indexA[1], indexA[2],
                               point[0]+0, point[0]+1, point[0]+2 ))
           return (FALSE);
        if (!VOXindex2point( voxel, indexB[0], indexB[1], indexB[2],
                               point[1]+0, point[1]+1, point[1]+2 ))
           return (FALSE);

        for( i=0; i<3; i++ )
        {
                line[i][0] = point[0][i];
                line[i][1] = point[1][i] - point[0][i];
                if( line[i][1] > 0 )
                        planeInc[i] = 1;
                else
                        planeInc[i] = -1;
        }

        for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
        {
                layer = handle->layer + layerNum;
                layer->tag++;
                if( layer->tag == 0 )
                        dxfClearTags( layer );

                if( layer->regions == NULL )
                {
                        if( intersectInList( layer->triangles, line, layer->tag ) )
                                return 0;
                }
                else
                {
                        for( i=0; i<3; i++ )                                                                                    /* find initial region */
                        {
                                index[i] = (int)floor((point[0][i] - layer->min[i]) / layer->step[i]);
                                                                                                                                                                                                /* Find first border */
                                plane[i] = layer->min[i] + layer->step[i] * index[i];
                                if( planeInc[i] > 0 )
                                        plane[i] += layer->step[i];
                        }

                        if( intersectInRegn( layer, index, line ) )
                                return 0;

                        prevParam = 0;
                        do
                        {
                                nextIntersect = -1;
                                for( i=0; i<3; i++ )                                            /* Find parameter of intersections */
                                {
                                        if( fabs(line[i][1]) > ZTOL )                           /* Parallel to plane */
                                        {
                                                param[i] = -(line[i][0] - plane[i]) / line[i][1];
                                                if( (param[i] >= prevParam) && (param[i] <= 1) )
                                                {
                                                        if( (nextIntersect == -1) || (param[i] < param[nextIntersect]) )
                                                                nextIntersect = i;
                                                }
                                        }
                                }

                                if( nextIntersect != -1 )
                                {
                                        index[nextIntersect] += planeInc[nextIntersect];
                                        plane[nextIntersect] += (planeInc[nextIntersect]
                                                                                                                                        * layer->step[nextIntersect]);
                                        prevParam = param[nextIntersect];

                                        if( intersectInRegn( layer, index, line ) )
                                                return 0;
                                }

                        } while( nextIntersect != -1 );

                } /* End if regions==NULL */
        } /* End for each layer */
        return 1;
}

/*
 * ==================
 * NAME:
 *                      static int
 *                      intersectInRegn( struct DXFlayer *layer, int index[3],
 *                                                                                       ftype line[3][2] )
 *
 * DESCRIPTION:
 *                      Tests to see if any of the triangles in a region intersect with
 *                      a given line segment.
 *
 * PARAMETERS:
 *                      layer                           The layer from which the region is taken.
 *                      index                           The index of the region to examine.
 *                      line                                    The line, specified so that the endpoints occur when
 *                                                                              the parameter is 0 or 1.
 *
 * RETURNS:
 *                      Non-zero if a triangle intersects with the line segment, 0 otherwise.
 *
 * NOTE:
 *                      As triangles are examined, DXFtriangle.tag is set to the current value
 *                      of DXFlayer.tag to prevent a triangle from being examined more than
 *                      once.  This function does NOT update DXFlayer.tag.
 * =================
 */
static int
#if XVT_CC_PROTO
intersectInRegn( struct DXFlayer *layer, int index[3], ftype line[3][2] )
#else
intersectInRegn( layer, index, line )
struct DXFlayer *layer;
int index[3];
ftype line[3][2];
#endif
{
        size_t regNum;                                                                  /* Number of the region to investigate */
        struct DXFtriangle *triangle;   /* Current triangle */
        struct DXFtriangle **table;             /* Table of triangles */
        ftype d;                                                                                                /* Parameter of plane not stored with tri */
        ftype t;                                                                                                /* Parameter of line */
        ftype point[3];                                                                 /* Point of line in plane of triangle */
        int i;

        for( i=0; i<3; i++ )                    /* Is region out of bounds? */
        {
                if( (index[i] < 0) || (index[i] >= layer->numRegns[i]) )
                        return 0;
        }

        regNum = ((index[0] * layer->numRegns[1]) + index[1])
                                                                                        * layer->numRegns[2] + index[2];
        if( layer->regnTag[regNum] == layer->tag )              /* Region already visited? */
                return 0;
        layer->regnTag[regNum] = layer->tag;

        table = layer->regions[regNum];
        if( table == NULL )                                                                                                     /* Region is empty */
                return 0;

        for( ; *table!=NULL; table++ )
        {
                triangle = *table;
                if( triangle->tag != layer->tag )
                {
                        triangle->tag = layer->tag;
                        if( intersectTriangle( triangle, line ) )
                                return 1;
                }
        }
        return 0;
}


/*
 * =================
 * NAME:
 *                      static int
 *                      intersectInList( struct DXFtriangle *triangle, ftype line[3][2],
 *                                                                                       char tag )
 *
 * DESCRIPTION:
 *                      Tests to see if any of the triangles in a list intersect with a
 *                      given line segment.
 *
 * PARAMETERS:
 *                      triangle                                        Head of the list of triangles (NULL-terminated).
 *                      line                                                    The line segment, specified so that the parameter
 *                                                                                              domain is [0,1].
 *                      tag                                                     The current value of DXFlayer.tag to prevent
 *                                                                                              triangles from being tested more than once.
 *
 * RETURNS:
 *                      Non-zero if one or more triangles intersect with the line segment, zero
 *                      otherwise.
 * =================
 */
static int
#if XVT_CC_PROTO
intersectInList( struct DXFtriangle *triangle, ftype line[3][2], char tag )
#else
intersectInList( triangle, line, tag )
struct DXFtriangle *triangle;
ftype line[3][2];
char tag;
#endif
{
        while( triangle != NULL )
        {
                if( triangle->tag != tag )
                {
                        triangle->tag = tag;
                        if( intersectTriangle( triangle, line ) )
                                return 1;
                }
                triangle = triangle->next;

        }
        return 0;
}


/*
 * ===================
 * NAME:
 *                      static int
 *                      intersectTriangle( struct DXFtriangle *triangle, ftype line[3][2] )
 *
 * DESCRIPTION:
 *                      Tests to see if a single triangle and a line intersect.  Special cases
 *                      are involved if a line terminates on a triangle, or if the line is
 *                      parallel to the plane of the triangle.
 *
 * PARAMETERS:
 *                      triangle                                        The triangle in question.
 *                      line                                                    The line to intersect with, defined so that the
 *                                                                                              domain of the parameter is [0,1].
 *
 * RETURNS:
 *                      Non-zero if an intersection occurs, zero otherwise.
 *
 * METHOD:
 *                      Finds the intersection of the line and the plane of the triangle.
 *                      The point of intersection is then tested to see if it is inside
 *                      the triangle.  If the line terminates on the triangle, an arbitrary
 *                      but consistent choice is made.  This is so that the triangle will be
 *                      in one region or the other.
 *
 *                      If a line is in the plane of the triangle and intersects with the
 *                      triangle, the intersection is always reported.
 * ===================
 */
static int
#if XVT_CC_PROTO
intersectTriangle( struct DXFtriangle *triangle, ftype line[3][2] )
#else
intersectTriangle( triangle, line )
struct DXFtriangle *triangle;
ftype line[3][2];
#endif
{
        ftype d;                                                /* The missing parameter of the plane */
        ftype t;                                                /* The parameter of the line/plane intersection */
        ftype point[2][3];      /* Coordinates of the intersection */
        ftype pline[2][2];      /* Line in the plane of the triangle */
        int i, j;

        d = -(  triangle->axis[2][0]*triangle->corner[0][0]
                                + triangle->axis[2][1]*triangle->corner[0][1]
                                + triangle->axis[2][2]*triangle->corner[0][2]);
        t =     triangle->axis[2][0]*line[0][1]
                        + triangle->axis[2][1]*line[1][1]
                        + triangle->axis[2][2]*line[2][1];

        if( fabs(t) < ZTOL )                                            /* Parallel to the plane */
        {
                for( i=0; i<2; i++ )
                {
                        for( j=0; j<3; j++ )                                                                                            /* Find endpoints of line */
                                point[i][j] = line[j][0] + i*line[j][1];
                        dxfChangeAxis( point[i], triangle->axis[0], triangle->axis[1],
                                                                                 triangle->axis[2] );

                        if( fabs( point[i][2] - triangle->elevation ) > ZTOL )
                                return 0;                                                                               /* Line is in different, parallel plane */

                        if( dxfInsideTriangle( triangle->points, 0,1, point[i][0], point[i][1] ) )
                        {
                                return 1;                                                                               /* Line crosses the triangle */
                        }
                }

                for( i=0; i<2; i++ )                                                    /* Find equations of line in 2D */
                {
                        pline[i][0] = point[0][i];
                        pline[i][1] = point[1][i] - point[0][i];
                }

                for( i=0; i<3; i++ )
                {
                        if( intersectLine( triangle->line[i], pline ) )
                        {
                                return 1;
                        }
                }
        }
        else                                                    /* The line crosses the plane of the triangle */
        {
                t = -(  triangle->axis[2][0]*line[0][0]
                                        + triangle->axis[2][1]*line[1][0]
                                        + triangle->axis[2][2]*line[2][0] + d) / t;

                if( (t >= 0) && (t <= 1) )                       /* Need to consider endpoints */
                {
                        for( i=0; i<3; i++ )
                                point[0][i] = line[i][0] + t*line[i][1];
                        dxfChangeAxis( point[0], triangle->axis[0], triangle->axis[1],
                                                                                 triangle->axis[2] );

                        if( dxfInsideTriangle( triangle->points, 0,1, point[0][0], point[0][1] ) )
                        {
                                if( (t == 0) || (t == 1) )                      /* On an end */
                                {
                                        t = (t < ZTOL) ? 1 : 0;                                                                  /* Find other end of line */
                                        for( i=0; i<3; i++ )
                                                point[0][i] = line[i][0] + t*line[i][1];
                                        dxfChangeAxis( point[0], triangle->axis[0], triangle->axis[1],
                                                                                                 triangle->axis[2] );
                                        if( point[0][2] > triangle->elevation )
                                                return 1;
                                }
                                else
                                        return 1;
                        }
                }
        }
        return 0;
}


/*
 * =====================
 * NAME:
 *                      static int
 *                      intersectLine( ftype lineA[2][2], ftype lineB[2][2] )
 *
 * DESCRIPTION:
 *                      Determines if two 2D line segments intersect.
 *
 * PRE:
 *                      The line segments are defined so that the endpoints occur when the
 *                      parameters are 0 or 1.
 *
 * POST:
 *                      0 is returned if the line segments do not intersect between their
 *                      endpoints.      Otherwise, non-zero is returned.
 *
 * METHOD:
 *                      The parameters of the intersection of the lines are found.      If both
 *                      the parameters r & s are in the range [0,1] then the line segments
 *                      intersect.
 *
 *                      If the lines are parallel (den=0) and colinear (r=0) then the endpoints
 *                      of lineB are compared against lineA.    If the endpoints lie in two
 *                      different sections of A (using the 0,1 boundaries) or if either lies
 *                      in the region r=[0,1], then the lines overlap.
 * ====================
 */
static int
#if XVT_CC_PROTO
intersectLine( ftype lineA[2][2], ftype lineB[2][2] )
#else
intersectLine( lineA, lineB )
ftype lineA[2][2], lineB[2][2];
#endif
{
        ftype numS;                                     /* Numerators of the fractions */
        ftype numR;
        ftype r, s;                                     /* Parameters of the lines */
        ftype den;                                              /* Denominator of the fraction */
        int prod;                                               /* Product used to find where the ends lie */
                                                                                                /*      >0 = no intersection, 0,-1 intersection */
        int i;

        numR =  lineB[1][1] * (lineB[0][0] - lineA[0][0])
                                + lineB[0][1] * (lineA[1][0] - lineB[1][0]);
        numS =  lineA[1][1] * (lineB[0][0] - lineA[0][0])
                                + lineA[0][1] * (lineB[1][0] - lineA[1][0]);
        den = lineA[0][1]*lineB[1][1] - lineA[1][1]*lineB[0][1];

        if( fabs(den) < ZTOL )                                                                                          /* Parallel lines */
        {
                if( fabs(numR) < ZTOL )                                                                                 /* colinear */
                {
                        prod = 1;
                        for( i=0; i<2; i++ )                                                    /* Examine ends of the line */
                        {
                                if( fabs( lineA[0][1] ) > ZTOL )                        /* Avoid division by 0 */
                                {
                                        r = (lineB[0][0] + i*lineB[0][1] - lineA[0][0]) / lineA[0][1];
                                        if( r < 0 )
                                                prod *= -1;
                                        else if( r <= 1 )
                                                prod = 0;
                                }
                                else if( fabs( lineA[1][1] ) > ZTOL )
                                {
                                        r = (lineB[1][0] + i*lineB[1][1] - lineA[1][0]) / lineA[1][1];
                                        if( r < 0 )
                                                prod *= -1;
                                        else if( r <= 1 )
                                                prod = 0;
                                }
                        }
                        if( prod <= 0 )
                                return 1;
                }
        }
        else
        {
                r = numR / den;
                s = numS / den;
                if( (r >= 0) && (r <= 1) && (s >= 0) && (s <= 1) )               /* Intersection */
                        return 1;
        }
        return 0;
}


/*
 * =================
 * NAME:
 *                      void
 *                      VOXblankEdges( struct DXFhandle *handle, struct DXFvoxel *voxel )
 *
 * DESCRIPTION:
 *                      Attempts to prevent leakage from voxels using data from the vol
 *                      module.  The points in the voxel which have no triangles above or
 *                      below them are marked as empty (0).
 *
 * PARAMETERS:
 *                      handle                          The handle holding the surfaces.
 *                      voxel                           The voxel to store the data in.
 *
 * PRE:
 *                      <handle> must be initialised and contain data from the vol module.
 *                      <voxel> must be initialised.
 * ================
 */
int
#if XVT_CC_PROTO
VOXblankEdges( struct DXFhandle *handle, struct DXFvoxel *voxel )
#else
VOXblankEdges( handle, voxel )
struct DXFhandle *handle;
struct DXFvoxel *voxel;
#endif
{
        int index[3];                                                   /* Index of current point */
        ftype point[3];                                                 /* Actual point */

        if (!( voxel != NULL ))
           return (FALSE);
        if (!( voxel->voxel != NULL ))
           return (FALSE);
        if (!( handle != NULL ))
           return (FALSE);
        if (!( handle->numLayers > 0 ))
           return (FALSE);

        for( index[0]=0; index[0] < voxel->size[0]; index[0]++ )
        {
                for( index[1]=0; index[1] < voxel->size[1]; index[1]++ )
                {
                        if (!VOXindex2point( voxel, index[0], index[1], 0,
                                                    point+0,  point+1,  point+2 ))
                           return (FALSE);
                        if( noTriangles( handle, point ) )
                        {
                                for( index[2]=0; index[2] < voxel->size[2]; index[2]++ )
                                {
                                   if ( VOXget( voxel, index[0], index[1], index[2] ) > V_LAST_REGN )
                                      VOXset( voxel, index[0], index[1], index[2], 0 );
                                }
                        } /* end for k */
                } /* end for j */
        } /* end for i */
        
        return (TRUE);
}


/*
 * =================
 * NAME:
 *                      static int
 *                      noTriangles( struct DXFhandle *handle, ftype point[3] )
 *
 * DESCRIPTION:
 *                      Determines if there are any triangles above a certain point.
 *
 * PARAMETERS:
 *                      handle                          The handle to take the triangles from.
 *                      point                           The coordinates of the point to examine.
 *
 * RETURNS:
 *                      Non-zero if there are no triangles above the point, zero otherwise.
 * =================
 */
static int
#if XVT_CC_PROTO
noTriangles( struct DXFhandle *handle, ftype point[3] )
#else
noTriangles( handle, point )
struct DXFhandle *handle;
ftype point[3];
#endif
{
        int layerNum;                                                                   /* Current layer */
        struct DXFlayer *layer;
        int index[3];                                                                   /* Region to examine */
        long regNum;                                                                            /* Region number */
        struct DXFtriangle **table;             /* Table of triangles */
        struct DXFtriangle *triangle;   /* Current triangle */
        int nextLayer;                                                                  /* Flag set to skip to next layer */
        int i;

        if (!( handle != NULL ))
           return (FALSE);
        if (!( handle->conv == convVol ))
           return (FALSE);

        for( layerNum=0; layerNum<handle->numLayers; layerNum++ )
        {
                layer = handle->layer + layerNum;
                layer->tag++;
                if( layer->tag == 0 )
                        dxfClearTags( layer );

                if( layer->regions == NULL )
                {
                        for( triangle=layer->triangles; triangle!=NULL; triangle=triangle->next )
                        {
                                if( dxfInsideTriangle( triangle->corner, 0, 1, point[0], point[1] ) )
                                        return 0;
                        }
                }
                else
                {
                        nextLayer = 0;
                        for( i=0; i<2; i++ )
                        {
                                index[i] = (int)((point[i] - layer->min[i]) / layer->step[i]);
                                if( (index[i] < 0) || (index[i] >= layer->numRegns[i]) )
                                        nextLayer = 1;
                        }
                        if( nextLayer )                                         /* Skip to next layer */
                                continue;

                        for( index[2]=0; index[2] < layer->numRegns[2]; index[2]++ )
                        {
                                regNum = ((index[0] * layer->numRegns[1]) + index[1])
                                                                                                                * layer->numRegns[2] + index[2];

                                table = layer->regions[regNum];
                                if( table != NULL )
                                {
                                        for( ; *table!=NULL; table++ )
                                        {
                                                triangle = *table;
                                                if( triangle->tag != layer->tag )
                                                {
                                                        triangle->tag = layer->tag;
                                                        if( dxfInsideTriangle( triangle->corner, 0, 1,
                                                                                                                                                 point[0], point[1] ) )
                                                        {
                                                                return 0;
                                                        }
                                                }
                                        }
                                }
                        } /* end for each z region */
                } /* end if regions==NULL */
        } /* end for each layer */
        return 1;
}
#endif

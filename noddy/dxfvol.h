/*
 * =============================
 * NAME:
 *                      dxfvol.h
 *
 * DESCRIPTION:
 *                      Include file specific to the vol module.        Automatically included if
 *                      required.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher, 1995
 *                      (c) Monash University, 1995
 * =============================
 */

#ifndef DXFVOL_H_INCLUDED
#define DXFVOL_H_INCLUDED

#ifdef DXF_VOL

struct DXFdistRes               /* Data returned by distTriangle */
{
  struct DXFtriangle *triangle; /* Triangle for which this data applies */
  ftype distance;               /* Distance to the triangle */
  ftype dot;                    /* Indicates above or below the triangle */
  ftype vdist;                  /* Vertical distance from the triangle (+ve) */
  ftype hdist;                  /* Horizontal distance from the triangle */
  int difficult;                /* Set if cannot tell between inside/outside */
};


/************ Constants for inside/outside map *************/
#define iOUTSIDE    0           /* Point definitely outside */
#define iINSIDE     1           /* Point definitely inside */
#define iUNKNOWN    2           /* Point is unknown */
#define iSETIN      3           /* Set to inside in next pass */
#define iSETOUT     4           /* Set to outside in next pass */


#if XVT_CC_PROTO
/* dxfvconv.c */
int DXFvolOrient( struct DXFhandle *handle );
int DXFvolConv( struct DXFhandle *handle, ftype matrix[4][4],
        int inside );
int dxfCmpCorner( ftype a[3], ftype b[3] );

/* dxfvout.c */
void DXFvolDist( struct DXFhandle *handle, int layerNum, ftype x, ftype y,
        ftype z, ftype *distance, int *inside );
void DXFvolRaster( struct DXFhandle *handle, int layerNum, ftype *dist,
        int *inside, int num[3], ftype min[3], ftype max[3] );
struct DXFtriangle *dxfClosestTriangle( struct DXFlayer *layer,
        ftype x, ftype y, ftype z, ftype *distance, int *inside );
void dxfClearTags( struct DXFlayer *layer );
void DXFfillGaps( int *inout, int xsize, int ysize, int zsize );
#else
/* dxfvconv.c */
int DXFvolOrient();
int DXFvolConv();
int dxfCmpCorner();

/* dxfvout.c */
void DXFvolDist();
void DXFvolRaster();
struct DXFtriangle *dxfClosestTriangle();
void dxfClearTags();
void DXFfillGaps();
#endif


#endif /* DXF_VOL */
#endif /* DXFVOL_H_INCLUDED */

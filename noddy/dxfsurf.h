/*
 * =============================
 * NAME:
 *                      dxfsurf.h
 *
 * DESCRIPTION:
 *                      Include file specific to the surf module.  Automatically included if
 *                      required.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher, 1995
 *                      (c) Monash University, 1995
 * =============================
 */

#ifndef DXFSURF_H_INCLUDED
#define DXFSURF_H_INCLUDED

#ifdef DXF_SURF


enum DXFaxis {
   aCentroid,    /* use centroid, least-squares to define axis */
   aWCS          /* use the file's World Coordinate System */
};



#if XVT_CC_PROTO
/* dxfsconv.c */
int DXFsurfConv( struct DXFhandle *handle, enum DXFaxis axistype,
        ftype matrix[4][4], int regionalise );

/* dxfsout.c */
void DXFsurfRaster( struct DXFhandle *handle, int layerNum, ftype *array,
        int num[2], ftype min[2], ftype max[2] );
int DXFsurfHeight( struct DXFhandle *handle, int layerNum, ftype x, ftype y,
        ftype z, ftype *dist );
int DXFsurfDist( struct DXFhandle *handle, int layerNum, ftype x, ftype y,
        ftype z, ftype *nx, ftype *ny, ftype *nz );
void DXFsurfAngle( struct DXFhandle *handle, ftype *intersect, ftype *axis );
#else
/* dxfsconv.c */
int DXFsurfConv();

/* dxfsout.c */
void DXFsurfRaster();
int DXFsurfHeight();
int DXFsurfDist();
void DXFsurfAngle();
#endif

#endif /* DXF_SURF */
#endif /* DXFSURF_H_INCLUDED */

/*
 * ==================================
 * NAME:
 *                      dxfvox.h
 *
 * DESCRIPTION:
 *                      Include file for generating voxels from multiple triangulated surfaces.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher 1995
 *                      (c) Monash University 1995
 * ==================================
 */

#ifndef DXFSVOX_H_INCLUDED
#define DXFSVOX_H_INCLUDED

#include <limits.h> 
#include "dxf.h"

/********************** Configurable section *****************/

/* The elements of the voxel are of the following type */
/* Note that vtype and VTYPE_MAX must agree */
typedef unsigned char vtype;
#define VTYPE_MAX UCHAR_MAX

#ifdef DXF_SURF 
/* This controls the voxel produced by VOXsurfInit. */
/* The vertical size of a voxel is layers*STEPS_PER_LAYER. */
/* A higher value gives a larger voxel. */
#define STEPS_PER_LAYER 10
#endif

/********************** End configurable section *************/

/************** Special values stored in voxels ************/
#define V_UNUSED VTYPE_MAX                                                      /* This location empty */
#define V_UNDECIDED (VTYPE_MAX - 1)             /* Don't know what to store there */
#define V_LAST_REGN (VTYPE_MAX - 2)             /* Last region number available */

/************** Data types *************/

typedef struct DXFvoxel
{
        vtype *voxel;                                                           /* Pointer to the voxel data */
        int size[3];                                                                    /* Dimensions of the voxel */
        ftype min[3];                                                           /* Minimum value of each coordinate */
        ftype step[3];                                                          /* Distance between voxel points */
} DXFVOXEL;


/************** Prototypes ************/

#if XVT_CC_PROTO
int VOXinit( struct DXFvoxel *voxel, ftype min[3], ftype max[3], int num[3] );
void VOXfree( struct DXFvoxel *voxel );
void VOXset( struct DXFvoxel *voxel, int x, int y, int z, vtype data );
vtype VOXget( struct DXFvoxel *voxel, int x, int y, int z );
int VOXindex2point( struct DXFvoxel *voxel, int i, int j, int k, ftype *x,
                                ftype *y, ftype *z );

#ifdef DXF_SURF
int VOXsurfInit( struct DXFvoxel *voxel, struct DXFhandle *handle, ftype min[3],
                                ftype max[3] );
int VOXsurfLayerAbove( struct DXFhandle *handle, struct DXFvoxel *voxel );
vtype VOXsurfRegion( struct DXFhandle *handle, struct DXFvoxel *voxel );
#endif

#ifdef DXF_VOL
vtype VOXvolRegion( struct DXFhandle *handle, struct DXFvoxel *voxel );
int VOXblankEdges( struct DXFhandle *handle, struct DXFvoxel *voxel );
#endif
#else
int VOXinit();
void VOXfree();
void VOXset();
vtype VOXget();
int VOXindex2point();

#ifdef DXF_SURF
int VOXsurfInit();
int VOXsurfLayerAbove();
vtype VOXsurfRegion();
#endif

#ifdef DXF_VOL
vtype VOXvolRegion();
int VOXblankEdges();
#endif
#endif

#endif /* DXFSVOX_H_INCLUDED */

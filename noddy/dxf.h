/*
 * ===========================================
 * NAME:
 *            dxf.h
 *
 * DESCRIPTION:
 *            Include file for users of the dxf modules.
 *
 * AUTHOR:
 *            Ewen Christopher
 *            (c) Ewen Christopher 1995
 *            (c) Monash University 1995
 * ===========================================
 */

#ifndef DXF_H_INCLUDED
#define DXF_H_INCLUDED

#include "xvt.h"

/**************** Configurable Section ****************/
/* To disable support for a module, comment out its define */
#define DXF_SURF
#define DXF_VOL
#define DXF_LINE

#ifdef DXF_SURF
#define DXF_SURF_OR_VOL
#endif
#ifdef DXF_VOL
#define DXF_SURF_OR_VOL
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#if (XVTWS == MTFWS) || (XVTWS == MACWS)
#define stricmp(X,Y) strcmp(X,Y)
#endif

/* Floating point type used */
typedef float ftype;

/* Length of a name string */
#define NAMELEN 40

/* Used to provide a relaxed 'equals' test for floating point numbers */
#define ZTOL (ftype)(1e-3)

/* Used to determine how many regions divideRegions creates */
#define TRI_PER_REG (1)

/* The constant Pi (not very accurate) */
#define PI 3.1415927

/* Define STATS to collect some statistics */
#define STATS


/***************** End configurable section ****************/

/***************** Consistency check for defines ************/



/***************** Data types ****************/

/* Specifies which conversion routine has been used on the data */
enum conversionEnum { convNone, convSurf, convVol, convLine };

#ifdef DXF_SURF_OR_VOL
struct DXFtriangle
/* Information on one triangle */
{
   struct DXFtriangle *next;  /* list of triangles */

                              /* first index is coord num, second is x,y,z */
   ftype corner[3][3];        /* Modified corners */
   ftype origcor[3][3];       /* Original coordinates */

   ftype min[3];              /* Minimum value of the coordinates */
   ftype max[3];              /* Maximum value of the coordinates */

#ifdef DXF_SURF
   int degenerate;            /* Degenerate in this orientation */
   ftype plane[4];            /* Parameters of the plane */
#endif

#ifdef DXF_VOL
   ftype axis[3][3];          /* Axis for the plane of the triangle */
   ftype points[3][3];        /* Points projected onto the plane */
   ftype elevation;           /* Elevation of the triangle from the plane */
   ftype line[3][2][2];       /* Equations of the 3 lines: Each line has 2 */
                              /*       equations, each of which have 2 consts */

   char tag;                  /* Tag to prevent repeated examinations via */
                              /*       exploreRegion */
#endif
};
#endif


struct DXFlayer
/* Information on each layer */
{
   char layerName[ NAMELEN ];         /* name of the layer */

#ifdef DXF_SURF_OR_VOL
   ftype min[3];                   /* Min & max values of each coordinate */
   ftype max[3];

   int numTriangles;               /* Number of triangles in the layer */
   struct DXFtriangle *triangles;  /* list of triangles */

   struct DXFtriangle ***regions;  /* array of regions */
   int numRegns[3];                /* number of regions in x,y direction */
   ftype step[3];                  /* size of regions */
#endif

#ifdef DXF_SURF
   ftype maxO[3];                  /* Min & max in original coordinates */
   ftype minO[3];
#endif

#ifdef DXF_VOL
   unsigned flip    :1;            /* All triangles in layer are flipped */
   char *regnTag;                  /* Array of tags for each region */
   char tag;                       /* tag for exploreRegions */
#endif

#ifdef DXF_LINE         /* Data for the lines module */
   struct DXFline *lines;          /* List of lines read in */
   struct DXFpoint *points;        /* List of points in order */
   ftype offset[2];                /* Offset used to shift the line */
   ftype rotation;                 /* Rotation of the line (radians) */
   ftype scale[2];                 /* Amount of scaling performed */
#endif

};


typedef struct DXFhandle
{
   int resultFlags;          /* 0=no error */
   enum conversionEnum conv; /* Conversion type performed */
                           
   int numLayers;            /* Number of layers in array */
   struct DXFlayer *layer;   /* Dynamic array of layer handles */

#ifdef DXF_SURF_OR_VOL
   ftype min[3];             /* Minimum values of x,y,z coordinates */
   ftype max[3];             /* Maximum values of x,y,z coordinates */
#endif

#ifdef DXF_SURF
   ftype maxO[3];            /* min, max in original coordinates */
   ftype minO[3];
   ftype axis[3][3];         /* for converting original to final coords */
   ftype centroid[3];        /* the centroid of all points */
   ftype planeConst;         /* Param d of the plane. axis[2]= -a,-b,-c */
   ftype aspect[3];
#endif
} DXFHANDLE;

struct DXFgroup                         /* Result of dxfReadGroup */
{
   int num;                                 /* Tag of this group */
   union
   {
      char text[100];                   /* Data from a textual group (eg layer name) */
      int inum;                               /* An integer group (eg flags) */
      ftype fnum;                          /* A floating point group (eg coordinate) */
   } d;
};




/************** Constants for the warning flags ***********/

                                                /* fatal error if any of these bits set */
#define fFATALMASK      (fFOPEN | fMEM | fNODATA | fMERGE)

#define fFOPEN        0x01             /* couldn't open file */
#define fBADFACE      0x02             /* a 3DFACE was found that wasn't a triangle */
#define fBADPOLY      0x04             /* a POLYLINE was found that we couldn't use */
#define fMEM             0x08             /* Out of memory */
#define fNODATA       0x10             /* No data to work with */

                                                /* Specific to the surf module */
#define fAXIS         0x20             /* couldn't transform to least squares axis */

                                                /* Specific to the vol module */
#define fDISJOINT       0x40             /* Surface is disjoint, or similar error */
#define fORIENT       0x80             /* Surface could not be oriented */

                                                /* Specific to the line module */
#define fMERGE        0x100         /* Could not merge segments into one line */


/************ Prototypes ***********/
#if XVT_CC_PROTO
/* DXFread.c */
void DXFread( struct DXFhandle *handle, char *filename );
struct DXFlayer *dxfStoreLayer( int *numLayers, struct DXFlayer *layer,
   char name[], int *flags );
void dxfCrossProduct( ftype result[3], ftype a[3], ftype b[3] );
void dxfArbitraryAxis( ftype x[3], ftype y[3], ftype z[3] );
void dxfReadGroup( FILE *dxfFile, struct DXFgroup *group );
#if (XVTWS == MTFWS)
void dxfScalarProduct( );
#else
#if (XVTWS == MACWS)
#else
void dxfScalarProduct( ftype [3], ftype, ftype [3]);
#endif
#endif
void dxfAddVector( ftype a[3], ftype b[3] );
void dxfUnitVector( ftype v[3] );

/* DXFmisc.c */
void DXFfree( struct DXFhandle *handle );
void dxfMinMax( struct DXFhandle *handle );
int dxfDivideRegions( struct DXFlayer *layer, enum conversionEnum conv );
void dxfApplyMatrix( struct DXFtriangle *triangle, ftype matrix[4][4] );
void dxfChangeOrigin( ftype point[3], ftype centroid[3] );
void dxfChangeAxis( ftype point[3], ftype x[3], ftype y[3],
   ftype z[3] );
int dxfInsideTriangle( ftype corner[3][3], int xx, int yy, ftype x, ftype y );
#else
/* DXFread.c */
void DXFread();
struct DXFlayer *dxfStoreLayer();
void dxfCrossProduct();
void dxfArbitraryAxis();
void dxfReadGroup();
#if (XVTWS == MACWS)
#else
void dxfScalarProduct();
#endif
void dxfAddVector();
void dxfUnitVector();

/* DXFmisc.c */
void DXFfree();
void dxfMinMax();
int dxfDivideRegions();
void dxfApplyMatrix();
void dxfChangeOrigin();
void dxfChangeAxis();
int dxfInsideTriangle();
#endif


/****************** Module-specific data structures and prototypes *********/
#ifdef DXF_SURF
#include "dxfsurf.h"
#endif

#ifdef DXF_VOL
#include "dxfvol.h"
#endif

#ifdef DXF_LINE
#include "dxfline.h"
#endif


#endif              /* DXF_H_INCLUDED */

/*****************************************************************************
 *	File:		3dHierarchy.h
 *
 *	©1991 Mark M. Owen. All rights reserved.
 *****************************************************************************/
#ifndef	_3dHIERARCHY_
#define	_3dHIERARCHY_

#include "3dColor.h"

#define	INVALID_REQUEST	-1
#define	MEMORY_ALLOC_ERROR	-2

#define	MAXFACETV	10	/* facet geometry max vertices	*/

typedef struct
{
	Point3d p;		/* vertex coordinate values	*/
	BOOLEAN impure;		/* true if already transformed	*/
} Vrtx, *pVrtx, **hVrtx;

typedef struct
{
	Point3d centroid;	/* center point			*/
	short nIx;
	short ixVrtx[MAXFACETV];/* number of vertex indices	*/
} Facet, *pFacet, **hFacet;

typedef struct
{
	short nV;		/* number of vertices		*/
	Vrtx __huge *pV;	/* handle to array of vertices	*/
	short nF;		/* number of facets		*/
	Facet __huge *pF;	/* handle to array of facets	*/
	RendAttr raOptions;	/* rendering options		*/
	BOOLEAN instanced;	/* apply instance transform?	*/
	Matrix3D instanceXf;	/* instance transformation	*/
	long refCon;		/* user reference constant	*/
} Patch, *pPatch, **hPatch;

typedef struct
{
	short nP;	/* number of patches */
	pPatch pP;	/* handle to array of patches	*/
} Group, *pGroup, **hGroup;
	
typedef struct
{
	short nG;	/* number of groups */
	pGroup *pG;	/* array of group handles */
} Collection, *pCollection,**hCollection;



#if XVT_CC_PROTO
int MakePolyFacet (Facet __huge *, int, int, int*, Vrtx __huge *);
void MakeRectFacet (Facet __huge *, int, int, int, int, int, Vrtx __huge *);

int NewPatch (pGroup, int, int, Vrtx __huge *, int, Facet __huge *, RendAttr, Matrix3D*);
void DisposePatch (pPatch, int);
int ClonePatch (pPatch, int);
pGroup NewGroup (int);
void DisposeGroup (pGroup);
int CloneGroup (pGroup*);
pCollection NewCollection (int);
void DisposeCollection (pCollection);
int CloneCollection (pCollection*);

int SetPatchTransform (pGroup,int,Matrix3D*);
int SetGroupTransform (pGroup,Matrix3D*);
int SetCollectionTransform (pCollection,Matrix3D*);

int CatPatchTransform (pGroup,int,Matrix3D*);
int CatGroupTransform (pGroup,Matrix3D*);
int CatCollectionTransform (pCollection,Matrix3D*);

int SetPatchRenderingAttribs (pGroup,int,RendAttr);
int SetGroupRenderingAttribs (pGroup,RendAttr);
int SetCollectionRenderingAttribs (pCollection,RendAttr);
#else
int MakePolyFacet ();
void MakeRectFacet ();

int NewPatch ();
void DisposePatch ();
int ClonePatch ();
pGroup NewGroup ();
void DisposeGroup ();
int CloneGroup ();
pCollection NewCollection ();
void DisposeCollection ();
int CloneCollection ();

int SetPatchTransform ();
int SetGroupTransform ();
int SetCollectionTransform ();

int CatPatchTransform ();
int CatGroupTransform ();
int CatCollectionTransform ();

int SetPatchRenderingAttribs ();
int SetGroupRenderingAttribs ();
int SetCollectionRenderingAttribs ();
#endif

#endif

/*
 * =========================
 * NAME:
 *                      dxfline.c
 *
 * DESCRIPTION:
 *                      Routines for the line related functions.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher, 1995
 *                      (c) Monash University, 1995
 * =========================
 */

#include "xvt.h"
#include <stdio.h>
#include <string.h>
#if (XVTWS == MACWS)
#else
#include <malloc.h>
#endif
#include <math.h>
#include <assert.h>
#include "dxf.h"
#include "noddy.h"

#ifndef DXF_LINE
#pragma message( "This file not useable without DXF_LINE" )
#endif

/************************** Prototypes ******************/

#if XVT_CC_PROTO
static struct DXFlayer *readLines( FILE *dxfFile, int *numLayers,
                                int *resultFlags );
static struct DXFlayer *storeLine( int *numLayers, struct DXFlayer *layer,
                                char *layerName, struct DXFline line, int *flags );
static int readPoly2D( FILE *dxfFile, struct DXFline *line, char layerName[] );
static void readVertex( FILE *dxfFile, ftype point[3], char layerName[] );
static void readEntLine( FILE *dxfFile, struct DXFline *line,
                                char layerName[] );
static void plistAppend( struct DXFpoint **list, ftype point[3] );
static void leftMost( struct DXFlayer *layer );
static void doRotation( struct DXFpoint *points, ftype rotation );
static void doOffset( struct DXFpoint *points, ftype offset[2] );
static ftype maxYvalue( struct DXFpoint *points );
static void doScale( struct DXFpoint *points, ftype scale[2] );
#else
static struct DXFlayer *readLines();
static struct DXFlayer *storeLine();
static int readPoly2D();
static void readVertex();
static void readEntLine();
static void plistAppend();
static void leftMost();
static void doRotation();
static void doOffset();
static ftype maxYvalue();
static void doScale();
#endif

/*
 * ===========================
 * NAME:
 *                      static struct DXFlayer *
 *                      readLines( FILE *dxfFile, int *numLayers, int *resultFlags )
 *
 * DESCRIPTION:
 *                      Reads any collection of LINEs, POLYLINEs and POINTS as a list of line
 *                      segments.
 *
 * PARAMETERS:
 *                      dxfFile                         The file to read from.
 *                      numLayers               Used to return the number of layers in the file.
 *                      resultFlags     Used to return error flags.
 *
 * RETURNS:
 *                      A pointer to a table of layers.
 *
 * NOTE:
 *                      Section divisions within the DXF file are ignored.
 *                      It is assumed that the file contains lines, not triangles or other
 *                      shapes.
 * ===========================
 */
static struct DXFlayer *
#if XVT_CC_PROTO
readLines( FILE *dxfFile, int *numLayers, int *resultFlags )
#else
readLines( dxfFile, numLayers, resultFlags )
FILE *dxfFile;
int *numLayers;
int *resultFlags;
#endif
{
        struct DXFgroup group;                                          /* Data read from the file */
        int result;                                                                                     /* Result from reading a line */
        char layerName[NAMELEN];                                        /* Layer data belongs to */
        struct DXFlayer *layer = NULL;          /* Table of layers */
        struct DXFline line;                                                    /* Line passed to reading functions */

        ftype point[3];                                                                         /* Point read from the file */
        char prevLayer[NAMELEN];                                        /* Previous layer */
        int pointRead = 0;                                                              /* Previous entity was a point */

        *numLayers = 0;
        *resultFlags = 0;

        while(1)
        {
                dxfReadGroup( dxfFile, &group );
                if( group.num != 0 )                                                                                                    /* Looking for an entity */
                        continue;
                else if( strcmp( group.d.text, "EOF" ) == 0 )
                        break;
                else if(         !strcmp( group.d.text, "POINT" )
                                                || !strcmp( group.d.text, "VERTEX" ) )
                {
                        readVertex( dxfFile, point, layerName );
                        if( pointRead )                                                          /* Attempt to string points into a line */
                        {
                                if( strcmp( prevLayer, layerName ) == 0 )
                                        plistAppend( &(line.points), point );
                                else
                                        layer = storeLine( numLayers, layer, prevLayer, line, resultFlags );
                        }
                        else
                        {
                                line.points = NULL;
                                plistAppend( &(line.points), point );
                        }
                        strcpy( prevLayer, layerName );
                        pointRead = 1;
                }
                else                                                                    /* not a point */
                {
                        if( pointRead )                                                         /* Some points still in storeage */
                        {
                                layer = storeLine( numLayers, layer, layerName, line, resultFlags );
                                pointRead = 0;
                        }

                        if( strcmp( group.d.text, "POLYLINE" ) == 0 )
                        {
                                result = readPoly2D( dxfFile, &line, layerName );
                                if( !result )
                                        (*resultFlags) |= fBADPOLY;
                                else
                                        layer = storeLine( numLayers, layer, layerName, line, resultFlags );
                        }
                        else if(         !strcmp( group.d.text, "LINE" )
                                                        || !strcmp( group.d.text, "3DLINE" ) )
                        {
                                readEntLine( dxfFile, &line, layerName );
                                layer = storeLine( numLayers, layer, layerName, line, resultFlags );
                        }
                }
        }

        if( pointRead )                                                         /* Some points still in storeage */
                layer = storeLine( numLayers, layer, layerName, line, resultFlags );
        return layer;
}



/*
 * =====================================
 * NAME:
 *                      static struct DXFlayer *
 *                      storeLine( int *numLayers, struct DXFlayer *layer,
 *                                                               char *layerName,
 *                                                               struct DXFline line,
 *                                                               int *flags )
 *
 * DESCRIPTION:
 *                      Allocates memory for line and adds it to the list for the
 *                      appropriate layer.      If the layer does not exist, it is created using
 *                      dxfStoreLayer.
 *
 * PARAMETERS:
 *                      numLayers, layer                                the table of layers
 *                      layerName                                                       the layer of this line
 *                      line                                                                            the data for this line
 *                      flags                                                                   pointer to resultFlags
 *
 * RETURNS:
 *                      Pointer to the new layer table
 * =====================================
 */
static struct DXFlayer *
#if XVT_CC_PROTO
storeLine( int *numLayers, struct DXFlayer *layer,
                                         char *layerName,
                                         struct DXFline line,
                                         int *flags )
#else
storeLine( numLayers, layer, layerName, line, flags )
int *numLayers;
struct DXFlayer *layer;
char *layerName;
struct DXFline line;
int *flags;
#endif
{
        int lnum;                                                                                                                                               /* current layer number */
        struct DXFline *newData = NULL;                                                         /* new triangle */

        for( lnum=0; lnum<(*numLayers); lnum++ )                        /* find a matching layer */
        {
                if( strcmp( layer[lnum].layerName, layerName ) == 0 )
                        break;
        }
        if( lnum >= (*numLayers) )
        {
         layer = dxfStoreLayer( numLayers, layer, layerName, flags );
         lnum = *numLayers - 1;
        }
                                                                                                                                                                                                        /* allocate storeage */
        newData = (struct DXFline  *)xvt_mem_zalloc( sizeof(struct DXFline ) );
        if( newData == NULL )
        {
                (*flags) |= fMEM;                                                                                                /* signal out of memory */
        }
        else
        {
                *newData = line;                                                                                                         /* store data */

                newData->next = layer[lnum].lines;              /* prepend to list */
                layer[lnum].lines = newData;
        }

        return layer;
}


/*
 * ======================
 * NAME:
 *                      static int
 *                      readPoly2D( FILE *dxfFile, struct DXFline *line,
 *                                                                      char layerName[] )
 *
 * DESCRIPTION:
 *                      Read a line from a POLYLINE and store the list of verticies.
 *                      Sets layerName to the layer of this entity, or "" if not given.
 *                      Positions the file after the POLYLINE's data.
 *
 * PARAMETERS:
 *                      dxfFile                                 The file to read from.
 *                      line                                            Pointer to the record to store the line in.
 *                      layerName                       Used to return the name of the layer.
 *
 * RETURNS:
 *                      Zero on error (eg a bad POLYLINE entity).
 *                      Non-zero otherwise.
 * =======================
 */
static int
#if XVT_CC_PROTO
readPoly2D( FILE *dxfFile, struct DXFline *line, char layerName[] )
#else
readPoly2D( dxfFile, line, layerName )
FILE *dxfFile;
struct DXFline *line;
char layerName[];
#endif
{
        struct DXFgroup group;                  /* Data read from the file */
        int twoDim = 0;                                                 /* 1 if it is a 2D polyline */
        int ecsUsed = 0;                                                /* 1 if ECS system was used */
        ftype offset[3];
        ftype extrusion = (ftype) 0;                            /* extrusion (38) of a 2D polyline */
        ftype Ze[3];                  /* the Z axis vector, groups 210, 220, 230 */
        ftype Xe[3], Ye[3];                             /* ECS axis */
        ftype temp[3], sum[3];

        int result = 1;                                                 /* 1 if a triangle was read */
        ftype point[3];                                                 /* The point / vertex read */

        offset[0] = offset[1] = offset[2] = (ftype) 0;
        Ze[0] = Ze[1] = Ze[2] = (ftype) 0;
        layerName[0] = '\0';        /* default - no layer name */
        line->points = NULL;                            /* Clear list of points */

        dxfReadGroup( dxfFile, &group);
        while( (group.num != 0) || (strcmp( group.d.text, "SEQEND") != 0) )
        {
                switch( group.num )
                {
                        case 0:
                                if( strcmp( group.d.text, "VERTEX" ) == 0 )
                                {
                                        readVertex( dxfFile, point, group.d.text );
                                        if( twoDim && ecsUsed )                                                 /* apply ECS transformation */
                                        {
                                                if( line->points == NULL )                              /* First point - find axis */
                                                        dxfArbitraryAxis( Xe, Ye, Ze );

                                                sum[0] = sum[1] = sum[2] = (ftype) 0;

                                                dxfScalarProduct( temp, point[0], Xe );
                                                dxfAddVector( sum, temp );
                                                dxfScalarProduct( temp, point[1], Ye );
                                                dxfAddVector( sum, temp );
                                                dxfScalarProduct( temp, point[2], Ze );
                                                dxfAddVector( sum, temp );

                                                memcpy( point, sum, sizeof(ftype)*3 );
                                        }
                                        plistAppend( &(line->points), point );
                                }
                                else                                                                            /* Unexpected group */
                                        result = 0;
                                break;

                        case 8:
                                strncpy( layerName, group.d.text, NAMELEN );
                                layerName[NAMELEN-1] = '\0';
                                break;

                        case 10:                                                                                        /* offset of ECS from WCS origin?? */
                        case 20:
                        case 30:
                                offset[group.num / 10 - 1] = group.d.fnum;
                                break;

                        case 38:                                                                                        /* extrusion along Ze axis */
                                extrusion = group.d.fnum;
                                break;

                        case 70:                                                                                        /* flags */
                                if( !(group.d.inum & 8) )                        /* 2D polyline */
                                        twoDim = 1;
                                break;

                        case 210:                                                                               /* ECS Z axis */
                        case 220:
                        case 230:
                                Ze[ group.num / 10 - 21 ] = group.d.fnum;
                                ecsUsed = 1;
                                break;
                }

                dxfReadGroup( dxfFile, &group );
        }

        return result;
}



/*
 * ============================
 * NAME:
 *                      static void
 *                      readVertex( FILE *dxfFile, ftype point[3], char layerName[] )
 *
 * DESCRIPTION:
 *                      Reads in a VERTEX or POINT entity.
 *
 * PARAMETERS:
 *                      dxfFile                                         The file to read from.
 *                      point                                           Used to store the point.
 *                      layerName                               Used to store the name of the layer.
 *
 * RETURNS:
 *                      <point> and <layerName> are used to return data.
 * ============================
 */
static void
#if XVT_CC_PROTO
readVertex( FILE *dxfFile, ftype point[3], char layerName[] )
#else
readVertex( dxfFile, point, layerName )
FILE *dxfFile;
ftype point[3];
char layerName[];
#endif
{
        long pos;                                                                                                       /* Position in the file */
        struct DXFgroup group;                                                  /* Data read from file */

        layerName[0] = '\0';

        point[0] = point[1] = point[2] = (ftype) 0;                     /* Assume zero coords */

        do
        {
                pos = ftell( dxfFile );
                dxfReadGroup( dxfFile, &group );
                if( group.num != 0 )
                {
                        switch( group.num )
                        {
                                case 8:                                                                                 /* Layer name */
                                        strncpy( layerName, group.d.text, NAMELEN-1 );
                                        layerName[NAMELEN-1] = '\0';
                                        break;

                                case 10:                                                                                /* Coordinates */
                                case 20:
                                case 30:
                                        point[ group.num / 10 - 1 ] = group.d.fnum;
                                        break;
                        }
                }
        } while( group.num != 0 );
        fseek( dxfFile, pos, SEEK_SET );
}


/*
 * =====================
 * NAME:
 *                      static void
 *                      readEntLine( FILE *dxfFile, struct DXFline *line, char layerName[] )
 *
 * DESCRIPTION:
 *                      Read in a LINE or 3DLINE entity.
 *
 * PARAMETERS:
 *                      dxfFile                                 The file to read from.
 *                      line                                            Pointer to the record to store the line in.
 *                      layerName                       Used to return the name of the layer.
 *
 * RETURNS:
 *                      <line> and <layerName> are used to return data.
 * =====================
 */
static void
#if XVT_CC_PROTO
readEntLine( FILE *dxfFile, struct DXFline *line, char layerName[] )
#else
readEntLine( dxfFile, line, layerName )
FILE *dxfFile;
struct DXFline *line;
char layerName[];
#endif
{
        long pos;                                                                       /* Position in the file */
        struct DXFgroup group;                  /* Data read from file */
        ftype a[3];                                                             /* Endpoints of the line */
        ftype b[3];

        layerName[0] = '\0';

        a[0] = a[1] = a[2] = (ftype) 0;                                                         /* Assume zero coords */
        b[0] = b[1] = b[2] = (ftype) 0;

        do
        {
                pos = ftell( dxfFile );
                dxfReadGroup( dxfFile, &group );
                switch( group.num )
                {
                        case 0:                                                                                 /* End of entity */
                                fseek( dxfFile, pos, SEEK_SET );
                                break;

                        case 8:                                                                                 /* Layer name */
                                strncpy( layerName, group.d.text, NAMELEN-1 );
                                layerName[NAMELEN-1] = '\0';
                                break;

                        case 10:                                                                                /* Coordinates */
                        case 20:
                        case 30:
                                a[ group.num / 10 - 1 ] = group.d.fnum;
                                break;

                        case 11:                                                                                /* Coordinates */
                        case 21:
                        case 31:
                                b[ group.num / 10 - 1 ] = group.d.fnum;
                                break;
                }
        } while( group.num != 0 );

        line->points = NULL;
        plistAppend( &(line->points), a );
        plistAppend( &(line->points), b );
}



/*
 * ==================================
 * NAME:
 *                      static void
 *                      plistAppend( struct DXFpoint **list, ftype point[3] )
 *
 * DESCRIPTION:
 *                      Append a point to the end of a circular list of points.
 *
 * PARAMETERS:
 *                      list                                    Pointer to the pointer to the list.
 *                      point                           Coordinates of the point to append.
 *
 * RETURNS:
 *                      The list is modified.
 * =================================
 */
static void
#if XVT_CC_PROTO
plistAppend( struct DXFpoint **list, ftype point[3] )
#else
plistAppend( list, point )
struct DXFpoint **list;
ftype point[3];
#endif
{
        struct DXFpoint *item = NULL;

        item = (struct DXFpoint *)xvt_mem_zalloc( sizeof(struct DXFpoint) );
        assert( item != NULL );
        if( *list == NULL )                                                                     /* Empty list */
        {
                item->next = item;
        }
        else
        {
                item->next = (*list)->next;
                (*list)->next = item;
        }
        *list = item;

        memcpy( item->point, point, sizeof(ftype)*3 );
}

/*
 * =======================
 * NAME:
 *                      static void
 *                      plistRemoveHead( struct DXFpoint **queue, ftype point[3] )
 *
 * DESCRIPTION:
 *                      Remove the head from a circular list of points.
 *
 * PRE:
 *                      The list is not empty (*list != NULL).
 *
 * PARAMETERS:
 *                      list                                    The list to use.
 *                      point                           Used to return the coordinates of the point.
 *
 * RETURNS:
 *                      The list is modified.
 *                      <point> is used to return data.
 * =======================
 */
static void
#if XVT_CC_PROTO
plistRemoveHead( struct DXFpoint **list, ftype point[3] )
#else
plistRemoveHead( list, point )
struct DXFpoint **list;
ftype point[3];
#endif
{
        struct DXFpoint *item;

        assert( *list != NULL );
        item = (*list)->next;
        if( item->next == item )
                *list = NULL;
        else
                (*list)->next = item->next;

        memcpy( point, item->point, sizeof(ftype)*3 );

        xvt_mem_free( (char *) item );
}


/*
 * ===================
 * NAME:
 *                      static struct DXFpoint *
 *                      plistJoin( struct DXFpoint *a, struct DXFpoint *b )
 *
 * DESCRIPTION:
 *                      Joins two plists together.      The second list is appended to the first.
 *                      The original pointers to the lists are not valid after calling
 *                      plistJoin, and should be set to NULL.
 *
 * PRE:
 *                      Neither list may be empty.
 *
 * PARAMETERS:
 *                      a, b                                    The lists to join.
 *
 * RETURNS:
 *                      A pointer to the joined list.
 * ===================
 */
static struct DXFpoint *
#if XVT_CC_PROTO
plistJoin( struct DXFpoint *a, struct DXFpoint *b )
#else
plistJoin( a, b )
struct DXFpoint *a;
struct DXFpoint *b;
#endif
{
        struct DXFpoint *headA;                                         /* Heads of the lists */
        struct DXFpoint *headB;                                         /* Note that a,b are the tails */

        assert( a != NULL );
        assert( b != NULL );

        headA = a->next;                                                                        /* Find the heads of each list */
        headB = b->next;

        a->next = headB;                                                                        /* Join */
        b->next = headA;

        return b;                                                                                               /* Return the last element */
}



/*
 * ====================
 * NAME:
 *                      static void
 *                      plistReverse( struct DXFpoint **list )
 *
 * DESCRIPTION:
 *                      Reverse the order of the elements in a plist.
 *
 * PARAMETERS:
 *                      list                            Pointer to the pointer to the list.
 *
 * RETURNS:
 *                      *list is set to point to the reversed list.
 * ====================
 */
static void
#if XVT_CC_PROTO
plistReverse( struct DXFpoint **list )
#else
plistReverse( list )
struct DXFpoint **list;
#endif
{
        struct DXFpoint *oldList;
        struct DXFpoint *newList = NULL;
        struct DXFpoint *temp;
        struct DXFpoint *lastElem;                                      /* Last element in the new list */

        oldList = *list;
        if( oldList != NULL )                                                   /* Beware empty list */
        {
                temp = oldList;                                                                         /* Convert the list to straight form */
                oldList = oldList->next;
                temp->next = NULL;
                lastElem = oldList;                                                     /* The last element on the new list */

                while( oldList != NULL )                                        /* Reverse the elements */
                {
                        temp = oldList;
                        oldList = oldList->next;
                        temp->next = newList;
                        newList = temp;
                }

                lastElem->next = newList;                               /* Convert the list to circular form */
                *list = lastElem;
        }
}


/*
 * ========================
 * NAME:
 *                      static int
 *                      mergeLines( struct DXFline **lines )
 *
 * DESCRIPTION:
 *                      Merges line segments to obtain a single line (if possible).
 *
 * PARAMETERS:
 *                      lines                           Pointer to the pointer to the list of lines.
 *
 * RETURNS:
 *                      *lines is set to point to the merged lines.
 * ========================
 */
static int
#if XVT_CC_PROTO
mergeLines( struct DXFline **lines )
#else
mergeLines( lines )
struct DXFline **lines;
#endif
{
        int progress = 0;                                                       /* For checking for dead ends */
        ftype point[3];                                                                 /* For removing a point from a list */
        struct DXFline *combine;                                /* Combined results */
        struct DXFline *toDo;                                   /* Lines to work on */
        struct DXFline *current;                                /* Current line */
        struct DXFline *prev;                                   /* Previous line */

        assert( lines != NULL );

        combine = *lines;
        if( combine == NULL )                                   /* no lines = No work to do */
                return 1;

        toDo = combine->next;
        if( toDo == NULL )                                                      /* only one line = No work to do */
                return 1;

        do
        {
                progress = 0;
                prev = NULL;                                                                                    /* Find a matching line */
                for( current=toDo; current!=NULL; current=current->next )
                {                                                                                                                               /* Different combinations of head/tail */
                        if( dxfCmpCorner( combine->points->point, current->points->point ) )
                        {
                                plistReverse( &(current->points) );
                                plistRemoveHead( &(current->points), point );
                                combine->points = plistJoin( combine->points, current->points );
                                progress = 1;
                        }
                        else if( dxfCmpCorner( combine->points->next->point,
                                                                                                                 current->points->point ) )
                        {
                                plistRemoveHead( &(combine->points), point );
                                combine->points = plistJoin( current->points, combine->points );
                                progress = 1;
                        }
                        else if( dxfCmpCorner( combine->points->point,
                                                                                                                 current->points->next->point ) )
                        {
                                plistRemoveHead( &(current->points), point );
                                combine->points = plistJoin( combine->points, current->points );
                                progress = 1;
                        }
                        else if( dxfCmpCorner( combine->points->next->point,
                                                                                                                 current->points->next->point ) )
                        {
                                plistReverse( &(current->points) );
                                plistRemoveHead( &(combine->points), point );
                                combine->points = plistJoin( current->points, combine->points );
                                progress = 1;
                        }

                        if( progress )                                                                  /* Remove empty line from the list */
                        {
                                if( prev == NULL )
                                        toDo = current->next;
                                else
                                        prev->next = current->next;
                                xvt_mem_free( (char *) current );
                                break;
                        }

                        prev = current;
                }
        } while( (toDo != NULL) && progress );

        combine->next = toDo;                                   /* Append any remains */
        *lines = combine;
        if( toDo == NULL )
                return 1;
        return 0;
}


/*
 * ====================
 * NAME:
 *                      static void
 *                      leftMost( struct DXFlayer *layer )
 *
 * DESCRIPTION:
 *                      Converts a single line (the result of a successful merge) into
 *                      a list of points, with the leftmost end first.
 *
 * PRE:
 *                      There must be exactly one line in layer->lines.
 *
 * POST:
 *                      layer->points contains the points from layer->lines.
 *                      layer->lines is destroyed and set to NULL.
 *
 * PARAMETERS:
 *                      layer                   The layer to work on.
 * ====================
 */
static void
#if XVT_CC_PROTO
leftMost( struct DXFlayer *layer )
#else
leftMost( layer )
struct DXFlayer *layer;
#endif
{
        struct DXFpoint *points;

        assert( layer != NULL );
        assert( layer->lines != NULL );
        assert( layer->lines->next == NULL );

        points = layer->lines->points;
        xvt_mem_free( (char *) layer->lines );
        layer->lines = NULL;

        if( points->point[0] < points->next->point[0] )                         /* Find the left end */
                plistReverse( &points );
        else if( points->point[0] == points->next->point[0] ) /* If vertical */
        {
                if( points->point[1] > points->next->point[1] )                 /* use the top */
                        plistReverse( &points );
        }

        layer->points = points;
}

/*
 * ============================
 * NAME:
 *                      void
 *                      DXFlineRead( struct DXFhandle *handle, char *filename )
 *
 * DESCRIPTION:
 *                      Controls the reading in of a series of line segments, and their merging
 *                      into one line.
 *
 * PARAMETERS:
 *                      handle                          Pointer to the handle to store data in.
 *                      filename                        Name of the DXF file to read from.
 * ============================
 */
void
#if XVT_CC_PROTO
DXFlineRead( struct DXFhandle *handle, char *filename )
#else
DXFlineRead( handle, filename )
struct DXFhandle *handle;
char *filename;
#endif
{
        FILE *dxfFile = NULL;
        int layerNum;                                                           /* Current layer */
        struct DXFlayer *layer;

        assert( handle != NULL );

        handle->resultFlags = 0;
        handle->layer = NULL;
        handle->numLayers = 0;
        handle->conv = convLine;

        dxfFile = fopen( filename, "rb" );
        if( dxfFile == NULL )
        {
                handle->resultFlags |= fFOPEN;
        }
        else
        {
                handle->layer = readLines( dxfFile, &(handle->numLayers),
                                                                                                                         &(handle->resultFlags) );
                fclose( dxfFile );

                if( handle->layer == NULL )
                        handle->resultFlags |= fNODATA;

                for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
                {
                        layer = handle->layer + layerNum;
                        if( mergeLines( &(layer->lines) ) )
                                leftMost( layer );
                        else
                                handle->resultFlags |= fMERGE;
                }
        }
}


/*
 * =============================
 * NAME:
 *                      void
 *                      DXFlineConv( struct DXFhandle *handle )
 *
 * DESCRIPTION:
 *                      Performs translation, rotation and scaling of the line.  The
 *                      transformations are chosen so that the endpoints of the line are at
 *                      (0,0) and (1,0), and the y values of the points lie within -1 to 1
 *                      inclusive.
 *
 * PARAMETERS:
 *                      handle                          The handle containing the lines.
 * =============================
 */
void
#if XVT_CC_PROTO
DXFlineConv( struct DXFhandle *handle )
#else
DXFlineConv( handle )
struct DXFhandle *handle;
#endif
{
        int layerNum;
        struct DXFlayer *layer;                                 /* Current layer */

        assert( handle->conv == convLine );

        for( layerNum=0; layerNum < handle->numLayers; layerNum++ )
        {                                                                                                                                                                                                       /* for each layer */
                layer = handle->layer + layerNum;
                assert( layer->points != NULL );
                                                                                                                                                                                                 /* Move one end to (0,0) */
                memcpy( layer->offset, layer->points->next->point, sizeof(ftype)*2 );
                doOffset( layer->points, layer->offset );

                                                                                                                                                        /* Put the other end on the x axis */
                layer->rotation = (ftype)atan2( layer->points->point[1],
                                                                                                                                                layer->points->point[0] );
                doRotation( layer->points, -layer->rotation );

                layer->scale[0] = 1 / layer->points->point[0];          /* other end at (1,0) */
                layer->scale[1] = 1 / maxYvalue( layer->points ); /* restrict the range */
                doScale( layer->points, layer->scale );
        }
}


/*
 * ===========================
 * NAME:
 *                      static void
 *                      doRotation( struct DXFpoint *points, ftype rotation )
 *
 * DESCRIPTION:
 *                      Applies a rotation to every point.
 *
 * PARAMETERS:
 *                      points                          Circularly linked list of points.
 *                      rotation                        Angle (in radians) to rotate the points.
 *
 * METHOD:
 *                      Applies a two-dimensional transformation matrix.
 *                      Source: Micrographics Techniques
 * ===========================
 */
static void
#if XVT_CC_PROTO
doRotation( struct DXFpoint *points, ftype rotation )
#else
doRotation( points, rotation )
struct DXFpoint *points;
ftype rotation;
#endif
{
        struct DXFpoint *current;                       /* Current point */
        ftype newPoint[2];                                                      /* Coordinates of the new point */
        ftype sinR;                                                                             /* Sine of the rotation */
        ftype cosR;                                                                             /* Cosine of the rotation */

        if( points != NULL )
        {
                sinR = (ftype)sin( rotation );
                cosR = (ftype)cos( rotation );
                current = points;
                do
                {
                        current = current->next;
                        newPoint[0] = cosR * current->point[0] - sinR * current->point[1];
                        newPoint[1] = sinR * current->point[0] + cosR * current->point[1];
                        memcpy( current->point, newPoint, sizeof(ftype)*2 );
                } while( current != points );
        }
}



/*
 * ===========================
 * NAME:
 *                      static void
 *                      doOffset( struct DXFpoint *points, ftype offset[2] )
 *
 * DESCRIPTION:
 *                      Subtracts an offset from every point.
 *
 * PARAMETERS:
 *                      points                          Circularly linked list of points.
 *                      offset                          Offset to subtract from the points.
 * ===========================
 */
static void
#if XVT_CC_PROTO
doOffset( struct DXFpoint *points, ftype offset[2] )
#else
doOffset( points, offset )
struct DXFpoint *points;
ftype offset[2];
#endif
{
        struct DXFpoint *current;

        if( points != NULL )
        {
                current = points;
                do
                {
                        current = current->next;
                        current->point[0] -= offset[0];
                        current->point[1] -= offset[1];
                } while( current != points );
        }
}


/*
 * ===========================
 * NAME:
 *                      static ftype
 *                      maxYvalue( struct DXFpoint *points )
 *
 * DESCRIPTION:
 *                      Finds the largest (magnitude) Y value of a set of points.
 *
 * PARAMETERS:
 *                      points                          Circularly linked list of points to examine.
 *
 * RETURNS:
 *                      The magnitude of the largest Y value.
 * ===========================
 */
static ftype
#if XVT_CC_PROTO
maxYvalue( struct DXFpoint *points )
#else
maxYvalue( points )
struct DXFpoint *points;
#endif
{
        struct DXFpoint *current;                       /* Current point */
        ftype maxY = (ftype) 0;                                                                 /* Smallest will be 0 */
        ftype curY;                                                                             /* Current absolute Y value */

        if( points != NULL )
        {
                current = points;
                do
                {
                        current = current->next;
                        curY = (ftype)fabs( current->point[1] );
                        if( curY > maxY )
                                maxY = curY;
                } while( current != points );
        }
        return maxY;
}


/*
 * ===========================
 * NAME:
 *                      static void
 *                      doScale( struct DXFpoint *points, ftype scale[2] )
 *
 * DESCRIPTION:
 *                      Applies a scaling to every point.
 *
 * PARAMETERS:
 *                      points                                  Circularly linked list of points.
 *                      scale                                   The scale factors to apply.
 * ===========================
 */
static void
#if XVT_CC_PROTO
doScale( struct DXFpoint *points, ftype scale[2] )
#else
doScale( points, scale )
struct DXFpoint *points;
ftype scale[2];
#endif
{
        struct DXFpoint *current;

        if( points != NULL )
        {
                current = points;
                do
                {
                        current = current->next;
                        current->point[0] *= scale[0];
                        current->point[1] *= scale[1];
                } while( current != points );
        }
}

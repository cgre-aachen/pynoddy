/*
 * =============================================================
 * NAME:
 *            dxfread.c
 *
 * DESCRIPTION:
 *            Contains the routines for reading in a DXF file.   Designed to be
 *            useable by both the surf and vol modules.
 *
 * DXF Files supported:
 *            See "Graphics File Formats" by Kay & Levine.
 *            The only entity types supported are POLYLINEs and 3DFACEs.      Others
 *            will be ignored.
 *
 * AUTHOR:
 *            Ewen Christopher
 *            (c) Ewen Christopher 1995
 *            (c) Monash University 1995
 * =============================================================
 */

#include "xvt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>
#include "dxf.h"
#include "noddy.h"

/******************* Prototypes *******************/
#if XVT_CC_PROTO
static struct DXFlayer *readTriangles( FILE *dxfFile,
            int *numLayers, int *resultFlags );
static struct DXFlayer *storeTriangle( int *numLayers, struct DXFlayer *layer,
            char *layerName, struct DXFtriangle *triangle, int *flags );
static int readFace( FILE *dxfFile, struct DXFtriangle *triangle, char layerName[] );
int matchFourth( ftype three[3][3], ftype fourth[3] );
static int readPoly( FILE *dxfFile, struct DXFtriangle *triangle,
            char layerName[] );
static int dxfReadLine( FILE *infile, char line[] );
static void removeSpaces( char *line );
static int degenerate3D( ftype corner[3][3] );
#else
static struct DXFlayer *readTriangles( );
static struct DXFlayer *storeTriangle( );
static int readFace( );
int matchFourth( );
static int readPoly( );
static int dxfReadLine( );
static void removeSpaces( );
static int degenerate3D( );
#endif

/*
 * =============================================
 * NAME:
 *            void
 *            DXFread( struct DXFhandle *handle, char filename[] )
 *
 * DESCRIPTION:
 *            Read in a DXF file for use by either the surf or vol modules.
 *            Initialises some fields in the handle.
 *
 * PARAMETERS:
 *            handle        Pointer to the handle to use.  Any existing data in the
 *                              handle is ignored.
 *            filename      DXF file to load.
 *
 * RETURNS:
 *            Fields in the handle are used to store data.    In particular, errors are
 *            returned in DXFhandle.resultFlags.
 *
 * CALLED BY:
 *            user program
 * =============================================
 */
void
#if XVT_CC_PROTO
DXFread( struct DXFhandle *handle, char *filename )
#else
DXFread( handle, filename )
struct DXFhandle *handle;
char *filename;
#endif
{
   FILE *dxfFile = NULL;                     /* File to read from */

   handle->resultFlags = 0;
   handle->layer = NULL;
   handle->numLayers = 0;
   handle->conv = convNone;                  /* No conversion so far */

   dxfFile = fopen( filename, "rb" );
   if( dxfFile == NULL )
   {
      handle->resultFlags |= fFOPEN;
   }
   else
   {
      handle->layer = readTriangles( dxfFile, &(handle->numLayers),
                                              &(handle->resultFlags) );
      fclose( dxfFile );

      if( handle->layer == NULL )
         handle->resultFlags |= fNODATA;
   }
}


/*
 * =============================================
 * NAME:
 *            static struct DXFlayer *
 *            readTriangles( FILE *dxfFile,
 *                                  int *numLayers,
 *                                  int *resultFlags )
 *
 * DESCRIPTION:
 *            Reads in triangles as POLYLINEs or as 3DFACEs.  Triangles will be
 *            read from anywhere in the file, not only from the ENTITIES or BLOCKS
 *            section.
 *            The corners of each triangle are stored in both the <corner> and
 *            <origcor> fields.
 *
 * PARAMETERS:
 *            dxfFile             File to read from (must be open).
 *            numLayers             Used to return the number of layers in the file
 *            resultFlags        Any errors or warnings are added to these flags
 *
 * RETURNS:
 *            A pointer to a table of layers is returned.  There will be <numLayers>
 *            entries in the table (numLayers is set by readTriangles).
 *            <resultFlags> will be altered by ORing with any appropriate error codes.
 *
 * NOTE:
 *            Does not check that the entities are in the ENTITIES or BLOCKS sections.
 *
 * CALLED BY:
 *            DXFread
 * =======================================================
 */
static struct DXFlayer *
#if XVT_CC_PROTO
readTriangles( FILE *dxfFile,
               int *numLayers,
               int *resultFlags )
#else
readTriangles( dxfFile, numLayers, resultFlags )
FILE *dxfFile;
int *numLayers;
int *resultFlags;
#endif
{
   struct DXFgroup group;       /* Data read from the DXF file */

   int triRead, i, j;           /* 1 if a triangle was read */
   struct DXFtriangle triangle; /* data for the triangle */
   struct DXFlayer *layer = NULL;
   char layerRead[NAMELEN];   /* name of the layer for the current triangle */

   *numLayers = 0;
   *resultFlags = 0;

   while(1)                /* POLYLINE or 3DFACE */
   {
      dxfReadGroup( dxfFile, &group );
      if( group.num != 0 )      /* Not a section - try again */
      {
         continue;
      }
      else if( strcmp( group.d.text, "EOF" ) == 0 ) /* end of file */
      {
         break;
      }
      else if( strcmp( group.d.text, "POLYLINE" ) == 0 ) /* a triangle */
      {
         triRead = readPoly( dxfFile, &triangle, layerRead );
         if( !triRead )                 /* signal a bad POLYLINE */
            (*resultFlags) |= fBADPOLY;
      }
      else if( strcmp( group.d.text, "3DFACE" ) == 0 )  /* a triangle */
      {
         triRead = readFace( dxfFile, &triangle, layerRead );
         if( !triRead )  
            (*resultFlags) |= fBADFACE; /* a bad 3dface was read */
      }
      else      /* Not interested, so skip */
      {
         continue;
      }

      if( triRead )          /* store the triangle */
      {
         if( !degenerate3D( triangle.corner ) )
         {
            for (i = 0; i < 3; i++)
               for (j = 0; j < 3; j++)
                  triangle.origcor[i][j] = triangle.corner[i][j];
            layer = storeTriangle( numLayers, layer, layerRead,
                                   &triangle, resultFlags );
         }
      }
   }

   return layer;
}




/*
 * ============================================
 * NAME:
 *            struct DXFlayer *
 *            dxfStoreLayer( int *numLayers, struct DXFlayer *layer,
 *                                  char name[], int *flags )
 *
 * DESCRIPTION:
 *            Adds layer <name> to the layer table.  No checking is performed to see
 *            if the layer is already in the layer table.
 *            Performs some initialisation of the new layer.
 *
 * RETURNS:
 *            Pointer to new layer table if successful.
 *            Pointer to old layer table if not successful (out of memory).
 *            Also sets bits in <flags> to indicate error condition.
 *
 * CALLED BY:
 *            storeTriangle
 * ================================================
 */
struct DXFlayer *
#if XVT_CC_PROTO
dxfStoreLayer( int *numLayers, struct DXFlayer *layer,
                      char name[], int *flags )
#else
dxfStoreLayer( numLayers, layer, name, flags )
int *numLayers;
struct DXFlayer *layer;
char name[];
int *flags;
#endif
{
   struct DXFlayer *newLayer;

   newLayer = (struct DXFlayer *)xvt_mem_zalloc( sizeof(struct DXFlayer)
                                                            * (*numLayers + 1) );
   if( newLayer == NULL )           /* out of memory */
   {
      (*flags) |= fMEM;
      newLayer = layer;
   }
   else
   {
      memcpy( newLayer, layer, sizeof(struct DXFlayer) * (*numLayers) );
      if (layer)
         xvt_mem_free( (char *) layer );

      layer = newLayer + (*numLayers);            /* use a shortcut pointer */
      strncpy( layer->layerName, name, NAMELEN );
      layer->layerName[ NAMELEN-1 ] = '\0';

      layer->numTriangles = 0;                        /* Clear empty fields */
      layer->triangles = NULL;
      layer->regions = NULL;

#ifdef DXF_VOL
      layer->regnTag = NULL;
#endif

#ifdef DXF_LINE
      layer->points = NULL;
      layer->lines = NULL;
      layer->offset[0] = layer->offset[1] = (ftype) 0.0;
      layer->rotation = (ftype) 0.0;
      layer->scale[0] = layer->scale[1] = (ftype) 0.0;
#endif

      (*numLayers)++;
   }
   return newLayer;
}



/*
 * =====================================
 * NAME:
 *            static struct DXFlayer *
 *            storeTriangle( int *numLayers, struct DXFlayer *layer,
 *                                  char *layerName,
 *                                  struct DXFtriangle triangle,
 *                                  int *flags )
 *
 * DESCRIPTION:
 *            Allocates memory for a triangle and adds it to the list for the
 *            appropriate layer.      If the layer does not exist, it is created using
 *            dxfStoreLayer.
 *
 * PARAMETERS:
 *            numLayers, layer            the table of layers
 *            layerName                         the layer of this triangle
 *            triangle                        the data for this triangle
 *            flags                           pointer to resultFlags
 *
 * RETURNS:
 *            Pointer to the new layer table
 *
 * CALLED BY:
 *            readTriangles
 * =====================================
 */
static struct DXFlayer *
#if XVT_CC_PROTO
storeTriangle( int *numLayers, struct DXFlayer *layer,
                      char *layerName,
                      struct DXFtriangle *triangle,
                      int *flags )
#else
storeTriangle( numLayers, layer, layerName, triangle, flags )
int *numLayers;
struct DXFlayer *layer;
char *layerName;
struct DXFtriangle *triangle;
int *flags;
#endif
{
   int lnum;                              /* current layer number */
   struct DXFtriangle *newData = NULL;    /* new triangle */

   for( lnum=0; lnum<(*numLayers); lnum++ ) /* find a matching layer */
   {
      if( strcmp( layer[lnum].layerName, layerName ) == 0 )
         break;
   }
   
   if ( lnum >= (*numLayers) )   /* ???? should never be called */
   {
      layer = dxfStoreLayer( numLayers, layer, layerName, flags );
      lnum = *numLayers - 1;
   }
                                                                           /* allocate storeage */
   newData = (struct DXFtriangle  *)xvt_mem_zalloc( sizeof(struct DXFtriangle ) );
   if( newData == NULL )
   {
      (*flags) |= fMEM;         /* signal out of memory */
   }
   else
   {                                          /* store data */
      memcpy (newData, triangle, sizeof (struct DXFtriangle));

#ifdef DXF_VOL
      newData->tag = 0;
#endif

      newData->next = layer[lnum].triangles;  /* prepend to list */
      layer[lnum].triangles = newData;
   }

   return layer;
}



/*
 * ======================
 * NAME:
 *            static int
 *            readFace( FILE *dxfFile, struct DXFtriangle  *triangle, char layerName[] )
 *
 * DESCRIPTION:
 *            Read a 3DFACE and determine if it is a triangle.
 *            Returns 1 if it is a triangle.
 *            Sets layerName to the layer of this entity, or "" if not given.
 *            Positions the file after the 3DFACE's data.
 *
 * CALLED BY:
 *            readTriangles
 * =======================
 */
static int
#if XVT_CC_PROTO
readFace( FILE *dxfFile, struct DXFtriangle *triangle, char layerName[] )
#else
readFace( dxfFile, triangle, layerName )
FILE *dxfFile;
struct DXFtriangle *triangle;
char layerName[];
#endif
{
   long filepos;                     /* need to rewind after reading following group */

   struct DXFgroup group;        /* Data from the file */
   int result = 0;                   /* 1 if a triangle was read */

   ftype corner4[3];                   /* data for 4th corner */
   int has4 = 0;                     /* the 4th corner was given */

   layerName[0] = '\0';   /* default layer name */

   filepos = ftell( dxfFile );
   dxfReadGroup( dxfFile, &group );
   while( group.num != 0 )
   {
      if( group.num >=10 && group.num < 38 )     /* coordinate groups */
      {
         if( group.num % 10 == 3 )            /* handle 4th corner separately */
         {
            has4 = 1;
            corner4[ group.num/10 - 1 ] = group.d.fnum;
         }
         else
         {
            triangle->corner[ group.num%10 ][ group.num/10 - 1 ] = group.d.fnum;
         }
      }
      else if( group.num == 8 )                     /* layer name */
      {
         strncpy( layerName, group.d.text, NAMELEN );
         layerName[NAMELEN-1] = '\0';
      }

      result = 1;                       /* some data has been read */
      filepos = ftell( dxfFile );
      dxfReadGroup( dxfFile, &group );
   }
   fseek( dxfFile, filepos, SEEK_SET );

   if( has4 )                     /* some data was read, check 4th corner */
   {
      result = matchFourth( triangle->corner, corner4 );
   }

   return result;
}


/*
 * ===============================
 * NAME:
 *            int
 *            matchFourth( ftype three[3][3], ftype fourth[3] )
 *            matchFourth
 *
 * DESCRIPTION:
 *            Reduces 4 points to 3 by attempting to match the fourth with one of the
 *            other points.
 *
 * RETURNS:
 *            Non-zero if the fourth corner can be matched, 0 otherwise.
 *
 * CALLED BY:
 *            readFace
 *            readPoly
 * ================================
 */
int
#if XVT_CC_PROTO
matchFourth( ftype three[3][3], ftype fourth[3] )
#else
matchFourth( three, fourth )
ftype three[3][3];
ftype fourth[3];
#endif
{
   int i, j;
   int match;

   for( i=2; i>=0; i-- )            /* most likely to match with third corner */
   {
      match = 1;
      for( j=0; j<3; j++ )
      {
         if( three[i][j] != fourth[j] )
         {
            match = 0;
            break;
         }
      }
      if( match )
         break;
   }
   return match;
}



/*
 * ======================
 * NAME:
 *            static int
 *            readPoly( FILE *dxfFile, struct DXFtriangle  *triangle, char layerName[] )
 *
 * DESCRIPTION:
 *            Read a POLYLINE and determine if it is a triangle.
 *            Returns 1 if it is a triangle.
 *            Sets layerName to the layer of this entity, or "" if not given.
 *            Positions the file after the POLYLINE's data.
 *
 * CALLED BY:
 *            readTriangles
 * =======================
 */
static int
#if XVT_CC_PROTO
readPoly( FILE *dxfFile, struct DXFtriangle  *triangle, char layerName[] )
#else
readPoly( dxfFile, triangle, layerName )
FILE *dxfFile;
struct DXFtriangle  *triangle;
char layerName[];
#endif
{
   struct DXFgroup group;        /* Data read from the file */

   ftype corner4[3];                   /* beware a degenerate corner */

   int twoDim = 0;                   /* 1 if it is a 2D polyline */
   ftype offset[3];
   ftype extrusion = (ftype) 0.0;             /* extrusion (38) of a 2D polyline */
   ftype Ze[3];        /* the Z axis vector, groups 210, 220, 230 */
   ftype Xe[3], Ye[3];                       /* ECS axis */
   ftype temp[3], sum[3];
   
   int result = 1;                   /* 1 if a triangle was read */
   int cornerNum = -1;              /* index of current corner */

   offset[0] = offset[1] = offset[2] = (ftype) 0.0;
   Ze[0] = Ze[1] = Ze[2] = (ftype) 0.0;

   layerName[0] = '\0';   /* default - no layer name */

   triangle->corner[0][2] = triangle->corner[1][2]    /* zero Z coords */
                                       = triangle->corner[2][2] = (ftype) 0.0;

   dxfReadGroup( dxfFile, &group );
   while( (group.num != 0) || (strcmp( group.d.text, "SEQEND") != 0) )
   {
      if( (group.num == 0) && (strcmp( group.d.text, "VERTEX" ) == 0) )
      {                                                    /* next corner */
         cornerNum++;
      }
      else if( cornerNum == -1 )               /* processing for the POLYLINE only */
      {
         switch( group.num )
         {
            case 8:
               strncpy( layerName, group.d.text, NAMELEN );
               layerName[NAMELEN-1] = '\0';
               break;

            case 10:                                 /* offset of ECS from WCS origin?? */
            case 20:
            case 30:
               offset[group.num / 10 - 1] = group.d.fnum;
               break;

            case 38:                                 /* extrusion along Ze axis */
               extrusion = group.d.fnum;
               break;

            case 70:                                 /* flags */
               if( !(group.d.inum & 1) )         /* closed polyline */
                  result = 0;
               if( !(group.d.inum & 8) )         /* 2D polyline */
                  twoDim = 1;
               break;

            case 210:
               Ze[0] = group.d.fnum;
               break;

            case 220:
               Ze[1] = group.d.fnum;
               break;

            case 230:
               Ze[2] = group.d.fnum;
               break;
         }
      }
      else if(cornerNum < 3)                                /* coords of a corner */
      {
         if( (group.num==10) || (group.num==20) || (group.num==30) )
         {
            triangle->corner[ cornerNum ][ group.num/10 - 1 ] = group.d.fnum;
         }
      }
      else if(cornerNum == 3)                                /* fourth corner */
      {
         if( (group.num==10) || (group.num==20) || (group.num==30) )
         {
            corner4[ group.num/10 - 1 ] = group.d.fnum;
         }
      }

      dxfReadGroup( dxfFile, &group );
   }

   if( cornerNum == 3 )                      /* handle fourth corner */
   {
      result = matchFourth( triangle->corner, corner4 );
   }
   else if( cornerNum != 2 )            /* need 3 corners for a triangle */
      result = 0;
   else if( twoDim  )                      /* translate to 3 dimensions */
   {
      dxfArbitraryAxis( Xe, Ye, Ze );

                                                            /* translate corners */
      for( cornerNum=0; cornerNum<3; cornerNum++ )
      {
         sum[0] = sum[1] = sum[2] = (ftype) 0.0;

         dxfScalarProduct( temp, triangle->corner[cornerNum][0], Xe );
         dxfAddVector( sum, temp );

         dxfScalarProduct( temp, triangle->corner[cornerNum][1], Ye );
         dxfAddVector( sum, temp );

         dxfScalarProduct( temp, triangle->corner[cornerNum][2], Ze );
         dxfAddVector( sum, temp );

         memcpy( triangle->corner[cornerNum], sum, sizeof(ftype)*3 );
      }
   }
   return result;
}

/*
 * ==========================
 * NAME:
 *            void
 *            dxfArbitraryAxis( ftype x[3], ftype y[3], ftype z[3] )
 *
 * DESCRIPTION:
 *            Apply AutoCAD's Arbitrary Axis Algorithm to a Z vector.
 *
 * PARAMETERS:
 *            Vectors representing the x, y & z axis of the new coordinate system.
 *            Note that the algorithm only uses the z axis.
 *
 * RETURNS:
 *            Modified x, y & z axis.
 * ==========================
 */
void
#if XVT_CC_PROTO
dxfArbitraryAxis( ftype x[3], ftype y[3], ftype z[3] )
#else
dxfArbitraryAxis( x, y, z )
ftype x[3], y[3], z[3];
#endif
{
   ftype arb[3];               /* the arbitrary axis */

   dxfUnitVector( z );

   arb[0] = arb[1] = arb[2] = (ftype) 0.0;    /* find Xl - arbitrary axis method */
   if( (fabs( z[0] ) < 1.0/64) && (fabs( z[1] ) < 1.0/64) )
      arb[1] = (ftype) 1;
   else
      arb[2] = (ftype) 1;
   dxfCrossProduct( x, arb, z );
   dxfUnitVector( x );

   dxfCrossProduct( y, z, x );         /* Right hand rule: Y = Z x X */
   dxfUnitVector( y );
}



/*
 * =============================
 * NAME:
 *            void
 *            dxfCrossProduct( ftype result[3], ftype a[3], ftype b[3] )
 *
 * DESCRIPTION:
 *            Calculates the cross product of two vectors: result = a X b
 * =============================
 */
void
#if XVT_CC_PROTO
dxfCrossProduct( ftype result[3], ftype a[3], ftype b[3] )
#else
dxfCrossProduct( result, a, b )
ftype result[3], a[3], b[3];
#endif
{
   result[0] = a[1]*b[2] - a[2]*b[1];
   result[1] = a[2]*b[0] - a[0]*b[2];
   result[2] = a[0]*b[1] - a[1]*b[0];
}


/*
 * ==========================
 * NAME:
 *            void
 *            dxfScalarProduct( ftype result[3], ftype k, ftype a[3] )
 *
 * DESCRIPTION:
 *            Calculates the scalar product: result = kA
 * ==========================
 */
void
#if XVT_CC_PROTO
dxfScalarProduct( ftype result[3], ftype k, ftype a[3] )
#else
dxfScalarProduct( result, k, a )
ftype result[3];
ftype k;
ftype a[3];
#endif
{
   result[0] = k * a[0];
   result[1] = k * a[1];
   result[2] = k * a[2];
}


/*
 * ==========================
 * NAME:
 *            void
 *            dxfAddVector( ftype a[3], ftype b[3] )
 *
 * DESCRIPTION:
 *            Adds two vectors:  a' = a + b
 * ==========================
 */
void
#if XVT_CC_PROTO
dxfAddVector( ftype a[3], ftype b[3] )
#else
dxfAddVector( a, b )
ftype a[3];
ftype b[3];
#endif
{
   a[0] += b[0];
   a[1] += b[1];
   a[2] += b[2];
}


/*
 * ======================================
 * NAME:
 *            void
 *            dxfUnitVector( ftype v[3] )
 *
 * DESCRIPTION:
 *            Scales a vector to make it a unit vector in the same direction.
 * ======================================
 */
void
#if XVT_CC_PROTO
dxfUnitVector( ftype v[3] )
#else
dxfUnitVector( v )
ftype v[3];
#endif
{
   ftype magnitude;

   magnitude = (ftype)sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
   v[0] /= magnitude;
   v[1] /= magnitude;
   v[2] /= magnitude;
}


/*
 * ======================================
 * NAME:
 *            void
 *            dxfReadGroup( FILE *dxfFile, struct DXFgroup *group )
 *
 * DESCRIPTION:
 *            Read in a group.   Returns the group number and sets groupText to
 *            the textual representation of the group data.
 *            If end of file is encountered, sets the group to 0,EOF
 *
 * PARAMETERS:
 *            dxfFile          File to read from
 *            group            Used to return data.
 *
 * RETURNS:
 *            The group number of the group is returned using group.num.
 *            Only one of group.d.text, group.d.inum or group.d.fnum will be valid,
 *            depending on the group number.
 *
 * CALLED BY:
 *            readLayers
 *            readTriangles
 *            readFace
 *            readPoly
 * ======================================
 */
void
#if XVT_CC_PROTO
dxfReadGroup( FILE *dxfFile, struct DXFgroup *group )
#else
dxfReadGroup( dxfFile, group )
FILE *dxfFile;
struct DXFgroup *group;
#endif
{
   int result;

   fscanf( dxfFile, "%i", &(group->num) );
   if( (group->num < 10) || (group->num == 999) )
   {
      result = dxfReadLine( dxfFile, group->d.text );
      removeSpaces( group->d.text );
   }
   else if( (group->num >= 60) && (group->num <= 79) )
   {
      result = fscanf( dxfFile, "%i", &(group->d.inum) );
   }
   else
   {
      result = fscanf( dxfFile, "%f", &(group->d.fnum) );
   }

   if( result == 0 )                   /* end of file */
   {
      group->num = 0;
      strcpy( group->d.text, "EOF" );
   }
}


/*
 * ==================================
 * NAME:
 *            static int
 *            dxfReadLine( FILE *infile, char line[] )
 *
 * DESCRIPTION:
 *            Reads in a line of text terminated with any combination of \n \r
 *            Will not return empty lines.
 *            Does not return \r or \n at the end of a line.
 *            Returns 0 on error (probably end of file)
 *
 * WHY:
 *            This routine is needed because tdxflibc uses \r to terminate its lines,
 *            which may not be picked up by fgets.
 *
 * CALLED BY:
 *            dxfReadGroup
 * ==================================
 */
static int
#if XVT_CC_PROTO
dxfReadLine( FILE *infile, char line[] )
#else
dxfReadLine( infile, line )
FILE *infile;
char line[];
#endif
{
   int ch = EOF;
   int pos = 0;

   while( !feof(infile) )
   {
      ch = getc( infile );
      if( ch==EOF )                        /* exit on end of file */
         break;

      if( (ch=='\n') || (ch=='\r') )
      {
         if( pos == 0 )              /* avoid blank lines */
            continue;
         else
            break;                       /* reached end of line */
      }
      else
      {
         line[pos] = (char)ch;
         pos++;
      }
   }

   line[pos] = '\0';        /* terminate line */
   if( feof(infile) || ch==EOF )
      return 0;
   else
      return 1;
}


/*
 * =======================================
 * NAME:
 *            static void
 *            removeSpaces( char *line )
 *
 * DESCRIPTION:
 *            Removes trailing and leading white-space from a string
 *
 * PURPOSE:
 *            To enable DXF names such as "ENTITIES" to be recognised even though
 *            they may be surrounded by varying types and amounts of white space,
 *            depending on the program that created them.
 *
 * CALLED BY:
 *            dxfReadGroup
 * =======================================
 */
static void
#if XVT_CC_PROTO
removeSpaces( char *line )
#else
removeSpaces( line )
char *line;
#endif
{
   int readPos;                               /* where to read from in string */
   int writePos = 0;                         /* where to write to in string */
   int skipping = 1;                         /* have only encountered white-space so far */

   readPos = strlen( line ) - 1;   /* remove trailing space first */
   for( ; readPos>=0; readPos-- )
   {
      if( isspace( line[readPos] ) )
         line[readPos] = '\0';
      else
         break;
   }

   for( readPos=0; line[readPos]!='\0'; readPos++ )
   {
      if( skipping && !isspace( line[readPos] ) )     /* first non-space */
      {
         skipping = 0;                                             /* next "if" will also run */
      }

      if( !skipping )                                              /* now in copying mode */
      {
         line[writePos] = line[readPos];
         writePos++;
      }
   }
   line[writePos] = '\0';           /* terminate string */
}



/*
 * ==========================
 * NAME:
 *            static int
 *            degenerate3D( ftype corner[3][3] )
 *
 * DESCRIPTION:
 *            Determines if a triangle is degenerate in a 3 dimensional sense.
 *
 * RETURNS:
 *            Non-zero if the triangle is degenerate.
 *
 * METHOD:
 *            If the triangle is degenerate, it encloses no area.  This routine
 *            finds the cross product of two of the edges.    If this is zero, the
 *            triangle is degenerate.
 * =========================
 */
static int
#if XVT_CC_PROTO
degenerate3D( ftype corner[3][3] )
#else
degenerate3D( corner )
ftype corner[3][3];
#endif
{
   ftype vec1[3], vec2[3];      /* vectors of two edges */
   ftype cross[3];              /* cross product */
   ftype magnitude2;            /* square of magnitude */
   int i;

   for( i=0; i<3; i++ )         /* find Z axis from points of triangle */
   {
      vec1[i] = corner[1][i] - corner[0][i];
      vec2[i] = corner[2][i] - corner[0][i];
   }
   dxfCrossProduct( cross, vec1, vec2 );

   magnitude2 = cross[0]*cross[0] + cross[1]*cross[1] + cross[2]*cross[2];

   if( magnitude2 == 0 )
      return 1;
   return 0;
}

/* **************************************************************************
** After dividing a voxel up into 5 tetrathedra, each one is processed in turn
** based on its tetcode, which depends on the number of different discontinuity
** codes in a tetrahedron. These codes are defined as:
**
**
**          A             A            A             A             A          
**        / |\          / |\         / |\          / |\          / |\      
**       /  | \        /  | \       /  | \        /  | \        /  | \      
**      A ..|..A      A ..|..A     B ..|..C      B ..|..B      B ..|..C      
**       \  | /        \  | /       \  | /        \  | /        \  | /      
**        \ |/          \ |/         \ |/          \ |/          \ |/         
**          A             B            A             A             D       
**
**      Alpha         Beta         Gamma         Delta        Epsilon
**
** Alpha Code: all discontinuity codes the same
**             therefore any given level can be contoured by 1 flat surface
**
** Beta Code:  one different, so a level can only intersect two A-A edges,
**             or occasionally a plane can intersect all three As. For former
**             case the line is projected onto a triangular plane that
**             separate AAA from B  and we then have to cut up this triangle
**             into bits.
**
** Gamma Code: two different, so  a level can only intersect one A-A
**             veredgetex, or occasionally a plane can intersect all two As.
**             For former case the point is projected onto a complex plane
**             that separate AA from B and C and we then have to cut up this
**             plane into bits.
**
** Delta Code: 2 sets of two different, so  a level can only intersect one
**             A-A edge, or one B-B edge, or occasionally a plane can
**             intersect all two As or Bs In the former case the point is
**             projected onto a flat plane that separate AA from BB and we
**             then have to cut up this trapezoidal plane into bits.
**
** Epsilon Code: all codes different, so complex planes separate A,B,C and D
**
**************************************************************************** */
#include "xvt.h"
#include "noddy.h"
#include "nodInc.h"
#include "3d.h"
#include "3dExtern.h"
#include "3dDraw.h"
#include "allSurf.h"
#include <math.h>
#if (XVTWS != MACWS)
#include <malloc.h>
#endif

#define DEBUG(X)  

                 /* ********************************** */
                 /* External Globals used in this file */
#if XVT_CC_PROTO
extern int cypherDiff(int, int, unsigned char **);
extern int lastdiff (unsigned char *, unsigned char *);
extern int dxopen(char *);
extern void setDxfFile (FILE *);
extern int dxclose();
extern void AlphaCode(double [8], double [8][3], int [8], TETINFO *);
extern void BetaCode(double [8], double [8][3], int [8], TETINFO *);
extern void GammaCode(double [8], double [8][3], int [8], TETINFO *);
extern void DeltaCode(double [8], double [8][3], int [8], TETINFO *);
extern void EpsilonCode(double [8][3], int [8], TETINFO *);
extern void qSortPoints(THREED_POINT_INFO __huge *, int, int, int);
extern int dxfa3l(float *, float *, float *);
extern int dxpl3l(float *,float *,float *,int,int);
extern FILE *getDxfFile ();
int allSetVerts(STORY ***, double ***, double [8][3], double [8], int [8], 
            int, int, int, double, int, unsigned char **);
int allDisplayPlane(double, int, int, int);
int Skinny(double [4][3]);
int tooLong(double [4][3]);
#else
extern int cypherDiff();
extern int lastdiff ();
extern int dxopen();
extern void setDxfFile ();
extern int dxclose();
extern void AlphaCode();
extern void BetaCode();
extern void GammaCode();
extern void DeltaCode();
extern void EpsilonCode();
extern void qSortPoints();
extern int dxfa3l();
extern int dxpl3l();
extern FILE *getDxfFile ();
int allSetVerts();
int allDisplayPlane();
int Skinny();
int tooLong();
#endif

extern long connect;

extern THREED_VIEW_OPTIONS threedViewOptions;
extern PROJECT_OPTIONS projectOptions;
extern Point3d *cameraPos;
extern COLOR backgroundColor;

#define ALL_SURF_POINT_STORE 400*3

COLOR layerColor; /* global color of current layer */
char clayer[81]; /* global array for current layer name */
static int noTri; /* number of triangles */
static THREED_POINT_INFO __huge *alltri = (THREED_POINT_INFO __huge *) NULL;
static THREED_IMAGE_DATA *threedDataPtr;
long vertexcount;
int **CUBE = NULL;
int **TETAPICES = NULL;
int **TETLINES = NULL;
int **LINES = NULL;
int *eventsForStratLayers;
int numEventsForStratLayers;
static double plotScale;

#if XVT_CC_PROTO
static int allSurfStart ();
static int allSurfFinish ();
void SetTet(int, int, TETINFO *);
void allOneTetrahedron(double [8], double [8][3], int [8], TETINFO *, unsigned char **);
double MidVal(double, double, double);
int CalcTetCode(int, int, int, int, unsigned char **);
void oneDisplayPlane(double [4][3],int);
void ShadeXYZ(double [4][3], COLOR *, double, double, double);
void ShadeWhite(COLOR *);
void ShadeLambert(double [4][3], COLOR *);
double determinant(double [4][3], int);
LAYER_PROPERTIES *renderLayer (OBJECT *, int);
void WriteVulcanHeader(char *);
void add_underscore(char *, char *);
#else
static int allSurfStart ();
static int allSurfFinish ();
void SetTet();
void allOneTetrahedron();
double MidVal();
int CalcTetCode();
void oneDisplayPlane();
void ShadeXYZ();
void ShadeWhite();
void ShadeLambert();
double determinant();
LAYER_PROPERTIES *renderLayer ();
void WriteVulcanHeader ();
void add_underscore();
#endif

static int allSurfStart ()
{
   noTri = 0;
   vertexcount = 0;
                           /*coordinates of vertices of cube */
   if (!(CUBE = (int **) create2DArray (8, 3, sizeof(int))))
      return (FALSE);
   CUBE[0][0]= 0; CUBE[0][1] = 0; CUBE[0][2] = 0; /* 0 A */
   CUBE[1][0]= 1; CUBE[1][1] = 0; CUBE[1][2] = 0; /* 1 B */
   CUBE[2][0]= 0; CUBE[2][1] = 1; CUBE[2][2] = 0; /* 2 C */
   CUBE[3][0]= 1; CUBE[3][1] = 1; CUBE[3][2] = 0; /* 3 D */
   CUBE[4][0]= 0; CUBE[4][1] = 0; CUBE[4][2] = 1; /* 4 E */
   CUBE[5][0]= 1; CUBE[5][1] = 0; CUBE[5][2] = 1; /* 5 F */
   CUBE[6][0]= 0; CUBE[6][1] = 1; CUBE[6][2] = 1; /* 6 G */
   CUBE[7][0]= 1; CUBE[7][1] = 1; CUBE[7][2] = 1; /* 7 H */

                                   /* Apices of each tetrahedra */
   if (!(TETAPICES = (int **) create2DArray (10, 4, sizeof(int))))
   {
      allSurfFinish ();
      return (FALSE);
   }
   TETAPICES[0][0] = 3; TETAPICES[0][1] = 5; TETAPICES[0][2] = 6; TETAPICES[0][3] = 7; /* 0 V */
   TETAPICES[1][0] = 0; TETAPICES[1][1] = 4; TETAPICES[1][2] = 5; TETAPICES[1][3] = 6; /* 1 W */
   TETAPICES[2][0] = 0; TETAPICES[2][1] = 2; TETAPICES[2][2] = 3; TETAPICES[2][3] = 6; /* 2 X */
   TETAPICES[3][0] = 0; TETAPICES[3][1] = 1; TETAPICES[3][2] = 3; TETAPICES[3][3] = 5; /* 3 Y */
   TETAPICES[4][0] = 0; TETAPICES[4][1] = 3; TETAPICES[4][2] = 5; TETAPICES[4][3] = 6; /* 4 Z */
                                             /*repeat for 2nd cube */
   TETAPICES[5][0] = 0; TETAPICES[5][1] = 1; TETAPICES[5][2] = 2; TETAPICES[5][3] = 4; /* 5 V */
   TETAPICES[6][0] = 1; TETAPICES[6][1] = 4; TETAPICES[6][2] = 5; TETAPICES[6][3] = 7; /* 6 W */
   TETAPICES[7][0] = 1; TETAPICES[7][1] = 2; TETAPICES[7][2] = 3; TETAPICES[7][3] = 7; /* 7 X */
   TETAPICES[8][0] = 2; TETAPICES[8][1] = 4; TETAPICES[8][2] = 6; TETAPICES[8][3] = 7; /* 8 Y */
   TETAPICES[9][0] = 1; TETAPICES[9][1] = 2; TETAPICES[9][2] = 4; TETAPICES[9][3] = 7; /* 9 Z */

                                      /* edges belonging to each tetrahedra */
   if (!(TETLINES = (int **) create2DArray (10, 6, sizeof(int))))
   {
      allSurfFinish ();
      return (FALSE);
   }
   TETLINES[0][0] = 17; TETLINES[0][1] = 13; TETLINES[0][2] = 12; TETLINES[0][3] =  9; TETLINES[0][4] =  5; TETLINES[0][5] =  6; /* 0 V */
   TETLINES[1][0] = 16; TETLINES[1][1] = 15; TETLINES[1][2] = 12; TETLINES[1][3] =  8; TETLINES[1][4] =  7; TETLINES[1][5] = 11; /* 1 W */
   TETLINES[2][0] = 15; TETLINES[2][1] = 13; TETLINES[2][2] = 14; TETLINES[2][3] = 10; TETLINES[2][4] =  2; TETLINES[2][5] =  3; /* 2 X */
   TETLINES[3][0] = 14; TETLINES[3][1] = 16; TETLINES[3][2] = 17; TETLINES[3][3] =  0; TETLINES[3][4] =  4; TETLINES[3][5] =  1; /* 3 Y */
   TETLINES[4][0] = 14; TETLINES[4][1] = 15; TETLINES[4][2] = 13; TETLINES[4][3] = 12; TETLINES[4][4] = 17; TETLINES[4][5] = 16; /* 4 Z */
                                               /* repeat for 2nd cube */
   TETLINES[5][0] = 18; TETLINES[5][1] = 29; TETLINES[5][2] = 21; TETLINES[5][3] = 31; TETLINES[5][4] = 30; TETLINES[5][5] = 34; /* 5 V */
   TETLINES[6][0] = 22; TETLINES[6][1] = 25; TETLINES[6][2] = 24; TETLINES[6][3] = 31; TETLINES[6][4] = 32; TETLINES[6][5] = 35; /* 6 W */
   TETLINES[7][0] = 19; TETLINES[7][1] = 20; TETLINES[7][2] = 23; TETLINES[7][3] = 30; TETLINES[7][4] = 32; TETLINES[7][5] = 33; /* 7 X */
   TETLINES[8][0] = 26; TETLINES[8][1] = 27; TETLINES[8][2] = 28; TETLINES[8][3] = 35; TETLINES[8][4] = 34; TETLINES[8][5] = 33; /* 8 Y */
   TETLINES[9][0] = 30; TETLINES[9][1] = 31; TETLINES[9][2] = 32; TETLINES[9][3] = 33; TETLINES[9][4] = 34; TETLINES[9][5] = 35; /* 9 Z */

                                     /* Edges of all tetrahedra */
   if (!(LINES = (int **) create2DArray (36, 2, sizeof(int))))
   {
      allSurfFinish ();
      return (FALSE);
   }
   LINES[0][0] = 0; LINES[0][1] = 1;   /* 0  AB */
   LINES[1][0] = 1; LINES[1][1] = 3;    /* 1  BD */
   LINES[2][0] = 2; LINES[2][1] = 3;    /* 2  CD */
   LINES[3][0] = 0; LINES[3][1] = 2;    /* 3  AC */
   LINES[4][0] = 1; LINES[4][1] = 5;    /* 4  BF */
   LINES[5][0] = 3; LINES[5][1] = 7;    /* 5  DH */
   LINES[6][0] = 5; LINES[6][1] = 7;    /* 6  FH */
   LINES[7][0] = 4; LINES[7][1] = 5;    /* 7  EF */
   LINES[8][0] = 4; LINES[8][1] = 6;    /* 8  EG */
   LINES[9][0] = 6; LINES[9][1] = 7;    /* 9  GH */
   LINES[10][0]= 2; LINES[10][1]= 6;    /* 10 CG */
   LINES[11][0]= 0; LINES[11][1]= 4;    /* 11 AE */
   LINES[12][0]= 5; LINES[12][1]= 6;    /* 12 FG */
   LINES[13][0]= 3; LINES[13][1]= 6;    /* 13 DG */
   LINES[14][0]= 0; LINES[14][1]= 3;    /* 14 AD */
   LINES[15][0]= 0; LINES[15][1]= 6;    /* 15 AG */
   LINES[16][0]= 0; LINES[16][1]= 5;    /* 16 AF */
   LINES[17][0]= 3; LINES[17][1]= 5;   /* 17 DF */     
                               /*repeat for 2nd cube orientation*/
   LINES[18][0]= 0; LINES[18][1]= 1;   /* 0  AB */
   LINES[19][0]= 1; LINES[19][1]= 3;   /* 1  BD */
   LINES[20][0]= 2; LINES[20][1]= 3;   /* 2  CD */
   LINES[21][0]= 0; LINES[21][1]= 2;   /* 3  AC */
   LINES[22][0]= 1; LINES[22][1]= 5;   /* 4  BF */
   LINES[23][0]= 3; LINES[23][1]= 7;    /* 5  DH */
   LINES[24][0]= 5; LINES[24][1]= 7;    /* 6  FH */
   LINES[25][0]= 4; LINES[25][1]= 5;    /* 7  EF */
   LINES[26][0]= 4; LINES[26][1]= 6;    /* 8  EG */
   LINES[27][0]= 6; LINES[27][1]= 7;    /* 9  GH */
   LINES[28][0]= 2; LINES[28][1]= 6;    /* 10 CG */
   LINES[29][0]= 0; LINES[29][1]= 4;    /* 11 AE */
   LINES[30][0]= 1; LINES[30][1]= 2;    /* 12 BC */
   LINES[31][0]= 1; LINES[31][1]= 4;    /* 13 BE */
   LINES[32][0]= 1; LINES[32][1]= 7;    /* 14 BH */
   LINES[33][0]= 2; LINES[33][1]= 7;    /* 15 CH */
   LINES[34][0]= 2; LINES[34][1]= 4;    /* 16 CE */
   LINES[35][0]= 4; LINES[35][1]= 7;    /* 17 EH */

   return (TRUE);
}

static int allSurfFinish ()
{
   if (CUBE)
      destroy2DArray ((char **) CUBE, 8, 3);
   CUBE = NULL;
   
   if (TETAPICES)
      destroy2DArray ((char **) TETAPICES, 10, 4); 
   TETAPICES = NULL;
   
   if (TETLINES)   
      destroy2DArray ((char **) TETLINES, 10, 6);
   TETLINES = NULL;
   
   if (LINES) 
      destroy2DArray ((char **) LINES, 36, 2);
   LINES = NULL;
      
   return (TRUE);
}

/* discretise voxel volume into surfaces for each stratigraphic horizon and
** fault, igneous & uc break */
void
#if XVT_CC_PROTO
allSurface(THREED_IMAGE_DATA *threedData, STORY ***dots3DC, double ***map3D,
           int nx, int ny, int nz, double plotscale, char *filename)
#else
allSurface(threedData, dots3DC, map3D, nx, ny, nz, plotscale, filename)
THREED_IMAGE_DATA *threedData;
STORY ***dots3DC;
double ***map3D;
int nx, ny, nz;
double plotscale;
char *filename;
#endif
{
   FILE *vulcanFile = NULL;
   int cyphno;
   unsigned char **cypher = NULL;
   int ii, jj, kk, mm, trot,nn;
   int SeqCode[8];
   double Points[8][3],  /* the xyz values of points being controured around */
          Values[8];     /* the height of the points being contoured around (after unevents) */
   int eventsForStrat[50]; 
   TETINFO t;
   int x, y, z, index, found;
   OBJECT *object;


   plotScale = plotscale;
   threedDataPtr = threedData;
   cypher = (unsigned char **) create2DArray (5000, ARRAY_LENGTH_OF_STRAT_CODE,
                                              sizeof(unsigned char)); // for really complex models 5000 contiguous volumes may not be enough?
   if (!cypher)
      return;
   if (!allSurfStart ())
      return;

   numEventsForStratLayers = assignEventNumForStratLayers (eventsForStrat, 50);
   eventsForStratLayers = &(eventsForStrat[0]);
                  /* Add the Fault break planes at the end */
   for (ii = 0; ii < countObjects(NULL_WIN); ii++)
   {
      object = nthObject (NULL_WIN, ii);
      if (object->shape == FAULT)
      {
         eventsForStrat[numEventsForStratLayers] = ii;
         numEventsForStratLayers++;
      }
   }
   
   if ((threedViewOptions.fillType == DXF_FILE_FILL_3DFACE) ||
       (threedViewOptions.fillType == DXF_FILE_FILL_POLYLINE))
   {
      if (!dxopen (filename))
      {
         xvt_dm_post_error ("Error, Could not open DXF File");
         return;
      }
   }
   if (threedViewOptions.fillType == VULCAN_FILE_FILL)
   {
      if (!(vulcanFile = (FILE *) fopen (filename, "w")))
      {
         xvt_dm_post_error ("Error, Could not open Vulcan File");
         return;
      }
      setDxfFile (vulcanFile);
   }


                   /* work out how many different discontinuity codes are in
                   ** the volume */
   iequal(cypher[1], dots3DC[1][1][1].sequence);
   cyphno = 1;
   for (z = 1; z <= nz; z++) 
   {
      for (y = 1; y <= ny; y++) 
      {                                                             
         for (x = 1; x <= nx; x++) 
         {                                                          
            found = FALSE;
            for (index = 1; index <= cyphno; index++) 
            {
               if (coincide(dots3DC[z][y][x].sequence, &(cypher[index][0])))
               {
                  found = TRUE;
                  break;
               }
            }
            if (!found) 
            {                                                  
               cyphno++;
               iequal(cypher[cyphno],dots3DC[z][y][x].sequence);// mwj_debug
               //iequal(&(cypher[cyphno][0]),dots3DC[z][y][x].sequence);// mwj_debug
            }                                                               
         }
      } 
   }

   vertexcount=1;

                  /* take a copy of the cypher */
   t.cyphno=cyphno;
   for (mm = 1; mm <= cyphno; mm++)
      iequal(&(t.cypher[mm][0]), &(cypher[mm][0]));
      
   for (ii = nx-1; ii >= 1; ii--)
   {
      for (jj = ny-1; jj >= 1; jj--)
      {                    /* perform calcs from back corner */
         if (threedData && threedData->declination < 180)
            x = nx - ii;
         else
            x = ii;
                           /* perform calcs from back corner */
         if (threedData && (threedData->declination > 90) &&
                           (threedData->declination < 270))
            y = jj;
         else
            y = ny - jj;
         
         incrementLongJob (INCREMENT_JOB);
         for (kk = 1; kk < nz; kk++)
         {
                             /* code for even/odd cubes */
            trot=((((x+y)%2)+(kk%2))%2);

            allSetVerts(dots3DC, map3D, Points, Values, SeqCode,
                        x, y, kk, plotscale, cyphno, cypher);
            noTri=0;

            for (mm = 0; mm < 5; mm++)
            {             /* Clear the 't' structure */
               SetTet(mm, trot, &t); /* store some useful info */
                                                   /* process one tet */
               allOneTetrahedron (Values, Points, SeqCode, &t, cypher);
            }
            if (noTri > 0)
               allDisplayPlane(plotscale, nx, ny, nz); /* plot planes using ZBuffering */
         }
      }
   }

   if ((threedViewOptions.fillType == DXF_FILE_FILL_3DFACE) ||
       (threedViewOptions.fillType == DXF_FILE_FILL_POLYLINE))
      dxclose ();
   if (threedViewOptions.fillType == VULCAN_FILE_FILL)
   {
      rewind (vulcanFile);
      WriteVulcanHeader (filename);
      fclose (vulcanFile);
   }
      
   allSurfFinish ();
   if (alltri)
   {
      xvt_mem_hfree ((char __huge *) alltri);
      alltri = (THREED_POINT_INFO __huge *) NULL;
   }
   if (cypher)
      destroy2DArray ((char **) cypher, 5000, ARRAY_LENGTH_OF_STRAT_CODE);
}

/*
** set up TETINFO structure with some basic info required later
*/
void
#if XVT_CC_PROTO
SetTet( int tno, int trot, TETINFO *t)
#else
SetTet(tno, trot, t)
int tno;
int trot;
TETINFO *t;
#endif
{
   t->tinc=tno+(trot*5); /* add offset for even/odd cubes */
   t->tetno=tno;

   t->ExCode = 0;
   t->InCode = 0;
   t->pC = 0;
   t->pC2 = 0;
   t->TetCode = 0;
   t->GoodPts[0] = 0; t->GoodPts[1] = 0;
   t->GoodPts[2] = 0; t->GoodPts[3] = 0;
   t->apexfirst = 0;
   t->apexmiddle = 0;
   t->apexlast = 0;
   t->order = 0;
   t->exact = 0;
}
   

/*
** make local copy of information for one voxel
*/
#if XVT_CC_PROTO
int allSetVerts(STORY ***dots3DC, double ***map3D,
            double Points[8][3], double Values[8], int SeqCode[8], 
            int ii, int jj, int kk, double plotscale,
            int cyphno, unsigned char **cypher)
#else
allSetVerts(dots3DC, map3D, Points, Values, SeqCode, ii, jj, kk, plotscale,
            cyphno, cypher)
STORY ***dots3DC;
double ***map3D, Points[8][3], Values[8];
int SeqCode[8], ii, jj, kk;
double plotscale;
int cyphno;
unsigned char **cypher;
#endif
{  
   int mm, nn;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   double xLoco,yLoco,zLoco;
   double yMax;

   xLoco = viewOptions->originX;
   yLoco = viewOptions->originY;
   zLoco = viewOptions->originZ - viewOptions->lengthZ;
   yMax = viewOptions->originY+viewOptions->lengthY;

                 /* Store the locations that will be tested */
   for (mm = 0; mm < 8; mm++)
   {
      Points[mm][0]=xLoco+(((ii-1)+CUBE[mm][0])*plotscale)+0.000001;
      Points[mm][1]=yMax-(yLoco+(((jj-1)+CUBE[mm][1])*plotscale))+0.000001;
      Points[mm][2]=zLoco+(((kk-1)+CUBE[mm][2])*plotscale)+0.000001;
   }
               
   for (mm = 0; mm < 8; mm++)
   {            
                  /* Store the height at the stored locaton */
      Values[mm] = map3D[kk+CUBE[mm][2]]
                        [jj+CUBE[mm][1]]      
                        [ii+CUBE[mm][0]];
      
      for (nn = 1; nn <= cyphno; nn++)
      {           /* work out if strats differ at these locations */
         if (coincide(&(cypher[nn][0]), dots3DC[kk+CUBE[mm][2]] /* mwj_fix */
                                             [jj+CUBE[mm][1]]
                                             [ii+CUBE[mm][0]].sequence))
         {
            SeqCode[mm]=nn;
            break;
         }
      }
      
      if(SeqCode[mm] != 1)
      {
         nn=1;
      }
   }
   return (TRUE);
}

/*
** calculate surfaces for one tetrahedron within voxel,
** first figuring out which type of combination of 4 end history codes
** is present:
** Alpha-   All 4 codes are same
** Beta-    3 same 1 diff
** Gamma-  2 same 2 diff
** Delta- 2 sets of 2 same
** Epsilon all 4 diff
*/
void
#if XVT_CC_PROTO
allOneTetrahedron(double Values[8], double Points[8][3],
                  int SeqCode[8], TETINFO *t, unsigned char **cypher)
#else
allOneTetrahedron(Values, Points, SeqCode, t, cypher)
double Values[8];
double Points[8][3];
int SeqCode[8];
TETINFO *t;
unsigned char **cypher;
#endif
{
   t->TetCode=CalcTetCode(SeqCode[TETAPICES[t->tinc][0]],
                          SeqCode[TETAPICES[t->tinc][1]],
                          SeqCode[TETAPICES[t->tinc][2]],
                          SeqCode[TETAPICES[t->tinc][3]], cypher);  
   
   switch (t->TetCode)
   {
      case 0:
         DEBUG(printf("Alpha");)
         AlphaCode (Values, Points, SeqCode, t);
         DEBUG(printf("\n");)
         break;
      case 100:
      case 10:
      case 1:
      case 111:
         DEBUG(printf("Beta");)
         BetaCode (Values, Points, SeqCode, t);
         DEBUG(printf("\n");)
      break;
      
      case 112:
      case 120:
      case 121:
      case 12:
      case 102:
      case 122:
         DEBUG(printf("Gamma");)
         GammaCode (Values, Points, SeqCode, t);
         DEBUG(printf("\n");)
         break;
      
      case 101:
      case 11:
      case 110:
         DEBUG(printf("Delta");)
         DeltaCode (Values, Points, SeqCode, t);
         DEBUG(printf("\n");)
         break;
      
      case 123:
         DEBUG(printf("Epsilon");)
         EpsilonCode (Points, SeqCode, t);
         DEBUG(printf("\n");)
         break;
      default:
         xvt_dm_post_error("I hadn't thought of that TetCode!");
   }
}



/*
** calculate  point position defined by end points and fraction along line
*/
double
#if XVT_CC_PROTO
MidVal(double x1, double x2, double bisect)
#else
MidVal(x1, x2, bisect)
double x1, x2, bisect;
#endif
{
   double val;
   
   val = ((x2-x1)*bisect) + x1;
   return (val);
}



/*
** calculate  code based on number of contiguous volumes for one tet
*/
int
#if XVT_CC_PROTO
CalcTetCode( int s1, int s2, int s3, int s4, unsigned char **cypher )
#else
CalcTetCode(s1, s2, s3, s4, cypher )
int s1, s2, s3, s4;
unsigned char **cypher;
#endif
{
   int tCode=0;
   
   if (cypherDiff(s1,s2,cypher))
      tCode += 100;
   
   if (cypherDiff(s1, s3,cypher))
   {
      if (cypherDiff(s2,s3,cypher) && tCode == 100)
         tCode += 20;
      else
         tCode += 10;
   }
   
   if (cypherDiff(s1,s4,cypher))
   {
      if (tCode==0)
         tCode+=1;
      else if (tCode==120 && cypherDiff(s2,s4,cypher) && cypherDiff(s3,s4,cypher))
         tCode+=3;
      else if (!cypherDiff(s2,s4,cypher))
         tCode+=1;
      else if (tCode==10 & !cypherDiff(s3,s4,cypher))
         tCode+=1;
      else
         tCode+=2;
   }
   
   return (tCode);
}


/*
** calculate number of midpoints
*/
int
#if XVT_CC_PROTO
allNoSurfaces(int mids[6])
#else
allNoSurfaces(mids)
int mids[6];
#endif
{
   int mm,count=0;
   
   for (mm=0;mm<18;mm++)
      if (mids[mm])
         count++;
         
   return(count);
}

/*
** store triangle for later ZBuffer processing
*/
void
#if XVT_CC_PROTO
ZBufferPlane(double conlist[4][3])
#else
ZBufferPlane(conlist)
double conlist[4][3];
#endif
{
   register int mm;

   if (!alltri) /* setup alltri first time through */
   {
      if (!(alltri = (THREED_POINT_INFO __huge *) xvt_mem_halloc (ALL_SURF_POINT_STORE,
                                                                  sizeof(THREED_POINT_INFO))))
         return;       
   }
   
   if (noTri > ALL_SURF_POINT_STORE) 
   {
      xvt_dm_post_error("That shouldn't happen in ZBuffer");
      return;
   }
   
   for (mm = 0; mm < 3; mm++)
   {
      alltri[noTri+mm].point.x = conlist[mm][0];
      alltri[noTri+mm].point.y = conlist[mm][1];
      alltri[noTri+mm].point.z = conlist[mm][2];
      alltri[noTri+mm].color = layerColor;
   }
   noTri+=3;
}
                                     
                                     
                                     
/*
** Sort triangles wrt distance from viewer and then plot
*/
int allDisplayPlane(plotscale, nx, ny, nz)
double plotscale;
int nx, ny, nz;
{
   register int mm, nn, point;
   double conlist[4][3];
   int halfnx = nx/2, halfny = ny/2, halfnz = nz/2;
   double plotRatio = plotscale/10.0;

   for (mm = 0; mm < noTri; mm+=3)
   {
      for (nn = 0; nn < 3; nn++)
      {
         /*
         ** The "/plotscale)*(nx/2) * (plotscale/10)" part is to scale the
         ** the triangle so it appears nicely in the viewing window
         */
         point = mm+nn;
         alltri[point].point.x = ((alltri[point].point.x/plotscale)-halfnx)
                                                           * plotRatio;
         alltri[point].point.y = ((alltri[point].point.y/plotscale)-halfny)
                                                           * plotRatio;
         alltri[point].point.z = ((alltri[point].point.z/plotscale)-halfnz)
                                                           * plotRatio;
      }
   }

                  /* Sort in order of closest planes */
   qSortPoints (alltri, 0, noTri-3, 3);

   for (mm = 0; mm < noTri; mm+=3)
   {
      for (nn = 0; nn < 3; nn++)
      {
         point = mm+nn;
         conlist[nn][0] = alltri[point].point.x;
         conlist[nn][1] = alltri[point].point.y;
         conlist[nn][2] = alltri[point].point.z;
      }  
      oneDisplayPlane(conlist, mm);
   }
   return (TRUE);
}
   

/*
** actually draw and write to file one 3D triangle
*/
void
#if XVT_CC_PROTO
oneDisplayPlane(double conlist[4][3],int triNo)
#else
oneDisplayPlane(conlist, triNo)
double conlist[4][3];
int triNo;
#endif
{
   PNT thePoly[4];
   int mm, index, polySize = 3;
   COLOR shading, lineShade;
   DRAW_CTOOLS tools;
   WINDOW win = getCurrentDrawingWindow ();
   Point3d point3D;
   Point2d point2D;
   int newSize;
   THREED_POINT_INFO __huge *newPoints;
   THREED_POINT_INFO *point;
   BLOCK_VIEW_OPTIONS *viewOptions;
   
   if (!threedDataPtr)
      return;

   xvt_dwin_get_draw_ctools (win, &tools);

                 /* make sure the threedData structure is big enough to
                 ** store the next few points */
   index = (int) threedDataPtr->surfaceWidth;
   if (index+3 > threedDataPtr->surfaceHeight)
   {                             /* Expand the array if needed */
      newSize = (int) threedDataPtr->surfaceHeight + 300; /* the next hunk of memory */
      newPoints = (THREED_POINT_INFO __huge *) xvt_mem_halloc(newSize, sizeof(THREED_POINT_INFO));
      if (newPoints)
      {
         if (threedDataPtr->surface)
         {
            hmemcpy (newPoints, threedDataPtr->surface,
                     threedDataPtr->surfaceHeight*sizeof(THREED_POINT_INFO));
            xvt_mem_hfree ((char __huge *) threedDataPtr->surface);
         }
         threedDataPtr->surfaceHeight = newSize;
         threedDataPtr->surface = newPoints;
      }
      else   /* make sure we dont try and fill it */
      {
         if (threedDataPtr->surface)
         {
            xvt_mem_hfree ((char __huge *) threedDataPtr->surface);
            threedDataPtr->surface = NULL;
         }
      }
   }
   
   for (mm = 2; mm >= 0; mm--)
   {
      point3D.x = conlist[mm][1];
      point3D.y = conlist[mm][2];
      point3D.z = conlist[mm][0];
                          /* Fill In the threedData structure (store points) */
      if (threedDataPtr->surface)
      {
         point = &(threedDataPtr->surface[index+mm]);
         point->point.x = point3D.x;
         point->point.y = point3D.y;
         point->point.z = point3D.z;
         point->color = alltri[triNo].color;
         threedDataPtr->surfaceWidth++;
      }

      ApplyTransform(&point3D, &point3D, &xFormViewer);
      Project(&point3D, &point2D);
      thePoly[mm].h = (short) point2D.x;
      thePoly[mm].v = (short) point2D.y;
   }

   if (threedViewOptions.fillType == XYZ_COLOR_FILL)
   {
      viewOptions = getViewOptions ();
      ShadeXYZ(conlist, &shading, viewOptions->lengthX, viewOptions->lengthY, viewOptions->lengthZ);
      lineShade= XVT_MAKE_COLOR(
                  (XVT_COLOR_GET_RED(shading)  +((255-XVT_COLOR_GET_RED(shading))/2)),
                  (XVT_COLOR_GET_GREEN(shading)+((255-XVT_COLOR_GET_GREEN(shading))/2)),
                  (XVT_COLOR_GET_BLUE(shading) +((255-XVT_COLOR_GET_BLUE(shading))/2)));
      tools.pen.width = 0; 
      tools.pen.color = lineShade; 
      tools.pen.pat = PAT_SOLID;
      tools.brush.color = shading; 
      tools.brush.pat = PAT_SOLID;
   }
   else if (threedViewOptions.fillType == SHADING_FILL)
   {
      ShadeLambert(conlist, &shading);
      tools.pen.width = 0; 
      tools.pen.color = shading; 
      tools.pen.pat = PAT_SOLID;
      tools.brush.color = shading; 
      tools.brush.pat = PAT_SOLID;
   }
   else if (threedViewOptions.fillType = LINES_FILL)
   {
      tools.pen.width = 0; 
      tools.pen.color = (COLOR) alltri[triNo].color;
      tools.pen.pat = PAT_SOLID;
      tools.brush.color = backgroundColor; 
      tools.brush.pat = PAT_SOLID;
   }
   xvt_dwin_set_draw_ctools (win, &tools);
   xvt_dwin_draw_polygon (win, thePoly, polySize);
}  

/*
** temporarily store and write to file one 3D triangle
*/
void
#if XVT_CC_PROTO
allDrawPlane(double conlist[4][3])
#else
allDrawPlane(conlist)
double conlist[4][3];
#endif
{
   FILE *fpvulcan;  /* Vulcan file pointer */
   register int mm;
   float xa[5], ya[5], za[5];
   
   if (Skinny(conlist) || tooLong(conlist))
      return;

   switch (threedViewOptions.fillType)
   {
      case (DXF_FILE_FILL_3DFACE):
      case (DXF_FILE_FILL_POLYLINE):
         for (mm = 0; mm < 3; mm++)
         {
            xa[mm+1]=(float) conlist[mm][0];
            ya[mm+1]=(float) conlist[mm][1];
            za[mm+1]=(float) conlist[mm][2];
         }
               
         xa[4] = (float) conlist[2][0];
         ya[4] = (float) conlist[2][1];
         za[4] = (float) conlist[2][2];
               
         if (threedViewOptions.fillType == DXF_FILE_FILL_3DFACE)
            dxfa3l(xa,ya,za);
         else /* DXF_FILE_FILL_POLYLINE */
            dxpl3l(xa,ya,za,3,1);
         break;
      case (VULCAN_FILE_FILL):
         fpvulcan = (FILE *) getDxfFile ();
         fprintf(fpvulcan,"%s\n",clayer);
         for(mm=0;mm<3;mm++)
            fprintf(fpvulcan,"%8.4lf\t%8.4lf\t%8.4lf\n",
                          conlist[mm][0] + projectOptions.easting,
                          conlist[mm][1] + projectOptions.northing, conlist[mm][2]);
         
         fprintf(fpvulcan,"%06ld\t%06ld\t%06ld\n",vertexcount,
                                                    vertexcount+1,vertexcount+2);
         vertexcount += 3;
         break;
      default: /*displaying as an image */
         ZBufferPlane(conlist);
   }
}

/*
** colour triangle according to position red=x, green=y, blue=z
*/
void
#if XVT_CC_PROTO
ShadeXYZ(double conlist[4][3], COLOR *shading, double lengthX, double lengthY, double lengthZ)
#else
ShadeXYZ(conlist, shading, lengthX, lengthY, lengthZ)
double conlist[4][3];
COLOR *shading;
double lengthX, lengthY, lengthZ;
#endif
{
   *shading = XVT_MAKE_COLOR (
              (((((conlist[0][0]+conlist[1][0]+conlist[2][0])/3)+lengthX/20.0)/(lengthX/10.0))*255),
              (((((conlist[0][1]+conlist[1][1]+conlist[2][1])/3)+lengthY/20.0)/(lengthY/10.0))*255),
              (((((conlist[0][2]+conlist[1][2]+conlist[2][2])/3)+lengthZ/20.0)/(lengthZ/10.0))*255));

/*
              (((((conlist[0][0]+conlist[1][0]+conlist[2][0])/3)+500)/1000.0)*255),
              (((((conlist[0][1]+conlist[1][1]+conlist[2][1])/3)+350)/700.0)*255),
              (((((conlist[0][2]+conlist[1][2]+conlist[2][2])/3)+250)/500.0)*255));
*/
}


/*
** colour triangle a shade of grey according to orientation
*/
void
#if XVT_CC_PROTO
ShadeLambert(double conlist[4][3], COLOR *shading)
#else
ShadeLambert(conlist, shading)
double conlist[4][3];
COLOR *shading;
#endif
{
   int value;
   double alpha, beta, gamma, ratio, angle;
   double a1=-0.57735, a2=0.57735, a3=0.57735;
   
   alpha = determinant(conlist,0);
   beta  = determinant(conlist,1);
   gamma = determinant(conlist,2);
   
   ratio = sqrt((alpha*alpha) + (beta*beta) + (gamma*gamma));
   
   if (ratio == 0.0)
      ratio = 0.000001;
       
   alpha = alpha/ratio;
   beta  = beta/ratio;
   gamma = gamma/ratio;
   
   angle = (alpha*a1) + (beta*a2) + (gamma*a3);
   
   if(angle < 0.0 )
      angle = -angle;

   value = (int) ((double) (angle*200.0) + 20.0);
   *shading = XVT_MAKE_COLOR(value, value, value);
}

/*
** colour triangle a shade of grey according to orientation second attempt
*/
void
#if XVT_CC_PROTO
Qd3DShadeLambert(double conlist[4][3], COLOR *shading)
#else
Qd3DShadeLambert(conlist, shading)
double conlist[4][3];
COLOR *shading;
#endif
{
#ifdef PAUL
   COLOR trgb;
   static Point3d pl[3], norm, avev,   
                         ka={0.2,0.2,0.2},
                         kd={0.8,0.8,0.8},
                         L={-10000,-10000,10000},
                         la={40000.0,0.0,0.0},
                         li={40000.0,40000.0,40000.0},
                         result;
   
   pl[0].x=conlist[0][0];
   pl[0].y=conlist[0][1];
   pl[0].z=conlist[0][2];
   
   pl[1].x=conlist[1][0];
   pl[1].y=conlist[1][1];
   pl[1].z=conlist[1][2];
   
   pl[2].x=conlist[2][0];
   pl[2].y=conlist[2][1];
   pl[2].z=conlist[2][2];
   
   avev.x=(pl[0].x+pl[1].x+pl[2].x)/3;
   avev.y=(pl[0].y+pl[1].y+pl[2].y)/3;
   avev.z=(pl[0].z+pl[1].z+pl[2].z)/3;

   /* L.x=L.x-avev.x;
   ** L.y=L.y-avev.y;
   ** L.z=L.z-avev.z; */
   
/*
** vnorm(pl, &norm);
** LambertLight(&ka,&kd,&L,&norm,&la,&li,&result);
** Color2RGB(&result, &trgb);
*/

   *shading = trgb;
#endif
}

/*
** calculate determinant of matrix for Lambert shading
*/
double
#if XVT_CC_PROTO
determinant(double conlist[4][3], int code)
#else
determinant(conlist, code)
double conlist[4][3];
int code;
#endif
{
   double t1,t2,t3,t4,tl[3][3];
   int mm,tc;
   
   
   for (mm = 0, tc = 0; mm < 3; mm++)
   {
      if (mm == code)
         continue;
      else
      {
         tl[0][tc]=conlist[0][mm];
         tl[1][tc]=conlist[1][mm];
         tl[2][tc]=conlist[2][mm];
         tc++;
      }
   }
   
   tl[0][2]=1;
   tl[1][2]=1;
   tl[2][2]=1;
   
   t1= tl[0][0]*((tl[1][1]*tl[2][2])-(tl[1][2]*tl[2][1]));
   t2=-tl[0][1]*((tl[1][0]*tl[2][2])-(tl[1][2]*tl[2][0]));
   t3= tl[0][2]*((tl[1][0]*tl[2][1])-(tl[1][1]*tl[2][0]));
   
   t4=t1+t2+t3;
   
   return(t4);
}

OBJECT *
#if XVT_CC_PROTO
SetCLayer(unsigned char *start1, unsigned char *start2,
          int InCode, int ExCode)
#else
SetCLayer(start1, start2, InCode, ExCode)
unsigned char *start1, *start2;
int InCode, ExCode;
#endif
{
   int break_code, swap_code, i;
   OBJECT *object = NULL;
   NODDY_COLOUR *colorStruct;
   int eventNum = 0;

   break_code = lastdiff(start1, start2);
   
   if (ExCode < InCode)
   {
      swap_code=InCode;
      InCode=ExCode;
      ExCode=swap_code;
   }
   sprintf(clayer,"BX%03d%03d%03d",break_code,InCode,ExCode);

                      /* check to make sure the layer is on */
   if (!threedViewOptions.allLayers)
   {
      for (i = 0; i < numEventsForStratLayers; i++)
      {               /* find layer corresponding to event */
         if (eventsForStratLayers[i] == break_code)
         {
            if (threedViewOptions.layerOn[i])
            {
               eventNum = break_code;  /* On */
               break;
            }
/* Cannot have this as it will stop after the first uncon layer not on
   We still want it to pass through if only the last layer in on.
            else
               break;
*/
         }
      }
   }
   else
      eventNum = break_code;

   if (!eventNum)
      return ((OBJECT *) NULL);
   
   if (!(object = (OBJECT *) nthObject (NULL_WIN, eventNum)))
      return ((OBJECT *) NULL);
   
   switch (object->shape)
   {
      case FAULT:
         if (colorStruct = getNoddyColourStructure (object))
            layerColor = XVT_MAKE_COLOR (colorStruct->red,
                                         colorStruct->green,
                                         colorStruct->blue);
         else
            layerColor = COLOR_RED;
         break;
      case UNCONFORMITY:
              /* Find last Layer still in Uncon (ie. the base layer) */
         while (eventsForStratLayers[i+1] == break_code)
            i++;
            
         if (threedViewOptions.layerOn[i])  /* If Base layer on draw the uncon plane */
         {
            object->generalData = 1;  /* get color of first layer */
            if (colorStruct = getNoddyColourStructure (object))
               layerColor = XVT_MAKE_COLOR (colorStruct->red,
                                            colorStruct->green,
                                            colorStruct->blue);
            else
               layerColor = COLOR_GREEN;
         }
         break;
      case DYKE: case PLUG:
         if (colorStruct = getNoddyColourStructure (object))
            layerColor = XVT_MAKE_COLOR (colorStruct->red,
                                         colorStruct->green,
                                         colorStruct->blue);
         else
            layerColor = COLOR_RED;
         break;
      default:
         layerColor = COLOR_WHITE;
         break;
   }
   
   return (object);
}

/*
** empty triangle check
*/
int 
#if XVT_CC_PROTO
Skinny(double conlist[4][3])
#else
Skinny(conlist)
double conlist[4][3];
#endif
{
   if(conlist[0][0]==conlist[1][0] && 
      conlist[0][1]==conlist[1][1] && 
      conlist[0][2]==conlist[1][2])
      return(TRUE);     
   else if(conlist[0][0]==conlist[2][0] && 
           conlist[0][1]==conlist[2][1] && 
           conlist[0][2]==conlist[2][2])
      return(TRUE);     
   else if(conlist[2][0]==conlist[1][0] && 
           conlist[2][1]==conlist[1][1] && 
           conlist[2][2]==conlist[1][2])
      return(TRUE);
   else
      return(FALSE);
}  
int 
#if XVT_CC_PROTO
tooLong(double conlist[4][3])
#else
tooLong(conlist)
double conlist[4][3];
#endif
{
   double length;
   int i, j;
   
   for (i = 0; i < 3; i++)
   {
      for (j = 0, length = 0.0; j < 3; j++)
      {
         length = length + ((conlist[i][j] - conlist[(i+1)%3][j])
                           *(conlist[i][j] - conlist[(i+1)%3][j]));
      }
      if (length > 7500*plotScale)
         return (TRUE);
   }

   return (FALSE);
}  



LAYER_PROPERTIES *
#if XVT_CC_PROTO
renderLayer (OBJECT *object, int eventNum)
#else
renderLayer (object, eventNum)
OBJECT *object;
int eventNum;
#endif
{
   LAYER_PROPERTIES *properties = (LAYER_PROPERTIES *) NULL;
   int layer, layersBefore;
   
   if (object->generalData == 0)
   {
#ifdef OLD
      if (object->shape == STRATIGRAPHY)
         object->generalData = 2; /* the first layer is not rendered */
      else
#endif
         object->generalData = 1; /* all layers rendered rendered */
   }

   if (threedViewOptions.allLayers)
   {       
      if (!(properties = getLayerPropertiesStructure (object)))
         object->generalData = 0; /* at the end of strat so start from start again */
      else
         object->generalData++;  /* get next layer next time */
   }
   else /* check individual layers */
   {                          /* Find num of layers before current event */
      for (layer = layersBefore = 0; layer < numEventsForStratLayers; layer++)
      {
         if (eventsForStratLayers[layer] != eventNum)
            layersBefore++;
         else
            break;
      }
                         /* Find the next layer that is turned on */
      for (layer = layersBefore + object->generalData-1; (layer < 99); layer++)
      {
         if (threedViewOptions.layerOn[layer])
         {
            object->generalData = (layer - layersBefore) + 1;
            if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY))
               object->generalData = -object->generalData; /* Start from top and not base */
               
            if (!(properties = getLayerPropertiesStructure (object)))
               object->generalData = 0; /* at the end of strat so start from start again */
            else
               object->generalData = abs(object->generalData) + 1;  /* get next layer next time */
            break;
         }
      }
   }
   
   return (properties);
}


void
#if XVT_CC_PROTO
WriteVulcanHeader(char *fname)
#else
WriteVulcanHeader(fname)
char *fname;
#endif
{
   FILE *headout;
   int nn;
   char layernew[OBJECT_TEXT_LENGTH];
   LAYER_PROPERTIES *properties[100];
   int eventNumbers[100];
   int numLayers, layer;
   OBJECT *object;
   
   addFileExtention (fname, ".hed");
   headout=fopen(fname,"w");
   addFileExtention (fname, ".vul");
   fprintf(headout,"%s\n%ld\n",fname,vertexcount);
   
   numLayers = assignPropertiesForStratLayers (properties, 100);
   numLayers = assignEventNumForStratLayers (eventNumbers, 100);


   for (nn = 0, layer = 0; nn < numLayers; nn++)
   {
      if (object = nthObject (NULL_WIN, eventNumbers[nn]))
      {
         switch(object->shape)
         {
            case STRATIGRAPHY:
               layer++;
               add_underscore(layernew, properties[nn]->unitName);
               fprintf(headout,"S_%02d_%02d_%s\n", eventNumbers[nn]+1, layer, layernew);
               break;
            case UNCONFORMITY:
               fprintf(headout,"B_uncon_%03d\n", eventNumbers[nn]+1);
               break;
            case DYKE:
               fprintf(headout,"B_dyke__%03d\n", eventNumbers[nn]+1);
               break;
            case PLUG:
               fprintf(headout,"B_plug__%03d\n", eventNumbers[nn]+1);
               break;
            case IMPORT:
               fprintf(headout,"B_imprt_%03d\n", eventNumbers[nn]+1);
               break;
         }
      }
   }
   
   numLayers = countObjects (NULL_WIN);
   for (nn = 0; nn < numLayers; nn++)
   {
      if (object = nthObject (NULL_WIN, nn))
      {
         if (object->shape == FAULT)
         {
            fprintf(headout,"B_fault_%03d\n", nn+1);
         }
      }
   }
   
   fclose(headout);
}

void
#if XVT_CC_PROTO
add_underscore(char *nameout, char *namein)
#else
add_underscore(nameout, namein)
char *nameout;
char *namein;
#endif
{
   int indexing=0;
   
   do
   {
      if(namein[indexing]==' ')
         nameout[indexing]='_';
      else
         nameout[indexing]=namein[indexing];
         
      indexing++;
   } while (namein[indexing-1] != '\0');
}


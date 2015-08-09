/* *************************************************************************** 
** This algorithm is based on a "simple" way of triangulating the voxel data
** set generated within Noddy. The first step is to divide the volume up into
** voxels and then divide up each voxel into 5 tetrahedra. For each apex on
** the tetrahedron the initial Z values are stored (Values), together with its
** discontinuity code (SeqCode), the current X,Z,Y coordinates (Points). This
** header file contains various ways of accessing and the individual vertices
** of each tetrahedra. Two index systems are needed to access tetrahedra in
** successive voxels so that adjacent tetrahedra are aligned. 
** 
** The basic voxel vertex coding system is contained in CUBE: 
**                 
**                               6------------7 
**                              /.           /|  
**                             / .          / |   
**                            /  .         /  |    
**                           4------------5   |     
**                           |   .        |   |      
**                           |   2........|...3       
**                           |  .         |  /         
**                           | .          | /           
**                           |.           |/             
**                           0------------1          
**                                            
** The sets of 4 apices for each of the five tetrahedra are contained in
** TETAPICES
** 
** The edges connecting the apices for each tetrahedron (labeled V-Z) are
** contained in TETLINES
** 
** The apex pairs connected by each edge are contained in LINES 
** 
** An individual apex code may be oobtained from TETAPICES, or
** alternatively by or LINES[TETLINES[0 to 4][mm]][0 or 1]
*************************************************************************** */

extern int **CUBE;
extern int **TETAPICES;
extern int **TETLINES;
extern int **LINES;
 
typedef struct s_TETINFO
{
   int tetno;  /* tetrahedron code */
   int tinc;   /* tetrahedron code accounting for odd/even cubes */
   int cyphno; /* number of discontinuity codes for whole model */
               /* list of discontinuity codes for whole system */
   unsigned char cypher[5000][ARRAY_LENGTH_OF_STRAT_CODE];
               /* code of apex not part of tetrahedron which could have a
               ** stratigraphic triangle */
   int ExCode;
               /* code of apex part of tetrahedron which could have a
               ** stratigraphic triangle */
   int InCode;
               /* number of triangulated surfaces which intersect a break
               ** surface */
   int pC;
               /* number of triangulated surfaces which intersect a break
               ** surface (only for delta codes) */
   int pC2;
               /* code of tetrahedron based on number of discontinuity
               ** codes present */
   int TetCode;
   int GoodPts[4]; /* ordered set of apex codes */
   int apexfirst;  /* apex for first triangle (only for beta codes) */
   int apexmiddle; /* apex for middle triangle (only for beta codes) */
   int apexlast;   /* apex for end triangle (only for beta codes) */
               /* flag to change order of break mid points (only for beta
               ** codes) */
   int order;
               /* flag to indicate layer exactly cuts an apex (only for beta
               ** codes) */
   int exact;
} TETINFO;


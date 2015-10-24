/*
*/
#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include <math.h>

#define DEBUG(X)    

#define     INCL    -67.0
#define     INTEN   63000.0
#define     ALTI    80.0
#define     MSIZE   6
#define     SUM     40

struct stratm
{
   double density;
   double magsus;
};

#if XVT_CC_PROTO
int convertVulcanBlockToNoddy (FILE_SPEC *, FILE_SPEC *, int, int, int);
#else
int convertVulcanBlockToNoddy ();
#endif


int
#if XVT_CC_PROTO
convertVulcanBlockToNoddy (FILE_SPEC *vulcanBlock, FILE_SPEC *noddyBlock,
                           int flipX, int flipY, int flipZ)
#else
convertVulcanBlockToNoddy (vulcanBlock, noddyBlock, flipX, flipY, flipZ)
FILE_SPEC *vulcanBlock, *noddyBlock;
int flipX, flipY, flipZ;
#endif
{
   FILE *in;
   int **layerDimensions, *cubeSizes;
   short ***volume;
   register int i, xs, ys, zs;
   int err, nx, ny, nz;             /* # of xyz, # of rock types */
   double x, y, z, xd, yd, zd, den, sus;
   int rock, numRock;
   LAYER_PROPERTIES *rockProps[250], *allocedProps;
   double maxx, maxy, maxz, minx, miny, minz, duma, dumb, dumc;
   int xin, yin, zin;
   char memLabel[10];
   
   strcpy (memLabel, "Anom");
   
   if (!memManagerAddLabel (memLabel))
   {
      xvt_dm_post_error ("Error, Not Enough memory for vulcan conversion");
      return (FALSE);
   }

   initLongJob (0, 100, "Scaning Vulcan File ...", memLabel);

                        /* Open the Input File */
   xvt_fsys_set_dir (&(vulcanBlock->dir));                        
   if ((in = (FILE *) fopen(vulcanBlock->name, "r" )) == 0L)
   {
      memManagerFreeAfterLabel (memLabel);
      finishLongJob ();
      return (FALSE);
   }

   /* As a Vulcan file is an order independent file with no real header to speak of
   ** we need to read through the file once to find out what area it covers. Then
   ** we can create an array big enough and read through a second time slowly filling
   ** up the array as we go */

                     /* Read in the first line and initialise the extents */
   fscanf(in,"%lf %lf %lf %lf %lf %lf %d %lf %lf",
             &maxx, &maxy, &maxz, &xd, &yd, &zd, &numRock, &den, &sus);
   minx = maxx;
   miny = maxy;
   minz = maxz;

                     /* Scan through the rest of the file and find its extents */
   while ((err = fscanf(in,"%lf %lf %lf %lf %lf %lf %d %lf %lf",
           &x, &y, &z, &duma, &dumb, &dumc, &rock, &den, &sus)) != EOF)
   {
      if (x > maxx) maxx = x;
      if (y > maxy) maxy = y;
      if (z > maxz) maxz = z;
      if (x < minx) minx = x;
      if (y < miny) miny = y;
      if (z < minz) minz = z;
      if (rock > numRock) numRock = rock;
      if (abortLongJob ())
      {
         fclose (in);
         memManagerFreeAfterLabel (memLabel);
         return (FALSE);
      }
   }
   rewind(in);

                     /* Calculate the size of the file */
   nx = 1 + (int) ((maxx-minx)/xd);
   ny = 1 + (int) ((maxy-miny)/yd);
   nz = 1 + (int) ((maxz-minz)/zd);

   initLongJob (0, nz*3, "Converting Vulcan File ...", NULL);
   incrementLongJob (nz); /* Start the Gauge from a third throuigh to account for first scan */

                     /* Get an array Big enough to hold everything */
   layerDimensions = (int **) create2DArray(nz, 2, sizeof(int));
   cubeSizes = (int *) create1DArray(nz, sizeof(int));
   allocedProps = (LAYER_PROPERTIES *) create1DArray (numRock+1,
                                            sizeof (LAYER_PROPERTIES));

   if (!layerDimensions || !cubeSizes || !allocedProps)
   {
      destroy2DArray((char **) layerDimensions, nz, 2);
      /*destroy3DIregArray((char ***) volume, nz, layerDimensions);*/
      if (cubeSizes) destroy1DArray((char *) cubeSizes);
      if (allocedProps) destroy1DArray((char *) allocedProps);
      fclose (in);
      memManagerFreeAfterLabel (memLabel);
      finishLongJob ();
      xvt_dm_post_error ("Error, Out Of Memory.");
      return (FALSE);
   }

   for (i = 0; i <= numRock; i++)
      rockProps[i] = &(allocedProps[i]);
   for (i = 0; i < nz; i++)
   {
      layerDimensions[i][0] = nx;
      layerDimensions[i][1] = ny;
      cubeSizes[i] = (int) xd;
   }

   volume = (short ***) create3DIregArray(nz, layerDimensions, sizeof (short));
   if (!volume)
   {
      destroy2DArray((char **) layerDimensions, nz, 2);
      destroy3DIregArray((char ***) volume, nz, layerDimensions);
      if (cubeSizes) destroy1DArray((char *) cubeSizes);
      if (allocedProps) destroy1DArray((char *) allocedProps);
      fclose (in);
      memManagerFreeAfterLabel (memLabel);
      finishLongJob ();
      xvt_dm_post_error ("Error, Out Of Memory.");
      return (FALSE);
   }

                      /* Read through the Vulcan file again Filling Our Array */
   for(zs = 0; zs < nz; zs++)
   {
      if (abortLongJob ())
      {
         fclose (in);
         memManagerFreeAfterLabel (memLabel);
         return (FALSE);
      }

      incrementLongJob (INCREMENT_JOB);
      for( ys = 0; ys < ny; ys++)
      {
         for ( xs = 0; xs < nx; xs++)
         {
            fscanf(in,"%lf %lf %lf %lf %lf %lf %d %lf %lf",
                   &x, &y, &z, &duma, &dumb, &dumc, &rock, &den, &sus);
            xin = (int) ((x-minx)/xd);
            yin = (int) ((y-miny)/yd);
            zin = (int) ((z-minz)/zd);
            if (((xin > -1) && (xin < nx)) && ((yin > -1) && (yin < ny))
                                           && ((zin > -1) && (zin < nz)))
            {
               if (flipX) xin = nx - xin - 1;
               if (flipY) yin = ny - yin - 1;
               if (flipZ) zin = nz - zin - 1;
               rockProps[rock]->density = den;
               rockProps[rock]->sus_X = sus/1e6;
               volume[zin][xin][yin] = rock;
            }
         }
      }
   }
   fclose(in);  /* Finished reading the Vulcan File */

   
                                /* Write out the Noddy File */
   xvt_fsys_set_dir (&(noddyBlock->dir));                        
   addFileExtention (noddyBlock->name, ANOM_HEADER_FILE_EXT);
   writeBlockHeaderToFile (noddyBlock->name, nz, layerDimensions,
                           minx, miny, minz, maxx, maxy, maxz,
                           nz,  cubeSizes, (int) MSIZE, (double) INCL,
                           (double) INTEN, (double) 0.0, (int) TRUE,
                           (int) TRUE, (int) FALSE, (int) FALSE,
                           (int) TRUE, numRock, rockProps);
   addFileExtention (noddyBlock->name, ANOM_INDEX_FILE_EXT);
   write3DIregBlockToFile (noddyBlock->name, (char ***) volume,
                           nz, layerDimensions, sizeof(short));
   
                                /* Free memory we have alloced */
   destroy3DIregArray((char ***) volume, nz, layerDimensions);
   destroy2DArray((char **) layerDimensions, nz, 2);
   if (cubeSizes) destroy1DArray((char *) cubeSizes);
   if (allocedProps) destroy1DArray((char *) allocedProps);

   memManagerFreeAfterLabel (memLabel);

   finishLongJob ();
   return (TRUE);
}



#include "xvt.h"
#include "noddy.h"

#define DEBUG(X)  

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern double **topographyMap;

                 /* ************************* */
                 /* Globals used in this file */


/************************************************************************
*                                                       *
*  void DoLineMap()                                       *
*                                                                       *
*                                                                       *
*  DoLineMap function calculates contact map for current geology      *
*                                                                       *
*  DoLineMap() takes no arguments                                     *
*  DoLineMap() returns no value                                       *
*                                                                       *
************************************************************************/
void DoLineMap_Line (win)
WINDOW win;
{
   register int m;
   OBJECT *event;
   STRATIGRAPHY_OPTIONS *stratOptions = NULL;
   COLOR layerColors[100];
   int nx,ny,cyphno=0,index1;
   int ncon, j, k, index, l, i, red, green, blue;
   unsigned int flavor, flavor1;
   unsigned char cypher[200][ARRAY_LENGTH_OF_STRAT_CODE];
   double cval[20];  /* contour values*/
   double dots1;
   int numEvents = (int) countObjects(NULL_WIN);
   double SectionDec = geologyOptions.welllogDeclination;
   int bar=0;
   int iign;  /* iign TRUE if DYKE or PLUG */
   double ***dots;
   STORY **histoire;
   double **map72;           /* second map for topo/line maps */
   double **map;             /* array of map positions for line maps */
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;

   nx=71;                                                                     
   ny=51;                                                                     
   ncon=7;

   if ((histoire = (STORY **) create2DArray(nx+1, ny+1, sizeof(STORY)))==0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }
       
   if ((dots = (double ***) create3DArray(nx+1, ny+1, 4, sizeof(double)))==0L)  /* new! */
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      destroy2DArray((char **) histoire,nx+1,ny+1);
      return;
   }

   if ((map = (double **) create2DArray(72, 52, sizeof(double))) == 0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      destroy2DArray((char **) histoire, nx+1, ny+1);
      destroy3DArray((char ***) dots, nx+1, ny+1, 4);
      return;
   }

   if ((map72 = (double **) create2DArray(73, 53, sizeof(double))) == 0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      destroy2DArray((char **) histoire, nx+1, ny+1);
      destroy3DArray((char ***) dots, nx+1, ny+1, 4);
      destroy2DArray((char **) map, 72, 52);
   }
   else
   {
                /* make sure all the drawing goes into this window */
      setCurrentDrawingWindow (win);

      initLongJob (0, 71*(countObjects(NULL_WIN)+1), "Calculating Line Map...", (char *) NULL);

      ScaleTopoOld (dots, map, cval, &ncon, nx, ny);
      
      if (geologyOptions.useTopography)
      {
         mcontr(topographyMap,nx-1,ny-1,cval,ncon+1,1.0e6,0.2,1.3,1.0,TRUE,
                                                  BASE_STRAT, FALSE, NULL);
         mcontrlabel(topographyMap,nx-1,ny-1,cval,ncon+1,1.0e6,0.2,1.3,1.0);
      }
      
      ldotmp(dots, histoire, LINE, cypher, &cyphno, 51, 71);

      mplot(4.0, 3.3, -3);
      for (j = 1; j <= cyphno; j++) 
      {
         iign=0;
 
         taste (numEvents, cypher[j], &flavor, &index);
         if (j < cyphno)
         {
            taste (numEvents, cypher[j+1], &flavor1, &index1);
         }
         else
            flavor1 = flavor;

         if (flavor != IGNEOUS_STRAT)
         {                                  
            for (k = 1; k <= 51; k++)
            {
               for (l = 1; l <= 71; l++)
               {
                  dots1 = dots[l][k][3];
                  if (coincide(histoire[l][k].sequence, cypher[j]))
                     map[l][k]=dots1;
                  else
                     map[l][k]=1.0e8;
               }                                                             
            }

            if (flavor == BASE_STRAT)
               index = 0;

              /* if the current event does not have a stratigraphy
              ** associated with it then use the base stratigraphy */
            if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
               event = (OBJECT *) nthObject (NULL_WIN, 0);
            if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
                                   getStratigraphyOptionsStructure (event)))
            {
               if (!(stratOptions = (STRATIGRAPHY_OPTIONS *)
                  getStratigraphyOptionsStructure(
                                         (OBJECT *) nthObject (NULL_WIN, 0))))
               return;  /* not even a base strat is defined yet */
            }

            for (m = 0; m < 10; m++)
               cval[m] = -99999.0;  /* initialise off the edge */
                            /* assign cval the height of each layer but first */
            for (m = 1; m <= (stratOptions->numLayers-1); m++)
               cval[m] = stratOptions->properties[m].height;

            for (m = 0; m < stratOptions->numLayers; m++)
            {
               red = stratOptions->properties[m].color.red;
               green = stratOptions->properties[m].color.green;
               blue = stratOptions->properties[m].color.blue;
               layerColors[m] = (COLOR) XVT_MAKE_COLOR(red, green, blue);
            }

            ncon = stratOptions->numLayers;
            mcontr(map, nx-1, ny-1, cval, ncon, 1.0e6, 0.2, 1.3, 1.0, FALSE,
                   flavor, iign, layerColors);
         }

       /*  for (k = 1; k <= 51; k++) 
         {
             for (l = 1; l <= 71; l++) 
            {                                                        
               dots1=dots[l][k][3];
               if (lineCoincide(histoire[l][k].sequence,cypher[j]))              
                  map[l][k]=1.0;
               else
                  map[l][k]=-1.0;
            }                                                             
         }

         ncon=1;
         cval[1]=0.0;

         ratio=1;
         testmap(map, histoire, nx-1, ny-1);
         smoothmap(map, map72, nx-1, ny-1);

         if (flavor == IGNEOUS_STRAT
                       || (flavor1 == IGNEOUS_STRAT && flavor != UNC_STRAT))
            iign = index;  * dyke or plug *
         else
            iign = 0;

         mcontr(map72, (nx-1)/ratio, (ny-1)/ratio, cval, ncon,
                                     1.0e9, 0.2*ratio, 1.3, 1.0, FALSE,
                                     flavor, iign, layerColors);*/
 
         penSize(1,1);

         for (i = 0; i < (71/cyphno); i++)
            incrementLongJob (INCREMENT_JOB);
      }
    
      contour_boundaries (histoire, nx-1, ny-1, 1.3, 1.0);
      iign=0; 
      PlotMapLegend(20, 300, absx, absy, absz);

      destroy2DArray((char **) histoire, nx+1, ny+1);
      destroy3DArray((char ***) dots, nx+1, ny+1, 4);
      destroy2DArray((char **) map, 72, 52);
      destroy2DArray((char **) map72, 73, 53);

      finishLongJob ();
   }
}


#include <math.h>
#include "xvt.h"
#include "noddy.h"

#define DEBUG(X)  

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;

                 /* ************************* */
                 /* Globals used in this file */


/************************************************************************
*                          *
*  void DoSection()                 *
*                          *
*                          *
*  DoSection function calculates contact map for current section  *
*                          *
*  DoSection() takes no arguments               *
*  DoSection() returns no value              *
*                          *
************************************************************************/
void DoSection_Line (win)
WINDOW win;
{
   register int l,i,m;
   OBJECT *event;
   STRATIGRAPHY_OPTIONS *stratOptions = NULL;
   COLOR layerColors[100];
   int ncon, j, k, red, green, blue;
   unsigned int flavor;
   int nx, ny, cyphno= 0, index;
   double dots1;
   double cval[100];  /* contour values*/
   int numEvents = (int) countObjects(NULL_WIN);
   int bar=0;
   unsigned char cypher[200][ARRAY_LENGTH_OF_STRAT_CODE];
   double ***dots;
   struct story **histoire;
   double **map72;           /* second map for topo/line maps */
   double **map;             /* array of map positions for line maps */
   double absx, absy, absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   
   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;
    
   nx=71;                                                                     
   ny=36;                                                                     

   DEBUG(printf("\nDoSection - Line - Start");)
   if ((histoire = (struct story **) strstomat(0,nx,0,ny))==0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }

   if ((dots = (double ***) qdtrimat(0,nx,0,ny,0,3))==0L)
   {  
      xvt_dm_post_error("Not enough memory, try closing some windows");
      free_strstomat(histoire,0,nx,0,ny);
      return;
   }

   if ((map = (double **) dmatrix(0,71,0,51)) == 0L)
   {  
      xvt_dm_post_error("Not enough memory, try closing some windows");
      free_strstomat(histoire,0,nx,0,ny);
      freeqdtrimat(dots,0,nx,0,ny,0,3); 
      return;
   }

   if ((map72 = (double **) dmatrix(0,72,0,52)) == 0L)
   {  
      xvt_dm_post_error("Not enough memory, try closing some windows");
      free_strstomat(histoire,0,nx,0,ny);
      freeqdtrimat(dots,0,nx,0,ny,0,3); 
      free_dmatrix(map, 0, 71, 0, 51);
      return;
   }
                /* make sure all the drawing goes into this window */
   setCurrentDrawingWindow (win);

   initLongJob (0, 71*(countObjects(NULL_WIN)+1), "Calculating Section...", NULL);

                            /* create the dot map for the section */
   DEBUG(printf("\nDoSection - Line - ldotmp");)
   ldotmp(dots, histoire, SECTION, cypher, &cyphno, 36, 71);

   mplot(4.0,3.3,-3);                                                    
   for (j = 1; j <= cyphno; j++) 
   {
      DEBUG(printf("\nDoSection - Line - j = %d of %d",j, cyphno);)
      taste(numEvents, &(cypher[j][0]), &flavor, &index);
      if (flavor != IGNEOUS_STRAT)
      {                                                 
         for (k = 1; k <= 36; k++)
         {
            for (l = 1; l <= 71; l++)
            {
               dots1 = dots[l][k][3];
               if (coincide(histoire[l][k].sequence, &(cypher[j][0])))
                  map[l][k] = dots1;
               else
                  map[l][k] = 1.0e8;
            }
         }
         DEBUG(printf("\nDoSection - flavor = %d",flavor);)

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
               getStratigraphyOptionsStructure((OBJECT *)
                                                     nthObject(NULL_WIN, 0))))
            return;  /* not even a base strat is defined yet */
         }

         for (m = 0; m < 100; m++)
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
         DEBUG(printf("\nDoSection - mcontr");)
         mcontr(map, nx-1, ny-1, cval, ncon, 1.0e6, 0.2, 1.3, 1.0, FALSE,
                flavor, FALSE, layerColors);
      }

      DEBUG(printf("\nDoSection - Seting dots1 and map");)
/*
      for (k=1;k<=36;k++) 
      {
         for (l=1;l<=71;l++) 
         {                                                        
            dots1=dots[l][k][3];
            if (lineCoincide(histoire[l][k].sequence, &(cypher[j][0])))
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
      mcontr(map72, (nx-1)/ratio, (ny-1)/ratio, cval, ncon,
                                  1.0e9, 0.2*ratio, 1.3, 1.0, FALSE,
                                  flavor, FALSE, layerColors);
*/
      penSize(1,1);    

      for (i = 0; i < (71/cyphno); i++)
         incrementLongJob (INCREMENT_JOB);
  } 
  contour_boundaries(histoire, nx-1, ny-1, 1.3, 1.0);
  PlotSectionLegend (40, 20, absx, absy, absz, geologyOptions.welllogDeclination);
  freeqdtrimat(dots,0,nx,0,ny,0,3); 
  free_strstomat(histoire,0,nx,0,ny);
  free_dmatrix(map, 0, 71, 0, 51);
  free_dmatrix(map72, 0, 72, 0, 52);

  finishLongJob ();

  DEBUG(printf("\nDoSection - Line - END");)
}


/* ======================================================================

                      ** PLEASE READ AND NOTE **

     This file is used for both the sequential and parallel versions
     of Noddy, MAKE SURE any changes that are made are compatible
     with both version of Noddy.

     You may use the _MPL defined symbol to conditionally compile
     code that is only associated with a particular version of Noddy.

     _MPL is defined only for the MASPAR Parallel verion of Noddy.

                      THANK-YOU FOR YOUR ATTENTION

====================================================================== */
#include <math.h>
#ifndef _MPL
#include "xvt.h"
#endif
#include "noddy.h"
#include "titles.h"

#define DEBUG(X)    

#define VERSION_TOLERANCE 0.01
                 /* ********************************** */
                 /* External Globals used in this file */
extern PROJECT_OPTIONS projectOptions;
extern BLOCK_VIEW_OPTIONS *blockViewOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern double iscale; /* scaling factor for geology */
extern FILE_SPEC topoFileSpec;
extern double minTopoValue, maxTopoValue;
extern double **topographyMap;
extern int TopoRow, TopoCol;
extern int batchExecution;
extern COLOR backgroundColor;

                 /* ************************* */
                 /* Globals used in this file */
int load_window_positions ();
int load_icon_positions ();
#if XVT_CC_PROTO
static int loadHistory (FILE *, double, int, int, int);
int loadBlockOpts (FILE *, double);
int loadGeoOpts (FILE *, double);
int loadGpsOpts (FILE *, double);
int load3dOpts (FILE *, double);
int loadProjOpts (FILE *, double);
extern int loadStratigraphy (FILE *, double, STRATIGRAPHY_OPTIONS *);
extern int loadFold (FILE *, double, FOLD_OPTIONS *);
extern int loadFault (FILE *, double, FAULT_OPTIONS *);
extern int loadUnconformity (FILE *, double, UNCONFORMITY_OPTIONS *);
extern int loadShearZone (FILE *, double, SHEAR_OPTIONS *);
extern int loadDyke (FILE *, double, DYKE_OPTIONS *);
extern int loadPlug (FILE *, double, PLUG_OPTIONS *);
extern int loadStrain (FILE *, double, STRAIN_OPTIONS *);
extern int loadTilt (FILE *, double, TILT_OPTIONS *);
extern int loadLineation (FILE *, double, LINEATION_OPTIONS *);
extern int loadFoliation (FILE *, double, FOLIATION_OPTIONS *);
extern int loadImport (FILE *, double, IMPORT_OPTIONS *);
extern int loadGeneric (FILE *, double, GENERIC_OPTIONS *);
extern int getFloatingMenuShape (WINDOW, int *, int *);
#else
extern int getFloatingMenuShape ();
#endif


int readInHist (in, version)
FILE *in;
double version;
{
   WINDOW win = (WINDOW) getEventDrawingWindow ();
   int numEvents;
   int baseHeight, baseWidth, baseColumn;
   OBJECT *p;

   numEvents = (int) totalObjects(win);

              /* append the  position of the events if we already have
              ** same defined */
   baseHeight = GRID_HEIGHT / 2;
   if (numEvents > 0)
   {         /* NB. all the events are so spaced an extra place
             ** because of the space taken by the ignored stratigraphy */
      p = (OBJECT *) nthObject (win, numEvents-1);
      baseWidth = p->bound.left;
      baseColumn = p->column - 1;
   }
   else
   {
      baseWidth = GRID_WIDTH / 2;
      baseColumn = 0;
   }

   DEBUG(printf("\n History Version = %f",version);)
   if (!loadHistory (in, version, numEvents, baseWidth, baseHeight))
   {
      if (batchExecution)
         fprintf(stderr, "Error Loading Event History\n");
      else
         xvt_dm_post_error("Error Loading Event history");
      return (FALSE);
   }

            /* only load options if we didn't have anything already */
   if (numEvents == 0)
   {
      if (version > 5.01)
         loadBlockOpts(in, version);
      loadGeoOpts(in, version);
      loadGpsOpts(in, version);
      load3dOpts(in, version);
      if (version > (3.0 + VERSION_TOLERANCE))
         loadProjOpts(in, version);
      if (version > 5.22)
         load_window_positions (in, version);
   }

   if (!batchExecution)
   {
      if (!load_icon_positions (in, version, baseWidth, baseHeight,
                                             baseColumn, numEvents))
      {
         tidyObjects (win);  /* fit it all together correctly */
      }
   }

   return (TRUE);
}

static int loadHistory (in, version, numEvents, baseWidth, baseHeight)
FILE *in;
double version;
int numEvents;
int baseWidth, baseHeight;
{
   static char *eventTypes[] = {
     "STRATIGRAPHY",          "FOLD",             "FAULT",
     "UNCONFORMITY",          "SHEAR_ZONE",       "DYKE",
     "PLUG",                  "STRAIN",           "TILT",
     "FOLIATION",             "LINEATION",        "IMPORT",
     "STOP",                  "GENERIC",          ""
   };
   WINDOW win = (WINDOW) getEventDrawingWindow ();
   WINDOW_INFO *wip;
   OBJECT *p, *head = NULL, *last;
   int numEventsInFile, event;
   int type;
   char eventType[100], text[OBJECT_TEXT_LENGTH + 10];


   copyUndoPositions ((WINDOW_INFO *) NULL, FALSE);

   loadit(in, "%d",(void *) &numEventsInFile);
/*
   if(numEventsInFile > 5 && DEMO_VERSION)
   {
      if (batchExecution)
         fprintf(stderr, "Sorry, only 5 events allowed in demo version");
      else
         xvt_dm_post_error("Sorry, only 5 events allowed in demo version");
      return (FALSE);
   }
   else
*/
   {
      for (event = 0; event < numEventsInFile; event++)
      {
         loadit(in, "%s",(void *) eventType);
         DEBUG(printf("\nEvent = %d of %d (%s)",event, numEventsInFile, eventType);)
                               /* get the number identifing this event */
         if (strcmp ("KINK", eventType) == 0)
            type = SHEAR_ZONE;
         else
         {
            for (type = 0; strlen(eventTypes[type]); type++)
            {
               if (strcmp (eventTypes[type], eventType) == 0)
                  break;
            }
         }
         if (strlen(eventTypes[type]) == 0)
            return (FALSE);

         DEBUG(printf("\nEventType = %d (%s)",type,eventType);)
                                  /* malloc the basic structure */
         if (!(p = (OBJECT *) xvt_mem_zalloc(sizeof(OBJECT))))
         {
            if (batchExecution)
               fprintf (stderr, "Not Enough Memory to load Events");
            else
               xvt_dm_post_error("Not Enough Memory to load Events");
            return (FALSE);
         }
         p->shape = type;
         p->drawEvent = TRUE;      p->selected = TRUE;
         p->newObject = FALSE;     p->next = NULL;

         switch(p->shape)
         {
            case STRATIGRAPHY:
            {
               STRATIGRAPHY_OPTIONS *options;

                            /* we must always read the stratigraphy even
                            ** if we already have one so we can get past it */
               options = (STRATIGRAPHY_OPTIONS *)
                                      xvt_mem_zalloc(sizeof (STRATIGRAPHY_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Stratigraphy Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Stratigraphy Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadStratigraphy (in, version, options);

               if (numEvents > 0)  /* if we dont have one then load this one */
               {
                  xvt_mem_free ((char *) p->options);
                  xvt_mem_free ((char *) p);
                  p = NULL;
               }
               break;
            }
            case FOLD:
            {
               FOLD_OPTIONS *options;

               options = (FOLD_OPTIONS *) xvt_mem_zalloc(sizeof (FOLD_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Fold Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Fold Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadFold (in, version, options);
               break;
            }
            case FAULT:
            {
               FAULT_OPTIONS *options;

               options = (FAULT_OPTIONS *) xvt_mem_zalloc(sizeof (FAULT_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Fault Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Fault Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadFault (in, version, options);
               break;
            }
            case UNCONFORMITY:
            {
               UNCONFORMITY_OPTIONS *options;

               options = (UNCONFORMITY_OPTIONS *)
                                    xvt_mem_zalloc(sizeof (UNCONFORMITY_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Unconformity Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Unconformity Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadUnconformity (in, version, options);
               break;
            }
            case SHEAR_ZONE:
            {
               SHEAR_OPTIONS *options;

               options = (SHEAR_OPTIONS *) xvt_mem_zalloc(sizeof (SHEAR_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Shear Zone Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Shear Zone Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadShearZone (in, version, options);
               break;
            }
            case DYKE:
            {
               DYKE_OPTIONS *options;

               options = (DYKE_OPTIONS *) xvt_mem_zalloc(sizeof (DYKE_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Dyke Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Dyke Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadDyke (in, version, options);
               break;
            }
            case PLUG:
            {
               PLUG_OPTIONS *options;

               options = (PLUG_OPTIONS *) xvt_mem_zalloc(sizeof (PLUG_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Plug Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Plug Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadPlug (in, version, options);
               break;
            }
            case STRAIN:
            {
               STRAIN_OPTIONS *options;

               options = (STRAIN_OPTIONS *) xvt_mem_zalloc(sizeof (STRAIN_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Strain Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Strain Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadStrain (in, version, options);
               break;
            }
            case TILT:
            {
               TILT_OPTIONS *options;

               options = (TILT_OPTIONS *) xvt_mem_zalloc(sizeof (TILT_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Tilt Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Tilt Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadTilt (in, version, options);
               break;
            }
            case LINEATION:
            {
               LINEATION_OPTIONS *options;

               options = (LINEATION_OPTIONS *)
                                    xvt_mem_zalloc(sizeof (LINEATION_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Lineation Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Lineation Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadLineation (in, version, options);
               break;
            }
            case FOLIATION:
            {
               FOLIATION_OPTIONS *options;

               options = (FOLIATION_OPTIONS *)
                                    xvt_mem_zalloc(sizeof (FOLIATION_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Foliation Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Foliation Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadFoliation (in, version, options);
               break;
            }
            case IMPORT:
            {
               IMPORT_OPTIONS *options;

               options = (IMPORT_OPTIONS *)
                                    xvt_mem_zalloc(sizeof (IMPORT_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Import Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Import Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadImport (in, version, options);
               break;
            }
            case GENERIC:
            {
               GENERIC_OPTIONS *options;

               options = (GENERIC_OPTIONS *)
                                    xvt_mem_zalloc(sizeof (GENERIC_OPTIONS));
               if (!options)
               {
                  if (batchExecution)
                     fprintf (stderr, "Not Enough Memory to Load Generic Event");
                  else
                     xvt_dm_post_error("Not Enough Memory to Load Generic Event");
                  return (FALSE);
               }
               p->options = (char *) options;
               setDefaultOptions (p);
               loadGeneric (in, version, options);
               break;
            }
            case STOP:
            {
               p->options = NULL;
               break;
            }
            default:
               break;
         }
         strcpy (text, "");  /* so a empty string does not get the last one */
         loadit(in, "%s",(void *) text);
         if (p)
         {
            strcpy (p->text, text);
            p->row = 1;
            p->column = numEvents + event + 1;
            p->bound.top = baseHeight;
            p->bound.bottom = p->bound.top + ICON_SIZE + 2;
            p->bound.left = event*GRID_WIDTH + baseWidth;
            p->bound.right = p->bound.left + ICON_SIZE + 2;
            gridCorrectRect (&(p->bound), &(p->bound));
            if (head)    /* keep a list of all the events read in */
            {
               last->next = p;
               last = p;
            }
            else
            {
               head = p;
               last = p;
            }
            copyUndoAdditions (p, FALSE);
         }
      }
             /* add the events we have loaded onto the events alread there */
      DEBUG(printf("\n **** PUTING IT ON THE EVENTS WE HAVE");)
      if (numEvents)
      {
         last = (OBJECT *) nthObject (win, numEvents-1);
         last->next = head;
      }
      else
      {
         wip = (WINDOW_INFO *) get_win_info(win);
         wip->head = head;
      }
   }
   reorderObjects (win);
   if (!batchExecution)
      xvt_dwin_invalidate_rect (win, NULL);
   updateFloatingMenu (getFloatingMenuWindow(), get_win_info(win)->head,
                                                get_win_info(win)->tool);
   DEBUG(printf("\nLOAD_HIST: FINISHED");)

   return (TRUE);
}

int loadBlockOpts(status, version)
FILE *status;
double version;
{
   int numViews, currentView, view;
   double originX, originY, originZ;
   double lengthX, lengthY, lengthZ;
   double geolCube, geopCube;
   char name[OBJECT_TEXT_LENGTH];
   BLOCK_VIEW_OPTIONS *viewOptions = NULL, *defaultView = NULL;
   
   loadit(status,"%d",(void *) &numViews);
   loadit(status,"%d",(void *) &currentView);

   for (view = 0; view < numViews; view++)
   {
      loadit(status,"%s",(void *) name);
   
      loadit(status,"%lf",(void *) &originX);
      loadit(status,"%lf",(void *) &originY);
      loadit(status,"%lf",(void *) &originZ);
   
      loadit(status,"%lf",(void *) &lengthX);
      loadit(status,"%lf",(void *) &lengthY);
      loadit(status,"%lf",(void *) &lengthZ);
   
      loadit(status,"%lf",(void *) &geolCube);
      loadit(status,"%lf",(void *) &geopCube);
   
      if (blockViewOptions && (strcmp(name, blockViewOptions->viewName) == 0))
		{
			blockViewOptions->originX = originX;
			blockViewOptions->originY = originY;
			blockViewOptions->originZ = originZ;
			blockViewOptions->lengthX = lengthX;
			blockViewOptions->lengthY = lengthY;
			blockViewOptions->lengthZ = lengthZ;
			blockViewOptions->geologyCubeSize = geolCube;
			blockViewOptions->geophysicsCubeSize = geopCube;
			viewOptions = blockViewOptions;
		}
		else
		{
			viewOptions = newViewOptions (name, originX, originY, originZ,
                            lengthX, lengthY, lengthZ,
                            geolCube, geopCube);
		}
      if (view == currentView)
         defaultView = viewOptions;
   }
   
   if (defaultView)  /* make sure the right View will be selected */
      blockViewOptions = defaultView;
   
   return (TRUE);
}

int loadGeoOpts(status, version)
FILE *status;
double version;
{
   char temp[255];
   int jj, kk, tempInt;
   double originX, originY, originZ, tempDouble;
   DEBUG(printf("\nLOAD_GEOLOGY: STARTED");)

   if (version < 5.10)
   {
      loadit(status,"%lf",(void *) &originX);
      loadit(status,"%lf",(void *) &originY);
      loadit(status,"%lf",(void *) &originZ);
      originZ += BLOCK_DIAGRAM_SIZE_Z;
   }
   
   loadit(status,"%lf",(void *) &iscale);
   loadit(status,"%lf",(void *) &geologyOptions.welllogDeclination);
   loadit(status,"%lf",(void *) &geologyOptions.welllogDepth);
   loadit(status,"%lf",(void *) &geologyOptions.welllogAngle);

   if (version > 7.02)
	{
		loadit(status,"%lf", &geologyOptions.boreholeX);
		loadit(status,"%lf", &geologyOptions.boreholeY);
		loadit(status,"%lf", &geologyOptions.boreholeZ);
		loadit(status,"%lf", &geologyOptions.boreholeDecl);
		loadit(status,"%lf", &geologyOptions.boreholeDip);
		loadit(status,"%lf", &geologyOptions.boreholeLength);

		loadit(status,"%lf", &geologyOptions.sectionX);
		loadit(status,"%lf", &geologyOptions.sectionY);
		loadit(status,"%lf", &geologyOptions.sectionZ);
		loadit(status,"%lf", &geologyOptions.sectionDecl);
		loadit(status,"%lf", &geologyOptions.sectionLength);
		loadit(status,"%lf", &geologyOptions.sectionHeight);
	}
	
   loadit(status,"%s",(void *) temp);
   if (strcmp (temp, "TRUE") == 0)
   {
      if (version > 3.1)
      {
         geologyOptions.useTopography = TRUE;
      }
      else
      {
         if (batchExecution)
            fprintf(stderr, "Topography was read from file when history saved.");
         else
            xvt_dm_post_error("Topography was read from file when history saved.");
         geologyOptions.useTopography = FALSE;
      }
   }
   else
      geologyOptions.useTopography = FALSE;

   if (version > 3.1)
   {
      char fileName[100];
      char pathName[200];
      
      loadit(status,"%s",(void *) fileName);
      loadit(status,"%s",(void *) pathName);
      strcpy (topoFileSpec.name, fileName);
      xvt_fsys_convert_str_to_dir (pathName, &(topoFileSpec.dir));
      
      loadit(status,"%lf",(void *) &geologyOptions.topoScaleFactor);
      loadit(status,"%lf",(void *) &geologyOptions.topoOffsetFactor);
      loadit(status,"%lf",(void *) &geologyOptions.topoLowContour);
      loadit(status,"%lf",(void *) &geologyOptions.topoContourInterval);
                             /* Load and scale the topography file */
      if (geologyOptions.useTopography)
      {
         if (!loadTopographyFile (&topoFileSpec))
            geologyOptions.useTopography = FALSE;
         else if (topographyMap)
         {
            for (jj = 1; jj <= TopoRow; jj++)
            {
               for (kk = 1; kk <= TopoCol; kk++)
               {
                  topographyMap[kk][jj] = geologyOptions.topoOffsetFactor
                       + (topographyMap[kk][jj] * geologyOptions.topoScaleFactor);
               }
            }
            minTopoValue = geologyOptions.topoOffsetFactor
                               + (minTopoValue*geologyOptions.topoScaleFactor);
            maxTopoValue = geologyOptions.topoOffsetFactor
                               + (maxTopoValue*geologyOptions.topoScaleFactor);
         }
      }

      if (version < 4.15)
      {
         loadit(status,"%d",(void *) &projectOptions.movieFramesPerEvent);
         loadit(status,"%lf",(void *) &projectOptions.moviePlaySpeed);
         projectOptions.movieType = BLOCK_DIAGRAM_MOVIE;
      }
   }
          
   if (version < 3.3) /* Pattern fills not supported in 3.3 and after */
   {
      loadit(status,"%s",(void *) temp);  /* just read and ignore */
   }

   loadit(status,"%s",(void *) temp);
   if (strcmp (temp, "TRUE") == 0)
      geologyOptions.calculateChairDiagram = TRUE;
   else
      geologyOptions.calculateChairDiagram = FALSE;

   loadit(status,"%lf",(void *) &geologyOptions.chair_X);
   loadit(status,"%lf",(void *) &geologyOptions.chair_Y);
   loadit(status,"%lf",(void *) &geologyOptions.chair_Z);

   if (version < 5.22)
   {
      loadit(status,"%lf",(void *) &tempDouble);  /* geologyOptions.linemap_X */
      loadit(status,"%lf",(void *) &tempDouble);  /* geologyOptions.linemap_X */
   }
   
   if (version < 5.10)
   {
      loadit(status,"%d",(void *) &tempInt);
   
      if (tempInt == 4) /* BLOCK_HIGH */
         tempInt = 50;
      else if (tempInt == 3) /* BLOCK_MEDHIGH */
         tempInt = 100;
      else if (tempInt == 2) /* BLOCK_MEDIUM */
         tempInt = 200;
      else if (tempInt == 1) /* BLOCK_MEDLOW */
         tempInt = 350;
      else  if (tempInt == 0) /* BLOCK_LOW */
         tempInt = 500;
   
      if (tempInt <= 0)
         tempInt = (int) DEFAULT_VIEW_GEOL_CUBE;
      newViewOptions ("Geology View", originX, originY, originZ, 10000.0, 7000.0, 5000.0,
                                      (double) tempInt, (double) tempInt);
   
      if (version > 3.0)
         loadit(status,"%lf",(void *) &tempDouble);  /* Cube Scale */
   }

   if (version < 3.7)
   {
      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "TRUE") == 0)
         projectOptions.newWindowEachStage = TRUE;
      else
         projectOptions.newWindowEachStage = FALSE;
   }

   DEBUG(printf("\nLOAD_GEOLOGY: FINISHED");)

   return (TRUE);
}

int loadGpsOpts (status,version)
FILE *status;
double version;
{
   char temp[255];
   double dudDouble, cubeSize;
   double lowerSouthWest_X, lowerSouthWest_Y, lowerSouthWest_Z;
   double upperNorthEast_X, upperNorthEast_Y, upperNorthEast_Z;
     
   DEBUG(printf("\nLOAD_GEOPHYSICS: STARTED");)
   if (version < 5.10)
      loadit(status,"%lf",(void *) &cubeSize);
      
   loadit(status,"%d",(void *) &geophysicsOptions.calculationRange);
   if (version < 5.10)  /* old historys the range was in cubes not meters so convert it */
      geophysicsOptions.calculationRange = (int) (geophysicsOptions.calculationRange*cubeSize);
   
   if (version > 3.3)
      loadit(status,"%lf",(void *) &geophysicsOptions.declination);
   else
      geophysicsOptions.declination = 0.0;

   if (version < 5.10)
   {
      if (cubeSize <= 0.0)
         cubeSize = DEFAULT_VIEW_GEOP_CUBE;
        
      loadit(status,"%lf",(void *) &lowerSouthWest_X);
      loadit(status,"%lf",(void *) &lowerSouthWest_Y);
      loadit(status,"%lf",(void *) &lowerSouthWest_Z);
      loadit(status,"%lf",(void *) &upperNorthEast_X);
      loadit(status,"%lf",(void *) &upperNorthEast_Y);
      loadit(status,"%lf",(void *) &upperNorthEast_Z);
      newViewOptions ("Geophysics View",
            lowerSouthWest_X, lowerSouthWest_Y, upperNorthEast_Z,
            (upperNorthEast_X-lowerSouthWest_X),
            (upperNorthEast_Y-lowerSouthWest_Y),
            (upperNorthEast_Z-lowerSouthWest_Z), cubeSize, cubeSize);
   }
        
   if (version < 4.05)
   {
                               /* geophysicsOptions.orientation */
      loadit(status,"%lf",(void *) &dudDouble);
                               /* geophysicsOptions.length */
      loadit(status,"%lf",(void *) &dudDouble);
   }
   loadit(status,"%lf",(void *) &geophysicsOptions.inclination);
   loadit(status,"%lf",(void *) &geophysicsOptions.intensity);

   if (version > 6.13)
	{
		loadit(status,"%s",(void *) temp);
		if (strcmp (temp, "FIXED") == 0)
			geophysicsOptions.fieldType = FIXED_FIELD;
		else
			geophysicsOptions.fieldType = VARIABLE_FIELD;
		loadit(status,"%lf",(void *) &geophysicsOptions.xPos);
		loadit(status,"%lf",(void *) &geophysicsOptions.yPos);
		loadit(status,"%lf",(void *) &geophysicsOptions.zPos);

		loadit(status,"%lf",(void *) &geophysicsOptions.inclinationOri);
		loadit(status,"%lf",(void *) &geophysicsOptions.inclinationChange);
		loadit(status,"%lf",(void *) &geophysicsOptions.intensityOri);
		loadit(status,"%lf",(void *) &geophysicsOptions.intensityChange);
		loadit(status,"%lf",(void *) &geophysicsOptions.declinationOri);
		loadit(status,"%lf",(void *) &geophysicsOptions.declinationChange);
	}
	else
		geophysicsOptions.fieldType = FIXED_FIELD;
   
   loadit(status,"%lf",(void *) &geophysicsOptions.altitude);

   loadit(status,"%s",(void *) temp);
   if (strcmp (temp, "TRUE") == 0)
      geophysicsOptions.calculationAltitude = AIRBORNE;
   else
      geophysicsOptions.calculationAltitude = SURFACE;

   if (version < 3.7)
   {                    /* calculationType of MAP or PROFILE
                        ** is no longer used */
      loadit(status,"%s",(void *) temp);  
   }
   
   if (version > 3.6)
   {
      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "SPATIAL") == 0)
         geophysicsOptions.calculationMethod = SPATIAL;
      else if (strcmp (temp, "SPECTRAL") == 0)
         geophysicsOptions.calculationMethod = SPECTRAL;
      else /* Full */
         geophysicsOptions.calculationMethod = SPATIAL_FULL;
      if (version > 5.32)
      {
         loadit(status,"%s",(void *) temp);
         if (strcmp (temp, "RAMP_PADDING") == 0)
            geophysicsOptions.spectralPaddingType = RAMP_PADDING;
         else if (strcmp (temp, "FENCE_MEAN_PADDING") == 0)
            geophysicsOptions.spectralPaddingType = FENCE_MEAN_PADDING;
         else if (strcmp (temp, "FENCE_VALUE_PADDING") == 0)
            geophysicsOptions.spectralPaddingType = FENCE_VALUE_PADDING;
         else if (strcmp (temp, "SET_MEAN_PADDING") == 0)
            geophysicsOptions.spectralPaddingType = SET_MEAN_PADDING;
         else if (strcmp (temp, "SET_VALUE_PADDING") == 0)
            geophysicsOptions.spectralPaddingType = SET_VALUE_PADDING;
         else if (strcmp (temp, "RECLECTION_PADDING") == 0)
            geophysicsOptions.spectralPaddingType = RECLECTION_PADDING;
         else
            geophysicsOptions.spectralPaddingType = RECLECTION_PADDING;

         loadit(status,"%d",(void *) &geophysicsOptions.spectralFence);
         loadit(status,"%d",(void *) &geophysicsOptions.spectralPercent);
      }
   }
   
   if (version > 4.00)
   {
      loadit(status,"%lf",(void *) &geophysicsOptions.constantBoxDepth);
      loadit(status,"%lf",(void *) &geophysicsOptions.cleverBoxRatio);
   }

   if (version < 4.0)
   {
      loadit(status,"%s",(void *) temp);  /* erm output */
   }

   if (version > 3.1)
   {
      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "TRUE") == 0)
         geophysicsOptions.deformableRemanence = TRUE;
      else
         geophysicsOptions.deformableRemanence = FALSE;

      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "TRUE") == 0)
         geophysicsOptions.deformableAnisotropy = TRUE;
      else
         geophysicsOptions.deformableAnisotropy = FALSE;
   }

	if (version > 7.00)
	{
      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "TRUE") == 0)
         geophysicsOptions.magneticVectorComponents = TRUE;
      else
         geophysicsOptions.magneticVectorComponents = FALSE;

      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "TRUE") == 0)
         geophysicsOptions.projectVectorsOntoField = TRUE;
      else
         geophysicsOptions.projectVectorsOntoField = FALSE;
	}
	else
	{
      geophysicsOptions.magneticVectorComponents = FALSE;
      geophysicsOptions.projectVectorsOntoField = TRUE;
	}

	if (version > 7.03)
	{
      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "TRUE") == 0)
         geophysicsOptions.padWithRealGeology = TRUE;
      else
         geophysicsOptions.padWithRealGeology = FALSE;
	}
	else
	{
      geophysicsOptions.padWithRealGeology = TRUE;
	}

   if (version > 3.71)
   {
      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "TRUE") == 0)
         geophysicsOptions.drapedSurvey = TRUE;
      else
         geophysicsOptions.drapedSurvey = FALSE;
   }
   DEBUG(printf("\nLOAD_GEOPHYSICS: FINISHED");)
   return (TRUE);
}

int load3dOpts (status, version)
FILE *status;
double version;
{
   int num;

   DEBUG(printf("\nLOAD_3d: STARTED");)
   loadit(status,"%lf",(void *) &threedViewOptions.declination);
   loadit(status,"%lf",(void *) &threedViewOptions.azimuth);
   loadit(status,"%lf",(void *) &threedViewOptions.scale);
   if (version < 5.20)
   {
      if (threedViewOptions.scale > 100.0)
         threedViewOptions.scale = 1.0;
   }

   loadit(status,"%lf",(void *) &threedViewOptions.offset_X);
   loadit(status,"%lf",(void *) &threedViewOptions.offset_Y);
   loadit(status,"%lf",(void *) &threedViewOptions.offset_Z);

   loadit(status,"%d",(void *) &num);
   //threedViewOptions.fillType = num;//mwj_hack
   threedViewOptions.fillType = 3;//mwj_hack so always DXF 3D Face in batch mode
   DEBUG(printf("\nLOAD_3d: FINISHED");)
   return (TRUE);
}

int loadProjOpts (status, version)
FILE *status;
double version;
{
   char temp[255];
   int value;

   DEBUG(printf("\nLOAD_PROJ: STARTED");)
   loadit(status,"%s",(void *) temp);
   if ((strcmp (temp, "SI") == 0) || (strcmp (temp, "0") == 0))
      projectOptions.susceptibilityUnits = SI;
   else
      projectOptions.susceptibilityUnits = CGS;
   loadit(status,"%d",(void *) &projectOptions.geophysicalCalc);
   loadit(status,"%s",(void *) temp);
   if ((strcmp (temp, "REMOTE_JOB") == 0) || (strcmp (temp, "PARALLEL") == 0))
      projectOptions.calculationType = REMOTE_JOB;
   else
      projectOptions.calculationType = LOCAL_JOB;
   if (version < 3.7)
   {
      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "SPATIAL") == 0)
         geophysicsOptions.calculationMethod = SPATIAL;
      else
         geophysicsOptions.calculationMethod = SPECTRAL;
   }
   loadit(status,"%d",(void *) &value);
   projectOptions.lengthScale = value;
   
   if (version > 3.7)
      loadit(status,"%lf",(void *) &projectOptions.printScalingFactor);
   if (version > 4.11)
      loadit(status,"%lf",(void *) &projectOptions.imageScalingFactor);
   
   if (version > 3.6)
   {
      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "TRUE") == 0)
         projectOptions.newWindowEachStage = TRUE;
      else
         projectOptions.newWindowEachStage = FALSE;
   }
   
   if (version > 5.30)  /* BackgroundColor for visulisations */
   {
      int red, green, blue;
      
      loadit(status,"%d",(void *) &red);
      loadit(status,"%d",(void *) &green);
      loadit(status,"%d",(void *) &blue);
      
      backgroundColor = XVT_MAKE_COLOR(red,green,blue);
   }

   loadit(status,"%s",(void *) projectOptions.internetAddress);
   loadit(status,"%s",(void *) projectOptions.accountName);
   loadit(status,"%s",(void *) projectOptions.noddyPath);

   if (version > 7.01)
	{
		char *helpPathOverride;
		if (helpPathOverride = getenv("NODDY_HELP_PATH"))
			strcpy(projectOptions.helpPath, helpPathOverride);
		else
			loadit(status,"%s",(void *) projectOptions.helpPath);
	}

   if (version > 4.11)
   {
      loadit(status,"%d",(void *) &projectOptions.movieFramesPerEvent);
      loadit(status,"%lf",(void *) &projectOptions.moviePlaySpeed);
      loadit(status,"%d",(void *) &value);
      projectOptions.movieType = value;
   }

   if (version > 4.15)
   {
      if (version > 5.1)
      {
         loadit(status,"%s",(void *) temp);
         if (strcmp (temp, "ABSOLUTE_CLIPPING") == 0)
            projectOptions.gravityDisplay.clippingType == ABSOLUTE_CLIPPING;
         else
            projectOptions.gravityDisplay.clippingType == RELATIVE_CLIPPING;
      }
      else
         projectOptions.gravityDisplay.clippingType == RELATIVE_CLIPPING;
         
      loadit(status,"%lf",(void *) &projectOptions.gravityDisplay.minClip);
      loadit(status,"%lf",(void *) &projectOptions.gravityDisplay.maxClip);
      if (projectOptions.gravityDisplay.minClip < 0.0)
         projectOptions.gravityDisplay.minClip = 0.0;
      if (projectOptions.gravityDisplay.minClip > 100.0)
         projectOptions.gravityDisplay.minClip = 100.0;

      if (projectOptions.gravityDisplay.maxClip < 0.0)
         projectOptions.gravityDisplay.maxClip = 0.0;
      if (projectOptions.gravityDisplay.maxClip > 100.0)
         projectOptions.gravityDisplay.maxClip = 100.0;

      loadit(status,"%s",(void *) temp);
      if (strcmp (temp, "CONTOUR") == 0)
         projectOptions.gravityDisplay.display = CONTOUR_IMAGE;
      else if (strcmp (temp, "PSEUDO_COLOR") == 0)
         projectOptions.gravityDisplay.display = PSEUDO_COLOR;
      else  /* GREY */
         projectOptions.gravityDisplay.display = GREY_COLOR;
      loadit(status,"%d",(void *) &projectOptions.gravityDisplay.numContours);

      if (version > 5.20)  /* load Magnetics Display Info seperately */
      {
         loadit(status,"%s",(void *) temp);
         if (strcmp (temp, "ABSOLUTE_CLIPPING") == 0)
            projectOptions.magneticsDisplay.clippingType == ABSOLUTE_CLIPPING;
         else
            projectOptions.magneticsDisplay.clippingType == RELATIVE_CLIPPING;
         
         loadit(status,"%lf",(void *) &projectOptions.magneticsDisplay.minClip);
         loadit(status,"%lf",(void *) &projectOptions.magneticsDisplay.maxClip);
         if (projectOptions.magneticsDisplay.minClip < 0.0)
            projectOptions.magneticsDisplay.minClip = 0.0;
         if (projectOptions.magneticsDisplay.minClip > 100.0)
            projectOptions.magneticsDisplay.minClip = 100.0;
   
         if (projectOptions.magneticsDisplay.maxClip < 0.0)
            projectOptions.magneticsDisplay.maxClip = 0.0;
         if (projectOptions.magneticsDisplay.maxClip > 100.0)
            projectOptions.magneticsDisplay.maxClip = 100.0;

         loadit(status,"%s",(void *) temp);
         if (strcmp (temp, "CONTOUR") == 0)
            projectOptions.magneticsDisplay.display = CONTOUR_IMAGE;
         else if (strcmp (temp, "PSEUDO_COLOR") == 0)
            projectOptions.magneticsDisplay.display = PSEUDO_COLOR;
         else  /* GREY */
            projectOptions.magneticsDisplay.display = GREY_COLOR;
         loadit(status,"%d",(void *) &projectOptions.magneticsDisplay.numContours);
      }
      else
         memcpy (&(projectOptions.magneticsDisplay),
                 &(projectOptions.gravityDisplay), sizeof(IMAGE_DISPLAY_DATA)); 
   }
   
   if (version > 5.31)
   {
      loadit(status,"%lf",(void *) &projectOptions.easting);
      loadit(status,"%lf",(void *) &projectOptions.northing);
   }
   else
   {
      projectOptions.easting = 0.0;
      projectOptions.northing = 0.0;
   }

   DEBUG(printf("\nLOAD_PROJ: FINISHED");)
   return (TRUE);
}

int load_window_positions (in, version)
FILE *in;
double version;
{
   register int winNum;
   int intVal, numWindows;
   RCT position;
   WINDOW win;
   PNT point;
   
   if (version < 5.30)
      return (FALSE);

   loadit(in,"%d",(void *) &numWindows);
   for (winNum = 0; winNum < numWindows; winNum++)
   {
      loadit(in,"%s",(void *) &winPositionOptions.winPositions[winNum].name);
      loadit(in,"%d",(void *) &intVal);
      winPositionOptions.winPositions[winNum].x = intVal;
      loadit(in,"%d",(void *) &intVal);
      winPositionOptions.winPositions[winNum].y = intVal;
      loadit(in,"%d",(void *) &intVal);
      winPositionOptions.winPositions[winNum].width = intVal;
      loadit(in,"%d",(void *) &intVal);
      winPositionOptions.winPositions[winNum].height = intVal;
      
   }

   if (!batchExecution)
   {
      if (getWindowStartPosition (HISTORY_TITLE, &position.left, &position.top, &position.right, &position.bottom, NULL, NULL))
      {
         if (win = (WINDOW) getEventDrawingWindow ())
            xvt_vobj_move (win, &position);
      }
         
      if (getWindowStartPosition (TOOLBAR_TITLE, &point.h, &point.v, NULL, NULL, NULL, NULL))
      {
         if (win = (WINDOW) getFloatingMenuWindow ())
         {
            xvt_vobj_get_outer_rect (win, &position);
            xvt_rect_set_pos(&position, point);
            xvt_vobj_move (win, &position);
         }
      }
   }
      
   return (TRUE);
}


int load_icon_positions (in, version, baseWidth, baseHeight,
                         baseColumn, lastEvent)
FILE *in;
double version;
int baseWidth, baseHeight;
int baseColumn, lastEvent;
{
   WINDOW win;
   OBJECT *p;
   int numEvents, event;
   int positionX, positionY, row, column;
   char lineBuf[100];
   BOOLEAN iconsPresent = FALSE;

   win = (WINDOW) getEventDrawingWindow ();

   while (!iconsPresent && fgets (lineBuf, 100, in))
   {
      if (strcmp (lineBuf,"#Icon Positions\n") == 0)
         iconsPresent = TRUE;
   }

   if (!iconsPresent)
      return (FALSE);

   loadit(in,"%d",(void *) &numEvents);

   if (lastEvent)   /* first was the strat so miss it */
   {                /* when we are appending to a hist */
      loadit(in,"%d",(void *) &row);
      loadit(in,"%d",(void *) &column);
      loadit(in,"%d",(void *) &positionX);
      loadit(in,"%d",(void *) &positionY);
      numEvents--;
   }

   for (event = 0; event < numEvents; event++)
   {
      loadit(in,"%d",(void *) &row);
      loadit(in,"%d",(void *) &column);
      loadit(in,"%d",(void *) &positionX); positionX--;
      loadit(in,"%d",(void *) &positionY); positionY--;
      p = (OBJECT *) nthObject (win, lastEvent + event);
      if (p)
      {
         p->row = row;
         p->column = column + baseColumn;
         p->bound.top = positionY*GRID_HEIGHT + baseHeight;
         p->bound.bottom = p->bound.top + ICON_SIZE + 2;
         p->bound.left = positionX*GRID_WIDTH + baseWidth;
         p->bound.right = p->bound.left + ICON_SIZE + 2;
         gridCorrectRect (&(p->bound), &(p->bound));
      }
   }

   if (version > 3.4)  /* Load menu shape as well */
   {
      int numItems;
      loadit(in,"%d",(void *) &row);
      loadit(in,"%d",(void *) &column);
      numItems = (int) getFloatingMenuShape (NULL_WIN, NULL, NULL);
      if (row*column >= numItems)  /* make sure the icons can fit - otherwise leave it alone */
         setFloatingMenuShape (NULL_WIN, row, column);
   }
   
   return (TRUE);
}


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

#define DEBUG(X)    
#define DEBUG1(X)   

                 /* ********************************** */
                 /* External Globals used in this file */
extern PROJECT_OPTIONS projectOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern double iscale; /* scaling factor for geology */

                 /* ************************* */
                 /* Globals used in this file */      
#if XVT_CC_PROTO
extern int read_hist(char *);
extern int read_strat(FILE *);
extern int readInHist (FILE *, double);
extern int load_folds (FILE *, int, char *, double, FOLD_OPTIONS *);
extern int load_dykes (FILE *, int, char *, double, DYKE_OPTIONS *);
extern int load_faults (FILE *, int, char *, double, FAULT_OPTIONS *);
extern int load_kinks (FILE *, int, char *, double, SHEAR_OPTIONS *);
extern int load_lineations (FILE *, int, char *, double, LINEATION_OPTIONS *);
extern int load_planes (FILE *, int, char *, double, FOLIATION_OPTIONS *);
extern int load_plugs (FILE *, int, char *, double, PLUG_OPTIONS *);
extern int load_tilts (FILE *, int, char *, double, TILT_OPTIONS *);
extern int load_strains (FILE *, int, char *, double, STRAIN_OPTIONS *);
extern int load_unconformities (FILE *, int, char *, double, UNCONFORMITY_OPTIONS *);
int load_strat (FILE *, double, int *, STRATIGRAPHY_OPTIONS [7]);
int load_hist(FILE *, double, int, STRATIGRAPHY_OPTIONS [7]);
int loadtype(FILE *, int *, char *);
int load_geol_opts(FILE *, double);
int load_gps_opts(FILE *, double);
int load_3d_opts(FILE *, double);
#else
extern int read_hist();
extern int read_strat();
extern int readInHist ();
extern int load_folds ();
extern int load_dykes ();
extern int load_faults ();
extern int load_kinks ();
extern int load_lineations ();
extern int load_planes ();
extern int load_plugs ();
extern int load_tilts ();
extern int load_strains ();
extern int load_unconformities ();
int load_strat ();
int load_hist();
int loadtype();
int load_geol_opts();
int load_gps_opts();
int load_3d_opts();
#endif                 
                 
                 
                 
static int typelut[25] = {0,1,1,1,1,4,1,2,2,3,3,4,4,5,6,8,7,7,10,9,0,0,0,0,2};
typedef unsigned char String20[21];
String20 NNames[11] = {  "........",    "Fold",        "Fault",
               "Unconformity",     "Shear_Zone",  "Dyke",
               "Plug",        "Strain", "Tilt",
               "Foliation",   "Lineation"};

int Load_status(fname)
char *fname;
{
   WINDOW win = (WINDOW) getEventDrawingWindow ();
   WINDOW_INFO *wip;
   OBJECT *p;
   FILE *status;
   ASK_RESPONSE response;
   double starttest;
   char endtest[255];
   double version;
   int numEvents = totalObjects(win);  /* every event already there */
   int numStrat;
   STRATIGRAPHY_OPTIONS stratOptions[7];

#ifdef _MPL
   printf ("\nLoading History File '%s'.",fname); fflush (stdout);
#endif

   if (numEvents > 0) /* then add the first strat as our base */
   {
#ifdef _MPL
      fprintf(stderr,"\nERROR: Trying to Load history a second time");
      exit (EXIT_ERROR_VALUE);
#else
      response = xvt_dm_post_ask("Merge", "Overwrite", "Cancel",
             "Are You Sure you wish to merge this with the current History ?");
      if (response == RESP_3)  /* Go no further */
         return (FALSE);
      else if (response == RESP_2)     /* Delete all the Events */
		{
			initProject (FALSE, TRUE);
         /* was deleteSelectedObjects (getEventDrawingWindow (), TRUE); */
		}
#endif
   }

   if (!(status = fopen (fname, "r")))
      return(FALSE);

   loadit(status,"%lf",(void *) &starttest);

   if (starttest != 111.0)
   {
      fclose(status);
      return(read_hist(fname));
   }
   else
   {
      copyit(status, (char *) "%lf", (void *) &version);
      if (version <= VERSION_NUMBER)
      {
         if (version < 3.0)
         {
            if (numEvents == 0) /* then add the first strat as our base */
            {
               if (!(p = (OBJECT *) xvt_mem_alloc(sizeof(OBJECT))))
               {
                  xvt_dm_post_error("Not Enough Memory to load Events");
                  fclose(status);
                  return (FALSE);
               }
               p->shape = STRATIGRAPHY;
               p->options = (char *) &(stratOptions[0]);
               setDefaultOptions (p);
            }
            load_strat(status, version, &numStrat, stratOptions);
            if (numEvents == 0) /* then add the first strat as our base */
            {
               wip = (WINDOW_INFO *) get_win_info(win);
               if (!(p->options = (char *)
                                xvt_mem_alloc(sizeof(STRATIGRAPHY_OPTIONS))))
               {
                  xvt_dm_post_error("Not Enough Memory to load Events");
                  fclose(status);
                  return (FALSE);
               }
               strcpy (p->text, "Strat");
               p->drawEvent = TRUE;
               p->row = 1; p->column = 1;
               p->newObject = FALSE;
               p->selected = TRUE;
               p->next = NULL;
               memcpy (p->options, &(stratOptions[0]),
                                            sizeof(STRATIGRAPHY_OPTIONS));
               wip->head = p;
               tidyObjects (win);

                            /* New History so load in every thing */
               load_hist(status,version, numStrat, stratOptions);
               load_geol_opts(status,version);
               load_gps_opts(status,version);
               load_3d_opts(status,version);
#ifdef _MPL
               printf (" Done."); fflush (stdout);
#endif
            }
            else
            {
                            /* a history already exists so just add **
                            ** the events, use the existing stratigraphy
                            ** and options */
               load_hist(status,version, numStrat, stratOptions);
               fclose(status);
#ifdef _MPL
               printf (" Done."); fflush (stdout);
#endif
               return(TRUE);
            }
         }
         else   /* new History file version for XVT version of Noddy */
         {
            if (!readInHist (status, version))
            {
               fclose(status);
               return(FALSE);
            }
            else
            {
               fclose(status);
#ifdef _MPL
               printf (" Done."); fflush (stdout);
#endif
               return(TRUE);
            }
         }

      }
      else
         xvt_dm_post_error("File version newer than program");

      readGoodLine(status,endtest);
      if (!equalstring2((char *)endtest,"End of Status Report"))
      {
         xvt_dm_post_error("Unable to read file correctly");
         fclose(status);
         return(FALSE);
      }

      fclose(status);
#ifdef _MPL
      printf (" Done."); fflush (stdout);
#endif
      return(TRUE);
   }      
}


int load_strat (status, version, numStrat, stratOptions)
FILE *status;
double version;
int *numStrat;
STRATIGRAPHY_OPTIONS stratOptions[7];
{    
   double value;
   int stages, element;
   int isum;
   int is; /* number of stratigraphies */
   int strasm[7]; /* total number of layers below given stratigraphy */
   int strano[7]; /* number of layers in stratigraphy */
   LAYER_PROPERTIES *properties;
   int colorValue;
          
   loadit(status, "%d\n",(void *) &is);
   DEBUG(printf("\nLOAD_STRAT: %d of them",is);)

   strasm[0] = 0; /* my ??*/
   for (stages = 0, isum=0; stages < is; stages++)
   {
      loadit(status, "%d",(void *) &strano[stages]);

      isum += strano[stages];    /* my ? */    
      strasm[stages] = isum;      /* my ? */

      stratOptions[stages].numLayers = (int) strano[stages];
   }

                                       /* read in each stratigraphy */
   for (stages = 0; stages < is; stages++)
   {
   DEBUG(printf("\nLOAD_STRAT: stages = %d",stages);)
                                       /* loop backwards over each layer */
      for (element = strano[stages]-1; element >= 0; element--)
      {
      DEBUG(printf("\nLOAD_STRAT: element = %d",element);)
         properties = &(stratOptions[stages].properties[element]);

         loaditrock(status,"%s",(void *) properties->unitName);
         loadit(status,"%lf",(void *) &value);
         properties->height = (int)floor(value);
      loadit(status,"%lf",(void *) &(properties->density));
      loadit(status,"%le",(void *) &(properties->sus_X));

         if (version >= 2.0)
         {
         loadit(status,"%le",(void *) &(properties->sus_Y));
         loadit(status,"%le",(void *) &(properties->sus_Z));
            loadit(status,"%lf",(void *) &value);
            properties->anisotropicField = (int) floor (value);

            loadit(status,"%lf",&value);
            properties->remanentMagnetization = (int) floor (value);
         loadit(status,"%le",(void *) &(properties->strength));
         loadit(status,"%le",(void *) &value);
         loadit(status,"%le",(void *) &value);
         loadit(status,"%lf",(void *) &(properties->inclination));
         loadit(status,"%lf",(void *) &(properties->angleWithNorth));
         }
         else
         {
            properties->sus_Y = 0.0;
            properties->sus_Z = 0.0;
            properties->anisotropicField = FALSE;

            properties->remanentMagnetization = FALSE;
            properties->strength = 0.0;
            properties->inclination = 0.0;
            properties->angleWithNorth = 0.0;
         }
      loadit(status,"%d",(void *) &colorValue);
      }
   }

   *numStrat = is;
   DEBUG(printf("\nLOAD_STRAT: Finished numStrat = %d",*numStrat);)
   return (TRUE);
}

int load_hist(status, version, numStrat, stratOptions)
FILE *status;
double version;
int numStrat;
STRATIGRAPHY_OPTIONS stratOptions[7];
{
   WINDOW win = (WINDOW) getEventDrawingWindow ();
   OBJECT *p, *head = NULL, *last;
   int stages,i,type;
   int numEvents, stratNumber = 1;
   char modifiers[255];
   int no;

   numEvents = totalObjects(win);  /* every event already there */
     
   loadit(status,"%d",(void *) &no);
   no++;
/*
   if(no >5 && DEMO_VERSION)
   {
      xvt_dm_post_error("Sorry, only 5 events allowed in demo version");
      return (FALSE);
   }
   else
*/
   {
      DEBUG(printf("\nLOAD_HIST: %d of them, %d Event Already",no, numEvents);)
      for(stages=1;stages<no;stages++)
      {
         loadtype(status, &type, (char *) modifiers);
         if ((type > 0) && (type < 10))
         {                         /* malloc the basic structure */
            if (!(p = (OBJECT *) xvt_mem_alloc(sizeof(OBJECT))))
            {
               xvt_dm_post_error("Not Enough Memory to load Events");
               return (FALSE);
            }
            p->drawEvent = TRUE;
            p->row = 1;
            p->selected = TRUE;
            p->newObject = FALSE;
            p->column = numEvents + stages; /* place abjects after existing */
            p->next = NULL;
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
         }
DEBUG(printf("\n READING EVENT TYPE %d",type);)
         switch(type)
         {
            case 1:
            {
               FOLD_OPTIONS *options;

               p->shape = FOLD;
               strcpy (p->text, "Fold");
               options = (FOLD_OPTIONS *) xvt_mem_alloc(sizeof (FOLD_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_folds(status,stages,(char *) modifiers,  version, options);
               break;
            }
            case 2:
            {
               FAULT_OPTIONS *options;

               p->shape = FAULT;
               strcpy (p->text, "Fault");
               options = (FAULT_OPTIONS *) xvt_mem_alloc(sizeof (FAULT_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_faults(status,stages,(char *) modifiers, version, options);
               break;
            }
            case 3:
            {
               UNCONFORMITY_OPTIONS *options;
               NODDY_COLOUR noddyColorCopy[7];

               p->shape = UNCONFORMITY;
               strcpy (p->text, "U/C");
               options = (UNCONFORMITY_OPTIONS *)
                                    xvt_mem_alloc(sizeof (UNCONFORMITY_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
                       /* copy the default colors as they are not read
                       ** from the file properly */
               for (i = 0; i < 7; i++)
               {
                  memcpy (&(noddyColorCopy[i]),
                          &(options->stratigraphyDetails.properties[i].color),
                          sizeof (NODDY_COLOUR));
               }

               memcpy (&(options->stratigraphyDetails),
                       &(stratOptions[stratNumber]),
                        sizeof (STRATIGRAPHY_OPTIONS)); stratNumber++;

                       /* restore the default colors as they are not read
                       ** from the file properly */
               for (i = 0; i < 7; i++)
               {
                  memcpy (&(options->stratigraphyDetails.properties[i].color),
                          &(noddyColorCopy[i]),
                          sizeof (NODDY_COLOUR));
               }
               load_unconformities(status,stages,(char *) modifiers, version,
                                   options);
               break;
            }
            case 4:
            {
               SHEAR_OPTIONS *options;

               p->shape = SHEAR_ZONE;
               strcpy (p->text, "Shear Zone");
               options = (SHEAR_OPTIONS *) xvt_mem_alloc(sizeof (SHEAR_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_kinks(status,stages,(char *) modifiers, version, options);
               break;
            }
            case 5:
            {
               DYKE_OPTIONS *options;

               p->shape = DYKE;
               strcpy (p->text, "Dyke");
               options = (DYKE_OPTIONS *) xvt_mem_alloc(sizeof (DYKE_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_dykes(status,stages,(char *) modifiers, version, options);
               break;
            }
            case 6:
            {
               PLUG_OPTIONS *options;

               p->shape = PLUG;
               strcpy (p->text, "Plug");
               options = (PLUG_OPTIONS *) xvt_mem_alloc(sizeof (PLUG_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_plugs(status,stages,(char *) modifiers, version, options);
               break;
            }
            case 7:
            {
               STRAIN_OPTIONS *options;

               p->shape = STRAIN;
               strcpy (p->text, "Strain");
               options = (STRAIN_OPTIONS *) xvt_mem_alloc(sizeof (STRAIN_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_strains(status,stages,(char *) modifiers, version, options);
               break;
            }
            case 8:
            {
               TILT_OPTIONS *options;

               p->shape = TILT;
               strcpy (p->text, "Tilt");
               options = (TILT_OPTIONS *) xvt_mem_alloc(sizeof (TILT_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_tilts(status,stages,(char *) modifiers, version, options);
               break;
            }
            case 10:
            {
               LINEATION_OPTIONS *options;

               p->shape = LINEATION;
               strcpy (p->text, "Lineation");
               options = (LINEATION_OPTIONS *)
                                    xvt_mem_alloc(sizeof (LINEATION_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_lineations(status, stages, (char *) modifiers, version,
                               options);
               break;
            }
            case 9:
            {
               FOLIATION_OPTIONS *options;

               p->shape = FOLIATION;
               strcpy (p->text, "Foliation");
               options = (FOLIATION_OPTIONS *)
                                    xvt_mem_alloc(sizeof (FOLIATION_OPTIONS));
               p->options = (char *) options;
               setDefaultOptions (p);
               load_planes(status,stages,(char *) modifiers, version, options);
               break;
            }
            default:
               break;
         }
      }
             /* add the events we have loaded onto the events alread there */
      last = (OBJECT *) nthObject (win, numEvents-1);
      last->next = head;
      tidyObjects (win);  /* fit it all together correctly */
   }
   DEBUG(printf("\nLOAD_HIST: FINISHED");)
   return (TRUE);
}

int loadtype(in,type,modifiers)
FILE *in;
int *type;
char *modifiers;
{
   char oneline[255],typename[255];
   int i=0,jj;

   readGoodLine(in,oneline);

   DEBUG(printf("\n--LOADTYPE = %s",oneline);)
   while (oneline[i] != '=' && oneline[i] != '\0' )
      i++;

   if (oneline[i] != '\0')
      sscanf((char *)&oneline[i+1],"%s", typename);

   for (jj=0;jj<12;jj++)
   {
      if (equalstring2((char *)typename,(char *)NNames[jj]))
         *type=jj;
   }
     
   while (oneline[i] != '(' && oneline[i] != '\0' )
      i++;
     
   if (oneline[i] != '\0')
      strcpy(modifiers,(char *)&oneline[i]);
   else
      strcpy(modifiers,"");
   return (TRUE);
}

int load_geol_opts(status,version)
FILE *status;
double version;
{
   char temp[255];
   int blockMode;
   double originX, originY, originZ;
   DEBUG(printf("\nLOAD_GEOLOGY: STARTED");)

   loadit(status,"%lf",(void *) &originX);
   loadit(status,"%lf",(void *) &originY);
   loadit(status,"%lf",(void *) &originZ);
   newViewOptions ("Geology View", originX, originY, originZ, 10000.0, 7000.0, 5000.0, 100.0, 100.0);

   loadit(status,"%lf",(void *) &iscale);
   iscale *= 10.0;

   loadit(status,"%lf",(void *) &geologyOptions.welllogDeclination);
   loadit(status,"%lf",(void *) &geologyOptions.welllogDepth);
   /*loadit(status,"%lf",(void *) &geologyOptions.welllogAngle);*/
   loadit(status,"%d",(void *) &blockMode);
   geologyOptions.welllogAngle = 0.0;

     
   loadit(status,"%s",(void *) temp);  /* & ? */
   if (equalstring2((char *) temp, "true"))
   {
      /*geologyOptions.useTopography = TRUE;  ?? */
      geologyOptions.useTopography = FALSE;
      xvt_dm_post_error("Topofile was open for this history. Now topofile = False");
   }
   else
      geologyOptions.useTopography = FALSE;
          
   loadit(status,"%s",(void *) temp); /* Pattern FIll no longer supported */

   /*
   geologyOptions.linemap_X = 1000*iscale + originX;
   geologyOptions.linemap_Y =  700*iscale + originY;
   */
   DEBUG(printf("\nLOAD_GEOLOGY: FINISHED");)
   return (TRUE);
}

int load_gps_opts(status,version)
FILE *status;
double version;
{
   char temp[255];
   int ivalue;
   double startx, starty;  /* start for profile, now just upperNorth X, Y */
   double endx, endy;  /* end for profile, now just length */
   double dvalue, cubeSize;
   double lowerSouthWest_X, lowerSouthWest_Y, lowerSouthWest_Z;
   double upperNorthEast_X, upperNorthEast_Y, upperNorthEast_Z;
     
   DEBUG(printf("\nLOAD_GEOPHYSICS: STARTED");)
   loadit(status,"%d",(void *) &cubeSize);
   loadit(status,"%d",(void *) &geophysicsOptions.calculationRange);
     
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
     
   loadit(status,"%lf",(void *) &startx);
   loadit(status,"%lf",(void *) &starty);
   loadit(status,"%lf",(void *) &endx);
   loadit(status,"%lf",(void *) &endy);

   loadit(status,"%lf",(void *) &geophysicsOptions.altitude);
   if (version < 4.05)
   {
      loadit(status,"%d",(void *) &ivalue);
      loadit(status,"%lf",(void *) &dvalue);
   }
   loadit(status,"%lf",(void *) &geophysicsOptions.inclination);
   loadit(status,"%lf",(void *) &geophysicsOptions.intensity);

   loadit(status,"%s",(void *) temp);  /* & ? */
   if (equalstring2((char *) temp, "true"))
      geophysicsOptions.calculationAltitude = AIRBORNE;
   else
      geophysicsOptions.calculationAltitude = SURFACE;

   if (version < 3.7)
   {               /* No Longer have calculationType of MAP or PROFILE */
      loadit(status,"%s",(void *) temp);  /* & ? */
   }

   if (version < 4.0)
   {
      loadit(status,"%s",(void *) temp); /* ermapper output */
   }
   
   if (version>=2.1)
   {
      loadit(status,"%s",(void *) temp); /* & ? */
      if (equalstring2((char *) temp, "true"))
         geophysicsOptions.deformableRemanence = FALSE;
      else
         geophysicsOptions.deformableRemanence = TRUE;

      loadit(status,"%s",(void *) temp); /* & ? */
      if (equalstring2((char *) temp, "true"))
         geophysicsOptions.drapedSurvey = FALSE;
      else
         geophysicsOptions.drapedSurvey = TRUE;
   }
   else
   {
      geophysicsOptions.deformableRemanence = TRUE;
      geophysicsOptions.drapedSurvey = FALSE;
   }
   DEBUG(printf("\nLOAD_GEOPHYSICS: FINISHED");)
   return (TRUE);
}

int load_3d_opts(status,version)
FILE *status;
double version;
{
   int StratNumber, LayerNumber, Declination3D, Azimuth3D, ShadingScheme;

   DEBUG(printf("\nLOAD_3d: STARTED");)
   loadit(status,"%d",(void *) &StratNumber);
   loadit(status,"%d",(void *) &LayerNumber);
   loadit(status,"%d",(void *) &Declination3D);
   loadit(status,"%d",(void *) &Azimuth3D);
   /*loadit(status,"%d",(void *) &Roll3D);
   loadit(status,"%d",(void *) &Tx);
   loadit(status,"%d",(void *) &Ty);
   loadit(status,"%lf",(void *) &ScreenScale); */
   /* ScreenScale now int !!!  + Tz  !!  & Rotx */
   loadit(status,"%d",(void *) &ShadingScheme);
   DEBUG(printf("\nLOAD_3d: FINISHED");)
   return (TRUE);
}


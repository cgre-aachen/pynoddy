#include "xvt.h"
#include "noddy.h"
#include "titles.h"
#include <time.h>

#define DEBUG(X)  
#define TOLERANCE   0.001

                 /* ********************************** */
                 /* External Globals used in this file */
extern PROJECT_OPTIONS projectOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern double iscale; /* scaling factor for geology */
extern FILE_SPEC topoFileSpec;
extern COLOR backgroundColor;
extern int batchExecution;

                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
extern int getFloatingMenuShape (WINDOW, int *, int *);
extern int report_folds(FILE *, FOLD_OPTIONS *);
extern int report_dykes(FILE *, DYKE_OPTIONS *);
extern int report_faults(FILE *, FAULT_OPTIONS *);
extern int report_shear_zone(FILE *, SHEAR_OPTIONS *);
extern int report_lineations(FILE *, LINEATION_OPTIONS *);
extern int report_planes(FILE *, FOLIATION_OPTIONS *);
extern int report_plugs(FILE *, PLUG_OPTIONS *);
extern int report_tilts(FILE *, TILT_OPTIONS *);
extern int report_strains(FILE *, STRAIN_OPTIONS *);
extern int report_import(FILE *, IMPORT_OPTIONS *);
extern int report_generic(FILE *, GENERIC_OPTIONS *);
extern int report_unconformities(FILE *, UNCONFORMITY_OPTIONS *);
extern int reportStratigraphy (FILE *, STRATIGRAPHY_OPTIONS *);
int report_file_history (FILE *, char *);
int report_hist (FILE *);
int report_block_opts(FILE *);
int report_geol_opts(FILE *);
int report_gps_opts(FILE *);
int report_3d_opts(FILE *);
int report_proj_opts(FILE *);
int report_window_positions(FILE *);
int report_icon_positions(FILE *);
#else
#endif

int report_status (filename)
char *filename;
{
   FILE *out;
   int error = TRUE;

	if (DEMO_VERSION)
	{
		xvt_dm_post_error ("Sorry, You can not save Histories in the Demo Version");
	   return (FALSE);
	}

   out = (FILE *) fopen (filename,"w");

   if (out != 0L)
   {
      DEBUG(printf("\n ABOUT TO WRITE HEADER");)
      report_file_history (out, filename);
      DEBUG(printf("\n ABOUT TO WRITE HISTORY");)
      report_hist (out);
      DEBUG(printf("\n ABOUT TO WRITE BLOCK OPS");)
      report_block_opts (out);
      DEBUG(printf("\n ABOUT TO WRITE GEOL OPS");)
      report_geol_opts (out);
      DEBUG(printf("\n ABOUT TO WRITE GEOPH OPS");)
      report_gps_opts (out);
      DEBUG(printf("\n ABOUT TO WRITE 3D OPS");)
      report_3d_opts (out);
      DEBUG(printf("\n ABOUT TO WRITE PROJECT OPS");)
      report_proj_opts (out);

      report_window_positions (out);
      report_icon_positions (out);

      fprintf(out,"End of Status Report\n");
      DEBUG(printf("\n FINISHED");)
      
      fflush(out);
      fclose(out);

      error = FALSE;
   }
   else
      xvt_dm_post_error("Cannot Open History File.");

   return (!error);
}

int report_file_history (out, filename)
FILE *out;
char *filename;
{
   time_t longTime;
   struct tm *today;

   time(&longTime); /* current Time */
   today = localtime(&longTime);

   fprintf(out,"#Filename = %s\n", filename);
   fprintf(out,"#Date Saved = %d/%d/%d %d:%d:%d\n",
                              today->tm_mday, today->tm_mon+1, today->tm_year+1900,
                              today->tm_hour, today->tm_min,   today->tm_sec);
   fprintf(out,"FileType = %d\nVersion = %.2f\n\n", 111, VERSION_NUMBER);
   return (TRUE);
}
      
int report_hist (out)
FILE *out;
{
   WINDOW win;
   OBJECT *p;
   register int numEvents, event;

   win = getEventDrawingWindow ();
   numEvents = totalObjects (win);
   

   fprintf(out,"\nNo of Events\t= %d\n", numEvents);

   for (event = 0; event < numEvents; event++)
   {
      p = (OBJECT *) nthObject (win, event);
      
      DEBUG(printf("\n p->shape = %d", p->shape);)
      if (p)
      {
         switch (p->shape)
         {
           case STRATIGRAPHY:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "STRATIGRAPHY");
              reportStratigraphy (out, (STRATIGRAPHY_OPTIONS *) p->options);
              break;
           }
           case FOLD:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "FOLD");
         report_folds(out, (FOLD_OPTIONS *) p->options);
              break;
           }
      case FAULT:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "FAULT");
              report_faults(out, (FAULT_OPTIONS *) p->options);
              break;
           }
      case UNCONFORMITY:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "UNCONFORMITY");
              report_unconformities(out, (UNCONFORMITY_OPTIONS *) p->options);
              break;
           }
      case SHEAR_ZONE:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "SHEAR_ZONE");
              report_shear_zone(out, (SHEAR_OPTIONS *) p->options);
              break;
           }
      case DYKE:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "DYKE");
              report_dykes(out, (DYKE_OPTIONS *) p->options);
              break;
           }
      case PLUG:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "PLUG");
              report_plugs(out, (PLUG_OPTIONS *) p->options);
              break;
           }
      case STRAIN:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "STRAIN");
              report_strains(out, (STRAIN_OPTIONS *) p->options);
              break;
           }
      case TILT:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "TILT");
              report_tilts(out, (TILT_OPTIONS *) p->options);
              break;
           }
      case FOLIATION:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "FOLIATION");
              report_planes(out, (FOLIATION_OPTIONS *) p->options);
              break;
           }
      case LINEATION:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "LINEATION");
              report_lineations(out, (LINEATION_OPTIONS *) p->options);
              break;
           }
      case IMPORT:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "IMPORT");
              report_import(out, (IMPORT_OPTIONS *) p->options);
              break;
           }
      case GENERIC:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "GENERIC");
              report_generic(out, (GENERIC_OPTIONS *) p->options);
              break;
           }
      case STOP:
           {
              fprintf (out, "Event #%d\t= %s\n", (event+1), "STOP");
              break;
           }
         }
         fprintf (out, "\tName\t= %s\n", p->text);
      }
   }
   return (TRUE);
}

int report_block_opts(out)
FILE *out;
{
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions (), *tempView;
   int view, numViews, currentView;
   
   currentView = 0;
   while (viewOptions->prev)   /* move to first view */
   {
      viewOptions = viewOptions->prev;
      currentView++;  /* Each View we go back means another veiw in */
   }

   numViews = 1;
   tempView = viewOptions;
   while (tempView->next)   /* count the num of views */
   {
      tempView = tempView->next;
      numViews++;
   }
  
   fprintf(out,"\n#BlockOptions\n");
   fprintf(out,"\tNumber of Views\t= %d\n", numViews);
   fprintf(out,"\tCurrent View\t= %d\n", currentView);

   for (view = 0; view < numViews; view++)
   {
      fprintf(out,"\tNAME\t= %s\n", viewOptions->viewName);
   
      fprintf(out,"\tOrigin X\t= %6.2lf\n", viewOptions->originX);
      fprintf(out,"\tOrigin Y\t= %6.2lf\n", viewOptions->originY);
      fprintf(out,"\tOrigin Z\t= %6.2lf\n", viewOptions->originZ);
   
      fprintf(out,"\tLength X\t= %6.2lf\n", viewOptions->lengthX);
      fprintf(out,"\tLength Y\t= %6.2lf\n", viewOptions->lengthY);
      fprintf(out,"\tLength Z\t= %6.2lf\n", viewOptions->lengthZ);
   
      fprintf(out,"\tGeology Cube Size\t= %6.2lf\n", viewOptions->geologyCubeSize);
      fprintf(out,"\tGeophysics Cube Size\t= %6.2lf\n", viewOptions->geophysicsCubeSize);
      
      viewOptions = viewOptions->next;
   }
   
   return (TRUE);
}

int report_geol_opts(out)
FILE *out;
{
   char directory[200];
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
  
   fprintf(out,"\n#GeologyOptions\n");
/*
   fprintf(out,"\tX\t= %6.2lf\n", viewOptions->originX);
   fprintf(out,"\tY\t= %6.2lf\n", viewOptions->originY);
   fprintf(out,"\tZ\t= %6.2lf\n", viewOptions->originZ - viewOptions->lengthZ);
*/

   fprintf(out,"\tScale\t= %6.2lf\n", iscale);
   fprintf(out,"\tSectionDec\t= %6.2lf\n", geologyOptions.welllogDeclination);
   fprintf(out,"\tWellDepth\t= %6.2lf\n", geologyOptions.welllogDepth);
   fprintf(out,"\tWellAngleZ\t= %6.2lf\n", geologyOptions.welllogAngle);

   fprintf(out,"\tBoreholeX\t= %6.2lf\n", geologyOptions.boreholeX);
   fprintf(out,"\tBoreholeX\t= %6.2lf\n", geologyOptions.boreholeY);
   fprintf(out,"\tBoreholeX\t= %6.2lf\n", geologyOptions.boreholeZ);
   fprintf(out,"\tBoreholeDecl\t= %6.2lf\n", geologyOptions.boreholeDecl);
   fprintf(out,"\tBoreholeDip\t= %6.2lf\n", geologyOptions.boreholeDip);
   fprintf(out,"\tBoreholeLength\t= %6.2lf\n", geologyOptions.boreholeLength);

   fprintf(out,"\tSectionX\t= %6.2lf\n", geologyOptions.sectionX);
   fprintf(out,"\tSectionY\t= %6.2lf\n", geologyOptions.sectionY);
   fprintf(out,"\tSectionZ\t= %6.2lf\n", geologyOptions.sectionZ);
   fprintf(out,"\tSectionDecl\t= %6.2lf\n", geologyOptions.sectionDecl);
   fprintf(out,"\tSectionLength\t= %6.2lf\n", geologyOptions.sectionLength);
   fprintf(out,"\tSectionHeight\t= %6.2lf\n", geologyOptions.sectionHeight);

   if (geologyOptions.useTopography)
   {
      fprintf(out,"\ttopofile\t= TRUE\n");
      fprintf(out, "\tTopo Filename\t= %s\n",topoFileSpec.name);
      xvt_fsys_convert_dir_to_str (&(topoFileSpec.dir), directory, 200);
   }
   else
   {
      fprintf(out,"\ttopofile\t= FALSE\n");
      fprintf(out, "\tTopo Filename\t= %s\n","   ");
      strcpy (directory, ".");
   }
   fprintf(out, "\tTopo Directory\t= %s\n",directory);
     
   fprintf(out, "\tTopo Scale\t= %6.2lf\n", geologyOptions.topoScaleFactor);
   fprintf(out, "\tTopo Offset\t= %6.2lf\n", geologyOptions.topoOffsetFactor);
   fprintf(out, "\tTopo First Contour\t= %6.2lf\n",
                                              geologyOptions.topoLowContour);
   fprintf(out, "\tTopo Contour Interval\t= %6.2lf\n",
                                              geologyOptions.topoContourInterval);
    
   if (geologyOptions.calculateChairDiagram)
      fprintf(out,"\tChair Diagram\t= TRUE\n");
   else
      fprintf(out,"\tChair Diagram\t= FALSE\n");
   fprintf(out,"\tChair_X\t= %6.2lf\n", geologyOptions.chair_X);
   fprintf(out,"\tChair_Y\t= %6.2lf\n", geologyOptions.chair_Y);
   fprintf(out,"\tChair_Z\t= %6.2lf\n", geologyOptions.chair_Z);

/*
   fprintf(out,"\tLinemap_X\t= %6.2lf\n", geologyOptions.linemap_X);
   fprintf(out,"\tLinemap_Y\t= %6.2lf\n", geologyOptions.linemap_Y);

   fprintf(out,"\tCube Resolution\t= %.0f\n",viewOptions->geologyCubeSize);
   fprintf(out,"\tCube Scale\t= 1.0\n");
*/
   return (TRUE);
}

int report_gps_opts(out)
FILE *out;
{
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   fprintf(out,"\n#GeophysicsOptions\n");

   fprintf(out,"\tGPSRange\t = %d\n",geophysicsOptions.calculationRange);
   fprintf(out,"\tDeclination\t= %6.2lf\n", geophysicsOptions.declination);
   
   fprintf(out,"\tInclination\t= %6.2lf\n",geophysicsOptions.inclination);
   fprintf(out,"\tIntensity\t= %6.2lf\n", geophysicsOptions.intensity);

   if (geophysicsOptions.fieldType == FIXED_FIELD)
      fprintf(out,"\tField Type\t= FIXED\n");
   else
      fprintf(out,"\tField Type\t= VARIABLE\n");
   fprintf(out,"\tField xPos\t= %6.2lf\n",geophysicsOptions.xPos);
   fprintf(out,"\tField yPos\t= %6.2lf\n",geophysicsOptions.yPos);
   fprintf(out,"\tField zPos\t= %6.2lf\n",geophysicsOptions.zPos);

   fprintf(out,"\tInclination Ori\t= %6.2lf\n",geophysicsOptions.inclinationOri);
   fprintf(out,"\tInclination Change\t= %6.2lf\n",geophysicsOptions.inclinationChange);
   fprintf(out,"\tIntensity Ori\t= %6.2lf\n", geophysicsOptions.intensityOri);
   fprintf(out,"\tIntensity Change\t= %6.2lf\n", geophysicsOptions.intensityChange);
   fprintf(out,"\tDeclination Ori\t= %6.2lf\n", geophysicsOptions.declinationOri);
   fprintf(out,"\tDeclination Change\t= %6.2lf\n", geophysicsOptions.declinationChange);


   fprintf(out,"\tAltitude\t= %6.2lf\n", geophysicsOptions.altitude);

   if (geophysicsOptions.calculationAltitude)
      fprintf(out,"\tAirborne= \tTRUE\n");
   else
      fprintf(out,"\tAirborne= \tFALSE\n");

   if (geophysicsOptions.calculationMethod == SPATIAL)
      fprintf(out,"\tCalculation Method\t= SPATIAL\n");
   else if (geophysicsOptions.calculationMethod == SPECTRAL)
      fprintf(out,"\tCalculation Method\t= SPECTRAL\n");
   else /* Full */
      fprintf(out,"\tCalculation Method\t= FULL\n");

   switch (geophysicsOptions.spectralPaddingType)
   {
      case (RAMP_PADDING):
         fprintf(out,"\tSpectral Padding Type\t= RAMP_PADDING\n");
         break;
      case (FENCE_MEAN_PADDING):
         fprintf(out,"\tSpectral Padding Type\t= FENCE_MEAN_PADDING\n");
         break;
      case (FENCE_VALUE_PADDING):
         fprintf(out,"\tSpectral Padding Type\t= FENCE_VALUE_PADDING\n");
         break;
      case (SET_MEAN_PADDING):
         fprintf(out,"\tSpectral Padding Type\t= SET_MEAN_PADDING\n");
         break;
      case (SET_VALUE_PADDING):
         fprintf(out,"\tSpectral Padding Type\t= SET_VALUE_PADDING\n");
         break;
      case (RECLECTION_PADDING):
         fprintf(out,"\tSpectral Padding Type\t= RECLECTION_PADDING\n");
         break;
      default:            
         fprintf(out,"\tSpectral Padding Type\t= UNKNOWN\n");
   }
   fprintf(out,"\tSpectral Fence\t= %d\n", geophysicsOptions.spectralFence);
   fprintf(out,"\tSpectral Percent\t= %d\n", geophysicsOptions.spectralPercent);

   fprintf(out,"\tConstant Boxing Depth\t= %6.2lf\n",
                           geophysicsOptions.constantBoxDepth);
   fprintf(out,"\tClever Boxing Ratio\t= %6.2lf\n",
                           geophysicsOptions.cleverBoxRatio);
   
   if (geophysicsOptions.deformableRemanence)
      fprintf(out,"\tDeformable Remanence= \tTRUE\n");
   else
      fprintf(out,"\tDeformable Remanence= \tFALSE\n");

   if (geophysicsOptions.deformableAnisotropy)
      fprintf(out,"\tDeformable Anisotropy= \tTRUE\n");
   else
      fprintf(out,"\tDeformable Anisotropy= \tFALSE\n");

   if (geophysicsOptions.magneticVectorComponents)
      fprintf(out,"\tVector Components= \tTRUE\n");
   else
      fprintf(out,"\tVector Components= \tFALSE\n");

   if (geophysicsOptions.projectVectorsOntoField)
      fprintf(out,"\tProject Vectors= \tTRUE\n");
   else
      fprintf(out,"\tProject Vectors= \tFALSE\n");

   if (geophysicsOptions.padWithRealGeology)
      fprintf(out,"\tPad With Real Geology= \tTRUE\n");
   else
      fprintf(out,"\tPad With Real Geology= \tFALSE\n");

   if (geophysicsOptions.drapedSurvey)
      fprintf(out,"\tDraped Survey= \tTRUE\n");
   else
      fprintf(out,"\tDraped Survey= \tFALSE\n");
   return (TRUE);
}

int report_3d_opts(out)
FILE *out;
{
   fprintf(out,"\n#3DOptions\n");

   fprintf(out,"\tDeclination\t= %f\n", threedViewOptions.declination);
   fprintf(out,"\tElevation\t= %f\n", threedViewOptions.azimuth);
   fprintf(out,"\tScale\t= %f\n", threedViewOptions.scale);

   fprintf(out,"\tOffset X\t= %f\n", threedViewOptions.offset_X);
   fprintf(out,"\tOffset Y\t= %f\n", threedViewOptions.offset_Y);
   fprintf(out,"\tOffset Z\t= %f\n", threedViewOptions.offset_Z);

   fprintf(out,"\tFill Type\t= %d\n", threedViewOptions.fillType);
   return (TRUE);
}

int report_proj_opts(out)
FILE *out;
{
   fprintf(out,"\n#ProjectOptions\n");

   if (projectOptions.susceptibilityUnits == SI)
      fprintf(out,"\tSusceptibility Units\t= SI\n");
   else
      fprintf(out,"\tSusceptibility Units\t= CGS\n");
   fprintf(out,"\tGeophysical Calculation\t= %d\n",
                                            projectOptions.geophysicalCalc);
   if (projectOptions.calculationType == REMOTE_JOB)
      fprintf(out,"\tCalculation Type\t= REMOTE_JOB\n");
   else
      fprintf(out,"\tCalculation Type\t= LOCAL_JOB\n");
   fprintf(out,"\tLength Scale\t= %d\n", projectOptions.lengthScale);
   fprintf(out,"\tPrinting Scale\t= %lf\n", projectOptions.printScalingFactor);
   fprintf(out,"\tImage Scale\t= %lf\n", projectOptions.imageScalingFactor);

   if (projectOptions.newWindowEachStage)
      fprintf(out,"\tNew Windows\t= TRUE\n");
   else
      fprintf(out,"\tNew Windows\t= FALSE\n");

   fprintf(out,"\tBackground Red Component\t= %d\n", XVT_COLOR_GET_RED(backgroundColor));
   fprintf(out,"\tBackground Green Component\t= %d\n", XVT_COLOR_GET_GREEN(backgroundColor));
   fprintf(out,"\tBackground Blue Component\t= %d\n", XVT_COLOR_GET_BLUE(backgroundColor));

   fprintf(out,"\tInternet Address\t= %s\n", projectOptions.internetAddress);
   fprintf(out,"\tAccount Name\t= %s\n", projectOptions.accountName);
   fprintf(out,"\tNoddy Path\t= %s\n", projectOptions.noddyPath);

   fprintf(out,"\tHelp Path\t= %s\n", projectOptions.helpPath);

   fprintf(out,"\tMovie Frames Per Event\t= %d\n",
                             projectOptions.movieFramesPerEvent);
   fprintf(out,"\tMovie Play Speed\t= %6.2lf\n",
                             projectOptions.moviePlaySpeed);
   fprintf(out,"\tMovie Type\t= %d\n",
                             projectOptions.movieType);


   if (projectOptions.gravityDisplay.clippingType == ABSOLUTE_CLIPPING)
      fprintf(out,"\tGravity Clipping Type\t= ABSOLUTE_CLIPPING\n");
   else 
      fprintf(out,"\tGravity Clipping Type\t= RELATIVE_CLIPPING\n");
   fprintf(out,"\tGravity Image Display Clip Min\t= %lf\n",
                             projectOptions.gravityDisplay.minClip);
   fprintf(out,"\tGravity Image Display Clip Max\t= %lf\n",
                             projectOptions.gravityDisplay.maxClip);
   if (projectOptions.gravityDisplay.display == CONTOUR_IMAGE)
      fprintf(out,"\tGravity Image Display Type\t= CONTOUR\n");
   else if (projectOptions.gravityDisplay.display == PSEUDO_COLOR)
      fprintf(out,"\tGravity Image Display Type\t= PSEUDO_COLOR\n");
   else /* GREY_COLOR */
      fprintf(out,"\tGravity Image Display Type\t= GREY\n");
   fprintf(out,"\tGravity Image Display Num Contour\t= %d\n",
                             projectOptions.gravityDisplay.numContours);


   if (projectOptions.magneticsDisplay.clippingType == ABSOLUTE_CLIPPING)
      fprintf(out,"\tMagnetics Clipping Type\t= ABSOLUTE_CLIPPING\n");
   else 
      fprintf(out,"\tMagnetics Clipping Type\t= RELATIVE_CLIPPING\n");
   fprintf(out,"\tMagnetics Image Display Clip Min\t= %lf\n",
                             projectOptions.magneticsDisplay.minClip);
   fprintf(out,"\tMagnetics Image Display Clip Max\t= %lf\n",
                             projectOptions.magneticsDisplay.maxClip);
   if (projectOptions.magneticsDisplay.display == CONTOUR_IMAGE)
      fprintf(out,"\tMagnetics Image Display Type\t= CONTOUR\n");
   else if (projectOptions.magneticsDisplay.display == PSEUDO_COLOR)
      fprintf(out,"\tMagnetics Image Display Type\t= PSEUDO_COLOR\n");
   else /* GREY_COLOR */
      fprintf(out,"\tMagnetics Image Display Type\t= GREY\n");
   fprintf(out,"\tMagnetics Image Display Num Contour\t= %d\n",
                             projectOptions.magneticsDisplay.numContours);

   fprintf(out,"\tFalse Easting\t= %lf\n", projectOptions.easting);
   fprintf(out,"\tFalse Northing\t= %lf\n", projectOptions.northing);

   return (TRUE);
}

int report_window_positions(out)
FILE *out;
{
   register int winNum;
   int x, y, width, height;
   WINDOW win;
   RCT rect;

   fprintf(out,"\n#Window Positions\n");
   fprintf(out,"\tNum Windows\t= %d\n",winPositionOptions.numWindows);
   for (winNum = 0; winNum < winPositionOptions.numWindows; winNum++)
   {
      x = winPositionOptions.winPositions[winNum].x;
      y = winPositionOptions.winPositions[winNum].y;
      width = winPositionOptions.winPositions[winNum].width;
      height = winPositionOptions.winPositions[winNum].height;

		if (!batchExecution)
		{
			if (strstr(winPositionOptions.winPositions[winNum].name, TOOLBAR_TITLE))
			{
				if (win = (WINDOW) getFloatingMenuWindow ())
				{
					xvt_vobj_get_outer_rect (win, &rect);
					x = rect.left;
					y = rect.top;
					x += xvt_vobj_get_attr(win, ATTR_FRAME_WIDTH)/2 + 1;
					y += xvt_vobj_get_attr(win, ATTR_TITLE_HEIGHT) +
						  xvt_vobj_get_attr(win, ATTR_FRAME_HEIGHT) - 1;
					width = height = -1;
				}
			}
			else if (strstr(winPositionOptions.winPositions[winNum].name, HISTORY_TITLE))
			{
				if (win = (WINDOW) getEventDrawingWindow ())
				{
					xvt_vobj_get_outer_rect (win, &rect);
					x = rect.left;
					y = rect.top;
					x += xvt_vobj_get_attr(win, ATTR_DOCFRAME_WIDTH)/2 + 1;
					y += xvt_vobj_get_attr(win, ATTR_TITLE_HEIGHT) +
						  xvt_vobj_get_attr(win, ATTR_DOCFRAME_HEIGHT) - 1;
					width = xvt_rect_get_width(&rect);
					height = xvt_rect_get_height(&rect);
				}
			}
		}      
      fprintf (out, "\tName\t= %s\n", winPositionOptions.winPositions[winNum].name);
      fprintf (out, "\tX\t= %d\n", x);
      fprintf (out, "\tY\t= %d\n", y);
      fprintf (out, "\tWidth\t= %d\n", width);
      fprintf (out, "\tHeight\t= %d\n", height);
   }
   return (TRUE);
}


int report_icon_positions(out)
FILE *out;
{
   WINDOW win;
   OBJECT *p;
   register int numEvents, event;
   int rows, cols;

   win = getEventDrawingWindow ();
   numEvents = totalObjects (win);
   
   fprintf(out,"\n#Icon Positions\n");
   fprintf(out,"\tNum Icons\t= %d\n",numEvents);
   for (event = 0; event < numEvents; event++)
   {
      p = (OBJECT *) nthObject (win, event);
      
      if (p)
      {
         fprintf (out, "\tRow\t= %d\n", p->row);
         fprintf (out, "\tColumn\t= %d\n", p->column);
         fprintf (out, "\tX Position\t= %d\n", (p->bound.left/GRID_WIDTH) + 1);
         fprintf (out, "\tY Position\t= %d\n", (p->bound.top/GRID_HEIGHT) + 1);
      }
   }
	getFloatingMenuShape (NULL_WIN, &rows, &cols);
   fprintf(out,"\tFloating Menu Rows\t= %d\n",rows);
   fprintf(out,"\tFloating Menu Cols\t= %d\n",cols);
   return (TRUE);
}



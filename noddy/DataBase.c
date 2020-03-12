/*
 ============================================================================
 Name        : DataBase.c
 Author      : 
 Version     :
 Copyright   : LGPL
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "noddy.h"
#include "xvt.h"
#include "nodStruc.h"
#include "time.h"
#include "string.h"
#include <sys/time.h>

#define DEBUG(X)
#define DEBUG1(X)

extern int Random;
extern int DataBase;

extern PROJECT_OPTIONS projectOptions;
extern BLOCK_VIEW_OPTIONS *blockViewOptions;
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern WINDOW_POSITION_OPTIONS winPositionOptions;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern double iscale; /* scaling factor for geology */
extern FILE_SPEC topoFileSpec;
extern double minTopoValue, maxTopoValue;

extern int batchExecution;
extern COLOR backgroundColor;
extern WINDOW_INFO batchWindowInfo;

int exit_nicely();
extern char time_stamp[100];

int connect() {
	const char *conninfo;
	const char *query;
	int i, j, nFields;
	PGconn *conn;

	conninfo =
			"host=130.95.198.59 dbname=noddyverse user=postgres password=loop123pgpw";
	conn = PQconnectdb(conninfo);

	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
		exit_nicely(conn);
	} else
		printf("connection OK\n");
}

int insert_string(PGconn *conn, char *table, char *field, char *text, int async) {

	PGresult *res;
	int asynch_res;
	char query[200];

	sprintf(query, "INSERT INTO public.%s (%s) VALUES (%s)", table, field,
			text);

	if (!async) {
		res = PQexec(conn, (const char*) query);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			fprintf(stderr, "INSERT failed: %s Table: %s Field: %s Text: %s\n", PQerrorMessage(conn),table, field, text);
			exit_nicely(conn);
		} else
			printf("queried\n");
	} else {
		asynch_res = PQsendQuery(conn, (const char*) query);
		if (asynch_res = 0L) {
			fprintf(stderr, "INSERT failed: %s", PQerrorMessage(conn));
			exit_nicely(conn);
		} else
			printf("queried2\n");
	}
}

int insert_query(char *query, int async) {

	PGresult *res;
	int asynch_res;
	const char *q2;
	PGconn *conn;
	const char *conninfo;
	FILE *his_out;
	int nprint;

	conninfo =
			"host=130.95.198.59 dbname=noddyverse user=postgres password=loop123pgpw";

	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
		exit_nicely(conn);
	}

	//printf("query= %s\n", query);

	//his_out = fopen("my.out", "w");
	//nprint = fprintf(his_out, "%s", query);
	//fflush(his_out);
	//fclose(his_out);
	//printf("nprint = %d\n", nprint);
	//if(nprint > 100)exit(1);
	if (!async) {
		res = PQexec(conn, (const char*) query);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			fprintf(stderr, "INSERT failed: %s", PQerrorMessage(conn));
			fprintf(stderr, "INSERT failed: %s  Query: %s\n", PQerrorMessage(conn),query);
			exit_nicely(conn);
		} else {
			//printf("queried\n");
		}
	} else {
		asynch_res = PQsendQuery(conn, (const char*) query);
		if (asynch_res = 0L) {
			fprintf(stderr, "INSERT failed: %s", PQerrorMessage(conn));
			exit_nicely(conn);
		} else {
			//printf("queried2\n");

		}
	}

	PQfinish(conn);

}

int exit_nicely(PGconn *conn) {
	PQfinish(conn);
	exit(1);
}

int upload_universe() {
	char query[1000];

	sprintf(query,
			"INSERT INTO public.universe (uid,comment,created) VALUES ('%s','OK',NOW())",
			time_stamp);
	insert_query(query, 0);

}

int upload_Random_window_positions(out, query)
	FILE *out;char *query; {
	register int winNum;
	int x, y, width, height;
	WINDOW win;
	RCT rect;
	sprintf(query, "%s\n#Window Positions\n", query);
	sprintf(query, "%s	Num Windows	= 16\n", query);
	sprintf(query, "%s	Name	= Block Diagram\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 500\n", query);
	sprintf(query, "%s	Height	= 300\n", query);
	sprintf(query, "%s	Name	= Movie\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 500\n", query);
	sprintf(query, "%s	Height	= 300\n", query);
	sprintf(query, "%s	Name	= Well Log\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 400\n", query);
	sprintf(query, "%s	Height	= 430\n", query);
	sprintf(query, "%s	Name	= Section\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 500\n", query);
	sprintf(query, "%s	Height	= 300\n", query);
	sprintf(query, "%s	Name	= Topography Map\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 490\n", query);
	sprintf(query, "%s	Height	= 375\n", query);
	sprintf(query, "%s	Name	= 3D Topography Map\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 490\n", query);
	sprintf(query, "%s	Height	= 375\n", query);
	sprintf(query, "%s	Name	= 3D Stratigraphy\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 490\n", query);
	sprintf(query, "%s	Height	= 375\n", query);
	sprintf(query, "%s	Name	= Line Map\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 500\n", query);
	sprintf(query, "%s	Height	= 300\n", query);
	sprintf(query, "%s	Name	= Profile - From Image\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 500\n", query);
	sprintf(query, "%s	Height	= 300\n", query);
	sprintf(query, "%s	Name	= Profile\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 490\n", query);
	sprintf(query, "%s	Height	= 600\n", query);
	sprintf(query, "%s	Name	= Sterographic Projections\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 430\n", query);
	sprintf(query, "%s	Height	= 430\n", query);
	sprintf(query, "%s	Name	= Stratigraphic Column\n", query);
	sprintf(query, "%s	X	= 60\n", query);
	sprintf(query, "%s	Y	= 60\n", query);
	sprintf(query, "%s	Width	= 230\n", query);
	sprintf(query, "%s	Height	= 400\n", query);
	sprintf(query, "%s	Name	= Image\n", query);
	sprintf(query, "%s	X	= 30\n", query);
	sprintf(query, "%s	Y	= 30\n", query);
	sprintf(query, "%s	Width	= 500\n", query);
	sprintf(query, "%s	Height	= 300\n", query);
	sprintf(query, "%s	Name	= Contour\n", query);
	sprintf(query, "%s	X	= 30\n", query);
	sprintf(query, "%s	Y	= 30\n", query);
	sprintf(query, "%s	Width	= 500\n", query);
	sprintf(query, "%s	Height	= 300\n", query);
	sprintf(query, "%s	Name	= Toolbar\n", query);
	sprintf(query, "%s	X	= 24\n", query);
	sprintf(query, "%s	Y	= 21\n", query);
	sprintf(query, "%s	Width	= -1\n", query);
	sprintf(query, "%s	Height	= -1\n", query);
	sprintf(query, "%s	Name	= History\n", query);
	sprintf(query, "%s	X	= 292\n", query);
	sprintf(query, "%s	Y	= 61\n", query);
	sprintf(query, "%s	Width	= 480\n", query);
	sprintf(query, "%s	Height	= 440\n", query);
	sprintf(query, "%s\n", query);

	return (TRUE);
}

int upload_Random_block_opts(query)
	char *query; {
	BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions(), *tempView;
	int view, numViews, currentView;

	currentView = 0;
	while (viewOptions->prev) /* move to first view */
	{
		viewOptions = viewOptions->prev;
		currentView++; /* Each View we go back means another veiw in */
	}

	numViews = 1;
	tempView = viewOptions;
	while (tempView->next) /* count the num of views */
	{
		tempView = tempView->next;
		numViews++;
	}

	sprintf(query, "%s\n#BlockOptions\n", query);
	sprintf(query, "%s\tNumber of Views\t= %d\n", query, numViews);
	sprintf(query, "%s\tCurrent View\t= %d\n", query, currentView);

	for (view = 0; view < numViews; view++) {
		sprintf(query, "%s\tNAME\t= %s\n", query, viewOptions->viewName);

		sprintf(query, "%s\tOrigin X\t= %6.2lf\n", query, viewOptions->originX);
		sprintf(query, "%s\tOrigin Y\t= %6.2lf\n", query, viewOptions->originY);
		sprintf(query, "%s\tOrigin Z\t= %6.2lf\n", query, viewOptions->originZ);

		sprintf(query, "%s\tLength X\t= %6.2lf\n", query, viewOptions->lengthX);
		sprintf(query, "%s\tLength Y\t= %6.2lf\n", query, viewOptions->lengthY);
		sprintf(query, "%s\tLength Z\t= %6.2lf\n", query, viewOptions->lengthZ);

		sprintf(query, "%s\tGeology Cube Size\t= %6.2lf\n", query,
				viewOptions->geologyCubeSize);
		sprintf(query, "%s\tGeophysics Cube Size\t= %6.2lf\n", query,
				viewOptions->geophysicsCubeSize);

		viewOptions = viewOptions->next;
	}

	return (TRUE);
}

int upload_Random_geol_opts(query)
	char *query; {
	char directory[200];
	BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions();

	sprintf(query, "%s\n#GeologyOptions\n", query);
	/*
	 sprintf(query,"%s\tX\t= %6.2lf\n",query, viewOptions->originX);
	 sprintf(query,"%s\tY\t= %6.2lf\n",query, viewOptions->originY);
	 sprintf(query,"%s\tZ\t= %6.2lf\n",query, viewOptions->originZ - viewOptions->lengthZ);
	 */

	sprintf(query, "%s\tScale\t= %6.2lf\n", query, iscale);
	sprintf(query, "%s\tSectionDec\t= %6.2lf\n", query,
			geologyOptions.welllogDeclination);
	sprintf(query, "%s\tWellDepth\t= %6.2lf\n", query,
			geologyOptions.welllogDepth);
	sprintf(query, "%s\tWellAngleZ\t= %6.2lf\n", query,
			geologyOptions.welllogAngle);

	sprintf(query, "%s\tBoreholeX\t= %6.2lf\n", query,
			geologyOptions.boreholeX);
	sprintf(query, "%s\tBoreholeX\t= %6.2lf\n", query,
			geologyOptions.boreholeY);
	sprintf(query, "%s\tBoreholeX\t= %6.2lf\n", query,
			geologyOptions.boreholeZ);
	sprintf(query, "%s\tBoreholeDecl\t= %6.2lf\n", query,
			geologyOptions.boreholeDecl);
	sprintf(query, "%s\tBoreholeDip\t= %6.2lf\n", query,
			geologyOptions.boreholeDip);
	sprintf(query, "%s\tBoreholeLength\t= %6.2lf\n", query,
			geologyOptions.boreholeLength);

	sprintf(query, "%s\tSectionX\t= %6.2lf\n", query, geologyOptions.sectionX);
	sprintf(query, "%s\tSectionY\t= %6.2lf\n", query, geologyOptions.sectionY);
	sprintf(query, "%s\tSectionZ\t= %6.2lf\n", query, geologyOptions.sectionZ);
	sprintf(query, "%s\tSectionDecl\t= %6.2lf\n", query,
			geologyOptions.sectionDecl);
	sprintf(query, "%s\tSectionLength\t= %6.2lf\n", query,
			geologyOptions.sectionLength);
	sprintf(query, "%s\tSectionHeight\t= %6.2lf\n", query,
			geologyOptions.sectionHeight);

	if (geologyOptions.useTopography) {
		sprintf(query, "%s\ttopofile\t= TRUE\n", query);
		sprintf(query, "%s\tTopo Filename\t= %s\n", query, topoFileSpec.name);
		//xvt_fsys_convert_dir_to_str(&(topoFileSpec.dir), directory, 200);
	} else {
		sprintf(query, "%s\ttopofile\t= FALSE\n", query);
		sprintf(query, "%s\tTopo Filename\t= %s\n", query, "   ");
		strcpy(directory, ".");
	}
	sprintf(query, "%s\tTopo Directory\t= %s\n", query, directory);

	sprintf(query, "%s\tTopo Scale\t= %6.2lf\n", query,
			geologyOptions.topoScaleFactor);
	sprintf(query, "%s\tTopo Offset\t= %6.2lf\n", query,
			geologyOptions.topoOffsetFactor);
	sprintf(query, "%s\tTopo First Contour\t= %6.2lf\n", query,
			geologyOptions.topoLowContour);
	sprintf(query, "%s\tTopo Contour Interval\t= %6.2lf\n", query,
			geologyOptions.topoContourInterval);

	if (geologyOptions.calculateChairDiagram)
		sprintf(query, "%s\tChair Diagram\t= TRUE\n", query);
	else
		sprintf(query, "%s\tChair Diagram\t= FALSE\n", query);
	sprintf(query, "%s\tChair_X\t= %6.2lf\n", query, geologyOptions.chair_X);
	sprintf(query, "%s\tChair_Y\t= %6.2lf\n", query, geologyOptions.chair_Y);
	sprintf(query, "%s\tChair_Z\t= %6.2lf\n", query, geologyOptions.chair_Z);

	/*
	 sprintf(query,"%s\tLinemap_X\t= %6.2lf\n",query, geologyOptions.linemap_X);
	 sprintf(query,"%s\tLinemap_Y\t= %6.2lf\n",query, geologyOptions.linemap_Y);

	 sprintf(query,"%s\tCube Resolution\t= %.0f\n",query,viewOptions->geologyCubeSize);
	 sprintf(query,"%s\tCube Scale\t= 1.0\n",query);
	 */
	return (TRUE);
}

int upload_Random_gps_opts(query)
	char *query; {
	BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions();

	sprintf(query, "%s\n#GeophysicsOptions\n", query);

	sprintf(query, "%s\tGPSRange\t = %d\n", query,
			geophysicsOptions.calculationRange);
	sprintf(query, "%s\tDeclination\t= %6.2lf\n", query,
			geophysicsOptions.declination);

	sprintf(query, "%s\tInclination\t= %6.2lf\n", query,
			geophysicsOptions.inclination);
	sprintf(query, "%s\tIntensity\t= %6.2lf\n", query,
			geophysicsOptions.intensity);

	if (geophysicsOptions.fieldType == FIXED_FIELD)
		sprintf(query, "%s\tField Type\t= FIXED\n", query);
	else
		sprintf(query, "%s\tField Type\t= VARIABLE\n", query);
	sprintf(query, "%s\tField xPos\t= %6.2lf\n", query, geophysicsOptions.xPos);
	sprintf(query, "%s\tField yPos\t= %6.2lf\n", query, geophysicsOptions.yPos);
	sprintf(query, "%s\tField zPos\t= %6.2lf\n", query, geophysicsOptions.zPos);

	sprintf(query, "%s\tInclination Ori\t= %6.2lf\n", query,
			geophysicsOptions.inclinationOri);
	sprintf(query, "%s\tInclination Change\t= %6.2lf\n", query,
			geophysicsOptions.inclinationChange);
	sprintf(query, "%s\tIntensity Ori\t= %6.2lf\n", query,
			geophysicsOptions.intensityOri);
	sprintf(query, "%s\tIntensity Change\t= %6.2lf\n", query,
			geophysicsOptions.intensityChange);
	sprintf(query, "%s\tDeclination Ori\t= %6.2lf\n", query,
			geophysicsOptions.declinationOri);
	sprintf(query, "%s\tDeclination Change\t= %6.2lf\n", query,
			geophysicsOptions.declinationChange);

	sprintf(query, "%s\tAltitude\t= %6.2lf\n", query,
			geophysicsOptions.altitude);

	if (geophysicsOptions.calculationAltitude)
		sprintf(query, "%s\tAirborne= \tTRUE\n", query);
	else
		sprintf(query, "%s\tAirborne= \tFALSE\n", query);

	if (geophysicsOptions.calculationMethod == SPATIAL)
		sprintf(query, "%s\tCalculation Method\t= SPATIAL\n", query);
	else if (geophysicsOptions.calculationMethod == SPECTRAL)
		sprintf(query, "%s\tCalculation Method\t= SPECTRAL\n", query);
	else
		/* Full */
		sprintf(query, "%s\tCalculation Method\t= FULL\n", query);

	switch (geophysicsOptions.spectralPaddingType) {
	case (RAMP_PADDING):
		sprintf(query, "%s\tSpectral Padding Type\t= RAMP_PADDING\n", query);
		break;
	case (FENCE_MEAN_PADDING):
		sprintf(query, "%s\tSpectral Padding Type\t= FENCE_MEAN_PADDING\n",
				query);
		break;
	case (FENCE_VALUE_PADDING):
		sprintf(query, "%s\tSpectral Padding Type\t= FENCE_VALUE_PADDING\n",
				query);
		break;
	case (SET_MEAN_PADDING):
		sprintf(query, "%s\tSpectral Padding Type\t= SET_MEAN_PADDING\n",
				query);
		break;
	case (SET_VALUE_PADDING):
		sprintf(query, "%s\tSpectral Padding Type\t= SET_VALUE_PADDING\n",
				query);
		break;
	case (REFLECTION_PADDING):
		sprintf(query, "%s\tSpectral Padding Type\t= REFLECTION_PADDING\n",
				query);
		break;
	default:
		sprintf(query, "%s\tSpectral Padding Type\t= UNKNOWN\n", query);
	}
	sprintf(query, "%s\tSpectral Fence\t= %d\n", query,
			geophysicsOptions.spectralFence);
	sprintf(query, "%s\tSpectral Percent\t= %d\n", query,
			geophysicsOptions.spectralPercent);

	sprintf(query, "%s\tConstant Boxing Depth\t= %6.2lf\n", query,
			geophysicsOptions.constantBoxDepth);
	sprintf(query, "%s\tClever Boxing Ratio\t= %6.2lf\n", query,
			geophysicsOptions.cleverBoxRatio);

	if (geophysicsOptions.deformableRemanence)
		sprintf(query, "%s\tDeformable Remanence= \tTRUE\n", query);
	else
		sprintf(query, "%s\tDeformable Remanence= \tFALSE\n", query);

	if (geophysicsOptions.deformableAnisotropy)
		sprintf(query, "%s\tDeformable Anisotropy= \tTRUE\n", query);
	else
		sprintf(query, "%s\tDeformable Anisotropy= \tFALSE\n", query);

	if (geophysicsOptions.magneticVectorComponents)
		sprintf(query, "%s\tVector Components= \tTRUE\n", query);
	else
		sprintf(query, "%s\tVector Components= \tFALSE\n", query);

	if (geophysicsOptions.projectVectorsOntoField)
		sprintf(query, "%s\tProject Vectors= \tTRUE\n", query);
	else
		sprintf(query, "%s\tProject Vectors= \tFALSE\n", query);

	if (geophysicsOptions.padWithRealGeology)
		sprintf(query, "%s\tPad With Real Geology= \tTRUE\n", query);
	else
		sprintf(query, "%s\tPad With Real Geology= \tFALSE\n", query);

	if (geophysicsOptions.drapedSurvey)
		sprintf(query, "%s\tDraped Survey= \tTRUE\n", query);
	else
		sprintf(query, "%s\tDraped Survey= \tFALSE\n", query);
	return (TRUE);
}

int upload_Random_3d_opts(query)
	char *query; {
	sprintf(query, "%s\n#3DOptions\n", query);

	sprintf(query, "%s\tDeclination\t= %f\n", query,
			threedViewOptions.declination);
	sprintf(query, "%s\tElevation\t= %f\n", query, threedViewOptions.azimuth);
	sprintf(query, "%s\tScale\t= %f\n", query, threedViewOptions.scale);

	sprintf(query, "%s\tOffset X\t= %f\n", query, threedViewOptions.offset_X);
	sprintf(query, "%s\tOffset Y\t= %f\n", query, threedViewOptions.offset_Y);
	sprintf(query, "%s\tOffset Z\t= %f\n", query, threedViewOptions.offset_Z);

	sprintf(query, "%s\tFill Type\t= %d\n", query, threedViewOptions.fillType);
	return (TRUE);
}

int upload_Random_proj_opts(query)
	char *query; {
	sprintf(query, "%s\n#ProjectOptions\n", query);

	if (projectOptions.susceptibilityUnits == SI)
		sprintf(query, "%s\tSusceptibility Units\t= SI\n", query);
	else
		sprintf(query, "%s\tSusceptibility Units\t= CGS\n", query);
	sprintf(query, "%s\tGeophysical Calculation\t= %d\n", query,
			projectOptions.geophysicalCalc);
	if (projectOptions.calculationType == REMOTE_JOB)
		sprintf(query, "%s\tCalculation Type\t= REMOTE_JOB\n", query);
	else
		sprintf(query, "%s\tCalculation Type\t= LOCAL_JOB\n", query);
	sprintf(query, "%s\tLength Scale\t= %d\n", query,
			projectOptions.lengthScale);
	sprintf(query, "%s\tPrinting Scale\t= %lf\n", query,
			projectOptions.printScalingFactor);
	sprintf(query, "%s\tImage Scale\t= %lf\n", query,
			projectOptions.imageScalingFactor);

	if (projectOptions.newWindowEachStage)
		sprintf(query, "%s\tNew Windows\t= TRUE\n", query);
	else
		sprintf(query, "%s\tNew Windows\t= FALSE\n", query);

	sprintf(query, "%s\tBackground Red Component\t= %d\n", query,
			XVT_COLOR_GET_RED(backgroundColor));
	sprintf(query, "%s\tBackground Green Component\t= %d\n", query,
			XVT_COLOR_GET_GREEN(backgroundColor));
	sprintf(query, "%s\tBackground Blue Component\t= %d\n", query,
			XVT_COLOR_GET_BLUE(backgroundColor));

	sprintf(query, "%s\tInternet Address\t= %s\n", query,
			projectOptions.internetAddress);
	sprintf(query, "%s\tAccount Name\t= %s\n", query,
			projectOptions.accountName);
	sprintf(query, "%s\tNoddy Path\t= %s\n", query, projectOptions.noddyPath);

	sprintf(query, "%s\tHelp Path\t= %s\n", query, projectOptions.helpPath);

	sprintf(query, "%s\tMovie Frames Per Event\t= %d\n", query,
			projectOptions.movieFramesPerEvent);
	sprintf(query, "%s\tMovie Play Speed\t= %6.2lf\n", query,
			projectOptions.moviePlaySpeed);
	sprintf(query, "%s\tMovie Type\t= %d\n", query, projectOptions.movieType);

	if (projectOptions.gravityDisplay.clippingType == ABSOLUTE_CLIPPING)
		sprintf(query, "%s\tGravity Clipping Type\t= ABSOLUTE_CLIPPING\n",
				query);
	else
		sprintf(query, "%s\tGravity Clipping Type\t= RELATIVE_CLIPPING\n",
				query);
	sprintf(query, "%s\tGravity Image Display Clip Min\t= %lf\n", query,
			projectOptions.gravityDisplay.minClip);
	sprintf(query, "%s\tGravity Image Display Clip Max\t= %lf\n", query,
			projectOptions.gravityDisplay.maxClip);
	if (projectOptions.gravityDisplay.display == CONTOUR_IMAGE)
		sprintf(query, "%s\tGravity Image Display Type\t= CONTOUR\n", query);
	else if (projectOptions.gravityDisplay.display == PSEUDO_COLOR)
		sprintf(query, "%s\tGravity Image Display Type\t= PSEUDO_COLOR\n",
				query);
	else
		/* GREY_COLOR */
		sprintf(query, "%s\tGravity Image Display Type\t= GREY\n", query);
	sprintf(query, "%s\tGravity Image Display Num Contour\t= %d\n", query,
			projectOptions.gravityDisplay.numContours);

	if (projectOptions.magneticsDisplay.clippingType == ABSOLUTE_CLIPPING)
		sprintf(query, "%s\tMagnetics Clipping Type\t= ABSOLUTE_CLIPPING\n",
				query);
	else
		sprintf(query, "%s\tMagnetics Clipping Type\t= RELATIVE_CLIPPING\n",
				query);
	sprintf(query, "%s\tMagnetics Image Display Clip Min\t= %lf\n", query,
			projectOptions.magneticsDisplay.minClip);
	sprintf(query, "%s\tMagnetics Image Display Clip Max\t= %lf\n", query,
			projectOptions.magneticsDisplay.maxClip);
	if (projectOptions.magneticsDisplay.display == CONTOUR_IMAGE)
		sprintf(query, "%s\tMagnetics Image Display Type\t= CONTOUR\n", query);
	else if (projectOptions.magneticsDisplay.display == PSEUDO_COLOR)
		sprintf(query, "%s\tMagnetics Image Display Type\t= PSEUDO_COLOR\n",
				query);
	else
		/* GREY_COLOR */
		sprintf(query, "%s\tMagnetics Image Display Type\t= GREY\n", query);
	sprintf(query, "%s\tMagnetics Image Display Num Contour\t= %d\n", query,
			projectOptions.magneticsDisplay.numContours);

	sprintf(query, "%s\tFalse Easting\t= %lf\n", query, projectOptions.easting);
	sprintf(query, "%s\tFalse Northing\t= %lf\n", query,
			projectOptions.northing);

	return (TRUE);
}

int upload_Random_folds(query, options)
	char *query;FOLD_OPTIONS *options; {
	int i;

	if (options->type == SINE_FOLD)
		sprintf(query, "%s\tType\t= %s\n", query, "Sine");
	else if (options->type == BIOT_FOLD)
		sprintf(query, "%s\tType\t= %s\n", query, "Biot");
	else
		sprintf(query, "%s\tType\t= %s\n", query, "Fourier");

	if (options->singleFold)
		sprintf(query, "%s\tSingle Fold\t= TRUE\n", query);
	else
		sprintf(query, "%s\tSingle Fold\t= FALSE\n", query);

	sprintf(query, "%s\tX\t= %6.2lf\n", query, options->positionX);
	sprintf(query, "%s\tY\t= %6.2lf\n", query, options->positionY);
	sprintf(query, "%s\tZ\t= %6.2lf\n", query, options->positionZ);

	sprintf(query, "%s\tDip Direction\t= %6.2lf\n", query,
			options->dipDirection);
	sprintf(query, "%s\tDip\t= %6.2lf\n", query, options->dip);
	sprintf(query, "%s\tPitch\t= %6.2lf\n", query, options->axisPitch);

	sprintf(query, "%s\tWavelength\t= %6.2lf\n", query, options->wavelength);
	sprintf(query, "%s\tAmplitude\t= %6.2lf\n", query, options->amplitude);
	sprintf(query, "%s\tCylindricity\t= %6.2lf\n", query, options->cycle);

	sprintf(query, "%s\tFourier Series\n", query);
	for (i = 0; i < 11; i++) {
		sprintf(query, "%s\t\tTerm A %d\t= %6.2lf\n", query, i,
				options->fourierCoeff[0][i]);
		sprintf(query, "%s\t\tTerm B %d\t= %6.2lf\n", query, i,
				options->fourierCoeff[1][i]);
	}

	uploadRandomProfile(query, &(options->profile));

	return (TRUE);
}

int upload_Random_dykes(query, options)
	char *query;DYKE_OPTIONS *options; {
	if (options->type == DILATION_DYKE)
		sprintf(query, "%s\tType\t= %s\n", query, "Dilate");
	else
		sprintf(query, "%s\tType\t= %s\n", query, "Stope");

	sprintf(query, "%s\tMerge Events\t= %d\n", query, options->mergeEvents);

	sprintf(query, "%s\tX\t= %6.2lf\n", query, options->positionX);
	sprintf(query, "%s\tY\t= %6.2lf\n", query, options->positionY);
	sprintf(query, "%s\tZ\t= %6.2lf\n", query, options->positionZ);

	sprintf(query, "%s\tDip Direction\t= %6.2lf\n", query,
			options->dipDirection);
	sprintf(query, "%s\tDip\t= %6.2lf\n", query, options->dip);
	sprintf(query, "%s\tPitch\t = %6.2lf\n", query, options->slipPitch);

	sprintf(query, "%s\tSlip\t = %6.2lf\n", query, options->slipLength);
	sprintf(query, "%s\tWidth\t = %6.2lf\n", query, options->width);

	if (options->alterationZones == NONE_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= NONE\n", query);
	else if (options->alterationZones == TOP_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= TOP\n", query);
	else if (options->alterationZones == BOTTOM_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= BOTTOM\n", query);
	else
		/* BOTH_ALTERATION */
		sprintf(query, "%s\tAlteration Type \t= BOTH\n", query);

	uploadRandomAlteration(query, options->alterationFuntions);

	uploadRandomProperties(query, &(options->properties));

	return (TRUE);
}

int upload_Random_faults(query, options)
	char *query;FAULT_OPTIONS *options; {
	int i;

	if (options->geometry == TRANSLATION)
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Translation");
	else if (options->geometry == ROTATION)
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Rotation");
	else if (options->geometry == ELLIPTICAL)
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Elliptical");
	else if (options->geometry == CURVED)
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Curved");
	else
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Ring");

	if (options->movement == HANGING_WALL)
		sprintf(query, "%s\tMovement\t= %s\n", query, "Hanging Wall");
	else if (options->movement == FOOT_WALL)
		sprintf(query, "%s\tMovement\t= %s\n", query, "Foot Wall");
	else
		sprintf(query, "%s\tMovement\t= %s\n", query, "Both");

	sprintf(query, "%s\tX\t= %6.2lf\n", query, options->positionX);
	sprintf(query, "%s\tY\t= %6.2lf\n", query, options->positionY);
	sprintf(query, "%s\tZ\t= %6.2lf\n", query, options->positionZ);

	sprintf(query, "%s\tDip Direction\t= %6.2lf\n", query,
			options->dipDirection);
	sprintf(query, "%s\tDip\t= %6.2lf\n", query, options->dip);

	sprintf(query, "%s\tPitch\t= %6.2lf\n", query, options->pitch);
	sprintf(query, "%s\tSlip\t= %6.2lf\n", query, options->slip);
	sprintf(query, "%s\tRotation\t= %6.2lf\n", query, options->rotation);
	sprintf(query, "%s\tAmplitude\t= %6.2lf\n", query, options->amplitude);
	sprintf(query, "%s\tRadius\t= %6.2lf\n", query, options->radius);
	sprintf(query, "%s\tXAxis\t= %6.2lf\n", query, options->xAxis);
	sprintf(query, "%s\tYAxis\t= %6.2lf\n", query, options->yAxis);
	sprintf(query, "%s\tZAxis\t= %6.2lf\n", query, options->zAxis);
	sprintf(query, "%s\tCyl Index\t= %6.2lf\n", query,
			options->cylindricalIndex);
	sprintf(query, "%s\tProfile Pitch\t= %6.2lf\n", query,
			options->profilePitch);

	uploadRandomColor(query, &(options->color));

	sprintf(query, "%s\tFourier Series\n", query);
	for (i = 0; i < 11; i++) {
		sprintf(query, "%s\t\tTerm A %d\t= %6.2lf\n", query, i,
				options->fourierCoeff[0][i]);
		sprintf(query, "%s\t\tTerm B %d\t= %6.2lf\n", query, i,
				options->fourierCoeff[1][i]);
	}

	uploadRandomProfile(query, &(options->profile));

	if (options->alterationZones == NONE_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= NONE\n", query);
	else if (options->alterationZones == TOP_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= TOP\n", query);
	else if (options->alterationZones == BOTTOM_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= BOTTOM\n", query);
	else
		/* BOTH_ALTERATION */
		sprintf(query, "%s\tAlteration Type \t= BOTH\n", query);

	uploadRandomAlteration(query, options->alterationFuntions);

	uploadRandomSurface(query, options->surfaceType,
			&(options->surfaceFilename), options->surfaceXDim,
			options->surfaceYDim, options->surfaceZDim);

	return (TRUE);
}

int upload_Random_shear_zone(query, options)
	char *query;SHEAR_OPTIONS *options; {
	int i;

	if (options->type == SHEAR_ZONE_EVENT)
		sprintf(query, "%s\tType\t= %s\n", query, "Shear Zone");
	else
		sprintf(query, "%s\tType\t= %s\n", query, "Kink");

	if (options->geometry == TRANSLATION)
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Translation");
	else if (options->geometry == ROTATION)
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Rotation");
	else if (options->geometry == ELLIPTICAL)
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Elliptical");
	else if (options->geometry == CURVED)
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Curved");
	else
		sprintf(query, "%s\tGeometry\t= %s\n", query, "Ring");

	if (options->movement == HANGING_WALL)
		sprintf(query, "%s\tMovement\t= %s\n", query, "Hanging Wall");
	else if (options->movement == FOOT_WALL)
		sprintf(query, "%s\tMovement\t= %s\n", query, "Foot Wall");
	else
		sprintf(query, "%s\tMovement\t= %s\n", query, "Both");

	sprintf(query, "%s\tX\t= %6.2lf\n", query, options->positionX);
	sprintf(query, "%s\tY\t= %6.2lf\n", query, options->positionY);
	sprintf(query, "%s\tZ\t= %6.2lf\n", query, options->positionZ);

	sprintf(query, "%s\tDip Direction\t= %6.2lf\n", query,
			options->dipDirection);
	sprintf(query, "%s\tDip\t= %6.2lf\n", query, options->dip);

	sprintf(query, "%s\tPitch\t= %6.2lf\n", query, options->pitch);
	sprintf(query, "%s\tSlip\t= %6.2lf\n", query, options->slip);
	sprintf(query, "%s\tRotation\t= %6.2lf\n", query, options->rotation);
	sprintf(query, "%s\tAmplitude\t= %6.2lf\n", query, options->amplitude);
	sprintf(query, "%s\tWidth\t= %6.2lf\n", query, options->width);
	sprintf(query, "%s\tRadius\t= %6.2lf\n", query, options->radius);
	sprintf(query, "%s\tXAxis\t= %6.2lf\n", query, options->xAxis);
	sprintf(query, "%s\tYAxis\t= %6.2lf\n", query, options->yAxis);
	sprintf(query, "%s\tZAxis\t= %6.2lf\n", query, options->zAxis);
	sprintf(query, "%s\tCyl Index\t= %6.2lf\n", query,
			options->cylindricalIndex);
	sprintf(query, "%s\tProfile Pitch\t= %6.2lf\n", query,
			options->profilePitch);

	uploadRandomColor(query, &(options->color));

	sprintf(query, "%s\tFourier Series\n", query);
	for (i = 0; i < 11; i++) {
		sprintf(query, "%s\t\tTerm A %d\t= %6.2lf\n", query, i,
				options->fourierCoeff[0][i]);
		sprintf(query, "%s\t\tTerm B %d\t= %6.2lf\n", query, i,
				options->fourierCoeff[1][i]);
	}

	uploadRandomProfile(query, &(options->profile));

	if (options->alterationZones == NONE_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= NONE\n", query);
	else if (options->alterationZones == TOP_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= TOP\n", query);
	else if (options->alterationZones == BOTTOM_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= BOTTOM\n", query);
	else
		/* BOTH_ALTERATION */
		sprintf(query, "%s\tAlteration Type \t= BOTH\n", query);

	uploadRandomAlteration(query, options->alterationFuntions);

	uploadRandomSurface(query, options->surfaceType,
			&(options->surfaceFilename), options->surfaceXDim,
			options->surfaceYDim, options->surfaceZDim);

	return (TRUE);
}

int upload_Random_lineations(query, options)
	char *query;LINEATION_OPTIONS *options; {
	sprintf(query, "%s\tPlunge Direction\t = %6.2lf\n", query,
			options->plungeDirection);
	sprintf(query, "%s\tPlunge\t = %6.2lf\n", query, options->plunge);
	return (TRUE);
}

int upload_Random_planes(query, options)
	char *query;FOLIATION_OPTIONS *options; {
	sprintf(query, "%s\tDip Direction\t = %6.2lf\n", query,
			options->dipDirection);
	sprintf(query, "%s\tDip\t = %6.2lf\n", query, options->dip);
	return (TRUE);
}

int upload_Random_plugs(query, options)
	char *query;PLUG_OPTIONS *options; {
	if (options->type == CYLINDRICAL_PLUG)
		sprintf(query, "%s\tType\t= %s\n", query, "Cylindrical");
	else if (options->type == CONE_PLUG)
		sprintf(query, "%s\tType\t= %s\n", query, "Cone");
	else if (options->type == PARABOLIC_PLUG)
		sprintf(query, "%s\tType\t= %s\n", query, "Parabolic");
	else
		sprintf(query, "%s\tType\t= %s\n", query, "Ellipsodial");

	sprintf(query, "%s\tMerge Events\t= %d\n", query, options->mergeEvents);

	sprintf(query, "%s\tX\t= %6.2lf\n", query, options->positionX);
	sprintf(query, "%s\tY\t= %6.2lf\n", query, options->positionY);
	sprintf(query, "%s\tZ\t= %6.2lf\n", query, options->positionZ);

	sprintf(query, "%s\tDip Direction\t= %6.2lf\n", query,
			options->dipDirection);
	sprintf(query, "%s\tDip\t= %6.2lf\n", query, options->dip);
	sprintf(query, "%s\tPitch\t = %6.2lf\n", query, options->axisPitch);

	sprintf(query, "%s\tRadius\t = %6.2lf\n", query, options->radius);
	sprintf(query, "%s\tApicalAngle\t = %6.2lf\n", query, options->apicalAngle);
	sprintf(query, "%s\tB-value\t = %6.2lf\n", query, options->BValue);
	sprintf(query, "%s\tA-value\t = %6.2lf\n", query, options->axisA);
	sprintf(query, "%s\tB-value\t = %6.2lf\n", query, options->axisB);
	sprintf(query, "%s\tC-value\t = %6.2lf\n", query, options->axisC);

	if (options->alterationZones == NONE_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= NONE\n", query);
	else if (options->alterationZones == TOP_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= TOP\n", query);
	else if (options->alterationZones == BOTTOM_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= BOTTOM\n", query);
	else
		/* BOTH_ALTERATION */
		sprintf(query, "%s\tAlteration Type \t= BOTH\n", query);

	uploadRandomAlteration(query, options->alterationFuntions);

	uploadRandomProperties(query, &(options->properties));

	return (TRUE);
}

int upload_Random_tilts(query, options)
	char *query;TILT_OPTIONS *options; {
	sprintf(query, "%s\tX\t= %6.2lf\n", query, options->positionX);
	sprintf(query, "%s\tY\t= %6.2lf\n", query, options->positionY);
	sprintf(query, "%s\tZ\t= %6.2lf\n", query, options->positionZ);

	sprintf(query, "%s\tRotation\t = %6.2lf\n", query, options->rotation);
	sprintf(query, "%s\tPlunge Direction\t = %6.2lf\n", query,
			options->plungeDirection);
	sprintf(query, "%s\tPlunge\t = %6.2lf\n", query, options->plunge);
	return (TRUE);
}

int upload_Random_strains(query, options)
	char *query;STRAIN_OPTIONS *options; {
	sprintf(query, "%s\tA\t = %6.2lf\n", query, options->tensor[0][0]);
	sprintf(query, "%s\tB\t = %6.2lf\n", query, options->tensor[1][0]);
	sprintf(query, "%s\tC\t = %6.2lf\n", query, options->tensor[2][0]);
	sprintf(query, "%s\tD\t = %6.2lf\n", query, options->tensor[0][1]);
	sprintf(query, "%s\tE\t = %6.2lf\n", query, options->tensor[1][1]);
	sprintf(query, "%s\tF\t = %6.2lf\n", query, options->tensor[2][1]);
	sprintf(query, "%s\tG\t = %6.2lf\n", query, options->tensor[0][2]);
	sprintf(query, "%s\tH\t = %6.2lf\n", query, options->tensor[1][2]);
	sprintf(query, "%s\tI\t = %6.2lf\n", query, options->tensor[2][2]);
	return (TRUE);
}

int upload_Random_import(query, options)
	char *query;IMPORT_OPTIONS *options; {
	int i;
	char temp[255];

	sprintf(query, "%s\tFilename\t = %s\n", query, options->importFile.name);
	//xvt_fsys_convert_dir_to_str(&(options->importFile.dir), temp, 255);
	sprintf(query, "%s\tDirectory\t = %s\n", query, temp);

	if (options->positionBy == POS_BY_CENTER)
		sprintf(query, "%s\tPosition By\t = Center\n", query);
	else
		sprintf(query, "%s\tPosition By\t = Corner\n", query);

	sprintf(query, "%s\tX\t= %6.2lf\n", query, options->positionX);
	sprintf(query, "%s\tY\t= %6.2lf\n", query, options->positionY);
	sprintf(query, "%s\tZ\t= %6.2lf\n", query, options->positionZ);

	sprintf(query, "%s\tBlock Size\t= %6.2lf\n", query, options->blockSize);
	sprintf(query, "%s\tFile Format\t= Noddy\n");

	sprintf(query, "%s\tNum Properties\t = %d\n", query,
			options->numProperties);
	for (i = 0; i < options->numProperties; i++) {
		uploadRandomProperties(query, &(options->properties[i]));
	}
	return (TRUE);
}

int upload_Random_generic(query, options)
	char *query;GENERIC_OPTIONS *options; {
	sprintf(query, "%s\tParam1\t = %6.2lf\n", query, options->param1);
	sprintf(query, "%s\tParam2\t = %6.2lf\n", query, options->param2);
	sprintf(query, "%s\tParam3\t = %6.2lf\n", query, options->param3);
	sprintf(query, "%s\tParam4\t = %6.2lf\n", query, options->param4);
	sprintf(query, "%s\tParam5\t = %6.2lf\n", query, options->param5);
	sprintf(query, "%s\tParam6\t = %6.2lf\n", query, options->param6);
	sprintf(query, "%s\tParam7\t = %6.2lf\n", query, options->param7);
	sprintf(query, "%s\tParam8\t = %6.2lf\n", query, options->param8);
	sprintf(query, "%s\tParam9\t = %6.2lf\n", query, options->param9);
	sprintf(query, "%s\tParam10\t = %6.2lf\n", query, options->param10);
	return (TRUE);
}

int upload_Random_unconformities(query, options)
	char *query;UNCONFORMITY_OPTIONS *options; {
	sprintf(query, "%s\tX\t= %6.2lf\n", query, options->positionX);
	sprintf(query, "%s\tY\t= %6.2lf\n", query, options->positionY);
	sprintf(query, "%s\tZ\t= %6.2lf\n", query, options->positionZ);

	sprintf(query, "%s\tDip Direction\t= %6.2lf\n", query,
			options->dipDirection);
	sprintf(query, "%s\tDip\t= %6.2lf\n", query, options->dip);

	if (options->alterationZones == NONE_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= NONE\n", query);
	else if (options->alterationZones == TOP_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= TOP\n", query);
	else if (options->alterationZones == BOTTOM_ALTERATION)
		sprintf(query, "%s\tAlteration Type \t= BOTTOM\n", query);
	else
		/* BOTH_ALTERATION */
		sprintf(query, "%s\tAlteration Type \t= BOTH\n", query);

	uploadRandomAlteration(query, options->alterationFuntions);

	uploadRandomSurface(query, options->surfaceType,
			&(options->surfaceFilename), options->surfaceXDim,
			options->surfaceYDim, options->surfaceZDim);

	uploadRandomStratigraphy(query, &(options->stratigraphyDetails));

	return (TRUE);
}

int upload_Random_file_history(filename, query)
	char *filename;char *query; {
	time_t longTime;
	struct tm *today;

	time(&longTime); /* current Time */
	today = localtime(&longTime);

	sprintf(query, "%s#Filename = %s\n", query, filename);
	sprintf(query, "%s#Date Saved = %d/%d/%d %d:%d:%d\n", query, today->tm_mday,
			today->tm_mon + 1, today->tm_year + 1900, today->tm_hour,
			today->tm_min, today->tm_sec);
	sprintf(query, "%sFileType = %d\nVersion = %.2f\n\n", query, 111,
	VERSION_NUMBER);
	//printf("\n\nfhist= %s\n",query);
	return (TRUE);
}

int upload_Random_hist(query)
	char *query; {
	WINDOW win;
	OBJECT *p;
	register int numEvents, event;
    char events[200];

	win = getEventDrawingWindow();
	numEvents = totalObjects(win);

	sprintf(query, "%s\nNo of Events\t= %d\n", query, numEvents);
	for (event = 0; event < numEvents; event++) {
		p = (OBJECT*) nthObject(win, event);

		DEBUG(printf("\n p->shape = %d", p->shape);)
		if (p) {
			switch (p->shape) {
			case STRATIGRAPHY: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"STRATIGRAPHY");
				uploadRandomStratigraphy(query,
						(STRATIGRAPHY_OPTIONS*) p->options);
                sprintf(events,"UPDATE public.universe SET events ='STRAT");
				break;
			}
			case FOLD: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"FOLD");
				upload_Random_folds(query, (FOLD_OPTIONS*) p->options);
				DEBUG(printf("\nfo p->shape = %d", p->shape);)
                strcat(events,"-FOLD");

				break;
			}
			case FAULT: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"FAULT");
				upload_Random_faults(query, (FAULT_OPTIONS*) p->options);
				DEBUG(printf("\nfa p->shape = %d\n",query, p->shape);)
                strcat(events,"-FAUL");

				break;
			}
			case UNCONFORMITY: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"UNCONFORMITY");
				upload_Random_unconformities(query,
						(UNCONFORMITY_OPTIONS*) p->options);
				DEBUG(printf("\nuc p->shape = %d", p->shape);)
                strcat(events,"-UNCO");

				break;
			}
			case SHEAR_ZONE: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"SHEAR_ZONE");
				upload_Random_shear_zone(query, (SHEAR_OPTIONS*) p->options);
				DEBUG(printf("\nsz p->shape = %d", p->shape);)
                strcat(events,"-SHEA");

				break;
			}
			case DYKE: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"DYKE");
				upload_Random_dykes(query, (DYKE_OPTIONS*) p->options);
                strcat(events,"-DYKE");

				break;
			}
			case PLUG: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"PLUG");
				upload_Random_plugs(query, (PLUG_OPTIONS*) p->options);
				DEBUG(printf("\npl p->shape = %d", p->shape);)
                strcat(events,"-PLUG");

				break;
			}
			case STRAIN: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"STRAIN");
				upload_Random_strains(query, (STRAIN_OPTIONS*) p->options);
                strcat(events,"-STRA");
				break;
			}
			case TILT: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"TILT");
				upload_Random_tilts(query, (TILT_OPTIONS*) p->options);
                strcat(events,"-TILT");
				break;
			}
			case FOLIATION: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"FOLIATION");
				upload_Random_planes(query, (FOLIATION_OPTIONS*) p->options);
                strcat(events,"-FOLI");
				break;
			}
			case LINEATION: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"LINEATION");
				upload_Random_lineations(query,
						(LINEATION_OPTIONS*) p->options);
                strcat(events,"-LINE");
				break;
			}
			case IMPORT: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"IMPORT");
				upload_Random_import(query, (IMPORT_OPTIONS*) p->options);
                strcat(events,"-IMPO");
				break;
			}
			case GENERIC: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"GENERIC");
				upload_Random_generic(query, (GENERIC_OPTIONS*) p->options);
                strcat(events,"-GENE");
				break;
			}
			case STOP: {
				sprintf(query, "%sEvent #%d\t= %s\n", query, (event + 1),
						"STOP");
                strcat(events,"-STOP");
				break;
			}
			}
			sprintf(query, "%s\tName\t= %s\n", query, p->text);
		}
	}
    sprintf(events,"%s' WHERE UID = '%s'",events,time_stamp);
	insert_query(events, 1);
    //printf("%s\n",events);


	return (TRUE);
}

int uploadRandomStratigraphy(query, options)
	char *query;STRATIGRAPHY_OPTIONS *options; {
	int i;

	sprintf(query, "%s\tNum Layers\t= %d\n", query, options->numLayers);
	for (i = 0; i < options->numLayers; i++) {
		uploadRandomProperties(query, &(options->properties[i]));
	}
	return (TRUE);
}

int uploadRandomProperties(query, options)
	char *query;LAYER_PROPERTIES *options; {
	sprintf(query, "%s\tUnit Name\t= %s\n", query, options->unitName);
	sprintf(query, "%s\tHeight\t= %d\n", query, options->height);
	if (options->applyAlterations)
		sprintf(query, "%s\tApply Alterations\t= ON\n", query);
	else
		sprintf(query, "%s\tApply Alterations\t= OFF\n", query);

	sprintf(query, "%s\tDensity\t= %6.2le\n", query, options->density);

	sprintf(query, "%s\tAnisotropic Field\t= %d\n", query,
			options->anisotropicField);
	sprintf(query, "%s\tMagSusX\t= %6.2le\n", query, options->sus_X);
	sprintf(query, "%s\tMagSusY\t= %6.2le\n", query, options->sus_Y);
	sprintf(query, "%s\tMagSusZ\t= %6.2le\n", query, options->sus_Z);
	sprintf(query, "%s\tMagSus Dip\t= %6.2le\n", query, options->sus_dip);
	sprintf(query, "%s\tMagSus DipDir\t= %6.2le\n", query,
			options->sus_dipDirection);
	sprintf(query, "%s\tMagSus Pitch\t= %6.2le\n", query, options->sus_pitch);

	sprintf(query, "%s\tRemanent Magnetization\t= %d\n", query,
			options->remanentMagnetization);
	sprintf(query, "%s\tInclination\t= %6.2lf\n", query, options->inclination);
	sprintf(query, "%s\tAngle with the Magn. North\t= %6.2lf\n", query,
			options->angleWithNorth);
	sprintf(query, "%s\tStrength\t= %6.2le\n", query, options->strength);

	uploadRandomColor(query, &(options->color));
	return (TRUE);
}

int uploadRandomColor(query, options)
	char *query;NODDY_COLOUR *options; {
	sprintf(query, "%s\tColor Name\t= %s\n", query, options->name);
	sprintf(query, "%s\tRed\t= %d\n", query, options->red);
	sprintf(query, "%s\tGreen\t= %d\n", query, options->green);
	sprintf(query, "%s\tBlue\t= %d\n", query, options->blue);
	return (TRUE);
}

int uploadRandomAlteration(query, options)
	char *query;PROFILE_OPTIONS *options; {
	PROFILE_OPTIONS *profOptions;
	int numProfiles = 0;

	for (profOptions = options; profOptions; profOptions = profOptions->next)
		numProfiles++;

	sprintf(query, "%s\tNum Profiles\t= %d\n", query, numProfiles);
	for (profOptions = options; profOptions; profOptions = profOptions->next) {
		uploadRandomProfile(query, profOptions);
	}

	return (TRUE);
}

int uploadRandomProfile(query, options)
	char *query;PROFILE_OPTIONS *options; {
	PROFILE_POINT *point;
	int numPoints = 0;

	sprintf(query, "%s\tName\t= %s\n", query, options->name);
	sprintf(query, "%s\tType\t= %d\n", query, options->type);
	if (options->joinType == LINE_SEGMENTS)
		sprintf(query, "%s\tJoin Type \t= LINES\n", query);
	else if (options->joinType == CURVE_SEGMENTS)
		sprintf(query, "%s\tJoin Type \t= CURVES\n", query);
	else
		sprintf(query, "%s\tJoin Type \t= SQUARE\n", query);
	sprintf(query, "%s\tGraph Length\t= %lf\n", query, options->graphLength);
	sprintf(query, "%s\tMin X\t= %lf\n", query, options->minX);
	sprintf(query, "%s\tMax X\t= %lf\n", query, options->maxX);
	sprintf(query, "%s\tMin Y Scale\t= %lf\n", query, options->minYScale);
	sprintf(query, "%s\tMax Y Scale\t= %lf\n", query, options->maxYScale);
	sprintf(query, "%s\tScale Origin\t= %lf\n", query, options->scaleOrigin);
	sprintf(query, "%s\tMin Y Replace\t= %lf\n", query, options->minYReplace);
	sprintf(query, "%s\tMax Y Replace\t= %lf\n", query, options->maxYReplace);
	/* Count the number of points we have */
	for (point = options->points; point; point = point->next)
		numPoints++;

	sprintf(query, "%s\tNum Points\t= %d\n", query, numPoints);
	for (point = options->points; point; point = point->next) {
		sprintf(query, "%s\t\tPoint X\t= %d\n", query, (int) point->point.h);
		sprintf(query, "%s\t\tPoint Y\t= %d\n", query, (int) point->point.v);
	}

	return (TRUE);
}

int uploadRandomSurface(query, surfaceType, surfaceFilename, surfaceXDim,
		surfaceYDim, surfaceZDim)
	char *query;SURFACE_TYPE surfaceType;FILE_SPEC *surfaceFilename;double
			surfaceXDim, surfaceYDim, surfaceZDim; {
	char strVal[100];

	strVal[0] = '\0';
	if (surfaceType == DXF_SURFACE)
		sprintf(query, "%s\tSurface Type\t= DXF_SURFACE\n", query);
	else if (surfaceType == UNKNOWN_SURFACE)
		sprintf(query, "%s\tSurface Type\t= UNKNOWN_SURFACE\n", query);
	else
		/* FLAT_SURFACE */
		sprintf(query, "%s\tSurface Type\t= FLAT_SURFACE\n", query);
	sprintf(query, "%s\tSurface Filename\t= %s\n", query,
			surfaceFilename->name);
	// if (!xvt_fsys_convert_dir_to_str(&(surfaceFilename->dir), strVal, 100))
	//    strcpy (strVal, ".");
	sprintf(query, "%s\tSurface Directory\t= %s\n", query, strVal);
	sprintf(query, "%s\tSurface XDim\t= %lf\n", query, surfaceXDim);
	sprintf(query, "%s\tSurface YDim\t= %lf\n", query, surfaceYDim);
	sprintf(query, "%s\tSurface ZDim\t= %lf\n", query, surfaceZDim);

	return (TRUE);
}

int upload_Random_icon_positions(char *query) {
	int i;
	WINDOW win;
	register int numEvents, event;

	win = getEventDrawingWindow();
	numEvents = totalRandomObjects(win);

	sprintf(query, "%s#Icon Positions\n", query);
	sprintf(query, "%s	Num Icons	= %d\n", query, numEvents);

	for (i = 1; i < numEvents + 1; i++) {
		sprintf(query, "%s	Row	= 1\n", query);
		sprintf(query, "%s	Column	= %d\n", query, i);
		sprintf(query, "%s	X Position	= %d\n", query, i + 1);
		sprintf(query, "%s	Y Position	= 1\n", query);
	}

	sprintf(query, "%s	Floating Menu Rows	= 1\n", query);
	sprintf(query, "%s	Floating Menu Cols	= 14\n", query);
	sprintf(query, "%sEnd of Status Report\n", query);

}

/* ======================================================================
 FUNCTION        uploadBlockHeader
 DESCRIPTION
 upload a block Header to a database
 INPUT
 char *filename; - base name for file
 int numLayers; - z dimension of block
 int **layerDimensions; - the x, y dimensions of each layer
 double topX, topY, topZ; - geological coordinates of volume start
 double botX, botY, botZ; - geological coordinates of volume end
 int numCubeSizes, *cubeSizes - cube sizes for layers
 ing calcRange; -
 double inclination; - inclination of earths mag field
 double intensity; - intensity of earth mag field
 double declination; - declination of volume wrt mag North
 int densityCalc, susCalc, remCalc, aniCalc;
 - flags showing which file are generated

 RETURNED
 TRUE if it is a valid file
 FALSE on error
 ====================================================================== */
int
#if XVT_CC_PROTO
uploadBlockHeader(char *filename, int numLayers, int **layerDimensions,
		double topX, double topY, double topZ, double botX, double botY,
		double botZ, int numCubeSizes, int *cubeSizes, int calcRange,
		double inclination, double intensity, double declination,
		int densityCalc, int susCalc, int remCalc, int aniCalc, int indexCalc,
		int numProps, LAYER_PROPERTIES **layerProps, char *query)
#else
uploadBlockHeader (filename, numLayers, layerDimensions,
               topX, topY, topZ, botX, botY, botZ,
               numCubeSizes, cubeSizes,
               calcRange, inclination, intensity, declination,
               densityCalc, susCalc, remCalc, aniCalc, indexCalc,
               numProps, layerProps,query)
char *filename;
int numLayers, **layerDimensions;
double topX, topY, topZ;
double botX, botY, botZ;
int numCubeSizes, *cubeSizes, calcRange;
double inclination, intensity, declination;
int densityCalc, susCalc, remCalc, aniCalc;
int indexCalc, numProps;
LAYER_PROPERTIES **layerProps;
char *query;
#endif
{
	FILE *fo;
	char filePrefix[50], dateString[50], timeString[50];
	char *charPtr, *densityPtr, *susPtr, *remPtr, *aniPtr, *indexedPtr;
	char yesString[4], noString[3];
	double minX, minY, minZ, maxX, maxY, maxZ;
	int i;

	strcpy(yesString, "Yes");
	strcpy(noString, "No");

	sprintf(query, "INSERT INTO public.block (uid, g00, g12) VALUES ('%s','",
			time_stamp);

	/* extract file prefix */
	strcpy(filePrefix, filename);
	if (charPtr = (char*) strchr(filePrefix, '.'))
		*charPtr = '\0'; /* terminate the string before extention */

	/* build date string */
	strcpy(dateString, "01/01/90");

	/* build time string */
	strcpy(timeString, "12:00:00");

	/* Assign files that are calculated */
	if (densityCalc)
		densityPtr = yesString;
	else
		densityPtr = noString;

	if (susCalc)
		susPtr = yesString;
	else
		susPtr = noString;

	if (remCalc)
		remPtr = yesString;
	else
		remPtr = noString;

	if (aniCalc)
		aniPtr = yesString;
	else
		aniPtr = noString;

	if (indexCalc)
		indexedPtr = yesString;
	else
		indexedPtr = noString;

	/* ************************** */
	/* *** Write out the file *** */
	sprintf(query, "%sVERSION = %.2lf\n", query, VERSION_NUMBER);
	sprintf(query, "%sFILE PREFIX = %s\n", query, filePrefix);
	sprintf(query, "%sDATE = %s\n", query, dateString);
	sprintf(query, "%sTIME = %s\n", query, timeString);

	minX = MIN(topX, botX);
	maxX = MAX(topX, botX);
	minY = MIN(topY, botY);
	maxY = MAX(topY, botY);
	minZ = MIN(topZ, botZ);
	maxZ = MAX(topZ, botZ);
	sprintf(query, "%sUPPER SW CORNER (X Y Z) = %.1lf %.1lf %.1lf\n", query,
			minX, minY, maxZ);
	sprintf(query, "%sLOWER NE CORNER (X Y Z) = %.1lf %.1lf %.1lf\n", query,
			maxX, maxY, minZ);
	sprintf(query, "%sNUMBER OF LAYERS = %d\n", query, numLayers);
	for (i = 0; i < numLayers; i++) {
		sprintf(query, "%s\tLAYER %d DIMENSIONS (X Y) = %d %d\n", query, i + 1,
				layerDimensions[i][0], layerDimensions[i][1]);
	}

	sprintf(query, "%sNUMBER OF CUBE SIZES = %d\n", query, numCubeSizes);
	for (i = 0; i < numCubeSizes; i++) {
		sprintf(query, "%s\tCUBE SIZE FOR LAYER %d = %d\n", query, i + 1,
				cubeSizes[i]);
	}

	sprintf(query, "%sCALCULATION RANGE = %d\n", query, calcRange);
	sprintf(query, "%sINCLINATION OF EARTH MAG FIELD = %.2lf\n", query,
			inclination);
	sprintf(query, "%sINTENSITY OF EARTH MAG FIELD = %.2lf\n", query,
			intensity);
	sprintf(query, "%sDECLINATION OF VOL. WRT. MAG NORTH = %.2lf\n", query,
			declination);

	sprintf(query, "%sDENSITY CALCULATED = %s\n", query, densityPtr);
	sprintf(query, "%sSUSCEPTIBILITY CALCULATED = %s\n", query, susPtr);
	sprintf(query, "%sREMANENCE CALCULATED = %s\n", query, remPtr);
	sprintf(query, "%sANISOTROPY CALCULATED = %s\n", query, aniPtr);

	sprintf(query, "%sINDEXED DATA FORMAT = %s\n", query, indexedPtr);
	if (indexCalc) {
		sprintf(query, "%sNUM ROCK TYPES = %d\n", query, numProps);
		for (i = 0; i < numProps; i++) {
			sprintf(query, "%sROCK DEFINITION %s = %d\n", query,
					layerProps[i]->unitName, i + 1);
			if (densityCalc || Random)
				sprintf(query, "%s\tDensity = %lf\n", query,
						layerProps[i]->density);

			if (susCalc || Random)
				sprintf(query, "%s\tSus = %lf\n", query, layerProps[i]->sus_X);

			if (remCalc) {
				sprintf(query, "%s\tDeclination = %lf\n", query,
						layerProps[i]->angleWithNorth);
				sprintf(query, "%s\tAzimuth = %lf\n", query,
						layerProps[i]->inclination);
				sprintf(query, "%s\tStrength = %lf\n", query,
						layerProps[i]->strength);
			}

			if (aniCalc) {
				sprintf(query, "%s\tDip = %lf\n", query,
						layerProps[i]->sus_dip);
				sprintf(query, "%s\tDip Direction = %lf\n", query,
						layerProps[i]->sus_dipDirection);
				sprintf(query, "%s\tPitch = %lf\n", query,
						layerProps[i]->sus_pitch);
				sprintf(query, "%s\tAxis 1 = %lf\n", query,
						layerProps[i]->sus_X);
				sprintf(query, "%s\tAxis 2 = %lf\n", query,
						layerProps[i]->sus_Y);
				sprintf(query, "%s\tAxis 3 = %lf\n", query,
						layerProps[i]->sus_Z);
			}
		}

	}

	sprintf(query, "%s','", query);

	//printf("%s\n",query);

	DEBUG(printf("\n FINISHED");)

	//free(query);

	return (TRUE);
}

int
#if XVT_CC_PROTO
upload3DIregBlock(char *filename, char ***data, int numLayers,
		int **layerDimensions, int dataSize, char *query)
#else
upload3DIregBlock (filename, data, numLayers, layerDimensions, dataSize,query)
char *filename;
char ***data;
int numLayers, **layerDimensions;
int dataSize;
char *query;
#endif
{
	FILE *fo;
	int x, y, z;
	int boring = 1,dataIncrement;
	short ***transposed, first;
	short **layer;
	char *dataPtr;

	if (!data || !numLayers || !layerDimensions || !dataSize)
		return (FALSE);

	transposed = (short***) create3DArray(numLayers+1, layerDimensions[0][0]+1,
			layerDimensions[0][1]+1, dataSize);

       //fo=fopen("debug.txt","w");
	   for (z = 0;z<numLayers;z++)
	   {
		   dataIncrement = dataSize;

		   layer=data[z];             /* Write out the array */
		   for (y = 0; y < layerDimensions[0][1]; y++)
		   {                  /* point to a single line of data */
		      dataPtr = (char *) layer[y];

		      for (x = 0; x < layerDimensions[0][0]; x++, dataPtr+=dataIncrement)
		      {

		    	  //fprintf(fo,"%d %d %d %d\n",200-x,y,z,*((short *) dataPtr));
		    	  //fflush(fo);
		    	  transposed[199-x][y][199-z]= *((short *) dataPtr);

		      }
		   }
	   }
       //fclose(fo);


	for (z = 0;z<numLayers;z++) {
		upload2DBlock(fo, transposed[z], layerDimensions[z][1],
				layerDimensions[z][0], dataSize, query);
		//printf("%d\n",z);
	}

	//strcat(query,  data);
	strcat(query, "')");

	//printf("%s\n",query);

	insert_query(query, 1);

	free(query);
	free(transposed);

	return (TRUE);
}

int
#if NOD_FN_PROTOTYPE
upload2DBlock (FILE *fo, char **data,
		int nx, int ny, int dataSize, char *query)
#else
upload2DBlock(fo, data, nx, ny, dataSize, query)
	FILE *fo;char **data;int nx, ny, dataSize;char *query;
#endif
{
	int x, y, dataIncrement;
	char *dataPtr;
	char local_str[405];

	if (!data || !dataSize)
		return (FALSE);

	/* the num of char to move for one element */

	/* Write out the array */

	for (x = 0; x < nx; x++) {
		memcpy(local_str, (char*) data[x], dataSize * ny);

		local_str[(dataSize * ny) + 1] = '\0';

		strcat(query, local_str);

	}

	return (TRUE);
}

void
#if XVT_CC_PROTO
CSIROUploadMap(int xmax, int ymax, int xsize, double *master, FILE *fname,
		int zmax, double airgap, BLOCK_VIEW_OPTIONS *viewOptions,
		GEOPHYSICS_OPTIONS *options, char *fileid)
#else
CSIROUploadMap(xmax, ymax, xsize, master, fname, zmax, airgap, viewOptions, options, fileid)
int xmax, ymax, xsize;
double *master;
FILE *fname;
int zmax;
double airgap;
BLOCK_VIEW_OPTIONS *viewOptions;
GEOPHYSICS_OPTIONS *options;
char *fileid;
#endif
{
	register int i, j, xy;
	double unitScale = 1.0000;
	int calcRangeInCubes;
	time_t longTime;
	struct tm *today;
	FILE_SPEC fileSpec;
	char *query, query2[100];

	query = (char*) calloc(xmax * ymax * 10, 1);
	query[0] = '\0';
	if (strstr(fileid, "333"))
		sprintf(query,
				"INSERT INTO public.magnetics (uid, header, mag) VALUES ('%s','333\n",
				time_stamp);
	else
		sprintf(query,
				"INSERT INTO public.gravity (uid, header, grv) VALUES ('%s','444\n",
				time_stamp);

	time(&longTime); /* current Time */
	today = localtime(&longTime);

	calcRangeInCubes = (int) 0;

	if (strstr(fileid, "333")) /* mag file */
	{
		unitScale = 1.00000;
	} else /* gravity */
	{
		unitScale = 1.0;
	}
	/* Make sure file contains correct altitude */
	if (options->calculationAltitude == AIRBORNE) {
		if (options->altitude > 1.0)
			airgap = options->altitude;
		else
			airgap = 1.0;
	} else
		airgap = 0.0;

	//sprintf(query, "%s%s\n", query, fileid);

	//getCurrentFileName(&fileSpec);
	sprintf(query, "%s#HISTORY FILE: %s\n", query, time_stamp);
	sprintf(query, "%s%s%s", query, ANOM_DATETIME_TAG, asctime(today)); /* asctime has a CR in it */

	sprintf(query, "%s%d\t%d\t%d\t%d\n", query, calcRangeInCubes, xmax + 1,
			ymax + 1, zmax);
	sprintf(query, "%s%f\t%f\t%f\n", query, (float) options->inclination,
			(float) options->declination, (float) options->intensity);
	sprintf(query, "%s%f\t%f\t%f\n", query,
			(float) viewOptions->originX + projectOptions.easting,
			(float) viewOptions->originY + projectOptions.northing,
			(float) viewOptions->originZ - viewOptions->lengthZ);
	sprintf(query, "%s%f\t%f\t%f\n", query,
			(float) viewOptions->originX + viewOptions->lengthX
					+ projectOptions.easting,
			(float) viewOptions->originY + viewOptions->lengthY
					+ projectOptions.northing, (float) viewOptions->originZ);
	sprintf(query, "%s%f\t%f\n", query, (float) viewOptions->geophysicsCubeSize,
			airgap);

	sprintf(query, "%s','", query);

	for (j = 0; j < ymax; j++) /* = 1 !! */
	{
		xy = (j) * xsize; /* - 1 !! */
		for (i = 0; i < xmax; i++) /* = 1 !! */
		{
			sprintf(query2, "%+6.2f ", (float) (master[xy++] * unitScale));
			strcat(query, query2);
		}
		//strcat(query, "\n");
	}

	strcat(query, "')");

	//printf("%s\n", query);

	insert_query(query, 1);

	free(query);

}


/*   SQL query needed to create empty POSTGRES database to store models
     uncomment DROP statements to replace existing database

--DROP TABLE public.magnetics;
--DROP TABLE public.history;
--DROP TABLE public.gravity;
--DROP TABLE public.block;
--DROP TABLE public.universe;


-- Table: public.universe


CREATE TABLE public.universe
(
    id  SERIAL,
    uid text,
    events text,
    comment text COLLATE pg_catalog."default",
    created date,
    CONSTRAINT universe_pkey PRIMARY KEY (uid)
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE public.universe
    OWNER to postgres;

--- Table: public.block


CREATE TABLE public.block
(
    id SERIAL ,
    uid text,
    g00 text COLLATE pg_catalog."default",
    g12 text COLLATE pg_catalog."default",
    CONSTRAINT block_pkey PRIMARY KEY (id),
    CONSTRAINT uid_f FOREIGN KEY (uid)
        REFERENCES public.universe (uid) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE public.block
    OWNER to postgres;


-- Table: public.gravity


CREATE TABLE public.gravity
(
    id SERIAL ,
    uid text,
    header text COLLATE pg_catalog."default",
    grv text COLLATE pg_catalog."default",
    CONSTRAINT gravity_pkey PRIMARY KEY (id),
    CONSTRAINT uid_f FOREIGN KEY (uid)
        REFERENCES public.universe (uid) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE public.gravity
    OWNER to postgres;


-- Table: public.magnetics


CREATE TABLE public.magnetics
(
    id SERIAL,
    uid text,
    header text COLLATE pg_catalog."default",
    mag text COLLATE pg_catalog."default",
    CONSTRAINT magnetics_pkey PRIMARY KEY (id),
    CONSTRAINT uid_f FOREIGN KEY (uid)
        REFERENCES public.universe (uid) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE public.magnetics
    OWNER to postgres;



-- Table: public.history


CREATE TABLE public.history
(
    id SERIAL ,
    uid text,
    his text COLLATE pg_catalog."default",
    CONSTRAINT history_pkey PRIMARY KEY (id),
    CONSTRAINT uid_f FOREIGN KEY (uid)
        REFERENCES public.universe (uid) MATCH SIMPLE
        ON UPDATE NO ACTION
        ON DELETE NO ACTION
)
WITH (
    OIDS = FALSE
)
TABLESPACE pg_default;

ALTER TABLE public.history
    OWNER to postgres;



*/

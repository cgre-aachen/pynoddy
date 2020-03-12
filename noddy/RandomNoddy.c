#include "stdio.h"
#include "stdlib.h"
#include "noddy.h"
#include "xvt.h"
#include "nodStruc.h"
#include "titles.h"
#include "time.h"
#include "string.h"
#include <sys/time.h>
#include <stdint.h>
#include "xoroshiro128plus.h"
#include <libpq-fe.h>
#include "petrophysics.h"
#include "petrophy_defs.h"


#define DEBUG(X)    
#define DEBUG1(X)   

static int loadRandomHistory(int);

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

extern int batchExecution;
extern COLOR backgroundColor;
extern WINDOW_INFO batchWindowInfo;
void copyUndoAdditions(OBJECT*, int);
int exit_nicely();

#define VERSION_TOLERANCE 0.01                
typedef unsigned char String20[21];
extern WINDOW historyWindow;

xrshr128p_state_t *state;
int Random = TRUE;
int unitNumber = 1;
int DataBase = TRUE;
char time_stamp[100];
extern PGconn *conn;
extern int rocktypes[5];

int RandomNoddy(char *output) {
	const char *conninfo;
	PGconn *conn;
	struct timeval start;
	char query[100], q2[1000];
	PGresult *res;

	gettimeofday(&start, NULL);
	sprintf(time_stamp, "%ld", start.tv_sec * 1000000 + start.tv_usec);

	BLOCK_VIEW_OPTIONS *viewOptions = NULL;
	GEOPHYSICS_OPTIONS *geophOptions = NULL;

	initVariables();
	//printf("batchWindowInfo %ld\n",query, batchWindowInfo);
	batchExecution = TRUE;

	readRandomHist();

	if (!geophOptions)
		geophOptions = &geophysicsOptions;
	if (!viewOptions)
		viewOptions = getViewOptions();

	if (DataBase == 1) {
		upload_universe();
	}

	report_Random_status(output);

	doGeophysics(BLOCK_AND_ANOM, viewOptions, geophOptions, output, output,
			NULL, 0, NULL, NULL, NULL);

}

int readRandomHist() {
	int numEvents = 5; // number of random events, including base STRATIGRAPHY and first TILT

	loadRandomHistory(numEvents);

	loadRandomBlockOpts();
	loadRandomGeoOpts();
	loadRandomGpsOpts();
	loadRandom3dOpts();
	loadRandomProjOpts();
	load_Random_window_positions();
	return (TRUE);
}

ReportRandomIcons(FILE *out) {
	int i;
	WINDOW win;
	register int numEvents, event;

	win = getEventDrawingWindow();
	numEvents = totalRandomObjects(win);

	fprintf(out, "#Icon Positions\n");
	fprintf(out, "	Num Icons	= %d\n", numEvents);

	for (i = 1; i < numEvents + 1; i++) {
		fprintf(out, "	Row	= 1\n");
		fprintf(out, "	Column	= %d\n", i);
		fprintf(out, "	X Position	= %d\n", i + 1);
		fprintf(out, "	Y Position	= 1\n");
	}

	fprintf(out, "	Floating Menu Rows	= 1\n");
	fprintf(out, "	Floating Menu Cols	= 14\n");
	fprintf(out, "End of Status Report\n");

}

static int loadRandomHistory(numEvents)
	int numEvents; {
	static char *eventTypes[] = { "STRATIGRAPHY", "FOLD", "FAULT",
			"UNCONFORMITY", "SHEAR_ZONE", "DYKE", "PLUG", "STRAIN", "TILT",
			"FOLIATION", "LINEATION", "IMPORT", "STOP", "GENERIC", "" };
	WINDOW win = (WINDOW) getEventDrawingWindow();
	WINDOW_INFO *wip;
	OBJECT *p, *head = NULL, *last;
	int numEventsInFile, event;
	int type, type2, count;
	char eventType[100], text[OBJECT_TEXT_LENGTH + 10];
	int baseHeight = 0, baseWidth = 0, baseColumn = 0;
	struct timeval start;

	gettimeofday(&start, NULL);

	//printf("%ld %ld\n",query,start.tv_sec,start.tv_usec);
	xrshr128p_init((uint64_t) (start.tv_sec * 1000000 + start.tv_usec), &state);

	// for(event=0;event<10;event++)  printf("%d %lf\n",query,event,xrshr128p_next_double(&state)); exit(1);

	//wip = (WINDOW_INFO *) get_win_info(win);
	wip = (WINDOW_INFO*) &batchWindowInfo;
	historyWindow = win; /* store as a global */

	numEventsInFile = numEvents;
	{
		for (event = 0; event < numEventsInFile; event++) {
			if (event == 0)
				type2 = STRATIGRAPHY;
			else if (event == 1)
				type2 = TILT;
			else {
				type = (int) (xrshr128p_next(&state) % 10) + 1;

				if (type == 1 || type == 2)
					type2 = FOLD;
				else if (type == 3 || type == 4)
					type2 = FAULT;
				else if (type == 5 || type == 6)
					type2 = SHEAR_ZONE;
				else if (type == 7)
					type2 = UNCONFORMITY;
				else if (type == 8)
					type2 = DYKE;
				else if (type == 9)
					type2 = PLUG;
				else
					type2 = TILT;
			}

			//printf("%d %d %d\n",query,numEvents,event, type);
			/* malloc the basic structure */
			if (!(p = (OBJECT*) xvt_mem_zalloc(sizeof(OBJECT)))) {
				fprintf(stderr, "Not Enough Memory to load Events");
				return (FALSE);
			}
			p->shape = type2;
			p->drawEvent = TRUE;
			p->selected = TRUE;
			p->newObject = FALSE;
			p->next = NULL;

			switch (p->shape) {
			case STRATIGRAPHY: {
				STRATIGRAPHY_OPTIONS *options;

				/* we must always read the stratigraphy even
				 ** if we already have one so we can get past it */
				options = (STRATIGRAPHY_OPTIONS*) xvt_mem_zalloc(
						sizeof(STRATIGRAPHY_OPTIONS));
				if (!options) {
					fprintf(stderr,
							"Not Enough Memory to Load Stratigraphy Event");
					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomStratigraphy(options);

				wip->head = p;
				break;
			}
			case FOLD: {
				FOLD_OPTIONS *options;

				options = (FOLD_OPTIONS*) xvt_mem_zalloc(sizeof(FOLD_OPTIONS));
				if (!options) {
					fprintf(stderr, "Not Enough Memory to Load Fold Event");
					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomFold(options);
				// printf("ranfold waves = %lf\n",query,options->wavelength);
				// printf("ranfold shape = %lf\n",query,p->shape);
				break;
			}
			case FAULT: {
				FAULT_OPTIONS *options;

				options = (FAULT_OPTIONS*) xvt_mem_zalloc(
						sizeof(FAULT_OPTIONS));
				if (!options) {
					fprintf(stderr, "Not Enough Memory to Load Fault Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomFault(options);
				break;
			}
			case UNCONFORMITY: {
				UNCONFORMITY_OPTIONS *options;

				options = (UNCONFORMITY_OPTIONS*) xvt_mem_zalloc(
						sizeof(UNCONFORMITY_OPTIONS));
				if (!options) {

					fprintf(stderr,
							"Not Enough Memory to Load Unconformity Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomUnconformity(options);
				break;
			}
			case SHEAR_ZONE: {
				SHEAR_OPTIONS *options;

				options = (SHEAR_OPTIONS*) xvt_mem_zalloc(
						sizeof(SHEAR_OPTIONS));
				if (!options) {

					fprintf(stderr,
							"Not Enough Memory to Load Shear Zone Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomShearZone(options);
				break;
			}
			case DYKE: {
				DYKE_OPTIONS *options;

				options = (DYKE_OPTIONS*) xvt_mem_zalloc(sizeof(DYKE_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Dyke Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomDyke(options);
				break;
			}
			case PLUG: {
				PLUG_OPTIONS *options;
				// printf("ppppllluuugggs\n");

				options = (PLUG_OPTIONS*) xvt_mem_zalloc(sizeof(PLUG_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Plug Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomPlug(options);
				break;
			}
			case TILT: {
				TILT_OPTIONS *options;
				//  printf("tiiiiiillltttss\n");
				options = (TILT_OPTIONS*) xvt_mem_zalloc(sizeof(TILT_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Tilt Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomTilt(options);
				break;
			}
			case STRAIN: {
				STRAIN_OPTIONS *options;

				options = (STRAIN_OPTIONS*) xvt_mem_zalloc(
						sizeof(STRAIN_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Strain Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomStrain(options);
				break;
			}
			case LINEATION: {
				LINEATION_OPTIONS *options;

				options = (LINEATION_OPTIONS*) xvt_mem_zalloc(
						sizeof(LINEATION_OPTIONS));
				if (!options) {

					fprintf(stderr,
							"Not Enough Memory to Load Lineation Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomLineation(options);
				break;
			}
			case FOLIATION: {
				FOLIATION_OPTIONS *options;

				options = (FOLIATION_OPTIONS*) xvt_mem_zalloc(
						sizeof(FOLIATION_OPTIONS));
				if (!options) {

					fprintf(stderr,
							"Not Enough Memory to Load Foliation Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				loadRandomFoliation(options);
				break;
			}
			case IMPORT: {
				IMPORT_OPTIONS *options;

				options = (IMPORT_OPTIONS*) xvt_mem_zalloc(
						sizeof(IMPORT_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Import Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				//loadRandomImport ( options);
				break;
			}
			case GENERIC: {
				GENERIC_OPTIONS *options;

				options = (GENERIC_OPTIONS*) xvt_mem_zalloc(
						sizeof(GENERIC_OPTIONS));
				if (!options) {

					fprintf(stderr, "Not Enough Memory to Load Generic Event");

					return (FALSE);
				}
				p->options = (char*) options;
				setDefaultOptions(p);
				//loadRandomGeneric ( options);
				break;
			}
			case STOP: {
				p->options = NULL;
				break;
			}
			default:
				break;
			}
			//strcpy (text, p->shape);  /* so an empty string does not get the last one */
			//loadit(in, "%s",(void *) text);
			if (p) {
				sprintf(p->text, "Event_%02d", event);
				p->row = 1;
				p->column = numEvents + event + 1;
				p->bound.top = baseHeight;
				p->bound.bottom = p->bound.top + ICON_SIZE + 2;
				p->bound.left = event * GRID_WIDTH + baseWidth;
				p->bound.right = p->bound.left + ICON_SIZE + 2;
				if (head) /* keep a list of all the events read in */
				{
					last->next = p;
					last = p;
				} else {
					head = p;
					last = p;
				}
				copyUndoAdditions(p, FALSE);

			}

		}

		/* add the events we have loaded onto the events alread there */
		/*     DEBUG(printf("\n **** PUTING IT ON THE EVENTS WE HAVE");)
		 if (numEvents)
		 {
		 last = (OBJECT *) nthObject (win, numEvents-1);
		 last->next = head;
		 }
		 else
		 {
		 wip = (WINDOW_INFO *) get_win_info(win);
		 wip->head = head;
		 }*/
		count = 0;
		for (p = wip->head, count = 0; p != NULL; p = p->next) {
			count++;
		}

		// printf("loadrandhist total object = %d\n",query,count);
	}

	return (TRUE);
}

int loadRandomBlockOpts()

{
	int numViews, currentView, view;
	double originX, originY, originZ;
	double lengthX, lengthY, lengthZ;
	double geolCube, geopCube;
	char name[OBJECT_TEXT_LENGTH];
	BLOCK_VIEW_OPTIONS *viewOptions = NULL, *defaultView = NULL;
	char vname[100];
	numViews = 1;
	currentView = 1;

	sprintf(vname, "View1");
	viewOptions = newViewOptions(vname, 0, 0, 4000, 4000, 4000, 4000, 20, 20);

	/*blockViewOptions->originX = 0;
	 blockViewOptions->originY = 0;
	 blockViewOptions->originZ = 4000;
	 blockViewOptions->lengthX = 4000;
	 blockViewOptions->lengthY = 4000;
	 blockViewOptions->lengthZ = 4000;
	 blockViewOptions->geologyCubeSize = 20;
	 blockViewOptions->geophysicsCubeSize = 20;
	 sprintf((void *)blockViewOptions->viewName,"Default");*/
	/*viewOptions = blockViewOptions;
	 defaultView = viewOptions;*/

	return (TRUE);
}

int loadRandomGeoOpts() {
	char temp[255];
	int jj, kk, tempInt;
	double originX, originY, originZ, tempDouble;

	iscale = 10;
	geologyOptions.welllogDeclination = 0;
	geologyOptions.welllogDepth = 400;
	geologyOptions.welllogAngle = 0;

	geologyOptions.boreholeX = 0;
	geologyOptions.boreholeY = 0;
	geologyOptions.boreholeZ = 4000;
	geologyOptions.boreholeDecl = 0;
	geologyOptions.boreholeDip = 0;
	geologyOptions.boreholeLength = 4000;

	geologyOptions.sectionX = 0;
	geologyOptions.sectionY = 0;
	geologyOptions.sectionZ = 4000;
	geologyOptions.sectionDecl = 90;
	geologyOptions.sectionLength = 4000;
	geologyOptions.sectionHeight = 4000;

	geologyOptions.useTopography = FALSE;

	geologyOptions.topoScaleFactor = 1;
	geologyOptions.topoOffsetFactor = 0;
	geologyOptions.topoLowContour = 0;
	geologyOptions.topoContourInterval = 0;

	geologyOptions.calculateChairDiagram = FALSE;

	geologyOptions.chair_X = 0;
	geologyOptions.chair_Y = 0;
	geologyOptions.chair_Z = 0;

	return (TRUE);
}

int loadRandomGpsOpts() {
	char temp[255];
	double dudDouble, cubeSize;
	double lowerSouthWest_X, lowerSouthWest_Y, lowerSouthWest_Z;
	double upperNorthEast_X, upperNorthEast_Y, upperNorthEast_Z;

	geophysicsOptions.calculationRange = 12000.0;

	geophysicsOptions.declination = 0.0;

	geophysicsOptions.inclination = 90.0;
	geophysicsOptions.intensity = 70000.0;

	geophysicsOptions.fieldType = FIXED_FIELD;

	geophysicsOptions.xPos = 0.0;
	geophysicsOptions.yPos = 0.0;
	geophysicsOptions.zPos = 5000.0;

	geophysicsOptions.inclinationOri = 0.0;
	geophysicsOptions.inclinationChange = 0.0;
	geophysicsOptions.intensityOri = 0.0;
	geophysicsOptions.intensityChange = 0.0;
	geophysicsOptions.declinationOri = 0.0;
	geophysicsOptions.declinationChange = 0.0;

	geophysicsOptions.altitude = 100.0;

	geophysicsOptions.calculationAltitude = AIRBORNE;
	//geophysicsOptions.calculationAltitude = SURFACE;

	geophysicsOptions.calculationMethod = SPECTRAL;

	geophysicsOptions.spectralPaddingType = REFLECTION_PADDING;

	geophysicsOptions.spectralFence = 50;
	geophysicsOptions.spectralPercent = 100;

	geophysicsOptions.constantBoxDepth = 0.0;
	geophysicsOptions.cleverBoxRatio = 1.0;

	geophysicsOptions.deformableRemanence = FALSE;

	geophysicsOptions.deformableAnisotropy = FALSE;

	geophysicsOptions.magneticVectorComponents = FALSE;

	geophysicsOptions.projectVectorsOntoField = TRUE;

	geophysicsOptions.padWithRealGeology = TRUE;

	geophysicsOptions.drapedSurvey = FALSE;

	return (TRUE);
}

int loadRandom3dOpts()

{
	int num;

	threedViewOptions.declination = 150.0;
	threedViewOptions.azimuth = 30.0;
	threedViewOptions.scale = 1.0;

	threedViewOptions.offset_X = 1.0;
	threedViewOptions.offset_Y = 1.0;
	threedViewOptions.offset_Z = 1.0;

	threedViewOptions.fillType = 3;
	return (TRUE);
}

int loadRandomProjOpts() {
	char temp[255];
	int value;
	int red, green, blue;

	projectOptions.susceptibilityUnits = SI;

	projectOptions.geophysicalCalc = 2;

	projectOptions.calculationType = LOCAL_JOB;

	projectOptions.lengthScale = 0;

	projectOptions.printScalingFactor = 1.0;
	projectOptions.imageScalingFactor = 10.0;

	projectOptions.newWindowEachStage = FALSE;

	red = 255;
	green = 255;
	blue = 255;

	backgroundColor = XVT_MAKE_COLOR(red, green, blue);

	sprintf(projectOptions.internetAddress, "");
	sprintf(projectOptions.accountName, "");
	sprintf(projectOptions.noddyPath, "");

	sprintf(projectOptions.helpPath, "");

	projectOptions.movieFramesPerEvent = 3;
	projectOptions.moviePlaySpeed = 10.0;
	projectOptions.movieType = 0;

	projectOptions.gravityDisplay.clippingType = RELATIVE_CLIPPING;
	projectOptions.gravityDisplay.minClip = 0.0;
	projectOptions.gravityDisplay.maxClip = 100.0;
	projectOptions.gravityDisplay.display = PSEUDO_COLOR;
	projectOptions.gravityDisplay.numContours = 25;

	projectOptions.magneticsDisplay.clippingType == RELATIVE_CLIPPING;
	projectOptions.magneticsDisplay.minClip = 0.0;
	projectOptions.magneticsDisplay.maxClip = 100.0;
	projectOptions.magneticsDisplay.display = PSEUDO_COLOR;
	projectOptions.magneticsDisplay.numContours = 25;

	projectOptions.easting = 0.0;
	projectOptions.northing = 0.0;

	return (TRUE);
}

int load_Random_window_positions() {
	register int winNum;
	int intVal, numWindows;
	RCT position;
	WINDOW win;
	PNT point;

	numWindows = 16;
	for (winNum = 0; winNum < numWindows; winNum++) {
		sprintf(winPositionOptions.winPositions[winNum].name, "Window_02%d",
				winNum);
		winPositionOptions.winPositions[winNum].x = 60;
		winPositionOptions.winPositions[winNum].y = 60;
		winPositionOptions.winPositions[winNum].width = 500;
		winPositionOptions.winPositions[winNum].height = 300;

	}

	if (!batchExecution) {
		if (getWindowStartPosition(HISTORY_TITLE, &position.left, &position.top,
				&position.right, &position.bottom, NULL, NULL)) {
			if (win = (WINDOW) getEventDrawingWindow())
				xvt_vobj_move (win, &position);
		}

	}

	return (TRUE);
}

int load_Random_icon_positions(in, version, baseWidth, baseHeight, baseColumn,
		lastEvent)
	FILE *in;double version;int baseWidth, baseHeight;int baseColumn, lastEvent; {
	WINDOW win;
	OBJECT *p;
	int numEvents, event;
	int positionX, positionY, row, column;
	char lineBuf[100];
	BOOLEAN iconsPresent = FALSE;

	return (FALSE);

	win = (WINDOW) getEventDrawingWindow();

	while (!iconsPresent && fgets(lineBuf, 100, in)) {
		if (strcmp(lineBuf, "#Icon Positions\n") == 0)
			iconsPresent = TRUE;
	}

	if (!iconsPresent)
		return (FALSE);

	loadit(in, "%d", (void*) &numEvents);

	if (lastEvent) /* first was the strat so miss it */
	{ /* when we are appending to a hist */
		loadit(in, "%d", (void*) &row);
		loadit(in, "%d", (void*) &column);
		loadit(in, "%d", (void*) &positionX);
		loadit(in, "%d", (void*) &positionY);
		numEvents--;
	}

	for (event = 0; event < numEvents; event++) {
		loadit(in, "%d", (void*) &row);
		loadit(in, "%d", (void*) &column);
		loadit(in, "%d", (void*) &positionX);
		positionX--;
		loadit(in, "%d", (void*) &positionY);
		positionY--;
		p = (OBJECT*) nthObject(win, lastEvent + event);
		if (p) {
			p->row = row;
			p->column = column + baseColumn;
			p->bound.top = positionY * GRID_HEIGHT + baseHeight;
			p->bound.bottom = p->bound.top + ICON_SIZE + 2;
			p->bound.left = positionX * GRID_WIDTH + baseWidth;
			p->bound.right = p->bound.left + ICON_SIZE + 2;
			/* DELETECANDIDATE*/
			//gridCorrectRect (&(p->bound), &(p->bound));
		}
	}

	if (version > 3.4) /* Load menu shape as well */
	{
		int numItems;
		loadit(in, "%d", (void*) &row);
		loadit(in, "%d", (void*) &column);
		/* DELETECANDIDATE*/

		// numItems = (int) getFloatingMenuShape (NULL_WIN, NULL, NULL);
		// if (row*column >= numItems)  /* make sure the icons can fit - otherwise leave it alone */
		//    setFloatingMenuShape (NULL_WIN, row, column);
		/* DELETECANDIDATE*/

	}

	return (TRUE);
}

int loadRandomFold(options)
	FOLD_OPTIONS *options; {
	int i;
	char temp[225];

	// printf("FOLLLLDDDSSS\n");

	options->type = SINE_FOLD;
	options->singleFold = FALSE;

	options->positionX = 4000.0 * xrshr128p_next_double(&state);
	options->positionY = 0.0;
	options->positionZ = 0.0;

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state);
	options->axisPitch = 90.0 * xrshr128p_next_double(&state);

	options->wavelength = 200.0 + 9800.0 * xrshr128p_next_double(&state);
	options->amplitude = 200 + 4800.0 * xrshr128p_next_double(&state);
	options->cycle = 500 + 9500.0 * xrshr128p_next_double(&state);

	convrt(options->dip, options->dipDirection, options->axisPitch, TRUE);
	rotset(options->dip, options->dipDirection, options->axisPitch,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomDyke(options)
	DYKE_OPTIONS *options; {
	double dipDirection;
	char temp[100], strVal[100];

	// printf("DYKESSSSSS\n");

	options->type = DILATION_DYKE;

	options->mergeEvents = 0;

	options->positionX = 4000.0 * xrshr128p_next_double(&state);
	options->positionY = 4000.0 * xrshr128p_next_double(&state);
	options->positionZ = 4000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state);
	options->slipPitch = 90.0 * xrshr128p_next_double(&state);

	options->slipLength = 0;
	options->width = 100 + 300.0 * xrshr128p_next_double(&state);

	options->alterationZones = NONE_ALTERATION;

	loadRandomProperties(-2, &(options->properties));

	if (options->dip == 90.0)
		dipDirection = options->dipDirection + 180.0;
	else
		dipDirection = options->dipDirection;

	convrt(options->dip, dipDirection, 90.0, TRUE);
	rotset(options->dip, dipDirection, 90.0, options->rotationMatrix.forward,
			options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomFault(options)
	FAULT_OPTIONS *options; {
	int i;
	double pitch, dip;
	char temp[100], strVal[100];

	// printf("FAAAAULLTSSS\n");

	options->type = FAULT_EVENT;

	options->geometry = TRANSLATION;

	options->movement = BOTH;

	options->positionX = 2000.0 + 2000.0 * xrshr128p_next_double(&state);
	options->positionY = 2000.0 + 2000.0 * xrshr128p_next_double(&state);
	options->positionZ = 2000.0 + 2000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * sqrt(xrshr128p_next_double(&state));
	options->pitch = 90.0 * xrshr128p_next_double(&state);

	options->slip = 2000.0 * xrshr128p_next_double(&state);
	options->rotation = 0.0;

	options->amplitude = 100.0;
	options->radius = 100.0;
	options->xAxis = 0.0;
	options->yAxis = 0.0;
	options->zAxis = 0.0;
	options->cylindricalIndex = 0.0;
	options->profilePitch = 0.0;

	loadRandomColor(&(options->color));

	if (options->geometry == ROTATION)
		pitch = 0.0;
	else
		pitch = options->pitch;

	if (options->dip == 90.0)
		dip = 89.999;
	else
		dip = options->dip;

	convrt(dip, options->dipDirection, pitch, TRUE);
	rotset(dip, options->dipDirection, pitch, options->rotationMatrix.forward,
			options->rotationMatrix.reverse);
	plane(&options->plane.a, &options->plane.b, &options->plane.c,
			&options->plane.d, options->positionX, options->positionY,
			options->positionZ);
	getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);
	return (TRUE);
}

int loadRandomShearZone(options)
	SHEAR_OPTIONS *options; {
	int i;
	double dip, pitch;
	char temp[100], strVal[100];

	// printf("SHEARZONESSS\n");

	if (xrshr128p_next(&state) % 2 > 0)
		options->type = SHEAR_ZONE_EVENT;
	else
		options->type = KINK_EVENT;

	options->geometry = TRANSLATION;

	options->movement = BOTH;

	options->positionX = 2000.0 + 2000.0 * xrshr128p_next_double(&state);
	options->positionY = 2000.0 + 2000.0 * xrshr128p_next_double(&state);
	options->positionZ = 2000.0 + 2000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state);
	options->pitch = 90.0 * xrshr128p_next_double(&state);

	options->slip = 2000.0 * xrshr128p_next_double(&state);
	options->rotation = 0.0;

	options->amplitude = 100.0;
	options->width = 100.0 + 1900.0 * xrshr128p_next_double(&state);
	options->radius = 100.0;
	options->xAxis = 0.0;
	options->yAxis = 0.0;
	options->zAxis = 0.0;
	options->cylindricalIndex = 0.0;
	options->profilePitch = 0.0;

	loadRandomColor(&(options->color));

	if (options->geometry == ROTATION)
		pitch = 0.0;
	else
		pitch = options->pitch;

	if (options->dip == 90.0)
		dip = 89.999;
	else
		dip = options->dip;

	convrt(dip, options->dipDirection, pitch, TRUE);
	rotset(dip, options->dipDirection, pitch, options->rotationMatrix.forward,
			options->rotationMatrix.reverse);
	plane(&options->plane.a, &options->plane.b, &options->plane.c,
			&options->plane.d, options->positionX, options->positionY,
			options->positionZ);
	getAlphaBetaGamma(&options->alpha, &options->beta, &options->gamma);
	return (TRUE);
}

int loadRandomLineation(options)
	LINEATION_OPTIONS *options; {
	options->plungeDirection = 0.0;
	options->plunge = 0.0;

	convrt(90.0 - options->plunge, 180.0 + options->plungeDirection, 0.0, TRUE);
	rotset(90.0 - options->plunge, 180.0 + options->plungeDirection, 0.0,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomFoliation(options)
	FOLIATION_OPTIONS *options; {
	options->dipDirection = 0.0;
	options->dip = 0.0;

	convrt(options->dip, options->dipDirection, 0.0, TRUE);
	rotset(options->dip, options->dipDirection, 0.0,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomPlug(options)
	PLUG_OPTIONS *options; {
	double pitch;
	char temp[100], strVal[100];
	int plugit;

	//  printf("PLUGGGSSSSSS\n");

	plugit = xrshr128p_next(&state) % 4;
	if (plugit == 0)
		options->type = CYLINDRICAL_PLUG;
	else if (plugit == 1)
		options->type = CONE_PLUG;
	else if (plugit == 2)
		options->type = PARABOLIC_PLUG;
	else
		options->type = ELLIPSOIDAL_PLUG;

	options->mergeEvents = 0;

	options->positionX = 1000.0 + 3000.0 * xrshr128p_next_double(&state);
	options->positionY = 1000.0 + 3000.0 * xrshr128p_next_double(&state);
	options->positionZ = 1000.0 + 3000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state);
	options->axisPitch = 90.0 * xrshr128p_next_double(&state);

	options->radius = 2000.0 * xrshr128p_next_double(&state);
	options->apicalAngle = 10.0 + 80.0 * xrshr128p_next_double(&state);
	options->BValue = 200 + 4800.0 * xrshr128p_next_double(&state);
	options->axisA = 200 + 3800.0 * xrshr128p_next_double(&state);
	options->axisB = 200 + 3800.0 * xrshr128p_next_double(&state);
	options->axisC = 200 + 3800.0 * xrshr128p_next_double(&state);

	if (options->type != ELLIPSOIDAL_PLUG)
		pitch = 0.0;
	else
		pitch = options->axisPitch;

	options->alterationZones = NONE_ALTERATION;

	loadRandomProperties(-1, &(options->properties));

	convrt(options->dip - 90.0, options->dipDirection, pitch, TRUE);
	rotset(options->dip - 90.0, options->dipDirection, pitch,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);
	return (TRUE);
}

int loadRandomTilt(options)
	TILT_OPTIONS *options; {
	int i, j;
	double angle;
	double axis[2][4], mata[4][4];

	//  printf("TILTSSSSSS\n");

	options->positionX = 2000.0;
	options->positionY = 2000.0;
	options->positionZ = 1000.0;

	options->rotation = -90+(90.0 * xrshr128p_next_double(&state));
	options->plungeDirection = 360.0 * xrshr128p_next_double(&state);
	options->plunge = 90.0 * xrshr128p_next_double(&state);

	angle = options->rotation * 0.0174532;
	paxis(options->plunge, options->plungeDirection, axis);
	irotate(mata, axis[1][1], axis[1][2], axis[1][3], angle);

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			options->rotationMatrix[i][j] = mata[i + 1][j + 1];
		}
	}
	return (TRUE);
}

int loadRandomStrain(options)
	STRAIN_OPTIONS *options; {
	int i, j;
	double tensor[4][4], inverseTensor[4][4];

	options->tensor[0][0] = 1.0;
	options->tensor[1][0] = 0.0;
	options->tensor[2][0] = 0.0;
	options->tensor[0][1] = 0.0;
	options->tensor[1][1] = 1.0;
	options->tensor[2][1] = 0.0;
	options->tensor[0][2] = 0.0;
	options->tensor[1][2] = 0.0;
	options->tensor[2][2] = 1.0;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			tensor[j + 1][i + 1] = options->tensor[i][j];
		}
	}
	if (!f01aaf(tensor, inverseTensor)) /* matrix inversion */
		return (FALSE);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			options->inverseTensor[i][j] = inverseTensor[i + 1][j + 1];
		}
	}
	return (TRUE);
}

int loadRandomGeneric(options)
	GENERIC_OPTIONS *options; {
	options->param1 = 0.0;
	options->param2 = 0.0;
	options->param3 = 0.0;
	options->param4 = 0.0;
	options->param5 = 0.0;
	options->param6 = 0.0;
	options->param7 = 0.0;
	options->param8 = 0.0;
	options->param9 = 0.0;
	options->param10 = 0.0;

	return (TRUE);
}

int loadRandomUnconformity(options)
	UNCONFORMITY_OPTIONS *options; {
	char strVal[100];

	// printf("UNCSSSSSSS\n");

	options->positionX = 2000 + 1000.0 * xrshr128p_next_double(&state);
	options->positionY = 2000 + 2000.0 * xrshr128p_next_double(&state);
	options->positionZ = 3000 + 1000.0 * xrshr128p_next_double(&state);

	options->dipDirection = 360.0 * xrshr128p_next_double(&state);
	options->dip = 90.0 * xrshr128p_next_double(&state)
			* xrshr128p_next_double(&state);

	options->alterationZones = NONE_ALTERATION;

	options->surfaceType = FLAT_SURFACE;

	loadRandomStratigraphy(&(options->stratigraphyDetails));

	convrt(options->dip, options->dipDirection, 0.0, TRUE);
	plane(&options->plane.a, &options->plane.b, &options->plane.c,
			&options->plane.d, options->positionX, options->positionY,
			options->positionZ);
	rotset(options->dip, options->dipDirection, 0.0,
			options->rotationMatrix.forward, options->rotationMatrix.reverse);

	return (TRUE);
}

int loadRandomStratigraphy(options)
	STRATIGRAPHY_OPTIONS *options; {
	int i;
	int maxLayers = 5; // maximum number of layers (minimum is 2)

	options->numLayers = (xrshr128p_next(&state) % 5) + 2;
	if (options->properties)
		xvt_mem_free((char* ) options->properties);
	if (!(options->properties = (LAYER_PROPERTIES*) xvt_mem_zalloc(
			options->numLayers * sizeof(LAYER_PROPERTIES))))
		return (FALSE);

	for (i = 0; i < options->numLayers; i++) {
		loadRandomProperties(i, &(options->properties[i]));
	}

	return (TRUE);
}

int loadRandomColor(options)
	NODDY_COLOUR *options; {

	options->red = (unsigned char) xrshr128p_next(&state) % 256;
	options->green = (unsigned char) xrshr128p_next(&state) % 256;
	options->blue = (unsigned char) xrshr128p_next(&state) % 256;

	return (TRUE);
}

int petrophysics(  int litho,  double *density, double *magsus)
{
  double U,V,X,Y;
  int lowmag;

  U=xrshr128p_next_double(&state);
  V=xrshr128p_next_double(&state);

  X=sqrt(-2.0*log(U)) * cos(2.0*3.1415927*V); //where U & V are random numbers between 0 & 1
  Y=sqrt(-2.0*log(U)) * sin(2.0*3.1415927*V); //produces pairs of independaent normally dist random numbers using
  	  	  	  	  	  	  	  	  	  	  	  //Box–Muller transform

  *density=X*(PPHYS_ROCK[litho].density_sd)+PPHYS_ROCK[litho].density_mean;

  if(PPHYS_ROCK[litho].bimodal==1)
  {
	  lowmag = (int) xrshr128p_next(&state) % 2;

	  if(lowmag==0)
		  *magsus=pow(10.0,(Y*PPHYS_ROCK[litho].magsus_sd/2)+(PPHYS_ROCK[litho].magsus_mean-0.75));
	  else
		  *magsus=pow(10.0,(Y*PPHYS_ROCK[litho].magsus_sd/2)+(PPHYS_ROCK[litho].magsus_mean+0.75));
  }
  else
	  *magsus=pow(10.0,Y*(PPHYS_ROCK[litho].magsus_sd)+PPHYS_ROCK[litho].magsus_mean);

   //printf("%d\t%lf\t%lf\t%lf\n",litho,PPHYS_ROCK[litho].density_mean,PPHYS_ROCK[litho].density_sd, *density);

}

int loadRandomProperties(layer, options)
	int layer;LAYER_PROPERTIES *options; {
	double value;
	char label[50];
	static int cum_height = 0;
    double density, magsus;
    static int stratcode;
    int lithocode;

	if(layer==-2) // dyke
	{
		lithocode=(xrshr128p_next(&state) % rocktypes[0]) ;
		petrophysics( lithocode, &density, &magsus );
	}
	else if(layer==-1) //plug
	{
		lithocode=(xrshr128p_next(&state) % rocktypes[1]) +rocktypes[0];
		petrophysics(lithocode, &density, &magsus );
	}
	else //strat
	{
		if(layer==0)
			stratcode=(xrshr128p_next(&state) % 3);
		if(stratcode==0) //Met
		{
			lithocode=(xrshr128p_next(&state) % rocktypes[2])+rocktypes[0]+rocktypes[1];
			petrophysics(lithocode, &density, &magsus );
		}
		else if(stratcode==1) //Volc
		{
			lithocode=(xrshr128p_next(&state) % rocktypes[3])+rocktypes[0]+rocktypes[1]+rocktypes[2];
			petrophysics( lithocode, &density, &magsus );

		}
		else //Sed
		{
			lithocode=(xrshr128p_next(&state) % rocktypes[4])+rocktypes[0]+rocktypes[1]+rocktypes[2]+rocktypes[3];
			petrophysics( lithocode, &density, &magsus );

		}
	}




	sprintf((void*) options->unitName, "%s_%02d", PPHYS_ROCK[lithocode].lithoname, unitNumber++);
	//printf("%s\n",options->unitName);

	if (layer == 0)
		options->height = -31000;
	else {
		options->height = 50
				+ (1950.0 * xrshr128p_next_double(&state));
		cum_height += options->height;
		options->height = cum_height;
	}
	options->applyAlterations = FALSE;

	options->density = density;
	options->anisotropicField = 0;
	options->sus_X = magsus;
	options->sus_Y = options->sus_X;
	options->sus_Z = options->sus_X;
	options->sus_dip = 0;
	options->sus_dipDirection = 0;
	options->sus_pitch = 0;
	options->remanentMagnetization = 0;
	options->inclination = 0;
	options->angleWithNorth = 0;
	options->strength = 0;

	loadRandomColor(&(options->color));

	/* calculate and store the forward and
	 ** reverse rotation matricies for remenance */
	convrt(90.0 - options->inclination, 180.0 + options->angleWithNorth, 0.0,
	TRUE);
	rotset(90.0 - options->inclination, 180.0 + options->angleWithNorth, 0.0,
			options->remRotationMatrix.forward,
			options->remRotationMatrix.reverse);

	convrt(options->sus_dip, options->sus_dipDirection, options->sus_pitch,
	TRUE);
	rotset(options->sus_dip, options->sus_dipDirection, options->sus_pitch,
			options->aniRotationMatrix.forward,
			options->aniRotationMatrix.reverse);
	return (TRUE);
}

/* ======================================================================
 FUNCTION  nthRandomObject

 DESCRIPTION
 get the nth object in the list or NULL if out of bounds
 ====================================================================== */
OBJECT*
#if XVT_CC_PROTO
nthRandomObject(WINDOW win, int itemCount)
#else
nthRandomObject (win, itemCount)
WINDOW win;
int itemCount;
#endif
{
	WINDOW_INFO *wip;
	register OBJECT *p;
	register int count;

	wip = (WINDOW_INFO*) &batchWindowInfo;

	if (!wip)
		return (0);

	count = 0;
	for (p = wip->head; p != NULL; p = p->next, count++) {
		if (count == itemCount)
			break;
	}

	return (p);
}

/* ======================================================================
 FUNCTION  totalRandomObjects

 DESCRIPTION
 get the number of objects in the list
 ====================================================================== */
int
#if XVT_CC_PROTO
totalRandomObjects()
#else
totalRandomObjects ()
#endif
{
	WINDOW_INFO *wip;
	register OBJECT *p;
	register int count;

	wip = (WINDOW_INFO*) &batchWindowInfo;
	//printf("totalRandomObjects batchWindowInfo %ld\n",query, batchWindowInfo);

	if (!wip)
		return (0);

	count = 0;
	for (p = wip->head; p != NULL; p = p->next) {
		count++;
	}

	return (count);
}

int report_Random_hist(out, query)
	FILE *out;char *query; {
	WINDOW win;
	OBJECT *p;
	register int numEvents, event;

	win = getEventDrawingWindow();
	numEvents = totalRandomObjects(win);
	//printf("win numevents %ld %d\n",query,win, numEvents);
	fprintf(out, "\nNo of Events\t= %d\n", numEvents);
	for (event = 0; event < numEvents; event++) {
		p = (OBJECT*) nthObject(win, event);
		// printf("p numevents shape %ld %d %d\n",query,p, event,p->shape);

		DEBUG(printf("\n p->shape = %d", p->shape);)
		if (p) {
			switch (p->shape) {
			case STRATIGRAPHY: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "STRATIGRAPHY");
				reportRandomStratigraphy(out,
						(STRATIGRAPHY_OPTIONS*) p->options);

				break;
			}
			case FOLD: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "FOLD");
				report_folds(out, (FOLD_OPTIONS*) p->options);
				DEBUG(printf("\nfo p->shape = %d", p->shape);)

				break;
			}
			case FAULT: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "FAULT");
				report_faults(out, (FAULT_OPTIONS*) p->options);
				//printf("\nfa p->shape = %d\n",query, p->shape);

				break;
			}
			case UNCONFORMITY: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "UNCONFORMITY");
				report_unconformities(out, (UNCONFORMITY_OPTIONS*) p->options);
				DEBUG(printf("\nuc p->shape = %d", p->shape);)

				break;
			}
			case SHEAR_ZONE: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "SHEAR_ZONE");
				report_shear_zone(out, (SHEAR_OPTIONS*) p->options);
				DEBUG(printf("\nsz p->shape = %d", p->shape);)

				break;
			}
			case DYKE: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "DYKE");
				report_dykes(out, (DYKE_OPTIONS*) p->options);
				DEBUG(printf("\ndyke p->shape = %d\n",query, p->shape);)

				break;
			}
			case PLUG: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "PLUG");
				report_plugs(out, (PLUG_OPTIONS*) p->options);
				DEBUG(printf("\npl p->shape = %d", p->shape);)

				break;
			}
			case STRAIN: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "STRAIN");
				report_strains(out, (STRAIN_OPTIONS*) p->options);
				break;
			}
			case TILT: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "TILT");
				report_tilts(out, (TILT_OPTIONS*) p->options);
				break;
			}
			case FOLIATION: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "FOLIATION");
				report_planes(out, (FOLIATION_OPTIONS*) p->options);
				break;
			}
			case LINEATION: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "LINEATION");
				report_lineations(out, (LINEATION_OPTIONS*) p->options);
				break;
			}
			case IMPORT: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "IMPORT");
				report_import(out, (IMPORT_OPTIONS*) p->options);
				break;
			}
			case GENERIC: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "GENERIC");
				report_generic(out, (GENERIC_OPTIONS*) p->options);
				break;
			}
			case STOP: {
				fprintf(out, "Event #%d\t= %s\n", (event + 1), "STOP");
				break;
			}
			}
			fprintf(out, "\tName\t= %s\n", p->text);
		}
	}
	return (TRUE);
}

int report_Random_status(filename)
	char *filename; {
	FILE *out;
	int error = TRUE;
	char *query;
	PGconn *conn;

	if (DataBase == 1) {
		query = (char*) calloc(16000001, sizeof(char));
		sprintf(query, "INSERT INTO public.history (uid, his) VALUES ('%s','",
				time_stamp);
	} else
		out = (FILE*) fopen(filename, "w");

	if (out != 0L && DataBase == 0) {
		DEBUG(printf("\n ABOUT TO WRITE HEADER");)
		report_file_history(out, filename);
		fflush(out);
		DEBUG(printf("\n ABOUT TO WRITE HISTORY");)
		report_Random_hist(out);
		fflush(out);
		DEBUG(printf("\n ABOUT TO WRITE BLOCK OPS");)
		report_block_opts(out);
		fflush(out);
		DEBUG(printf("\n ABOUT TO WRITE GEOL OPS");)
		report_geol_opts(out);
		fflush(out);
		DEBUG(printf("\n ABOUT TO WRITE GEOPH OPS");)
		report_gps_opts(out);
		fflush(out);
		DEBUG(printf("\n ABOUT TO WRITE 3D OPS");)
		report_3d_opts(out);
		fflush(out);
		DEBUG(printf("\n ABOUT TO WRITE PROJECT OPS");)
		report_Random_proj_opts(out);
		fflush(out);
		report_Random_window_positions(out);
		fflush(out);
		ReportRandomIcons(out);
		fflush(out);

		error = FALSE;
	} else if (DataBase == 1) {
		DEBUG(printf("\n ABOUT TO WRITE HEADER");)
		upload_Random_file_history(filename, query);
		DEBUG(printf("\n ABOUT TO WRITE HISTORY");)
		upload_Random_hist(query);
		DEBUG(printf("\n ABOUT TO WRITE BLOCK OPS");)
		upload_Random_block_opts(query);
		DEBUG(printf("\n ABOUT TO WRITE GEOL OPS");)
		upload_Random_geol_opts(query);
		DEBUG(printf("\n ABOUT TO WRITE GEOPH OPS");)
		upload_Random_gps_opts(query);
		DEBUG(printf("\n ABOUT TO WRITE 3D OPS");)
		upload_Random_3d_opts(query);
		DEBUG(printf("\n ABOUT TO WRITE PROJECT OPS");)
		upload_Random_proj_opts(query);

		upload_Random_window_positions(query);
		upload_Random_icon_positions(query);

		sprintf(query, "%s')", query);

		//printf("%s\n",query);
		insert_query(query, 1);

		DEBUG(printf("\n FINISHED");)

		if (DataBase == 1) {
			free(query);
		} else {
			fflush(out);
			fclose(out);
		}

		error = FALSE;
	} else
		xvt_dm_post_error("Cannot Open History File.");

	return (!error);
}

int report_Random_window_positions(out)
	FILE *out; {
	register int winNum;
	int x, y, width, height;
	WINDOW win;
	RCT rect;
	fprintf(out, "\n#Window Positions\n");
	fprintf(out, "	Num Windows	= 16\n");
	fprintf(out, "	Name	= Block Diagram\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 500\n");
	fprintf(out, "	Height	= 300\n");
	fprintf(out, "	Name	= Movie\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= -1\n");
	fprintf(out, "	Height	= -1\n");
	fprintf(out, "	Name	= Well Log\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 400\n");
	fprintf(out, "	Height	= 430\n");
	fprintf(out, "	Name	= Section\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 500\n");
	fprintf(out, "	Height	= -1\n");
	fprintf(out, "	Name	= Topography Map\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 490\n");
	fprintf(out, "	Height	= 375\n");
	fprintf(out, "	Name	= 3D Topography Map\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 490\n");
	fprintf(out, "	Height	= 375\n");
	fprintf(out, "	Name	= 3D Stratigraphy\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 490\n");
	fprintf(out, "	Height	= 375\n");
	fprintf(out, "	Name	= Line Map\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 500\n");
	fprintf(out, "	Height	= -1\n");
	fprintf(out, "	Name	= Profile - From Image\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= -1\n");
	fprintf(out, "	Height	= -1\n");
	fprintf(out, "	Name	= Profile\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 490\n");
	fprintf(out, "	Height	= 600\n");
	fprintf(out, "	Name	= Sterographic Projections\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 430\n");
	fprintf(out, "	Height	= 430\n");
	fprintf(out, "	Name	= Stratigraphic Column\n");
	fprintf(out, "	X	= 60\n");
	fprintf(out, "	Y	= 60\n");
	fprintf(out, "	Width	= 230\n");
	fprintf(out, "	Height	= 400\n");
	fprintf(out, "	Name	= Image\n");
	fprintf(out, "	X	= 30\n");
	fprintf(out, "	Y	= 30\n");
	fprintf(out, "	Width	= -1\n");
	fprintf(out, "	Height	= -1\n");
	fprintf(out, "	Name	= Contour\n");
	fprintf(out, "	X	= 30\n");
	fprintf(out, "	Y	= 30\n");
	fprintf(out, "	Width	= -1\n");
	fprintf(out, "	Height	= -1\n");
	fprintf(out, "	Name	= Toolbar\n");
	fprintf(out, "	X	= 26\n");
	fprintf(out, "	Y	= 24\n");
	fprintf(out, "	Width	= -1\n");
	fprintf(out, "	Height	= -1\n");
	fprintf(out, "	Name	= History\n");
	fprintf(out, "	X	= 292\n");
	fprintf(out, "	Y	= 61\n");
	fprintf(out, "	Width	= 480\n");
	fprintf(out, "	Height	= 440\n");
	fprintf(out, "\n");

	return (TRUE);
}

int report_Random_file_history(out, filename, query)
	FILE *out;char *filename;char *query; {
	time_t longTime;
	struct tm *today;

	time(&longTime); /* current Time */
	today = localtime(&longTime);

	fprintf(out, "#Filename = %s\n", query, filename);
	fprintf(out, "#Date Saved = %d/%d/%d %d:%d:%d\n", query, today->tm_mday,
			today->tm_mon + 1, today->tm_year + 1900, today->tm_hour,
			today->tm_min, today->tm_sec);
	fprintf(out, "FileType = %d\nVersion = %.2f\n\n", query, 111,
	VERSION_NUMBER);
	return (TRUE);
}

int report_Random_block_opts(out, query)
	FILE *out;char *query; {
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

	fprintf(out, "\n#BlockOptions\n");
	fprintf(out, "\tNumber of Views\t= %d\n", query, numViews);
	fprintf(out, "\tCurrent View\t= %d\n", query, currentView);

	for (view = 0; view < numViews; view++) {
		fprintf(out, "\tNAME\t= %s\n", query, viewOptions->viewName);

		fprintf(out, "\tOrigin X\t= %6.2lf\n", query, viewOptions->originX);
		fprintf(out, "\tOrigin Y\t= %6.2lf\n", query, viewOptions->originY);
		fprintf(out, "\tOrigin Z\t= %6.2lf\n", query, viewOptions->originZ);

		fprintf(out, "\tLength X\t= %6.2lf\n", query, viewOptions->lengthX);
		fprintf(out, "\tLength Y\t= %6.2lf\n", query, viewOptions->lengthY);
		fprintf(out, "\tLength Z\t= %6.2lf\n", query, viewOptions->lengthZ);

		fprintf(out, "\tGeology Cube Size\t= %6.2lf\n", query,
				viewOptions->geologyCubeSize);
		fprintf(out, "\tGeophysics Cube Size\t= %6.2lf\n", query,
				viewOptions->geophysicsCubeSize);

		viewOptions = viewOptions->next;
	}

	return (TRUE);
}

int report_Random_geol_opts(out, query)
	FILE *out;char *query; {
	char directory[200];
	BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions();

	fprintf(out, "\n#GeologyOptions\n");
	/*
	 fprintf(out,"\tX\t= %6.2lf\n",query, viewOptions->originX);
	 fprintf(out,"\tY\t= %6.2lf\n",query, viewOptions->originY);
	 fprintf(out,"\tZ\t= %6.2lf\n",query, viewOptions->originZ - viewOptions->lengthZ);
	 */

	fprintf(out, "\tScale\t= %6.2lf\n", query, iscale);
	fprintf(out, "\tSectionDec\t= %6.2lf\n", query,
			geologyOptions.welllogDeclination);
	fprintf(out, "\tWellDepth\t= %6.2lf\n", query, geologyOptions.welllogDepth);
	fprintf(out, "\tWellAngleZ\t= %6.2lf\n", query,
			geologyOptions.welllogAngle);

	fprintf(out, "\tBoreholeX\t= %6.2lf\n", query, geologyOptions.boreholeX);
	fprintf(out, "\tBoreholeX\t= %6.2lf\n", query, geologyOptions.boreholeY);
	fprintf(out, "\tBoreholeX\t= %6.2lf\n", query, geologyOptions.boreholeZ);
	fprintf(out, "\tBoreholeDecl\t= %6.2lf\n", query,
			geologyOptions.boreholeDecl);
	fprintf(out, "\tBoreholeDip\t= %6.2lf\n", query,
			geologyOptions.boreholeDip);
	fprintf(out, "\tBoreholeLength\t= %6.2lf\n", query,
			geologyOptions.boreholeLength);

	fprintf(out, "\tSectionX\t= %6.2lf\n", query, geologyOptions.sectionX);
	fprintf(out, "\tSectionY\t= %6.2lf\n", query, geologyOptions.sectionY);
	fprintf(out, "\tSectionZ\t= %6.2lf\n", query, geologyOptions.sectionZ);
	fprintf(out, "\tSectionDecl\t= %6.2lf\n", query,
			geologyOptions.sectionDecl);
	fprintf(out, "\tSectionLength\t= %6.2lf\n", query,
			geologyOptions.sectionLength);
	fprintf(out, "\tSectionHeight\t= %6.2lf\n", query,
			geologyOptions.sectionHeight);

	if (geologyOptions.useTopography) {
		fprintf(out, "\ttopofile\t= TRUE\n");
		fprintf(out, "\tTopo Filename\t= %s\n", query, topoFileSpec.name);
		//xvt_fsys_convert_dir_to_str(&(topoFileSpec.dir), directory, 200);
	} else {
		fprintf(out, "\ttopofile\t= FALSE\n");
		fprintf(out, "\tTopo Filename\t= %s\n", query, "   ");
		strcpy(directory, ".");
	}
	fprintf(out, "\tTopo Directory\t= %s\n", query, directory);

	fprintf(out, "\tTopo Scale\t= %6.2lf\n", query,
			geologyOptions.topoScaleFactor);
	fprintf(out, "\tTopo Offset\t= %6.2lf\n", query,
			geologyOptions.topoOffsetFactor);
	fprintf(out, "\tTopo First Contour\t= %6.2lf\n", query,
			geologyOptions.topoLowContour);
	fprintf(out, "\tTopo Contour Interval\t= %6.2lf\n", query,
			geologyOptions.topoContourInterval);

	if (geologyOptions.calculateChairDiagram)
		fprintf(out, "\tChair Diagram\t= TRUE\n");
	else
		fprintf(out, "\tChair Diagram\t= FALSE\n");
	fprintf(out, "\tChair_X\t= %6.2lf\n", query, geologyOptions.chair_X);
	fprintf(out, "\tChair_Y\t= %6.2lf\n", query, geologyOptions.chair_Y);
	fprintf(out, "\tChair_Z\t= %6.2lf\n", query, geologyOptions.chair_Z);

	/*
	 fprintf(out,"\tLinemap_X\t= %6.2lf\n",query, geologyOptions.linemap_X);
	 fprintf(out,"\tLinemap_Y\t= %6.2lf\n",query, geologyOptions.linemap_Y);

	 fprintf(out,"\tCube Resolution\t= %.0f\n",query,viewOptions->geologyCubeSize);
	 fprintf(out,"\tCube Scale\t= 1.0\n");
	 */
	return (TRUE);
}

int report_Random_gps_opts(out, query)
	FILE *out;char *query; {
	BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions();

	fprintf(out, "\n#GeophysicsOptions\n");

	fprintf(out, "\tGPSRange\t = %d\n", query,
			geophysicsOptions.calculationRange);
	fprintf(out, "\tDeclination\t= %6.2lf\n", query,
			geophysicsOptions.declination);

	fprintf(out, "\tInclination\t= %6.2lf\n", query,
			geophysicsOptions.inclination);
	fprintf(out, "\tIntensity\t= %6.2lf\n", query, geophysicsOptions.intensity);

	if (geophysicsOptions.fieldType == FIXED_FIELD)
		fprintf(out, "\tField Type\t= FIXED\n");
	else
		fprintf(out, "\tField Type\t= VARIABLE\n");
	fprintf(out, "\tField xPos\t= %6.2lf\n", query, geophysicsOptions.xPos);
	fprintf(out, "\tField yPos\t= %6.2lf\n", query, geophysicsOptions.yPos);
	fprintf(out, "\tField zPos\t= %6.2lf\n", query, geophysicsOptions.zPos);

	fprintf(out, "\tInclination Ori\t= %6.2lf\n", query,
			geophysicsOptions.inclinationOri);
	fprintf(out, "\tInclination Change\t= %6.2lf\n", query,
			geophysicsOptions.inclinationChange);
	fprintf(out, "\tIntensity Ori\t= %6.2lf\n", query,
			geophysicsOptions.intensityOri);
	fprintf(out, "\tIntensity Change\t= %6.2lf\n", query,
			geophysicsOptions.intensityChange);
	fprintf(out, "\tDeclination Ori\t= %6.2lf\n", query,
			geophysicsOptions.declinationOri);
	fprintf(out, "\tDeclination Change\t= %6.2lf\n", query,
			geophysicsOptions.declinationChange);

	fprintf(out, "\tAltitude\t= %6.2lf\n", query, geophysicsOptions.altitude);

	if (geophysicsOptions.calculationAltitude)
		fprintf(out, "\tAirborne= \tTRUE\n");
	else
		fprintf(out, "\tAirborne= \tFALSE\n");

	if (geophysicsOptions.calculationMethod == SPATIAL)
		fprintf(out, "\tCalculation Method\t= SPATIAL\n");
	else if (geophysicsOptions.calculationMethod == SPECTRAL)
		fprintf(out, "\tCalculation Method\t= SPECTRAL\n");
	else
		/* Full */
		fprintf(out, "\tCalculation Method\t= FULL\n");

	switch (geophysicsOptions.spectralPaddingType) {
	case (RAMP_PADDING):
		fprintf(out, "\tSpectral Padding Type\t= RAMP_PADDING\n");
		break;
	case (FENCE_MEAN_PADDING):
		fprintf(out, "\tSpectral Padding Type\t= FENCE_MEAN_PADDING\n");
		break;
	case (FENCE_VALUE_PADDING):
		fprintf(out, "\tSpectral Padding Type\t= FENCE_VALUE_PADDING\n");
		break;
	case (SET_MEAN_PADDING):
		fprintf(out, "\tSpectral Padding Type\t= SET_MEAN_PADDING\n");
		break;
	case (SET_VALUE_PADDING):
		fprintf(out, "\tSpectral Padding Type\t= SET_VALUE_PADDING\n");
		break;
	case (REFLECTION_PADDING):
		fprintf(out, "\tSpectral Padding Type\t= REFLECTION_PADDING\n");
		break;
	default:
		fprintf(out, "\tSpectral Padding Type\t= UNKNOWN\n");
	}
	fprintf(out, "\tSpectral Fence\t= %d\n", query,
			geophysicsOptions.spectralFence);
	fprintf(out, "\tSpectral Percent\t= %d\n", query,
			geophysicsOptions.spectralPercent);

	fprintf(out, "\tConstant Boxing Depth\t= %6.2lf\n", query,
			geophysicsOptions.constantBoxDepth);
	fprintf(out, "\tClever Boxing Ratio\t= %6.2lf\n", query,
			geophysicsOptions.cleverBoxRatio);

	if (geophysicsOptions.deformableRemanence)
		fprintf(out, "\tDeformable Remanence= \tTRUE\n");
	else
		fprintf(out, "\tDeformable Remanence= \tFALSE\n");

	if (geophysicsOptions.deformableAnisotropy)
		fprintf(out, "\tDeformable Anisotropy= \tTRUE\n");
	else
		fprintf(out, "\tDeformable Anisotropy= \tFALSE\n");

	if (geophysicsOptions.magneticVectorComponents)
		fprintf(out, "\tVector Components= \tTRUE\n");
	else
		fprintf(out, "\tVector Components= \tFALSE\n");

	if (geophysicsOptions.projectVectorsOntoField)
		fprintf(out, "\tProject Vectors= \tTRUE\n");
	else
		fprintf(out, "\tProject Vectors= \tFALSE\n");

	if (geophysicsOptions.padWithRealGeology)
		fprintf(out, "\tPad With Real Geology= \tTRUE\n");
	else
		fprintf(out, "\tPad With Real Geology= \tFALSE\n");

	if (geophysicsOptions.drapedSurvey)
		fprintf(out, "\tDraped Survey= \tTRUE\n");
	else
		fprintf(out, "\tDraped Survey= \tFALSE\n");
	return (TRUE);
}

int report_Random_3d_opts(out, query)
	FILE *out;char *query; {
	fprintf(out, "\n#3DOptions\n");

	fprintf(out, "\tDeclination\t= %f\n", query, threedViewOptions.declination);
	fprintf(out, "\tElevation\t= %f\n", query, threedViewOptions.azimuth);
	fprintf(out, "\tScale\t= %f\n", query, threedViewOptions.scale);

	fprintf(out, "\tOffset X\t= %f\n", query, threedViewOptions.offset_X);
	fprintf(out, "\tOffset Y\t= %f\n", query, threedViewOptions.offset_Y);
	fprintf(out, "\tOffset Z\t= %f\n", query, threedViewOptions.offset_Z);

	fprintf(out, "\tFill Type\t= %d\n", query, threedViewOptions.fillType);
	return (TRUE);
}

int report_Random_proj_opts(out, query)
	FILE *out;char *query; {
	fprintf(out, "\n#ProjectOptions\n");

	if (projectOptions.susceptibilityUnits == SI)
		fprintf(out, "\tSusceptibility Units\t= SI\n");
	else
		fprintf(out, "\tSusceptibility Units\t= CGS\n");
	fprintf(out, "\tGeophysical Calculation\t= %d\n", query,
			projectOptions.geophysicalCalc);
	if (projectOptions.calculationType == REMOTE_JOB)
		fprintf(out, "\tCalculation Type\t= REMOTE_JOB\n");
	else
		fprintf(out, "\tCalculation Type\t= LOCAL_JOB\n");
	fprintf(out, "\tLength Scale\t= %d\n", query, projectOptions.lengthScale);
	fprintf(out, "\tPrinting Scale\t= %lf\n", query,
			projectOptions.printScalingFactor);
	fprintf(out, "\tImage Scale\t= %lf\n", query,
			projectOptions.imageScalingFactor);

	if (projectOptions.newWindowEachStage)
		fprintf(out, "\tNew Windows\t= TRUE\n");
	else
		fprintf(out, "\tNew Windows\t= FALSE\n");

	fprintf(out, "\tBackground Red Component\t= %d\n", query,
			XVT_COLOR_GET_RED(backgroundColor));
	fprintf(out, "\tBackground Green Component\t= %d\n", query,
			XVT_COLOR_GET_GREEN(backgroundColor));
	fprintf(out, "\tBackground Blue Component\t= %d\n", query,
			XVT_COLOR_GET_BLUE(backgroundColor));

	fprintf(out, "\tInternet Address\t= %s\n", query,
			projectOptions.internetAddress);
	fprintf(out, "\tAccount Name\t= %s\n", query, projectOptions.accountName);
	fprintf(out, "\tNoddy Path\t= %s\n", query, projectOptions.noddyPath);

	fprintf(out, "\tHelp Path\t= %s\n", query, projectOptions.helpPath);

	fprintf(out, "\tMovie Frames Per Event\t= %d\n", query,
			projectOptions.movieFramesPerEvent);
	fprintf(out, "\tMovie Play Speed\t= %6.2lf\n", query,
			projectOptions.moviePlaySpeed);
	fprintf(out, "\tMovie Type\t= %d\n", query, projectOptions.movieType);

	if (projectOptions.gravityDisplay.clippingType == ABSOLUTE_CLIPPING)
		fprintf(out, "\tGravity Clipping Type\t= ABSOLUTE_CLIPPING\n");
	else
		fprintf(out, "\tGravity Clipping Type\t= RELATIVE_CLIPPING\n");
	fprintf(out, "\tGravity Image Display Clip Min\t= %lf\n", query,
			projectOptions.gravityDisplay.minClip);
	fprintf(out, "\tGravity Image Display Clip Max\t= %lf\n", query,
			projectOptions.gravityDisplay.maxClip);
	if (projectOptions.gravityDisplay.display == CONTOUR_IMAGE)
		fprintf(out, "\tGravity Image Display Type\t= CONTOUR\n");
	else if (projectOptions.gravityDisplay.display == PSEUDO_COLOR)
		fprintf(out, "\tGravity Image Display Type\t= PSEUDO_COLOR\n");
	else
		/* GREY_COLOR */
		fprintf(out, "\tGravity Image Display Type\t= GREY\n");
	fprintf(out, "\tGravity Image Display Num Contour\t= %d\n", query,
			projectOptions.gravityDisplay.numContours);

	if (projectOptions.magneticsDisplay.clippingType == ABSOLUTE_CLIPPING)
		fprintf(out, "\tMagnetics Clipping Type\t= ABSOLUTE_CLIPPING\n");
	else
		fprintf(out, "\tMagnetics Clipping Type\t= RELATIVE_CLIPPING\n");
	fprintf(out, "\tMagnetics Image Display Clip Min\t= %lf\n", query,
			projectOptions.magneticsDisplay.minClip);
	fprintf(out, "\tMagnetics Image Display Clip Max\t= %lf\n", query,
			projectOptions.magneticsDisplay.maxClip);
	if (projectOptions.magneticsDisplay.display == CONTOUR_IMAGE)
		fprintf(out, "\tMagnetics Image Display Type\t= CONTOUR\n");
	else if (projectOptions.magneticsDisplay.display == PSEUDO_COLOR)
		fprintf(out, "\tMagnetics Image Display Type\t= PSEUDO_COLOR\n");
	else
		/* GREY_COLOR */
		fprintf(out, "\tMagnetics Image Display Type\t= GREY\n");
	fprintf(out, "\tMagnetics Image Display Num Contour\t= %d\n", query,
			projectOptions.magneticsDisplay.numContours);

	fprintf(out, "\tFalse Easting\t= %lf\n", query, projectOptions.easting);
	fprintf(out, "\tFalse Northing\t= %lf\n", query, projectOptions.northing);

	return (TRUE);
}

int reportRandomStratigraphy(stream, options)
	FILE *stream;STRATIGRAPHY_OPTIONS *options; {
	int i;

	fprintf(stream, "\tNum Layers\t= %d\n", options->numLayers);
	for (i = 0; i < options->numLayers; i++) {
		reportRandomProperties(stream, &(options->properties[i]));
	}
	return (TRUE);
}

int reportRandomColor(stream, options)
	FILE *stream;NODDY_COLOUR *options; {
	fprintf(stream, "\tColor Name\t= %s\n", options->name);
	fprintf(stream, "\tRed\t= %d\n", options->red);
	fprintf(stream, "\tGreen\t= %d\n", options->green);
	fprintf(stream, "\tBlue\t= %d\n", options->blue);
	return (TRUE);
}

int reportRandomProperties(stream, options)
	FILE *stream;LAYER_PROPERTIES *options; {
	fprintf(stream, "\tUnit Name\t= %s\n", options->unitName);
	fprintf(stream, "\tHeight\t= %d\n", options->height);
	if (options->applyAlterations)
		fprintf(stream, "\tApply Alterations\t= ON\n");
	else
		fprintf(stream, "\tApply Alterations\t= OFF\n");

	fprintf(stream, "\tDensity\t= %6.2le\n", options->density);

	fprintf(stream, "\tAnisotropic Field\t= %d\n", options->anisotropicField);
	fprintf(stream, "\tMagSusX\t= %6.2le\n", options->sus_X);
	fprintf(stream, "\tMagSusY\t= %6.2le\n", options->sus_Y);
	fprintf(stream, "\tMagSusZ\t= %6.2le\n", options->sus_Z);
	fprintf(stream, "\tMagSus Dip\t= %6.2le\n", options->sus_dip);
	fprintf(stream, "\tMagSus DipDir\t= %6.2le\n", options->sus_dipDirection);
	fprintf(stream, "\tMagSus Pitch\t= %6.2le\n", options->sus_pitch);

	fprintf(stream, "\tRemanent Magnetization\t= %d\n",
			options->remanentMagnetization);
	fprintf(stream, "\tInclination\t= %6.2lf\n", options->inclination);
	fprintf(stream, "\tAngle with the Magn. North\t= %6.2lf\n",
			options->angleWithNorth);
	fprintf(stream, "\tStrength\t= %6.2le\n", options->strength);

	reportRandomColor(stream, &(options->color));
	return (TRUE);
}



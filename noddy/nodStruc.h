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
#ifndef NODDY_STRUCTURES
#define NODDY_STRUCTURES

#include "3d.h"
#include "dxf.h"
/*
**      the dimensions of the block diagram when it is drawn
*/
#define BLOCK_DIAGRAM_SIZE_X  10000
#define BLOCK_DIAGRAM_SIZE_Y  7000
#define BLOCK_DIAGRAM_SIZE_Z  5000
#define SZ_FNAME 100
/*
**      window states
*/
#define WIN_MINIMIZED 0
#define WIN_NORMAL    1
#define WIN_MAXIMIZED 2

#define ERROR_MESSAGE_LENGTH  50

#define DEFAULT_BLOCK_SCALING        0.035
#define DEFAULT_LPREVIEW_SCALING     0.7
#define PREVIEW_SIZE_RATIO           0.6

#define NUM_PROPERTIES_LUT    100
#define NUM_PROPERTIES_COLORS 36
#define PROPERTIES_COLORS_SAT 204
/*
**      declaration of all the different types of events
*/
typedef enum { STRATIGRAPHY,       FOLD,          FAULT,
               UNCONFORMITY,       SHEAR_ZONE,    DYKE,
               PLUG,               STRAIN,        TILT,
               FOLIATION,          LINEATION,     IMPORT,
               STOP,               GENERIC
} OBJECTS;

/*
**      declaration of all the different types of options
*/
typedef enum { UNKNOWN_OP,
               GEOLOGY_DISPLAY_OP,      GEOPHYSICS_SURVEY_OP,
               GEOPHYSICS_CALC_OP,      GEOPHYSICS_DISPLAY_OP,
               PROJECT_OP,              WINDOW_POSITION_OP,
               BLOCK_VIEW_OP,           MOVIE_OP,
               BOREHOLE_SECTION_OP,     VOLUME_SURFACE_OP
} OPTION_TYPE;

/*
**      declaration of all the different of Legend
*/
typedef enum { UNKNOWN_LEGENDS,
					BLOCK_DIAGRAM_LEGENDS,  BOREHOLE_LEGENDS,
					SECTION_LEGENDS,        MAP_LEGENDS,
					PROFILE_LEGENDS,        THREED_LEGENDS,
					MAG_IMAGE_LEGENDS,      GRAV_IMAGE_LEGENDS
} LEGEND_TYPES;

typedef enum { UNKNOWN_LEGEND,
               STRAT_COL_LEGEND,       BLOCK_DIM_LEGEND,
					ANOM_LUT_LEGEND,        NODDY_ID_LEGEND,
					ORIENT_LEGEND
} LEGEND_COMPONENTS;

typedef enum { DRAW_UNKNOWN,
		         DRAW_SET_POS,				DRAW_LINE,
					DRAW_CIRCLE,				DRAW_TEXT,
					DRAW_IMAGE,             DRAW_RECT
} DRAW_OPERATION;

/*
**      general type that is useful
*/
typedef enum { FIRST,   SECOND,   THIRD, FORTH, FIFTH, 
               SIXTH,   SEVENTH,  EIGTH, NINETH, TENTH
} ORDER;

typedef enum { 
          ROCK_PROP,
          DENSITY_PROP,    SUS_X_PROP,         SUS_Y_PROP,
          SUS_Z_PROP,      SUS_DIP_PROP,       SUS_DDIR_PROP,
          SUS_PITCH_PROP,  REM_STR_PROP,       REM_INC_PROP,
          REM_DEC_PROP,    ANISOTROPY_ON_PROP, REM_ON_PROP,
          UNDEFINED_PROP
} PROPERTY_TYPE;

/*
**  different batch operations that can be performed with noddy
*/
typedef enum {  BLOCK_MODEL,            DICER_BLOCK_MODEL,
                LINE_MAP_DIAGRAM,       SECTION_DIAGRAM,
                BLOCK_DIAGRAM,
                ANOMALIES,              ANOMALIES_AND_BLOCK,
                ANOMALIES_FROM_BLOCK,   ANOMALIES_IMAGE,
                ANOMALIES_PROFILE
} OPERATIONS;

/*
**       Operations to calculate in function Noddy
*/
#define CALC_BLOCK_MODEL       ((int) 1)   /* 0000 0001 */
#define CALC_MAGNETICS_IMAGE   ((int) 2)   /* 0000 0010 */
#define CALC_GRAVITY_IMAGE     ((int) 4)   /* 0000 0100 */
#define CALC_SURF_MODEL        ((int) 8)   /* 0000 1000 */
#define CALC_BLOCK_GEOPHYS     ((int) 16)  /* 0001 0000 */
#define CALC_ALL               ((int) 32)  /* 0010 0000 */
typedef enum {  INPUT_MEM_HISTORY,      INPUT_FILE_HISTORY,
                INPUT_FILE_BLOCK
} DATA_INPUT;

typedef enum {  OUTPUT_FILE_BLOCK,
                OUTPUT_FILE_IMAGE
} DATA_OUTPUT;

/*
**  error conditions for remote process communication
*/
typedef enum {  COMMAND_OK,             NO_COMMAND,
                NO_SYSTEM,              NO_INPUT_FILE,
                COMMAND_FAILED,         NO_OUTPUT_FILE
} REMOTE_CMD_STATUS;

/*
**  Side identifications of cube faces 
*/
typedef enum {  TOP_SIDE,               BOTTOM_SIDE,
                NORTH_SIDE,             EAST_SIDE,
                SOUTH_SIDE,             WEST_SIDE
} CUBE_SIDES;

/*
**  Operations for the 'copyOptions' routine in nodLib1.c
*/
typedef enum {  OPTIONS_RESTORE,      OPTIONS_SAVE,
                OPTIONS_FREE_MEMORY
} OPTION_OPERATIONS;


                          /* Types of Rock Stratigraphy */
#define BASE_STRAT      0x00     /* 0000 0000 */
#define UNC_STRAT       0x01     /* 0000 0001 */
#define FAULT1_STRAT    0x02     /* 0000 0010 */
#define IGNEOUS_STRAT   0x03     /* 0000 0011 */
#define FAULT2_STRAT    0x04     /* 0000 0100 */   /* This and FAULT3 OR'ed must be unique */
#define FAULT3_STRAT    0x08     /* 0000 1000 */
#define ALL_STRAT       0x0f     /* 0000 1111 */

                          /* flag used to increment the process bar */
#define INCREMENT_JOB    -1
#define INC_JOB_BY(X)    -X
#define ABORT_JOB        99

                          /* types of lookup for rocks */
enum {    LINE,     TOPOMAP,  BLOCK,    STRATKEY,
          TRAVERSE, CON,      IM,       SECTION,
          WELLLOG,  FOREIGN,  THREED,   THREEDLAYER
};
                          /* Anomilies that can be calculated */
enum {    ANOM,               ANOM_FROM_BLOCK,
          BLOCK_AND_ANOM,     BLOCK_ONLY,
          XYZ_ANOM, TOPOLOGY
};
                          /* PROFILES THAT OBJECTS CAN HAVE */
enum {    STANDARD_PROFILE,   ALTERATION_PROFILE
};

                          /* Event Options logical groupings */
typedef int OPTION_GROUPS;

#define TAB_LABEL_LENGTH    40
typedef struct s_tab {

   RCT region;                   /* tab position */
   int icon;
   char label[TAB_LABEL_LENGTH]; /* tab label */
   OPTION_GROUPS group;          /* group associate with tab */
   WINDOW win;

} TAB;

/*
** Pen sizes for drawing different block sizes
*/
#if (XVTWS == MACWS)
#define LARGE_PEN_SIZE        17
#define MED_LARGE_PEN_SIZE    9
#define MEDIUM_PEN_SIZE  4
#define MED_SMALL_PEN_SIZE    3
#define SMALL_PEN_SIZE   2
#else
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
#define LARGE_PEN_SIZE        17
#define MED_LARGE_PEN_SIZE    9
#define MEDIUM_PEN_SIZE  4
#define MED_SMALL_PEN_SIZE    3
#define SMALL_PEN_SIZE   2
#else
#if (XVTWS == MTFWS)
#define LARGE_PEN_SIZE        16
#define MED_LARGE_PEN_SIZE    8
#define MEDIUM_PEN_SIZE  4
#define MED_SMALL_PEN_SIZE    3
#define SMALL_PEN_SIZE   2
#else
#define LARGE_PEN_SIZE        14
#define MED_LARGE_PEN_SIZE    8
#define MEDIUM_PEN_SIZE  4
#define MED_SMALL_PEN_SIZE    2
#define SMALL_PEN_SIZE   1
#endif
#endif
#endif


/*
** character array dimensions used in structures
*/
#define OBJECT_TEXT_LENGTH      100
#define UNIT_NAME_LENGTH        20
#define COLOUR_NAME_LENGTH      30


/*
**      Data structure to hold information about each object. The head of a
**      linked-list of these structures is in the window-info structure.
*/
typedef struct s_object {          /* data for displayable object */
   struct s_object *next;          /* next object in this window's list */
   BOOLEAN drawEvent;
   BOOLEAN selected;               /* is it selected? */
   OBJECTS shape;                  /* shape */
   char text[OBJECT_TEXT_LENGTH];  /* text associated with object */
   RCT bound;                      /* bounding box */
   int row, column;                /* row and column of position in grid */
   BOOLEAN newObject;              /* tell if object is just created */
   OPTION_GROUPS optionGroup;
   WINDOW previewWin;              /* window to draw previews in */
   int generalData;                /* used as a general info tag */
   char *options;                  /* pointer to the options structure */
} OBJECT;


/*
** structures used for 3d window data
*/
typedef struct s_3d_point {
   Point3d point;
   COLOR color;
} THREED_POINT_INFO;

typedef struct s_3d_data {

   enum { TOPO_3D, STRAT_3D, LAYERS_3D } imageType;
   XVT_PIXMAP pixmap;
   Point3d camera;
   Point3d focus;
   THREED_POINT_INFO __huge *surface;
   long surfaceWidth;
   long surfaceHeight;
   double declination, azimuth, scale;

} THREED_IMAGE_DATA;

typedef struct s_block_surf_data {

   double xStart, yStart, zStart;
   double xEnd, yEnd, zEnd;
   double dip, dipDirection;
   enum { FORWARD_BOTH, FORWARD_GREATER, FORWARD_LESS } forwardCheck;
   short depth;
   COLOR **surfaceData;
   int dataDim1, dataDim2;
   struct s_block_surf_data *next;

} BLOCK_SURFACE_DATA;


/*
** structures used for xyz data import
*/
typedef struct s_xyz_import {

   enum { XYZ_GEO_IMPORT, BOREHOLE_IMPORT, XYZ_IMAGE_IMPORT } type;
   FILE_SPEC fileSpec;
   long data;
   
} XYZ_IMPORT;


/*
** structures used for linemap window data
*/
#define MAX_LINEMAP_EVENTS    100
#define NUM_LINEMAP_TERMS     8 
typedef struct s_section_data {

   XVT_PIXMAP pixmap;
   double orientations[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS];
   int count;  /* count of the number of orientations */
   enum { BEDDING_SYM = 1,    FOLIATION_SYM,
          LINEATION_SYM,      BD_CL_SYM,
          CL_CL_SYM } symbol;
   int event1, event2;
   int define;
   BLOCK_SURFACE_DATA *surfaceData;
   
} SECTION_DATA;

/*
** structures used for movie window data
*/
typedef struct s_movie_data {

   XVT_PIXMAP pixmap;
   struct s_movie_data *next;
   struct s_movie_data *prev;

} MOVIE_DATA;

/*
** structures used for anomalies image window data
*/
typedef struct s_image_display_data {

   enum { RELATIVE_CLIPPING, ABSOLUTE_CLIPPING } clippingType;
   double minClip, maxClip;  /* always stored in absolute values */
   enum { GREY_COLOR, PSEUDO_COLOR, CONTOUR_IMAGE } display;
   int numContours;
	COLOR *lut;
	int lutSize;
	BOOLEAN gridOverlay, autoGridSpacing, sensorOverlay;
	double gridStartX, gridStartY;
	double gridIncX, gridIncY;

} IMAGE_DISPLAY_DATA;

typedef struct s_anomImage_data {

   XVT_PIXMAP pixmap;
   double **imageData;
   int dataXSize, dataYSize;
   double minValue, maxValue, scale;
   double geoXStart, geoXEnd, geoYStart, geoYEnd, geoHeight;
	double fileCubeSize, fileBlockTop, fileBlockBottom;
   double inclination, declination, intensity;
   enum { GRAVITY_DATA, MAGNETICS_DATA } dataType;
   IMAGE_DISPLAY_DATA imageDisplay;

	double scalingAtCreation;

	char historyFile[SZ_FNAME];
	char dateTime[30];

} ANOMIMAGE_DATA;

typedef struct s_draw_data {

   DRAW_OPERATION operation;
	PNT point;
	char *data;
   struct s_draw_data *next;

} DRAW_DATA;

#define LEGEND_TITLE_LENGTH   30
typedef struct s_legend_data {

   char title[LEGEND_TITLE_LENGTH];
   LEGEND_COMPONENTS component;
	XVT_PIXMAP pixmap;
	DRAW_DATA *drawData;
	PNT origin;
   struct s_legend_data *next;

} LEGEND_DATA;

typedef struct s_double_2d_image {

   double **data;
   int dim1, dim2;

} DOUBLE_2D_IMAGE;


/*
** structures used for profile drawing
*/
#define X_PROFILE_RESOLUTION    628  /* 0 to 628 */
#define Y_PROFILE_RESOLUTION    100  /* from -ve 100 to +ve 100 */
#define PROFILE_DRAGPOINT_SIZE  6
#define PROFILE_TEXT_LENGTH     50

enum { STRAIGHT_PROFILE, SINE_PROFILE };

typedef struct s_profile_point {

   struct s_profile_point *next;
   PNT point;

} PROFILE_POINT;

typedef struct s_profile_options {

   char name[PROFILE_TEXT_LENGTH];
   enum { UNDEFINED_PROFILE, PLANE_PROFILE,
          DENSITY_PROFILE,   ANI_PROFILE,       SUS_X_PROFILE,
          SUS_Y_PROFILE,     SUS_Z_PROFILE,     SUS_DIP_PROFILE,
          SUS_DDIR_PROFILE,  SUS_PITCH_PROFILE, REM_PROFILE,
          REM_DEC_PROFILE,   REM_INC_PROFILE,   REM_STR_PROFILE
        } type;
   PROFILE_POINT *points;
   PROFILE_POINT *currentPoint;
   enum { LINE_SEGMENTS, CURVE_SEGMENTS, SQUARE_SEGMENTS } joinType;
   float *array;
   double graphLength;
   double minX, maxX;
   double minYScale,   maxYScale, scaleOrigin;
   double minYReplace, maxYReplace;
   struct s_profile_options *next;

} PROFILE_OPTIONS;

/*
** structures used for trace window data
*/
typedef struct s_trace_data {

   XVT_PIXMAP pixmap;
   OBJECT *object;
   PROFILE_POINT *points;
   WINDOW menuWindow, imageWindow;
   
} TRACE_DATA;

/*
** structures used for the event builder
*/
typedef enum { FLAT_SURFACE, DXF_SURFACE,
               UNKNOWN_SURFACE
} SURFACE_TYPE;

typedef enum { NONE_ALTERATION,   TOP_ALTERATION,
               BOTTOM_ALTERATION, BOTH_ALTERATION
} ALTERATION_TYPE;

typedef struct s_plane {

   double a, b, c, d;

} PLANE;

typedef struct s_rotation_matrices {

   double forward[3][3];
   double reverse[3][3];

} ROTATION_MATRICES;

typedef struct s_noddy_color {

   unsigned char red;
   unsigned char green;
   unsigned char blue;
   char name[COLOUR_NAME_LENGTH];

} NODDY_COLOUR;

typedef struct s_layer_properties {
                          /* Geology */
   char unitName[UNIT_NAME_LENGTH];
   int height;
                          /* Density */
   double density;
                          /* Susceptibility */
   int anisotropicField;
   double sus_X;
   double sus_Y;
   double sus_Z;
   double sus_dip;
   double sus_dipDirection;
   double sus_pitch;
                          /* Remanent */
   int remanentMagnetization;
   double inclination;
   double angleWithNorth;
   double strength;
             /* How Alteration Zones effect Layer */
   int applyAlterations;
             /* define Remenant + Susceptibility */
   ROTATION_MATRICES remRotationMatrix;
   ROTATION_MATRICES aniRotationMatrix;
                          /* Colour */
   NODDY_COLOUR color;
   
} LAYER_PROPERTIES;

typedef struct s_block_diag_data {

   COLOR ***blockData;
   float ***valueData;
   float minValueData, maxValueData;
   COLOR *lut;
   int lutSize;
   enum { UNDEFINED_BLOCK, SOLID_BLOCK,     LAYERED_BLOCK,
          DENSITY_BLOCK,   SUS_X_BLOCK,     SUS_Y_BLOCK,
          SUS_Z_BLOCK,     SUS_DIP_BLOCK,   SUS_DDIR_BLOCK,
          SUS_PITCH_BLOCK, REM_STR_BLOCK,   REM_INC_BLOCK,
          REM_DEC_BLOCK
        } type;
   LAYER_PROPERTIES **layersToDraw;
   int numLayersToDraw;
   int numBoreHoles;
   double ***boreHoles;
   int *numPointInBoreHoles;
   int nx, ny, nz;
   double minXLoc, minYLoc, minZLoc;
   double blockSize;
   THREED_IMAGE_DATA threedData;
   BLOCK_SURFACE_DATA *surfaces;

} BLOCK_DIAGRAM_DATA;

typedef struct s_stratigraphy {

   int numLayers;
   LAYER_PROPERTIES *properties;

} STRATIGRAPHY_OPTIONS;

typedef struct s_fold {
                          /* Form */
   enum { SINE_FOLD, BIOT_FOLD, FOURIER_FOLD } type;
   int singleFold;
                         /* Position */
   double positionX;
   double positionY;
   double positionZ;
                         /* Orientation */
   double dipDirection;
   double dip;
   double axisPitch;
                         /* Scale */
   double wavelength;
   double amplitude;
   double cycle;

   ROTATION_MATRICES rotationMatrix;
   double fourierCoeff[2][11];
   PROFILE_OPTIONS profile;

} FOLD_OPTIONS;

typedef struct s_fault {
                /* Form */
   enum { FAULT_EVENT, SHEAR_ZONE_EVENT, KINK_EVENT } type;
   enum { TRANSLATION, ROTATION, ELLIPTICAL, CURVED, RING } geometry;
   enum { HANGING_WALL, FOOT_WALL, BOTH } movement;
                         /* Position */
   double positionX;
   double positionY;
   double positionZ;
                         /* Orientation */
   double dipDirection;
   double dip;
   double pitch;
                         /* Scale */
   double rotation;
   double slip;
   double amplitude;
   double radius;
   double width;
   double xAxis;
   double yAxis;
   double zAxis;
   double cylindricalIndex;
   double profilePitch;

   NODDY_COLOUR color;

   ROTATION_MATRICES rotationMatrix;
   double fourierCoeff[2][11];
   PROFILE_OPTIONS profile;
   PLANE plane;
   double alpha, beta, gamma;

   ALTERATION_TYPE alterationZones;
   PROFILE_OPTIONS *alterationFuntions;
   
   FILE_SPEC surfaceFilename;
   SURFACE_TYPE surfaceType;
   double surfaceXDim;
   double surfaceYDim;
   double surfaceZDim;
   DXFHANDLE dxfData;

} FAULT_OPTIONS;

typedef struct s_unconformity {
                         /* Position */
   double positionX;
   double positionY;
   double positionZ;
                         /* Orientation */
   double dipDirection;
   double dip;
                         /* stratigraphy Details */
   STRATIGRAPHY_OPTIONS stratigraphyDetails;
   ROTATION_MATRICES rotationMatrix;
   PLANE plane;

   ALTERATION_TYPE alterationZones;
   PROFILE_OPTIONS *alterationFuntions;

   FILE_SPEC surfaceFilename;
   SURFACE_TYPE surfaceType;
   double surfaceXDim;
   double surfaceYDim;
   double surfaceZDim;
   DXFHANDLE dxfData;

} UNCONFORMITY_OPTIONS;


typedef FAULT_OPTIONS SHEAR_OPTIONS;


typedef struct s_dyke {
                          /* Form */
   enum { DILATION_DYKE, STOPE_DYKE } type;
   int mergeEvents;
                         /* Position */
   double positionX;
   double positionY;
   double positionZ;
                         /* Orientation */
   double dipDirection;
   double dip;
   double slipPitch;
                         /* Scale */
   double slipLength;
   double width;
                         /* properties */
   LAYER_PROPERTIES properties;

   ROTATION_MATRICES rotationMatrix;

   ALTERATION_TYPE alterationZones;
   PROFILE_OPTIONS *alterationFuntions;

} DYKE_OPTIONS;

typedef struct s_plug {
                          /* Form */
   enum { CYLINDRICAL_PLUG, CONE_PLUG, PARABOLIC_PLUG, ELLIPSOIDAL_PLUG } type;
   int mergeEvents;
                         /* Position */
   double positionX;
   double positionY;
   double positionZ;
                         /* Orientation */
   double dipDirection;
   double dip;
   double axisPitch;
                         /* Scale */
   double radius;
   double apicalAngle;
   double BValue;
   double axisA;
   double axisB;
   double axisC;
                         /* properties */
   LAYER_PROPERTIES properties;

   ROTATION_MATRICES rotationMatrix;

   ALTERATION_TYPE alterationZones;
   PROFILE_OPTIONS *alterationFuntions;

} PLUG_OPTIONS;

typedef struct s_strain {

   double tensor[3][3];

   double inverseTensor[3][3];

} STRAIN_OPTIONS;

typedef struct s_tilt {
                         /* Position */
   double positionX;
   double positionY;
   double positionZ;
                         /* Orientation */
   double rotation;
   double plungeDirection;
   double plunge;

   double rotationMatrix[3][3];

} TILT_OPTIONS;

typedef struct s_foliation {
                         /* Orientation */
   double dipDirection;
   double dip;

   ROTATION_MATRICES rotationMatrix;

} FOLIATION_OPTIONS;

typedef struct s_lineation {
                         /* Orientation */
   double plungeDirection;
   double plunge;

   ROTATION_MATRICES rotationMatrix;

} LINEATION_OPTIONS;

typedef struct s_import {
                         /* Position */
   enum { POS_BY_CENTER, POS_BY_CORNER } positionBy;
   double positionX;
   double positionY;
   double positionZ;
                         /* Scale */
   double blockSize;
   
   enum { NODDY_BLOCK_MODEL } fileType;

                         /* properties */
   LAYER_PROPERTIES *properties;
   int numProperties;
                         /* the store for the imported block data */
   short ***blockData;
   int numLayers;
   int **layerDimensions;
   int transparentLayer;
   
                         /* filename */
   FILE_SPEC importFile;

   ROTATION_MATRICES rotationMatrix;

} IMPORT_OPTIONS;

typedef struct s_generic {
   
   double param1;
   double param2;
   double param3;
   double param4;
   double param5;
   double param6;
   double param7;
   double param8;
   double param9;
   double param10;

} GENERIC_OPTIONS;
/*
** structures used for the options windows
*/
#define CGS_TO_SI 12.56637061  /* SI = CGS*4*PI */
typedef struct s_project_ops {
                         /* Orientation */
   enum { SI, CGS } susceptibilityUnits;
   enum { CALC_GRAVITY, CALC_MAGNETICS, CALC_GRAV_AND_MAG } geophysicalCalc;
   enum { LOCAL_JOB, REMOTE_JOB } calculationType;
   enum { METRES, FEET } lengthScale;
   double printScalingFactor;
   double imageScalingFactor;
   IMAGE_DISPLAY_DATA gravityDisplay;
   IMAGE_DISPLAY_DATA magneticsDisplay;
   short newWindowEachStage;
   char internetAddress[100];
   char accountName[50];
   char noddyPath[100];
   char helpPath[100];

   int movieFramesPerEvent;
   double moviePlaySpeed;
   enum { BLOCK_DIAGRAM_MOVIE,      MAP_MOVIE,
          LINE_MAP_MOVIE,           SECTION_MOVIE,
          LINE_SECTION_MOVIE,       WELL_LOG_MOVIE,
          THREED_STRAT_MOVIE,       GRAVITY_IMAGE_MOVIE,
          MAGNETICS_IMAGE_MOVIE
   } movieType;
   
   double easting;     /* False Eastings and Northing for block */
   double northing;

} PROJECT_OPTIONS;

#define DEFAULT_VIEW_NAME       "Default"
#define DEFAULT_VIEW_ORIGIN_X   0.0
#define DEFAULT_VIEW_ORIGIN_Y   0.0
#define DEFAULT_VIEW_ORIGIN_Z   5000.0
#define DEFAULT_VIEW_LENGTH_X   10000.0
#define DEFAULT_VIEW_LENGTH_Y   7000.0
#define DEFAULT_VIEW_LENGTH_Z   5000.0
#define DEFAULT_VIEW_GEOL_CUBE  200.0
#define DEFAULT_VIEW_GEOP_CUBE  200.0
typedef struct s_block_view_ops {
   
   char viewName[OBJECT_TEXT_LENGTH];
        /* Upper South West Corner (MinX, MinY, MaxZ) */
   double originX, originY, originZ;
   double lengthX, lengthY, lengthZ;

   double geologyCubeSize;
   double geophysicsCubeSize;
   
   struct s_block_view_ops *prev, *next;

} BLOCK_VIEW_OPTIONS;

typedef struct s_geophysics_ops {
                         /* Orientation */
   int calculationRange;
	int analyticRange;
	BOOLEAN allExact;
   double inclination;
   double intensity;
   double declination;
	double xPos, yPos, zPos;
   double inclinationOri, inclinationChange;
   double intensityOri,   intensityChange;
   double declinationOri, declinationChange;
	enum { FIXED_FIELD, VARIABLE_FIELD } fieldType;

   enum { SURFACE, AIRBORNE } calculationAltitude;
   double altitude;

   enum { SPATIAL, SPECTRAL, SPATIAL_FULL } calculationMethod;

   double constantBoxDepth;
   double cleverBoxRatio;

   BOOLEAN drapedSurvey;
   BOOLEAN deformableRemanence;
   BOOLEAN deformableAnisotropy;
   
   BOOLEAN magneticVectorComponents;
   BOOLEAN projectVectorsOntoField;
   int padWithRealGeology;
   
   enum { RAMP_PADDING,        FENCE_MEAN_PADDING,
          FENCE_VALUE_PADDING, SET_MEAN_PADDING,
          SET_VALUE_PADDING,   RECLECTION_PADDING } spectralPaddingType;
   int spectralFence, spectralPercent;
	double spectralSusX, spectralSusY, spectralSusZ, spectralDensity;

} GEOPHYSICS_OPTIONS;

typedef struct s_geology_ops {

   BOOLEAN calculateChairDiagram;
   double chair_X;
   double chair_Y;
   double chair_Z;

   double boreholeX, boreholeY, boreholeZ;
   double boreholeDecl, boreholeDip, boreholeLength;

   double sectionX, sectionY, sectionZ;
   double sectionDecl, sectionLength, sectionHeight;
   
   double welllogAngle;
   double welllogDeclination;
   double welllogDepth;

   BOOLEAN useTopography;

   double topoScaleFactor;
   double topoOffsetFactor;
   double topoLowContour;
   double topoContourInterval;
   
   BLOCK_DIAGRAM_DATA blockDiagram;
   
} GEOLOGY_OPTIONS;

typedef struct s_3dview_ops {
                         /* Orientation */
   double declination;
   double azimuth;
   double scale;

   double offset_X;
   double offset_Y;
   double offset_Z;

   int allLayers;
   short layerOn[100];

   enum { XYZ_COLOR_FILL,         SHADING_FILL,
          LINES_FILL,             DXF_FILE_FILL_3DFACE,
          DXF_FILE_FILL_POLYLINE, VULCAN_FILE_FILL
        } fillType;

} THREED_VIEW_OPTIONS;

typedef struct s_winpos {
                         /* Orientation */
   char name[30];
   short x, y, width, height;

} WINDOW_POSITION;

typedef struct s_winpos_ops {

   int numWindows;
   WINDOW_POSITION winPositions[16];
   
} WINDOW_POSITION_OPTIONS;


/*
** structures used for rock Database
*/
typedef struct s_file_format {

   enum { NODDY_BLOCK_FILE,         DICER_BLOCK_FILE,
          TABULATED_COL_BLOCK_FILE 
        } type;
   enum { INPUT, OUTPUT } access;
   PROPERTY_TYPE property;
   int singleValuePerLine;
   
   ORDER xOrder, yOrder, zOrder;

   enum { EAST_WEST,   WEST_EAST   } xDirection; 
   enum { NORTH_SOUTH, SOUTH_NORTH } yDirection; 
   enum { UP_DOWN,     DOWN_UP     } zDirection; 

} FILE_FORMAT;


/*
** structures used for rock Database
*/
typedef struct s_rock_database {

   LAYER_PROPERTIES *database;
   int numProps;

} ROCK_DATABASE;

/*
**      Comment tags used in anom image files
*/
#define ANOM_HISTORY_TAG	"#HISTORY FILE: "
#define ANOM_DATETIME_TAG	"#DATE AND TIME: "


/*
**      Other data stuctures used within noddy
*/

#define ARRAY_LENGTH_OF_STRAT_CODE     64
#define NUM_STRAT_CODE_IN_BYTE      2
#define NUM_BYTES_IN_STRAT_CODE     4  /* 8/NUM_STRAT_CODE_IN_BYTE */
typedef struct story {
    int again;
    unsigned char sequence[ARRAY_LENGTH_OF_STRAT_CODE];
} STORY;

typedef struct _pixel {
   unsigned short x;
   unsigned short y;
   double value;
} PIXEL;

/*
**  Font Sizes
*/
#if (XVTWS == MTFWS)
#define FONT_SMALL	8
#define FONT_MEDIUM	10
#define FONT_NORMAL	12
#define FONT_LARGE	14
#define FONT_HUGE		40
#else
#define FONT_SMALL	6
#define FONT_MEDIUM	8
#define FONT_NORMAL	10
#define FONT_LARGE	12
#define FONT_HUGE		30
#endif

/*
**  Useful MACROS
*/
#if (XVTWS == XOLWS)
#define SHORT_SWAP(X)   { unsigned char val1, val2;                \
                          val1 = (X&0xff00) >> 8;                  \
                          val2 = X&0x00ff;                         \
                          X = 0;                                   \
                          X = (val2<<8) | val1;                    \
                        }
#define DOUBLE_SWAP(X)  { unsigned char *valPtr;                   \
                          unsigned char val1, val2, val3, val4;    \
                          unsigned char val5, val6, val7, val8;    \
                          valPtr = (unsigned char *) &X;           \
                          val1 = valPtr[7]; val2 = valPtr[6];      \
                          val3 = valPtr[5]; val4 = valPtr[4];      \
                          val5 = valPtr[3]; val6 = valPtr[2];      \
                          val7 = valPtr[1]; val8 = valPtr[0];      \
                          valPtr[7] = val8; valPtr[6] = val7;      \
                          valPtr[5] = val6; valPtr[4] = val5;      \
                          valPtr[3] = val4; valPtr[2] = val3;      \
                          valPtr[1] = val2; valPtr[0] = val1;      \
                        }
#endif

#define SNAP_DOWN_BY(X,Y) if (X%Y) if (X<0) X-=(X%Y); else X+=Y-(X%Y);
#define SNAP_UP_BY(X,Y)   if (X%Y) if (X<0) X-=Y-abs(X%Y); else X-=(X%Y);

#if (XVTWS == MTFWS)
#define CORRECT_WIN_RESIZE(W, P) \
   {                                                                     \
      PNT corner;                                                        \
      corner.h = 0; corner.v = 0;                                        \
      xvt_vobj_translate_points(W, xvt_vobj_get_parent(W), &corner, 1);  \
      corner.h += xvt_vobj_get_attr(W, ATTR_DOCFRAME_WIDTH)/2 + 1;       \
      corner.v += xvt_vobj_get_attr(W, ATTR_TITLE_HEIGHT)                \
                  + xvt_vobj_get_attr(W, ATTR_DOCFRAME_HEIGHT) - 1;      \
      xvt_rect_set_pos (&P, corner);                                     \
   }
#endif
#if (XVTWS == XOLWS)
#define CORRECT_WIN_RESIZE(W, P) \
   {                                                                     \
      PNT corner;                                                        \
      corner.h = 0; corner.v = 0;                                        \
      xvt_vobj_translate_points(W, xvt_vobj_get_parent(W), &corner, 1);  \
      xvt_rect_set_pos (&P, corner);                                     \
   }
#endif
#if (XVTWS == MACWS)
#define CORRECT_WIN_RESIZE(W, P) \
   {                                                                     \
      PNT corner;                                                        \
      corner.h = 0; corner.v = 0;                                        \
      xvt_vobj_translate_points(W, xvt_vobj_get_parent(W), &corner, 1);  \
      xvt_rect_set_pos (&P, corner);                                     \
   }
#endif
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
#define CORRECT_WIN_RESIZE(W, P) \
   {                                                                     \
      PNT corner;                                                        \
      corner.h = 0; corner.v = 0;                                        \
      xvt_vobj_translate_points(W, xvt_vobj_get_parent(W), &corner, 1);  \
      xvt_rect_set_pos (&P, corner);                                     \
   }
#endif

#if ((XVTWS != WIN32WS) && (XVTWS != WIN16WS))
//#include <X11/Intrinsic.h>
/*
 * This file contains information used for cloning colors from
 * colors from the default X colormap to a copied colormap of the
 * appropriate depth for GL.  The following types determine how
 * to select the colors from the X colormap.
 */

#define GLXC_ABSOLUTE	0	/* use absolute pixel value */
#define GLXC_NAMED	1	/* look up by name */
#define GLXC_RESOURCE	2	/* lookup resource in parent widget */
    
/*
 * An array must be passed in of the following structure type, describing
 * the colors to be passed in.  The first three fields must be filled
 * in for each color, with the XColor being calculated.  Based on type,
 * value should be as follows:
 *
 *	type		value
 *	GLXC_ABSOLUTE	the pixel number to copy (e.g. 5)
 *	GLXC_NAMED	a color name (e.g. "red")
 *	GLXC_RESOURCE	a pixel resource name (e.g. XmNbackground)
 */

//struct glxcColorInfo
//{
//    int type;		/* One of above types */
//    caddr_t value;	/* based on above types */
//    long *result;	/* store result in this variable */
//   XColor color;	/* the xcolor definition */
//};
#endif


#endif

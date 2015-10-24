#ifndef BUILDER_INCLUDE
#define BUILDER_INCLUDE
/*
** BUILDER.H   header for builder.
*/

#include "nodStruc.h"

#ifndef MIN
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#endif


#define RID_ATTRIB      100
#define CID_TEXT     4
#define CID_OVAL     5
#define CID_RECT     6

#define TASK_MENUBAR    100
#define WIN_MENUBAR     200
#define M_HISTORY    768
#define M_TOOL_POINTER     769

#define M_HISTORY_STRATIGRAPHY   780
#define M_HISTORY_FOLD     781
#define M_HISTORY_FAULT    782
#define M_HISTORY_UNCONFORMITY   783
#define M_HISTORY_SHEAR    784
#define M_HISTORY_DYKE     785
#define M_HISTORY_PLUG     786
#define M_HISTORY_STRAIN   787
#define M_HISTORY_TILT     788
#define M_HISTORY_FOLIATION   789
#define M_HISTORY_LINEATION   790
#define M_HISTORY_STOP     791
#define M_HISTORY_IMPORT   792
#define M_HISTORY_GENERIC  793

#define RID_INSTR          200
#define LAST_RID_INSTR     (RID_INSTR + 44)

#define POINTER_ICON       10000
#define STRATIGRAPHY_ICON  10001
#define FOLD_ICON          10002
#define FAULT_ICON         10003
#define UNCONFORMITY_ICON  10004
#define SHEAR_ICON         10005
#define DYKE_ICON          10006
#define PLUG_ICON          10007
#define STRAIN_ICON        10008
#define TILT_ICON          10009
#define FOLIATION_ICON     10010
#define LINEATION_ICON     10011
#define STOP_ICON          10012
#define IMPORT_ICON        10013
#define GENERIC_ICON       10014


#define EVENT_POSITION_ICON   10100

#define FORM_ICON          10200
#define POSITION_ICON      10201
#define ORIENTATION_ICON   10202
#define SCALE_ICON         10203
#define TIME_ICON          10204
#define SURFACE_ICON       10205
#define ELLIPSOID_ICON     10206

#define STATUS_BAR         10999
/*
** General variable definitions for layout of icons
*/
#if (XVTWS == MACWS)
#define ICON_SIZE               32  /* the icon size used for events */
#define GRID_WIDTH              60  /* width of the invisable grid */
#define GRID_HEIGHT             75  /* height of the invisable grid */
#endif
#if (XVTWS == XOLWS)
#define ICON_SIZE               48  /* the icon size used for events */
#define GRID_WIDTH              80  /* width of the invisable grid */
#define GRID_HEIGHT             100 /* height of the invisable grid */
#endif
#if (XVTWS == WINWS) || (XVTWS == WIN32WS) || (XVTWS == WIN16WIN)
#define ICON_SIZE               32  /* the icon size used for events */
#define GRID_WIDTH              80  /* width of the invisable grid */
#define GRID_HEIGHT             100 /* height of the invisable grid */
#endif
#if (XVTWS == MTFWS)
#define ICON_SIZE               48  /* the icon size used for events */
#define GRID_WIDTH              80  /* width of the invisable grid */
#define GRID_HEIGHT             100 /* height of the invisable grid */
#endif
/*
**      Following typedefs are for object's shape and drawing tool.
*/
typedef enum {TL_POINTER, TL_STRATIGRAPHY,
                TL_FOLD, TL_FAULT, TL_UNCONFORMITY, TL_SHEAR, TL_DYKE,
                TL_PLUG, TL_STRAIN, TL_TILT, TL_FOLIATION, TL_LINEATION,
                TL_IMPORT, TL_STOP, TL_GENERIC} TOOL;
typedef enum { ALL_VALID, NO_STRATIGRAPHY } MENU_STATE;
typedef enum {NO_SETUP, NO_INSTRUCTS, INSTRUCTS} WIN_DATA_STATE;
/*
**      Data structure to hold information about each window.
*/
typedef struct {                        /* data for window */
        PNT origin;                     /* window upper left corner */
        PNT range;                              /* window width, height */
        OBJECT *head;                   /* object list */
        TOOL tool;                              /* current tool */
} WINDOW_INFO;


#endif

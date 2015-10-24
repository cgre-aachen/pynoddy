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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define XVTWS           100
#define MTFWS           100
#define TRUE            1
#define FALSE           0
#define BOOLEAN         short int
#define __huge		

typedef long           WINDOW;    /* window descriptor */
typedef long           PICTURE;   /* encapsulated picture descriptor */
typedef unsigned long  COLOR;     /* color encapsulation */
typedef unsigned long  EVENT_MASK;/* event delivery mask */
typedef unsigned long  GHANDLE;   /* handle to global memory block */
typedef short          MENU_TAG;  /* menu item tag */
typedef short          CURSOR;    /* cursor shape */
typedef short          TXEDIT;    /* Identifies a text-edit object */
typedef unsigned short T_CNUM;
typedef unsigned short T_CPOS;
typedef unsigned short T_LNUM;
typedef WINDOW XVT_PIXMAP;
typedef struct s_event {
	int type;
} EVENT;
typedef enum { HSCROLL, VSCROLL, HVSCROLL } SCROLL_TYPE;
typedef enum { RESP_DEFAULT, RESP_2, RESP_3 } ASK_RESPONSE;
typedef long (* EVENT_HANDLER) (WINDOW win, EVENT *ep);

typedef struct {
    char type[6];
    char name[200];
    char dir[200];
    char creator[6];
} FILE_SPEC;

typedef struct s_rct {          /* mathematical rectangle */
    short top;                  /* top coordinate */
    short left;                 /* left coordinate */
    short bottom;               /* bottom coordinate */
    short right;                /* right coordinate */
} RCT, *RCT_PTR;

typedef struct s_pnt {          /* mathematical point */
    short v;                    /* vertical (y) coordinate */
    short h;                    /* horizontal (x) coordinate */
} PNT, *PNT_PTR;

#define NULL_WIN	(WINDOW) NULL

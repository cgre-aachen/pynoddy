#ifndef XVT_H
#define XVT_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define FCN_NODDY_ONLY

#define XVTWS          1
#define XVT_CC_PROTO   1
#define GRID_WIDTH 10
#define GRID_HEIGHT 10
#define ICON_SIZE 10

#define CTL_FLAG_CHECKED 0
#define CTL_FLAG_INVISIBLE 0
#define SCREEN_WIN 0

#define DATA_PTR       (char *)
#define CORRECT_WIN_RESIZE(W,R)  
#define XVT_CC_ARG(T,N)  T N;
#define XVT_CC_LARG(T,N)  T N;

#define min(x,y)       ((x)<(y)?(x):(y))
#define max(x,y)       ((x)>(y)?(x):(y))
#define BOOLEAN        short int
#define TRUE           1
#define FALSE          0
#define __huge 
#define _huge 
#define far 
#define WindowPtr      int
#define XVT_IOSTREAM   FILE *
#define PTR_LONG       (long *)

typedef int            ICON_RESOURCE;
typedef long           WINDOW;      /* window descriptor */
typedef WINDOW         XVT_PIXMAP;
typedef WINDOW         XVT_IMAGE;
typedef int            XVT_PALETTE;
typedef long           PICTURE;     /* encapsulated picture descriptor */
typedef unsigned long  COLOR;  	    /* color encapsulation */
typedef unsigned long  EVENT_MASK;  /* event delivery mask */
typedef unsigned long  GHANDLE;     /* handle to global memory block */
typedef short          MENU_TAG;    /* menu item tag */
typedef short          CURSOR;      /* cursor shape */
typedef short          TXEDIT;      /* Identifies a text-edit object */
typedef unsigned short T_CNUM;
typedef unsigned short T_CPOS;
typedef unsigned short T_LNUM;
typedef unsigned short T_PNUM;
typedef int            XVT_HELP_INFO;
typedef char           SSTR;



/*---------------------------------------------------------------------------
        Cursors
---------------------------------------------------------------------------*/
#define CURSOR_ARROW    0   /* arrow */
#define CURSOR_IBEAM    1   /* I-beam */
#define CURSOR_CROSS    2   /* cross hair */
#define CURSOR_PLUS     3   /* plus sign (fatter than cross hair) */
#define CURSOR_WAIT     4   /* waiting symbol (e.g., hourglass) */
#define CURSOR_HELP     5   /* help system */
#define CURSOR_USER     11  /* user defined shape (>= 11) */

/*---------------------------------------------------------------------------
        Standard tool constants
---------------------------------------------------------------------------*/
#define TL_PEN_BLACK    1L
#define TL_PEN_HOLLOW   2L
#define TL_PEN_RUBBER   3L
#define TL_PEN_WHITE    4L

#define TL_BRUSH_BLACK  0L
#define TL_BRUSH_WHITE  1L

/*---------------------------------------------------------------------------
        Colors
---------------------------------------------------------------------------*/
#define COLOR_RED       0x01FF0000L
#define COLOR_GREEN     0x0200FF00L
#define COLOR_BLUE      0x030000FFL
#define COLOR_CYAN      0x0400FFFFL
#define COLOR_MAGENTA   0x05FF00FFL
#define COLOR_YELLOW    0x06FFFF00L
#define COLOR_BLACK     0x07000000L
#define COLOR_DKGRAY    0x08404040L
#define COLOR_GRAY      0x09808080L
#define COLOR_LTGRAY    0x0AC0C0C0L
#define COLOR_WHITE     0x0BFFFFFFL

#define COLOR_INVALID   ((COLOR)~0)


typedef struct {
   SSTR *rtype;   /* resource type (e.g., "ICON") */
   int   rid;     /* ID */
   char *data;    /* pointer to already-loaded data */
} RESOURCE_INFO;

#define ATTR_BASE                 0
#define ATTR_BACK_COLOR           (ATTR_BASE + 100)
#define XVT_FILE_ATTR_EXIST       1L
#define XVT_FILE_ATTR_READ        2L
#define XVT_FILE_ATTR_WRITE       3L
#define XVT_FILE_ATTR_EXECUTE     4L
#define XVT_FILE_ATTR_DIRECTORY   5L
#define XVT_FILE_ATTR_NUMLINKS    6L
#define XVT_FILE_ATTR_SIZE        7L
#define XVT_FILE_ATTR_ATIME       8L
#define XVT_FILE_ATTR_MTIME       9L
#define XVT_FILE_ATTR_CTIME      10L
#define XVT_FILE_ATTR_CREATORSTR 11L
#define XVT_FILE_ATTR_DIRSTR     12L
#define XVT_FILE_ATTR_FILESTR    13L
#define XVT_FILE_ATTR_TYPESTR    14L

#define NULL_WIN     (WINDOW) NULL
#define NULL_PIXMAP  (XVT_PIXMAP) NULL
#define TASK_WIN     0

#define WSF_NONE        0x00000000L
#define WSF_SIZE        0x00000001L /* is user sizeable */
#define WSF_CLOSE       0x00000002L /* is user closeable */
#define WSF_HSCROLL     0x00000004L /* has horz. scrolbar outside client area */
#define WSF_VSCROLL     0x00000008L /* has vert. scrolbar outside client area */
#define WSF_DECORATED   0x0000000FL /* all of above four flags */
#define WSF_INVISIBLE   0x00000010L /* is initially invisible */
#define WSF_DISABLED    0x00000020L /* is initially disabled */
#define WSF_FLOATING    0x00000040L /* is floating */
#define WSF_ICONIZABLE  0x00000080L
#define WSF_ICONIZED    0x00000100L /* is initially iconized */
#define WSF_SIZEONLY    0x00000200L /* lacks border rectangles (Mac only) */
#define WSF_NO_MENUBAR  0x00000800L /* has no menu bar of its own */
#define WSF_MAXIMIZED   0x00001000L /* initially maximized */

#define EM_NONE       ((EVENT_MASK)0L)
#define EM_ALL        ((EVENT_MASK)~0L)
#define EM_CREATE     ((EVENT_MASK)(1L << E_CREATE))
#define EM_DESTROY    ((EVENT_MASK)(1L << E_DESTROY))
#define EM_FOCUS      ((EVENT_MASK)(1L << E_FOCUS))
#define EM_SIZE       ((EVENT_MASK)(1L << E_SIZE))
#define EM_UPDATE     ((EVENT_MASK)(1L << E_UPDATE))
#define EM_CLOSE      ((EVENT_MASK)(1L << E_CLOSE))
#define EM_MOUSE_DOWN ((EVENT_MASK)(1L << E_MOUSE_DOWN))
#define EM_MOUSE_UP   ((EVENT_MASK)(1L << E_MOUSE_UP))
#define EM_MOUSE_MOVE ((EVENT_MASK)(1L << E_MOUSE_MOVE))
#define EM_MOUSE_DBL  ((EVENT_MASK)(1L << E_MOUSE_DBL))
#define EM_CHAR       ((EVENT_MASK)(1L << E_CHAR))
#define EM_VSCROLL    ((EVENT_MASK)(1L << E_VSCROLL))
#define EM_HSCROLL    ((EVENT_MASK)(1L << E_HSCROLL))
#define EM_COMMAND    ((EVENT_MASK)(1L << E_COMMAND))
#define EM_FONT       ((EVENT_MASK)(1L << E_FONT))
#define EM_CONTROL    ((EVENT_MASK)(1L << E_CONTROL))
#define EM_TIMER      ((EVENT_MASK)(1L << E_TIMER))
#define EM_QUIT       ((EVENT_MASK)(1L << E_QUIT))
#define EM_HELP       ((EVENT_MASK)(1L << E_HELP))
#define M_HELP        ((EVENT_MASK)(1L << E_HELP))
#define EM_USER       ((EVENT_MASK)(1L << E_USER))

#define ATTR_HELP_HOOK     (ATTR_BASE + 710)
#define ATTR_HELP_CONTEXT  (ATTR_BASE + 711)

typedef unsigned long XVT_ERRID;     /* Error Message Identifier */
 
typedef struct {long* unsupp;}  *XVT_ERRMSG;    /* Error Message Object handle*/

typedef enum {
   XVT_IMAGE_NONE,
   XVT_IMAGE_CL8,
   XVT_IMAGE_RGB,
   XVT_IMAGE_MONO
} XVT_IMAGE_FORMAT;

typedef enum {
   XVT_PALETTE_NONE,
   XVT_PALETTE_STOCK,
   XVT_PALETTE_CURRENT,
   XVT_PALETTE_CUBE16,
   XVT_PALETTE_CUBE256,
   XVT_PALETTE_USER
} XVT_PALETTE_TYPE;

typedef enum {
   XVT_PIXMAP_NONE,
   XVT_PIXMAP_DEFAULT
} XVT_PIXMAP_FORMAT;
 
typedef struct s_xvt_config {
   short menu_bar_ID;      /* menu bar resource ID */
   short about_box_ID;     /* about box resource ID */
   SSTR *base_appl_name;   /* application's "file" name */
   SSTR *appl_name;        /* application's name */
   SSTR *taskwin_title;    /* title for task window */
} XVT_CONFIG;

typedef enum e_pat {
   PAT_NONE,
   PAT_HOLLOW,
   PAT_SOLID,
   PAT_HORZ,
   PAT_VERT,
   PAT_FDIAG,
   PAT_BDIAG,
   PAT_CROSS,
   PAT_DIAGCROSS,
   PAT_RUBBER,
   PAT_SPECIAL
} PAT_STYLE;

typedef enum e_pen_style {      /* pen style (must be P_SOLID) */
    P_SOLID,                    /* solid */
    P_DOT,                      /* dotted line */
    P_DASH                     /* dashed line */
} PEN_STYLE;

typedef struct s_cbrush {
    PAT_STYLE pat;
    COLOR color;
} CBRUSH;

typedef struct s_cpen {
    short width;
    PAT_STYLE pat;
    PEN_STYLE style;
    COLOR color;
} CPEN;

typedef enum e_mode {           /* drawing (transfer) mode */
    M_COPY,                     /*   patCopy (Mac),    R2_COPYPEN (Win) */
    M_OR,                       /*   patOr,            R2_MASKPEN */
    M_XOR,                      /*   patXor,           R2_NOTXORPEN */
    M_CLEAR,                    /*   patBic,           R2_MERGENOTPEN */
    M_NOT_COPY,                 /*   notPatCopy,       R2_NOTCOPYPEN */
    M_NOT_OR,                   /*   notPatOr,         R2_MASKNOTPEN */
    M_NOT_XOR,                  /*   notPatXor,        R2_XORPEN */
    M_NOT_CLEAR                /*   notPatBic,        R2_MERGEPEN */
} DRAW_MODE;

typedef struct s_drawct {       /* R4 set of drawing tools */
    CPEN pen;                   /* color pen */
    CBRUSH brush;               /* color brush */
    DRAW_MODE mode;             /* drawing mode */
    COLOR fore_color;           /* foreground color */
    COLOR back_color;           /* background color */
    BOOLEAN opaque_text;        /* is text drawn opaquely? */
} DRAW_CTOOLS;

typedef enum e_eol {            /* terminator found by find_eol fcn */
    EOL_NORMAL,                 /* normal (or first) line terminator */
    EOL_DIFF,                   /* terminator different from previous */
    EOL_NONE                   /* end of buf before any terminator */
} EOL_FORMAT, *EOL_FORMAT_PTR;

typedef enum e_access {
    A_LOCK, 
    A_GET, 
    A_UNLOCK 
} ACCESS_CMD;

typedef enum e_ask_resp {       /* response from ask fcn */
    RESP_DEFAULT,               /* default button */
    RESP_2,                     /* second button */
    RESP_3                     /* third button */
} ASK_RESPONSE;

typedef enum e_cb {             /* standard clipboard format */
   CB_TEXT,                    /* ASCII text */
   CB_PICT,                    /* encapsulated picture */
   CB_APPL,                    /* app's own type (must have name) */
   CB_PIXMAP
} CB_FORMAT;

typedef struct s_dir {         /* DIRECTORY - directory descriptor */
   char path[100];
} DIRECTORY, *DIRECTORY_PTR;

typedef enum e_file {           /* result from file open & save dialogs */
   FL_BAD,                     /* error occurred */
   FL_CANCEL,                  /* cancel button clicked */
   FL_OK                      /* OK button clicked */
} FL_STATUS;


typedef struct s_pnt {          /* mathematical point */
   short v;                    /* vertical (y) coordinate */
   short h;                    /* horizontal (x) coordinate */
} PNT, *PNT_PTR;

typedef struct s_printrcd {     /* print record (holds setup) */
    char data;
} PRINT_RCD, *PRINT_RCD_PTR;    /* structure to make type unique */

typedef struct s_rct {          /* mathematical rectangle */
    short top;                  /* top coordinate */
    short left;                 /* left coordinate */
    short bottom;               /* bottom coordinate */
    short right;                /* right coordinate */
} RCT, *RCT_PTR;

typedef enum e_scroll_ctl {     /* site of scroll bar activity */
    SC_NONE,                    /* nowhere (event should be ignored) */
    SC_LINE_UP,                 /* one line up */
    SC_LINE_DOWN,               /* one line down */
    SC_PAGE_UP,                 /* previous page */
    SC_PAGE_DOWN,               /* next page */
    SC_THUMB,                   /* thumb repositioning */
    SC_THUMBTRACK              /* dynamic thumb tracking */
} SCROLL_CONTROL;

typedef enum e_scroll_type {    /* type of scroll bar */
    HSCROLL,                    /* horizontal */
    VSCROLL,                    /* vertical */
    HVSCROLL                   /* either (used for dlg box ctls) */
} SCROLL_TYPE;

typedef struct s_slist {        /* string list */
    short dummy;
} *SLIST;

typedef struct s_slistelt {     /* element of string list */
    short dummy;
} *SLIST_ELT;

typedef struct s_filespec {     /* file specification */
   DIRECTORY dir;              /* directory */
   char type[6];               /* file type or extension */
   char name[101];    /* file name (may be partial path) */
   char creator[6];            /* file creator */
} FILE_SPEC, *FILE_SPEC_PTR;

typedef enum _event_type {
   E_CREATE,        /* creation */
   E_DESTROY,       /* destruction */
   E_FOCUS,         /* window focus gain/loss */
   E_SIZE,          /* resize */
   E_UPDATE,        /* update */
   E_CLOSE,         /* close window request */
   E_MOUSE_DOWN,    /* mouse down */
   E_MOUSE_UP,      /* mouse up */
   E_MOUSE_MOVE,    /* mouse move */
   E_MOUSE_DBL,     /* mouse double click */
   E_CHAR,          /* character typed */
   E_VSCROLL,       /* horz. window scrollbar activity */
   E_HSCROLL,       /* vert. window scrollbar activity */
   E_COMMAND,       /* menu command */
   E_FONT,          /* font menu selection */
   E_CONTROL,       /* control activity */
   E_TIMER,         /* timer */
   E_QUIT,          /* application shutdown request */
   E_HELP,          /* help invoked */
   E_USER           /* user defined */
} EVENT_TYPE;

typedef enum e_win_type {       /* type of window */
   W_NONE,          /* marker for end of WIN_DEF array */
   W_DOC,           /* document window */
   W_PLAIN,         /* window with plain border */
   W_DBL,           /* window with double border */
   W_PRINT,         /* XVT internal use only */
   W_TASK,          /* task window */
   W_SCREEN,        /* screen window */
   W_NO_BORDER,     /* no border */
   W_PIXMAP,        /* pixmap */
   WD_MODAL,        /* modal dialog */
   WD_MODELESS,     /* modeless dialog */
   WC_PUSHBUTTON,   /* button control */
   WC_RADIOBUTTON,  /* radio button control */
   WC_CHECKBOX,     /* check box control */
   WC_HSCROLL,      /* horizontal scroll bar control */
   WC_VSCROLL,      /* vertical scroll bar control */
   WC_EDIT,         /* edit control */
   WC_TEXT,         /* static text control */
   WC_LBOX,         /* list box control */
   WC_LISTBUTTON,   /* button with list */
   WC_LISTEDIT,     /* edit field with list */
   WC_GROUPBOX,     /* group box */
   WO_TE,           /* text-edit object */
   WC_ICON,         /* icon control */
   WC_IMAGEBUTTON,  /* image button */
   WC_IMAGEICON,    /* image icon (i.e.  portable icon) */
   WC_NUM_WIN_TYPES /* number of WIN_TYPE's */
} WIN_TYPE;

typedef struct s_ctlinfo {     /* info passed with E_CONTROL event*/
   WIN_TYPE type;              /* WC_* control type */
   WINDOW win;                 /* WINDOW id of the control being operated */
   union {
      struct s_pushbutton {
         int reserved;            /* Reserved...no usage yet.*/
      } pushbutton;
      struct s_radiobutton {
         int reserved;            /* Reserved...no usage yet.*/
      } radiobutton;
      struct s_checkbox {
         int reserved;            /* Reserved...no usage yet.*/
      } checkbox;
      struct s_scroll {           /* scroll bar action */
         SCROLL_CONTROL what;     /* site of activity */
         short pos;               /* thumb position */
      } scroll;
      struct s_edit {
         BOOLEAN focus_change;    /* is event a focus change? */
         BOOLEAN active;          /* if so: gaining focus? (vs.  losing) */
      } edit;
      struct s_statictext {
         int reserved;            /* Reserved...no usage yet.*/
      } statictext;
      struct s_lbox {              /* list box action */
         BOOLEAN dbl_click;       /* double click (vs. single)? */
      } lbox;
      struct s_listbutton {
         int reserved;            /* Reserved...no usage yet.*/
      } listbutton;
      struct s_listedit {
         BOOLEAN focus_change;    /* Did the edit field part change focus? */
         BOOLEAN active;          /* If so, focus gained (vs lost)? */
      } listedit;
      struct s_groupbox {
         int reserved;            /* Reserved...no usage yet.*/
      } groupbox;
      struct s_icon {
         int reserved;            /* Reserved...no usage yet.*/
      } icon;
      struct s_chr {
         /* defunct -- only for R2 compatibility layer */
         short ch;
         BOOLEAN shift;
         BOOLEAN control;
      } chr;
   } v;
} CONTROL_INFO, *CONTROL_INFO_PTR;


typedef struct s_event {
   EVENT_TYPE type;
   union {
      struct s_mouse {          /* E_MOUSE_DOWN, E_MOUSE_UP, E_MOUSE_MOVE,
                                   E_MOUSE_DBL */
          PNT where;            /* location of event (window relative) */
          BOOLEAN shift;        /* shift key down? */
          BOOLEAN control;      /* control or option key down? */
          short button;         /* button number */
      } mouse;
      struct s_char {           /* E_CHAR */
          short ch;             /* character */
          BOOLEAN shift;        /* shift key down? */
          BOOLEAN control;      /* control or option key down? */
      } chr;
      BOOLEAN active;           /* E_FOCUS: activation? (vs.  deactivation) */
      BOOLEAN query;            /* E_QUIT: query only? (app calls quit_OK) */
      struct s_scroll_info {    /* E_VSCROLL, E_HSCROLL */
          SCROLL_CONTROL what;  /* site of activity */
          short pos;            /* thumb position, if SC_THUMB */
      } scroll;
      struct s_cmd {            /* E_COMMAND */
          MENU_TAG tag;         /* menu item tag */
          BOOLEAN shift;        /* shift key? */
          BOOLEAN control;      /* control or option key? */
      } cmd;
      struct s_size {           /* E_SIZE */
          short height;         /* new height */
          short width;          /* new width */
      } size;
      struct s_efont {          /* E_FONT */
          int font_id;    /* R4 font id of selected font */
              } font;
                                                                /* Do NOT use the following 2 definitions */
              struct s_efont_r4 {        /* R3/R4 conversion module def initions */
          int font_id;    /* R4 font id of selected font */
      } font_R4;
      struct s_efont_r3 {       /* R3/R4 conversion module definitions */
          int font;         /* R3 - selected font */
          int part;    /* R3 - part that changed */
              } font_R3;
      struct s_ctl {            /* E_CONTROL */
          short id;             /* control's ID */
          CONTROL_INFO ci;      /* control info */
      } ctl;
      struct s_update {         /* E_UPDATE */
          RCT rct;              /* update rectangle */
      } update;
      struct s_timer {          /* E_TIMER */
          long id;              /* timer ID */
      } timer;
      struct s_user {           /* E_USER */
          long id;              /* application ID */
          void *ptr;            /* application pointer */
      } user;
      struct s_help {           /* E_HELP */
          WINDOW obj;           /* help for control, window, dialog */
          MENU_TAG tag;         /* help for menu item */
          int  tid;    /* predefined help topic */
      } help;
   } v;
} EVENT, *EVENT_PTR;

typedef long (* EVENT_HANDLER) (WINDOW, EVENT *);

typedef struct s_mitem {
   int tag;                /* bad alignment, but this struc ture */
   SSTR *text;                  /* may be statically intialized by app */
   short mkey;
   unsigned enabled:1;
   unsigned checked:1;
   unsigned checkable:1;
   unsigned separator:1;
   unsigned:0;
   struct s_mitem *child;
   int def_item;
} MENU_ITEM;

typedef struct s_win_def {
    WIN_TYPE  wtype;
    RCT       rct;
    SSTR     *text;
    int units;
    union {
        struct {
            short int menu_rid;
            MENU_ITEM *menu_p;
            long flags;
        } win;
        struct {
            long flags;
        } dlg;
        struct {
            short int ctrl_id;
            short int icon_id;
            long flags;
        } ctl;
        struct {
            unsigned short attrib;
            int font_id;
            short margin;
            short limit;
            short int tx_id;
                } tx;
        struct {              /* Do NOT even dream abo ut using this */
            unsigned short attrib;
            int font_id;
            short margin;
            short limit;
            short int tx_id;
        } tx_R4;              /* For the R3/R4 convers ion module */
        struct {              /* Do NOT even dream abo ut using this */
            unsigned short attrib;
            int font;
            short margin;
            short limit;
            short int tx_id;
        } tx_R3;              /* For the R3/R4 convers ion module */
    } v;
} WIN_DEF;


#define ATTR_TITLE_HEIGHT          0
#define ATTR_DOCFRAME_WIDTH        0
#define ATTR_DOCFRAME_HEIGHT       0
#define ATTR_DOC_STAGGER_HORZ      0
#define ATTR_DOC_STAGGER_VERT      0
#define ATTR_X_PLACE_WINDOW_EXACT  0
#define ATTR_MAC_NO_SELECT_WINDOW  0
#define ATTR_WIN_PM_NO_TWIN        0
#define SEV_FATAL                  0
#define ERR_ASSERT_4               0
#define ATTR_SCREEN_WIDTH          0
#define ATTR_SCREEN_HEIGHT         0
#define ATTR_NATIVE_WINDOW         0

#define CB_APPL                    0

/*
* Reserved help topic IDs
*/
#define XVT_TPC_BASE             32000
#define XVT_TPC_HELPONHELP       (XVT_TPC_BASE +  0)
#define XVT_TPC_INDEX            (XVT_TPC_BASE +  1)
#define XVT_TPC_TUTORIAL         (XVT_TPC_BASE +  2)
#define XVT_TPC_BASICSKILLS      (XVT_TPC_BASE +  3)
#define XVT_TPC_PROCEDURES       (XVT_TPC_BASE +  4)
#define XVT_TPC_KEYBOARD         (XVT_TPC_BASE +  5)
#define XVT_TPC_CONTENTS         (XVT_TPC_BASE +  6)
#define XVT_TPC_ABOUT            (XVT_TPC_BASE +  7)
#define XVT_TPC_COMMANDS         (XVT_TPC_BASE +  8)
#define XVT_TPC_GLOSSARY         (XVT_TPC_BASE +  9)
#define XVT_TPC_ABOUTHELP        (XVT_TPC_BASE + 10)



                  /* ************************************************ */
                  /* ************* OVER RIDE FUNCTIONS ************** */
                  /* ************************************************ */
#define xvt_dm_post_error(T)               fprintf(stderr,T,"\n")
#define xvt_mem_alloc(X)                   (memset(malloc(X), 0, X))
#define xvt_mem_zalloc(X)                  (memset(malloc(X), 0, X))
#define xvt_mem_free(P)                    free(P)
#define xvt_mem_halloc(X,Y)                (memset(malloc((X)*(Y)), 0, (X)*(Y)))
#define xvt_mem_zhalloc(X,Y)               (memset(malloc((X)*(Y)), 0, (X)*(Y)))
#define xvt_mem_hfree(P)                   free(P)
#define xvt_vobj_set_attr(P,L,I)           0
#define xvt_vobj_destroy(P)                0
#define xvt_app_create(P,A,B,C,D)          0
#define xvt_vobj_get_attr(P,L)             0  
#define xvt_errmsg_sig_if(P,A,B,C,D,E,F)   0
#define xvt_win_create(P,A,B,C,E,F,L,I,U)  0
#define MaxMem(P)                          0
#define xvt_app_destroy()                  0
#define xvt_app_allow_quit()               ;     
#define xvt_dm_post_ask(P,A,B,C)           0
#define xvt_win_get_ctl(P,C)               0
#define xvt_ctl_check_radio_button(P,B,I)         
#define xvt_fsys_get_default_dir(P)        strcpy ((*(P)).path, ".")      
#define xvt_help_open_helpfile(P,L)        0
#define xvt_dm_post_file_open(P,B)         0
#define xvt_fsys_get_file_attr(F,P)        0
#define xvt_dm_post_note(P)                0
#define xvt_help_process_event(P,A,B)      0
#define xvt_help_close_helpfile(P)         0
#define xvt_mem_realloc(P,S)               0
#define xvt_help_set_win_assoc(P,A,B,C)    0
#define xvt_help_set_menu_assoc(P,A,B,C,D) 0
#define xvt_list_suspend(P)                0
#define xvt_list_add(P,I,T)                0
#define xvt_list_resume(P)                 0
#define xvt_vobj_get_data(P)               0
#define xvt_dwin_clear(W,P)                0
#define xvt_win_dispatch_event(P,E)        0
#define xvt_ctl_set_checked(P,I)           0
#define xvt_ctl_is_checked(P)              0
#define xvt_help_display_topic(P,T)        0
#define xvt_list_get_first_sel(P,T,I)      0
#define xvt_sbar_get_pos(W,P)              0
#define xvt_tx_process_event(W,E)          0
#define xvt_vobj_get_title(P,T,I)          0
#define xvt_fsys_set_dir(P)                0
#define xvt_vobj_set_enabled(P,I)          0
#define xvt_vobj_get_client_rect(P,R)      0
#define xvt_sbar_set_range(P,A,M,X)        0
#define xvt_sbar_set_pos(P,A,B)            0
#define xvt_dwin_scroll_rect(P,A,B,C)      0
#define xvt_vobj_set_title(P,T)            0
#define xvt_list_get_sel_index(P)          0
#define xvt_dm_post_file_save(P,T)         0
#define xvt_fsys_set_file_attr(F,A,L)      0
#define xvt_dlg_create_res(A,B,C,D,E)      0
#define xvt_app_get_default_ctools(P)      0
#define xvt_dwin_set_draw_ctools(W,T)      0
#define xvt_dwin_get_font_metrics(A,B,C,D) 0
#define xvt_dwin_set_fore_color(P,C)       0
#define xvt_dwin_is_update_needed(P,R)     0
#define xvt_dwin_set_clip(P,R)             0
#define xvt_dwin_invalidate_rect(P,R)      0
#define xvt_vobj_set_data(W,P)             0
#define xvt_dwin_get_font_size(P)          0
#define xvt_res_get_win_def(P)             0
#define xvt_win_create_def(P,A,B,C,D)      0
#define xvt_res_free_win_def(P)            0
#define xvt_vobj_set_visible(P,T)          0 
#define xvt_res_get_dlg_def(P)             0
#define xvt_dlg_create_def(P,A,B,C)        0
#define xvt_rect_set(P,X,Y,W,H)            0
#define xvt_rect_has_point(P,R)            0
#define xvt_rect_offset(P,X,Y)             0
#define xvt_dwin_draw_set_pos(W,P)         0
#define xvt_dwin_draw_line(W,P)            0
#define xvt_dwin_draw_aline(w,P,S,E)       0
#define xvt_dwin_update(P)                 0
#define xvt_dwin_get_draw_ctools(W,P)      0
#define xvt_dwin_draw_rect(W,P)            0
#define xvt_vobj_translate_points(W,A,P,I) 0
#define xvt_vobj_get_parent(W)             0
#define xvt_rect_set_pos(P,A)              0
#define xvt_vobj_move(W,P)    
#define xvt_win_set_cursor(W,P)            0
#define xvt_dwin_draw_roundrect(P,R,W,H)   0
#define xvt_dwin_get_text_width(P,S,L)     0
#define xvt_dwin_draw_text(W,X,Y,T,P)      0
#define xvt_dwin_draw_icon(P,X,Y,I)        0
#define xvt_win_trap_pointer(P)            0
#define xvt_win_release_pointer()          0
#define xvt_vobj_get_outer_rect(P,R)       0
#define xvt_app_get_files_count(P,C)       0
#define xvt_app_get_file()                 0
#define xvt_app_set_file_processed()       0
#define xvt_xres_build_icon(P)             0
#define xvt_scr_list_wins()                0
#define xvt_slist_get_first(P)             0
#define xvt_slist_get_next(P,E)            0
#define xvt_slist_get(P,E,L)               0
#define xvt_slist_destroy(P)               0
#define xvt_menu_update(P)                 0
#define xvt_menu_set_item_enabled(P,T,I)   0
#define xvt_menu_get_tree(W)               0
#define xvt_slist_count(S)                 0
#define xvt_menu_set_tree(P,T)             0
#define xvt_res_free_menu_tree(T)          0
#define xvt_rect_get_width(R)              0
#define xvt_rect_get_height(R)             0
#define xvt_print_create(P)                0
#define xvt_dm_post_page_setup(P)          0
#define xvt_print_create_win(P,T)          0
#define xvt_print_open_page(P)             0
#define xvt_print_get_next_band()          0
#define xvt_dwin_draw_pmap(W,P,A,B)        0
#define xvt_print_close_page(P)            0
#define xvt_cb_open(P)                     0
#define xvt_cb_get_data(P,A,B)             0
#define xvt_cb_close()                     0
#define xvt_cb_alloc_data(P)               0
#define xvt_cb_put_data(P,A,B,C)           0
#define xvt_cb_free_data()                 0
#define xvt_res_get_menu(P)                0
#define xvt_list_clear(P)                  0
#define xvt_sbar_get_range(P,T,M,X)        0
#define xvt_list_set_sel(P,A,B)            0
#define xvt_list_count_all(P)              0
#define xvt_list_is_sel(P,I)               0
#define xvt_tx_add_par(P,I,A)              0
#define xvt_app_process_pending_events()   0
#define xvt_slist_create()                 0
#define xvt_slist_add_sorted(P,T,A,B,C)    0
#define XVT_MAKE_COLOR(R,G,B)              0
#define xvt_pmap_create(P,A,B,C,D)         0
#define xvt_pmap_destroy(P)                0
#define xvt_image_create(P,A,B,C)          0
#define xvt_palet_create(P,B)              0
#define xvt_image_destroy(P)               0
#define xvt_palet_set_tolerance(P,T)       0
#define xvt_palet_add_colors(P,L,I)        0
#define xvt_vobj_set_palet(W,P)            0
#define xvt_image_set_pixel(P,X,Y,C)       0
#define xvt_dwin_draw_image(W,P,A,B)       0
#define xvt_palet_destroy(P)               0
#define xvt_image_read(P)                  0
#define xvt_image_get_dimensions(P,W,H)    0
#define xvt_palet_get_colors(P,C,I)        0
#define xvt_image_set_ncolors(P,N)         0
#define xvt_image_set_clut(P,I,C)          0
#define xvt_image_get_from_pmap(I,P,S,D)   0
#define xvt_iostr_create_fwrite(P)         0
#define xvt_image_write_macpict_to_iostr(P) 0
#define xvt_iostr_destroy(P)               0
#define xvt_fsys_get_dir(P)                strcpy((*(P)).path, ".")
#define xvt_print_start_thread(P,M)        0
#define hfree(P)    
#define xvt_dwin_set_back_color(W,C)       0
#define xvt_menu_set_item_checked(W,I,B)   0
#define xvt_timer_destroy(P)               0
#define xvt_timer_create(W,T)              0
#define xvt_dwin_set_draw_mode(W,M)        0
#define xvt_dwin_set_std_cpen(P,T)         0
#define xvt_dwin_draw_oval(W,R)            0
#define xvt_dwin_set_font_size(W,S)        0
#define xvt_dwin_set_cpen(W,P)             0
#define xvt_dwin_set_cbrush(W,P)           0
#define xvt_dwin_draw_polygon(W,P,I)       0
#define XVT_COLOR_GET_RED(P)               0
#define XVT_COLOR_GET_GREEN(P)             0
#define XVT_COLOR_GET_BLUE(P)              0
#define halloc(P)                          0
#define xvt_fsys_convert_dir_to_str(P, Q, W)    strcpy(W, (*(P)).path)
#define xvt_image_get_pixel(P,X,Y)         0
#define xvt_rect_intersect(P,A,B)          0
#define SelectWindow(P)                    0
#define xvt_fsys_set_dir_startup()
#define xvt_vobj_raise(W)
#define xvt_scr_set_focus_vobj(W) 
#define xvt_ctl_create(A,B,C,D,E,F,G)      0
#define statbar_create(A,B,C,D,E,F,G,H,I,J,K)   0
#define xvt_dwin_draw_polyline(W,P,I)      0
#define xvt_vobj_get_type(W)               0
#define xvt_win_get_tx(W,I)                0
#define xvt_tx_suspend(W)                  0
#define xvt_tx_clear(W)                    0
#define xvt_tx_resume(W)                   0
#define xvt_fsys_rem_file(F)               0
#define xvt_image_write_bmp_to_iostr(I,S)  0
#define xvt_rect_set_width(R, W)           0
#define xvtcm_eh_start(W,E)
#define xvtcm_eh_end(W,E)
#define xvtcm_create(A,B,C,D,E,F,G,H,I,J,K,L,M)   0
#define xvtcm_set_src_char_size(P)
//#define stricmp(A,B)                       strcmp(A,B)
#define xvt_dm_post_string_prompt(A,B,C)   NULL
#define xvtcm_parent_event(W,E)    
#define xvt_vobj_get_palet(W)              0

#define NO_REF(x)                          
#define XVT_CC_ARGS(X)  ()
#define XVT_CALLCONV1            
#define XVT_CALLCONV2            

#define xvt_fsys_convert_str_to_dir(X,Y) strcpy((*(Y)).path, (X))

#endif

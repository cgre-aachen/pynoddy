#include "xvt.h"
#include "xvtcm.h"
#include "graph.h"
#include "noddy.h"
#include "nodInc.h"
                  
extern WINDOW lastActiveWindow;
                  
#if XVT_CC_PROTO
extern PROFILE_POINT *profilePointSelect (WINDOW, PNT *);
extern int profilePointMove (WINDOW, PROFILE_POINT *, PNT *);
extern int profilePointAdd (WINDOW, PNT *);
extern int updateProfileLocation (WINDOW, PROFILE_OPTIONS *, PNT *);
#else
extern PROFILE_POINT *profilePointSelect ();
extern int profilePointMove ();
extern int profilePointAdd ();
extern int updateProfileLocation ();
#endif

#if XVT_CC_PROTO
PROFILE_OPTIONS *getCurrentProfileOptions (WINDOW, PROFILE_OPTIONS *);
#else
PROFILE_OPTIONS *getCurrentProfileOptions ();
#endif
                  
/*
    Event handler.
*/
static long 
graph_eh(win, ep)
WINDOW win;
EVENT * ep;
{
    static PROFILE_POINT *dragPoint = NULL;
    xvtcm_eh_start(win, ep);

    switch (ep->type) {
    case E_CREATE:
       {
        PROFILE_OPTIONS *profOptions;
        OBJECT *object;
        
        object = (OBJECT *) xvt_vobj_get_data (xvt_vobj_get_parent(win));
        profOptions = (PROFILE_OPTIONS *) getProfileOptionsStructure(object);
        lastActiveWindow = win;        

        xvt_vobj_set_data (win, (long) profOptions);
        break;
       }
    case E_SIZE:
        break;
    case E_UPDATE:
        DrawProfileFrame (win, getCurrentProfileOptions(win,
                (PROFILE_OPTIONS *) xvt_vobj_get_data(win)));
        DrawProfileGraph (win, getCurrentProfileOptions(win,
                (PROFILE_OPTIONS *) xvt_vobj_get_data(win)));
        break;
    case E_FOCUS:
        break;
    case E_CHAR:
        break;
    case E_MOUSE_DBL:
        break;
    case E_MOUSE_MOVE:
        updateProfileLocation (win, getCurrentProfileOptions(win,
                      (PROFILE_OPTIONS *) xvt_vobj_get_data(win)),
                                            &(ep->v.mouse.where));
        if (dragPoint)
        {
           profilePointDrawMoving (win, dragPoint);
           profilePointMove (win, dragPoint, &(ep->v.mouse.where));
           profilePointDrawMoving (win, dragPoint);
        }
        break;
    case E_MOUSE_DOWN:
        {
           xvt_win_trap_pointer(win);
           if (!(dragPoint = (PROFILE_POINT *) profilePointSelect (win, &(ep->v.mouse.where))))
           {
              if (profilePointAdd (win, &(ep->v.mouse.where)))
              {
                 dragPoint = (PROFILE_POINT *) profilePointSelect (win, &(ep->v.mouse.where));
                 profilePointDrawMoving (win, dragPoint);
              }
              else
                 dragPoint = (PROFILE_POINT *) NULL;
           }
           else
              profilePointDrawMoving (win, dragPoint);
        }
        break;
    case E_MOUSE_UP:
        dragPoint = (PROFILE_POINT *) NULL;
        xvt_win_release_pointer();
        xvt_dwin_invalidate_rect (win, (RCT *) NULL);
        xvt_dwin_update(win);
        break;
    case E_DESTROY:
        lastActiveWindow = NULL_WIN;
        break;
    }
    xvtcm_eh_end(win, ep);
    return 0L;
}

static void 
set_creation_args (a)
long *a;
{
    /*
        Change any args prior to window creation. See "xvtcm.h" for the
        members in WIN_CREATION_ARGS.
    */
    //mwj_hack NOREF(a);
}

/*
    Creation function. Normally, will just call xvtcm_create. Doing argument
    adjustments in set_creation_args ensures that they will be set from both
    C and C++.
*/
WINDOW
graph_create (cid, left, top, right, bottom, prop_count, prop_list, parent_win, parent_rid, parent_flags, parent_class)
int cid;
int left;
int top;
int right;
int bottom;
int prop_count;
char ** prop_list;
WINDOW parent_win;
int parent_rid;
long parent_flags;
char * parent_class;
{
    return xvtcm_create(cid, left, top, right, bottom, prop_count,
      prop_list, parent_win, parent_rid, parent_flags, parent_class,
      graph_eh, set_creation_args);
}


PROFILE_OPTIONS *
#if XVT_CC_PROTO
getCurrentProfileOptions (WINDOW win, PROFILE_OPTIONS *firstProfOptions)
#else
getCurrentProfileOptions (win, firstProfOptions)
WINDOW win;
PROFILE_OPTIONS *firstProfOptions;
#endif
{
   PROFILE_OPTIONS *profOptions = NULL;
   WINDOW listWin;
   char graphName[PROFILE_TEXT_LENGTH];
   
             /* match the graph selected with the options structure */
   if (!(listWin = xvt_win_get_ctl (win, PROFILE_CURRENT_GRAPH)))
      listWin = xvt_win_get_ctl (xvt_vobj_get_parent(win),
                                 PROFILE_CURRENT_GRAPH);
   
   if (listWin)
   {
      xvt_list_get_first_sel (listWin, graphName, PROFILE_TEXT_LENGTH);
      for (profOptions = firstProfOptions; 
               profOptions && (strcmp(profOptions->name, graphName) != 0);
                               profOptions = profOptions->next)
         ; /* match the current Graph */
   }
   
   if (!profOptions)
      profOptions = firstProfOptions;

   return (profOptions);
}

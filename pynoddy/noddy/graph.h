#ifndef GRAPH_INCL
#define GRAPH_INCL
/*
      Copyright 1994 XVT Software Inc. All rights reserved.
      May be used freely by licensed and registered users of XVT.
      May be distributed in source form only when embedded in an
      XVT user's application.
 */

/*
    Template for Type 1 custom controls for C to be used with
    XVT-Design (2.01 or later).

    See the document "Using and Implementing Type 1 Custom Controls."

    In addition to its role as a template, this header and the companion
    implementation "graph.c" also implement a button control whose class
    is "graph".
*/

#include "xvtcm.h"

/*
    Symbols for notices.
*/
#define N_GRAPH_NOTICE         101

typedef struct {
    XVTCM_CONTROL_INFO ci;      /* required as first member */
    short btn_number;             /* replace with your notice data */
} GRAPH_DATA;

/*
    Function to create the control, called automatically by XVT-Design. May
    also be called explicitly.
*/
WINDOW graph_create XVT_CC_ARGS((int cid, int left, int top, int right,
  int bottom, int prop_count, char **prop_list, WINDOW parent_win,
  int parent_rid, long parent_flags, char *parent_class));

#endif /* GRAPH_INCL */
/* End */

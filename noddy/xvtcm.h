#ifndef XVTCM_INCL
#define XVTCM_INCL


#define XVTCM_GET_INFO(ep)      (XVTCM_CONTROL_INFO *)(ep->v.user.ptr)
#define XVTCM_GET_INFOPP(d)     (XVTCM_CONTROL_INFO *)(d)
#define XVTCM_GET_ID(ep)        (short)ep->v.user.id

typedef struct s_XVTCM_CONTROL_INFO {
    int type;                  /* dud */
} XVTCM_CONTROL_INFO;

#endif



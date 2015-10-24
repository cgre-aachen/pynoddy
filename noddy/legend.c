#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
#include "titles.h"
#include "time.h"

#define DEBUG(X)    
#define XVT_FS_BOLD 1
#define XVT_FS_NONE 1
#define XVT_FS_ITALIC 1

                                /* Constants to this file */
#define TOLERANCE   0.001
#define RATIO_TOLERANCE       0.0200
/*
     Geometrical constants.
*/
#define PAGE_INCR        25        /* "line" increment for scrolling */
#define MIN_SIZE         5        /* min. size for object */
#define MIN_MOVE         2         /* min. distance for moves */

extern COLOR backgroundColor;
extern PROJECT_OPTIONS projectOptions;
extern WINDOW lastActiveWindow;

WINDOW legendWindow = NULL_WIN;
BOOLEAN legendShowing = FALSE;
XVT_PIXMAP legendPixmap = NULL_PIXMAP;

                                /* External variables / functions to this file */
#if XVT_CC_PROTO
extern int getAnomImagePointFromLoc (ANOMIMAGE_DATA *, double, double, double, int *, int *);
extern int DrawStereoNet (WINDOW, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS], int);
#else
extern int getAnomImagePointFromLoc ();
extern int DrawStereoNet ();
#endif


#if XVT_CC_PROTO
int showLegendWindow (int);
int toggleLegendWindowDisplay ();
WINDOW createLegendWindow ();
int destroyLegendWindow ();
WINDOW getLegendWindow ();
int drawLegend (WINDOW, RCT *);
int freeDrawData (DRAW_DATA *);
int updateLegendData (WINDOW, LEGEND_TYPES);
XVT_PIXMAP getLegendPixmap ();
int overlayAnomGrid (WINDOW, ANOMIMAGE_DATA *);
long legendEventHandler (WINDOW, EVENT *);
#else
int showLegendWindow ();
int toggleLegendWindowDisplay ();
WINDOW createLegendWindow ();
int destroyLegendWindow ();
WINDOW getLegendWindow ();
int drawLegend ();
int freeDrawData ();
int updateLegendData ();
XVT_PIXMAP getLegendPixmap ();
int overlayAnomGrid ();
long legendEventHandler ();
#endif


#if XVT_CC_PROTO
static void scroll_sync(WINDOW);
#else
static void scroll_sync();
#endif

/* ======================================================================
FUNCTION        showLegendWindow
DESCRIPTION
        specifically sets the legend window to be showing or not
INPUT

OUTPUT

RETURNED   TRUE - showing, FALSE - not showing
====================================================================== */
int
#if XVT_CC_PROTO
showLegendWindow(int show)
#else
showLegendWindow (show)
int show;
#endif
{
	if (show)
	{
	   if (!legendWindow)
			createLegendWindow();

		if (legendWindow)
		{
			xvt_vobj_set_visible(legendWindow, TRUE);
			legendShowing = TRUE;
		}
	}
	else  /* Hide */
	{
		if (legendWindow)
		{
		   xvt_vobj_set_visible(legendWindow, FALSE);
			xvt_vobj_destroy(legendWindow);
		}
	   legendShowing = FALSE;
	}

   updateMenuOptions (TASK_MENUBAR, NULL_WIN);

	return (legendShowing);
}

/* ======================================================================
FUNCTION        toggleLegendWindowDisplay
DESCRIPTION
        toggle the legend window is showing or not
INPUT

OUTPUT

RETURNED   TRUE - showing, FALSE - not showing
====================================================================== */
int
#if XVT_CC_PROTO
toggleLegendWindowDisplay ()
#else
toggleLegendWindowDisplay ()
#endif
{
	return (showLegendWindow(!legendShowing));
}


/* ======================================================================
FUNCTION        createLegendWindow
DESCRIPTION
        create the legend window initially
INPUT

OUTPUT

RETURNED
====================================================================== */
WINDOW
#if XVT_CC_PROTO
createLegendWindow ()
#else
createLegendWindow ()
#endif
{
	if (legendWindow)
		return (legendWindow);
	else
	{
		WINDOW win = NULL_WIN;
		RCT position;

		xvt_rect_set(&position, LEGEND_POS_X, LEGEND_POS_Y,
			                     LEGEND_POS_X + LEGEND_WIDTH,
										LEGEND_POS_Y + LEGEND_HEIGHT);

	   win = xvt_win_create (W_DOC, &position,
                   LEGEND_TITLE, (int) NULL, TASK_WIN,
                   WSF_NO_MENUBAR | WSF_CLOSE | WSF_SIZE |
                   WSF_INVISIBLE | WSF_ICONIZABLE |
						 WSF_HSCROLL | WSF_VSCROLL,
                   EM_ALL, legendEventHandler, 0L);

		legendWindow = win;
		updateLegendData(NULL_WIN, -1);

		return (win);
	}
}

/* ======================================================================
FUNCTION        destoyLegendWindow
DESCRIPTION
        destroy the legend window initially
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
destroyLegendWindow ()
#else
destroyLegendWindow ()
#endif
{
   if (!legendWindow)
		return (FALSE);
	xvt_vobj_destroy(legendWindow);
	legendWindow = NULL_WIN;
	legendShowing = FALSE;

	return (TRUE);
}

/* ======================================================================
FUNCTION        getLegendWindow
DESCRIPTION
        return the WINDOW that is used for drawing the Legend in
INPUT

OUTPUT

RETURNED
====================================================================== */
WINDOW
#if XVT_CC_PROTO
getLegendWindow ()
#else
getLegendWindow ()
#endif
{
	return (legendWindow);
}

/* ======================================================================
FUNCTION        drawLegend
DESCRIPTION
        update the contents of the Legend Window to reflect current window
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
drawLegend (WINDOW win, RCT *rect)
#else
drawLegend (win, rect)
WINDOW win;
RCT *rect;
#endif
{
	LEGEND_DATA *legendData = NULL;
	RCT imageRect, destRect, srcRect;
	int scrollX, scrollY, addOn;
	PNT rectPos;

	if (!win)
		return (FALSE);
	if (!(legendData = (LEGEND_DATA *) xvt_vobj_get_data(win)))
		return (FALSE);

	xvt_dwin_clear(win, COLOR_WHITE);

	if (rect)
		xvt_rect_set(rect, 0, 0, 0, 0);

	if (rect)
	{
		scrollX = 0;
		scrollY = 0;
	}
	else
	{
		scrollX = xvt_sbar_get_pos (win, HSCROLL);
		scrollY = xvt_sbar_get_pos (win, VSCROLL);
	}
	rectPos.h = -scrollX;
	rectPos.v = -scrollY;

	while (legendData)
	{
		xvt_vobj_get_client_rect(legendData->pixmap, &imageRect);
		destRect = imageRect;
		xvt_rect_set_pos(&destRect, rectPos);
		if ((destRect.right > 0) && (destRect.bottom > 0))  /* Draw if it if showing */
		{
			srcRect = imageRect;
			if (destRect.left < 0)
			{
				addOn = abs(destRect.left);
				destRect.left += addOn;
				srcRect.left  += addOn;
			}
			if (destRect.top < 0)
			{
				addOn = abs(destRect.top);
				destRect.top += addOn;
				srcRect.top  += addOn;
			}
			xvt_dwin_draw_pmap (win, legendData->pixmap, &destRect, &srcRect);
		}

		if (rect)  /* Get the total extent of the images we are drawing */
		{
			if (destRect.bottom > rect->bottom)
				rect->bottom = destRect.bottom;
			if (destRect.right > rect->right)
				rect->right = destRect.right;
		}

		rectPos.v = -scrollY;
		               /* Put Block Dim below Noddy ID when near each other */
		if ((legendData->component == NODDY_ID_LEGEND) && (legendData->next->component == BLOCK_DIM_LEGEND))
		   rectPos.v += xvt_rect_get_height (&imageRect);  /* Put under Noddy ID */
		else
		   rectPos.h += xvt_rect_get_width (&imageRect); /* Put everything else beside */
		legendData = legendData->next;
	}

	return (TRUE);
}

/* ======================================================================
FUNCTION        createNoddyIdLegend
DESCRIPTION
        Create the a pixmap with company Logo and Noddy logo in
INPUT

OUTPUT

RETURNED
====================================================================== */
static LEGEND_DATA *
#if XVT_CC_PROTO
createNoddyIdLegend ()
#else
createNoddyIdLegend ()
#endif
{
	LEGEND_DATA *legendData = NULL;
	XVT_PIXMAP pixmap, companyPixmap, noddyPixmap;
	int legendWidth = 300;
	RCT pixmapSize, destRect;

	if (!(legendData = (LEGEND_DATA *) xvt_mem_zalloc(sizeof(LEGEND_DATA))))
		return (legendData);

   strcpy(legendData->title, "Noddy ID");
	legendData->component = NODDY_ID_LEGEND;
	if (!(legendData->pixmap = xvt_pmap_create(legendWindow, XVT_PIXMAP_DEFAULT, (short) legendWidth, ICON_SIZE*3, NULL)))
	{
		xvt_mem_free ((char *) legendData);
		return (NULL);
	}
	pixmap = legendData->pixmap;
	xvt_dwin_clear (pixmap, COLOR_WHITE);
	xvt_dwin_set_fore_color (pixmap, COLOR_BLACK);

	xvt_fsys_set_dir_startup();
	noddyPixmap = loadPixmap (NODDY_LOGO_NAME);
	companyPixmap = loadPixmap (COMPANY_LOGO_NAME);

	if (companyPixmap)
	{
		xvt_vobj_get_client_rect(companyPixmap, &pixmapSize);
		xvt_rect_set(&destRect, 0, 0, (short) (legendWidth-ICON_SIZE*3), (short) (ICON_SIZE*3));
		xvt_dwin_draw_pmap(pixmap, companyPixmap, &destRect, &pixmapSize);
	}
	if (noddyPixmap)
	{
		xvt_vobj_get_client_rect(noddyPixmap, &pixmapSize);
		if (companyPixmap)
		   xvt_rect_set(&destRect, (short) (legendWidth-ICON_SIZE*3), 0, 
			                        (short) legendWidth, (short) ICON_SIZE*3);
		else
		   xvt_rect_set(&destRect, (short) (legendWidth/2-ICON_SIZE*1.5), 0, 
			                        (short) (legendWidth/2+ICON_SIZE*1.5), (short) ICON_SIZE*3);
		xvt_dwin_draw_pmap(pixmap, noddyPixmap, &destRect, &pixmapSize);
	}
	else
	{
		//xvt_dwin_set_font_family(pixmap, "helvetica");
		xvt_dwin_set_font_size(pixmap, FONT_HUGE);
		////xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD | XVT_FS_ITALIC);
		xvt_dwin_draw_text(pixmap, legendWidth/2 - xvt_dwin_get_text_width(pixmap, "NODDY", -1)/2,
			                        ICON_SIZE*2, "NODDY", -1);
	}

	if (noddyPixmap)
		xvt_pmap_destroy(noddyPixmap);
	if (companyPixmap)
		xvt_pmap_destroy(companyPixmap);

	return (legendData);
}

/* ======================================================================
FUNCTION        createBlockDimLegend
DESCRIPTION
        update the contents of the Legend Window to reflect current window
INPUT

OUTPUT

RETURNED
====================================================================== */
static LEGEND_DATA *
#if XVT_CC_PROTO
createBlockDimLegend (char *legendTitle, double north, double south, double east, double west,
		double top, double bottom, double cubeSize, double easting, double northing,
		char *filename, char *dateTime, char *noteLabel, char *note1, char *note2, char *note3, char *note4,
		int scaleLength, double scaleDistance, int northArrow)
#else
createBlockDimLegend (legendTitle, north, south, east, west, top, bottom, cubeSize, easting, northing,
				filename, dateTime, noteLabel, note1, note2, note3, note4, scaleLength, scaleDistance, northArrow)
double north, south, east, west, top, bottom, cubeSize, easting, northing;
char *filename, *dateTime, *noteLabel, *note1, *note2, *note3, *note4;
int scaleLength;
double scaleDistance;
int northArrow;
#endif
{
	LEGEND_DATA *legendData = NULL;
	char label[20];
	PNT point;
	RCT rect;
	int x, y, legendWidth = 300, legendHeight = 310;
	int fAscent, fDescent, fLeading, fHeight, strLength;
	CPEN pen;
	XVT_PIXMAP pixmap;

	if (!(legendData = (LEGEND_DATA *) xvt_mem_zalloc(sizeof(LEGEND_DATA))))
		return (legendData);
   if (legendTitle)
	   strcpy(legendData->title, legendTitle);
	else
	   strcpy(legendData->title, "Block");
	legendData->component = BLOCK_DIM_LEGEND;
	if (!(legendData->pixmap = xvt_pmap_create(legendWindow, XVT_PIXMAP_DEFAULT, (short) legendWidth, (short) legendHeight, NULL)))
	{
		xvt_mem_free ((char *) legendData);
		return (NULL);
	}
	pixmap = legendData->pixmap;
	xvt_dwin_clear(pixmap, COLOR_WHITE);
  // xvt_dwin_set_font_family(pixmap, "helvetica");
	xvt_dwin_set_font_size(pixmap, FONT_NORMAL);
	xvt_dwin_get_font_metrics(pixmap, &fLeading, &fAscent, &fDescent);
	fHeight = fAscent + fDescent;
	pen.width = 2;       pen.color = COLOR_BLACK;
   pen.style = P_SOLID; pen.pat = PAT_SOLID;
	xvt_dwin_set_cpen(pixmap, &pen);

	////xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD | XVT_FS_ITALIC);
	xvt_dwin_draw_text(pixmap, 2, 10, legendData->title, -1);
	point.h = xvt_dwin_get_text_width(pixmap, legendData->title, -1)+7; point.v = 5;
	xvt_dwin_draw_set_pos (pixmap, point);
	point.h = legendWidth-5; xvt_dwin_draw_line (pixmap, point);
	point.v = legendHeight-5; xvt_dwin_draw_line (pixmap, point);
	point.h = 5;   xvt_dwin_draw_line (pixmap, point);
	point.v = 15;  xvt_dwin_draw_line (pixmap, point);

	x = 120; y = 27;
	////xvt_dwin_set_font_style (pixmap, XVT_FS_NONE);
	////xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD);
	xvt_dwin_draw_text(pixmap, x-xvt_dwin_get_text_width(pixmap,"Dimensions :",-1), y, "Dimensions :", -1);
	xvt_dwin_draw_text(pixmap, x+5,  y,    "North", -1);
	xvt_dwin_draw_text(pixmap, x+70, y,    "South", -1);
	xvt_dwin_draw_text(pixmap, x+5,  y+35, "East", -1);
	xvt_dwin_draw_text(pixmap, x+70, y+35, "West", -1); 
	xvt_dwin_draw_text(pixmap, x+5,  y+70, "Top", -1);
	xvt_dwin_draw_text(pixmap, x+70, y+70, "Bottom", -1); 
	////xvt_dwin_set_font_style (pixmap, XVT_FS_NONE);
   sprintf(label,"%.0lf", north); xvt_dwin_draw_text(pixmap, x+5,  y+15, label, -1);
	sprintf(label,"%.0lf", south); xvt_dwin_draw_text(pixmap, x+70, y+15, label, -1);
   sprintf(label,"%.0lf", east);  xvt_dwin_draw_text(pixmap, x+5,  y+50, label, -1);
	sprintf(label,"%.0lf", west);  xvt_dwin_draw_text(pixmap, x+70, y+50, label, -1);
   sprintf(label,"%.0lf", top);  xvt_dwin_draw_text(pixmap, x+5,  y+85, label, -1);
	sprintf(label,"%.0lf", bottom);  xvt_dwin_draw_text(pixmap, x+70, y+85, label, -1);

	x = 120; y += 105;
	////xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD);
	xvt_dwin_draw_text(pixmap, x-xvt_dwin_get_text_width(pixmap,"Cube Size :",-1), y, "Cube Size :", -1);
//	//xvt_dwin_set_font_style (pixmap, XVT_FS_NONE);
	sprintf(label,"%.0lf",cubeSize);
	xvt_dwin_draw_text(pixmap, x+5, y, label, -1);

	x = 120; y += 20;
	//xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD);
	xvt_dwin_draw_text(pixmap, x-xvt_dwin_get_text_width(pixmap,"Origin :",-1), y, "Origin :", -1);
//	//xvt_dwin_set_font_style (pixmap, XVT_FS_NONE);
	sprintf(label,"%.0lfE",easting);
	xvt_dwin_draw_text(pixmap, x+5,  y, label, -1);
	sprintf(label,"%.0lfN",northing);
	xvt_dwin_draw_text(pixmap, x+5,  y+15, label, -1);

	x = 120; y += 35;
//	//xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD);
	xvt_dwin_draw_text(pixmap, x-xvt_dwin_get_text_width(pixmap,"Date & Time :",-1), y, "Date & Time :", -1);
//	//xvt_dwin_set_font_style (pixmap, XVT_FS_NONE);
	if (dateTime)
	{
		strLength = strlen(dateTime);
		if (strchr(dateTime,'\n'))
			strLength--;
	   xvt_dwin_draw_text(pixmap, x+5,  y, dateTime, strLength);
	}
	
	x = 120; y += 20;
	//xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD);
	xvt_dwin_draw_text(pixmap, x-xvt_dwin_get_text_width(pixmap,"History File :",-1), y, "History File :", -1);
	//xvt_dwin_set_font_style (pixmap, XVT_FS_NONE);
	xvt_dwin_draw_text(pixmap, x+5,  y, filename, -1);
	
	x = 120; y += 20;
	//xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD);
	if (noteLabel)
		xvt_dwin_draw_text(pixmap, x-xvt_dwin_get_text_width(pixmap,noteLabel,-1), y, noteLabel, -1);
	else
		xvt_dwin_draw_text(pixmap, x-xvt_dwin_get_text_width(pixmap,"Notes :",-1), y, "Notes :", -1);
	//xvt_dwin_set_font_style (pixmap, XVT_FS_NONE);
	////xvt_dwin_set_font_style (pixmap, XVT_FS_ITALIC);
	if (note1)
		xvt_dwin_draw_text(pixmap, x+5, y, note1, -1);
	if (note2)
   	xvt_dwin_draw_text(pixmap, x+5, y+15, note2, -1);
	if (note3)
   	xvt_dwin_draw_text(pixmap, x+5, y+30, note3, -1);
	if (note4)
   	xvt_dwin_draw_text(pixmap, x+5, y+45, note4, -1);

	if (scaleLength > 0)  /* Draw a scale bar */
	{
		double distPerPixel;
		double barDistance;
		int barLength, maxBarLength, numUnits, unit;
		PNT labelMinPos, labelMaxPos;
		char minLabel[20], maxLabel[20];

		y += 60;
		maxBarLength = legendWidth - 20;
		distPerPixel = (double) (scaleDistance/(double) scaleLength);
		barDistance = 10;
		barLength = (int) (barDistance/distPerPixel);
		while (barLength < maxBarLength)
		{
			barDistance *= 10;
			barLength = (int) (barDistance/distPerPixel);
		}
		barDistance /= 10; /* move back to where it fitted in the width */
		barLength = (int) (barDistance/distPerPixel);
		xvt_rect_set(&rect, 0, (short) y, (short) barLength, (short) (y+8));  /* Unit Bar */
		labelMinPos.v = y - 2;
		labelMaxPos.v = y - 2;

		if (barLength*5 < maxBarLength)
		{          /* Draw 5 Units */
			barDistance *= 5;
		   barLength *= 5;
			numUnits = 5;
		}
		else if (barLength*2 < maxBarLength)
		{          /* Draw 2 Units */
			barDistance *= 2;
			barLength *= 2;
			numUnits = 2;
		}
		else
		{          /* Draw single unit in 2 halves (note length and Dist stays constant) */
			numUnits = 2;
		}

		strcpy(minLabel, "0");
		sprintf(maxLabel, "%.0lf", barDistance);

		rect.left = legendWidth/2 - barLength/2;
		rect.right = rect.left + barLength/numUnits;
		labelMinPos.h = rect.left;
		xvt_dwin_set_std_cpen(pixmap, TL_PEN_BLACK);
		for (unit = 0; unit < numUnits; unit++)
		{
		/* 	if ((unit+1)%2)
				xvt_dwin_set_std_cbrush(pixmap, TL_BRUSH_BLACK);
			else
				xvt_dwin_set_std_cbrush(pixmap, TL_BRUSH_WHITE); */

			xvt_dwin_draw_rect(pixmap, &rect);

			rect.left += barLength/numUnits;
			rect.right += barLength/numUnits;
		}
		labelMaxPos.h = rect.left - xvt_dwin_get_text_width(pixmap, maxLabel, -1);

		xvt_dwin_draw_text(pixmap, labelMinPos.h, labelMinPos.v, minLabel, -1);
		xvt_dwin_draw_text(pixmap, labelMaxPos.h, labelMaxPos.v, maxLabel, -1);
	}

	if (northArrow)  /* Draw a North Arrow */
	{
		PNT arrowPoint;

		////xvt_dwin_set_font_style (pixmap, XVT_FS_ITALIC);
		arrowPoint.h = 20;
		arrowPoint.v = 100;
		xvt_dwin_draw_set_pos(pixmap, arrowPoint);
		arrowPoint.v -= 50;
		xvt_dwin_draw_aline(pixmap, arrowPoint, FALSE, TRUE);
		arrowPoint.h += 5;
		arrowPoint.v += 25;
		xvt_dwin_draw_text(pixmap, arrowPoint.h, arrowPoint.v, "N", -1);

	}

	return (legendData);
}

/* ======================================================================
FUNCTION        createAnomLUTLegend
DESCRIPTION
        update the contents of the Legend Window to reflect current window
INPUT

OUTPUT

RETURNED
====================================================================== */
static LEGEND_DATA *
#if XVT_CC_PROTO
createAnomLUTLegend (COLOR *lut, int lutSize, double minValue, double maxValue,
							double minClip, double maxClip, char *label)
#else
createAnomLUTLegend (lut, lutSize, minValue, maxValue, minClip, maxClip, label)
COLOR *lut;
int lutSize;
double minValue, maxValue, minClip, maxClip;
char *label;
#endif
{
	LEGEND_DATA *legendData = NULL;
	DRAW_CTOOLS tools;
	char string[20];
	RCT rect;
	double rectSize;
	int legendWidth = 100, legendHeight = 310+ICON_SIZE*3, barHeight, barBottom;
	int fAscent, fDescent, fLeading, fHeight, index, labelPos;
	XVT_PIXMAP pixmap;

	if (!(legendData = (LEGEND_DATA *) xvt_mem_zalloc(sizeof(LEGEND_DATA))))
		return (legendData);
   strcpy(legendData->title, "Anom LUT");
	legendData->component = ANOM_LUT_LEGEND;
	if (!(legendData->pixmap = xvt_pmap_create(legendWindow, XVT_PIXMAP_DEFAULT, (short) legendWidth, (short) legendHeight, NULL)))
	{
		xvt_mem_free ((char *) legendData);
		return (NULL);
	}
	pixmap = legendData->pixmap;
	xvt_dwin_clear(pixmap, COLOR_WHITE);
  // xvt_dwin_set_font_family(pixmap, "helvetica");
	xvt_dwin_set_font_size(pixmap, FONT_NORMAL);
	xvt_dwin_get_font_metrics(pixmap, &fLeading, &fAscent, &fDescent);
	fHeight = fAscent + fDescent;

   xvt_dwin_get_draw_ctools (pixmap, &tools);
   tools.pen.width = 1;
   xvt_dwin_set_draw_ctools (pixmap, &tools);

	barHeight = legendHeight - 20;
	rectSize = barHeight/(double) lutSize;
	barBottom = legendHeight+((legendHeight-barHeight)/2);
	rect.left = 5; rect.right = 30;
	for (index = 0; index < lutSize; index++)
	{
		rect.bottom = (short) (barBottom - index*rectSize);
		rect.top = (short) (barBottom - (index+1)*rectSize);
      tools.pen.color = lut[index];
		tools.brush.color = lut[index];
	   xvt_dwin_set_draw_ctools (pixmap, &tools);
		xvt_dwin_draw_rect(pixmap, &rect);
	}

	////xvt_dwin_set_font_style (pixmap, XVT_FS_BOLD | XVT_FS_ITALIC);
	sprintf(string,"%.1lf",minValue);
	xvt_dwin_draw_text(pixmap, 35, legendHeight, string, -1);
	sprintf(string,"%.1lf",maxValue);
	xvt_dwin_draw_text(pixmap, 35, 30, string, -1);

	if (minClip != minValue)
	{
		sprintf(string,"%.1lf",minClip);
		labelPos = (int) (barBottom - ((double) barHeight) * ((minClip - minValue)/(maxValue-minValue)));
		xvt_dwin_draw_text(pixmap, 35, labelPos, string, -1);
	}

	if (maxClip != maxValue)
	{
		sprintf(string,"%.1lf",maxClip);
		labelPos = (int) (barBottom - ((double) barHeight) * ((maxClip - minValue)/(maxValue-minValue)));
		xvt_dwin_draw_text(pixmap, 35, labelPos, string, -1);
	}
            
	return (legendData);
}

/* ======================================================================
FUNCTION        createStratColumnLegend
DESCRIPTION
        update the contents of the Legend Window to reflect current window
INPUT

OUTPUT

RETURNED
====================================================================== */
static LEGEND_DATA *
#if XVT_CC_PROTO
createStratColumnLegend ()
#else
createStratColumnLegend ()
#endif
{
#define STRAT_COLUMN_WIDTH        230
#define NUM_SINGLE_UNITS_IN_STRAT 8
	LEGEND_DATA *legendData = NULL;
	int legendWidth = 100, legendHeight = 310+ICON_SIZE*3-10;
	int totalWidthStrat = 0, totalDykes = 0, totalPlugs = 0;
	XVT_PIXMAP pixmap;
	OBJECT *object;

	if (!(legendData = (LEGEND_DATA *) xvt_mem_zalloc(sizeof(LEGEND_DATA))))
		return (legendData);
   strcpy(legendData->title, "Strat Column");
	legendData->component = STRAT_COL_LEGEND;

   for (object = nthObject(NULL_WIN, 0); object != NULL; object = object->next)
   {
      if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY) || (object->shape == IMPORT))
         totalWidthStrat += STRAT_COLUMN_WIDTH;
      if (object->shape == DYKE)
         totalDykes++;
		if (object->shape == PLUG)
         totalPlugs++;
   }
	legendWidth = totalWidthStrat;
	if (totalDykes > 0)
	{
		legendWidth += (totalDykes/NUM_SINGLE_UNITS_IN_STRAT)*STRAT_COLUMN_WIDTH;
		if (totalDykes%NUM_SINGLE_UNITS_IN_STRAT) legendWidth += STRAT_COLUMN_WIDTH;
	}
	if (totalPlugs > 0)
	{
		legendWidth += (totalPlugs/NUM_SINGLE_UNITS_IN_STRAT)*STRAT_COLUMN_WIDTH;
		if (totalPlugs%NUM_SINGLE_UNITS_IN_STRAT) legendWidth += STRAT_COLUMN_WIDTH;
	}

	if (!(legendData->pixmap = xvt_pmap_create(legendWindow, XVT_PIXMAP_DEFAULT, (short) legendWidth, (short) legendHeight, NULL)))
	{
		xvt_mem_free ((char *) legendData);
		return (NULL);
	}
	pixmap = legendData->pixmap;

   plotStratColumns (pixmap);

	return (legendData);
}

/* ======================================================================
FUNCTION        createOrientationLegend
DESCRIPTION
        update the contents of the Legend Window to reflect current window
INPUT

OUTPUT

RETURNED
====================================================================== */
static LEGEND_DATA *
#if XVT_CC_PROTO
createOrientationLegend (double orientations[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS], int count)
#else
createOrientationLegend (orientations, count)
double orientations[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS];
int count;  /* count of the number of orientations */
#endif
{
	LEGEND_DATA *legendData = NULL;
	int legendWidth = 310+ICON_SIZE*3-10, legendHeight = 310+ICON_SIZE*3-10;
	XVT_PIXMAP pixmap;

	if (!(legendData = (LEGEND_DATA *) xvt_mem_zalloc(sizeof(LEGEND_DATA))))
		return (legendData);
   strcpy(legendData->title, "Orientation Plot");
	legendData->component = ORIENT_LEGEND;

	if (!(legendData->pixmap = xvt_pmap_create(legendWindow, XVT_PIXMAP_DEFAULT, (short) legendWidth, (short) legendHeight, NULL)))
	{
		xvt_mem_free ((char *) legendData);
		return (NULL);
	}
	pixmap = legendData->pixmap;
	xvt_dwin_clear(pixmap, COLOR_WHITE);

   DrawStereoNet (pixmap, orientations, count);

	return (legendData);
}

/* ======================================================================
FUNCTION        freeDrawData
DESCRIPTION
        update the contents of the Legend Window to reflect current window
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
freeDrawData (DRAW_DATA *drawData)
#else
freeDrawData (drawData)
DRAW_DATA *drawData;
#endif
{
	DRAW_DATA *nextDrawData;

	while (drawData)
	{
		if (drawData->data)
			xvt_mem_free((char *) drawData->data);
		nextDrawData = drawData->next;
		xvt_mem_free((char *) drawData);
		drawData = nextDrawData;
	}

	return (TRUE);
}

/* ======================================================================
FUNCTION        updateLegendData
DESCRIPTION
        update the contents of the Legend Window to reflect current window
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
updateLegendData (WINDOW win, LEGEND_TYPES type)
#else
updateLegendData (win, type)
WINDOW win;
LEGEND_TYPES type;
#endif
{
	LEGEND_DATA *legendData = NULL, *nextLegendData, *newLegendData = NULL;
	FILE_SPEC fileSpec;
	BLOCK_VIEW_OPTIONS *blockView;
	char legendTitle[40];
   time_t longTime;
   struct tm *today;


	if (!legendWindow)
		return (FALSE);

   time(&longTime); /* current Time */
   today = localtime(&longTime);

	if (win)
		xvt_vobj_get_title(win, legendTitle, 40);
	else
		strcpy(legendTitle, "Block");

	switch (type)
	{
		case (SECTION_LEGENDS):
			{
			   SECTION_DATA *sectionData;
				RCT rect;
				int scaleSize;
				double scaleDistance, xDist, yDist, zDist;
				int northArrow;

				if (win)
					sectionData = (SECTION_DATA *) xvt_vobj_get_data (win);
				if (!sectionData)
					break;
				getCurrentFileName(&fileSpec);
				blockView = getViewOptions();
				xvt_vobj_get_client_rect(win, &rect);
				xDist = sectionData->surfaceData->xEnd - sectionData->surfaceData->xStart;
				yDist = sectionData->surfaceData->yEnd - sectionData->surfaceData->yStart;
				zDist = sectionData->surfaceData->zEnd - sectionData->surfaceData->zStart;
				if (zDist < 1)
					northArrow = TRUE;  /* Map so put Nth On */
				else
					northArrow = FALSE; /* Section so no arrow */
				scaleDistance = sqrt(xDist*xDist + yDist*yDist + zDist*zDist);
				scaleSize = (int) sqrt(xvt_rect_get_width(&rect)*xvt_rect_get_width(&rect)
					                 + xvt_rect_get_height(&rect)*xvt_rect_get_height(&rect));
				if (!(newLegendData = createNoddyIdLegend ()))
					return (FALSE);
				newLegendData->next = createBlockDimLegend (legendTitle, sectionData->surfaceData->yStart,
					    sectionData->surfaceData->yEnd, sectionData->surfaceData->xStart,
						 sectionData->surfaceData->xEnd, sectionData->surfaceData->zStart,
						 sectionData->surfaceData->zEnd, blockView->geologyCubeSize,
						 projectOptions.easting, projectOptions.northing, fileSpec.name, asctime(today),
						 NULL, NULL, NULL, NULL, NULL, scaleSize, scaleDistance, northArrow);
				newLegendData->next->next = createStratColumnLegend ();
				newLegendData->next->next->next = createOrientationLegend (sectionData->orientations, sectionData->count);
			}
			break;
		case (THREED_LEGENDS):
			{
            THREED_IMAGE_DATA *threedData = NULL;
				char declLabel[30], aziLabel[30], sclLabel[30];

				if (win)
					threedData = (THREED_IMAGE_DATA *) xvt_vobj_get_data (win);
				getCurrentFileName(&fileSpec);
				blockView = getViewOptions();
				if (!(newLegendData = createNoddyIdLegend ()))
					return (FALSE);
				sprintf(declLabel,"Declination = %.1lf", threedData->declination);
				sprintf(aziLabel,"Azimuth = %.1lf", threedData->azimuth);
				sprintf(sclLabel,"Scale = %.1lf", threedData->scale*100.0);
				newLegendData->next = createBlockDimLegend (legendTitle, blockView->originY+blockView->lengthY, blockView->originY,
						 blockView->originX+blockView->lengthX, blockView->originX, blockView->originZ,
						 blockView->originZ-blockView->lengthZ, blockView->geologyCubeSize,
						 projectOptions.easting, projectOptions.northing, fileSpec.name, asctime(today),
						 "Orientation :", declLabel, aziLabel, sclLabel, NULL, -1, 0.0, FALSE);
				newLegendData->next->next = createStratColumnLegend ();
			}
			break;
		case (BLOCK_DIAGRAM_LEGENDS):
			{
            BLOCK_DIAGRAM_DATA *diagram = NULL;
				char declLabel[30], aziLabel[30], sclLabel[30];
				double clipMin, clipMax, clipInc;
				int index;

				if (win)
					diagram = (BLOCK_DIAGRAM_DATA *) xvt_vobj_get_data (win);
				getCurrentFileName(&fileSpec);
				blockView = getViewOptions();
				if (!(newLegendData = createNoddyIdLegend ()))
					return (FALSE);
				if (diagram)
				{
					sprintf(declLabel,"Declination = %.1lf", diagram->threedData.declination);
					sprintf(aziLabel,"Azimuth = %.1lf", diagram->threedData.azimuth);
					sprintf(sclLabel,"Scale = %.1lf", diagram->threedData.scale*100.0);
					newLegendData->next = createBlockDimLegend (legendTitle, diagram->minYLoc+diagram->ny*diagram->blockSize,
					       diagram->minYLoc, diagram->minXLoc+diagram->nx*diagram->blockSize, diagram->minXLoc,
							 diagram->minZLoc, diagram->minZLoc+diagram->nz*diagram->blockSize, diagram->blockSize,
							 projectOptions.easting, projectOptions.northing, fileSpec.name, asctime(today),
							 "Orientation :", declLabel, aziLabel, sclLabel, NULL, -1, 0.0, FALSE);
				}
				else
					newLegendData->next = createBlockDimLegend (legendTitle, blockView->originY+blockView->lengthY, blockView->originY,
							 blockView->originX+blockView->lengthX, blockView->originX, blockView->originZ,
							 blockView->originZ-blockView->lengthZ, blockView->geologyCubeSize,
							 projectOptions.easting, projectOptions.northing, fileSpec.name, asctime(today),
							 NULL, NULL, NULL, NULL, NULL, -1, 0.0, FALSE);
				if (diagram)
				{
					if ((diagram->type >= DENSITY_BLOCK) && diagram->lut)
					{
						clipMin = (double) diagram->minValueData;
						clipMax = (double) diagram->maxValueData;
						clipInc = (clipMax - clipMin)/diagram->lutSize;
						for (index = 0; diagram->lut[index] == COLOR_INVALID; index++)
							; /* do nothing */
						if (index > 0)
							clipMin += index*clipInc;

						for (index = 0; diagram->lut[diagram->lutSize-index-1] == COLOR_INVALID; index++)
							; /* do nothing */
						if (index > 0)
						   clipMax -= index*clipInc;

						newLegendData->next->next = createAnomLUTLegend (diagram->lut, diagram->lutSize,
							              (double) diagram->minValueData, (double) diagram->maxValueData,
							              clipMin, clipMax, NULL);
					}
					else
						newLegendData->next->next = createStratColumnLegend ();
				}
				else
					newLegendData->next->next = createStratColumnLegend ();
			}
			break;
		case (MAG_IMAGE_LEGENDS):
			{
				ANOMIMAGE_DATA *anomData = NULL;
				char incLabel[30], decLabel[30], intenLabel[30], altLabel[30];
				int northArrow, scaleLength;
				double scaleDistance;
				RCT rect;

				if (win)
					anomData = (ANOMIMAGE_DATA *) xvt_vobj_get_data (win);
				if (!anomData)
					break;
				xvt_vobj_get_client_rect(win, &rect);
				getCurrentFileName(&fileSpec);
				blockView = getViewOptions();
				if (anomData->dataYSize > 1)  /* Map and not profile */
				{
					northArrow = TRUE;
					scaleDistance = anomData->geoXEnd-anomData->geoXStart;
					scaleLength = rect.right-rect.left;
				}
				else
				{
					northArrow = FALSE;
					scaleDistance = 0.0;
					scaleLength = -1;
				}

				if (!(newLegendData = createNoddyIdLegend ()))
					return (FALSE);
				sprintf(incLabel,"Inclination = %.1lf",anomData->inclination);
				sprintf(decLabel,"Declination = %.1lf",anomData->declination);
				sprintf(intenLabel,"Intensity = %.1lf",anomData->intensity);
				sprintf(altLabel,"Altitude = %.1lf",anomData->geoHeight);
				newLegendData->next = createBlockDimLegend (legendTitle, anomData->geoYEnd, anomData->geoYStart,
							 anomData->geoXEnd, anomData->geoXStart, anomData->fileBlockTop, anomData->fileBlockBottom,
							 anomData->fileCubeSize, 0.0, 0.0, anomData->historyFile, anomData->dateTime,
							 "Earth's Field :", incLabel, decLabel, intenLabel, altLabel,
							 scaleLength, scaleDistance, northArrow);
				if (anomData->dataYSize > 1)  /* Map and not profile */
					newLegendData->next->next = createAnomLUTLegend (anomData->imageDisplay.lut,
					       anomData->imageDisplay.lutSize, (double) anomData->minValue, (double) anomData->maxValue,
							 (double) anomData->imageDisplay.minClip, (double) anomData->imageDisplay.maxClip, NULL);
			}
			break;
		default:
			getCurrentFileName(&fileSpec);
			blockView = getViewOptions();
			if (!(newLegendData = createNoddyIdLegend ()))
				return (FALSE);
			newLegendData->next = createBlockDimLegend (legendTitle, blockView->originY+blockView->lengthY, blockView->originY,
				       blockView->originX+blockView->lengthX, blockView->originX, blockView->originZ,
						 blockView->originZ-blockView->lengthZ, blockView->geologyCubeSize,
						 projectOptions.easting, projectOptions.northing, fileSpec.name, asctime(today), NULL, "Current Block",
						 NULL, NULL, NULL, -1, 0.0, FALSE);
			break;
	}

	legendData = (LEGEND_DATA *) xvt_vobj_get_data(legendWindow);
	while (legendData)    /* Free any Legend Data that we already had */
	{
		if (legendData->pixmap)
			xvt_pmap_destroy(legendData->pixmap);
		if (legendData->drawData)
		{
			if (freeDrawData(legendData->drawData))
				legendData->drawData = NULL;
		}
		nextLegendData = legendData->next;
		xvt_mem_free((char *) legendData);
		legendData = nextLegendData;
	}

	xvt_vobj_set_data(legendWindow, (long) newLegendData);

	xvt_dwin_invalidate_rect(legendWindow, NULL);
	xvt_dwin_update(legendWindow);

	return (TRUE);
}

/* ======================================================================
FUNCTION        getLegendPixmap
DESCRIPTION
        create one big pixmap that contains everything in the legend
INPUT

OUTPUT

RETURNED
====================================================================== */
XVT_PIXMAP
#if XVT_CC_PROTO
getLegendPixmap ()
#else
getLegendPixmap ()
#endif
{
	XVT_PIXMAP pixmap = NULL_PIXMAP;
	RCT boundingRect;

	if (legendPixmap)
	{
		xvt_pmap_destroy(legendPixmap);
		legendPixmap = NULL_PIXMAP;
	}

	drawLegend (legendWindow, &boundingRect);
	if (!(pixmap = xvt_pmap_create(legendWindow, XVT_PIXMAP_DEFAULT,
		            (short) boundingRect.right, (short) boundingRect.bottom, NULL)))
		return (pixmap);

	xvt_vobj_set_data(pixmap, xvt_vobj_get_data(legendWindow));
	drawLegend (pixmap, &boundingRect);

	if (pixmap)
		legendPixmap = pixmap;

	return (pixmap);
}

/* ======================================================================
FUNCTION        overlayAnomGrid
DESCRIPTION
        Draw a gird over an anomalies image
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
overlayAnomGrid (WINDOW win, ANOMIMAGE_DATA *imageData)
#else
overlayAnomGrid (win, imageData)
WINDOW win;
ANOMIMAGE_DATA *imageData;
#endif
{
#define NUM_GRID_LINES 10	
	double lengthNS, lengthEW, winScale;  /* Use longs so we have whole numbers */
	double xInc, yInc, xPos, yPos;
	long imageStartEW, imageEndEW, imageStartNS, imageEndNS;
	long incNS, incEW, roundNS, roundEW;
	long locNS, locEW, gridStartNS, gridStartEW, fontSize1, fontSize2;
	char *ptr, digitLabel[10], zeroLabelNS[10], zeroLabelEW[10];
	int width, height, x, y, digit;
	COLOR overlayColor;
	DRAW_CTOOLS tools;
	RCT position;
	PNT point;
	
	if (!imageData)
		return (FALSE);


	if (imageData->imageDisplay.sensorOverlay)
	{						/* ******************************** */
							/* Now draw the grid over the image */
		if (imageData->imageDisplay.display == GREY_COLOR)
			overlayColor = COLOR_BLUE;
		else
			overlayColor = COLOR_LTGRAY;
		xvt_dwin_get_draw_ctools (win, &tools);
		tools.pen.width = 1;
		tools.pen.pat = PAT_SOLID;
		tools.pen.style = P_SOLID;
		tools.pen.color = overlayColor;
		tools.brush.pat = PAT_HOLLOW;
		tools.mode = M_COPY;
		xvt_dwin_set_draw_ctools (win, &tools);

		xvt_vobj_get_client_rect (win, &position);
		width = xvt_rect_get_width (&position);
		height = xvt_rect_get_height (&position);
		xInc = ((double) width) / ((double) imageData->dataXSize-1);
		yInc = ((double) height)/ ((double) imageData->dataYSize-1);

		for (xPos = 0.0; xPos <= width+1; xPos += xInc)
		{
			position.left = (int) floor(xPos - 2.0 + 0.5);
			position.right = (int) floor(xPos + 2.0 + 0.5);
			for (yPos = 0.0; yPos <= height+1; yPos += yInc)
			{
				position.top = (int) floor(yPos - 2.0 + 0.5);
				position.bottom = (int) floor(yPos + 2.0 + 0.5);
				xvt_dwin_draw_oval(win, &position);
			}
		}
	}

	if (!imageData->imageDisplay.gridOverlay)
		return (FALSE);

	lengthNS = imageData->geoYEnd - imageData->geoYStart;
	lengthEW = imageData->geoXEnd - imageData->geoXStart;

	imageStartNS = (long) (imageData->geoYStart);
	imageEndNS = (long) (imageStartNS + lengthNS);
	imageStartEW = (long) (imageData->geoXStart);
	imageEndEW = (long) (imageStartEW + lengthEW);

							/* ******************************** */
							/* Now draw the grid over the image */
	if (imageData->imageDisplay.display == GREY_COLOR)
		overlayColor = COLOR_RED;
	else
		overlayColor = COLOR_DKGRAY;

	xvt_vobj_get_client_rect (win, &position);
	width = xvt_rect_get_width (&position);
	height = xvt_rect_get_height (&position);
	winScale = ((double) width)/((double) (imageData->dataXSize-1));
	xvt_dwin_get_draw_ctools (win, &tools);
	tools.pen.width = 1;
	tools.pen.pat = PAT_SOLID;
	tools.pen.style = P_SOLID;
	tools.pen.color = overlayColor;
	tools.mode = M_COPY;
	xvt_dwin_set_draw_ctools (win, &tools);

	if (imageData->imageDisplay.autoGridSpacing)
	{
		incNS = (long) (lengthNS/(double) NUM_GRID_LINES);
		incEW = (long) (lengthEW/(double) NUM_GRID_LINES);
		gridStartNS = imageStartNS;
		gridStartEW = imageStartEW;

							/* ****************************************************** */
		roundNS = 10; /* Find a nice number of lines to space over the image NS */
		strcpy(zeroLabelNS, "0");
		while (incNS > roundNS)
		{
			incNS /= roundNS;
			incNS *= roundNS;
			roundNS *= 10;   /* move up to next digit */
			strcat(zeroLabelNS, "0");
		}
		incNS = roundNS;
		gridStartNS /= roundNS;
		gridStartNS *= roundNS;
		while (gridStartNS < imageStartNS)
			gridStartNS += incNS; /* make sure we draw on image */

							/* ****************************************************** */
		roundEW = 10; /* Find a nice number of lines to space over the image EW */
		strcpy(zeroLabelEW, "0");
		while (incEW > roundEW)
		{
			incEW /= roundEW;
			incEW *= roundEW;
			roundEW *= 10;   /* move up to next digit */
			strcat(zeroLabelEW, "0");
		}
		incEW = roundEW;
		gridStartEW /= roundEW;
		gridStartEW *= roundEW;
		while (gridStartEW < imageStartEW)
			gridStartEW += incEW; /* make sure we draw on image */

	}
	else  /* Use the values that were typed */
	{
		gridStartNS = (long) (imageData->imageDisplay.gridStartY);
		gridStartEW = (long) (imageData->imageDisplay.gridStartX);
		incNS = (long) fabs(imageData->imageDisplay.gridIncY);
		incEW = (long) fabs(imageData->imageDisplay.gridIncX);
		if (incNS < 10)
			incNS = imageEndNS-imageStartNS;
		if (incEW < 10)
			incEW = imageEndEW-imageStartEW;

		/* pad the grid start locations up to the start of the image */
		while (gridStartNS + incNS < imageStartNS)
			gridStartNS += incNS;
		while (gridStartEW + incEW < imageStartEW)
			gridStartEW += incEW;

		roundNS = 1; /* Find a nice number of 0's to do small */
		sprintf(digitLabel, "%ld", incNS);
		strcpy(zeroLabelNS, "");
		while (ptr = strchr(digitLabel,'0'))
		{
			*ptr = ' '; /* make sure we dont get the same zero again */
			strcat(zeroLabelNS, "0");
			roundNS *= 10;
		}
		roundEW = 1;
		sprintf(digitLabel, "%ld", incEW);
		strcpy(zeroLabelEW, "");
		while (ptr = strchr(digitLabel,'0'))
		{
			*ptr = ' '; /* make sure we dont get the same zero again */
			strcat(zeroLabelEW, "0");
			roundEW *= 10;
		}
	}

								/* ***************************************** */
								/* Draw the Actual Tick marks over the image */
	for (locEW = gridStartEW; locEW <= imageEndEW+1; locEW += incEW)  /* Tick Marks */
	{
		for (locNS = gridStartNS; locNS <= imageEndNS+1; locNS += incNS)
		{
			getAnomImagePointFromLoc (imageData, (double) locEW, (double) locNS, winScale, &x, &y);
			point.h = x-5; point.v = y; xvt_dwin_draw_set_pos (win, point);
			point.h += 11;              xvt_dwin_draw_line (win, point);

			point.h = x; point.v = y-5; xvt_dwin_draw_set_pos (win, point);
			point.v += 11;              xvt_dwin_draw_line (win, point);
		}
	}

	fontSize1 = FONT_NORMAL; fontSize2 = FONT_MEDIUM;       /* Label the Grid */
   //xvt_dwin_set_font_family(win, "helvetica");
	xvt_dwin_set_fore_color(win, overlayColor);
	strcat (zeroLabelEW,"E");
	for (locNS = imageStartNS, locEW = gridStartEW+incEW; locEW <= imageEndEW; locEW += incEW)  /* EW Labels */
	{
      getAnomImagePointFromLoc (imageData, (double) locEW, (double) locNS, winScale, &x, &y);
		y = height - 2;  /* Draw Just off Bottom */
		digit = (int) (locEW / roundEW);
		sprintf (digitLabel, "%d", digit);
		width = xvt_dwin_get_text_width(win, digitLabel, -1);
		xvt_dwin_set_font_size(win, fontSize1);
		xvt_dwin_draw_text (win, x-width, y/*-fontSize1*/, digitLabel, -1);
		xvt_dwin_set_font_size(win, fontSize2);
		xvt_dwin_draw_text (win, x+3, y/*-fontSize1*/, zeroLabelEW, -1);
	}

	strcat (zeroLabelNS,"N");
	for (locNS = gridStartNS, locEW = imageStartEW; locNS <= imageEndNS; locNS += incNS)
	{
      getAnomImagePointFromLoc (imageData, (double) locEW, (double) locNS, winScale, &x, &y);
		x = 2;  /* Draw Just off Edge */
		digit = (int) (locNS / roundNS);
		sprintf (digitLabel, "%d", digit);
		width = xvt_dwin_get_text_width(win, digitLabel, -1);
		xvt_dwin_set_font_size(win, fontSize1);
		xvt_dwin_draw_text (win, x, y, digitLabel, -1);
		xvt_dwin_set_font_size(win, fontSize2);
		xvt_dwin_draw_text (win, x+width+3, y, zeroLabelNS, -1);
	}
		
	return (TRUE);
}

/*
     Function to synchronize scroll bars with position of drawing.
*/
static void
#if XVT_CC_PROTO
scroll_sync(WINDOW win)
#else
scroll_sync(win)
WINDOW win;
#endif
{
	LEGEND_DATA *legendData = NULL;
	PNT range;
   RCT rct;

	if (!(legendData = (LEGEND_DATA *) xvt_vobj_get_data(win)))
		return;

	if (legendPixmap == NULL_PIXMAP)
		return;

	xvt_vobj_get_client_rect(legendPixmap, &rct);
	range.h = xvt_rect_get_width(&rct);
	range.v = xvt_rect_get_height(&rct);

	xvt_vobj_get_client_rect(win, &rct);
	xvt_sbar_set_range(win, HSCROLL, 0, xvt_rect_get_width(&rct));
	xvt_sbar_set_range(win, VSCROLL, 0, xvt_rect_get_height(&rct));
	xvt_sbar_set_pos(win, HSCROLL, legendData->origin.h);
	xvt_sbar_set_pos(win, VSCROLL, legendData->origin.v);
}


/*
     Function to shift the view after scrolling.
*/
static void
#if XVT_CC_PROTO
shift_view(WINDOW win, int dx, int dy, PNT *pntp)
#else
shift_view(win, dx, dy, pntp)
WINDOW win;
int dx, dy;
PNT *pntp;
#endif
{
	LEGEND_DATA *legendData = NULL;
	PNT range;
	RCT rct;

	if (!(legendData = (LEGEND_DATA *) xvt_vobj_get_data(win)))
		return;
	if (legendPixmap == NULL_PIXMAP)
		return;

	xvt_vobj_get_client_rect(legendPixmap, &rct);
	range.h = xvt_rect_get_width(&rct);
	range.v = xvt_rect_get_height(&rct);

	dx = (dx >= 0 ? 1 : -1) * ((abs(dx) + 7) / 8) * 8; /* force aligned pats */
	dy = (dy >= 0 ? 1 : -1) * ((abs(dy) + 7) / 8) * 8;
	xvt_dwin_update(win);
	if (dx != 0) {
		if (dx > 0)
			dx = MIN(dx, legendData->origin.h);
		else
			dx = MAX(dx, legendData->origin.h - range.h);
		legendData->origin.h -= dx;
		xvt_sbar_set_pos(win, HSCROLL, legendData->origin.h);
	}
	if (dy != 0) {
		if (dy > 0)
			dy = MIN(dy, legendData->origin.v);
		else
			dy = MAX(dy, legendData->origin.v - range.v);
		legendData->origin.v -= dy;
		xvt_sbar_set_pos(win, VSCROLL, legendData->origin.v);
	}
	xvt_vobj_get_client_rect(win, &rct);
	xvt_dwin_scroll_rect(win, &rct, dx, dy);
	if (pntp != NULL) {
		pntp->h += dx;
		pntp->v += dy;
	}
}

/* ======================================================================
FUNCTION  doScroll

DESCRIPTION
     Function to handle scroll-bar events.

RETURNED
====================================================================== */
static void
#if XVT_CC_PROTO
doScroll (WINDOW win, EVENT *xdEvent)
#else
doScroll (win, xdEvent)
WINDOW win;
EVENT *xdEvent;
#endif
{
	LEGEND_DATA *legendData = NULL;
	int pos;
   int minValue, maxValue;

	if (!(legendData = (LEGEND_DATA *) xvt_vobj_get_data(win)))
		return;

	switch (xdEvent->type) {
	case E_HSCROLL:
		switch (xdEvent->v.scroll.what) {
		case SC_LINE_UP:
			pos = xvt_sbar_get_pos (win, HSCROLL) - 1;
			break;
		case SC_LINE_DOWN:
			pos = xvt_sbar_get_pos (win, HSCROLL) + 1;
			break;
		case SC_PAGE_UP:
			pos = xvt_sbar_get_pos (win, HSCROLL) - 10;
			break;
		case SC_PAGE_DOWN:
			pos = xvt_sbar_get_pos (win, HSCROLL) + 10;
			break;
		case SC_THUMB:
			pos = xdEvent->v.scroll.pos;
			break;
		case SC_THUMBTRACK:
			pos = xdEvent->v.scroll.pos;
			break;
		}
		xvt_sbar_get_range (win, HSCROLL, &minValue, &maxValue);
		if (pos < minValue) pos = minValue;
		if (pos > maxValue) pos = maxValue;
		xvt_sbar_set_pos (win, HSCROLL, pos);
		break;
	case E_VSCROLL:
		switch (xdEvent->v.scroll.what) {
		case SC_LINE_UP:
			pos = xvt_sbar_get_pos (win, VSCROLL) - 1;
			break;
		case SC_LINE_DOWN:
			pos = xvt_sbar_get_pos (win, VSCROLL) + 1;
			break;
		case SC_PAGE_UP:
			pos = xvt_sbar_get_pos (win, VSCROLL) - 10;
			break;
		case SC_PAGE_DOWN:
			pos = xvt_sbar_get_pos (win, VSCROLL) + 10;
			break;
		case SC_THUMB:
			pos = xdEvent->v.scroll.pos;
			break;
		case SC_THUMBTRACK:
			pos = xdEvent->v.scroll.pos;
			break;
		}
		xvt_sbar_get_range (win, VSCROLL, &minValue, &maxValue);
		if (pos < minValue) pos = minValue;
		if (pos > maxValue) pos = maxValue;
		xvt_sbar_set_pos (win, VSCROLL, pos);
		break;
	}
}

/* ======================================================================
FUNCTION        legendEventHandler

DESCRIPTION
        legend event handler

RETURNED
====================================================================== */
long
#if XVT_CC_PROTO
legendEventHandler (WINDOW win, EVENT *ep)
#else
legendEventHandler (win, ep)
WINDOW win;
EVENT *ep;
#endif
{

   switch (ep->type)
   {
      case E_CREATE:
		   xvt_sbar_set_range (win, HSCROLL, 0, 1000);
		   xvt_sbar_set_range (win, VSCROLL, 0, 200);
			updateMenuOptions (TASK_MENUBAR, NULL_WIN);
			break;
      case E_DESTROY:
			if (win == legendWindow)
			  legendWindow = NULL_WIN;
         if (win == lastActiveWindow)  /* make sure lastActiveWindow is valid */
           lastActiveWindow = NULL_WIN;

			if (legendPixmap)
			{
				xvt_pmap_destroy(legendPixmap);
				legendPixmap = NULL_PIXMAP;
			}

			updateMenuOptions (TASK_MENUBAR, win);
			break;
      case E_CLOSE:
			showLegendWindow(FALSE);
			break;
      case E_COMMAND:
#if (XVTWS == MACWS) || (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
			do_TASK_MENUBAR(win, ep);
#endif
			break;
      case E_HSCROLL:
      case E_VSCROLL:
			doScroll (win, ep);
			xvt_dwin_invalidate_rect (win, NULL);
			break;
      case E_MOUSE_DOWN:
      case E_MOUSE_DBL:
      case E_MOUSE_MOVE:
         break;
      case E_MOUSE_UP:
         bringWindowToFront(win);
         break;
      case E_SIZE:
			xvt_dwin_invalidate_rect (win, NULL);
			break;
		case E_UPDATE:
			drawLegend (win, NULL);
			break;
      case E_FOCUS:
			lastActiveWindow = win;
         break;
   }
   return 0L;
}


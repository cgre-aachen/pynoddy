/*
*/
#include "xvt.h"         /* standard XVT header */
#include "noddy.h"       /* resource constants for this program */
#include "nodInc.h"
#include <math.h>
/* #include "color.h" */

#define DEBUG(X)    

                                /* Constants in this file */
#define TOLERANCE             0.01
#define HIGH_TOLERANCE             0.00001

#define DEFINED_COL_GRID_WIDTH          10
#define DEFINED_COL_GRID_HEIGHT         10

#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
#define DEFINED_COL_GRID_TOP            130
#define DEFINED_COL_GRID_BOTTOM         251
#else
#define DEFINED_COL_GRID_TOP            90
#define DEFINED_COL_GRID_BOTTOM         211
#endif

#if (XVTWS == MACWS)
#define DEFINED_COL_GRID_LEFT           10
#define DEFINED_COL_GRID_RIGHT          191
#else
#if (XVTWS == MTFWS)
#define DEFINED_COL_GRID_LEFT           10
#define DEFINED_COL_GRID_RIGHT          191
#else
#define DEFINED_COL_GRID_LEFT           25
#define DEFINED_COL_GRID_RIGHT          206
#endif
#endif

#define CUSTOM_COL_GRID_WIDTH      20
#define CUSTOM_COL_GRID_HEIGHT          20
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
#define CUSTOM_COL_GRID_TOP        220
#define CUSTOM_COL_GRID_BOTTOM          241
#else
#define CUSTOM_COL_GRID_TOP        180
#define CUSTOM_COL_GRID_BOTTOM          201
#endif
#if (XVTWS == MACWS)
#define CUSTOM_COL_GRID_LEFT       180
#define CUSTOM_COL_GRID_RIGHT      381
#else
#if (XVTWS == MTFWS)
#define CUSTOM_COL_GRID_LEFT       180
#define CUSTOM_COL_GRID_RIGHT      381
#else
#define CUSTOM_COL_GRID_LEFT       235
#define CUSTOM_COL_GRID_RIGHT      436
#endif
#endif

#if (XVTWS == MACWS)
#define STRAT_LAYERS_PIC_WIDTH          300
#define STRAT_LAYERS_PIC_START_X   15
#else
#define STRAT_LAYERS_PIC_WIDTH          380
#define STRAT_LAYERS_PIC_START_X   20
#endif
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
#define STRAT_LAYERS_PIC_HEIGHT         350
#define STRAT_LAYERS_PIC_START_Y   110
#else
#define STRAT_LAYERS_PIC_HEIGHT         300
#define STRAT_LAYERS_PIC_START_Y   80
#endif
#define STRAT_LAYERS_PIC_BORDER_X  10
#define STRAT_LAYERS_PIC_STRAT_WIDTH    30
#define STRAT_LAYERS_PIC_INFO_OFFSET    90

extern double winPositionScale;

                                /* Global variables in this file */
static RCT *predefinedColorSamples = NULL; /* [NUM_PREDEFINED_COLORS]; */
static RCT *customColorSamples = NULL;     /* [NUM_CUSTOM_COLORS]; */
static XVT_PIXMAP stratLayersPicture = NULL_PIXMAP;
COLOR backgroundColor = COLOR_BLACK;

static NODDY_COLOUR *customColors = NULL;
static NODDY_COLOUR *predefinedColors = NULL;
static int NUM_PREDEFINED_COLORS = 0; /* 216 */
static int NUM_CUSTOM_COLORS = 0; /* 10 */

                                /* Functions in this file */
#if XVT_CC_PROTO
NODDY_COLOUR *getPredefinedColors (int *);
NODDY_COLOUR *getCustomColors (int *);
void interactivelySelectColor (WINDOW, PNT);
void makeColorSamples (WINDOW);
void destroyColorSamples (WINDOW);
void makeColorList (WINDOW, int);
void selectColor (WINDOW, int, NODDY_COLOUR);
void setCustomColor (WINDOW, NODDY_COLOUR);
int getColorRGB (char *, unsigned char *, unsigned char *, unsigned char *);
int getColorSelection (WINDOW, NODDY_COLOUR *);
void drawSelectedColorGrid (WINDOW);
void drawDefinedColorGrid (WINDOW);
void drawCustomColorGrid (WINDOW);
void createStratLayersPicture (WINDOW, OBJECT *);
void destroyStratLayersPicture ();
int drawRasterImage (WINDOW, XVT_PIXMAP, RCT);
XVT_PIXMAP createPixmapFromDoubleData (WINDOW, double **, double, double, RCT,
                           NODDY_COLOUR *, double);
XVT_PIXMAP drawDoubleImage (WINDOW, double **, double, double, int, int,
                            NODDY_COLOUR  *, double);
XVT_PIXMAP createRasterImage (WINDOW, unsigned char **, RCT,
                           NODDY_COLOUR *, double);
void destroyRasterImage (XVT_PIXMAP);
void assignGreyLut (NODDY_COLOUR *, COLOR *, int, int, int, int);
void assignPseudoLut (NODDY_COLOUR *, COLOR *, int, int, int, int, int);
int setStratPalet (WINDOW);
#else
NODDY_COLOUR *getPredefinedColors ();
NODDY_COLOUR *getCustomColors ();
void interactivelySelectColor ();
void makeColorSamples ();
void destroyColorSamples ();
void makeColorList ();
void selectColor ();
void setCustomColor ();
int getColorRGB ();
int getColorSelection ();
void drawSelectedColorGrid ();
void drawDefinedColorGrid ();
void drawCustomColorGrid ();
void createStratLayersPicture ();
void destroyStratLayersPicture ();
int drawRasterImage ();
XVT_PIXMAP createPixmapFromDoubleData ();
XVT_PIXMAP drawDoubleImage ();
XVT_PIXMAP createRasterImage ();
void destroyRasterImage ();
void assignGreyLut ();
void assignPseudoLut ();
int setStratPalet ();
#endif


/* ======================================================================
FUNCTION        getPredefinedColors
DESCRIPTION
        select a color if it is clicked on
INPUT

OUTPUT

RETURNED
====================================================================== */
NODDY_COLOUR *
#if XVT_CC_PROTO
getPredefinedColors (int *numColors)
#else
getPredefinedColors (numColors)
int *numColors;
#endif
{
   if (!predefinedColors)
   {
      int color = 0;
      NODDY_COLOUR *lut;

      if (predefinedColors = (NODDY_COLOUR *) create1DArray(216, sizeof(NODDY_COLOUR)))
      {
         lut = &(predefinedColors[color]);
         assignPseudoLut(lut, NULL, 36, 36, 0, 36, 255);
         color += 36;

         lut = &(predefinedColors[color]);
         assignPseudoLut(lut, NULL, 36, 36, 0, 36, 204);
         color += 36;
   
         lut = &(predefinedColors[color]);
         assignPseudoLut(lut, NULL, 36, 36, 0, 36, 153);
         color += 36;

         lut = &(predefinedColors[color]);
         assignPseudoLut(lut, NULL, 36, 36, 0, 36, 102);
         color += 36;

         lut = &(predefinedColors[color]);
         assignPseudoLut(lut, NULL, 36, 36, 0, 36, 51);
         color += 36;

         lut = &(predefinedColors[color]);
         assignGreyLut(lut, NULL, 36, 36, 0, 36);
         color += 36;
      }

      NUM_PREDEFINED_COLORS = color;
      
      for (color = 0; color < NUM_PREDEFINED_COLORS; color++)
      {
         sprintf(predefinedColors[color].name, "Color %d", color+1);
      }
   }

   *numColors = NUM_PREDEFINED_COLORS;
   return (predefinedColors);
}

/* ======================================================================
FUNCTION        getCustomColors
DESCRIPTION
        select a color if it is clicked on
INPUT

OUTPUT

RETURNED
====================================================================== */
NODDY_COLOUR *
#if XVT_CC_PROTO
getCustomColors (int *numColors)
#else
getCustomColors (numColors)
int *numColors;
#endif
{
   if (!customColors)
   {
      int color = 0;
      
      if (customColors = (NODDY_COLOUR *) create1DArray(10, sizeof(NODDY_COLOUR)))
      {
         customColors[color].red = 255;
         customColors[color].green = 255;
         customColors[color].blue = 255;
         strcpy (customColors[color++].name, "Custom Colour 1");
         
         customColors[color].red = 0;
         customColors[color].green = 0;
         customColors[color].blue = 0;
         strcpy (customColors[color++].name, "Custom Colour 2");

         customColors[color].red = 0;
         customColors[color].green = 0;
         customColors[color].blue = 255;
         strcpy (customColors[color++].name, "Custom Colour 3");

         customColors[color].red = 0;
         customColors[color].green = 255;
         customColors[color].blue = 0;
         strcpy (customColors[color++].name, "Custom Colour 4");

         customColors[color].red = 255;
         customColors[color].green = 0;
         customColors[color].blue = 0;
         strcpy (customColors[color++].name, "Custom Colour 5");

         customColors[color].red = 255;
         customColors[color].green = 255;
         customColors[color].blue = 255;
         strcpy (customColors[color++].name, "Custom Colour 6");
         
         customColors[color].red = 0;
         customColors[color].green = 0;
         customColors[color].blue = 0;
         strcpy (customColors[color++].name, "Custom Colour 7");

         customColors[color].red = 0;
         customColors[color].green = 0;
         customColors[color].blue = 255;
         strcpy (customColors[color++].name, "Custom Colour 8");

         customColors[color].red = 0;
         customColors[color].green = 255;
         customColors[color].blue = 0;
         strcpy (customColors[color++].name, "Custom Colour 9");

         customColors[color].red = 255;
         customColors[color].green = 0;
         customColors[color].blue = 0;
         strcpy (customColors[color++].name, "Custom Colour 10");
         
         NUM_CUSTOM_COLORS = color;
      }
   }

   *numColors = NUM_CUSTOM_COLORS;
   return (customColors);
}


/* ======================================================================
FUNCTION        interactivelySelectColor
DESCRIPTION
        select a color if it is clicked on
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
interactivelySelectColor (WINDOW win, PNT point)
#else
interactivelySelectColor (win, point)
WINDOW win;
PNT point;
#endif
{
   int i, foundDefined = FALSE, foundCustom = FALSE;

   for (i = 0; i < NUM_PREDEFINED_COLORS; i++)
   {
      if (xvt_rect_has_point (&(predefinedColorSamples[i]), point))
      {
         foundDefined = TRUE;
         break;
      }
   }

   if (!foundDefined)
   {
      for (i = 0; i < NUM_CUSTOM_COLORS; i++)
      {
         if (xvt_rect_has_point (&(customColorSamples[i]), point))
         {
            foundCustom = TRUE;
            break;
         }
      }
   }

   if (foundDefined)
   {
      selectColor (win, DEFINE_COLOUR_WINDOW_LISTEDIT_41, predefinedColors[i]);
   }
   else if (foundCustom)
   {
      selectColor (win, DEFINE_COLOUR_WINDOW_LISTEDIT_41, customColors[i]);
   }
}
/* ======================================================================
FUNCTION        makeColorSamples
DESCRIPTION
        make up the positions for the predefiend color samples
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
makeColorSamples (WINDOW win)
#else
makeColorSamples (win)
WINDOW win;
#endif
{
   RCT containerPos, colorRect;
   int colorValue;
   WINDOW labelWin;
   RCT labelPos;

                           /* ******************************* */
                           /* first the predefined color list */
   labelWin = xvt_win_get_ctl (win, DEFINE_COLOUR_WINDOW_TEXT_45);
   xvt_vobj_get_outer_rect (labelWin, &labelPos);

   containerPos.top = labelPos.bottom + 5;
   containerPos.bottom = containerPos.top +
                         (DEFINED_COL_GRID_BOTTOM-DEFINED_COL_GRID_TOP);
   containerPos.left = labelPos.left - 2;
   containerPos.right = containerPos.left +
                        (DEFINED_COL_GRID_RIGHT-DEFINED_COL_GRID_LEFT);
   
   colorRect.left = containerPos.left;
   colorRect.right = containerPos.left + DEFINED_COL_GRID_WIDTH;
   colorRect.top = containerPos.top;
   colorRect.bottom = colorRect.top + DEFINED_COL_GRID_HEIGHT;
   colorValue = 0;

   if (!(predefinedColorSamples = (RCT *) xvt_mem_zalloc(NUM_PREDEFINED_COLORS*sizeof(RCT))))
      return;
   if (!(customColorSamples = (RCT *) xvt_mem_zalloc(NUM_CUSTOM_COLORS*sizeof(RCT))))
      return;
      
   do
   {
      predefinedColorSamples[colorValue].left = colorRect.left;
      predefinedColorSamples[colorValue].right = colorRect.right;
      predefinedColorSamples[colorValue].top = colorRect.top;
      predefinedColorSamples[colorValue].bottom = colorRect.bottom;

      if ((colorRect.right + DEFINED_COL_GRID_WIDTH) < containerPos.right)
      {
         colorRect.left += DEFINED_COL_GRID_WIDTH;
         colorRect.right += DEFINED_COL_GRID_WIDTH;
      }
      else
      {
         colorRect.left = containerPos.left;
         colorRect.right = containerPos.left + DEFINED_COL_GRID_WIDTH;
         colorRect.top += DEFINED_COL_GRID_HEIGHT;
         colorRect.bottom += DEFINED_COL_GRID_HEIGHT;
      }

      colorValue++;

   } while ((colorValue < NUM_PREDEFINED_COLORS) &&
                              (colorRect.bottom < containerPos.bottom) &&
                              (colorRect.right < containerPos.right));

                           /* ********************** */
                           /* then the custom colors */
   labelWin = xvt_win_get_ctl (win, DEFINE_COLOUR_WINDOW_TEXT_46);
   xvt_vobj_get_outer_rect (labelWin, &labelPos);

   containerPos.top = labelPos.bottom + 5;
   containerPos.bottom = containerPos.top +
                         (CUSTOM_COL_GRID_BOTTOM-CUSTOM_COL_GRID_TOP);
   containerPos.left = labelPos.left - 2;
   containerPos.right = containerPos.left +
                        (CUSTOM_COL_GRID_RIGHT-CUSTOM_COL_GRID_LEFT);
   
   colorRect.left = containerPos.left;
   colorRect.right = containerPos.left + CUSTOM_COL_GRID_WIDTH;
   colorRect.top = containerPos.top;
   colorRect.bottom = colorRect.top + CUSTOM_COL_GRID_HEIGHT;
   colorValue = 0;

   do
   {
      customColorSamples[colorValue].left = colorRect.left;
      customColorSamples[colorValue].right = colorRect.right;
      customColorSamples[colorValue].top = colorRect.top;
      customColorSamples[colorValue].bottom = colorRect.bottom;

      if ((colorRect.right + CUSTOM_COL_GRID_WIDTH) < containerPos.right)
      {
         colorRect.left += CUSTOM_COL_GRID_WIDTH;
         colorRect.right += CUSTOM_COL_GRID_WIDTH;
      }
      else
      {
         colorRect.left = containerPos.left;
         colorRect.right = containerPos.left + CUSTOM_COL_GRID_WIDTH;
         colorRect.top += CUSTOM_COL_GRID_HEIGHT;
         colorRect.bottom += CUSTOM_COL_GRID_HEIGHT;
      }

      colorValue++;

   } while ((colorValue < NUM_CUSTOM_COLORS) &&
                              (colorRect.bottom < containerPos.bottom) &&
                              (colorRect.right < containerPos.right));
}

void
#if XVT_CC_PROTO
destroyColorSamples (WINDOW win)
#else
destroyColorSamples (win)
WINDOW win;
#endif
{
   xvt_mem_free ((char *) predefinedColorSamples);
   predefinedColorSamples = NULL;

   xvt_mem_free ((char *) customColorSamples);
   customColorSamples = NULL;
}

/* ======================================================================
FUNCTION        makeColorList
DESCRIPTION
        put all the predefiend colors into the list
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
makeColorList (WINDOW parentWin, int listId)
#else
makeColorList (parentWin, listId)
WINDOW parentWin;
int listId;
#endif
{
   WINDOW win;
   SLIST sortedList;
   int i;

   win = xvt_win_get_ctl (parentWin, listId);
   
   if (sortedList = xvt_slist_create())
   {
                     /* ************************************** */
                     /* put in all the predefined colors first */
      for (i = 0; i < NUM_PREDEFINED_COLORS; i++)
      {
         xvt_slist_add_sorted (sortedList, predefinedColors[i].name,
                           0L, TRUE, TRUE);
      }
                     /* **************************** */
                     /* put in all the custom colors */
      for (i = 0; i < NUM_CUSTOM_COLORS; i++)
      {
         xvt_slist_add_sorted (sortedList, customColors[i].name,
                           0L, TRUE, TRUE);
      }

      xvt_list_add (win, -1, (char far *) sortedList);
      xvt_slist_destroy (sortedList);
   }
   else
   {
      xvt_dm_post_error("Memory too low to create Colour Selection List.");
   }

}

/* ======================================================================
FUNCTION        selectColor
DESCRIPTION
        select a predefiend or custom color
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
selectColor (WINDOW parentWin, int listId, NODDY_COLOUR colour)
#else
selectColor (parentWin, listId, colour)
WINDOW parentWin;
int listId;
NODDY_COLOUR colour;
#endif
{
   WINDOW win;
   int i, foundDefined = FALSE, foundCustom = FALSE;
   int enable;

   if (!strstr(colour.name, "Custom Colour"))
   {
      for (i = 0; i < NUM_PREDEFINED_COLORS; i++)
      {
         if (/*(strcmp (colour.name, predefinedColors[i].name) == 0) && */
                    (colour.red == predefinedColors[i].red)
                 && (colour.green == predefinedColors[i].green)
                 && (colour.blue == predefinedColors[i].blue))
         {
            foundDefined = TRUE;
            strcpy (colour.name, predefinedColors[i].name);
            break;
         }
      }
   }

   if (!foundDefined)
   {
      for (i = 0; i < NUM_CUSTOM_COLORS; i++)
      {
         if ((strcmp (colour.name, customColors[i].name) == 0)
              && (colour.red == customColors[i].red)
              && (colour.green == customColors[i].green)
              && (colour.blue == customColors[i].blue))
         {
            foundCustom = TRUE;
            break;
         }
      }
   }

   if (!foundDefined && !foundCustom)  /* Search for something close */
   {
      int closeColor = 1;
      
      while (!foundDefined)
      {
         for (i = 0; i < NUM_PREDEFINED_COLORS; i++)
         {
            if ((abs(colour.red-predefinedColors[i].red) < closeColor) &&
                (abs(colour.green-predefinedColors[i].green) < closeColor) &&
                (abs(colour.blue-predefinedColors[i].blue) < closeColor))
            {
               foundDefined = TRUE;
               strcpy (colour.name, predefinedColors[i].name);
               colour.red = predefinedColors[i].red;
               colour.green = predefinedColors[i].green;
               colour.blue = predefinedColors[i].blue;
               break;
            }
         }
         closeColor++;
      }
   }

   win = xvt_win_get_ctl (parentWin, listId);
   xvt_vobj_set_title (win, colour.name);

   if (foundCustom)
   {
      enable = TRUE;
   }
   else
   {
      enable = FALSE;
   }

   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_104_GROUPBOX_11), (BOOLEAN) enable);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_104_HSCROLL_5), (BOOLEAN) enable);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_104_TEXT_8), (BOOLEAN) enable);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_104_HSCROLL_6), (BOOLEAN) enable);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_104_TEXT_9), (BOOLEAN) enable);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_104_HSCROLL_7), (BOOLEAN) enable);
   xvt_vobj_set_enabled (xvt_win_get_ctl(parentWin, WIN_104_TEXT_10), (BOOLEAN) enable);
   

   updateScrollField (parentWin, WIN_104_HSCROLL_5, colour.red);
   updateScrollField (parentWin, WIN_104_HSCROLL_6, colour.green);
   updateScrollField (parentWin, WIN_104_HSCROLL_7, colour.blue);

   drawSelectedColorGrid (parentWin);
	drawDefinedColorGrid (parentWin);
   drawCustomColorGrid (parentWin);
}

/* ======================================================================
FUNCTION        setCustomColor
DESCRIPTION
        set the custom color described by 'color'.name to the RGB values
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
setCustomColor (WINDOW parentWin, NODDY_COLOUR color)
#else
setCustomColor (parentWin, color)
WINDOW parentWin;
NODDY_COLOUR color;
#endif
{
   int i, foundCustom;

   for (i = 0; i < NUM_CUSTOM_COLORS; i++)
   {
      if (strcmp (color.name, customColors[i].name) == 0)
      {
         foundCustom = TRUE;
         break;
      }
   }

   if (foundCustom)
   {
      customColors[i].red = color.red;
      customColors[i].green = color.green;
      customColors[i].blue = color.blue;
   }

   drawSelectedColorGrid (parentWin);
   drawCustomColorGrid (parentWin);
}

/* ======================================================================
FUNCTION        getColorRGB
DESCRIPTION
        get the rgb value associated with a paticular name
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
getColorRGB (char *name, unsigned char *red,
                         unsigned char *green,
                         unsigned char *blue)
#else
getColorRGB (name, red, green, blue)
char *name;
unsigned char *red, *green, *blue;
#endif
{
   int i, found = FALSE;

   for (i = 0; i < NUM_PREDEFINED_COLORS; i++)
   {
      if (strcmp (name, predefinedColors[i].name) == 0)
      {
         found = TRUE;
         break;
      }
   }

   if (found)
   {
      *red = predefinedColors[i].red;
      *green = predefinedColors[i].green;
      *blue = predefinedColors[i].blue;
      return (found);
   }

                           /* Try the custom colors as well */
   for (i = 0; i < NUM_CUSTOM_COLORS; i++)
   {
      if (strcmp (name, customColors[i].name) == 0)
      {
         found = TRUE;
         break;
      }
   }

   if (found)
   {
      *red = customColors[i].red;
      *green = customColors[i].green;
      *blue = customColors[i].blue;
   }

   return (found);
}

int
#if XVT_CC_PROTO
getSpacedColorRGB (int totalColors, int thisColor, NODDY_COLOUR *color)
#else
getSpacedColorRGB (totalColors, thisColor, color)
int totalColors;
int thisColor;
NODDY_COLOUR *color;
#endif
{
   int colorIndex;
   
   colorIndex = ((NUM_PREDEFINED_COLORS)/(totalColors+1)) * thisColor;
   
   if (colorIndex >= NUM_PREDEFINED_COLORS)
      colorIndex = NUM_PREDEFINED_COLORS-1;

	if (predefinedColors)
		memcpy (color, &(predefinedColors[colorIndex]), sizeof (NODDY_COLOUR));
	else
		memset (color, 0, sizeof (NODDY_COLOUR));
   
   return (TRUE);
}  


/* ======================================================================
FUNCTION        getColorSelection
DESCRIPTION
        get the color structure that defines the current selection
INPUT

OUTPUT

RETURNED
     the index (starting from 1) of the color in the predefined color
        list if the color is in there
     the -ve index (starting from -1) of the color in the custom color
        list if the color is in there
        0 - not a defined color at all
====================================================================== */
int
#if XVT_CC_PROTO
getColorSelection (WINDOW parentWin, NODDY_COLOUR *colour)
#else
getColorSelection (parentWin, colour)
WINDOW parentWin;
NODDY_COLOUR *colour;
#endif
{
   int i, foundDefined = FALSE, foundCustom = FALSE;

   xvt_vobj_get_title (xvt_win_get_ctl(parentWin, DEFINE_COLOUR_WINDOW_LISTEDIT_41),
                             colour->name, COLOUR_NAME_LENGTH);
   colour->red = xvt_sbar_get_pos (xvt_win_get_ctl(parentWin, WIN_104_HSCROLL_5),
                                 HVSCROLL);
   colour->green = xvt_sbar_get_pos (xvt_win_get_ctl(parentWin,WIN_104_HSCROLL_6),
                                 HVSCROLL);
   colour->blue = xvt_sbar_get_pos (xvt_win_get_ctl(parentWin, WIN_104_HSCROLL_7),
                                 HVSCROLL);

   for (i = 0; i < NUM_PREDEFINED_COLORS; i++)
   {
      if ((strcmp (colour->name, predefinedColors[i].name) == 0)
              && (colour->red == predefinedColors[i].red)
              && (colour->green == predefinedColors[i].green)
              && (colour->blue == predefinedColors[i].blue))
      {
         foundDefined = TRUE;
         break;
      }
   }

   if (!foundDefined)
   {
      for (i = 0; i < NUM_CUSTOM_COLORS; i++)
      {
         if ((strcmp (colour->name, customColors[i].name) == 0)
              && (colour->red == customColors[i].red)
              && (colour->green == customColors[i].green)
              && (colour->blue == customColors[i].blue))
         {
            foundCustom = TRUE;
            break;
         }
      }
   }


   if (foundDefined)
      return (i + 1);
   else if (foundCustom)
      return ( -(i + 1));
   else
      return (0);
}

/* ======================================================================
FUNCTION        drawSelectedColorGrid
DESCRIPTION
        draw a grid containg a selection of the defined colors
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
drawSelectedColorGrid (WINDOW parentWin)
#else
drawSelectedColorGrid (parentWin)
WINDOW parentWin;
#endif
{
#define SHADE_1   1.2  /* 0.9 */
#define SHADE_2   0.8  /* 0.8 */
   NODDY_COLOUR selectedColor;
   DRAW_CTOOLS tools;
   int colorSize;
   RCT colorRect, border, control;
   COLOR color, colorUp, colorDown;
	PNT pos;

   getColorSelection (parentWin, &selectedColor);
	color = XVT_MAKE_COLOR(selectedColor.red, selectedColor.green, selectedColor.blue);
	colorUp = XVT_MAKE_COLOR(selectedColor.red*SHADE_1, selectedColor.green*SHADE_1, selectedColor.blue*SHADE_1);
	colorDown = XVT_MAKE_COLOR(selectedColor.red*SHADE_2, selectedColor.green*SHADE_2, selectedColor.blue*SHADE_2);

   xvt_vobj_get_outer_rect(xvt_win_get_ctl(parentWin, DEFINE_COLOUR_WINDOW_GROUPBOX_44), &control);

	border.top = control.top + 5;
   border.bottom = control.bottom;
	colorSize = xvt_rect_get_height(&border)/3;
   border.left = control.right + 5;
   border.right = border.left + colorSize;

             /* clear the area we want to redraw */
   xvt_app_get_default_ctools (&tools);
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.brush.color = (COLOR) xvt_vobj_get_attr (parentWin, ATTR_BACK_COLOR);
   xvt_dwin_set_draw_ctools(parentWin, &tools);
   xvt_dwin_draw_rect (parentWin, &border);

             /* setup to draw colored blocks */
	xvt_rect_set (&colorRect, 0, 0, (short) colorSize, (short) colorSize);
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.pen.width = 0;
   pos.h = border.left;
									/* Color Down */
	pos.v = border.bottom - colorSize;
	tools.brush.color = colorDown;
   tools.pen.color = colorDown;
	xvt_rect_set_pos (&colorRect, pos);
   xvt_dwin_set_draw_ctools(parentWin, &tools);
   xvt_dwin_draw_rect (parentWin, &colorRect);
									/* Color */
	pos.v = border.bottom - colorSize*2;
	tools.brush.color = color;
   tools.pen.color = color;
	xvt_rect_set_pos (&colorRect, pos);
   xvt_dwin_set_draw_ctools(parentWin, &tools);
   xvt_dwin_draw_rect (parentWin, &colorRect);
									/* Color Up */
	pos.v = border.bottom - colorSize*3;
	tools.brush.color = colorUp;
   tools.pen.color = colorUp;
	xvt_rect_set_pos (&colorRect, pos);
   xvt_dwin_set_draw_ctools(parentWin, &tools);
   xvt_dwin_draw_rect (parentWin, &colorRect);
}

/* ======================================================================
FUNCTION        drawDefinedColorGrid
DESCRIPTION
        draw a grid containg a selection of the defined colors
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
drawDefinedColorGrid (WINDOW parentWin)
#else
drawDefinedColorGrid (parentWin)
WINDOW parentWin;
#endif
{
   NODDY_COLOUR colour;
   DRAW_CTOOLS tools;
   int colorValue, selectedColorIndex;
   RCT highlightRect, border;
   COLOR color;

   border.top = predefinedColorSamples[0].top - 3;
   border.bottom = border.top +
                  (DEFINED_COL_GRID_BOTTOM-DEFINED_COL_GRID_TOP) + 5;
   border.left = predefinedColorSamples[0].left - 3;
   border.right = border.left +
                  (DEFINED_COL_GRID_RIGHT-DEFINED_COL_GRID_LEFT) + 5;

             /* clear the area we want to redraw */
   xvt_app_get_default_ctools (&tools);
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.brush.color = (COLOR) xvt_vobj_get_attr (parentWin, ATTR_BACK_COLOR);
   xvt_dwin_set_draw_ctools(parentWin, &tools);
   xvt_dwin_draw_rect (parentWin, &border);

             /* Draw a border around the colors */
   tools.pen.width = 0;
   tools.mode = M_COPY;
   tools.pen.pat = PAT_SOLID;
   tools.pen.width = 1;
   tools.pen.color = COLOR_BLACK;
   tools.brush.pat = PAT_HOLLOW;
   xvt_dwin_set_draw_ctools(parentWin, &tools);
   xvt_dwin_draw_rect (parentWin, &border);

             /* setup to draw colored blocks */
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.pen.width = 0;

   for (colorValue = 0; colorValue < NUM_PREDEFINED_COLORS; colorValue++)
   {
      color = XVT_MAKE_COLOR (predefinedColors[colorValue].red,
                              predefinedColors[colorValue].green,
                              predefinedColors[colorValue].blue);
      tools.brush.color = color;
      tools.pen.color = color;
      xvt_dwin_set_draw_ctools(parentWin, &tools);

      xvt_dwin_draw_rect (parentWin, &predefinedColorSamples[colorValue]);
   }

   selectedColorIndex = getColorSelection (parentWin, &colour) - 1;
   if (selectedColorIndex >= 0)
   {
      tools.pen.pat = PAT_SOLID;
      tools.pen.width = 2;
      tools.pen.color = COLOR_BLACK;
      tools.brush.pat = PAT_HOLLOW;
      xvt_dwin_set_draw_ctools(parentWin, &tools);

      highlightRect.top = predefinedColorSamples[selectedColorIndex].top - 2;
      highlightRect.bottom= predefinedColorSamples[selectedColorIndex].bottom+2;
      highlightRect.left = predefinedColorSamples[selectedColorIndex].left- 2;
      highlightRect.right = predefinedColorSamples[selectedColorIndex].right+ 2;
      xvt_dwin_draw_rect (parentWin, &highlightRect);
   }
}

/* ======================================================================
FUNCTION        drawCustomColorGrid
DESCRIPTION
        draw a grid containg a selection of the defined colors
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
drawCustomColorGrid (WINDOW parentWin)
#else
drawCustomColorGrid (parentWin)
WINDOW parentWin;
#endif
{
   NODDY_COLOUR colour;
   DRAW_CTOOLS tools;
   int colorValue, selectedColorIndex;
   RCT highlightRect, border;
   COLOR color;

   border.top = customColorSamples[0].top - 3;
   border.bottom = border.top +
                  (CUSTOM_COL_GRID_BOTTOM-CUSTOM_COL_GRID_TOP) + 5;
   border.left = customColorSamples[0].left - 3;
   border.right = border.left +
                  (CUSTOM_COL_GRID_RIGHT-CUSTOM_COL_GRID_LEFT) + 5;

             /* clear the area we want to redraw */
   xvt_app_get_default_ctools (&tools);
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.brush.color = (COLOR) xvt_vobj_get_attr (parentWin, ATTR_BACK_COLOR);
   xvt_dwin_set_draw_ctools(parentWin, &tools);
   xvt_dwin_draw_rect (parentWin, &border);

             /* Draw a border around the colors */
   tools.pen.width = 0;
   tools.mode = M_COPY;
   tools.pen.pat = PAT_SOLID;
   tools.pen.width = 1;
   tools.pen.color = COLOR_BLACK;
   tools.brush.pat = PAT_HOLLOW;
   xvt_dwin_set_draw_ctools(parentWin, &tools);
   xvt_dwin_draw_rect (parentWin, &border);

             /* setup to draw colored blocks */
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.pen.width = 0;


   for (colorValue = 0; colorValue < NUM_CUSTOM_COLORS; colorValue++)
   {
      color = XVT_MAKE_COLOR (customColors[colorValue].red,
                          customColors[colorValue].green,
                          customColors[colorValue].blue);
      tools.brush.color = color;
      tools.pen.color = color;
      xvt_dwin_set_draw_ctools(parentWin, &tools);

      xvt_dwin_draw_rect (parentWin, &customColorSamples[colorValue]);
   }

   selectedColorIndex = (-(getColorSelection (parentWin, &colour))) - 1;
   if (selectedColorIndex >= 0)
   {
      tools.pen.pat = PAT_SOLID;
      tools.pen.width = 2;
      tools.pen.color = COLOR_BLACK;
      tools.brush.pat = PAT_HOLLOW;
      xvt_dwin_set_draw_ctools(parentWin, &tools);

      highlightRect.top = customColorSamples[selectedColorIndex].top - 2;
      highlightRect.bottom= customColorSamples[selectedColorIndex].bottom+2;
      highlightRect.left = customColorSamples[selectedColorIndex].left- 2;
      highlightRect.right = customColorSamples[selectedColorIndex].right+ 2;
      xvt_dwin_draw_rect (parentWin, &highlightRect);
   }
}

/* ======================================================================
FUNCTION        createStratLayersPicture
DESCRIPTION
        draw into the given win the picture that describes the
        current stratigraphy setup
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
createStratLayersPicture (WINDOW win, OBJECT *object)
#else
createStratLayersPicture (win, object)
WINDOW win;
OBJECT *object;
#endif
{
   XVT_PIXMAP pixmap;
   STRATIGRAPHY_OPTIONS *stratOptions;
   DRAW_CTOOLS tools;
   COLOR color;
   RCT diagramSize, column;
   int strat, numStrat;
   int totalStratHeight;
   int *layerWidth;
   double percentOfTotal;
   char label[40], remAniLabel[40], densityLabel[40];
   int picWidth, picHeight;
   PNT point;

   if (!(stratOptions = getStratigraphyOptionsStructure (object)))
      return;

   if (!(layerWidth = (int *) xvt_mem_zalloc (stratOptions->numLayers*sizeof(int))))
      return;
      
                             /* position strat column based on surrounds */
   pixmap = win;
                           /* how big the diagram will be */
   xvt_vobj_get_client_rect (win, &diagramSize);
   picWidth = xvt_rect_get_width (&diagramSize);
   picHeight= xvt_rect_get_height(&diagramSize);

   xvt_app_get_default_ctools (&tools);

                           /* clear the area we are about to draw in */
   tools.pen.pat = PAT_HOLLOW;
   tools.pen.color = COLOR_BLACK;
   tools.brush.pat = PAT_SOLID;
   tools.brush.color = (COLOR) xvt_vobj_get_attr (pixmap, ATTR_BACK_COLOR);
   xvt_dwin_set_draw_ctools(pixmap, &tools);
   xvt_dwin_draw_rect (pixmap, &diagramSize);

         /* get the total height of the strat without first & last layers */
   numStrat = stratOptions->numLayers;
   if (numStrat > 2)
   {
      totalStratHeight = (stratOptions->properties[numStrat-1].height
                              - stratOptions->properties[1].height);
      totalStratHeight += (int) (0.2 * totalStratHeight);
   }
   else
   {
      totalStratHeight = 0;
   }


              /* work out the height of each layer */
   for (strat = 0; strat < numStrat; strat++)
   {
                  /* first and last are fixed sizes */
      if ((strat == 0) || (strat == (numStrat-1)))
      {
         if (numStrat == 1)
            percentOfTotal = 1.0;   /* if only one then it fills it */
         else if (numStrat == 2)
            percentOfTotal = 0.5;   /* if there is two then halt each */
         else
            percentOfTotal = 0.1;   /* otherwise first and last get 10% */
      }
      else
      {
         percentOfTotal = (double) (stratOptions->properties[strat+1].height -
                                    stratOptions->properties[strat].height)
                                                         / totalStratHeight;
      }
      layerWidth[strat] = (int) (percentOfTotal * picHeight);
   }

              /* draw a scaled representation of the stratigraphy column */
   column.left = STRAT_LAYERS_PIC_BORDER_X;
   column.right = column.left + STRAT_LAYERS_PIC_STRAT_WIDTH;
   column.top = picHeight;
   for (strat = 0; strat < numStrat; strat++)
   {
      color = XVT_MAKE_COLOR (stratOptions->properties[strat].color.red,
                              stratOptions->properties[strat].color.green,
                              stratOptions->properties[strat].color.blue);
      tools.brush.color = color;
      xvt_dwin_set_draw_ctools(pixmap, &tools);
 
                 /* this comes right on top of the last */
      column.bottom = column.top + 1;
      column.top = column.bottom - layerWidth[strat];

      if (column.top < 0)
         column.top = 0;
      if (column.bottom > picHeight)
         column.bottom = picHeight;
         
             /* Only draw whats big enough to see */
      if (xvt_rect_get_height (&column) < 1)
         continue;

      xvt_dwin_draw_rect (pixmap, &column);

      if (object->generalData == strat+1) /* current Selected Strat */
      {
         RCT indicatorRect;
         
         xvt_rect_set (&indicatorRect, (short) 2, (short) column.top,
			                           (short) (column.left-2), (short) column.bottom);
         
         tools.brush.color = COLOR_BLACK;
         xvt_dwin_set_draw_ctools(pixmap, &tools);

         xvt_dwin_draw_rect (pixmap, &indicatorRect);
      }

/*      if (strat > 0) */
      {
         tools.pen.pat = PAT_RUBBER;
         xvt_dwin_set_draw_ctools(pixmap, &tools);

                                      /* draw in the dividing lines */
         point.h = column.right; point.v = column.bottom;
                                      /* the point the layers change */
         xvt_dwin_draw_set_pos (pixmap, point);
         point.h = STRAT_LAYERS_PIC_INFO_OFFSET - STRAT_LAYERS_PIC_INFO_OFFSET/4;

                                      /* horizontal for a small section */
         xvt_dwin_draw_line (pixmap, point);
         xvt_dwin_draw_set_pos (pixmap, point);

         point.h = STRAT_LAYERS_PIC_INFO_OFFSET;
         point.v = (picHeight/numStrat) * (numStrat-(strat));
         xvt_dwin_draw_line (pixmap, point);  /* to the even spacing area */
         xvt_dwin_draw_set_pos (pixmap, point);
                           
         if (strat > 0)   /* label the heights as the columns are drawn */
         {
            sprintf (label, "%d", stratOptions->properties[strat].height);
            xvt_dwin_draw_text (pixmap, point.h + 2, point.v - 2, label, -1);
         }

         strcpy (remAniLabel,"");
         if ((stratOptions->properties[strat].anisotropicField) &&
             (stratOptions->properties[strat].remanentMagnetization))
            strcat (remAniLabel, "(Ani+Rem)    ");
         else if (stratOptions->properties[strat].anisotropicField)
            strcat (remAniLabel, "(Ani)    ");
         else if (stratOptions->properties[strat].remanentMagnetization)
            strcat (remAniLabel, "(Rem)    ");
         sprintf (densityLabel, "Density: %.1lf", stratOptions->properties[strat].density);
         strcpy (label, remAniLabel);
         strcat (label, densityLabel);
         xvt_dwin_draw_text (pixmap, picWidth-xvt_dwin_get_text_width(pixmap, label, -1)-2,
                                     point.v - 2, label, -1);

         point.h = picWidth - 5;
         xvt_dwin_draw_line (pixmap, point);  /* to the even spacing area */

         tools.pen.pat = PAT_HOLLOW;
         xvt_dwin_set_draw_ctools(pixmap, &tools);
      }
   }
   xvt_mem_free ((char *) layerWidth);
}


/* ======================================================================
FUNCTION        destroyStratLayersPicture
DESCRIPTION
        draw into an offscreen win the picture that describes the
        current stratigraphy setup
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
destroyStratLayersPicture ()
#else
destroyStratLayersPicture ()
#endif
{
   if (stratLayersPicture)
   {
      xvt_pmap_destroy (stratLayersPicture);
      stratLayersPicture = NULL_PIXMAP;
   }
}

/* ======================================================================
FUNCTION        drawRasterImage
DESCRIPTION
        draw the offscreen picture in a window
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
drawRasterImage (WINDOW win, XVT_PIXMAP image, RCT imagePos)
#else
drawRasterImage (win, image, imagePos)
WINDOW win;
XVT_PIXMAP image;
RCT imagePos;
#endif
{
   if (!image)
      return (FALSE);

   xvt_dwin_draw_pmap (win, image, &imagePos, &imagePos);

   return (TRUE);
}


/* ======================================================================
FUNCTION        createPixmapFromDoubleData
DESCRIPTION
        create the offscreen picture from raster pixel information
        allowing for interpolation if needed.
INPUT

OUTPUT

RETURNED
====================================================================== */
XVT_PIXMAP
#if XVT_CC_PROTO
createPixmapFromDoubleData (WINDOW win, double **imageData,
                         double minValue, double maxValue,
                         RCT imageSize, NODDY_COLOUR  *lut, double interpolate)
#else
createPixmapFromDoubleData (win, imageData, minValue, maxValue, imageSize, lut, interpolate)
WINDOW win;
double **imageData;
double minValue, maxValue;
RCT imageSize;
NODDY_COLOUR *lut;
double interpolate;
#endif
{     
   XVT_IMAGE image = (XVT_IMAGE) NULL;
/*   XVT_PALETTE palet; */
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   DRAW_CTOOLS tools;
/*   COLOR color; */
   register double topDifference, bottomDifference, vertDifference;
   double topValue, bottomValue, vertValue;
   double *topLeft, *topRight, *bottomLeft, *bottomRight;
   RCT arraySize;
   double changeRatio;
   register double percent;
   register int x, y, index;
   int pixel, line, lineLength;
   int arrayPixel, arrayLine, width, height;
   double range, increment;
/*   COLOR colors[256];  */

   range = maxValue - minValue;
   increment = range/256.0;

   if (fabs(interpolate - 1.0) > TOLERANCE)
   {                     /* take a copy of how bit the array is */
      arraySize.top = imageSize.top;
      arraySize.bottom = imageSize.bottom;
      arraySize.left = imageSize.left;
      arraySize.right = imageSize.right;
                         /* scale bounding rect for interpolated image */
      imageSize.top = (short) (imageSize.top * interpolate);
      imageSize.bottom = (short) ((imageSize.bottom-1) * interpolate);
      imageSize.left = (short) (imageSize.left * interpolate);
      imageSize.right = (short) ((imageSize.right-1) * interpolate);
	}
	else
	{
		arraySize = imageSize;
	}

   xvt_app_get_default_ctools (&tools);
   
   if (!(image = xvt_image_create (XVT_IMAGE_RGB,
                                   (short) (imageSize.right-imageSize.left),
                                   (short) (imageSize.bottom-imageSize.top), NULL)))
   {
      xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }

#ifdef OLD
   if (!(palet = xvt_palet_create (XVT_PALETTE_USER, NULL)))
   {
      xvt_image_destroy (image);
      xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }   
   xvt_palet_set_tolerance (palet, 0L);  /* disable closness of colors */


   for (arrayPixel = 0; arrayPixel < 255; arrayPixel++)
   {
      colors[arrayPixel] = XVT_MAKE_COLOR (lut[arrayPixel].red,
                                           lut[arrayPixel].green,
                                           lut[arrayPixel].blue);
   }
   xvt_palet_add_colors (palet, colors, arrayPixel);
#endif


   if (!(pixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                                   (short) (imageSize.right-imageSize.left),
                                   (short) (imageSize.bottom-imageSize.top), NULL)))
   {
      xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }

#ifdef OLD
   xvt_vobj_set_palet (pixmap, palet); 
   xvt_vobj_set_palet (win, palet); 
#endif

                           /* clear the area we are about to draw in */
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.brush.color = (COLOR) xvt_vobj_get_attr (win, ATTR_BACK_COLOR);
   xvt_dwin_set_draw_ctools(pixmap, &tools);
   xvt_dwin_draw_rect (pixmap, &imageSize);


   if (fabs(interpolate - 1.0) > TOLERANCE)
   {
                           /* ratio for each pixel in interpolation */
      changeRatio = 1.0 / (interpolate - 1.0);
      lineLength = arraySize.right - arraySize.left;
            /* minus 1 from width, height because interpolation looks 1 ahead */
      width = arraySize.right - arraySize.left - 1;
      height = arraySize.bottom - arraySize.top - 1;

      for (arrayLine = 0; arrayLine < height; arrayLine++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (arrayPixel = 0; arrayPixel < width; arrayPixel++)
         {
            topLeft = &(imageData[arrayLine][arrayPixel]);
            topRight = &(imageData[arrayLine][arrayPixel+1]);
            bottomLeft = &(imageData[arrayLine+1][arrayPixel]);
            bottomRight = &(imageData[arrayLine+1][arrayPixel+1]);
                     /* *********************************************** */
                     /* now we interpolate to fill in this little block */
            topDifference = (double) *topRight - *topLeft;
            bottomDifference = (double) *bottomRight - *bottomLeft;
            for (x = (int) (arrayPixel*interpolate) + imageSize.left, pixel = 0;
                                            pixel < interpolate;
                                                            pixel++, x++)
            {
                       /* work out the percentage of the difference to add on */
               percent = (double) pixel * changeRatio;
                           /* first interpolate across top */
               topValue = (*topLeft + (double) (percent * topDifference));
                           /* then interpolate across bottom */
               bottomValue = (*bottomLeft + (double) (percent * bottomDifference));

                    /* now interpolate down from top to bottom interpolations */
               vertDifference = (double) bottomValue - topValue;
               for (y = (int) (arrayLine*interpolate) + imageSize.top, line = 0;
                                             line < interpolate;
                                                            line++, y++)
               {
                       /* work out the percentage of the difference to add on */
                  percent = (double) line * changeRatio;
                  vertValue = topValue + (percent * vertDifference);
                  
                  if (range > HIGH_TOLERANCE)
                     index = (int) floor ((vertValue - minValue)/increment + 0.5);
                  else
                     index = 0;
                  if (index < 0) index = 0;
                  if (index > 255) index = 255;
                  xvt_image_set_pixel (image, (short) x, (short) y,
                                       XVT_MAKE_COLOR (lut[index].red,
                                                       lut[index].green,
                                                       lut[index].blue));
               }
            }
         }
      }
   }
   else          /* no interpolation, just straight image */
   {
      pixel = 0;
      lineLength = imageSize.right - imageSize.left;
      for  (y = imageSize.top; y < imageSize.bottom; y++)
      {
         incrementLongJob (INCREMENT_JOB);
         for  (x = imageSize.left; x < imageSize.right; x++)
         {
            vertValue = imageData[y-imageSize.top][x-imageSize.left];

            if (range > HIGH_TOLERANCE)
               index = (int) floor ((vertValue - minValue)/increment + 0.5);
            else
               index = 0;
            if (index < 0) index = 0;
            if (index > 255) index = 255;
            xvt_image_set_pixel (image, (short) x, (short) y,
                                 XVT_MAKE_COLOR (lut[index].red,
                                                 lut[index].green,
                                                 lut[index].blue));
            pixel++;
         }
      }
   }

   xvt_dwin_draw_image (pixmap, image, &imageSize, &imageSize); 
/*   xvt_palet_destroy (palet); */
   xvt_image_destroy (image);

   return (pixmap);
}

/* ======================================================================
FUNCTION        drawDoubleImage
DESCRIPTION
        create the offscreen picture from raster pixel information
        allowing for interpolation if needed.
INPUT

OUTPUT

RETURNED
====================================================================== */
XVT_PIXMAP
#if XVT_CC_PROTO
drawDoubleImage (WINDOW win, double **imageData, double minValue, double maxValue,
                 int xSize, int ySize, NODDY_COLOUR  *lut, double interpolate)
#else
drawDoubleImage (win, imageData, minValue, maxValue, xSize, ySize, lut, interpolate)
WINDOW win;
double **imageData;
double minValue, maxValue;
int xSize, ySize;
NODDY_COLOUR *lut;
double interpolate;
#endif
{     
#ifdef OLD_INTERP
   XVT_IMAGE image = (XVT_IMAGE) NULL;
/*   XVT_PALETTE palet; */
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   DRAW_CTOOLS tools;
   COLOR color;
   register double topDifference, bottomDifference, vertDifference;
   double topValue, bottomValue, vertValue;
   double *topLeft, *topRight, *bottomLeft, *bottomRight;
   RCT arraySize;
   double changeRatio;
   register double percent;
   register int x, y, index;
   int pixel, line, lineLength;
   int arrayPixel, arrayLine, width, height;
   double range, increment;
   COLOR colors[256]; 
   RCT imageSize;
   
   xvt_rect_set(&imageSize, 0, 0, xSize, ySize);

   range = maxValue - minValue;
   increment = range/256.0;

   if (fabs(interpolate - 1.0) > TOLERANCE)
   {                     /* take a copy of how bit the array is */
      arraySize.top = imageSize.top;
      arraySize.bottom = imageSize.bottom;
      arraySize.left = imageSize.left;
      arraySize.right = imageSize.right;
                         /* scale bounding rect for interpolated image */
      imageSize.top = (short) (imageSize.top * interpolate);
      imageSize.bottom = (short) ((imageSize.bottom-1) * interpolate);
      imageSize.left = (short) (imageSize.left * interpolate);
      imageSize.right = (short) ((imageSize.right-1) * interpolate);
   }

   xvt_app_get_default_ctools (&tools);
   
   if (!(image = xvt_image_create (XVT_IMAGE_RGB,
                                   imageSize.right-imageSize.left,
                                   imageSize.bottom-imageSize.top, NULL)))
   {
      xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }

   if (!(pixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                                   imageSize.right-imageSize.left,
                                   imageSize.bottom-imageSize.top, NULL)))
   {
      xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }

                           /* clear the area we are about to draw in */
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.brush.color = (COLOR) xvt_vobj_get_attr (win, ATTR_BACK_COLOR);
   xvt_dwin_set_draw_ctools(pixmap, &tools);
   xvt_dwin_draw_rect (pixmap, &imageSize);


   if (fabs(interpolate - 1.0) > TOLERANCE)
   {
                           /* ratio for each pixel in interpolation */
      changeRatio = 1.0 / (interpolate - 1.0);
      lineLength = arraySize.right - arraySize.left;
            /* minus 1 from width, height because interpolation looks 1 ahead */
      width = arraySize.right - arraySize.left - 1;
      height = arraySize.bottom - arraySize.top - 1;

      for (arrayLine = 0; arrayLine < height; arrayLine++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (arrayPixel = 0; arrayPixel < width; arrayPixel++)
         {
            topLeft = &(imageData[arrayLine][arrayPixel]);
            topRight = &(imageData[arrayLine][arrayPixel+1]);
            bottomLeft = &(imageData[arrayLine+1][arrayPixel]);
            bottomRight = &(imageData[arrayLine+1][arrayPixel+1]);
                     /* *********************************************** */
                     /* now we interpolate to fill in this little block */
            topDifference = (double) *topRight - *topLeft;
            bottomDifference = (double) *bottomRight - *bottomLeft;
            for (x = (int) (arrayPixel*interpolate) + imageSize.left, pixel = 0;
                                            pixel < interpolate;
                                                            pixel++, x++)
            {
                       /* work out the percentage of the difference to add on */
               percent = (double) pixel * changeRatio;
                           /* first interpolate across top */
               topValue = (*topLeft + (double) (percent * topDifference));
                           /* then interpolate across bottom */
               bottomValue = (*bottomLeft + (double) (percent * bottomDifference));

                    /* now interpolate down from top to bottom interpolations */
               vertDifference = (double) bottomValue - topValue;
               for (y = (int) (arrayLine*interpolate) + imageSize.top, line = 0;
                                             line < interpolate;
                                                            line++, y++)
               {
                       /* work out the percentage of the difference to add on */
                  percent = (double) line * changeRatio;
                  vertValue = topValue + (percent * vertDifference);
                  
                  if (range > HIGH_TOLERANCE)
                     index = (int) floor ((vertValue - minValue)/increment + 0.5);
                  else
                     index = 0;
                  if (index < 0) index = 0;
                  if (index > 255) index = 255;
                  xvt_image_set_pixel (image, x, y,
                                       XVT_MAKE_COLOR (lut[index].red,
                                                       lut[index].green,
                                                       lut[index].blue));
               }
            }
         }
      }
   }
   else          /* no interpolation, just straight image */
   {
      pixel = 0;
      lineLength = imageSize.right - imageSize.left;
      for  (y = imageSize.top; y < imageSize.bottom; y++)
      {
         incrementLongJob (INCREMENT_JOB);
         for  (x = imageSize.left; x < imageSize.right; x++)
         {
            vertValue = imageData[y-imageSize.top][x-imageSize.left];

            if (range > HIGH_TOLERANCE)
               index = (int) floor ((vertValue - minValue)/increment + 0.5);
            else
               index = 0;
            if (index < 0) index = 0;
            if (index > 255) index = 255;
            xvt_image_set_pixel (image, x, y,
                                 XVT_MAKE_COLOR (lut[index].red,
                                                 lut[index].green,
                                                 lut[index].blue));
            pixel++;
         }
      }
   }

   xvt_dwin_draw_image (pixmap, image, &imageSize, &imageSize);
   xvt_image_destroy (image);

   return (pixmap);
#endif

#ifdef NEW_INTERP
#endif
   XVT_IMAGE image = (XVT_IMAGE) NULL;
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   register int x, y, index;
   double range, increment, value;
   double **tempImage = NULL;
   int resultX, resultY;
   RCT imageSize;

   range = maxValue - minValue;
   increment = range/256.0;

   resultX = (int) floor (((double) xSize)*interpolate + 0.5);
   resultY = (int) floor (((double) ySize)*interpolate + 0.5);
   
   if (!(tempImage = (double **) create2DArray (resultY, resultX, sizeof(double))))
      return (NULL_PIXMAP);

   if (!interpDblArray (imageData, tempImage, ySize, xSize, resultY, resultX, 1.0, 0.0))
      return (NULL_PIXMAP);
/*
   if (!mapArrayToArray(imageData, xSize,   ySize,   10000.0, 0.0, 0.0, 10000.0,
                        tempImage, resultX, resultY, 10000.0, 0.0, 0.0, 10000.0, FALSE))
      return (NULL_PIXMAP);
*/
   
   if (!(image = xvt_image_create (XVT_IMAGE_RGB, (short) resultX, (short) resultY, NULL)))
   {
      destroy2DArray ((char **) tempImage, resultY, resultX);
      return (NULL_PIXMAP);
   }

   if (!(pixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT, (short) resultX, (short) resultY, NULL)))
   {
      destroy2DArray ((char **) tempImage, resultY, resultX);
      return (NULL_PIXMAP);
   }

                           /* clear the area we are about to draw in */
   for  (y = 0; y < resultY; y++)
   {
      incrementLongJob (INCREMENT_JOB);
      for  (x = 0; x < resultX; x++)
      {
         value = tempImage[y][x];

         if (range > HIGH_TOLERANCE)
         {
            index = (int) floor ((value - minValue)/increment + 0.5);
            if (index < 0)
               index = 0;
            else if (index > 255)
               index = 255;
         }
         else
            index = 0;
         
         xvt_image_set_pixel (image, (short) x, (short) y,
			XVT_MAKE_COLOR(lut[index].red, lut[index].green, lut[index].blue));
      }
   }

   destroy2DArray ((char **) tempImage, resultY, resultX);
   xvt_rect_set (&imageSize, (short) 0, (short) 0, (short) resultX, (short) resultY);
   xvt_dwin_draw_image (pixmap, image, &imageSize, &imageSize);
   xvt_image_destroy (image);

   return (pixmap);
}

/* ======================================================================
FUNCTION        createRasterImage
DESCRIPTION
        create the offscreen picture from raster pixel information
        allowing for interpolation if needed.
INPUT

OUTPUT

RETURNED
====================================================================== */
XVT_PIXMAP
#if XVT_CC_PROTO
createRasterImage (WINDOW win, unsigned char **imageData, RCT imageSize,
                   NODDY_COLOUR  *lut, double interpolate)
#else
createRasterImage (win, imageData, imageSize, lut, interpolate)
WINDOW win;
unsigned char **imageData;
RCT imageSize;
NODDY_COLOUR *lut;
double interpolate;
#endif
{     
   XVT_IMAGE image = (XVT_IMAGE) NULL;
/*   XVT_PALETTE palet; */
/*   COLOR color; */
   XVT_PIXMAP pixmap = (XVT_PIXMAP) NULL;
   DRAW_CTOOLS tools;
   register double topDifference, bottomDifference, vertDifference;
   unsigned char topValue, bottomValue, vertValue;
   unsigned char *topLeft, *topRight, *bottomLeft, *bottomRight;
   RCT arraySize;
   double changeRatio;
   register double percent;
   register int x, y;
   int pixel, line, lineLength;
   int arrayPixel, arrayLine, width, height;

   if (fabs(interpolate - 1.0) > TOLERANCE)
   { 
                         /* take a copy of how bit the array is */
      arraySize.top = imageSize.top;
      arraySize.bottom = imageSize.bottom;
      arraySize.left = imageSize.left;
      arraySize.right = imageSize.right;
                         /* scale bounding rect for interpolated image */
      imageSize.top = (short) (imageSize.top * interpolate);
      imageSize.bottom = (short) ((imageSize.bottom-1) * interpolate);
      imageSize.left = (short) (imageSize.left * interpolate);
      imageSize.right = (short) ((imageSize.right-1) * interpolate);
   }

   xvt_app_get_default_ctools (&tools);
   
   if (!(image = xvt_image_create (XVT_IMAGE_RGB,
                                   (short) (imageSize.right-imageSize.left),
                                   (short) (imageSize.bottom-imageSize.top), NULL)))
   {
      xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }

#ifdef OLD
   if (!(palet = xvt_palet_create (XVT_PALETTE_USER, NULL)))
   {
      xvt_image_destroy (image);
      xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }   
   xvt_palet_set_tolerance (palet, 0L);  /* disable closness of colors */

   for (arrayPixel = 0; arrayPixel < 255; arrayPixel++)
   {
      color = XVT_MAKE_COLOR (lut[arrayPixel].red, lut[arrayPixel].green,
                                                   lut[arrayPixel].blue);
      xvt_palet_add_colors (palet, &color, 1);
   }
#endif

   if (!(pixmap = xvt_pmap_create (win, XVT_PIXMAP_DEFAULT,
                                   (short) (imageSize.right-imageSize.left),
                                   (short) (imageSize.bottom-imageSize.top), NULL)))
   {
      xvt_dm_post_error("Cannot Create Raster Image.");
      return (NULL_PIXMAP);
   }

#ifdef OLD
   xvt_vobj_set_palet (pixmap, palet);
   xvt_vobj_set_palet (win, palet);
#endif

                           /* clear the area we are about to draw in */
   tools.pen.pat = PAT_HOLLOW;
   tools.brush.pat = PAT_SOLID;
   tools.brush.color = (COLOR) xvt_vobj_get_attr (win, ATTR_BACK_COLOR);
   xvt_dwin_set_draw_ctools(pixmap, &tools);
   xvt_dwin_draw_rect (pixmap, &imageSize);


   if (fabs(interpolate - 1.0) > TOLERANCE)
   {
                           /* ratio for each pixel in interpolation */
      changeRatio = 1.0 / (interpolate - 1.0);
      lineLength = arraySize.right - arraySize.left;
            /* minus 1 from width, height because interpolation looks 1 ahead */
      width = arraySize.right - arraySize.left - 1;
      height = arraySize.bottom - arraySize.top - 1;

      for (arrayLine = 0; arrayLine < height; arrayLine++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (arrayPixel = 0; arrayPixel < width; arrayPixel++)
         {
            topLeft = &(imageData[arrayLine][arrayPixel]);
            topRight = &(imageData[arrayLine][arrayPixel+1]);
            bottomLeft = &(imageData[arrayLine+1][arrayPixel]);
            bottomRight = &(imageData[arrayLine+1][arrayPixel+1]);
                     /* *********************************************** */
                     /* now we interpolate to fill in this little block */
            topDifference = (double) *topRight - *topLeft;
            bottomDifference = (double) *bottomRight - *bottomLeft;
            for (x = (int) (arrayPixel*interpolate) + imageSize.left, pixel = 0;
                                            pixel < interpolate;
                                                            pixel++, x++)
            {
                       /* work out the percentage of the difference to add on */
               percent = (double) pixel * changeRatio;
                           /* first interpolate across top */
               topValue = (int) (*topLeft + (double) (percent * topDifference));
                           /* then interpolate across bottom */
               bottomValue = (int) (*bottomLeft + (double) (percent * bottomDifference));

                    /* now interpolate down from top to bottom interpolations */
               vertDifference = (double) bottomValue - topValue;
               for (y = (int) (arrayLine*interpolate) + imageSize.top, line = 0;
                                             line < interpolate;
                                                            line++, y++)
               {
                       /* work out the percentage of the difference to add on */
                  percent = (double) line * changeRatio;
                  vertValue = (unsigned char) ((double) topValue
                                         + (double) (percent * vertDifference));

                  xvt_image_set_pixel (image, (short) x, (short) y,
                                       XVT_MAKE_COLOR (lut[vertValue].red,
                                                       lut[vertValue].green,
                                                       lut[vertValue].blue));
               }
            }
         }
      }
   }
   else          /* no interpolation, just straight image */
   {
      pixel = 0;
      lineLength = imageSize.right - imageSize.left;
      for  (y = imageSize.top; y < imageSize.bottom; y++)
      {
         incrementLongJob (INCREMENT_JOB);
         for  (x = imageSize.left; x < imageSize.right; x++)
         {
            vertValue = (unsigned char ) imageData[y-imageSize.top][x-imageSize.left];
            xvt_image_set_pixel (image, (short) x, (short) y,
                                 XVT_MAKE_COLOR (lut[vertValue].red,
                                                 lut[vertValue].green,
                                                 lut[vertValue].blue));
            pixel++;
         }
      }
   }

   xvt_dwin_draw_image (pixmap, image, &imageSize, &imageSize);
/*   xvt_palet_destroy (palet); */
   xvt_image_destroy (image);

   return (pixmap);
}

/* ======================================================================
FUNCTION        destroyRasterImage
DESCRIPTION
        destroy the memory assiciated with a raster image
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
destroyRasterImage (XVT_PIXMAP pixmap)
#else
destroyRasterImage (pixmap)
XVT_PIXMAP pixmap;
#endif
{
   if (pixmap)
   {
      xvt_pmap_destroy (pixmap);
      pixmap = (XVT_PIXMAP) 0;
   }
}

/* ======================================================================
FUNCTION        assignGreyLut
DESCRIPTION
        assign the lut indicies to reflect a grey scale
        with rgb ranging between 0 - 255
INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
assignGreyLut (NODDY_COLOUR *lut, COLOR *clut, int size, int numGrey,
              int startIndex, int endIndex)
#else
assignGreyLut (lut, clut, size, numGrey, startIndex, endIndex)
NODDY_COLOUR *lut;
COLOR *clut;
int size;
int numGrey;
int startIndex, endIndex;
#endif
{
   int i, color;
   double increment;
   int incEvery;
   int range;
   
   range = endIndex - startIndex;

   if (range <= numGrey)
   {
      increment = (double) ((double) 256.0 / (double) (numGrey-1));
      incEvery = 1;
   }
   else /* range > numGrey */
   {
      increment = (double) ((double) 256.0 / (double) (numGrey-1));
      incEvery = range / numGrey;
   }

   color = 0;
   for (i = 0; i < startIndex; i++)
   {
      if (lut)
      {
         lut[i].red = color;
         lut[i].green = color;
         lut[i].blue = color;
      }

      if (clut)
         clut[i] = XVT_MAKE_COLOR(color, color, color);
   }

   for (; i < endIndex; i++)
   {
      if (lut)
      {
         lut[i].red = lut[i].green = lut[i].blue = color;
      }
      
      if (clut)
         clut[i] = XVT_MAKE_COLOR(color, color, color);

      if  (i && (!(i%incEvery)))
      {
         color = (int) (color + increment);
         if (color > 255)
            color = 255;
      }
   }

   for (; i < size; i++)
   {
      if (lut)
      {
         lut[i].red = color;
         lut[i].green = color;
         lut[i].blue = color;
      }

      if (clut)
         clut[i] = XVT_MAKE_COLOR(color, color, color);
   }
}

/* ======================================================================
FUNCTION        assignPseudoLut
DESCRIPTION
        assign the lut indicies to reflect a pseudo Color scale

INPUT

OUTPUT

RETURNED
====================================================================== */
void
#if XVT_CC_PROTO
assignPseudoLut (NODDY_COLOUR *lut, COLOR *clut, int size, int numColor,
                 int startIndex, int endIndex, int saturation)
#else
assignPseudoLut (lut, clut, size, numColor, startIndex, endIndex, saturation)
NODDY_COLOUR *lut;
COLOR *clut;
int size;
int numColor;
int startIndex, endIndex;
int saturation;
#endif
{
   int i, colorRed, colorGreen, colorBlue;
   double increment;
   int incEvery;
   int range;

   if (saturation < 1)
      saturation = 255;
   if (size < endIndex)
      endIndex = size;
      
   range = endIndex - startIndex;

   if (range <= numColor)
   {
      increment = (double) ((double) (saturation+1.0) / (double) (numColor-1));
      incEvery = 1;
   }
   else /* range > numGrey */
   {
      increment = (double) ((double) (saturation+1.0) / (double) (numColor-1));
      incEvery = range / numColor;
   }

   colorRed = saturation;  /* 0 */
   colorGreen = 0;
   colorBlue = saturation;
   for (i = 0; i < startIndex; i++)
   {
      if (lut)
      {
         lut[i].red = colorRed;
         lut[i].green = colorGreen;
         lut[i].blue = colorBlue;
      }

      if (clut)
         clut[i] = XVT_MAKE_COLOR(colorRed, colorGreen, colorBlue);
   }
   
   {
      int fifthColors, fifthRange;
      int colorInc, ii;
      
      fifthColors = numColor / 5;
      fifthRange = range / 5;
      colorInc = saturation/fifthColors;

      for (; i < endIndex; i++)
      {
         if (lut)
         {
            lut[i].red = colorRed;
            lut[i].green = colorGreen;
            lut[i].blue = colorBlue;
         }

         if (clut)
            clut[i] = XVT_MAKE_COLOR(colorRed, colorGreen, colorBlue);

         if  ((i && (!(i%incEvery))) || (incEvery == 1))
         {
            ii = i - startIndex;
            
                           /* Setup the colors to draw the contours */
            if (ii < fifthRange*1)
            {
               colorRed -= colorInc;
               colorGreen = 0;
               colorBlue = saturation;
            }
            else if (ii < fifthRange*2)
            {
               colorRed = 0;
               colorGreen += colorInc;
               colorBlue = saturation;
            }
            else if (ii < fifthRange*3)
            {
               colorRed = 0;
               colorGreen = saturation;
               colorBlue -= colorInc;
            }
            else if (ii < fifthRange*4)
            {
               colorRed += colorInc;
               colorGreen = saturation;
               colorBlue = 0;
            }
            else if (ii < fifthRange*5)
            {
               colorRed = saturation;
               colorGreen -= colorInc;
               colorBlue = 0;
            }

            if (colorRed > 255)  /* Make sure we stay in range */
               colorRed = 255;
            else if (colorRed < 0)
               colorRed = 0;
               
            if (colorGreen > 255)
               colorGreen = 255;
            else if (colorGreen < 0)
               colorGreen = 0;
               
            if (colorBlue > 255)
               colorBlue = 255;
            else if (colorBlue < 0)
               colorBlue = 0;
         }
      }
   }

   for (; i < size; i++)
   {
      if (lut)
      {
         lut[i].red = saturation;
         lut[i].green = 0;
         lut[i].blue = 0;
      }

      if (clut)
         clut[i] = XVT_MAKE_COLOR(colorRed, colorGreen, colorBlue);
   }
   
}

/* ======================================================================
FUNCTION        setStratPalet
DESCRIPTION
        make sure the window passed in can display all the colors needed
        for the stratigraphy
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
setStratPalet (WINDOW win)
#else
setStratPalet (win)
WINDOW win;
#endif
{
   XVT_PALETTE winPalette;

   if (!win)
      return (FALSE);
      
   if (winPalette = xvt_vobj_get_palet(TASK_WIN))
      xvt_vobj_set_palet(win, winPalette);

   return (TRUE);
}



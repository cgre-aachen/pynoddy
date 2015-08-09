/*****************************************************************************
 *	File:		3dText.c
 *
 *	Purpose:	
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "xvt.h"
#include "3dDraw.h"
#include "3dText.h"

#ifdef PAUL
/*****************************************************************************
 *
 *	Function:	StringTo3d(É)
 *
 *	Purpose:	Via a call to BitMapTo3d converts a string to a set of three
 *				dimensional vertices representing the exterior of the separate
 *				characters in the text string.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if problems allocating memory
 *				TRUE				if successful
 *
 *****************************************************************************
 */
int StringTo3d (char *str,	/*	input pascal type string	*/
		Point3d	**ppPt,	/*	output array of vertices	*/
		int		*pInt	/*	output vertex count		*/
	)
{
	RCT R;
	int fontHeight;
	int strSize;
	XVT_FNTID fontId;
	int fontLeading, fontAscent, fontDecent;
	XVT_PIXMAP pixmap;
	XVT_IMAGE image;

	/*	based on the font metrics, determine how large a bitMap we need,
	 *	and allocate it.
	 */
	fontId = xvt_dwin_get_font (currentWin);
	xvt_font_get_metrics (fontId, &fontLeading, &fontAscent, &fontDecent);
	fontHeight = fontAscent + fontDecent + fontLeading;
	strSize = strlen(str);

	xvt_rect_set(&R, 0, 0, xvt_dwin_text_width(currentWin, str, strSize) + 40,
	                       40 + fontHeight);

	/*	set up a port (offscreen) in which to draw the string, so we
	 *	can get access to it as a bit map rather than as text
	 */
	if (!(pixmap = xvt_pmap_create (currentWin, XVT_PIXMAP_DEFAULT,
				xvt_rect_get_width(&R), xvt_rect_get_height(&R), 0L)))
		return (FALSE);
	if (!(image = xvt_image_create (XVT_IMAGE_CL8,
				xvt_rect_get_width(&R), xvt_rect_get_height(&R), 0L)))
	{
		xvt_pmap_destroy (pixmap);
		return (FALSE);
	}
	xvt_dwin_set_font (pixmap, fontId);

	/*	draw the string (ie: generate its bitMap )
	 */                                                               
	xvt_dwin_clear (pixmap, COLOR_BLACK);
	xvt_dwin_set_std_cpen (pixmap, TL_PEN_WHITE);
	xvt_dwin_draw_text(pixmap, 0, fontLeading+fontAscent, str, strSize);
	xvt_image_get_from_pmap (image, pixmap, &R, &R);

	/*	don't need the port any longer, so get rid of it
	 */
	xvt_pmap_destroy (pixmap);

	/*	call the funtion that does the real work of converting the
	 *	bitMap into an array of vertices
	 */
	if (BitMapTo3d(image, ppPt, pInt ) != TRUE)
	{
		/*	too bad, too little memory, or other trouble
		 */
		xvt_image_destroy (image);
		return MEMORY_ALLOC_ERROR;
	}
	/*	don't need the port any longer, so get rid of it
	 */
	xvt_image_destroy (image);
	
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	DrawString3d(É)
 *
 *	Purpose:	Draws a conventional Quickdraw Text string at a point in three
 *				dimensional space.
 *
 *****************************************************************************
 */
void	DrawString3d (Fixed x, Fixed y, Fixed z,
		char *str,		/*	the pascal string to be drawn		*/
		enum alignment	aligned)
{
	int length;
	
	length = strlen(str);
	
	Moveto3d( x, y, z );
	switch( aligned )
	{
		case leftAlign:
			break;
		case rightAlign:
			Move(-length,0);
			break;
		case centerAlign:
			Move(-(length/2),0);
			break;
	}
	DrawString( (unsigned char *)str );
}          
#endif

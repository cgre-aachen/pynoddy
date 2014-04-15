//
// A Class that controls the drawing of the Paramater Editing control in open GL
//
//
#ifndef _CPARAMEDITOR_H_
#define _CPARAMEDITOR_H_

extern "C" 
{
#include "xvt.h"
}
#if XVTWS == WIN32WS
#undef BOOLEAN          // undefine a few things in XVT that interfear with 'windows.h'
typedef short SHORT;
typedef long LONG;
typedef char CHAR;
#include <windows.h>
#else  // Motif
#endif

#include "CParamHa.h"

class CParamEditor
{
//
// Public Definitions
//
public:
	CParamEditor();			// Constructor
	~CParamEditor();			// Descructor

#if XVTWS == WIN32WS
	void setWindow(HDC pWindow);
#else
	void setWindow(WINDOW pWindow);
#endif
	int addDipDirHandle();
	int addDipHandle();
	int addPitchHandle();
   int addParamHandle(CParamHandle *newParamHandle);
	CParamHandle *getParamHandleNumber(int) const;

	void setRotateX(double dwRotateX);
	double getRotateX() const;
	void setRotateY(double dwRotateY);
	double getRotateY() const;

	void setOrigin(double dwOriginX = 0.0, double dwOriginY = 0.0, double dwOriginZ = 0.0);
	void getOrigin(double *pdwOriginX, double *pdwOriginY, double *pdwOriginZ) const;
	void setMousePos(int nMouseXPos, int nMouseYPos);
	void getMousePos(int *pnMouseXPos, int *pnMouseYPos) const;
	void setMouseButtonDown(int nMouseButtonDown);
	int getMouseButtonDown() const;
	void setFrontSelection(int nFrontSelection);
	int getFrontSelection() const;
	void setSecondarySelection(int nSecondarySelection);
	int getSecondarySelection() const;
	int getParamHandlePosition (int nHandleNum, double *pdwX, double *pdwY, double *pdwZ) const;
	int setParamHandlePosition (int nHandleNum, double dwX, double dwY, double dwZ);
	int getParamHandleValue (int nHandleNum, double *pdwValue) const;
	double setParamHandleValue (int nHandleNum, double dwValue);


	int Draw ( void );     // Draw the scene to the current GL window (m_pWindow)
	virtual int DrawPreview ( void );
	int CreateDrawing();			// Create a OpenGL Drawing to be drawn
//
// Private Definitions
//
private:
	
	// data used to draw the control
#if XVTWS == WIN32WS
	HDC m_pWindow;										// the window in which the control is drawn (WIN32)
#else
	WINDOW m_pWindow;									// the window in which the control is drawn (MOTIF)
#endif
	double m_dwRotateX, m_dwRotateY;
	double m_dwOriginX, m_dwOriginY, m_dwOriginZ;	// the center of the paramater editor
	int m_nMouseXPos, m_nMouseYPos;
	int m_nMouseButtonDown;

	unsigned int m_unDisplayList;
	int m_nFrontSelection, m_nSecondarySelection;   // itentifier of the front selection item
	int m_nNumHandles;
	CParamHandle **m_pParamHandles;
	int m_nDipDirHandleIndex, m_nDipHandleIndex, m_nPitchHandleIndex;
};

#endif


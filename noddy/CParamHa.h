// the base class for all the different types of handles that can be
// used to change values;
#ifndef _CPARAMHANDLE_H_
#define _CPARAMHANDLE_H_
#if XVTWS == WIN32WS
#undef BOOLEAN          // undefine a few things in XVT that interfear with 'windows.h'
typedef short SHORT;
typedef long LONG;
typedef char CHAR;
#include <windows.h>
#else  // Motif
#endif

#include <gl\gl.h>
#include <gl\glu.h>
#define PARAM_HANDLE_DESCRIPTION_LENGTH	50

class CParamHandle 
{
public:
	CParamHandle ();
	~CParamHandle ();

	virtual int draw () const;
	virtual int drawChangeArc () const;
	virtual double getValue () const;
	virtual double setValue (double dwValue);
	virtual double getScale () const;
	virtual double setScale (double dwScale);
	
	char *getDescription();
	int setDescription(char const * const cDescription);

	void getPosition (double *pdwX, double *pdwY, double *pdwZ) const;
	void setPosition (double dwX, double dwY, double dwZ);
	void setOrigin(double dwOriginX = 0.0, double dwOriginY = 0.0, double dwOriginZ = 0.0);
	void getOrigin(double *pdwOriginX, double *pdwOriginY, double *pdwOriginZ) const;

private:
	char m_cDescription[PARAM_HANDLE_DESCRIPTION_LENGTH];
	double m_dwValue, m_dwScaleValue;
	double m_dwOriginX, m_dwOriginY, m_dwOriginZ;	// the reference position of this control
	double m_dwX, m_dwY, m_dwZ;
};

#endif

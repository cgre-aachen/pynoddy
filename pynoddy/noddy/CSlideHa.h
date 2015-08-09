// A child class for the implement a sliding handle from side to side.
//
#ifndef _CSLIDEHANDLE_H_
#define _CSLIDEHANDLE_H_
#include "CParamHa.h"

class CSlideHandle : public CParamHandle
{
public:
	CSlideHandle ();
	~CSlideHandle ();
	double setValue (double dwValue);

private:
	// thereoretical scale of values for the sphere you can move around
	double m_dwMinScaleValue, m_dwMaxScaleValue;
	// Actual upper and lower values that can be recorded
	double m_dwMinValue, m_dwMaxValue;
		
};

#endif

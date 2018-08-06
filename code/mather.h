/*
File:   mather.h
Author: Taylor Robbins
Date:   08\05\2018
*/

#ifndef _MATHER_H
#define _MATHER_H

struct MatherData_t
{
	bool initialized;
	
	rec viewRec;
	rec sampleRec;
	
	bool grabbing;
	v2 grabViewPos;
	v2 grabSampleTopLeft;
};


#endif //  _MATHER_H

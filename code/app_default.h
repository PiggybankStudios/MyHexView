/*
File:   app_default.h
Author: Taylor Robbins
Date:   03\03\2018
*/

#ifndef _APP_DEFAULT_H
#define _APP_DEFAULT_H

struct DefaultData_t
{
	bool initialized;
	
	Texture_t testTexture;
	Texture_t circuitTexture;
	Texture_t missingTexture;
	
	u32 backgroundColorIndex;
	
	v2 quadStart;
	v2 quadControl;
	v2 quadEnd;
	
	v2 cubicStart;
	v2 cubicControl1;
	v2 cubicControl2;
	v2 cubicEnd;
	
	AlgGroup_t algGroup;
	
	rec testPackBin;
	u32 numTestPackRecs;
	rec* testPackRecs;
};

#endif //  _APP_DEFAULT_H

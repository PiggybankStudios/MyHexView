/*
File:   app_visualizer.h
Author: Taylor Robbins
Date:   03\21\2018
*/

#ifndef _APP_VISUALIZER_H
#define _APP_VISUALIZER_H

#include "app_visTypes.h"

struct VisHexData_t
{
	v2 viewPos;
	v2 viewPosGoto;
	
	u32 numColumns;
	u32 selectionStart;
	u32 selectionEnd;
	
	bool isHovering;
	u32 hoverIndex;
	
	rec viewRec;
};

struct VisIntelData_t
{
	u32 numRegions;
	//TODO: Add stuff here
};

struct VisData_t
{
	bool initialized;
	
	Font_t smallFont;
	Font_t mediumFont;
	Font_t largeFont;
	
	bool fileOpen;
	char filePath[512];
	FileInfo_t file;
	VisFileType_t fileType;
	VisViewMode_t viewMode;
	
	VisHexData_t hexData;
	VisIntelData_t intelData;
};

#endif //  _APP_VISUALIZER_H

/*
File:   app_visualizer.h
Author: Taylor Robbins
Date:   03\21\2018
*/

#ifndef _APP_VISUALIZER_H
#define _APP_VISUALIZER_H

#include "app_visTypes.h"

struct VisHexDataRegion_t
{
	MemoryArena_t* allocArena;
	u32 address;
	u32 size;
	u8* data;
};

struct VisHexData_t
{
	v2 viewPos;
	v2 viewPosGoto;
	v2 interpViewPos;
	
	u32 selectionStart;
	u32 selectionEnd;
	
	bool isHovering;
	u32 hoverIndex;
	
	u32 addressMin;
	u32 addressMax;
	u32 numColumns;
	u32 numRows;
	v2 tileSize;
	v2 spacing;
	v2 dataSize;
	
	v2 interpTileSize;
	v2 interpSpacing;
	v2 interpDataSize;
	
	u32 regionsSize;
	MemoryArena_t* allocArena;
	u32 numRegions;
	VisHexDataRegion_t* regions;
	
	// +==============================+
	// |           UI Stuff           |
	// +==============================+
	rec viewRec;
	rec interpRec;
	rec scrollGutterRec;
	rec addressesRec;
	rec scrollBarRec;
	rec selectionInfoRec;
	rec fileInfoRec;
	rec structsTabRec;
	rec structsPlusRec;
	rec structViewRec;
	rec structTypesRec;
	
	v2 selInfoViewPos;
	v2 selInfoViewPosGoto;
	v2 selInfoGrabPos;
	v2 selInfoDataSize;
	
	v2 fileInfoViewPos;
	v2 fileInfoViewPosGoto;
	v2 fileInfoGrabPos;
	v2 fileInfoDataSize;
	
	bool draggingScrollbar;
	v2 dragScrollbarOffset;
};

struct VisIntelData_t
{
	u32 numRegions;
	//TODO: Add stuff here
};

struct VisData_t
{
	bool initialized;
	
	v2 oldRenderScreenSize;
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

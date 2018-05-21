/*
File:   app_data.h
Author: Taylor Robbins
Date:   03\02\2018
*/

#ifndef _APP_DATA_H
#define _APP_DATA_H

struct TimedBlockInfo_t
{
	const char* blockName;
	const char* parentName;
	u64 counterElapsed;
	u32 numCalls;
	u32 numParents;
};

struct AppData_t
{
	MemoryArena_t mainHeap;
	MemoryArena_t tempArena;
	u32 appInitTempHighWaterMark;
	
	#if DEBUG
	bool showDebugMenu;
	u32 debugNumTimeBlocks;
	u64 debugTimeBlocks[MAX_TIME_BLOCK_DEPTH];
	const char* debugTimeBlockNames[MAX_TIME_BLOCK_DEPTH];
	
	u32 activeNumTimedBlockInfos;
	TimedBlockInfo_t activeTimedBlockInfos[MAX_TIMED_BLOCKS];
	
	u32 lastNumTimedBlockInfos;
	TimedBlockInfo_t lastTimedBlockInfos[MAX_TIMED_BLOCKS];
	#endif
	
	//AppState Data Structures
	DefaultData_t defaultData;
	VisData_t visualizerData;
	
	AppState_t appState;
	AppState_t newAppState;
	bool skipInitialization;
	bool skipDeinitialization;
	
	RenderContext_t renderContext;
	Shader_t defaultShader;
	Font_t defaultFont;
	NewFont_t newFont;
	NewFont_t debugFont;
	NewFont_t japaneseFont;
	Texture_t testTexture;
	
	PyObject* pyPlaygroundModule;
	PyObject* pyPlaygroundClass;
	
	u32 numModules;
	u32 maxModules;
	PythonPluginModule_t* modules;
	
	bool polygonFinished;
	Polygon_t testPolygon;
	u32 testNumTriangles;
	Triangle_t* testTriangles;
	VertexBuffer_t trianglesBuffer;
	
	bool glyphFilled;
	Texture_t glyphTexture;
};

#endif //  _APP_DATA_H

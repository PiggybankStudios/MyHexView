/*
File:   app_data.h
Author: Taylor Robbins
Date:   03\02\2018
*/

#ifndef _APP_DATA_H
#define _APP_DATA_H

struct AppData_t
{
	MemoryArena_t mainHeap;
	MemoryArena_t tempArena;
	u32 appInitTempHighWaterMark;
	
	//AppState Data Structures
	DefaultData_t defaultData;
	
	AppState_t appState;
	AppState_t newAppState;
	bool skipInitialization;
	bool skipDeinitialization;
	
	RenderContext_t renderContext;
	Shader_t defaultShader;
	Font_t defaultFont;
	Texture_t testTexture;
	
	PyObject* pyPlaygroundModule;
	PyObject* pyPlaygroundClass;
	
	u32 numModules;
	u32 maxModules;
	PythonPluginModule_t* modules;
};

#endif //  _APP_DATA_H

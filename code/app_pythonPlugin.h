/*
File:   app_pythonPlugin.h
Author: Taylor Robbins
Date:   03\03\2018
*/

#ifndef _APP_PYTHON_PLUGIN_H
#define _APP_PYTHON_PLUGIN_H

struct PythonPlugin_t
{
	bool loaded;
	char className[64];
	
	PyTypeObject* pyType;
	PyObject* pyInstance;
	
	union
	{
		PyObject* pyFunctions[3];
		struct
		{
			PyObject* pyFunc_PluginLoaded;
			PyObject* pyFunc_Run;
			PyObject* pyFunc_ButtonPressed;
			PyObject* pyFunc_MousePressed;
		};
	};
};

struct PythonPluginModule_t
{
	bool checked;
	bool loaded;
	MemoryArena_t* allocArena;
	
	char filePath[64];
	char moduleName[64];
	FileTime_t fileWriteTime;
	
	PyObject* pyModule;
	
	u32 numPlugins;
	PythonPlugin_t* plugins;
};

void Plugin_PluginLoaded(const PythonPlugin_t* pluginPntr);
void Plugin_ButtonPressed(const PythonPlugin_t* pluginPntr, const char* button);
void Plugin_MousePressed(const PythonPlugin_t* pluginPntr, v2 mousePos);

#endif //  _APP_PYTHON_PLUGIN_H

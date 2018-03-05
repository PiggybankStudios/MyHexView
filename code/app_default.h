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
	
	PythonPluginModule_t pluginModule;
};

#endif //  _APP_DEFAULT_H

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
	
	//TODO: Add application data here
};

#endif //  _APP_DATA_H

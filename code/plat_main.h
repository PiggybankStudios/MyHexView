/*
File:   plat_main.h
Author: Taylor Robbins
Date:   03\02\2018
*/

#ifndef _PLAT_MAIN_H
#define _PLAT_MAIN_H

// +--------------------------------------------------------------+
// |                      Public Definitions                      |
// +--------------------------------------------------------------+
#define DEFAULT_WINDOW_TITLE      PROJECT_NAME
#define APPLICATION_DLL_NAME      PROJECT_NAME ".dll"
#define APPLICATION_TEMP_DLL_NAME PROJECT_NAME "_TEMP.dll"
#define SHOW_CONSOLE_WINDOW       DEBUG
#define TOPMOST_WINDOW            DEBUG

#ifdef WINDOWS_COMPILATION

#define EXE_ICON_ID 101

#elif OSX_COMPILATION

//TODO: Add platform specific defines here

#elif LINUX_COMPILATION

//TODO: Add platform specific defines here

#endif

// +--------------------------------------------------------------+
// |                        Public Globals                        |
// +--------------------------------------------------------------+
Version_t PlatformVersion = {
	PLATFORM_VERSION_MAJOR,
	PLATFORM_VERSION_MINOR,
	PLATFORM_VERSION_BUILD
};
PlatformInfo_t PlatformInfo = {};
AppInput_t* AppInput = nullptr;
const char* WorkingDirectory = nullptr;

#ifdef WINDOWS_COMPILATION

//TODO: Add platform specific globals here

#elif OSX_COMPILATION

//TODO: Add platform specific globals here

#elif LINUX_COMPILATION

//TODO: Add platform specific globals here

#endif

// +--------------------------------------------------------------+
// |                     Function Prototypes                      |
// +--------------------------------------------------------------+
#ifdef WINDOWS_COMPILATION

void Win32_Write(const char* message);
void Win32_WriteLine(const char* message);
void Win32_Print(const char* formatString, ...);
void Win32_PrintLine(const char* formatString, ...);

#define DEBUG_Write(formatStr)          Win32_Write(formatStr)
#define DEBUG_WriteLine(formatStr)      Win32_WriteLine(formatStr)
#define DEBUG_Print(formatStr, ...)     Win32_Print(formatStr, __VA_ARGS__);
#define DEBUG_PrintLine(formatStr, ...) Win32_PrintLine(formatStr, __VA_ARGS__);

#elif OSX_COMPILATION

void OSX_Write(const char* message);
void OSX_WriteLine(const char* message);
void OSX_Print(const char* formatString, ...);
void OSX_PrintLine(const char* formatString, ...);

#define DEBUG_Write(formatStr)          OSX_Write(formatStr)
#define DEBUG_WriteLine(formatStr)      OSX_WriteLine(formatStr)
#define DEBUG_Print(formatStr, ...)     OSX_Print(formatStr, __VA_ARGS__);
#define DEBUG_PrintLine(formatStr, ...) OSX_PrintLine(formatStr, __VA_ARGS__);

#elif LINUX_COMPILATION

void LNX_Write(const char* message);
void LNX_WriteLine(const char* message);
void LNX_Print(const char* formatString, ...);
void LNX_PrintLine(const char* formatString, ...);

#define DEBUG_Write(formatStr)          LNX_Write(formatStr)
#define DEBUG_WriteLine(formatStr)      LNX_WriteLine(formatStr)
#define DEBUG_Print(formatStr, ...)     LNX_Print(formatStr, __VA_ARGS__);
#define DEBUG_PrintLine(formatStr, ...) LNX_PrintLine(formatStr, __VA_ARGS__);

#endif

#endif //  _PLAT_MAIN_H

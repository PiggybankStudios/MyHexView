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
#define DEFAULT_WINDOW_TITLE        PROJECT_NAME
#define APPLICATION_DLL_NAME        PROJECT_NAME ".dll"
#define APPLICATION_TEMP_DLL_NAME   PROJECT_NAME "_TEMP.dll"
#define ICON16_FILE_PATH            "Resources/Sprites/icon16.png"
#define ICON32_FILE_PATH            "Resources/Sprites/icon32.png"
#define SHOW_CONSOLE_WINDOW         DEBUG
#define TOPMOST_WINDOW              DEBUG
#define WINDOW_RESIZEABLE           true
#define DEFAULT_WINDOW_WIDTH        960
#define DEFAULT_WINDOW_HEIGHT       540
#define WINDOW_MIN_WIDTH            480 //GLFW_DONT_CARE
#define WINDOW_MIN_HEIGHT           270 //GLFW_DONT_CARE
#define WINDOW_MAX_WIDTH            1920 //GLFW_DONT_CARE
#define WINDOW_MAX_HEIGHT           1080 //GLFW_DONT_CARE
#define WINDOW_FORCE_ASPECT_RATIO   true
#define WINDOW_ASPECT_RATIO         16, 10
#define OPENGL_ANTIALIASING_SAMPLES 4
#define PLATFORM_TEMP_ARENA_SIZE    Kilobytes(1)
#define APP_PERMANANT_MEMORY_SIZE   Megabytes(1)
#define APP_TRANSIENT_MEMORY_SIZE   Megabytes(1)

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
PlatformInfo_t* PlatformInfo = nullptr;
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

#define HandleError(outputString) do                                             \
{                                                                                \
	glfwTerminate();                                                             \
	DEBUG_WriteLine(outputString);                                               \
	MessageBoxA(NULL, outputString, "Initialization Error Encountered!", MB_OK); \
	return 1;                                                                    \
} while(0)

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

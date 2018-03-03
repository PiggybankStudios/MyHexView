/*
File:   platformInterface.h
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** This is the only common file between the application and the platform layer
*/

#ifndef _PLATFORM_INTERFACE_H
#define _PLATFORM_INTERFACE_H

#define USE_ASSERT_FAILURE_FUNCTION true
#include "mylib.h"

#if WINDOWS_COMPILATION

#include <windows.h>
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)

#elif OSX_COMPILATION

#define EXPORT extern "C" __attribute__((visibility("default")))
#define IMPORT

#elif LINUX_COMPILATION

#define EXPORT extern "C" __attribute__((visibility("default")))
#define IMPORT

#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "my_boundedStrList.h"
#include "plat_types.h"

// +--------------------------------------------------------------+
// |          Platform Layer Function Definition Macros           |
// +--------------------------------------------------------------+
#define DebugWrite_DEFINITION(functionName)        void functionName(const char* string)
#define DebugWriteLine_DEFINITION(functionName)    void functionName(const char* message)
#define DebugPrint_DEFINITION(functionName)        void functionName(const char* formatString, ...)
#define DebugPrintLine_DEFINITION(functionName)    void functionName(const char* formatString, ...)

#define FreeFileMemory_DEFINITION(functionName)    void functionName(FileInfo_t* fileInfo)
#define ReadEntireFile_DEFINITION(functionName)    FileInfo_t functionName(const char* filename)
#define WriteEntireFile_DEFINITION(functionName)   bool functionName(const char* filename, void* memory, uint32 memorySize)
#define OpenFile_DEFINITION(functionName)          bool functionName(const char* fileName, OpenFile_t* openFileOut)
#define AppendFile_DEFINITION(functionName)        bool functionName(OpenFile_t* filePntr, const void* newData, u32 newDataSize)
#define CloseFile_DEFINITION(functionName)         void functionName(OpenFile_t* filePntr)
#define LaunchFile_DEFINITION(functionName)        bool functionName(const char* filename)

#define CopyToClipboard_DEFINITION(functionName)   void functionName(const void* dataPntr, u32 dataSize)
#define CopyFromClipboard_DEFINITION(functionName) void* functionName(MemoryArena_t* arenaPntr, u32* dataLengthOut)

#define CreateNewWindow_DEFINITION(functionName)   void functionName()
#define GetAbsolutePath_DEFINITION(functionName)   char* functionName(MemoryArena_t* arenaPntr, const char* relativePath)

typedef DebugWrite_DEFINITION(DebugWrite_f);
typedef DebugWriteLine_DEFINITION(DebugWriteLine_f);
typedef DebugPrint_DEFINITION(DebugPrint_f);
typedef DebugPrintLine_DEFINITION(DebugPrintLine_f);

typedef FreeFileMemory_DEFINITION(FreeFileMemory_f);
typedef ReadEntireFile_DEFINITION(ReadEntireFile_f);
typedef WriteEntireFile_DEFINITION(WriteEntireFile_f);
typedef OpenFile_DEFINITION(OpenFile_f);
typedef AppendFile_DEFINITION(AppendFile_f);
typedef CloseFile_DEFINITION(CloseFile_f);
typedef LaunchFile_DEFINITION(LaunchFile_f);

typedef CopyToClipboard_DEFINITION(CopyToClipboard_f);
typedef CopyFromClipboard_DEFINITION(CopyFromClipboard_f);

typedef CreateNewWindow_DEFINITION(CreateNewWindow_f);
typedef GetAbsolutePath_DEFINITION(GetAbsolutePath_f);

// +--------------------------------------------------------------+
// |               Application Resource Structures                |
// +--------------------------------------------------------------+

struct PlatformInfo_t
{
	PlatformType_t platformType;
	Version_t version;
	
	v2i screenSize;
	bool windowResized;
	bool windowHasFocus;
	bool windowIsMinimized;
	
	FreeFileMemory_f*  FreeFileMemory;
	ReadEntireFile_f*  ReadEntireFile;
	WriteEntireFile_f* WriteEntireFile;
	OpenFile_f*        OpenFile;
	AppendFile_f*      AppendFile;
	CloseFile_f*       CloseFile;
	LaunchFile_f*      LaunchFile;
	
	DebugWrite_f*     DebugWrite;
	DebugWriteLine_f* DebugWriteLine;
	DebugPrint_f*     DebugPrint;
	DebugPrintLine_f* DebugPrintLine;
	
	CopyToClipboard_f*   CopyToClipboard;
	CopyFromClipboard_f* CopyFromClipboard;
	
	CreateNewWindow_f* CreateNewWindow;
	GetAbsolutePath_f* GetAbsolutePath;
	
	r64 timeDelta;
	u64 programTime;
	RealTime_t systemTime;
	RealTime_t localTime;
	
	GLFWwindow* window;
};

struct AppMemory_t
{
	u32 permanantSize;
	u32 transientSize;
	
	void* permanantPntr;
	void* transientPntr;
};

struct ButtonState_t
{
	u32  transCount; //Transition count since last frame
	u32  pressCount; //Number of times the button was pressed (including holding key messages)
	bool isDown; //Whether or not the button ended down on this frame
};

struct AppInput_t
{
	bool mouseInsideWindow;
	v2 mousePos;
	v2 mouseStartPos[3];
	r32 mouseMaxDist[3];
	v2 scrollDelta;
	v2 scrollValue;
	
	uint32_t numButtonsDown;
	ButtonModifier_t modifiers;
	ButtonState_t buttons[Buttons_NumButtons];
	
	uint8_t textInputLength;
	char textInput[64];
	
	u8 numDroppedFiles;
	const char* droppedFiles[4];
};

struct AppOutput_t
{
	bool recenterMouse;
	bool showMouse;
	bool closeWindow;
	
	CursorType_t cursorType;
	
	char windowTitle[64];
};

// +--------------------------------------------------------------+
// |       Application Exported Function Definition Macros        |
// +--------------------------------------------------------------+
#define AppGetVersion_DEFINITION(functionName)      Version_t functionName(bool* resetApplication)
typedef AppGetVersion_DEFINITION(AppGetVersion_f);

#define AppInitialize_DEFINITION(functionName)      void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
typedef AppInitialize_DEFINITION(AppInitialize_f);

#define AppReloading_DEFINITION(functionName)       void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
typedef AppReloading_DEFINITION(AppReloading_f);

#define AppReloaded_DEFINITION(functionName)        void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
typedef AppReloaded_DEFINITION(AppReloaded_f);

#define AppUpdate_DEFINITION(functionName)          void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory, const AppInput_t* appInput, AppOutput_t* appOutput)
typedef AppUpdate_DEFINITION(AppUpdate_f);

#define AppClosing_DEFINITION(functionName)         void functionName(const PlatformInfo_t* platformInfo, const AppMemory_t* appMemory)
typedef AppClosing_DEFINITION(AppClosing_f);

#endif //  _PLATFORM_INTERFACE_H

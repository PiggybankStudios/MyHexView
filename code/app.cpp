/*
File:   app.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds the top-level functions that are exported in the application DLL that gets loaded by the platform layer
	** This is the only file that really gets compiled for the application. All other files are included inside this one
*/

#define RESET_APPLICATION false
#define USE_ASSERT_FAILURE_FUNCTION true

#include "platformInterface.h"
#include "app_version.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "my_tempMemory.h"
#include "my_tempMemory.cpp"

// +--------------------------------------------------------------+
// |                   Application Header Files                   |
// +--------------------------------------------------------------+
#include "app_defines.h"
#include "app_structs.h"
#include "app_renderContext.h"
#include "app_pythonPlugin.h"
#include "app_algebra.h"

#include "app_default.h"
#include "app_data.h"

// +--------------------------------------------------------------+
// |                     Application Globals                      |
// +--------------------------------------------------------------+
const PlatformInfo_t* platform = nullptr;
const AppInput_t* input = nullptr;
AppOutput_t* appOutput = nullptr;

AppData_t* app = nullptr;
DefaultData_t* defData = nullptr;

MemoryArena_t* mainHeap = nullptr;
RenderContext_t* renderContext = nullptr;
v2 RenderScreenSize = Vec2_Zero;
v2 RenderMousePos = Vec2_Zero;
v2 RenderMouseStartLeft = Vec2_Zero;
v2 RenderMouseStartRight = Vec2_Zero;

// +--------------------------------------------------------------+
// |                      Application Macros                      |
// +--------------------------------------------------------------+
#define DEBUG_Write(formatStr) do {      \
	if (platform != nullptr &&           \
		platform->DebugWrite != nullptr) \
	{                                    \
		platform->DebugWrite(formatStr); \
	}                                    \
} while (0)

#define DEBUG_WriteLine(formatStr) do {      \
	if (platform != nullptr &&               \
		platform->DebugWriteLine != nullptr) \
	{                                        \
		platform->DebugWriteLine(formatStr); \
	}                                        \
} while (0)

#define DEBUG_Print(formatStr, ...) do {              \
	if (platform != nullptr &&                        \
		platform->DebugPrint != nullptr)              \
	{                                                 \
		platform->DebugPrint(formatStr, __VA_ARGS__); \
	}                                                 \
} while (0)

#define DEBUG_PrintLine(formatStr, ...) do {              \
	if (platform != nullptr &&                            \
		platform->DebugPrintLine != nullptr)              \
	{                                                     \
		platform->DebugPrintLine(formatStr, __VA_ARGS__); \
	}                                                     \
} while (0)

#define HandleButton(button) do { app->buttonHandled[button] = true; } while(0)
#define IsButtonHandled(button) app->buttonHandled[button]

#define ButtonPressed(button) ((input->buttons[button].isDown && input->buttons[button].transCount > 0) || input->buttons[button].transCount >= 2)
#define ButtonPressedUnhandled(button) (app->buttonHandled[button] == false && ((input->buttons[button].isDown && input->buttons[button].transCount > 0) || input->buttons[button].transCount >= 2))
#define ButtonReleased(button) ((!input->buttons[button].isDown && input->buttons[button].transCount > 0) || input->buttons[button].transCount >= 2)
#define ButtonReleasedUnhandled(button) (app->buttonHandled[button] == false && ((!input->buttons[button].isDown && input->buttons[button].transCount > 0) || input->buttons[button].transCount >= 2))
#define ButtonDown(button) (input->buttons[button].isDown)
#define ButtonDownUnhandled(button) (app->buttonHandled[button] == false && input->buttons[button].isDown)

#define ClickedOnRec(rectangle) (ButtonReleasedUnhandled(MouseButton_Left) && IsInsideRec(rectangle, RenderMousePos) && IsInsideRec(rectangle, RenderMouseStartPos))

// +--------------------------------------------------------------+
// |                   Application Source Files                   |
// +--------------------------------------------------------------+
#include "app_loadingFunctions.cpp"
#include "app_fontHelpers.cpp"
#include "app_renderContext.cpp"
#include "app_pythonFunctions.cpp"
#include "app_pythonPlugin.cpp"
#include "app_algebra.cpp"

#include "app_helpers.cpp"
#include "app_default.cpp"

// +--------------------------------------------------------------+
// |                    Local Helper Functions                    |
// +--------------------------------------------------------------+
void AppLoadContent(bool firstLoad)
{
	if (!firstLoad)
	{
		DestroyShader(&app->defaultShader);
		DestroyFont(&app->defaultFont);
	}
	
	app->defaultShader = LoadShader(SHADERS_FOLDER "simple-vertex.glsl", SHADERS_FOLDER "simple-fragment.glsl");
	app->defaultFont   = LoadFont(FONTS_FOLDER "consolab.ttf", 24, 256, 256, ' ', 96);
}

// +--------------------------------------------------------------+
// |                       App Get Version                        |
// +--------------------------------------------------------------+
// Version_t App_GetVersion(bool* resetApplication)
EXPORT AppGetVersion_DEFINITION(App_GetVersion)
{
	Version_t version = { APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD };
	if (resetApplication != nullptr) { *resetApplication = RESET_APPLICATION; }
	return version;
}

// +--------------------------------------------------------------+
// |                        App Initialize                        |
// +--------------------------------------------------------------+
// void App_Initialize(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory)
EXPORT AppInitialize_DEFINITION(App_Initialize)
{
	platform = PlatformInfo;
	input = nullptr;
	appOutput = nullptr;
	app = (AppData_t*)AppMemory->permanantPntr;
	mainHeap = &app->mainHeap;
	defData = &app->defaultData;
	TempArena = &app->tempArena;
	renderContext = &app->renderContext;
	RenderScreenSize = NewVec2(PlatformInfo->screenSize);
	RenderMousePos = Vec2_Zero;
	RenderMouseStartLeft = Vec2_Zero;
	RenderMouseStartRight = Vec2_Zero;
	
	DEBUG_WriteLine("Initializing application...");
	
	// +==============================+
	// |     Setup Memory Arenas      |
	// +==============================+
	{
		Assert(sizeof(AppData_t) < AppMemory->permanantSize);
		ClearPointer(app);
		
		u32 mainHeapSize = AppMemory->permanantSize - sizeof(AppData_t);
		InitializeMemoryArenaHeap(&app->mainHeap, (void*)(app + 1), mainHeapSize);
		
		InitializeMemoryArenaTemp(&app->tempArena, AppMemory->transientPntr, AppMemory->transientSize, TRANSIENT_MAX_NUMBER_MARKS);
		
		DEBUG_PrintLine("Main Heap:  %u bytes", mainHeapSize);
		DEBUG_PrintLine("Temp Arena: %u bytes", AppMemory->transientSize);
	}
	
	TempPushMark();
	
	// +==============================+
	// |      Initialize Things       |
	// +==============================+
	InitializeRenderContext();
	AppLoadContent(true);
	AddPythonFunctions();
	LoadPlaygroundPythonModule();
	
	// +==============================+
	// | Initialize Starting AppState |
	// +==============================+
	app->appState = AppState_Default;
	app->newAppState = app->appState;
	DEBUG_PrintLine("[Initializing AppState_%s]", GetAppStateStr(app->appState));
	switch (app->appState)
	{
		case AppState_Default: InitializeDefaultState(); StartDefaultState(app->appState); break;
	}
	
	DEBUG_WriteLine("Application initialization finished!");
	TempPopMark();
	app->appInitTempHighWaterMark = ArenaGetHighWaterMark(TempArena);
	ArenaResetHighWaterMark(TempArena);
}

// +--------------------------------------------------------------+
// |                          App Update                          |
// +--------------------------------------------------------------+
// void App_Update(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory, const AppInput_t* AppInput, AppOutput_t* AppOutput)
EXPORT AppUpdate_DEFINITION(App_Update)
{
	platform = PlatformInfo;
	input = AppInput;
	appOutput = AppOutput;
	app = (AppData_t*)AppMemory->permanantPntr;
	mainHeap = &app->mainHeap;
	defData = &app->defaultData;
	TempArena = &app->tempArena;
	renderContext = &app->renderContext;
	RenderScreenSize = NewVec2(PlatformInfo->screenSize);
	RenderMousePos = AppInput->mousePos;
	RenderMouseStartLeft = AppInput->mouseStartPos[MouseButton_Left];
	RenderMouseStartRight = AppInput->mouseStartPos[MouseButton_Right];
	
	TempPushMark();
	
	// +==============================+
	// |   Relaod Content Shortcut    |
	// +==============================+
	if (ButtonPressed(Button_F5))
	{
		DEBUG_WriteLine("Reloading app content");
		AppLoadContent(false);
	}
	// +==============================+
	// |  Restart Python Interpreter  |
	// +==============================+
	if (ButtonPressed(Button_F6))
	{
		DEBUG_WriteLine("Restarting python interpreter");
		UnloadAllPythonModules();
		Py_FinalizeEx();
		Py_Initialize();
		AddPythonFunctions();
		LoadPlaygroundPythonModule();
	}
	
	// +==============================+
	// |     Load Python Modules      |
	// +==============================+
	UpdatePythonModuleLoader();
	
	// +==============================+
	// |   Update Current AppState    |
	// +==============================+
	switch (app->appState)
	{
		case AppState_Default: UpdateAndRenderDefaultState(); break;
	}
	
	// +==============================+
	// |       Change AppStates       |
	// +==============================+
	if (app->newAppState != app->appState)
	{
		//Deinitialize
		if (!app->skipDeinitialization)
		{
			DEBUG_PrintLine("[Deinitializing AppState_%s]", GetAppStateStr(app->appState));
			switch (app->appState)
			{
				case AppState_Default: DeinitializeDefaultState(); break;
			}
		}
		
		AppState_t oldState = app->appState;
		app->appState = app->newAppState;
		
		//Initialize
		if (!app->skipInitialization)
		{
			DEBUG_PrintLine("[Initializing AppState_%s]", GetAppStateStr(app->appState));
			switch (app->appState)
			{
				case AppState_Default: InitializeDefaultState(); break;
			}
		}
		
		app->skipInitialization = false;
		app->skipDeinitialization = false;
		
		//Start
		DEBUG_PrintLine("[Starting AppState_%s]", GetAppStateStr(app->appState));
		switch (app->appState)
		{
			case AppState_Default: StartDefaultState(oldState); break;
		}
	}
	
	TempPopMark();
}

// +--------------------------------------------------------------+
// |                        App Reloading                         |
// +--------------------------------------------------------------+
// void App_Reloading(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory)
EXPORT AppReloading_DEFINITION(App_Reloading)
{
	platform = PlatformInfo;
	input = nullptr;
	appOutput = nullptr;
	app = (AppData_t*)AppMemory->permanantPntr;
	mainHeap = &app->mainHeap;
	defData = &app->defaultData;
	TempArena = &app->tempArena;
	renderContext = &app->renderContext;
	RenderScreenSize = NewVec2(PlatformInfo->screenSize);
	RenderMousePos = Vec2_Zero;
	RenderMouseStartLeft = Vec2_Zero;
	RenderMouseStartRight = Vec2_Zero;
	
	//TODO: Deallocate stuff that might change
}

// +--------------------------------------------------------------+
// |                         App Reloaded                         |
// +--------------------------------------------------------------+
// void App_Reloaded(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory)
EXPORT AppReloaded_DEFINITION(App_Reloaded)
{
	platform = PlatformInfo;
	input = nullptr;
	appOutput = nullptr;
	app = (AppData_t*)AppMemory->permanantPntr;
	mainHeap = &app->mainHeap;
	defData = &app->defaultData;
	TempArena = &app->tempArena;
	renderContext = &app->renderContext;
	RenderScreenSize = NewVec2(PlatformInfo->screenSize);
	RenderMousePos = Vec2_Zero;
	RenderMouseStartLeft = Vec2_Zero;
	RenderMouseStartRight = Vec2_Zero;
	
	// TODO: Reallocate stuff that was deallocated in App_Reloading
}

// +--------------------------------------------------------------+
// |                         App Closing                          |
// +--------------------------------------------------------------+
// void App_Closing(const PlatformInfo_t* PlatformInfo, const AppMemory_t* AppMemory)
EXPORT AppClosing_DEFINITION(App_Closing)
{
	platform = PlatformInfo;
	input = nullptr;
	appOutput = nullptr;
	app = (AppData_t*)AppMemory->permanantPntr;
	mainHeap = &app->mainHeap;
	defData = &app->defaultData;
	TempArena = &app->tempArena;
	renderContext = &app->renderContext;
	RenderScreenSize = NewVec2(PlatformInfo->screenSize);
	RenderMousePos = Vec2_Zero;
	RenderMouseStartLeft = Vec2_Zero;
	RenderMouseStartRight = Vec2_Zero;
	
	//TODO: Deallocate anything?
}

#if DEBUG
//This function is declared in my_assert.h and needs to be implemented by us for a debug build to compile successfully
void AssertFailure(const char* function, const char* filename, int lineNumber, const char* expressionStr)
{
	DEBUG_PrintLine("Assertion Failure! %s in \"%s\" line %d: (%s) is not true", function, GetFileNamePart(filename), lineNumber, expressionStr);
}
#endif

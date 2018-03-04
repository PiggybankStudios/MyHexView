/*
File:   win32_main.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds the main entry point for the platform executable 
*/

//Standard Libraries
#include <windows.h>
#include "Shlwapi.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdbool.h>

//External Libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Project Headers
#include "win32_version.h"
#include "platformInterface.h"

//NOTE: There are two TempArena globals. One here in the platform layer and one in the application DLL.
#include "my_tempMemory.h"
#include "my_tempMemory.cpp"

// +--------------------------------------------------------------+
// |                     Common Source Files                      |
// +--------------------------------------------------------------+
#include "plat_main.h"

#include "plat_keymap.cpp"
#include "plat_callbacks.cpp"
#include "plat_clipboard.cpp"

// +--------------------------------------------------------------+
// |                     Windows Source Files                     |
// +--------------------------------------------------------------+
#include "win32_debug.cpp"
#include "win32_helpers.cpp"
#include "win32_files.cpp"
#include "win32_misc.cpp"
#include "win32_appLoading.cpp"

// +--------------------------------------------------------------+
// |                   Windows Main Entry Point                   |
// +--------------------------------------------------------------+
#if SHOW_CONSOLE_WINDOW
int main()
#else
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
#endif
{
	DEBUG_PrintLine(PROJECT_NAME " Windows Platform Layer v%u.%u(%u)", PlatformVersion.major, PlatformVersion.minor, PlatformVersion.build);
	
	char argv0[256];
	DWORD moduleFilenameLength = GetModuleFileNameA(0, argv0, ArrayCount(argv0));
	DEBUG_PrintLine("Argv[0] = \"%s\"", argv0);
	
	// +================================+
	// | Initialize Platform Temp Arena |
	// +================================+
	MemoryArena_t tempArena = {};
	void* tempMemory;
	DEBUG_PrintLine("Allocating platform temporary memory");
	tempMemory = malloc(PLATFORM_TEMP_ARENA_SIZE);
	InitializeMemoryArenaTemp(&tempArena, tempMemory, PLATFORM_TEMP_ARENA_SIZE, 8);
	TempArena = &tempArena;
	
	// +==============================+
	// |       Initialize GLFW        |
	// +==============================+
	{
		glfwSetErrorCallback(GlfwErrorCallback);
		if (glfwInit() == false)
		{
			HandleError("GLFW initialization failed");
		}
		i32 openglMajor, openglMinor, openglRevision;
		glfwGetVersion(&openglMajor, &openglMinor, &openglRevision);
		Win32_PrintLine("OpenGL %d.%d(%d) supported.", openglMajor, openglMinor, openglRevision);
	}
	
	// +==============================+
	// |     GLFW Window Creation     |
	// +==============================+
	GLFWwindow* window = nullptr;
	v2i screenSize = Vec2i_Zero;
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
		glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_ANY_PROFILE);//GLFW_OPENGL_CORE_PROFILE
		glfwWindowHint(GLFW_RESIZABLE,             WINDOW_RESIZEABLE);
		glfwWindowHint(GLFW_FLOATING,              TOPMOST_WINDOW);
		glfwWindowHint(GLFW_DECORATED,             GL_TRUE);
		glfwWindowHint(GLFW_FOCUSED,               GL_TRUE);
		glfwWindowHint(GLFW_DOUBLEBUFFER,          GL_TRUE);
		glfwWindowHint(GLFW_RED_BITS,              8);
		glfwWindowHint(GLFW_GREEN_BITS,            8);
		glfwWindowHint(GLFW_BLUE_BITS,             8);
		glfwWindowHint(GLFW_ALPHA_BITS,            8);
		glfwWindowHint(GLFW_DEPTH_BITS,            8);
		glfwWindowHint(GLFW_STENCIL_BITS,          8);
		glfwWindowHint(GLFW_SAMPLES,               OPENGL_ANTIALIASING_SAMPLES);
		
		DEBUG_Write("Creating GLFW window...");
		window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_TITLE, NULL, NULL);
		if (window == nullptr)
		{
			HandleError("GLFW window creation failed!");
		}
		
		glfwSetWindowSizeLimits(window, WINDOW_MIN_WIDTH, WINDOW_MIN_HEIGHT, WINDOW_MAX_WIDTH, WINDOW_MAX_HEIGHT);
		#if WINDOW_FORCE_ASPECT_RATIO
		glfwSetWindowAspectRatio(window, WINDOW_ASPECT_RATIO);
		#endif
		
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);
		glfwGetFramebufferSize(window, &screenSize.width, &screenSize.height);
		glViewport(0, 0, screenSize.width, screenSize.height);
		glClearColor(20/255.f, 20/255.f, 20/255.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();
		glfwSwapBuffers(window);
		DEBUG_WriteLine("Done!");
	}
	
	// +==============================+
	// |       Load Window Icon       |
	// +==============================+
	{
		FileInfo_t iconFile16 = Win32_ReadEntireFile(ICON16_FILE_PATH);
		FileInfo_t iconFile32 = Win32_ReadEntireFile(ICON32_FILE_PATH);
		
		if (iconFile16.content != nullptr && iconFile32.content != nullptr)
		{
			i32 numChannels16, numChannels32;
			i32 width16, height16, width32, height32;
			u8* imageData16 = stbi_load_from_memory((u8*)iconFile16.content, iconFile16.size, &width16, &height16, &numChannels16, 4);
			u8* imageData32 = stbi_load_from_memory((u8*)iconFile32.content, iconFile32.size, &width32, &height32, &numChannels32, 4);
			
			GLFWimage images[2];
			images[0].width  = width16;
			images[0].height = height16;
			images[0].pixels = imageData16;
			images[1].width  = width32;
			images[1].height = height32;
			images[1].pixels = imageData32;
			
			glfwSetWindowIcon(window, 2, images);
			
			stbi_image_free(imageData16);
			stbi_image_free(imageData32);
			Win32_FreeFileMemory(&iconFile16);
			Win32_FreeFileMemory(&iconFile32);
		}
		else
		{
			DEBUG_PrintLine("Couldn't open icon files at \"%s\" and \"%s\"", ICON16_FILE_PATH, ICON32_FILE_PATH);
		}
	}
	
	// +==============================+
	// |    Create Cursor Options     |
	// +==============================+
	CursorType_t currentCursor = Cursor_Default;
	GLFWcursor* glfwCursors[NumCursorTypes] = {};
	{
		glfwCursors[Cursor_Default]          = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		glfwCursors[Cursor_Text]             = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		glfwCursors[Cursor_Pointer]          = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		glfwCursors[Cursor_ResizeHorizontal] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		glfwCursors[Cursor_ResizeVertical]   = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		glfwSetCursor(window, glfwCursors[currentCursor]);
	}
	
	// +==============================+
	// |       Initialize GLEW        |
	// +==============================+
	const GLFWvidmode* glfwModePntr = nullptr;
	{
		glewExperimental = GL_TRUE;
		GLenum glewInitError = glewInit();
		if (glewInitError != GLEW_OK)
		{
			HandleError("GLEW initialization failed!");
		}
		
		const GLubyte* openglVersionStr = glGetString(GL_VERSION);
		const GLubyte* rendererStr = glGetString(GL_RENDERER);
		glfwModePntr = glfwGetVideoMode(glfwGetPrimaryMonitor());
		DEBUG_PrintLine("OpenGL Version %s", openglVersionStr);
		DEBUG_PrintLine("Rendering with \"%s\"", rendererStr);
		DEBUG_PrintLine("Monitor Refresh Rate: %dHz", glfwModePntr->refreshRate);
	}
	
	// +==============================+
	// |      Initialize Python       |
	// +==============================+
	{
		DEBUG_WriteLine("Initializing Python");
		wchar_t* program = Py_DecodeLocale(argv0, NULL);
		if (program == nullptr)
		{
			HandleError("Cannot decode locale of argv!");
		}
		Py_SetProgramName(program);
		Py_Initialize();
		PyMem_RawFree(program);
	}
	
	// +==============================+
	// |      Register Callbacks      |
	// +==============================+
	{
		glfwSetWindowCloseCallback(window,     GlfwWindowCloseCallback);
		glfwSetFramebufferSizeCallback(window, GlfwWindowSizeCallback);
		glfwSetWindowPosCallback(window,       GlfwWindowMoveCallback);
		glfwSetWindowIconifyCallback(window,   GlfwWindowMinimizeCallback);
		glfwSetWindowFocusCallback(window,     GlfwWindowFocusCallback);
		glfwSetKeyCallback(window,             GlfwKeyPressedCallback);
		glfwSetCharCallback(window,            GlfwCharPressedCallback);
		glfwSetCursorPosCallback(window,       GlfwCursorPosCallback);
		glfwSetMouseButtonCallback(window,     GlfwMousePressCallback);
		glfwSetScrollCallback(window,          GlfwMouseScrollCallback);
		glfwSetCursorEnterCallback(window,     GlfwCursorEnteredCallback);
		glfwSetDropCallback(window,            GlfwDropCallback);
	}
	
	// +==============================+
	// |      Fill Platform Info      |
	// +==============================+
	PlatformInfo_t platformInfo = {};
	PlatformInfo_ = &platformInfo;
	{
		platformInfo.platformType   = Platform_Windows;
		platformInfo.version        = PlatformVersion;
		platformInfo.screenSize     = screenSize;
		platformInfo.windowHasFocus = true;
		platformInfo.window         = window;
		
		platformInfo.DebugWrite        = Win32_Write;
		platformInfo.DebugWriteLine    = Win32_WriteLine;
		platformInfo.DebugPrint        = Win32_Print;
		platformInfo.DebugPrintLine    = Win32_PrintLine;
		platformInfo.FreeFileMemory    = Win32_FreeFileMemory;
		platformInfo.ReadEntireFile    = Win32_ReadEntireFile;
		platformInfo.WriteEntireFile   = Win32_WriteEntireFile;
		platformInfo.OpenFile          = Win32_OpenFile;
		platformInfo.AppendFile        = Win32_AppendFile;
		platformInfo.CloseFile         = Win32_CloseFile;
		platformInfo.LaunchFile        = Win32_LaunchFile;
		platformInfo.CopyToClipboard   = PLT_CopyToClipboard;
		platformInfo.CopyFromClipboard = PLT_CopyFromClipboard;
		platformInfo.CreateNewWindow   = Win32_CreateNewWindow;
		platformInfo.GetAbsolutePath   = Win32_GetAbsolutePath;
	}
	
	// +==============================+
	// | Allocate Application Memory  |
	// +==============================+
	AppMemory_t appMemory = {};
	{
		appMemory.permanantSize = APP_PERMANANT_MEMORY_SIZE;
		appMemory.transientSize = APP_TRANSIENT_MEMORY_SIZE;
		appMemory.permanantPntr = (void*)VirtualAlloc(0, appMemory.permanantSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		appMemory.transientPntr = (void*)VirtualAlloc(0, appMemory.transientSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		
		if (appMemory.permanantPntr == nullptr || appMemory.transientPntr == nullptr)
		{
			HandleError("Could not allocate enough memory for application!");
		}
	}
	
	// +==============================+
	// |     Initialize AppInput      |
	// +==============================+
	AppInput_t appInputArray[2] = {};
	AppInput_ = &appInputArray[0];
	AppInput_t* lastInput = &appInputArray[1];
	{
		r64 mousePosX, mousePosY;
		glfwGetCursorPos(window, &mousePosX, &mousePosY);
		AppInput_->mousePos = NewVec2((r32)mousePosX, (r32)mousePosY);
		AppInput_->mouseInsideWindow = (mousePosX >= 0 && mousePosY >= 0 && mousePosX < screenSize.width && mousePosY < screenSize.height);
	}
	
	// +==============================+
	// |     Initialize AppOutput     |
	// +==============================+
	AppOutput_t appOutput = {};
	{
		appOutput.showMouse = true;
		appOutput.cursorType = currentCursor;
		strncpy(appOutput.windowTitle, DEFAULT_WINDOW_TITLE, ArrayCount(appOutput.windowTitle)-1);
		appOutput.windowTitle[ArrayCount(appOutput.windowTitle)-1] = '\0';
	}
	
	// +==============================+
	// |     Load Application DLL     |
	// +==============================+
	LoadedApp_t application = {};
	char* appDllPath = nullptr;
	char* appTempDllPath = nullptr;
	{
		char exeDirectory[256];
		u32 exeDirectoryLength = GetRunningDirectory(exeDirectory, ArrayCount(exeDirectory));
		DEBUG_PrintLine("Exe Directory: \"%s\"", exeDirectory);
		
		appDllPath = StrCat(TempArena, NtStr(exeDirectory), NtStr(APPLICATION_DLL_NAME));
		appTempDllPath = StrCat(TempArena, NtStr(exeDirectory), NtStr(APPLICATION_TEMP_DLL_NAME));
		
		if (PathFileExists(appDllPath) == false)
		{
			char* errorStr = TempPrint("Could not find application DLL at \"%s\"", appDllPath);
			HandleError(errorStr);
		}
		
		if (LoadDllCode(appDllPath, appTempDllPath, &application))
		{
			DEBUG_PrintLine("Found application v%u.%u(%u)", application.version.major, application.version.minor, application.version.build);
		}
		else
		{
			HandleError("Application DLL is invalid");
		}
	}
	
	// +==============================+
	// |      Call AppInitialize      |
	// +==============================+
	{
		DEBUG_WriteLine("+==============================+");
		DEBUG_WriteLine("|   Initializing Application   |");
		DEBUG_WriteLine("+==============================+");
		application.Initialize(&platformInfo, &appMemory);
	}
	
	// +--------------------------------------------------------------+
	// |                          Main Loop                           |
	// +--------------------------------------------------------------+
	DEBUG_WriteLine("+==============================+");
	DEBUG_WriteLine("|      Starting Main Loop      |");
	DEBUG_WriteLine("+==============================+");
	while (glfwWindowShouldClose(window) == false)
	{
		#if DEBUG
		// +==============================+
		// |    Reload Application DLL    |
		// +==============================+
		{
			//TOOD: Delay this so we don't get double loads
			//Check to see if we should reload the application DLL
			FILETIME newDllFiletime = GetFileWriteTime(appDllPath);
			if (CompareFileTime(&newDllFiletime, &application.lastWriteTime) != 0)
			{
				application.Reloading(&platformInfo, &appMemory);
				FreeDllCode(&application);
				
				if (LoadDllCode(appDllPath, appTempDllPath, &application))
				{
					DEBUG_PrintLine("Loaded application v%u.%u(%u)", application.version.major, application.version.minor, application.version.build);
					
					bool resetApplication = false;
					application.GetVersion(&resetApplication);
					if (resetApplication)
					{
						DEBUG_WriteLine("Resetting application");
						
						memset(appMemory.permanantPntr, 0x00, appMemory.permanantSize);
						memset(appMemory.transientPntr, 0x00, appMemory.transientSize);
						
						//TODO: Find a way to reset the opengl context or
						//		maybe re-open the window altogether
						
						application.Initialize(&platformInfo, &appMemory);
					}
					else
					{
						application.Reloaded(&platformInfo, &appMemory);
					}
				}
				else
				{
					DEBUG_WriteLine("Failed to reload application DLL!");
					// glfwSetWindowShouldClose(window, true);
				}
				
				// UpdateWindowTitle(window, &PlatformVersion, &application.version);
			}
		}
		#endif
		
		// +==============================+
		// |        Swap AppInputs        |
		// +==============================+
		{
			AppInput_t* tempPntr = lastInput;
			lastInput = AppInput_;
			AppInput_ = tempPntr;
			
			memcpy(AppInput_, lastInput, sizeof(AppInput_t));
			AppInput_->textInputLength = 0;
			AppInput_->scrollDelta = Vec2_Zero;
			//Clear the transition counts
			for (uint32_t bIndex = 0; bIndex < ArrayCount(AppInput_->buttons); bIndex++)
			{
				AppInput_->buttons[bIndex].transCount = 0;
				AppInput_->buttons[bIndex].pressCount = 0;
			}
			//Free the dropped file paths that were malloc'd in the DropCallback
			for (uint32_t fIndex = 0; fIndex < AppInput_->numDroppedFiles; fIndex++)
			{
				if (AppInput_->droppedFiles[fIndex] != nullptr)
				{
					free((char*)AppInput_->droppedFiles[fIndex]);
				}
			}
			AppInput_->numDroppedFiles = 0;
		}
		
		// +==============================+
		// |         Poll Events          |
		// +==============================+
		platformInfo.windowResized = false;
		glfwPollEvents();
		
		// +==============================+
		// |  Update Window Information   |
		// +==============================+
		glfwGetFramebufferSize(window, &screenSize.width, &screenSize.height);
		glViewport(0, 0, screenSize.width, screenSize.height);
		platformInfo.screenSize = screenSize;
		
		// +==============================+
		// |   Update Time Information    |
		// +==============================+
		{
			u64 lastTime = platformInfo.programTime;
			platformInfo.programTime = (u64)(glfwGetTime() * 1000);
			platformInfo.timeDelta = (r64)(platformInfo.programTime - lastTime) / (1000.0 / glfwModePntr->refreshRate);
			
			SYSTEMTIME systemTime = {};
			SYSTEMTIME localTime = {};
			GetSystemTime(&systemTime);
			GetLocalTime(&localTime);
			
			platformInfo.systemTime = {};
			platformInfo.systemTime.year        = systemTime.wYear;
			platformInfo.systemTime.month       = (systemTime.wMonth-1);
			platformInfo.systemTime.day         = (systemTime.wDay-1);
			platformInfo.systemTime.hour        = systemTime.wHour;
			platformInfo.systemTime.minute      = systemTime.wMinute;
			platformInfo.systemTime.second      = systemTime.wSecond;
			platformInfo.systemTime.millisecond = systemTime.wMilliseconds;
			platformInfo.localTime = {};
			platformInfo.localTime.year         = localTime.wYear;
			platformInfo.localTime.month        = (localTime.wMonth-1);
			platformInfo.localTime.day          = (localTime.wDay-1);
			platformInfo.localTime.hour         = localTime.wHour;
			platformInfo.localTime.minute       = localTime.wMinute;
			platformInfo.localTime.second       = localTime.wSecond;
			platformInfo.localTime.millisecond  = localTime.wMilliseconds;
		}
		
		// +==============================+
		// |      Application Update      |
		// +==============================+
		application.Update(&platformInfo, &appMemory, AppInput_, &appOutput);
		glfwSwapBuffers(window);
		
		// +==============================+
		// |       Handle AppOutput       |
		// +==============================+
		UpdateWindowTitle(window, appOutput.windowTitle, &PlatformVersion, &application.version);
		if (appOutput.cursorType != currentCursor)
		{
			glfwSetCursor(window, glfwCursors[appOutput.cursorType]);
			currentCursor = appOutput.cursorType;
		}
		if (appOutput.closeWindow)
		{
			DEBUG_WriteLine("+==============================+");
			DEBUG_WriteLine("| Application Requested Close  |");
			DEBUG_WriteLine("+==============================+");
			glfwSetWindowShouldClose(window, true);
		}
	}
	
	application.Closing(&platformInfo, &appMemory);
	glfwTerminate();
	Py_FinalizeEx();
	
	DEBUG_WriteLine("Goodbye World!");
	return 0;
}

#if DEBUG
void AssertFailure(const char* function, const char* filename, int lineNumber, const char* expressionStr)
{
	DEBUG_PrintLine("Assertion Failure! %s in \"%s\" line %d: (%s) is not true", function, GetFileNamePart(filename), lineNumber, expressionStr);
}
#endif

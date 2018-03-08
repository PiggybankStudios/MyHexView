/*
File:   win32_appLoading.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds some helper functions that handle loading the application DLL into a usable structure 
*/

// +--------------------------------------------------------------+
// |                     Structure Definition                     |
// +--------------------------------------------------------------+
struct LoadedApp_t
{
	bool isValid;
	Version_t version;
	HMODULE module;
	FileTime_t lastWriteTime;
	
	AppGetVersion_f* GetVersion;
	AppReloading_f*  Reloading;
	AppReloaded_f*   Reloaded;
	AppInitialize_f* Initialize;
	AppUpdate_f*     Update;
	AppClosing_f*    Closing;
};

// +--------------------------------------------------------------+
// |                        Stub Functions                        |
// +--------------------------------------------------------------+
AppGetVersion_DEFINITION(AppGetVersion_Stub)
{
	Version_t version = { 0, 0, 0 };
	return version;
}
AppInitialize_DEFINITION(AppInitialize_Stub)
{
	
}
AppReloading_DEFINITION(AppReloading_Stub)
{
	
}
AppReloaded_DEFINITION(AppReloaded_Stub)
{
	
}
AppUpdate_DEFINITION(AppUpdate_Stub)
{
	
}
AppClosing_DEFINITION(AppClosing_Stub)
{
	
}


// +--------------------------------------------------------------+
// |                        Main Functions                        |
// +--------------------------------------------------------------+
bool LoadDllCode(const char* appDllName, const char* tempDllName, LoadedApp_t* loadedApp)
{
	ClearPointer(loadedApp);
	
	Win32_GetFileTime(appDllName, &loadedApp->lastWriteTime);
	
	#if DEBUG
		u32 copyTries = 0;
		while (!CopyFileA(appDllName, tempDllName, false))
		{
			DWORD error = GetLastError();
			if (error != ERROR_SHARING_VIOLATION)
			{
				Win32_PrintLine("CopyFileA error: %u", GetLastError());
				copyTries++;
				
				if (copyTries >= 100) 
				{
					Win32_WriteLine("Could not copy DLL.");
					return false;
				}
			}
		}
		// Win32_PrintLine("Tried to copy %u times", copyTries);
		loadedApp->module = LoadLibraryA(tempDllName);
	#else
		loadedApp->module = LoadLibraryA(appDllName);
	#endif
	
	
	if (loadedApp->module != 0)
	{
		loadedApp->GetVersion = (AppGetVersion_f*) GetProcAddress(loadedApp->module, "App_GetVersion");
		loadedApp->Initialize = (AppInitialize_f*) GetProcAddress(loadedApp->module, "App_Initialize");
		loadedApp->Reloading  = (AppReloading_f*)  GetProcAddress(loadedApp->module, "App_Reloading");
		loadedApp->Reloaded   = (AppReloaded_f*)   GetProcAddress(loadedApp->module, "App_Reloaded");
		loadedApp->Update     = (AppUpdate_f*)     GetProcAddress(loadedApp->module, "App_Update");
		loadedApp->Closing    = (AppClosing_f*)    GetProcAddress(loadedApp->module, "App_Closing");
		
		loadedApp->isValid = (
			loadedApp->GetVersion != nullptr &&
			loadedApp->Initialize != nullptr &&
			loadedApp->Reloading  != nullptr &&
			loadedApp->Reloaded   != nullptr &&
			loadedApp->Update     != nullptr &&
			loadedApp->Closing    != nullptr
		);
	}
	
	if (!loadedApp->isValid)
	{
		loadedApp->GetVersion = AppGetVersion_Stub;
		loadedApp->Initialize = AppInitialize_Stub;
		loadedApp->Reloading  = AppReloading_Stub;
		loadedApp->Reloaded   = AppReloaded_Stub;
		loadedApp->Update     = AppUpdate_Stub;
		loadedApp->Closing    = AppClosing_Stub;
	}
	
	loadedApp->version = loadedApp->GetVersion(nullptr);
	
	return loadedApp->isValid;
}

void FreeDllCode(LoadedApp_t* loadedApp)
{
	if (loadedApp != nullptr &&
		loadedApp->module != 0)
	{
		bool32 freeResult = FreeLibrary(loadedApp->module);
		Assert(freeResult);
	}
	
	ClearPointer(loadedApp);
}

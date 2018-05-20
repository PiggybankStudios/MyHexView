/*
File:   win32_paths.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds random extra functions that the platform layer provides
*/


// +==============================+
// |    Win32_CreateNewWindow     |
// +==============================+
// void CreateNewWindow()
CreateNewWindow_DEFINITION(Win32_CreateNewWindow)
{
	char moduleName[256] = {};
	DWORD moduleNameLength = GetModuleFileNameA(0, moduleName, ArrayCount(moduleName));
	
	PROCESS_INFORMATION procInfo = {};
	
	STARTUPINFO startupInfo = {};
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_SHOWMINNOACTIVE;
	
	bool32 createProcessResult = CreateProcess(NULL,
		(LPSTR)moduleName, // command line
		NULL,              // process security attributes
		NULL,              // primary thread security attributes
		true,              // handles are inherited
		0,                 // creation flags
		NULL,              // use parent's environment
		NULL,              // use parent's current directory
		&startupInfo,      // STARTUPINFO pointer
		&procInfo);        // receives PROCESS_INFORMATION
	
	if (createProcessResult)
	{
		Win32_WriteLine("Created new window!");
	}
	else
	{
		Win32_WriteLine("Failed to create new window");
	}
}

// +==============================+
// |    Win32_GetAbsolutePath     |
// +==============================+
// char* GetAbsolutePath(MemoryArena_t* arenaPntr, const char* relativePath)
GetAbsolutePath_DEFINITION(Win32_GetAbsolutePath)
{
	char workingDir[256] = {};
	u32 workingDirLength = GetRunningDirectory(workingDir, ArrayCount(workingDir));
	
	char* result = ArenaPrint(arenaPntr, "%s%s", workingDir, relativePath);
	StrReplaceInPlace(NtStr(result), "/", "\\", 1);
	
	return result;
}

// +==============================+
// |       Win32_FrameFlip        |
// +==============================+
// void FrameFlip()
FrameFlip_DEFINITION(Win32_FrameFlip)
{
	glfwSwapBuffers(PlatformInfo_->window);
}

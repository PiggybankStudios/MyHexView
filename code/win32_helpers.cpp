/*
File:   win32_helpers.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds some helper functions that help us carry out specific tasks 
*/

//TODO: This actually gets the executable directory. The name is misleading. Should be renamed
u32 GetRunningDirectory(char* buffer, u32 maxBufferSize)
{
	DWORD moduleFilenameLength = GetModuleFileNameA(0, buffer, maxBufferSize);
	char* lastSlash = buffer;
	for(char* scan = buffer; *scan != '\0'; scan++)
	{
		if (*scan == '\\')
		{
			lastSlash = scan + 1;
		}
	}
	memset(lastSlash, '\0', buffer + maxBufferSize - lastSlash);
	
	return (u32)(lastSlash - buffer);
}

#if 0
inline FILETIME GetFileWriteTime(const char* filename)
{
	FILETIME lastWriteTime = {};
	
	WIN32_FIND_DATA findData;
	HANDLE fileHandle = FindFirstFileA(filename, &findData);
	
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		lastWriteTime = findData.ftLastWriteTime;
		FindClose(fileHandle);
	}
	
	return lastWriteTime;
}
#endif

void UpdateWindowTitle(GLFWwindow* window, const char* baseName, Version_t* platformVersion, Version_t* appVersion)
{
	char windowTitle[128] = {};
	
	#if DEBUG
		snprintf(windowTitle, ArrayCount(windowTitle),
			"%s (Platform %u.%u:%03u App %u.%u:%03u)", baseName,
			platformVersion->major, platformVersion->minor, platformVersion->build,
			appVersion->major, appVersion->minor, appVersion->build);
	#else
		snprintf(windowTitle, ArrayCount(windowTitle),
			"%s (v%u.%u)", baseName,
			appVersion->major, appVersion->minor);
	#endif
	
	glfwSetWindowTitle(window, windowTitle);
}


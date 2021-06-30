/*
File:   win32_files.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds the functions that handle File IO on Windows
*/

// +==============================+
// |     Win32_FreeFileMemory     |
// +==============================+
// void FreeFileMemory(FileInfo_t* fileInfo)
FreeFileMemory_DEFINITION(Win32_FreeFileMemory)
{
	if(fileInfo->content != nullptr)
	{
		VirtualFree(fileInfo->content, 0, MEM_RELEASE);
		fileInfo->content = nullptr;
	}
}

// +==============================+
// |     Win32_ReadEntireFile     |
// +==============================+
// FileInfo_t ReadEntireFile(const char* filename)
ReadEntireFile_DEFINITION(Win32_ReadEntireFile)
{
	FileInfo_t result = {};
	
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if(GetFileSizeEx(fileHandle, &fileSize))
		{
			//TODO: Define and use SafeTruncateUInt64 
			uint32 fileSize32 = (uint32)(fileSize.QuadPart);
			result.content = VirtualAlloc(0, fileSize32+1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if(result.content)
			{
				DWORD bytesRead;
				if(ReadFile(fileHandle, result.content, fileSize32, &bytesRead, 0) &&
				   (fileSize32 == bytesRead))
				{
					// NOTE: File read successfully
					result.size = fileSize32;
					((u8*)result.content)[fileSize32] = '\0';
				}
				else
				{
					Win32_FreeFileMemory(&result);
					result.content = 0;
				}
			}
			else
			{
				// TODO: Logging
			}
		}
		else
		{
			// TODO: Logging
		}

		CloseHandle(fileHandle);
	}
	else
	{
		// TODO(casey): Logging
	}

	return result;
}

// +==============================+
// |    Win32_WriteEntireFile     |
// +==============================+
// bool WriteEntireFile(const char* filename, void* memory, uint32 memorySize)
WriteEntireFile_DEFINITION(Win32_WriteEntireFile)
{
	bool result = false;
	
	HANDLE fileHandle = CreateFileA(
		filename,              //Name of the file
		GENERIC_WRITE,         //Open for writing
		0,                     //Do not share
		NULL,                  //Default security
		CREATE_ALWAYS,         //Always overwrite
		FILE_ATTRIBUTE_NORMAL, //Default file attributes
		0                      //No Template File
	);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
		{
			// NOTE: File read successfully
			result = (bytesWritten == memorySize);
		}
		else
		{
			// TODO: Logging
		}

		CloseHandle(fileHandle);
	}
	else
	{
		// TODO: Logging
	}

	return result;
}

// +==============================+
// |        Win32_OpenFile        |
// +==============================+
// bool OpenFile(const char* fileName, OpenFile_t* openFileOut)
OpenFile_DEFINITION(Win32_OpenFile)
{
	Assert(openFileOut != nullptr);
	Assert(fileName != nullptr);
	
	HANDLE fileHandle = CreateFileA(
		fileName,              //Name of the file
		GENERIC_WRITE,         //Open for reading and writing
		FILE_SHARE_READ,       //Do not share
		NULL,                  //Default security
		OPEN_ALWAYS,           //Open existing or create new
		FILE_ATTRIBUTE_NORMAL, //Default file attributes
		NULL                   //No Template File
	);
	
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		Win32_PrintLine("Couldn't open/create file: \"%s\"", fileName);
		return false;
	}
	
	SetFilePointer(fileHandle, 0, NULL, FILE_END);
	
	openFileOut->isOpen = true;
	openFileOut->handle = fileHandle;
	return true;
}

// +==============================+
// |       Win32_AppendFile       |
// +==============================+
// bool AppendFile(OpenFile_t* filePntr, const void* newData, u32 newDataSize)
AppendFile_DEFINITION(Win32_AppendFile)
{
	Assert(filePntr != nullptr);
	Assert(filePntr->isOpen);
	Assert(filePntr->handle != INVALID_HANDLE_VALUE);
	
	if (newDataSize == 0) return true;
	Assert(newData != nullptr);
	
	DWORD bytesWritten = 0;
	if (WriteFile(filePntr->handle, newData, newDataSize, &bytesWritten, 0))
	{
		// NOTE: File read successfully
		if (bytesWritten == newDataSize)
		{
			return true;
		}
		else
		{
			Win32_PrintLine("Not all bytes appended to file. %u/%u written", bytesWritten, newDataSize);
			return false;
		}
	}
	else
	{
		Win32_PrintLine("WriteFile failed. %u/%u written", bytesWritten, newDataSize);
		return false;
	}
}

// +==============================+
// |       Win32_CloseFile        |
// +==============================+
// void CloseFile(OpenFile_t* filePntr)
CloseFile_DEFINITION(Win32_CloseFile)
{
	if (filePntr == nullptr) return;
	if (filePntr->handle == INVALID_HANDLE_VALUE) return;
	if (filePntr->isOpen == false) return;
	
	CloseHandle(filePntr->handle);
	filePntr->handle = INVALID_HANDLE_VALUE;
	filePntr->isOpen = false;
}

// +==============================+
// |       Win32_LaunchFile       |
// +==============================+
// bool LaunchFile(const char* filename)
LaunchFile_DEFINITION(Win32_LaunchFile)
{
	char realPath[256] = {};
	strncpy(realPath, filename, sizeof(realPath));
	//Replace '/' with '\' for windows compatibility
	for (u32 cIndex = 0; realPath[cIndex] != '\0' && cIndex < sizeof(realPath); cIndex++)
	{
		if (realPath[cIndex] == '/')
		{
			realPath[cIndex] = '\\';
		}
	}
	
	u64 executeResult = (u64)ShellExecute(
		NULL,   //No parent window
		"open", //The action verb
		realPath, //The target file
		NULL, //No parameters
		NULL, //Use default working directory
		SW_SHOWNORMAL //Show command is normal
	);
	
	if (executeResult > 32)
	{
		return true;
	}
	else
	{
		switch (executeResult)
		{
			case ERROR_FILE_NOT_FOUND:
			{
				Win32_WriteLine("ShellExecute returned ERROR_FILE_NOT_FOUND");
			} break;
			
			default:
			{
				Win32_PrintLine("ShellExecute failed with result 0x%02X", executeResult);
			} break;
		};
		
		return false;
	}
}

// +==============================+
// |      Win32_GetFileTime       |
// +==============================+
// bool GetFileTime(const char* filePath, FileTime_t* fileTimeOut);
GetFileTime_DEFINITION(Win32_GetFileTime)
{
	Assert(filePath != nullptr);
	Assert(fileTimeOut != nullptr);
	ClearPointer(fileTimeOut);
	
	WIN32_FIND_DATA findData;
	HANDLE fileHandle = FindFirstFileA(filePath, &findData);
	
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		fileTimeOut->value = findData.ftLastWriteTime;
		FindClose(fileHandle);
		return true;
	}
	else
	{
		return false;
	}
}

// +==============================+
// |    Win32_CompareFileTimes    |
// +==============================+
// i32 CompareFileTimes(const FileTime_t* fileTime1, const FileTime_t* fileTime2)
CompareFileTimes_DEFINITION(Win32_CompareFileTimes)
{
	Assert(fileTime1 != nullptr);
	Assert(fileTime2 != nullptr);
	
	i32 result = CompareFileTime(&fileTime1->value, &fileTime2->value);
	return result;
}

// +==============================+
// |  Win32_GetNumFilesInFolder   |
// +==============================+
// u32 GetNumFilesInFolder(const char* folderPath)
u32 Win32_GetNumFilesInFolder(const char* folderPath)
{
	Assert(folderPath != nullptr);
	u32 result = 0;
	TempPushMark();
	
	u32 folderPathLength = (u32)strlen(folderPath);
	char* tempDirBuffer = PushArray(TempArena, char, folderPathLength+2);
	memcpy(tempDirBuffer, folderPath, folderPathLength);
	tempDirBuffer[folderPathLength] = '*';
	tempDirBuffer[folderPathLength+1] = '\0';
	StrReplaceCharInPlace(NtStr(tempDirBuffer), '/', '\\');
	Assert(tempDirBuffer[folderPathLength-1] == '\\');
	// Win32_PrintLine("Counting files in \"%s\"", tempDirBuffer);
	
	WIN32_FIND_DATA findData = {};
	HANDLE findHandle = FindFirstFile(tempDirBuffer, &findData);
	while (true)
	{
		if (findHandle == INVALID_HANDLE_VALUE) { break; }
		
		if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
		{
			// Win32_PrintLine("Found file \"%s\"", findData.cFileName);
			result++;
		}
		
		if (FindNextFile(findHandle, &findData) == 0)
		{
			break;
		}
	}
	
	TempPopMark();
	return result;
}

// +==============================+
// |    Win32_GetFileInFolder     |
// +==============================+
// char* GetFileInFolder(MemoryArena_t* arenaPntr, const char* folderPath, u32 fileIndex)
char* Win32_GetFileInFolder(MemoryArena_t* arenaPntr, const char* folderPath, u32 fileIndex)
{
	Assert(folderPath != nullptr);
	char* result = nullptr;
	TempPushMark();
	
	u32 folderPathLength = (u32)strlen(folderPath);
	char* tempDirBuffer = PushArray(TempArena, char, folderPathLength+2);
	memcpy(tempDirBuffer, folderPath, folderPathLength);
	tempDirBuffer[folderPathLength] = '*';
	tempDirBuffer[folderPathLength+1] = '\0';
	StrReplaceCharInPlace(NtStr(tempDirBuffer), '/', '\\');
	Assert(tempDirBuffer[folderPathLength-1] == '\\');
	
	WIN32_FIND_DATA findData = {};
	HANDLE findHandle = FindFirstFile(tempDirBuffer, &findData);
	u32 fIndex = 0;
	while (true)
	{
		if (findHandle == INVALID_HANDLE_VALUE) { break; }
		
		if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
		{
			// Win32_PrintLine("Found file \"%s\"", findData.cFileName);
			if (fIndex == fileIndex)
			{
				result = ArenaNtString(arenaPntr, findData.cFileName);
				break;
			}
			fIndex++;
		}
		
		if (FindNextFile(findHandle, &findData) == 0)
		{
			break;
		}
	}
	
	TempPopMark();
	return result;
}

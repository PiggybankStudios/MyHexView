/*
File:   plat_clipboard.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds platform layer functions for copying data to and form the clipboard
*/

// +==============================+
// |     PLT_CopyToClipboard      |
// +==============================+
// void CopyToClipboard(const void* dataPntr, u32 dataSize)
CopyToClipboard_DEFINITION(PLT_CopyToClipboard)
{
	char* tempSpace = (char*)malloc(dataSize+1);
	memcpy(tempSpace, dataPntr, dataSize);
	tempSpace[dataSize] = '\0';
	
	glfwSetClipboardString(PlatformInfo->window, tempSpace);
	
	free(tempSpace);
}

// +==============================+
// |    PLT_CopyFromClipboard     |
// +==============================+
// void* CopyFromClipboard(MemoryArena_t* arenaPntr, u32* dataLengthOut)
CopyFromClipboard_DEFINITION(PLT_CopyFromClipboard)
{
	*dataLengthOut = 0;
	
	const char* contents = glfwGetClipboardString(PlatformInfo->window);
	if (contents == nullptr) { return nullptr; }
	
	*dataLengthOut = (u32)strlen(contents);

	return (void*)contents;
}


/*
File:   win32_debug.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds the debug output functions for Windows 
*/

#define DEBUG_PRINT_BUFFER_SIZE 2048

#if SHOW_CONSOLE_WINDOW
	void Win32_Write(const char* message)
	{
		OutputDebugStringA(message);
		printf(message);
	}

	void Win32_WriteLine(const char* message)
	{
		OutputDebugStringA(message);
		OutputDebugStringA("\n");
		printf(message);
		printf("\n");
	}

	void Win32_Print(const char* formatString, ...)
	{
		char printBuffer[DEBUG_PRINT_BUFFER_SIZE];
		va_list args;
		
		va_start(args, formatString);
		size_t length = vsnprintf(printBuffer, DEBUG_PRINT_BUFFER_SIZE, formatString, args);
		va_end(args);
		
		if (length == 0)
		{
			
		}
		else if (length < DEBUG_PRINT_BUFFER_SIZE)
		{
			printBuffer[length] = '\0';
			OutputDebugStringA(printBuffer);
			printf(printBuffer);
		}
		else
		{
			OutputDebugStringA("[DEBUG PRINT BUFFER OVERFLOW]");
			printf("[DEBUG PRINT BUFFER OVERFLOW]");
		}
	}
	void Win32_PrintLine(const char* formatString, ...)
	{
		char printBuffer[DEBUG_PRINT_BUFFER_SIZE];
		va_list args;
		
		va_start(args, formatString);
		size_t length = vsnprintf(printBuffer, DEBUG_PRINT_BUFFER_SIZE, formatString, args);
		va_end(args);
		
		if (length == 0)
		{
			
		}
		else if (length < DEBUG_PRINT_BUFFER_SIZE)
		{
			printBuffer[length] = '\0';
			OutputDebugStringA(printBuffer);
			printf(printBuffer);
		}
		else
		{
			OutputDebugStringA("[DEBUG PRINT BUFFER OVERFLOW]");
			printf("[DEBUG PRINT BUFFER OVERFLOW]");
		}
		
		OutputDebugStringA("\n");
		printf("\n");
	}
#else
	void Win32_Write(const char* formatString) {}
	void Win32_WriteLine(const char* formatString) { }
	void Win32_Print(const char* formatString, ...) {}
	void Win32_PrintLine(const char* formatString, ...) {}
#endif

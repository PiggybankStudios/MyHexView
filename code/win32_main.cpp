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

// +--------------------------------------------------------------+
// |                     Common Source Files                      |
// +--------------------------------------------------------------+
#include "plat_main.h"

#include "plat_keymap.cpp"
#include "plat_callbacks.cpp"
// #include "plat_clipboard.cpp"

// +--------------------------------------------------------------+
// |                     Windows Source Files                     |
// +--------------------------------------------------------------+
#include "win32_debug.cpp"
#include "win32_helpers.cpp"
#include "win32_files.cpp"
// #include "win32_appLoading.cpp"
// #include "win32_clipboard.cpp"

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
	
	//TODO: Add a platform layer
	
	return 1;
}


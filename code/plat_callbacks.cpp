/*
File:   plat_callbacks.cpp
Author: Taylor Robbins
Date:   03\02\2018
Description: 
	** Holds all the GLFW callback functions that we use to get various types of input
*/

void GlfwErrorCallback(i32 errorCode, const char* description)
{
	DEBUG_PrintLine("GlfwErrorCallback: %X \"%s\"", errorCode, description);
}
void GlfwWindowCloseCallback(GLFWwindow* window)
{
	DEBUG_WriteLine("Window Closing");
	
	//TODO: Should we dissallow the close in some scenarios?
	// glfwSetWindowShouldClose(window, false);
}
void GlfwWindowSizeCallback(GLFWwindow* window, i32 screenWidth, i32 screenHeight)
{
	DEBUG_PrintLine("Resized: %dx%d", screenWidth, screenHeight);
	
	PlatformInfo.screenSize = NewVec2i(screenWidth, screenHeight);
	PlatformInfo.windowResized = true;
}
void GlfwWindowMoveCallback(GLFWwindow* window, i32 posX, i32 posY)
{
	// DEBUG_PrintLine("Received GlfwWindowMoveCallback: (%d, %d)", posX, posY);
}
void GlfwWindowMinimizeCallback(GLFWwindow* window, i32 isMinimized)
{
	DEBUG_PrintLine("Window %s", isMinimized ? "Minimized" : "Restored");
	PlatformInfo.windowIsMinimized = (isMinimized > 0);
}
void GlfwWindowFocusCallback(GLFWwindow* window, i32 isFocused)
{
	DEBUG_PrintLine("Window %s focus!", isFocused ? "Gained" : "Lost");
	PlatformInfo.windowHasFocus = (isFocused > 0);
}
void GlfwKeyPressedCallback(GLFWwindow* window, i32 key, i32 scanCode, i32 action, i32 modifiers)
{
	// const char* actionStr = "Pressed";
	// if (action == GLFW_REPEAT) actionStr = "Repeated";
	// if (action == GLFW_RELEASE) actionStr = "Released";
	// DEBUG_PrintLine("KeyPress %d %d %s (%d)", key, scanCode, actionStr, modifiers);
	
	HandleKeyEvent(window, AppInput, key, action);
	
	//Push enter, tab, and backspace characters onto the textInput
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		if (key == GLFW_KEY_ENTER || key == GLFW_KEY_TAB || key == GLFW_KEY_BACKSPACE)
		{
			char newChar = '?';
			if (key == GLFW_KEY_ENTER)     { newChar = '\n'; }
			if (key == GLFW_KEY_TAB)       { newChar = '\t'; }
			if (key == GLFW_KEY_BACKSPACE) { newChar = '\b'; }
			
			if (AppInput->textInputLength < ArrayCount(AppInput->textInput))
			{
				AppInput->textInput[AppInput->textInputLength] = newChar;
				AppInput->textInputLength++;
			}
		}
	}
}
void GlfwCharPressedCallback(GLFWwindow* window, u32 codepoint)
{
	// DEBUG_PrintLine("Text Input: 0x%X", codepoint);
	
	if (AppInput->textInputLength < ArrayCount(AppInput->textInput))
	{
		AppInput->textInput[AppInput->textInputLength] = (u8)codepoint;
		AppInput->textInputLength++;
	}
}
void GlfwCursorPosCallback(GLFWwindow* window, real64 mouseX, real64 mouseY)
{
	// DEBUG_PrintLine("Received GlfwCursorPosCallback: (%f, %f)", mouseX, mouseY);
	
	AppInput->mousePos = NewVec2((r32)mouseX, (r32)mouseY);
	
	if (AppInput->buttons[MouseButton_Left].isDown)
	{
		r32 distance = Vec2Length(AppInput->mousePos - AppInput->mouseStartPos[MouseButton_Left]);
		if (distance > AppInput->mouseMaxDist[MouseButton_Left])
		{
			AppInput->mouseMaxDist[MouseButton_Left] = distance;
		}
	}
	if (AppInput->buttons[MouseButton_Right].isDown)
	{
		r32 distance = Vec2Length(AppInput->mousePos - AppInput->mouseStartPos[MouseButton_Right]);
		if (distance > AppInput->mouseMaxDist[MouseButton_Right])
		{
			AppInput->mouseMaxDist[MouseButton_Right] = distance;
		}
	}
	if (AppInput->buttons[MouseButton_Middle].isDown)
	{
		r32 distance = Vec2Length(AppInput->mousePos - AppInput->mouseStartPos[MouseButton_Middle]);
		if (distance > AppInput->mouseMaxDist[MouseButton_Middle])
		{
			AppInput->mouseMaxDist[MouseButton_Middle] = distance;
		}
	}
}
void GlfwMousePressCallback(GLFWwindow* window, i32 button, i32 action, i32 modifiers)
{
	// const char* actionStr = "Pressed";
	// if (action == GLFW_REPEAT) actionStr = "Repeated";
	// if (action == GLFW_RELEASE) actionStr = "Released";
	// DEBUG_PrintLine("MousePress %d %s (%d)", button, actionStr, modifiers);
	
	//TODO: Handle repeated keys?
	if (action == GLFW_REPEAT)
		return;
	
	HandleMouseEvent(window, AppInput, button, action == GLFW_PRESS);
}
void GlfwMouseScrollCallback(GLFWwindow* window, real64 deltaX, real64 deltaY)
{
	// DEBUG_PrintLine("Received GlfwMouseScrollCallback: %f, %f", deltaX, deltaY);
	
	AppInput->scrollValue += NewVec2((r32)deltaX, (r32)deltaY);
	AppInput->scrollDelta += NewVec2((r32)deltaX, (r32)deltaY);
}
void GlfwCursorEnteredCallback(GLFWwindow* window, i32 entered)
{
	DEBUG_PrintLine("Mouse %s window", entered ? "entered" : "left");
	AppInput->mouseInsideWindow = (entered > 0);
}
void GlfwDropCallback(GLFWwindow* window, i32 numPaths, const char** paths)
{
	DEBUG_PrintLine("Dropped %d paths", numPaths);
	for (i32 pIndex = 0; pIndex < numPaths; pIndex++)
	{
		const char* path = paths[pIndex];
		DEBUG_PrintLine("[%d]: \"%s\"", pIndex, path);
		
		if (AppInput->numDroppedFiles < ArrayCount(AppInput->droppedFiles))
		{
			u32 pathLength = (u32)strlen(path);
			char* tempLocation = (char*)malloc(pathLength+1);
			memcpy(tempLocation, path, pathLength);
			tempLocation[pathLength] = '\0';
			AppInput->droppedFiles[AppInput->numDroppedFiles] = tempLocation;
			AppInput->numDroppedFiles++;
		}
		else
		{
			DEBUG_WriteLine("No space for more files in the droppedFiles input buffer");
		}
	}
}

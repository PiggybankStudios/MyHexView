/*
File:   app_default.cpp
Author: Taylor Robbins
Date:   03\03\2018
Description: 
	** Holds the functions for AppState_Default which is a multi-purpose area for random small things
*/

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void InitializeDefaultState()
{
	//TODO: Initialize stuff
	
	defData->initialized = true;
}

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartDefaultState(AppState_t fromState)
{
	//TODO: Set up some stuff?
}

// +--------------------------------------------------------------+
// |                         Deinitialize                         |
// +--------------------------------------------------------------+
void DeinitializeDefaultState()
{
	//TODO: Deallocate stuff
	
	ClearPointer(defData);
}

// +--------------------------------------------------------------+
// |                      Update And Render                       |
// +--------------------------------------------------------------+
void UpdateAndRenderDefaultState()
{
	// +--------------------------------------------------------------+
	// |                            Update                            |
	// +--------------------------------------------------------------+
	{
		
	}
	
	// +--------------------------------------------------------------+
	// |                            Render                            |
	// +--------------------------------------------------------------+
	{
		RcBegin(&app->defaultShader, &app->defaultFont, NewRec(Vec2_Zero, RenderScreenSize));
		RcClearColorBuffer(NewColor(Color_Yellow));
		RcClearDepthBuffer(1.0f);
		
		RcDrawRectangle(NewRec(RenderMousePos, NewVec2(100)), NewColor(Color_Red));
	}
}



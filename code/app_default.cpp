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
	defData->testTexture = LoadTexture(TEXTURES_FOLDER "test.png");
	defData->circuitTexture = LoadTexture(TEXTURES_FOLDER "circuit.png");
	defData->missingTexture = LoadTexture(TEXTURES_FOLDER "something_that_doesnt_exit.png");
	
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
		// +==============================+
		// | Change Background Color Keys |
		// +==============================+
		if (ButtonPressed(Button_Right))
		{
			defData->backgroundColorIndex++;
			if (defData->backgroundColorIndex >= NUM_COLORS) { defData->backgroundColorIndex = 0; }
		}
		if (ButtonPressed(Button_Left))
		{
			if (defData->backgroundColorIndex > 0) { defData->backgroundColorIndex--; }
			else { defData->backgroundColorIndex = NUM_COLORS-1; }
		}
	}
	
	// +--------------------------------------------------------------+
	// |                            Render                            |
	// +--------------------------------------------------------------+
	{
		// +==============================+
		// |            Setup             |
		// +==============================+
		Color_t backgroundColor = NewColor(GetColorByIndex(defData->backgroundColorIndex));
		{
			RcBegin(&app->defaultShader, &app->defaultFont, NewRec(Vec2_Zero, RenderScreenSize));
			RcClearColorBuffer(backgroundColor);
			RcClearDepthBuffer(1.0f);
		}
		
		RcDrawString(GetColorName(backgroundColor.value), NewVec2(5, app->defaultFont.maxExtendUp), ColorComplimentary(backgroundColor));
		
		v2 drawPos = NewVec2(0, 100);
		
		RcDrawTexture(&defData->testTexture, drawPos);
		drawPos.x += renderContext->boundTexture->width;
		
		RcDrawTexture(&defData->circuitTexture, drawPos);
		drawPos.x += renderContext->boundTexture->width;
		
		RcDrawTexture(&defData->missingTexture, drawPos);
		drawPos.x += renderContext->boundTexture->width;
		
		rec colorRec = NewRec(0, 25, 20, 50);
		for (u32 cIndex = 0; cIndex < 16; cIndex++)
		{
			Color_t recColor = NewColor(GetColorByIndex(cIndex));
			RcDrawRectangle(colorRec, recColor);
			colorRec.x += colorRec.width;
		}
		
		RcDrawCircle(RenderMousePos, 10.0f, NewColor(Color_Navy));
	}
}



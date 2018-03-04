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
	defData->backgroundColorIndex = GetColorIndex(Color_IvoryBlack);
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
		
		const char* scriptPath = SCRIPTS_FOLDER "test.py";
		if (ButtonPressed(Button_Enter))
		{
			DEBUG_PrintLine("Running python script \"%s\"", scriptPath);
			FileInfo_t scriptFile = platform->ReadEntireFile(scriptPath);
			if (scriptFile.content != nullptr)
			{
				PyRun_SimpleString((const char*)scriptFile.content);
				
				platform->FreeFileMemory(&scriptFile);
			}
			else
			{
				DEBUG_WriteLine("Could not open file");
			}
		}
		
		if (ButtonPressed(Button_Space))
		{
			char* absScriptPath = platform->GetAbsolutePath(TempArena, scriptPath);
			
			PyObject* pyModule = PyImport_ImportModule("Resources.Scripts.myModule");
			DEBUG_PrintLine("pyModule = %p", pyModule);
			PyImport_ReloadModule(pyModule);
			
			if (pyModule != nullptr)
			{
				PyObject* pyFunction = PyObject_GetAttrString(pyModule, "SomeFunction");
				
				if (pyFunction != nullptr && PyCallable_Check(pyFunction))
				{
					long inputValue = 1000;
					
					PyObject* pyFuncArgs = PyTuple_New(1);
					PyObject* pyArg0 = PyLong_FromLong(inputValue);
					PyTuple_SetItem(pyFuncArgs, 0, pyArg0);
					
					PyObject* pyReturnValue = PyObject_CallObject(pyFunction, pyFuncArgs);
					Py_DECREF(pyFuncArgs);
					
					if (pyReturnValue != nullptr)
					{
						long result = PyLong_AsLong(pyReturnValue);
						DEBUG_PrintLine("SomeFunction returned %d", result);
						Py_DECREF(pyReturnValue);
					}
					else
					{
						DEBUG_WriteLine("Failed to call function");
					}
				}
				else
				{
					DEBUG_WriteLine("Couldn't find SomeFunction in the python script");
				}
				
				Py_XDECREF(pyFunction);
				
				PyObject* pyObject = PyObject_GetAttrString(pyModule, "SomeClass");
				DEBUG_PrintLine("pyObject = %p", pyObject);
				if (pyObject != nullptr && PyType_Check(pyObject))
				{
					PyTypeObject* pyType = (PyTypeObject*)pyObject;
					PyObject* pyCreateArgs = PyTuple_New(0);
					PyObject* pyCreateKeywords = PyTuple_New(0);
					PyObject* pyInstance = PyType_GenericNew(pyType, pyCreateArgs, pyCreateKeywords);
					if (pyInstance != nullptr)
					{
						PyObject* pyInstanceFunc = PyObject_GetAttrString(pyInstance, "PrintLocalVar");
						if (pyInstanceFunc != nullptr && PyCallable_Check(pyInstanceFunc))
						{
							for (u32 cIndex = 0; cIndex < 4; cIndex++)
							{
								PyObject* pyFuncArgs = PyTuple_New(0);
								// PyTuple_SetItem(pyFuncArgs, 0, pyInstance);
								
								DEBUG_PrintLine("Calling instance function x%u", cIndex);
								PyObject* pyReturnValue = PyObject_CallObject(pyInstanceFunc, pyFuncArgs);
								if (pyReturnValue != nullptr)
								{
									DEBUG_WriteLine("Call successful");
								}
								else
								{
									DEBUG_WriteLine("Call failed");
								}
								
								Py_XDECREF(pyFuncArgs);
								Py_XDECREF(pyReturnValue);
							}
						}
						else
						{
							DEBUG_WriteLine("Couldn't find instance function");
						}
						
						Py_XDECREF(pyInstanceFunc);
					}
					else
					{
						DEBUG_WriteLine("Failed to create class instance");
					}
					
					Py_XDECREF(pyCreateArgs);
					Py_XDECREF(pyCreateKeywords);
					Py_XDECREF(pyInstance);
				}
				else
				{
					DEBUG_WriteLine("Couldn't find class object");
				}
				
				Py_XDECREF(pyObject);
				Py_DECREF(pyModule);
			}
			else
			{
				DEBUG_WriteLine("Couldn't open python file as module");
			}
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
		
		if (input->mouseInsideWindow)
		{
			RcDrawDonut(RenderMousePos, 11.0f, 4.0f, NewColor(Color_IvoryBlack));
			RcDrawDonut(RenderMousePos, 10.0f, 5.0f, NewColor(Color_GhostWhite));
		}
		
		RcDrawLineArrow(NewVec2(RenderScreenSize.width - 10, 0), NewVec2(RenderScreenSize.width - 10, RenderScreenSize.height), 10.0f, 2.0f, NewColor(Color_Red));
		RcDrawLineArrow(NewVec2(RenderScreenSize.width - 10, RenderScreenSize.height), NewVec2(RenderScreenSize.width - 10, 0), 10.0f, 2.0f, NewColor(Color_Red));
		char* heightStr = TempPrint("%.0f", RenderScreenSize.height);
		RcDrawString(heightStr, NewVec2(RenderScreenSize.width-10, RenderScreenSize.height/2), NewColor(Color_Red), 1.0f, Alignment_Right);
		
		u32 ratioWidth, ratioHeight;
		FindRatio((u32)RenderScreenSize.width, (u32)RenderScreenSize.height, &ratioWidth, &ratioHeight);
		char* ratioStr = TempPrint("%u:%u", ratioWidth, ratioHeight);
		RcDrawString(ratioStr, NewVec2(RenderScreenSize.width-20, RenderScreenSize.height-15-app->defaultFont.maxExtendDown), NewColor(Color_Red), 1.0f, Alignment_Right);
		
		RcDrawLineArrow(NewVec2(0, RenderScreenSize.height - 10), NewVec2(RenderScreenSize.width, RenderScreenSize.height - 10), 10.0f, 2.0f, NewColor(Color_Red));
		RcDrawLineArrow(NewVec2(RenderScreenSize.width, RenderScreenSize.height - 10), NewVec2(0, RenderScreenSize.height - 10), 10.0f, 2.0f, NewColor(Color_Red));
		char* widthStr = TempPrint("%.0f", RenderScreenSize.width);
		RcDrawString(widthStr, NewVec2(RenderScreenSize.width/2, RenderScreenSize.height - 10 - app->defaultFont.maxExtendDown), NewColor(Color_Red), 1.0f, Alignment_Center);
	}
}



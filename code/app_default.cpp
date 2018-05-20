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
	
	defData->quadStart = NewVec2(10, 100);
	defData->quadControl = NewVec2(50, 130);
	defData->quadEnd = NewVec2(210, 100);
	
	defData->cubicStart = NewVec2(10, 100);
	defData->cubicControl1 = NewVec2(50, 130);
	defData->cubicControl2 = NewVec2(150, 30);
	defData->cubicEnd = NewVec2(210, 100);
	
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
		
		// +==============================+
		// |       Rec Packing Test       |
		// +==============================+
		StartTimeBlock("Rect Packing");
		if (ButtonDown(Button_Control) && ButtonDown(Button_P))
		{
			if (defData->testPackRecs != nullptr)
			{
				ArenaPop(mainHeap, defData->testPackRecs);
				defData->testPackRecs = nullptr;
			}
			
			defData->testPackBin = NewRec(RenderScreenSize - NewVec2(512 + 20), NewVec2(512));
			defData->numTestPackRecs = 100;
			defData->testPackRecs = PushArray(mainHeap, rec, defData->numTestPackRecs);
			for (u32 rIndex = 0; rIndex < defData->numTestPackRecs; rIndex++)
			{
				defData->testPackRecs[rIndex] = NewRec(0, 0, (r32)RandU32(1, 100), (r32)RandU32(1, 100));
				defData->testPackRecs[rIndex].topLeft = defData->testPackBin.topLeft - defData->testPackRecs[rIndex].size;
			}
			
			TempPushMark();
			{
				stbrp_context stbContext;
				i32 numNodes = (i32)defData->testPackBin.width;
				stbrp_node* stbNodes = PushArray(TempArena, stbrp_node, numNodes);
				stbrp_init_target(&stbContext, (i32)defData->testPackBin.width, (i32)defData->testPackBin.height, stbNodes, numNodes);
				
				TempPushMark();
				{
					stbrp_rect* stbRecs = PushArray(TempArena, stbrp_rect, defData->numTestPackRecs);
					for (u32 rIndex = 0; rIndex < defData->numTestPackRecs; rIndex++)
					{
						stbRecs[rIndex].id = (i32)rIndex;
						stbRecs[rIndex].w = (stbrp_coord)defData->testPackRecs[rIndex].width;
						stbRecs[rIndex].h = (stbrp_coord)defData->testPackRecs[rIndex].height;
					}
					
					stbrp_pack_rects(&stbContext, stbRecs, (i32)defData->numTestPackRecs);
					
					u32 numUnpacked = 0;
					r32 unpackLineupPos = 0;
					for (u32 rIndex = 0; rIndex < defData->numTestPackRecs; rIndex++)
					{
						stbrp_rect* stbRec = &stbRecs[rIndex];
						Assert(stbRec->id >= 0 && (u32)stbRec->id < defData->numTestPackRecs);
						rec* ourRec = &defData->testPackRecs[stbRec->id];
						if (stbRec->was_packed)
						{
							ourRec->x = defData->testPackBin.x + (r32)stbRec->x;
							ourRec->y = defData->testPackBin.y + (r32)stbRec->y;
						}
						else
						{
							ourRec->x = defData->testPackBin.x;
							ourRec->y = defData->testPackBin.y - ourRec->height - unpackLineupPos;
							numUnpacked++;
							unpackLineupPos += ourRec->height;
						}
					}
					
					DEBUG_PrintLine("Failed to pack %u rectangles", numUnpacked);
				}
				TempPopMark();
			}
			TempPopMark();
		}
		EndTimeBlock();
		
		// +==============================+
		// |     Refresh Algebra Test     |
		// +==============================+
		if (ButtonPressed(Button_A))
		{
			if (defData->algGroup.terms != nullptr)
			{
				DestroyAlgGroup(&defData->algGroup);
			}
			
			CreateAlgGroup(&defData->algGroup, mainHeap, 8, nullptr);
			
			AlgGroup_t group1 = {};
			CreateAlgGroup(&group1, mainHeap, 2, nullptr);
			AlgGroupAddTerm(&group1, NewAlgTermFromString("a"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("-3at"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("+3bt"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("+3att"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("-6btt"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("+3ctt"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("-attt"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("+3bttt"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("-3cttt"));
			AlgGroupAddTerm(&group1, NewAlgTermFromString("+dttt"));
			
			MultiplyGroups(&group1, &group1, &defData->algGroup, mainHeap);
			
			DestroyAlgGroup(&group1);
			
		}
		if (ButtonPressed(Button_C))
		{
			CombineLikeTerms(&defData->algGroup);
			
			if (ButtonDown(Button_Control))
			{
				TempPushMark();
				char* groupStr = nullptr;
				u32 groupStrLength = GetAlgGroupStr(TempArena, &defData->algGroup, &groupStr);
				platform->CopyToClipboard(groupStr, groupStrLength);
				TempPopMark();
			}
		}
		
		// +==============================+
		// |      Change Quad Curve       |
		// +==============================+
		{
			if (ButtonDown(MouseButton_Left))
			{
				defData->cubicControl1 = RenderMousePos;
			}
			if (ButtonDown(MouseButton_Right))
			{
				defData->cubicEnd = RenderMousePos;
			}
		}
		
		#if 0
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
		#endif
		
		// +--------------------------------------------------------------+
		// |                  Load Python Modules Hotkey                  |
		// +--------------------------------------------------------------+
		#if 0
		bool fileModified = false;
		FileTime_t newFileTime = {};
		if (defData->pluginModule.loaded && platform->GetFileTime(defData->pluginModule.filePath, &newFileTime))
		{
			if (platform->CompareFileTimes(&newFileTime, &defData->pluginModule.fileWriteTime) != 0)
			{
				DEBUG_PrintLine("\"%s\" was modified", defData->pluginModule.filePath);
				fileModified = true;
			}
		}
		if (ButtonPressed(Button_Backspace) || fileModified)
		{
			if (LoadPythonPluginModule(mainHeap, &defData->pluginModule, SCRIPTS_FOLDER "myModule.py"))
			{
				DEBUG_PrintLine("Module loaded successfully. %u plugins found", defData->pluginModule.numPlugins);
			}
			else
			{
				DEBUG_WriteLine("Module was not loaded successfully");
			}
		}
		#endif
		
		// +--------------------------------------------------------------+
		// |                 Call Python Plugin Functions                 |
		// +--------------------------------------------------------------+
		// +==============================+
		// |         MousePressed         |
		// +==============================+
		if (ButtonPressed(MouseButton_Left))
		{
			AllPlugins_MousePressed(RenderMousePos);
		}
		// +==============================+
		// |        ButtonPressed         |
		// +==============================+
		for (u32 bIndex = Button_A; bIndex < Buttons_NumButtons; bIndex++)
		{
			Buttons_t button = (Buttons_t)bIndex;
			if (ButtonPressed(button))
			{
				AllPlugins_ButtonPressed(GetButtonName(button));
			}
		}
		if (ButtonDown(Button_Control) && ButtonPressed(Button_R))
		{
			RunCommand("AwesomePlugin");
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
		
		// +==============================+
		// | Draw Background Color String |
		// +==============================+
		RcDrawString(GetColorName(backgroundColor.value), NewVec2(5, app->defaultFont.maxExtendUp), ColorComplimentary(backgroundColor));
		
		// +==============================+
		// |  Draw Algebra Group String   |
		// +==============================+
		{
			v2 algebraDrawPos = NewVec2(5, 200 + app->defaultFont.maxExtendUp);
			TempPushMark();
			char* groupStr = nullptr;
			GetAlgGroupStr(TempArena, &defData->algGroup, &groupStr);
			
			if (groupStr == nullptr)
			{
				RcDrawString("(nullptr)", algebraDrawPos, ColorComplimentary(backgroundColor));
			}
			else
			{
				// RcDrawString(groupStr, algebraDrawPos, ColorComplimentary(backgroundColor));
				RcDrawFormattedString(groupStr, algebraDrawPos, RenderScreenSize.width - algebraDrawPos.x, ColorComplimentary(backgroundColor), Alignment_Left, true);
			}
			TempPopMark();
		}
		
		// +==============================+
		// |       Draw Packed Recs       |
		// +==============================+
		if (defData->testPackRecs != nullptr)
		{
			StartTimeBlock("Rect Rendering");
			for (u32 rIndex = 0; rIndex < defData->numTestPackRecs; rIndex++)
			{
				RcDrawButton(defData->testPackRecs[rIndex], ColorTransparent(NewColor(Color_Red), 0.1f), NewColor(Color_Red), 2);
			}
			RcDrawButton(RecInflate(defData->testPackBin, 2), NewColor(Color_TransparentBlack), NewColor(Color_Yellow), 2);
			EndTimeBlock();
		}
		
		// +==============================+
		// |        Draw Textures         |
		// +==============================+
		#if 0
		{
			v2 drawPos = NewVec2(0, 100);
			
			RcDrawTexture(&defData->testTexture, drawPos);
			drawPos.x += renderContext->boundTexture->width;
			
			RcDrawTexture(&defData->circuitTexture, drawPos);
			drawPos.x += renderContext->boundTexture->width;
			
			RcDrawTexture(&defData->missingTexture, drawPos);
			drawPos.x += renderContext->boundTexture->width;
		}
		#endif
		
		// +==============================+
		// |   New Font Rendering Tests   |
		// +==============================+
		// StartTimeBlock("New Font Test");
		{
			// RcBindTexture(&app->defaultFont.bitmap);
			// RcDrawTexturedRec(NewRec(500, 500, (r32)app->defaultFont.bitmap.width, (r32)app->defaultFont.bitmap.height), NewColor(Color_White));
			// RcBindTexture(&app->newFont.bakes[1].texture);
			// RcDrawTexturedRec(NewRec(500, 500, (r32)app->newFont.bakes[1].texture.width, (r32)app->newFont.bakes[1].texture.height), NewColor(Color_White));
			RcPrintString(NewVec2(500, 400), NewColor(Color_White), 1.0f, "NumBakes: %u", app->newFont.numBakes);
			u32 memUsage = FontGetMemoryUsage(&app->newFont);
			RcPrintString(NewVec2(500, 420), NewColor(Color_White), 1.0f, "Memory Usage: %s/%s (%.1f%%)", FormattedSizeStr(memUsage), FormattedSizeStr(mainHeap->size), (r32)memUsage / (r32)mainHeap->size);
			
			FontBake_t* fontBake = FontGetBakeFor(&app->newFont, 32, FontStyle_Default, nullptr);
			if (fontBake != nullptr)
			{
				RcBindTexture(&fontBake->texture);
				RcDrawTexturedRec(NewRec(100, 100, (r32)fontBake->texture.width, (r32)fontBake->texture.height), NewColor(Color_White));
			}
			
			const char* printStr = "(This) is a \b\x01\xFF\xFF\x01\areally long\a\x02\b \v\x41string\v\x40 with \v\x31stuff\v\x30 like\t\t\v\x21\"quotations\"\v\x20 and [brackets]!\x03\x18 and \aother\a things too\x04 \x03\x0CPeanuts\x04\n\nWe are \r\x01\x01\xFF\xFFrendering\x02\r using \v\x61\v\x71your\v\x60 font :P";
			v2 textPos = NewVec2(500, 450);
			// r32 FontGetMaxExtendUp(NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default)
			r32 fontMaxExtendUp = FontGetMaxExtendUp(&app->newFont, 32, FontStyle_Default);
			r32 maxWidth = RenderMousePos.x - textPos.x;
			if (maxWidth < 0) { maxWidth = 0; }
			RcBindNewFont(&app->newFont);
			RcSetFontAlignment(Alignment_Left);
			// RcSetFontAlignment(Alignment_Center); maxWidth *= 2;
			RcSetFontSize(32);
			RcSetFontStyle(FontStyle_Default);
			
			FontFlowInfo_t flowInfo; ClearStruct(flowInfo);
			RcNewDrawNtString(printStr, textPos, NewColor(Color_White), maxWidth, &flowInfo);
			RcDrawRectangle(flowInfo.extents, ColorTransparent(NewColor(Color_White), 0.1f));
			// RcDrawRectangle(NewRec(flowInfo.position.x, flowInfo.position.y, flowInfo.extentRight, flowInfo.extentDown), ColorTransparent(NewColor(Color_White), 0.1f));
			// RcDrawLineArrow(flowInfo.endPos + NewVec2(10,20), flowInfo.endPos, 4, 1.0f, NewColor(Color_White));
			
			v2 arrowStartPos = textPos + NewVec2(0, -fontMaxExtendUp);
			v2 arrowEndPos = arrowStartPos + NewVec2(maxWidth, 0);
			// RcDrawLineArrow(arrowStartPos, arrowEndPos, 4, 1, NewColor(Color_White));
			RcDrawLine(arrowEndPos + NewVec2(0, -1000), arrowEndPos + NewVec2(0, 1000), 1, NewColor(Color_Red));
			
			u32 numCharsFit = FontMeasureLineWidth(printStr, (u32)strlen(printStr), &maxWidth, &app->newFont, 32, FontStyle_Default);
			// RcDrawRectangle(NewRec(flowInfo.position, NewVec2(maxWidth, flowInfo.extentDown)), ColorTransparent(NewColor(Color_White), 0.5f));
			
			// FontChar_t fontChar;
			// v2 printPos = NewVec2(500, 450);
			// for (u32 cIndex = 0; printStr[cIndex] != '\0'; cIndex++)
			// {
			// 	if (FontGetChar(&app->newFont, &fontChar, printStr[cIndex], 32))
			// 	{
			// 		RcBindTexture(fontChar.texture);
			// 		RcDrawTexturedRec(NewRec(printPos - fontChar.info->origin, fontChar.info->size), NewColor(Color_White), NewRec(fontChar.info->bakeRec));
			// 		printPos.x += fontChar.info->advanceX;
			// 	}
			// 	else
			// 	{
			// 		printPos.x += 20;
			// 	}
			// }
		}
		// EndTimeBlock();
		
		// +==============================+
		// |     Draw Color Swatches      |
		// +==============================+
		#if 1
		{
			rec colorRec = NewRec(0, 25, 20, 50);
			for (u32 cIndex = 0; cIndex < 16; cIndex++)
			{
				Color_t recColor = NewColor(GetColorByIndex(cIndex));
				RcDrawRectangle(colorRec, recColor);
				colorRec.x += colorRec.width;
			}
		}
		#endif
		
		// +==============================+
		// |      Draw Cursor Donut       |
		// +==============================+
		#if 1
		if (input->mouseInsideWindow)
		{
			RcDrawDonut(RenderMousePos, 11.0f, 4.0f, NewColor(Color_IvoryBlack));
			RcDrawDonut(RenderMousePos, 10.0f, 5.0f, NewColor(Color_GhostWhite));
		}
		#endif
		
		// +==============================+
		// |     Draw Quadratic Curve     |
		// +==============================+
		#if 0
		{
			RcDrawCircle(defData->quadStart, 3.0f, NewColor(Color_Yellow));
			RcDrawCircle(defData->quadControl, 3.0f, NewColor(Color_Yellow));
			RcDrawCircle(defData->quadEnd, 3.0f, NewColor(Color_Yellow));
			RcDrawQuadCurve(defData->quadStart, defData->quadControl, defData->quadEnd, 60, 2, NewColor(Color_Red));
		}
		#endif
		
		// +==============================+
		// |       Draw Cubic Curve       |
		// +==============================+
		#if 1
		{
			RcDrawCircle(defData->cubicStart, 3.0f, NewColor(Color_Yellow));
			RcDrawCircle(defData->cubicControl1, 3.0f, NewColor(Color_Yellow));
			RcDrawCircle(defData->cubicControl2, 3.0f, NewColor(Color_Yellow));
			RcDrawCircle(defData->cubicEnd, 3.0f, NewColor(Color_Yellow));
			RcDrawCubicCurve(defData->cubicStart, defData->cubicControl1, defData->cubicControl2, defData->cubicEnd, 60, 2, NewColor(Color_Red));
		}
		#endif
		
		// +==================================+
		// | Draw Render Screen Measurements  |
		// +==================================+
		#if 1
		{
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
		#endif
	}
}



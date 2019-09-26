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
	if (defData->testPolygon.verts != nullptr)
	{
		ArenaPop(mainHeap, &defData->testPolygon.verts);
	}
	if (defData->testTriangles != nullptr)
	{
		ArenaPop(mainHeap, defData->testTriangles);
	}
	if (defData->testPackRecs != nullptr)
	{
		ArenaPop(mainHeap, defData->testPackRecs);
	}
	
	DestroyAlgGroup(&defData->algGroup);
	DestroyTexture(&defData->testTexture);
	DestroyTexture(&defData->circuitTexture);
	DestroyTexture(&defData->missingTexture);
	
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
		// |     Glyph Rendering Test     |
		// +==============================+
		if (ButtonPressed(Button_G))
		{
			if (app->glyphFilled)
			{
				DestroyTexture(&app->glyphTexture);
				app->glyphFilled = false;
			}
			app->glyphFilled = FontGetGlyph(&app->glyphTexture, &app->japaneseFont, 128, 0x958b, FontStyle_Default);
		}
		
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
		if (false)
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
		
		// +==============================+
		// |   Check Can Place Vertice    |
		// +==============================+
		bool canPlaceVerticeHere = true;
		StartTimeBlock("Polygon Intersection");
		if (defData->testPolygon.numVerts > 1)
		{
			for (u32 vIndex = 0; vIndex+1 < defData->testPolygon.numVerts; vIndex++)
			{
				//TODO: Find a line intersection test function that I wrote?
			}
		}
		EndTimeBlock();
		
		// +==============================+
		// |     Add Vertices Polygon     |
		// +==============================+
		bool closedPolygonLoop = false;
		if (ButtonReleased(MouseButton_Left))// && input->mouseMaxDist[MouseButton_Left] < 5)
		{
			if (defData->testPolygon.numVerts == 0 || defData->polygonFinished)
			{
				if (defData->testPolygon.verts != nullptr)
				{
					ArenaPop(mainHeap, &defData->testPolygon.verts);
					defData->testPolygon.verts = nullptr;
					defData->testPolygon.numVerts = 0;
				}
				
				defData->testPolygon.numVerts = 1;
				defData->testPolygon.verts = PushArray(mainHeap, v2, 1);
				defData->testPolygon.verts[0] = RenderMousePos;
				defData->polygonFinished = false;
			}
			else
			{
				Assert(defData->testPolygon.verts != nullptr);
				if (defData->testPolygon.numVerts >= 3 && Vec2Length(RenderMousePos - defData->testPolygon.verts[0]) < 10)
				{
					closedPolygonLoop = true;
				}
				else
				{
					v2* newVerts = PushArray(mainHeap, v2, defData->testPolygon.numVerts+1);
					memcpy(newVerts, defData->testPolygon.verts, defData->testPolygon.numVerts * sizeof(v2));
					ArenaPop(mainHeap, defData->testPolygon.verts);
					defData->testPolygon.verts = newVerts;
					defData->testPolygon.verts[defData->testPolygon.numVerts] = RenderMousePos;
					defData->testPolygon.numVerts++;
				}
			}
		}
		
		// +==============================+
		// |     Trianglulate Polygon     |
		// +==============================+
		if (closedPolygonLoop)
		{
			if (defData->testPolygon.numVerts >= 3)
			{
				if (defData->testTriangles != nullptr)
				{
					ArenaPop(mainHeap, defData->testTriangles);
					defData->testTriangles = nullptr;
					defData->testNumTriangles = 0;
					DestroyVertexBuffer(&defData->trianglesBuffer);
				}
				
				defData->testTriangles = TriangulatePolygonEars(mainHeap, &defData->testPolygon, &defData->testNumTriangles);
				Assert(defData->testTriangles != nullptr || defData->testNumTriangles == 0);
				
				if (defData->testTriangles != nullptr && defData->testNumTriangles > 0)
				{
					TempPushMark();
					Vertex_t* triangleVerts = PushArray(TempArena, Vertex_t, defData->testNumTriangles * 3);
					for (u32 tIndex = 0; tIndex < defData->testNumTriangles; tIndex++)
					{
						
						triangleVerts[tIndex*3 + 0].position = NewVec3(defData->testTriangles[tIndex].p0, 0);
						triangleVerts[tIndex*3 + 0].color = NewVec4(NewColor(GetColorByIndex(tIndex)));
						triangleVerts[tIndex*3 + 0].color.a = 1.0f;
						triangleVerts[tIndex*3 + 0].texCoord = Vec2_Zero;
						
						triangleVerts[tIndex*3 + 1].position = NewVec3(defData->testTriangles[tIndex].p1, 0);
						triangleVerts[tIndex*3 + 1].color = NewVec4(NewColor(GetColorByIndex(tIndex)));
						triangleVerts[tIndex*3 + 1].color.a = 1.0f;
						triangleVerts[tIndex*3 + 1].texCoord = Vec2_Zero;
						
						triangleVerts[tIndex*3 + 2].position = NewVec3(defData->testTriangles[tIndex].p2, 0);
						triangleVerts[tIndex*3 + 2].color = NewVec4(NewColor(GetColorByIndex(tIndex)));
						triangleVerts[tIndex*3 + 2].color.a = 1.0f;
						triangleVerts[tIndex*3 + 2].texCoord = Vec2_Zero;
					}
					defData->trianglesBuffer = CreateVertexBuffer(triangleVerts, defData->testNumTriangles * 3);
					TempPopMark();
				}
			}
			
			defData->polygonFinished = true;
		}
		if (ButtonReleased(MouseButton_Right) && defData->testPolygon.verts != nullptr)
		{
			ArenaPop(mainHeap, &defData->testPolygon.verts);
			defData->testPolygon.verts = nullptr;
			defData->testPolygon.numVerts = 0;
		}
		
		if (ButtonDown(Button_T) && defData->polygonFinished)
		{
			for (u32 times = 0; times < 100; times++)
			{
				TempPushMark();
				StartTimeBlock("Triangulation");
				TriangulatePolygonEars(TempArena, &defData->testPolygon, nullptr);
				EndTimeBlock();
				TempPopMark();
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
				// RcDrawButton(defData->testPackRecs[rIndex], ColorTransparent(NewColor(Color_Red), 0.1f), NewColor(Color_Red), 2);
				RcEnableVignette(0.6f, 0.7f);
				RcDrawRectangle(defData->testPackRecs[rIndex], NewColor(Color_Red));
				RcDisableVignette();
			}
			RcDrawButton(RecInflate(defData->testPackBin, 2), NewColor(Color_TransparentBlack), NewColor(Color_Yellow), 2);
			EndTimeBlock();
		}
		
		// +==============================+
		// |   Draw Vignette Rectangle    |
		// +==============================+
		#if 1
		{
			RcEnableVignette(0.7f, 0.5f);
			RcDrawRectangle(NewRec(RenderScreenSize.width/2, 0, 200, 200), NewColor(Color_OrangeRed));
			RcDisableVignette();
		}
		#endif
		
		// +==============================+
		// |          Draw Glyph          |
		// +==============================+
		if (app->glyphFilled)
		{
			rec glyphRec = NewRec(RenderScreenSize.width/2 + 10, 10, (r32)app->glyphTexture.width, (r32)app->glyphTexture.height);
			RcBindTexture(&app->glyphTexture);
			RcDrawTexturedRec(glyphRec, NewColor(Color_White));
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
			r32 maxWidth = 180;
			#if 0
			maxWidth = RenderMousePos.x - textPos.x;
			if (maxWidth < 0) { maxWidth = 0; }
			#endif
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
		// |     Draw Test Triangles      |
		// +==============================+
		RcSetDepth(1.0f);
		if (defData->trianglesBuffer.filled && defData->polygonFinished)
		{
			RcNewPrintString(NewVec2(100, 100), NewColor(Color_White), "%u verts", defData->trianglesBuffer.numVertices);
			RcBindTexture(&renderContext->dotTexture);
			RcSetSourceRectangle(NewRec(0, 0, 1, 1));
			RcSetColor(NewColor(Color_White));
			RcSetWorldMatrix(Matrix4_Identity);
			RcBindBuffer(&defData->trianglesBuffer);
			RcDrawBufferTriangles();
		}
		else if (defData->testNumTriangles > 0 && defData->testTriangles != nullptr)
		{
			for (u32 tIndex = 0; tIndex < defData->testNumTriangles; tIndex++)
			{
				r32 offsetX = 0;//SinR32((r32)((platform->programTime + tIndex*291)%1000) / 1000.f * 2*Pi32) * 2;
				r32 offsetY = 0;//CosR32((r32)((platform->programTime + tIndex*291)%1000) / 1000.f * 2*Pi32) * 2;
				v2 offset = NewVec2(offsetX, offsetY);
				RcDrawLine(defData->testTriangles[tIndex].p0+offset, defData->testTriangles[tIndex].p1+offset, 1, NewColor(GetColorByIndex(tIndex)));
				RcDrawLine(defData->testTriangles[tIndex].p1+offset, defData->testTriangles[tIndex].p2+offset, 1, NewColor(GetColorByIndex(tIndex)));
				RcDrawLine(defData->testTriangles[tIndex].p2+offset, defData->testTriangles[tIndex].p0+offset, 1, NewColor(GetColorByIndex(tIndex)));
			}
		}
		
		// +==============================+
		// |         Draw Polygon         |
		// +==============================+
		RcSetDepth(0.0f);
		if (defData->testPolygon.numVerts > 0 && defData->testPolygon.verts != nullptr)
		{
			for (u32 vIndex = 0; vIndex < defData->testPolygon.numVerts; vIndex++)
			{
				v2 p0 = defData->testPolygon.verts[vIndex];
				v2 p1 = Vec2_Zero;
				if (vIndex+1 < defData->testPolygon.numVerts)
				{
					p1 = defData->testPolygon.verts[vIndex+1];
				}
				else if (!defData->polygonFinished)
				{
					p1 = RenderMousePos;
				}
				else
				{
					p1 = defData->testPolygon.verts[0];
				}
				
				Color_t vertColor = NewColor(Color_Yellow);
				r32 vertRadius = 3;
				if (vIndex == 0 && defData->testPolygon.numVerts >= 3 && Vec2Length(RenderMousePos - defData->testPolygon.verts[0]) < 10)
				{
					vertColor = NewColor(Color_OrangeRed);
					vertRadius = 10;
				}
				
				RcDrawCircle(p0, vertRadius, vertColor);
				RcDrawLine(p0, p1, 2, NewColor(Color_Red));
			}
		}
		
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
		#if 0
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



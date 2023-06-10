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
	defData->backgroundColorIndex = GetNamedColorIndex(ColorIvoryBlack);
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
			if (defData->backgroundColorIndex >= NUM_PREDEF_NAMED_COLORS) { defData->backgroundColorIndex = 0; }
		}
		if (ButtonPressed(Button_Left))
		{
			if (defData->backgroundColorIndex > 0) { defData->backgroundColorIndex--; }
			else { defData->backgroundColorIndex = NUM_PREDEF_NAMED_COLORS-1; }
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
			}
			
			u32 testCodepoints[] = {
				0x958b, //open
				0x9589, //close
				0x9591, //leisure
				0x9583, //flash
				0x95a4, //small side gate
			};
			app->glyphIndex = (app->glyphFilled) ? app->glyphIndex+1 : 0;
			if (app->glyphIndex < ArrayCount(testCodepoints))
			{
				app->glyphFilled = FontGetGlyph(&app->glyphTexture, &app->japaneseFont, 128, testCodepoints[app->glyphIndex], FontStyle_Default);
			}
			else
			{
				app->glyphFilled = false;
			}
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
		if (ButtonDown(Button_Control))
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
		if (!ButtonDown(Button_Control) && ButtonReleased(MouseButton_Left))// && input->mouseMaxDist[MouseButton_Left] < 5)
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
						triangleVerts[tIndex*3 + 0].color = NewVec4FromColor(GetNamedColorByIndex(tIndex));
						triangleVerts[tIndex*3 + 0].color.a = 1.0f;
						triangleVerts[tIndex*3 + 0].texCoord = Vec2_Zero;
						
						triangleVerts[tIndex*3 + 1].position = NewVec3(defData->testTriangles[tIndex].p1, 0);
						triangleVerts[tIndex*3 + 1].color = NewVec4FromColor(GetNamedColorByIndex(tIndex));
						triangleVerts[tIndex*3 + 1].color.a = 1.0f;
						triangleVerts[tIndex*3 + 1].texCoord = Vec2_Zero;
						
						triangleVerts[tIndex*3 + 2].position = NewVec3(defData->testTriangles[tIndex].p2, 0);
						triangleVerts[tIndex*3 + 2].color = NewVec4FromColor(GetNamedColorByIndex(tIndex));
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
		Color_t backgroundColor = GetNamedColorByIndex(defData->backgroundColorIndex);
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
				// RcDrawButton(defData->testPackRecs[rIndex], ColorTransparent(PureRed, 0.1f), PureRed, 2);
				RcEnableVignette(0.6f, 0.7f);
				RcDrawRectangle(defData->testPackRecs[rIndex], PureRed);
				RcDisableVignette();
			}
			RcDrawButton(RecInflate(defData->testPackBin, 2), TransparentBlack, PureYellow, 2);
			EndTimeBlock();
		}
		
		// +==============================+
		// |   Draw Vignette Rectangle    |
		// +==============================+
		#if 1
		{
			RcEnableVignette(0.7f, 0.5f);
			RcDrawRectangle(NewRec(RenderScreenSize.width/2, 0, 200, 200), ColorOrangeRed);
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
			RcDrawTexturedRec(glyphRec, White);
		}
		
		// +==============================+
		// |   New Font Rendering Tests   |
		// +==============================+
		// StartTimeBlock("New Font Test");
		{
			// RcBindTexture(&app->defaultFont.bitmap);
			// RcDrawTexturedRec(NewRec(500, 500, (r32)app->defaultFont.bitmap.width, (r32)app->defaultFont.bitmap.height), White);
			// RcBindTexture(&app->newFont.bakes[1].texture);
			// RcDrawTexturedRec(NewRec(500, 500, (r32)app->newFont.bakes[1].texture.width, (r32)app->newFont.bakes[1].texture.height), White);
			RcPrintString(NewVec2(500, 400), White, 1.0f, "NumBakes: %u", app->newFont.numBakes);
			u32 memUsage = FontGetMemoryUsage(&app->newFont);
			RcPrintString(NewVec2(500, 420), White, 1.0f, "Memory Usage: %s/%s (%.1f%%)", FormattedSizeStr(memUsage), FormattedSizeStr(mainHeap->size), (r32)memUsage / (r32)mainHeap->size);
			
			FontBake_t* fontBake = FontGetBakeFor(&app->newFont, 32, FontStyle_Default, nullptr);
			if (fontBake != nullptr)
			{
				RcBindTexture(&fontBake->texture);
				RcDrawTexturedRec(NewRec(100, 100, (r32)fontBake->texture.width, (r32)fontBake->texture.height), White);
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
			RcNewDrawNtString(printStr, textPos, White, maxWidth, &flowInfo);
			RcDrawRectangle(flowInfo.extents, ColorTransparent(White, 0.1f));
			// RcDrawRectangle(NewRec(flowInfo.position.x, flowInfo.position.y, flowInfo.extentRight, flowInfo.extentDown), ColorTransparent(White, 0.1f));
			// RcDrawLineArrow(flowInfo.endPos + NewVec2(10,20), flowInfo.endPos, 4, 1.0f, White);
			
			v2 arrowStartPos = textPos + NewVec2(0, -fontMaxExtendUp);
			v2 arrowEndPos = arrowStartPos + NewVec2(maxWidth, 0);
			// RcDrawLineArrow(arrowStartPos, arrowEndPos, 4, 1, White);
			RcDrawLine(arrowEndPos + NewVec2(0, -1000), arrowEndPos + NewVec2(0, 1000), 1, PureRed);
			
			u32 numCharsFit = FontMeasureLineWidth(printStr, (u32)strlen(printStr), &maxWidth, &app->newFont, 32, FontStyle_Default);
			// RcDrawRectangle(NewRec(flowInfo.position, NewVec2(maxWidth, flowInfo.extentDown)), ColorTransparent(White, 0.5f));
			
			// FontChar_t fontChar;
			// v2 printPos = NewVec2(500, 450);
			// for (u32 cIndex = 0; printStr[cIndex] != '\0'; cIndex++)
			// {
			// 	if (FontGetChar(&app->newFont, &fontChar, printStr[cIndex], 32))
			// 	{
			// 		RcBindTexture(fontChar.texture);
			// 		RcDrawTexturedRec(NewRec(printPos - fontChar.info->origin, fontChar.info->size), White, NewRec(fontChar.info->bakeRec));
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
			RcNewPrintString(NewVec2(100, 100), White, "%u verts", defData->trianglesBuffer.numVertices);
			RcBindTexture(&renderContext->dotTexture);
			RcSetSourceRectangle(NewRec(0, 0, 1, 1));
			RcSetColor(White);
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
				RcDrawLine(defData->testTriangles[tIndex].p0+offset, defData->testTriangles[tIndex].p1+offset, 1, GetNamedColorByIndex(tIndex));
				RcDrawLine(defData->testTriangles[tIndex].p1+offset, defData->testTriangles[tIndex].p2+offset, 1, GetNamedColorByIndex(tIndex));
				RcDrawLine(defData->testTriangles[tIndex].p2+offset, defData->testTriangles[tIndex].p0+offset, 1, GetNamedColorByIndex(tIndex));
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
				
				Color_t vertColor = PureYellow;
				r32 vertRadius = 3;
				if (vIndex == 0 && defData->testPolygon.numVerts >= 3 && Vec2Length(RenderMousePos - defData->testPolygon.verts[0]) < 10)
				{
					vertColor = ColorOrangeRed;
					vertRadius = 10;
				}
				
				RcDrawCircle(p0, vertRadius, vertColor);
				RcDrawLine(p0, p1, 2, PureRed);
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
				Color_t recColor = GetNamedColorByIndex(cIndex);
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
			RcDrawCircle(defData->quadStart, 3.0f, PureYellow);
			RcDrawCircle(defData->quadControl, 3.0f, PureYellow);
			RcDrawCircle(defData->quadEnd, 3.0f, PureYellow);
			RcDrawQuadCurve(defData->quadStart, defData->quadControl, defData->quadEnd, 60, 2, PureRed);
		}
		#endif
		
		// +==============================+
		// |       Draw Cubic Curve       |
		// +==============================+
		#if 1
		{
			RcDrawCircle(defData->cubicStart, 3.0f, PureYellow);
			RcDrawCircle(defData->cubicControl1, 3.0f, PureYellow);
			RcDrawCircle(defData->cubicControl2, 3.0f, PureYellow);
			RcDrawCircle(defData->cubicEnd, 3.0f, PureYellow);
			RcDrawCubicCurve(defData->cubicStart, defData->cubicControl1, defData->cubicControl2, defData->cubicEnd, 60, 2, PureRed);
		}
		#endif
		
		// +==================================+
		// | Draw Render Screen Measurements  |
		// +==================================+
		#if 1
		{
			RcDrawLineArrow(NewVec2(RenderScreenSize.width - 10, 0), NewVec2(RenderScreenSize.width - 10, RenderScreenSize.height), 10.0f, 2.0f, PureRed);
			RcDrawLineArrow(NewVec2(RenderScreenSize.width - 10, RenderScreenSize.height), NewVec2(RenderScreenSize.width - 10, 0), 10.0f, 2.0f, PureRed);
			char* heightStr = TempPrint("%.0f", RenderScreenSize.height);
			RcDrawString(heightStr, NewVec2(RenderScreenSize.width-10, RenderScreenSize.height/2), PureRed, 1.0f, Alignment_Right);
			
			u32 ratioWidth, ratioHeight;
			FindRatio((u32)RenderScreenSize.width, (u32)RenderScreenSize.height, &ratioWidth, &ratioHeight);
			char* ratioStr = TempPrint("%u:%u", ratioWidth, ratioHeight);
			RcDrawString(ratioStr, NewVec2(RenderScreenSize.width-20, RenderScreenSize.height-15-app->defaultFont.maxExtendDown), PureRed, 1.0f, Alignment_Right);
			
			RcDrawLineArrow(NewVec2(0, RenderScreenSize.height - 10), NewVec2(RenderScreenSize.width, RenderScreenSize.height - 10), 10.0f, 2.0f, PureRed);
			RcDrawLineArrow(NewVec2(RenderScreenSize.width, RenderScreenSize.height - 10), NewVec2(0, RenderScreenSize.height - 10), 10.0f, 2.0f, PureRed);
			char* widthStr = TempPrint("%.0f", RenderScreenSize.width);
			RcDrawString(widthStr, NewVec2(RenderScreenSize.width/2, RenderScreenSize.height - 10 - app->defaultFont.maxExtendDown), PureRed, 1.0f, Alignment_Center);
		}
		#endif
	}
}



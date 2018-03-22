/*
File:   app_visualizer.cpp
Author: Taylor Robbins
Date:   03\21\2018
Description: 
	** Contains an application state that visualizes files that are dropped onto the window 
*/

void VisLoadFonts()
{
	if (visData->initialized)
	{
		DestroyFont(&visData->smallFont);
		DestroyFont(&visData->mediumFont);
		DestroyFont(&visData->largeFont);
	}
	
	visData->smallFont  = LoadFont(FONTS_FOLDER "consolab.ttf", 12, 256, 256, ' ', 96);
	visData->mediumFont = LoadFont(FONTS_FOLDER "consolab.ttf", 18, 256, 256, ' ', 96);
	visData->largeFont  = LoadFont(FONTS_FOLDER "consolab.ttf", 48, 256, 256, ' ', 96);
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void InitializeVisualizerState()
{
	VisLoadFonts();
	
	
	
	visData->initialized = true;
}

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartVisualizerState(AppState_t fromState)
{
	//TODO: Resume?
}

// +--------------------------------------------------------------+
// |                         Deinitialize                         |
// +--------------------------------------------------------------+
void DeinitializeVisualizerState()
{
	//TODO: Deallocate things
}

// +--------------------------------------------------------------+
// |                      Update And Render                       |
// +--------------------------------------------------------------+
void UpdateAndRenderVisualizerState()
{
	// +--------------------------------------------------------------+
	// |                            Update                            |
	// +--------------------------------------------------------------+
	{
		// +==============================+
		// |     Reload Fonts Button      |
		// +==============================+
		if (ButtonPressed(Button_F5))
		{
			VisLoadFonts();
		}
		
		// +==============================+
		// |     Handle Dropped File      |
		// +==============================+
		if (input->numDroppedFiles > 0)
		{
			if (visData->fileOpen)
			{
				platform->FreeFileMemory(&visData->file);
				ClearStruct(visData->file);
				visData->fileOpen = false;
				//TODO: Free any other parsing data?
			}
			
			DEBUG_PrintLine("Dropped file \"%s\"", input->droppedFiles[0]);
			visData->file = platform->ReadEntireFile(input->droppedFiles[0]);
			if (visData->file.content != nullptr)
			{
				visData->fileOpen = true;
				visData->fileType = VisFileType_Unknown; //TODO: Find out which file type it is
				CopyStringIntoArray(visData->filePath, input->droppedFiles[0]);
				visData->viewMode = VisViewMode_RawHex;
				visData->hexData.numColumns = 32;
				visData->hexData.selectionStart = 0;
				visData->hexData.selectionEnd = 0;
				visData->hexData.viewPos = Vec2_Zero;
				visData->hexData.viewPosGoto = Vec2_Zero;
				DEBUG_WriteLine("Opened file successfully");
			}
			else
			{
				DEBUG_WriteLine("Couldn't open file");
			}
		}
		
		// +==============================+
		// |        Raw Hex Update        |
		// +==============================+
		if (visData->viewMode == VisViewMode_RawHex)
		{
			v2 addressStrSize = MeasureString(&visData->mediumFont, " 0x00000000 ");
			v2 tileSize = MeasureString(&visData->smallFont, "00") + Vec2_One;
			v2 spacing = NewVec2(4);
			
			// +==============================+
			// |      Calculate viewRec       |
			// +==============================+
			visData->hexData.viewRec = NewRec(Vec2_Zero, RenderScreenSize);
			visData->hexData.viewRec.width = spacing.x + visData->hexData.numColumns*(tileSize.x + spacing.x);
			if (visData->hexData.viewRec.width > RenderScreenSize.width - addressStrSize.width*2)
			{
				visData->hexData.viewRec.width = RenderScreenSize.width - addressStrSize.width*2;
			}
			visData->hexData.viewRec.x = RenderScreenSize.width/2 - visData->hexData.viewRec.width/2;
			
			u32 numTileRows = 0;
			v2 totalTileSize = Vec2_Zero;
			visData->hexData.isHovering = false;
			
			if (visData->fileOpen)
			{
				numTileRows = visData->file.size / visData->hexData.numColumns;
				if (visData->file.size % visData->hexData.numColumns) { numTileRows++; }
				totalTileSize = spacing + Vec2Multiply(tileSize + spacing, NewVec2((r32)visData->hexData.numColumns, (r32)numTileRows));
				
				// +==============================+
				// |         Update Tiles         |
				// +==============================+
				rec baseTileRec = NewRec(visData->hexData.viewRec.topLeft + spacing - visData->hexData.viewPos, tileSize);
				u32 startTileY = (u32)((visData->hexData.viewPos.y - spacing.height) / (baseTileRec.height + spacing.height));
				for (u32 tileY = startTileY; tileY < numTileRows; tileY++)
				{
					for (u32 tileX = 0; tileX < visData->hexData.numColumns; tileX++)
					{
						rec tileRec = baseTileRec;
						tileRec.x += (r32)tileX * (tileRec.width + spacing.width);
						tileRec.y += (r32)tileY * (tileRec.height + spacing.height);
						rec tileMouseRec = tileRec;
						tileMouseRec.size += spacing;
						
						u32 bIndex = (tileY*visData->hexData.numColumns) + tileX;
						
						if (input->mouseInsideWindow && IsInsideRec(tileMouseRec, RenderMousePos) && IsInsideRec(visData->hexData.viewRec, RenderMousePos))
						{
							visData->hexData.isHovering = true;
							visData->hexData.hoverIndex = bIndex;
							
							if (IsInsideRec(visData->hexData.viewRec, RenderMouseStartLeft))
							{
								if (ButtonPressed(MouseButton_Left))
								{
									visData->hexData.selectionStart = bIndex;
								}
								if (ButtonDown(MouseButton_Left))
								{
									if (bIndex >= visData->hexData.selectionStart)
									{
										visData->hexData.selectionEnd = bIndex+1;
									}
									else
									{
										visData->hexData.selectionEnd = bIndex;
									}
								}
							}
						}
					}
					
					if ((baseTileRec.y + (r32)tileY * (baseTileRec.height + spacing.height)) > visData->hexData.viewRec.height)
					{
						break;
					}
				}
			}
			
			// +==============================+
			// |     Handle Scroll Wheels     |
			// +==============================+
			if (input->scrollDelta.y != 0)
			{
				if (ButtonDown(Button_Shift))
				{
					visData->hexData.viewPosGoto.x += -input->scrollDelta.y * 45;
				}
				else
				{
					visData->hexData.viewPosGoto.y += -input->scrollDelta.y * 45;
				}
			}
			if (input->scrollDelta.x != 0)
			{
				visData->hexData.viewPosGoto.x += -input->scrollDelta.x * 45;
			}
			
			// +==============================+
			// |         Update View          |
			// +==============================+
			if (visData->hexData.viewPosGoto.x > totalTileSize.width - visData->hexData.viewRec.width) { visData->hexData.viewPosGoto.x = totalTileSize.width - visData->hexData.viewRec.width; }
			if (visData->hexData.viewPosGoto.y > totalTileSize.height - visData->hexData.viewRec.height) { visData->hexData.viewPosGoto.y = totalTileSize.height - visData->hexData.viewRec.height; }
			if (visData->hexData.viewPosGoto.x < 0) { visData->hexData.viewPosGoto.x = 0; }
			if (visData->hexData.viewPosGoto.y < 0) { visData->hexData.viewPosGoto.y = 0; }
			if (visData->hexData.viewPos != visData->hexData.viewPosGoto)
			{
				v2 viewDiffVec = visData->hexData.viewPosGoto - visData->hexData.viewPos;
				if (Vec2Length(viewDiffVec) < 0.01f)
				{
					visData->hexData.viewPos = visData->hexData.viewPosGoto;
				}
				else
				{
					visData->hexData.viewPos += viewDiffVec / 8.0f;
				}
			}
		}
		else if (visData->fileType == VisFileType_IntelHex)
		{
			
		}
		else if (visData->fileType == VisFileType_Elf)
		{
			
		}
		else if (visData->fileType == VisFileType_Png)
		{
			
		}
		else if (visData->fileType == VisFileType_Jpeg)
		{
			
		}
		else if (visData->fileType == VisFileType_Bmp)
		{
			
		}
		else if (visData->fileType == VisFileType_O)
		{
			
		}
		else if (visData->fileType == VisFileType_Exe)
		{
			
		}
	}
	
	// +--------------------------------------------------------------+
	// |                            Render                            |
	// +--------------------------------------------------------------+
	{
		// +==============================+
		// |            Setup             |
		// +==============================+
		{
			RcBegin(&app->defaultShader, &app->defaultFont, NewRec(Vec2_Zero, RenderScreenSize));
			RcClearColorBuffer(NewColor(Color_CadetBlue));
			RcClearDepthBuffer(1.0f);
		}
		
		if (visData->viewMode == VisViewMode_RawHex)
		{
			v2 tileSize = MeasureString(&visData->smallFont, "00") + Vec2_One;
			v2 spacing = NewVec2(4);
			
			RcDrawRectangle(visData->hexData.viewRec, NewColor(Color_White));
			
			u32 numTileRows = 0;
			v2 totalTileSize = Vec2_Zero;
			if (visData->fileOpen)
			{
				numTileRows = visData->file.size / visData->hexData.numColumns;
				if (visData->file.size % visData->hexData.numColumns) { numTileRows++; }
				totalTileSize = spacing + Vec2Multiply(tileSize + spacing, NewVec2((r32)visData->hexData.numColumns, (r32)numTileRows));
				
				// +==============================+
				// |          Draw Tiles          |
				// +==============================+
				RcSetViewport(visData->hexData.viewRec);
				rec baseTileRec = NewRec(visData->hexData.viewRec.topLeft + spacing - visData->hexData.viewPos, tileSize);
				u32 startTileY = (u32)((visData->hexData.viewPos.y - spacing.height) / (baseTileRec.height + spacing.height));
				for (u32 tileY = startTileY; tileY < numTileRows; tileY++)
				{
					for (u32 tileX = 0; tileX < visData->hexData.numColumns; tileX++)
					{
						rec tileRec = baseTileRec;
						tileRec.x += (r32)tileX * (tileRec.width + spacing.width);
						tileRec.y += (r32)tileY * (tileRec.height + spacing.height);
						tileRec = RecInflate(tileRec, spacing/2);
						
						u32 bIndex = (tileY*visData->hexData.numColumns) + tileX;
						
						if (bIndex < visData->file.size && RecIntersects(tileRec, visData->hexData.viewRec))
						{
							TempPushMark();
							char* hexStr = TempPrint("%02X", ((u8*)visData->file.content)[bIndex]);
							v2 hexStrSize = MeasureString(&visData->smallFont, hexStr);
							v2 hexStrPos = tileRec.topLeft + tileRec.size/2 - hexStrSize/2;
							hexStrPos.y += visData->smallFont.maxExtendUp;
							
							Color_t backColor = NewColor(Color_White);
							Color_t textColor = NewColor(Color_Black);
							Color_t borderColor = NewColor(Color_TransparentBlack);
							
							bool isSelected = false;
							if (bIndex >= visData->hexData.selectionStart && bIndex < visData->hexData.selectionEnd) { isSelected = true; }
							if (bIndex <= visData->hexData.selectionStart && bIndex >= visData->hexData.selectionEnd) { isSelected = true; }
							if (visData->hexData.isHovering && bIndex == visData->hexData.hoverIndex)
							{
								if (isSelected)
								{
									backColor = NewColor(Color_SlateBlue);
								}
								else
								{
									backColor = NewColor(Color_LightGrey);
								}
							}
							else if (isSelected)
							{
								backColor = NewColor(Color_LightBlue);
							}
							
							RcDrawButton(tileRec, backColor, borderColor, 1);
							RcBindFont(&visData->smallFont);
							RcDrawString(hexStr, hexStrPos, textColor);
							TempPopMark();
						}
					}
					
					if ((baseTileRec.y + (r32)tileY * (baseTileRec.height + spacing.height)) > visData->hexData.viewRec.height)
					{
						break;
					}
				}
				RcSetViewport(NewRec(Vec2_Zero, RenderScreenSize));
				if (visData->hexData.viewPosGoto.x > tileSize.width/2)
				{
					rec gradientRec = visData->hexData.viewRec;
					gradientRec.width = 6;
					RcDrawGradient(gradientRec, NewColor(Color_Black), NewColor(Color_TransparentBlack), Dir2_Right);
				}
				if (visData->hexData.viewPosGoto.x + visData->hexData.viewRec.width < totalTileSize.width - tileSize.width/2)
				{
					rec gradientRec = visData->hexData.viewRec;
					gradientRec.x += gradientRec.width;
					gradientRec.width = 6;
					gradientRec.x -= gradientRec.width;
					RcDrawGradient(gradientRec, NewColor(Color_Black), NewColor(Color_TransparentBlack), Dir2_Left);
				}
				if (visData->hexData.viewPosGoto.y > tileSize.width/2)
				{
					rec gradientRec = visData->hexData.viewRec;
					gradientRec.height = 6;
					RcDrawGradient(gradientRec, NewColor(Color_Black), NewColor(Color_TransparentBlack), Dir2_Down);
				}
				if (visData->hexData.viewPosGoto.y + visData->hexData.viewRec.height < totalTileSize.height - tileSize.height/2)
				{
					rec gradientRec = visData->hexData.viewRec;
					gradientRec.y += gradientRec.height;
					gradientRec.height = 6;
					gradientRec.y -= gradientRec.height;
					RcDrawGradient(gradientRec, NewColor(Color_Black), NewColor(Color_TransparentBlack), Dir2_Up);
				}
				
				// +==============================+
				// |      Draw Row Addresses      |
				// +==============================+
				for (u32 tileY = 0; tileY < numTileRows; tileY++)
				{
					r32 textTop = baseTileRec.y;
					textTop += (r32)tileY * (baseTileRec.height + spacing.height) + baseTileRec.height/2 - visData->mediumFont.lineHeight/2;
					if (textTop < RenderScreenSize.height && textTop + visData->mediumFont.lineHeight > 0)
					{
						u32 bIndex = tileY * visData->hexData.numColumns;
						TempPushMark();
						char* addressStr = TempPrint("0x%08X", bIndex);
						v2 addressStrPos = NewVec2(visData->hexData.viewRec.x - 2, textTop + visData->mediumFont.maxExtendUp);
						addressStrPos.x = (r32)RoundR32(addressStrPos.x);
						addressStrPos.y = (r32)RoundR32(addressStrPos.y);
						RcBindFont(&visData->mediumFont);
						RcDrawString(addressStr, addressStrPos, NewColor(Color_DarkSlateBlue), 1.0f, Alignment_Right);
						TempPopMark();
					}
				}
			}
		}
		else if (visData->fileType == VisFileType_IntelHex)
		{
			
		}
		else if (visData->fileType == VisFileType_Elf)
		{
			
		}
		else if (visData->fileType == VisFileType_Png)
		{
			
		}
		else if (visData->fileType == VisFileType_Jpeg)
		{
			
		}
		else if (visData->fileType == VisFileType_Bmp)
		{
			
		}
		else if (visData->fileType == VisFileType_O)
		{
			
		}
		else if (visData->fileType == VisFileType_Exe)
		{
			
		}
	}
}

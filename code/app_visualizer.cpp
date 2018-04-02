/*
File:   app_visualizer.cpp
Author: Taylor Robbins
Date:   03\21\2018
Description: 
	** Contains an application state that visualizes files that are dropped onto the window 
*/

#include "app_parseIntelHex.cpp"

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
	visData->largeFont  = LoadFont(FONTS_FOLDER "consolab.ttf", 48, 512, 512, ' ', 96);
}

void VisDetermineFileType()
{
	//TODO: Determine the file type
	visData->fileType = VisFileType_Unknown;
	visData->viewMode = VisViewMode_RawHex;
	
	const char* fileExtension = GetFileExtPart(visData->filePath);
	if (fileExtension != nullptr)
	{
		DEBUG_PrintLine("File extension: \"%s\"", fileExtension);
		if (strcmp(fileExtension, "hex") == 0)
		{
			visData->fileType = VisFileType_IntelHex;
			visData->viewMode = VisViewMode_RawHex;
		}
	}
}

void VisParseFile()
{
	VisHexData_t* hexData = &visData->hexData;
	if (hexData->allocArena != nullptr && hexData->regions != nullptr)
	{
		for (u32 rIndex = 0; rIndex < hexData->numRegions; rIndex++)
		{
			VisHexDataRegion_t* region = &hexData->regions[rIndex];
			if (region->allocArena != nullptr && region->data != nullptr)
			{
				ArenaPop(region->allocArena, region->data);
			}
		}
		ArenaPop(hexData->allocArena, hexData->regions);
	}
	ClearPointer(hexData);
	
	bool parsedSuccessfully = false;
	if (visData->fileType == VisFileType_IntelHex)
	{
		hexData->regions = VisParseIntelHex(mainHeap, (char*)visData->file.content, visData->file.size, &hexData->numRegions);
		if (hexData->regions != nullptr)
		{
			parsedSuccessfully = true;
		}
		else
		{
			DEBUG_WriteLine("Could not parse Intel HEX file correctly");
			visData->fileType = VisFileType_Unknown;
		}
	}
	
	if (!parsedSuccessfully)
	{
		hexData->numRegions = 1;
		hexData->regions = PushArray(mainHeap, VisHexDataRegion_t, hexData->numRegions);
		hexData->allocArena = mainHeap;
		hexData->regions[0].allocArena = nullptr;
		hexData->regions[0].address = 0x00000000;
		hexData->regions[0].size = visData->file.size;
		hexData->regions[0].data = (u8*)visData->file.content;
	}
	
	hexData->addressMin = 0x00000000;
	hexData->addressMax = 0x00000000;
	hexData->regionsSize = 0;
	for (u32 rIndex = 0; rIndex < hexData->numRegions; rIndex++)
	{
		VisHexDataRegion_t* region = &hexData->regions[rIndex];
		if (rIndex == 0 || region->address < hexData->addressMin)
		{
			hexData->addressMin = region->address;
		}
		if (region->address + region->size > hexData->addressMax)
		{
			hexData->addressMax = region->address + region->size;
		}
		hexData->regionsSize += region->size;
	}
	
	hexData->numColumns = 32;
	if (hexData->addressMin%hexData->numColumns != 0)
	{
		hexData->addressMin = (hexData->addressMin/hexData->numColumns) * hexData->numColumns;
	}
	
	hexData->numRows = ((hexData->addressMax - hexData->addressMin) / hexData->numColumns);
	if (visData->file.size % hexData->numColumns) { hexData->numRows++; }
	hexData->tileSize = MeasureString(&visData->smallFont, "00") + Vec2_One;
	hexData->spacing = NewVec2(4);
	hexData->dataSize = hexData->spacing + Vec2Multiply(hexData->tileSize + hexData->spacing, NewVec2((r32)hexData->numColumns, (r32)hexData->numRows));
	
	hexData->interpTileSize = MeasureString(&visData->smallFont, ".") + Vec2_One;
	hexData->interpTileSize.height = hexData->tileSize.height;
	hexData->interpSpacing = hexData->spacing;
	hexData->interpSpacing.width = 0;
	hexData->interpDataSize = hexData->interpSpacing + Vec2Multiply(hexData->interpTileSize + hexData->interpSpacing, NewVec2((r32)hexData->numColumns, (r32)hexData->numRows));
	
	hexData->viewPos = Vec2_Zero;
	hexData->viewPosGoto = hexData->viewPos;
	
	hexData->selInfoViewPos = Vec2_Zero;
	hexData->selInfoViewPosGoto = hexData->selInfoViewPos;
	hexData->selInfoGrabPos = Vec2_Zero;
	
	hexData->fileInfoViewPos = Vec2_Zero;
	hexData->fileInfoViewPosGoto = hexData->fileInfoViewPos;
	hexData->fileInfoGrabPos = Vec2_Zero;
}

void VisGenerateUi()
{
	if (visData->viewMode == VisViewMode_RawHex)
	{
		VisHexData_t* hexData = &visData->hexData;
		
		v2 addressSize = MeasureString(&visData->smallFont, " 00000000 ");
		r32 minSideWidth = 200;
		r32 maxViewWidth = RenderScreenSize.width - (minSideWidth*2);
		r32 viewToInterpRatio = hexData->interpDataSize.width / hexData->dataSize.width;
		r32 totalWidth = hexData->dataSize.width + hexData->interpDataSize.width;
		if (totalWidth > maxViewWidth) { totalWidth = maxViewWidth; }
		r32 viewWidth = totalWidth / (1 + viewToInterpRatio);
		r32 interpWidth = totalWidth - viewWidth;
		
		hexData->viewRec = NewRec(0, 0, viewWidth, RenderScreenSize.height);
		hexData->viewRec.x = RenderScreenSize.width/2 - totalWidth/2;
		hexData->viewRec.x = (r32)RoundR32(hexData->viewRec.x);
		hexData->viewRec.y = (r32)RoundR32(hexData->viewRec.y);
		
		hexData->interpRec = hexData->viewRec;
		hexData->interpRec.x += hexData->interpRec.width;
		hexData->interpRec.width = interpWidth;
		
		hexData->scrollGutterRec = hexData->interpRec;
		hexData->scrollGutterRec.x += hexData->interpRec.width;
		hexData->scrollGutterRec.width = 22;
		
		hexData->scrollBarRec = hexData->scrollGutterRec;
		hexData->scrollBarRec.height *= (hexData->interpRec.height / hexData->dataSize.height);
		if (hexData->scrollBarRec.height < 15) { hexData->scrollBarRec.height = 15; }
		hexData->scrollBarRec.y = (hexData->scrollGutterRec.height - hexData->scrollBarRec.height) * (hexData->viewPos.y / (hexData->dataSize.height - hexData->interpRec.height));
		
		hexData->addressesRec = hexData->viewRec;
		hexData->addressesRec.width = addressSize.width;
		hexData->addressesRec.x -= hexData->addressesRec.width;
		
		hexData->fileInfoRec = NewRec(0, RenderScreenSize.height, hexData->addressesRec.x, RenderScreenSize.height/3);
		hexData->fileInfoRec.y -= hexData->fileInfoRec.height;
		
		hexData->selectionInfoRec = hexData->fileInfoRec;
		hexData->selectionInfoRec.y -= hexData->selectionInfoRec.height + visData->smallFont.lineHeight;
		
		hexData->structsTabRec = NewRec(hexData->scrollGutterRec.x + hexData->scrollGutterRec.width, 0, 0, visData->smallFont.lineHeight + 5*2);
		hexData->structsTabRec.width = RenderScreenSize.width - hexData->structsTabRec.x;
		hexData->structsTabRec = RecDeflate(hexData->structsTabRec, 3);
		hexData->structsTabRec.width -= hexData->structsTabRec.height;
		
		hexData->structsPlusRec = hexData->structsTabRec;
		hexData->structsPlusRec.x += hexData->structsTabRec.width;
		hexData->structsPlusRec.width = hexData->structsPlusRec.height;
		
		hexData->structViewRec = NewRec(
			hexData->structsTabRec.x,
			hexData->structsTabRec.y + hexData->structsTabRec.height + 3,
			hexData->structsPlusRec.x + hexData->structsPlusRec.width - hexData->structsTabRec.x,
			0
		);
		hexData->structViewRec.height = RenderScreenSize.height - hexData->structViewRec.y;
		hexData->structViewRec.height -= (3*2 + RenderScreenSize.height/3);
		
		hexData->structTypesRec = hexData->structViewRec;
		hexData->structTypesRec.y += hexData->structViewRec.height + 3;
		hexData->structTypesRec.height = RenderScreenSize.height - hexData->structTypesRec.y - 3;
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
				DEBUG_WriteLine("Opened file successfully");
				CopyStringIntoArray(visData->filePath, input->droppedFiles[0]);
				visData->fileOpen = true;
				
				VisDetermineFileType();
				VisParseFile();
				VisGenerateUi();
				visData->oldRenderScreenSize = RenderScreenSize;
			}
			else
			{
				DEBUG_WriteLine("Couldn't open file");
			}
		}
		
		// +==============================+
		// |        Regenerate UI         |
		// +==============================+
		bool screenSizeChanged = false;
		if (visData->oldRenderScreenSize != RenderScreenSize)
		{
			DEBUG_WriteLine("RenderScreenSize changed");
			screenSizeChanged = true;
			visData->oldRenderScreenSize = RenderScreenSize;
		}
		if (screenSizeChanged || ButtonPressed(Button_F7))
		{
			if (visData->fileOpen)
			{
				VisGenerateUi();
			}
		}
		
		// +==============================+
		// |        Raw Hex Update        |
		// +==============================+
		if (visData->viewMode == VisViewMode_RawHex)
		{
			VisHexData_t* hexData = &visData->hexData;
			visData->hexData.isHovering = false;
			
			bool mouseInsideDataArea = (IsInsideRec(hexData->viewRec, RenderMousePos) || IsInsideRec(hexData->interpRec, RenderMousePos) || IsInsideRec(hexData->scrollGutterRec, RenderMousePos));
			
			// +==============================+
			// |     Handle Scroll Wheels     |
			// +==============================+
			if (input->mouseInsideWindow && mouseInsideDataArea && !hexData->draggingScrollbar)
			{
				if (input->scrollDelta.y != 0)
				{
					if (ButtonDown(Button_Shift))
					{
						hexData->viewPosGoto.x += -input->scrollDelta.y * 45;
					}
					else
					{
						hexData->viewPosGoto.y += -input->scrollDelta.y * 45;
					}
				}
				if (input->scrollDelta.x != 0)
				{
					hexData->viewPosGoto.x += -input->scrollDelta.x * 45;
				}
			}
			
			// +==============================+
			// |   PageUp/Down and Home/End   |
			// +==============================+
			if (ButtonPressed(Button_PageUp))
			{
				hexData->viewPosGoto.y -= hexData->viewRec.height;
			}
			if (ButtonPressed(Button_PageDown))
			{
				hexData->viewPosGoto.y += hexData->viewRec.height;
			}
			if (ButtonPressed(Button_Home))
			{
				hexData->viewPosGoto.y = 0;
			}
			if (ButtonPressed(Button_End))
			{
				hexData->viewPosGoto.y = hexData->dataSize.height - hexData->viewRec.height;
			}
			
			// +==============================+
			// |    Scrollbar Interaction     |
			// +==============================+
			hexData->scrollBarRec.y = (hexData->scrollGutterRec.height - hexData->scrollBarRec.height) * (hexData->viewPos.y / (hexData->dataSize.height - hexData->viewRec.height));
			{
				if (hexData->draggingScrollbar)
				{
					if (ButtonDown(MouseButton_Left))
					{
						r32 newScrollbarPos = RenderMousePos.y - hexData->dragScrollbarOffset.y;
						if (newScrollbarPos > hexData->scrollGutterRec.y+hexData->scrollGutterRec.height - hexData->scrollBarRec.height) { newScrollbarPos = hexData->scrollGutterRec.y+hexData->scrollGutterRec.height - hexData->scrollBarRec.height; }
						if (newScrollbarPos < hexData->scrollGutterRec.y) { newScrollbarPos = hexData->scrollGutterRec.y; }
						hexData->viewPos.y = ((hexData->dataSize.height - hexData->viewRec.height) * newScrollbarPos) / (hexData->scrollGutterRec.height - hexData->scrollBarRec.height);
						hexData->viewPosGoto.y = hexData->viewPos.y;
					}
					else { hexData->draggingScrollbar = false; }
				}
				else if (IsInsideRec(hexData->scrollGutterRec, RenderMousePos))
				{
					if (IsInsideRec(hexData->scrollBarRec, RenderMousePos))
					{
						if (ButtonPressed(MouseButton_Left))
						{
							hexData->draggingScrollbar = true;
							hexData->dragScrollbarOffset = RenderMousePos - hexData->scrollBarRec.topLeft;
						}
					}
					else
					{
						if (ButtonPressed(MouseButton_Left))
						{
							r32 newScrollbarPos = RenderMousePos.y;
							if (newScrollbarPos < hexData->scrollGutterRec.y) { newScrollbarPos = hexData->scrollGutterRec.y; }
							if (newScrollbarPos > hexData->scrollGutterRec.y+hexData->scrollGutterRec.height - hexData->scrollBarRec.height) { newScrollbarPos = hexData->scrollGutterRec.y+hexData->scrollGutterRec.height - hexData->scrollBarRec.height; }
							hexData->viewPosGoto.y = ((hexData->dataSize.height - hexData->viewRec.height) * newScrollbarPos) / (hexData->scrollGutterRec.height - hexData->scrollBarRec.height);
						}
					}
				}
			}
			hexData->scrollBarRec.y = (hexData->scrollGutterRec.height - hexData->scrollBarRec.height) * (hexData->viewPos.y / (hexData->dataSize.height - hexData->viewRec.height));
			
			// +==============================+
			// |         Update View          |
			// +==============================+
			if (hexData->viewPosGoto.x > hexData->dataSize.width - hexData->viewRec.width) { hexData->viewPosGoto.x = hexData->dataSize.width - hexData->viewRec.width; }
			if (hexData->viewPosGoto.y > hexData->dataSize.height - hexData->viewRec.height) { hexData->viewPosGoto.y = hexData->dataSize.height - hexData->viewRec.height; }
			if (hexData->viewPosGoto.x < 0) { hexData->viewPosGoto.x = 0; }
			if (hexData->viewPosGoto.y < 0) { hexData->viewPosGoto.y = 0; }
			if (hexData->viewPos != hexData->viewPosGoto)
			{
				v2 viewDiffVec = hexData->viewPosGoto - hexData->viewPos;
				if (Vec2Length(viewDiffVec) < 0.01f)
				{
					hexData->viewPos = hexData->viewPosGoto;
				}
				else
				{
					hexData->viewPos += viewDiffVec / 3.0f;
				}
			}
			hexData->scrollBarRec.y = (hexData->scrollGutterRec.height - hexData->scrollBarRec.height) * (hexData->viewPos.y / (hexData->dataSize.height - hexData->viewRec.height));
			
			if (hexData->viewPos.x == 0) { hexData->interpViewPos.x = 0; }
			else 
			{
				hexData->interpViewPos.x = (hexData->interpDataSize.width - hexData->interpRec.width) * (hexData->viewPos.width / (hexData->dataSize.width - hexData->viewRec.width));
			}
			if (hexData->viewPos.y == 0) { hexData->interpViewPos.y = 0; }
			else 
			{
				hexData->interpViewPos.y = (hexData->interpDataSize.height - hexData->interpRec.height) * (hexData->viewPos.height / (hexData->dataSize.height - hexData->viewRec.height));
			}
			
			// +==============================+
			// |         Update Tiles         |
			// +==============================+
			if (visData->fileOpen)
			{
				rec baseTileRec = NewRec(hexData->viewRec.topLeft + hexData->spacing - hexData->viewPos, hexData->tileSize);
				u32 startTileY = (u32)((hexData->viewPos.y - hexData->spacing.height) / (baseTileRec.height + hexData->spacing.height));
				for (u32 tileY = startTileY; tileY < hexData->numRows; tileY++)
				{
					for (u32 tileX = 0; tileX < hexData->numColumns; tileX++)
					{
						rec tileRec = baseTileRec;
						tileRec.x += (r32)tileX * (tileRec.width + hexData->spacing.width);
						tileRec.y += (r32)tileY * (tileRec.height + hexData->spacing.height);
						rec tileHoverRec = tileRec;
						tileHoverRec.size += hexData->spacing;
						
						u32 bIndex = (tileY*hexData->numColumns) + tileX;
						
						if (input->mouseInsideWindow && IsInsideRec(tileHoverRec, RenderMousePos) && IsInsideRec(hexData->viewRec, RenderMousePos))
						{
							hexData->isHovering = true;
							hexData->hoverIndex = bIndex;
							
							if (IsInsideRec(hexData->viewRec, RenderMouseStartLeft))
							{
								if (ButtonPressed(MouseButton_Left) && !ButtonDown(Button_Shift))
								{
									hexData->selectionStart = bIndex;
								}
								if (ButtonDown(MouseButton_Left))
								{
									if (bIndex >= hexData->selectionStart)
									{
										hexData->selectionEnd = bIndex+1;
									}
									else
									{
										hexData->selectionEnd = bIndex;
									}
								}
							}
						}
					}
					
					if ((baseTileRec.y + (r32)tileY * (baseTileRec.height + hexData->spacing.height)) > hexData->viewRec.height)
					{
						break;
					}
				}
			}
			
			// +==============================+
			// |     Update Interp Tiles      |
			// +==============================+
			if (visData->fileOpen)
			{
				rec baseTileRec = NewRec(visData->hexData.interpRec.topLeft + hexData->interpSpacing - hexData->interpViewPos, hexData->interpTileSize);
				u32 startTileY = (u32)((hexData->interpViewPos.y - hexData->interpSpacing.height) / (baseTileRec.height + hexData->interpSpacing.height));
				for (u32 tileY = startTileY; tileY < hexData->numRows; tileY++)
				{
					for (u32 tileX = 0; tileX < hexData->numColumns; tileX++)
					{
						rec tileRec = baseTileRec;
						tileRec.x += (r32)tileX * (tileRec.width + hexData->interpSpacing.width);
						tileRec.y += (r32)tileY * (tileRec.height + hexData->interpSpacing.height);
						rec tileHoverRec = tileRec;
						tileHoverRec.size += hexData->interpSpacing;
						
						u32 bIndex = (tileY*hexData->numColumns) + tileX;
						
						if (input->mouseInsideWindow && IsInsideRec(tileHoverRec, RenderMousePos) && IsInsideRec(hexData->interpRec, RenderMousePos))
						{
							hexData->isHovering = true;
							hexData->hoverIndex = bIndex;
							
							if (IsInsideRec(hexData->interpRec, RenderMouseStartLeft))
							{
								if (ButtonPressed(MouseButton_Left) && !ButtonDown(Button_Shift))
								{
									hexData->selectionStart = bIndex;
								}
								if (ButtonDown(MouseButton_Left))
								{
									if (bIndex >= hexData->selectionStart)
									{
										hexData->selectionEnd = bIndex+1;
									}
									else
									{
										hexData->selectionEnd = bIndex;
									}
								}
							}
						}
					}
					
					if ((baseTileRec.y + (r32)tileY * (baseTileRec.height + hexData->spacing.height)) > hexData->interpRec.height)
					{
						break;
					}
				}
			}
			
			// +==============================+
			// |    Update File Info Menu     |
			// +==============================+
			{
				
			}
			
			// +==============================+
			// |  Update Selection Info Menu  |
			// +==============================+
			{
				
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
			RcClearColorBuffer(VisBlueGray);
			RcClearDepthBuffer(1.0f);
		}
		
		if (visData->fileOpen)
		{
			// +--------------------------------------------------------------+
			// |                        Render Raw Hex                        |
			// +--------------------------------------------------------------+
			if (visData->viewMode == VisViewMode_RawHex)
			{
				VisHexData_t* hexData = &visData->hexData;
				
				RcDrawRectangle(visData->hexData.viewRec, VisSilver);
				
				RcDrawButton(hexData->structsTabRec, VisLightBlueGray, VisLightGray, 1);
				RcDrawButton(hexData->structsPlusRec, VisGreen, VisLightGreen, 1);
				RcDrawButton(hexData->structViewRec, VisLightBlueGray, VisLightGray, 1);
				RcDrawButton(hexData->structTypesRec, VisLightBlueGray, VisLightGray, 1);
				
				u32 selectionMin = min(hexData->selectionStart, hexData->selectionEnd);
				u32 selectionMax = max(hexData->selectionStart, hexData->selectionEnd);
				u32 selectionLength = selectionMax - selectionMin;
				if (hexData->selectionStart > hexData->selectionEnd) { selectionLength += 1; }
				const u8* selectionDataPntr = nullptr;
				for (u32 rIndex = 0; rIndex < hexData->numRegions; rIndex++)
				{
					VisHexDataRegion_t* regionPntr = &hexData->regions[rIndex];
					if (regionPntr->address <= hexData->addressMin + selectionMin && regionPntr->address + regionPntr->size >= hexData->addressMin + selectionMax)
					{
						selectionDataPntr = &regionPntr->data[hexData->addressMin + selectionMin - regionPntr->address];
						break;
					}
				}
				
				bool outlineByteFilled = false;
				u8 outlineByte = 0x00;
				if (selectionLength == 1 && selectionDataPntr != nullptr)
				{
					outlineByte = *selectionDataPntr;
					outlineByteFilled = true;
				}
				
				// +==============================+
				// |          Draw Tiles          |
				// +==============================+
				RcSetViewport(visData->hexData.viewRec);
				rec baseTileRec = NewRec(visData->hexData.viewRec.topLeft + hexData->spacing - visData->hexData.viewPos, hexData->tileSize);
				u32 startTileY = (u32)((visData->hexData.viewPos.y - hexData->spacing.height) / (baseTileRec.height + hexData->spacing.height));
				for (u32 tileY = startTileY; tileY < hexData->numRows; tileY++)
				{
					for (u32 tileX = 0; tileX < visData->hexData.numColumns; tileX++)
					{
						rec tileRec = baseTileRec;
						tileRec.x += (r32)tileX * (tileRec.width + hexData->spacing.width);
						tileRec.y += (r32)tileY * (tileRec.height + hexData->spacing.height);
						tileRec = RecInflate(tileRec, hexData->spacing/2);
						
						u32 bIndex = (tileY*visData->hexData.numColumns) + tileX;
						u32 byteAddress = hexData->addressMin + bIndex;
						
						if (bIndex < visData->file.size && RecIntersects(tileRec, visData->hexData.viewRec))
						{
							TempPushMark();
							bool byteFilled = false;
							u8 byteValue = 0x00;
							for (u32 rIndex = 0; rIndex < hexData->numRegions; rIndex++)
							{
								VisHexDataRegion_t* region = &hexData->regions[rIndex];
								if (byteAddress >= region->address && byteAddress < region->address + region->size)
								{
									byteFilled = true;
									byteValue = region->data[byteAddress - region->address];
									break;
								}
							}
							char* hexStr = nullptr;
							if (byteFilled) { hexStr = TempPrint("%02X", byteValue); }
							else { hexStr = "??"; }
							v2 hexStrSize = MeasureString(&visData->smallFont, hexStr);
							v2 hexStrPos = tileRec.topLeft + tileRec.size/2 - hexStrSize/2;
							hexStrPos.y += visData->smallFont.maxExtendUp;
							hexStrPos.x = (r32)RoundR32(hexStrPos.x);
							hexStrPos.y = (r32)RoundR32(hexStrPos.y);
							
							Color_t backColor = VisWhite;
							Color_t textColor = VisBlueGray;
							Color_t borderColor = NewColor(Color_TransparentBlack);
							
							bool isSelected = false;
							if (bIndex >= visData->hexData.selectionStart && bIndex < visData->hexData.selectionEnd) { isSelected = true; }
							if (bIndex <= visData->hexData.selectionStart && bIndex >= visData->hexData.selectionEnd) { isSelected = true; }
							
							if (byteFilled)
							{
								if (outlineByteFilled && byteValue == outlineByte) { backColor = VisSilver; }
								if (visData->hexData.isHovering && bIndex == visData->hexData.hoverIndex)
								{
									if (isSelected)
									{
										backColor = VisPurple;
										textColor = VisWhite;
									}
									else
									{
										backColor = VisSilver;
									}
								}
								else if (isSelected)
								{
									backColor = VisLightPurple;
									textColor = VisWhite;
								}
							}
							else
							{
								backColor = VisGray;
							}
							
							RcDrawButton(tileRec, backColor, borderColor, 1);
							RcBindFont(&visData->smallFont);
							RcDrawString(hexStr, hexStrPos, textColor);
							TempPopMark();
						}
					}
					
					if ((baseTileRec.y + (r32)tileY * (baseTileRec.height + hexData->spacing.height)) > visData->hexData.viewRec.height)
					{
						break;
					}
				}
				RcSetViewport(NewRec(Vec2_Zero, RenderScreenSize));
				
				// +==============================+
				// |    Draw Viewport Shadows     |
				// +==============================+
				Color_t shadowColor1 = ColorTransparent(VisBlueGray, 0.8f);
				Color_t shadowColor2 = ColorTransparent(VisBlueGray, 0.0f);
				r32 shadowSize = 10;
				if (visData->hexData.viewPosGoto.x > hexData->tileSize.width/2)
				{
					rec gradientRec = visData->hexData.viewRec;
					gradientRec.width = shadowSize;
					RcDrawGradient(gradientRec, shadowColor1, shadowColor2, Dir2_Right);
				}
				if (visData->hexData.viewPosGoto.x + visData->hexData.viewRec.width < hexData->dataSize.width - hexData->tileSize.width/2)
				{
					rec gradientRec = visData->hexData.viewRec;
					gradientRec.x += gradientRec.width;
					gradientRec.width = shadowSize;
					gradientRec.x -= gradientRec.width;
					RcDrawGradient(gradientRec, shadowColor1, shadowColor2, Dir2_Left);
				}
				if (visData->hexData.viewPosGoto.y > hexData->tileSize.width/2)
				{
					rec gradientRec = visData->hexData.viewRec;
					gradientRec.height = shadowSize;
					RcDrawGradient(gradientRec, shadowColor1, shadowColor2, Dir2_Down);
				}
				if (visData->hexData.viewPosGoto.y + visData->hexData.viewRec.height < hexData->dataSize.height - hexData->tileSize.height/2)
				{
					rec gradientRec = visData->hexData.viewRec;
					gradientRec.y += gradientRec.height;
					gradientRec.height = shadowSize;
					gradientRec.y -= gradientRec.height;
					RcDrawGradient(gradientRec, shadowColor1, shadowColor2, Dir2_Up);
				}
				
				// +==============================+
				// |  Draw Interpretation Tiles   |
				// +==============================+
				RcSetViewport(visData->hexData.interpRec);
				rec interpTileRec = NewRec(visData->hexData.interpRec.topLeft + hexData->interpSpacing - hexData->interpViewPos, hexData->interpTileSize);
				u32 interpStartTileY = (u32)((hexData->interpViewPos.y - hexData->interpSpacing.height) / (interpTileRec.height + hexData->interpSpacing.height));
				for (u32 tileY = interpStartTileY; tileY < hexData->numRows; tileY++)
				{
					for (u32 tileX = 0; tileX < visData->hexData.numColumns; tileX++)
					{
						rec tileRec = interpTileRec;
						tileRec.x += (r32)tileX * (tileRec.width + hexData->interpSpacing.width);
						tileRec.y += (r32)tileY * (tileRec.height + hexData->interpSpacing.height);
						tileRec = RecInflate(tileRec, hexData->interpSpacing/2);
						
						u32 bIndex = (tileY*visData->hexData.numColumns) + tileX;
						u32 byteAddress = hexData->addressMin + bIndex;
						
						if (bIndex < visData->file.size && RecIntersects(tileRec, visData->hexData.interpRec))
						{
							TempPushMark();
							bool byteFilled = false;
							u8 byteValue = 0x00;
							for (u32 rIndex = 0; rIndex < hexData->numRegions; rIndex++)
							{
								VisHexDataRegion_t* region = &hexData->regions[rIndex];
								if (byteAddress >= region->address && byteAddress < region->address + region->size)
								{
									byteFilled = true;
									byteValue = region->data[byteAddress - region->address];
									break;
								}
							}
							char* hexStr = nullptr;
							if (byteFilled)
							{
								hexStr = TempString(1+1);
								if (IsCharClassPrintable((char)byteValue)) { hexStr[0] = (char)byteValue;}
								else { hexStr[0] = '-'; }
								hexStr[1] = '\0';
							}
							else { hexStr = "?"; }
							v2 hexStrSize = MeasureString(&visData->smallFont, hexStr, 1);
							v2 hexStrPos = tileRec.topLeft + tileRec.size/2 - hexStrSize/2;
							hexStrPos.y += visData->smallFont.maxExtendUp;
							hexStrPos.x = (r32)RoundR32(hexStrPos.x);
							hexStrPos.y = (r32)RoundR32(hexStrPos.y);
							
							Color_t backColor = VisWhite;
							Color_t textColor = VisBlueGray;
							Color_t borderColor = NewColor(Color_TransparentBlack);
							
							bool isSelected = false;
							if (bIndex >= visData->hexData.selectionStart && bIndex < visData->hexData.selectionEnd) { isSelected = true; }
							if (bIndex <= visData->hexData.selectionStart && bIndex >= visData->hexData.selectionEnd) { isSelected = true; }
							
							if (byteFilled)
							{
								if (outlineByteFilled && byteValue == outlineByte) { backColor = VisSilver; }
								if (visData->hexData.isHovering && bIndex == visData->hexData.hoverIndex)
								{
									if (isSelected)
									{
										backColor = VisPurple;
										textColor = VisWhite;
									}
									else
									{
										backColor = VisSilver;
									}
								}
								else if (isSelected)
								{
									backColor = VisLightPurple;
									textColor = VisWhite;
								}
							}
							else
							{
								backColor = VisGray;
							}
							
							RcDrawButton(tileRec, backColor, borderColor, 1);
							RcBindFont(&visData->smallFont);
							RcDrawString(hexStr, 1, hexStrPos, textColor);
							TempPopMark();
						}
					}
					
					if ((interpTileRec.y + (r32)tileY * (interpTileRec.height + hexData->interpSpacing.height)) > visData->hexData.viewRec.height)
					{
						break;
					}
				}
				RcSetViewport(NewRec(Vec2_Zero, RenderScreenSize));
				
				// // +==============================+
				// // |    Draw Viewport Shadows     |
				// // +==============================+
				// Color_t shadowColor1 = ColorTransparent(VisBlueGray, 0.8f);
				// Color_t shadowColor2 = ColorTransparent(VisBlueGray, 0.0f);
				// r32 shadowSize = 10;
				// if (visData->hexData.viewPosGoto.x > hexData->tileSize.width/2)
				// {
				// 	rec gradientRec = visData->hexData.viewRec;
				// 	gradientRec.width = shadowSize;
				// 	RcDrawGradient(gradientRec, shadowColor1, shadowColor2, Dir2_Right);
				// }
				// if (visData->hexData.viewPosGoto.x + visData->hexData.viewRec.width < hexData->dataSize.width - hexData->tileSize.width/2)
				// {
				// 	rec gradientRec = visData->hexData.viewRec;
				// 	gradientRec.x += gradientRec.width;
				// 	gradientRec.width = shadowSize;
				// 	gradientRec.x -= gradientRec.width;
				// 	RcDrawGradient(gradientRec, shadowColor1, shadowColor2, Dir2_Left);
				// }
				// if (visData->hexData.viewPosGoto.y > hexData->tileSize.width/2)
				// {
				// 	rec gradientRec = visData->hexData.viewRec;
				// 	gradientRec.height = shadowSize;
				// 	RcDrawGradient(gradientRec, shadowColor1, shadowColor2, Dir2_Down);
				// }
				// if (visData->hexData.viewPosGoto.y + visData->hexData.viewRec.height < hexData->dataSize.height - hexData->tileSize.height/2)
				// {
				// 	rec gradientRec = visData->hexData.viewRec;
				// 	gradientRec.y += gradientRec.height;
				// 	gradientRec.height = shadowSize;
				// 	gradientRec.y -= gradientRec.height;
				// 	RcDrawGradient(gradientRec, shadowColor1, shadowColor2, Dir2_Up);
				// }
				
				// +==============================+
				// |      Draw Row Addresses      |
				// +==============================+
				// RcDrawRectangle(hexData->addressesRec, NewColor(Color_LightGrey));
				for (u32 tileY = 0; tileY < hexData->numRows; tileY++)
				{
					r32 textTop = baseTileRec.y;
					textTop += (r32)tileY * (baseTileRec.height + hexData->spacing.height) + baseTileRec.height/2 - visData->smallFont.lineHeight/2;
					if (textTop < RenderScreenSize.height && textTop + visData->smallFont.lineHeight > 0)
					{
						u32 bIndex = tileY * visData->hexData.numColumns;
						TempPushMark();
						char* addressStr = TempPrint("%08X", hexData->addressMin + bIndex);
						v2 addressStrPos = NewVec2(visData->hexData.viewRec.x - 2, textTop + visData->smallFont.maxExtendUp);
						addressStrPos.x = (r32)RoundR32(addressStrPos.x);
						addressStrPos.y = (r32)RoundR32(addressStrPos.y);
						RcBindFont(&visData->smallFont);
						RcDrawString(addressStr, addressStrPos, VisYellow, 1.0f, Alignment_Right);
						TempPopMark();
					}
				}
				
				// +==============================+
				// |      Draw Scroll Gutter      |
				// +==============================+
				{
					RcDrawGradient(hexData->scrollGutterRec, VisGray, VisBlueGray, Dir2_Left);
					for (u32 rIndex = 0; rIndex < hexData->numRegions; rIndex++)
					{
						VisHexDataRegion_t* region = &hexData->regions[rIndex];
						u32 startRow = (region->address - hexData->addressMin) / hexData->numColumns;
						u32 endRow = (region->address + region->size - hexData->addressMin) / hexData->numColumns;
						rec regionRec = hexData->scrollGutterRec;
						regionRec.y = hexData->scrollGutterRec.y + hexData->scrollGutterRec.height * ((r32)startRow / (r32)hexData->numRows);
						regionRec.height = hexData->scrollGutterRec.height * ((r32)(endRow - startRow) / (r32)hexData->numRows);
						RcDrawGradient(regionRec, VisSilver, VisLightGray, Dir2_Left);
					}
				}
				
				// +==============================+
				// |        Draw Scrollbar        |
				// +==============================+
				{
					RcDrawGradient(hexData->scrollBarRec, VisGreen, VisLightGreen, Dir2_Right);
					RcDrawButton(hexData->scrollBarRec, NewColor(Color_TransparentBlack), VisBlueGray, 1);
					
					if (IsInsideRec(hexData->scrollGutterRec, RenderMousePos) || hexData->draggingScrollbar)
					{
						if (IsInsideRec(hexData->scrollBarRec, RenderMousePos) || hexData->draggingScrollbar)
						{
							r32 hoverPosY = hexData->viewPos.y;
							u32 hoverRow = (u32)((hoverPosY - hexData->spacing.height) / (hexData->tileSize.height + hexData->spacing.height));
							u32 hoverAddress = hexData->addressMin + hoverRow * hexData->numColumns;
							v2 strPos = hexData->scrollBarRec.topLeft + NewVec2(0, -visData->smallFont.maxExtendDown);
							if (strPos.y < visData->smallFont.maxExtendUp)
							{
								strPos.y = visData->smallFont.maxExtendUp;
								strPos.x = hexData->scrollBarRec.x + hexData->scrollBarRec.width+1;
							}
							strPos.x = (r32)RoundR32(strPos.x);
							strPos.y = (r32)RoundR32(strPos.y);
							RcBindFont(&visData->smallFont);
							RcPrintString(strPos + Vec2_One, VisBlueGray, 1.0f, "%08X", hoverAddress);
							RcPrintString(strPos, VisYellow, 1.0f, "%08X", hoverAddress);
						}
						else
						{
							r32 hoverPosY = (RenderMousePos.y - hexData->scrollGutterRec.y) / hexData->scrollGutterRec.height;
							hoverPosY *= hexData->dataSize.height;
							u32 hoverRow = (u32)((hoverPosY - hexData->spacing.height) / (hexData->tileSize.height + hexData->spacing.height));
							u32 hoverAddress = hexData->addressMin + hoverRow * hexData->numColumns;
							v2 strPos = RenderMousePos + NewVec2(0, -visData->smallFont.maxExtendDown);
							if (strPos.y < visData->smallFont.maxExtendUp)
							{
								strPos.y = visData->smallFont.maxExtendUp;
								strPos.x = hexData->scrollGutterRec.x + hexData->scrollGutterRec.width+1;
							}
							strPos.x = (r32)RoundR32(strPos.x);
							strPos.y = (r32)RoundR32(strPos.y);
							RcBindFont(&visData->smallFont);
							RcPrintString(strPos + Vec2_One, VisBlueGray, 1.0f, "%08X", hoverAddress);
							RcPrintString(strPos, VisYellow, 1.0f, "%08X", hoverAddress);
						}
					}
				}
				
				RcDrawButton(hexData->fileInfoRec, VisLightBlueGray, VisLightGray, 1);
				RcDrawButton(hexData->selectionInfoRec, VisLightBlueGray, VisLightGray, 1);
				
				// +==============================+
				// |     Draw File Info View      |
				// +==============================+
				RcBindFont(&visData->smallFont);
				RcDrawString("File", hexData->fileInfoRec.topLeft + NewVec2(2, 0), VisLightGray);
				RcSetViewport(hexData->fileInfoRec);
				{
					v2 textPos = hexData->fileInfoRec.topLeft + NewVec2(2, 2 + visData->smallFont.maxExtendUp) - hexData->fileInfoViewPos;
					hexData->fileInfoDataSize = Vec2_Zero;
					
					#define DrawFileInfoItem(formatStr, ...) do                                                                                                       \
					{                                                                                                                                                 \
						char* infoStr = TempPrint(formatStr, __VA_ARGS__);                                                                                            \
						v2 infoStrSize = MeasureString(&visData->smallFont, infoStr);                                                                                 \
						if (textPos.x + infoStrSize.width > hexData->fileInfoDataSize.width) { hexData->fileInfoDataSize.width = textPos.x + infoStrSize.width; }     \
						if (textPos.y + infoStrSize.height > hexData->fileInfoDataSize.height) { hexData->fileInfoDataSize.height = textPos.y + infoStrSize.height; } \
						RcDrawString(infoStr, textPos, VisWhite);                                                                                                     \
						textPos.y += visData->smallFont.lineHeight + 2;                                                                                               \
					} while(0)
					
					DrawFileInfoItem("File: \"%s\"", visData->filePath);
					DrawFileInfoItem("Type: \"%s\"", GetVisFileTypeString(visData->fileType));
					DrawFileInfoItem("Size: %s (%u bytes)", FormattedSizeStr(visData->file.size), visData->file.size);
					DrawFileInfoItem("");
					DrawFileInfoItem("Address Range: %08X-%08X", hexData->addressMin, hexData->addressMax);
					DrawFileInfoItem("Regions Size: %s (%u bytes)", FormattedSizeStr(hexData->regionsSize), hexData->regionsSize);
					DrawFileInfoItem("Regions[%u]:", hexData->numRegions);
					for (u32 rIndex = 0; rIndex < hexData->numRegions; rIndex++)
					{
						VisHexDataRegion_t* regionPntr = &hexData->regions[rIndex];
						//TODO: change visData->file.size to be a total byte count of all regions
						DrawFileInfoItem("\t[%u] %08X-%08X %s (%.0f%%)", rIndex, regionPntr->address, regionPntr->address + regionPntr->size, FormattedSizeStr(regionPntr->size), (r32)regionPntr->size / (r32)hexData->regionsSize * 100);
					}
				}
				RcSetViewport(NewRec(Vec2_Zero, RenderScreenSize));
				
				// +==============================+
				// |   Draw Selection Info View   |
				// +==============================+
				RcBindFont(&visData->smallFont);
				RcDrawString("Selection", hexData->selectionInfoRec.topLeft + NewVec2(2, 0), VisLightGray);
				RcSetViewport(hexData->selectionInfoRec);
				{
					v2 textPos = hexData->selectionInfoRec.topLeft + NewVec2(2, 2 + visData->smallFont.maxExtendUp) - hexData->selInfoViewPos;
					hexData->selInfoDataSize = Vec2_Zero;
					
					#define DrawSelInfoItem(formatStr, ...) do                                                                                                      \
					{                                                                                                                                               \
						char* infoStr = TempPrint(formatStr, __VA_ARGS__);                                                                                          \
						v2 infoStrSize = MeasureString(&visData->smallFont, infoStr);                                                                               \
						if (textPos.x + infoStrSize.width > hexData->selInfoDataSize.width) { hexData->selInfoDataSize.width = textPos.x + infoStrSize.width; }     \
						if (textPos.y + infoStrSize.height > hexData->selInfoDataSize.height) { hexData->selInfoDataSize.height = textPos.y + infoStrSize.height; } \
						RcDrawString(infoStr, textPos, VisWhite);                                                                                                   \
						textPos.y += visData->smallFont.lineHeight + 2;                                                                                             \
					} while(0)
					
					DrawSelInfoItem("Hover Pos: %08X", hexData->addressMin + hexData->hoverIndex);
					DrawSelInfoItem("Selection: %08X-%08X", hexData->addressMin + hexData->selectionStart, hexData->addressMin + hexData->selectionEnd);
					DrawSelInfoItem("Size: %s (%u bytes)", FormattedSizeStr(selectionLength), selectionLength);
					DrawFileInfoItem("");
					
					if (selectionDataPntr != nullptr && selectionLength > 0 && selectionLength < 2048)
					{
						DrawFileInfoItem("String: \"%.*s\"", selectionLength, (char*)selectionDataPntr);
					}
					
					if (selectionDataPntr != nullptr && (selectionLength == 1 || selectionLength == 2 || selectionLength == 4 || selectionLength == 8))
					{
						u8 bigEndBuffer[8]; memcpy(bigEndBuffer, selectionDataPntr, selectionLength);
						u8 littleEndBuffer[8]; memcpy(littleEndBuffer, selectionDataPntr, selectionLength);
						ReverseByteArray(bigEndBuffer, selectionLength);
						
						if (selectionLength > 1) { DrawSelInfoItem("Little Endian:"); }
						if (selectionLength == 1)
						{
							u8 data = *(&littleEndBuffer[0]);
							i8 dataSigned = *(i8*)(&littleEndBuffer[0]);
							DrawSelInfoItem("\tUnsigned:    %u", data);
							DrawSelInfoItem("\tSigned:      %d", dataSigned);
							DrawSelInfoItem("\tHexadecimal: %02X", data);
							DrawSelInfoItem("\tBinary:      %s", GetBinaryStrU8(data, true));
							if (IsCharClassPrintable((char)data)){ DrawSelInfoItem("\tASCII:       \'%c\'", (char)data); }
							else { DrawSelInfoItem("\tASCII:       Invalid"); }
						}
						else if (selectionLength == 2)
						{
							u16 data = *(u16*)(&littleEndBuffer[0]);
							i16 dataSigned = *(i16*)(&littleEndBuffer[0]);
							DrawSelInfoItem("\tUnsigned:    %u", data);
							DrawSelInfoItem("\tSigned:      %d", dataSigned);
							DrawSelInfoItem("\tHexadecimal: %04X", data);
							DrawSelInfoItem("\tBinary:      %s", GetBinaryStrU16(data, true));
						}
						else if (selectionLength == 4)
						{
							u32 data = *(u32*)(&littleEndBuffer[0]);
							i32 dataSigned = *(i32*)(&littleEndBuffer[0]);
							DrawSelInfoItem("\tUnsigned:    %u", data);
							DrawSelInfoItem("\tSigned:      %d", dataSigned);
							DrawSelInfoItem("\tHexadecimal: %08X", data);
							DrawSelInfoItem("\tBinary:      %s", GetBinaryStrU32(data, true));
						}
						else if (selectionLength == 8)
						{
							u64 data = *(u64*)(&littleEndBuffer[0]);
							i64 dataSigned = *(i64*)(&littleEndBuffer[0]);
							DrawSelInfoItem("\tUnsigned:    %u", data);
							DrawSelInfoItem("\tSigned:      %d", dataSigned);
							DrawSelInfoItem("\tHexadecimal: %016X", data);
							DrawSelInfoItem("\tBinary:      %s", GetBinaryStrU64(data, true));
						}
						DrawFileInfoItem("");
						
						if (selectionLength > 1)
						{
							DrawSelInfoItem("Big Endian:");
							if (selectionLength == 2)
							{
								u16 data = *(u16*)(&bigEndBuffer[0]);
								i16 dataSigned = *(i16*)(&littleEndBuffer[0]);
								DrawSelInfoItem("\tUnsigned:    %u", data);
								DrawSelInfoItem("\tSigned:      %d", dataSigned);
								DrawSelInfoItem("\tHexadecimal: %04X", data);
								DrawSelInfoItem("\tBinary:      %s", GetBinaryStrU16(data, true));
							}
							else if (selectionLength == 4)
							{
								u32 data = *(u32*)(&bigEndBuffer[0]);
								i32 dataSigned = *(i32*)(&littleEndBuffer[0]);
								DrawSelInfoItem("\tUnsigned:    %u", data);
								DrawSelInfoItem("\tSigned:      %d", dataSigned);
								DrawSelInfoItem("\tHexadecimal: %08X", data);
								DrawSelInfoItem("\tBinary:      %s", GetBinaryStrU32(data, true));
							}
							else if (selectionLength == 8)
							{
								u64 data = *(u64*)(&bigEndBuffer[0]);
								i64 dataSigned = *(i64*)(&littleEndBuffer[0]);
								DrawSelInfoItem("\tUnsigned:    %u", data);
								DrawSelInfoItem("\tSigned:      %d", dataSigned);
								DrawSelInfoItem("\tHexadecimal: %016X", data);
								DrawSelInfoItem("\tBinary:      %s", GetBinaryStrU64(data, true));
							}
							DrawFileInfoItem("");
						}
					}
				}
				RcSetViewport(NewRec(Vec2_Zero, RenderScreenSize));
				
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
		else
		{
			RcBindFont(&visData->largeFont);
			RcDrawString("Drop a file to view", RenderScreenSize/2, VisWhite, 1.0f, Alignment_Center);
		}
	}
}

/*
File:   app_font.cpp
Author: Taylor Robbins
Date:   05\15\2018
Description:
	** Handles interfacing with STB TrueType to generate font bakes and
	** glyphs for the application to use in rendering text

#included from app.cpp
*/

void DestroyNewFont(NewFont_t* fontPntr)
{
	Assert(fontPntr != nullptr);
	Assert(fontPntr->allocArena != nullptr);
	
	if (fontPntr->files != nullptr)
	{
		for (u32 fIndex = 0; fIndex < fontPntr->numFiles; fIndex++)
		{
			FontFile_t* filePntr = &fontPntr->files[fIndex];
			if (filePntr->fileData != nullptr)
			{
				ArenaPop(fontPntr->allocArena, filePntr->fileData);
				filePntr->fileData = nullptr;
			}
		}
		ArenaPop(fontPntr->allocArena, fontPntr->files);
		fontPntr->files = nullptr;
		fontPntr->numFiles = 0;
	}
	
	if (fontPntr->bakes != nullptr)
	{
		for (u32 bIndex = 0; bIndex < fontPntr->numBakes; bIndex++)
		{
			FontBake_t* bakePntr = &fontPntr->bakes[bIndex];
			DestroyTexture(&bakePntr->texture);
			if (bakePntr->charInfos != nullptr)
			{
				ArenaPop(fontPntr->allocArena, bakePntr->charInfos);
				bakePntr->charInfos = nullptr;
			}
		}
		ArenaPop(fontPntr->allocArena, fontPntr->bakes);
		fontPntr->bakes = nullptr;
		fontPntr->numBakes = 0;
	}
	
	if (fontPntr->glyphs != nullptr)
	{
		for (u32 gIndex = 0; gIndex < fontPntr->numGlyphs; gIndex++)
		{
			FontGlyph_t* glyphPntr = &fontPntr->glyphs[gIndex];
			DestroyTexture(&glyphPntr->texture);
		}
		ArenaPop(fontPntr->allocArena, fontPntr->glyphs);
		fontPntr->glyphs = nullptr;
		fontPntr->numGlyphs = 0;
		fontPntr->maxGlyphs = 0;
	}
}

bool CreateGameFont(NewFont_t* fontPntr, MemoryArena_t* arenaPntr)
{
	Assert(fontPntr != nullptr);
	Assert(arenaPntr != nullptr);
	
	ClearPointer(fontPntr);
	fontPntr->allocArena = arenaPntr;
	
	return true;
}

bool FontAddFile(NewFont_t* fontPntr, const void* fileData, u32 fileLength, FontStyle_t styleFlags = FontStyle_Default)
{
	Assert(fontPntr != nullptr);
	Assert(fontPntr->allocArena != nullptr);
	Assert(fileData != nullptr);
	Assert(fileLength > 0);
	
	for (u32 fIndex = 0; fIndex < fontPntr->numFiles; fIndex++)
	{
		FontFile_t* filePntr = &fontPntr->files[fIndex];
		if (filePntr->styleFlags == styleFlags)
		{
			DEBUG_PrintLine("Another file with styleFlags %02X was already added to this font!", styleFlags);
			return false;
		}
	}
	
	FontFile_t* newFilePntr = nullptr;
	if (fontPntr->files != nullptr)
	{
		FontFile_t* newFiles = PushArray(fontPntr->allocArena, FontFile_t, fontPntr->numFiles+1);
		memcpy(newFiles, fontPntr->files, sizeof(FontFile_t) * fontPntr->numFiles);
		ArenaPop(fontPntr->allocArena, fontPntr->files);
		fontPntr->files = newFiles;
		fontPntr->numFiles++;
		
		newFilePntr = &fontPntr->files[fontPntr->numFiles-1];
	}
	else
	{
		fontPntr->files = PushArray(fontPntr->allocArena, FontFile_t, 1);
		fontPntr->numFiles = 1;
		
		newFilePntr = &fontPntr->files[0];
	}
	Assert(newFilePntr != nullptr);
	ClearPointer(newFilePntr);
	
	newFilePntr->styleFlags = styleFlags;
	newFilePntr->fileLength = fileLength;
	newFilePntr->fileData = PushArray(fontPntr->allocArena, u8, fileLength);
	memcpy(newFilePntr->fileData, fileData, fileLength);
	
	int stbResult = stbtt_InitFont(&newFilePntr->stbInfo, newFilePntr->fileData, stbtt_GetFontOffsetForIndex(newFilePntr->fileData, 0));
	if (stbResult == 0)
	{
		DEBUG_WriteLine("stbtt_InitFont failed!");
		ArenaPop(fontPntr->allocArena, newFilePntr->fileData);
		newFilePntr->fileData = nullptr;
		newFilePntr->fileLength = 0;
	}
	
	return true;
}

bool FontLoadFile(NewFont_t* fontPntr, const char* filePath, FontStyle_t styleFlags = FontStyle_Default)
{
	Assert(filePath != nullptr);
	
	FileInfo_t fontFile = platform->ReadEntireFile(filePath);
	if (fontFile.content == nullptr) { return false; }
	bool result = FontAddFile(fontPntr, (const u8*)fontFile.content, fontFile.size, styleFlags);
	platform->FreeFileMemory(&fontFile);
	return result;
}

bool FontBake(NewFont_t* fontPntr, r32 size, FontStyle_t styleFlags = FontStyle_Default, u8 firstChar = 0x20, u8 numChars = 96)
{
	Assert(fontPntr != nullptr);
	Assert(fontPntr->allocArena != nullptr);
	Assert((u32)firstChar + numChars < 256);
	
	FontFile_t* fontFilePntr = nullptr;
	for (u32 fIndex = 0; fIndex < fontPntr->numFiles; fIndex++)
	{
		FontFile_t* filePntr = &fontPntr->files[fIndex];
		if (filePntr->styleFlags == styleFlags)
		{
			fontFilePntr = filePntr;
			break;
		}
	}
	if (fontFilePntr == nullptr)
	{
		DEBUG_PrintLine("There is not font file for styleFlags %02X in this font!", styleFlags);
		return false;
	}
	Assert(fontFilePntr->fileData != nullptr);
	
	FontBake_t* bakePntr = nullptr;
	if (fontPntr->bakes != nullptr)
	{
		FontBake_t* newBakes = PushArray(fontPntr->allocArena, FontBake_t, fontPntr->numBakes+1);
		memcpy(newBakes, fontPntr->bakes, sizeof(FontBake_t) * fontPntr->numBakes);
		ArenaPop(fontPntr->allocArena, fontPntr->bakes);
		fontPntr->bakes = newBakes;
		fontPntr->numBakes++;
		
		bakePntr = &fontPntr->bakes[fontPntr->numBakes-1];
	}
	else
	{
		fontPntr->bakes = PushArray(fontPntr->allocArena, FontBake_t, 1);
		bakePntr = fontPntr->bakes;
		fontPntr->numBakes = 1;
	}
	Assert(bakePntr != nullptr);
	ClearPointer(bakePntr);
	
	bakePntr->firstChar = firstChar;
	bakePntr->numChars = numChars;
	bakePntr->size = size;
	bakePntr->styleFlags = styleFlags;
	
	//TODO: Make these dynamically chosen somehow?
	u32 bitmapWidth = 512;
	u32 bitmapHeight = 512;
	
	TempPushMark();
	{
		u8* bitmapData = PushArray(TempArena, u8, bitmapWidth * bitmapHeight * 4);
		Assert(bitmapData != nullptr);
		
		TempPushMark();
		{
			stbtt_bakedchar* stbCharInfos = PushArray(TempArena, stbtt_bakedchar, numChars);
			Assert(stbCharInfos != nullptr);
			
			int bakeResult = stbtt_BakeFontBitmap(fontFilePntr->fileData, 0, size, //TODO: Change 0 for something if styleFlags are set
				bitmapData, bitmapWidth, bitmapHeight,
				firstChar, numChars, stbCharInfos); //TODO: firstChar and numChars are ints so maybe they support unicode points?
			if (bakeResult <= 0)
			{
				DEBUG_PrintLine("STB Bake could only fit %d/%u characters", -bakeResult, numChars);
				TempPopMark();
				TempPopMark();
				return false;
			}
			
			bakePntr->charInfos = PushArray(fontPntr->allocArena, NewFontCharInfo_t, numChars);
			for (u8 cIndex = 0; cIndex < numChars; cIndex++)
			{
				NewFontCharInfo_t* charPntr = &bakePntr->charInfos[cIndex];
				stbtt_bakedchar* stbCharPntr = &stbCharInfos[cIndex];
				charPntr->bakePos  = NewVec2i(
					stbCharPntr->x0,
					stbCharPntr->y0
				);
				charPntr->bakeSize = NewVec2i(
					stbCharPntr->x1 - stbCharPntr->x0,
					stbCharPntr->y1 - stbCharPntr->y0
				);
				charPntr->size = NewVec2(charPntr->bakeSize);
				charPntr->origin = NewVec2(-stbCharPntr->xoff, -stbCharPntr->yoff);
				charPntr->advanceX = stbCharPntr->xadvance;
			}
		}
		TempPopMark();
			
		//Conver the grayscale bitmap to a 32 bit color bitmap
		//NOTE: We reuse the same bitmap buffer by moving backwards
		for (u32 bIndex = bitmapWidth*bitmapHeight; bIndex > 0; bIndex--)
		{
			u32* pixelPntr = (u32*)(&bitmapData[(bIndex-1) * sizeof(u32)]);
			u8* grayscalePntr = &bitmapData[(bIndex-1) * sizeof(u8)];
			Assert((u8*)pixelPntr >= grayscalePntr);
			Assert((u8*)pixelPntr < bitmapData + (bitmapWidth * bitmapHeight * 4));
			
			*pixelPntr = (*grayscalePntr << 24) | 0x00FFFFFF;
		}
		
		bakePntr->texture = CreateTexture(bitmapData, bitmapWidth, bitmapHeight);
	}
	TempPopMark();
	
	//Calculate maxExtendUp, maxExtendDown, and lineHeight
	bakePntr->maxExtendUp = 0;
	bakePntr->maxExtendDown = 0;
	for (u8 cIndex = 0; cIndex < numChars; cIndex++)
	{
		NewFontCharInfo_t* charInfoPntr = &bakePntr->charInfos[cIndex];
		
		if (charInfoPntr->origin.y > bakePntr->maxExtendUp)
		{
			bakePntr->maxExtendUp = charInfoPntr->origin.y;
		}
		
		if (charInfoPntr->size.height - charInfoPntr->origin.y > bakePntr->maxExtendDown)
		{
			bakePntr->maxExtendDown = charInfoPntr->size.height - charInfoPntr->origin.y;
		}
	}
	bakePntr->lineHeight = bakePntr->maxExtendDown + bakePntr->maxExtendUp;
	
	return true;
}

void FontDropFiles(NewFont_t* fontPntr)
{
	Assert(fontPntr != nullptr);
	if (fontPntr->files != nullptr)
	{
		for (u32 fIndex = 0; fIndex < fontPntr->numFiles; fIndex++)
		{
			FontFile_t* filePntr = &fontPntr->files[fIndex];
			if (filePntr->fileData != nullptr)
			{
				ArenaPop(fontPntr->allocArena, filePntr->fileData);
				filePntr->fileData = nullptr;
			}
		}
		ArenaPop(fontPntr->allocArena, fontPntr->files);
		fontPntr->files = nullptr;
		fontPntr->numFiles = 0;
	}
}

u32 FontGetMemoryUsage(NewFont_t* fontPntr)
{
	Assert(fontPntr != nullptr);
	
	u32 memoryUsage = 0;
	
	for (u32 fIndex = 0; fIndex < fontPntr->numFiles; fIndex++)
	{
		memoryUsage += fontPntr->files[fIndex].fileLength;
	}
	memoryUsage += fontPntr->numFiles * sizeof(FontFile_t);
	
	for (u32 bIndex = 0; bIndex < fontPntr->numBakes; bIndex++)
	{
		memoryUsage += fontPntr->bakes[bIndex].numChars * sizeof(NewFontCharInfo_t);
	}
	memoryUsage += fontPntr->numBakes * sizeof(FontBake_t);
	
	memoryUsage += fontPntr->numGlyphs * sizeof(FontGlyph_t);
	
	return memoryUsage;
}

// TODO: This should be doing a best match if the font doesn't match exactly
FontBake_t* FontGetBakeFor(NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default, char* neededChar = nullptr)
{
	Assert(fontPntr != nullptr);
	
	FontBake_t* bestMatch = nullptr;
	for (u32 bIndex = 0; bIndex < fontPntr->numBakes; bIndex++)
	{
		Assert(fontPntr->bakes != nullptr);
		FontBake_t* bakePntr = &fontPntr->bakes[bIndex];
		if (neededChar == nullptr || ((u8)*neededChar >= bakePntr->firstChar && (u8)*neededChar - bakePntr->firstChar < bakePntr->numChars))
		{
			u8 newStyleDiff = FlagsDiffU8((bakePntr->styleFlags & FontStyle_BakeMask), (styleFlags & FontStyle_BakeMask));
			if (bakePntr->size == fontSize && newStyleDiff == 0)
			{
				return bakePntr;
			}
			else if ((fontSize == 0 || bakePntr->size == fontSize || !IsFlagSet(styleFlags, FontStyle_StrictSize)) &&
				(bakePntr->styleFlags == styleFlags || !IsFlagSet(styleFlags, FontStyle_StrictStyle)))
			{
				if (bestMatch != nullptr)
				{
					r32 newSizeDiff = AbsR32(fontSize - bakePntr->size);
					r32 oldSizeDiff = AbsR32(fontSize - bestMatch->size);
					u8 oldStyleDiff = FlagsDiffU8((bestMatch->styleFlags & FontStyle_BakeMask), (styleFlags & FontStyle_BakeMask));
					if (newSizeDiff < oldSizeDiff)
					{
						bestMatch = bakePntr;
					}
					else if (newSizeDiff == oldSizeDiff && newStyleDiff < oldStyleDiff)
					{
						bestMatch = bakePntr;
					}
				}
				else
				{
					bestMatch = bakePntr;
				}
			}
		}
	}
	
	return bestMatch;
}
r32 FontGetLineHeight(NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default)
{
	FontBake_t* fontBake = FontGetBakeFor(fontPntr, fontSize, styleFlags);
	if (fontBake != nullptr)
	{
		return fontBake->lineHeight;
	}
	else
	{
		return fontSize; //This is often correct
	}
}
r32 FontGetMaxExtendUp(NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default)
{
	FontBake_t* fontBake = FontGetBakeFor(fontPntr, fontSize, styleFlags);
	if (fontBake != nullptr)
	{
		return fontBake->maxExtendUp;
	}
	else
	{
		return fontSize * (3/4); //NOTE: This is just a random guess
	}
}
r32 FontGetMaxExtendDown(NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default)
{
	FontBake_t* fontBake = FontGetBakeFor(fontPntr, fontSize, styleFlags);
	if (fontBake != nullptr)
	{
		return fontBake->maxExtendDown;
	}
	else
	{
		return fontSize * (1/4); //NOTE: This is just a random guess
	}
}

struct FontChar_t
{
	bool baked;
	Texture_t* texture;
	NewFontCharInfo_t* info;
};

//NOTE: fontCharOut can be nullptr
bool FontGetChar(NewFont_t* fontPntr, FontChar_t* fontCharOut, char character, r32 size = 0, FontStyle_t styleFlags = FontStyle_Default)
{
	Assert(fontPntr != nullptr);
	
	FontBake_t* bakePntr = FontGetBakeFor(fontPntr, size, styleFlags, &character);
	if (bakePntr != nullptr)
	{
		if (fontCharOut != nullptr)
		{
			ClearPointer(fontCharOut);
			fontCharOut->baked = true;
			fontCharOut->texture = &bakePntr->texture;
			fontCharOut->info = &bakePntr->charInfos[(u8)character - bakePntr->firstChar];
		}
		return true;
	}
	
	if (IsFlagSet(styleFlags, FontStyle_CreateGlyphs))
	{
		Assert(false); //TODO: Not yet supported!
	}
	
	return false;
}

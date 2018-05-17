/*
File:   app_fontFlow.cpp
Author: Taylor Robbins
Date:   05\15\2018
Description: 
	** Holds the font flow functions that handle the spacing and rendering
	** of fonts on the screen or for sizing purposes 
*/

//NOTE: If maxWidth is not nullptr then it will be filled with the actual width of the line that fit into that space
u32 FontMeasureLineWidth(const char* strPntr, u32 strLength, r32* maxWidthPntr, NewFont_t* fontPntr,
	r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default)
{
	Assert(strPntr != nullptr);
	Assert(strLength > 0);
	Assert(fontPntr != nullptr);
	
	FontStyle_t currentStyle = styleFlags;
	r32 currentSize = fontSize;
	r32 maxWidth = 0;
	if (maxWidthPntr != nullptr)
	{
		maxWidth = *maxWidthPntr;
		*maxWidthPntr = 0;
	}
	
	u32 breakPoint = 0;
	r32 xPos = 0;
	r32 lastRenderWidth = 0;
	bool wasRenderable = false;
	for (u32 cIndex = 0; cIndex < strLength; cIndex++)
	{
		char nextChar = strPntr[cIndex];
		u32 numCharsLeft = strLength - (cIndex+1);
		
		FontChar_t fontChar;
		r32 charWidth = 0;
		r32 charAdvance = 0;
		bool isRenderable = false;
		if (nextChar == '\t')
		{
			if (FontGetChar(fontPntr, &fontChar, ' ', currentSize, currentStyle))
			{
				charWidth = (fontChar.info->size.width - fontChar.info->origin.x) * TAB_WIDTH;
				charAdvance = fontChar.info->advanceX * TAB_WIDTH;
			}
		}
		else if (nextChar == ' ')
		{
			if (FontGetChar(fontPntr, &fontChar, ' ', currentSize, currentStyle))
			{
				charWidth = fontChar.info->size.width - fontChar.info->origin.x;
				charAdvance = fontChar.info->advanceX;
			}
		}
		else if (nextChar == '\n')
		{
			//Handled more below
		}
		else if (nextChar == '\b')
		{
			currentStyle = (FontStyle_t)(currentStyle ^ FontStyle_Bold);
		}
		else if (nextChar == '\r')
		{
			currentStyle = (FontStyle_t)(currentStyle ^ FontStyle_Italic);
		}
		else if (nextChar == '\x01' && numCharsLeft >= 3)
		{
			//NOTE: Color doesn't matter for size
			cIndex += 3;
		}
		else if (nextChar == '\x02')
		{
			//NOTE: Color doesn't matter for size
		}
		else if (nextChar == '\x03' && numCharsLeft >= 1)
		{
			currentSize = (r32)((u8)strPntr[cIndex+1]);
			cIndex += 1;
		}
		else if (nextChar == '\x04')
		{
			currentSize = fontSize;
		}
		else
		{
			if (FontGetChar(fontPntr, &fontChar, nextChar, currentSize, currentStyle))
			{
				if (!IsCharClassAlphaNumeric(nextChar) && IsCharClassBeginningCharacter(nextChar))
				{
					breakPoint = cIndex;
					if (maxWidthPntr != nullptr) { *maxWidthPntr = lastRenderWidth; }
				}
				isRenderable = true;
				charWidth = fontChar.info->size.width - fontChar.info->origin.x;
				charAdvance = fontChar.info->advanceX;
			}
		}
		
		r32 renderWidth = xPos + charWidth;
		
		if (isRenderable)
		{
			if (maxWidthPntr != nullptr && maxWidth != 0 && renderWidth > maxWidth)
			{
				if (breakPoint == 0)
				{
					if (cIndex > 0)
					{
						breakPoint = cIndex;
						*maxWidthPntr = lastRenderWidth;
					}
					else
					{
						breakPoint = 1;
						*maxWidthPntr = renderWidth;
					}
				}
				break;
			}
			else if (!IsCharClassAlphaNumeric(nextChar) && !IsCharClassBeginningCharacter(nextChar))
			{
				breakPoint = cIndex+1;
				if (maxWidthPntr != nullptr) { *maxWidthPntr = renderWidth; }
			}
			else if (cIndex == strLength-1) //end of the string
			{
				breakPoint = cIndex+1; //All the characters fit
				if (maxWidthPntr != nullptr) { *maxWidthPntr = renderWidth; }
			}
		}
		else
		{
			breakPoint = cIndex+1;
			if (wasRenderable && maxWidthPntr != nullptr)
			{
				*maxWidthPntr = lastRenderWidth;
			}
			if (nextChar == '\n') { break; }
		}
		
		
		xPos += charAdvance;
		lastRenderWidth = renderWidth;
		wasRenderable = isRenderable;
	}
	
	// if (breakPoint == 0) { breakPoint = 1; }
	return breakPoint;
}

//Returns the end position of the text
v2 FontPerformTextFlow(bool drawToScreen, const char* strPntr, u32 strLength, v2 position, Color_t color, NewFont_t* fontPntr,
	Alignment_t alignment = Alignment_Left, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	r32 lineHeight = FontGetLineHeight(fontPntr, fontSize, styleFlags);
	r32 maxExtendUp = FontGetMaxExtendUp(fontPntr, fontSize, styleFlags);
	r32 maxExtendDown = FontGetMaxExtendDown(fontPntr, fontSize, styleFlags);
	if (flowInfo != nullptr)
	{
		ClearPointer(flowInfo);
		flowInfo->strPntr = strPntr;
		flowInfo->strLength = strLength;
		flowInfo->position = position;
		flowInfo->fontPntr = fontPntr;
		flowInfo->alignment = alignment;
		flowInfo->fontSize = fontSize;
		flowInfo->styleFlags = styleFlags;
		
		flowInfo->totalSize = NewVec2(0, lineHeight);
		flowInfo->extents = NewRec(position.x, position.y - maxExtendUp, 0, lineHeight);
		flowInfo->endPos = position;
		flowInfo->numLines = 0;
		flowInfo->numRenderables = 0;
	}
	
	r32 underlineThickness = 1;
	v2 underlineOffset = NewVec2(0, 1);
	FontStyle_t currentStyle = styleFlags;
	Color_t currentColor = color;
	r32 currentSize = fontSize;
	v2 drawPos = position;
	FontChar_t fontChar;
	u32 chunkStart = 0;
	while (chunkStart < strLength)
	{
		const char* chunkPntr = &strPntr[chunkStart];
		r32 lineWidth = maxWidth;
		u32 chunkLength = FontMeasureLineWidth(chunkPntr, strLength - chunkStart, &lineWidth, fontPntr, currentSize, currentStyle);
		DEBUG_PrintLine("Got chunk %u-%u/%u", chunkStart, chunkStart + chunkLength, strLength);
		if (alignment == Alignment_Right)
		{
			drawPos.x -= lineWidth;
		}
		else if (alignment == Alignment_Center)
		{
			drawPos.x -= lineWidth / 2;
		}
		
		v2 underlineStart = drawPos;
		for (u32 cIndex = 0; cIndex < chunkLength; cIndex++)
		{
			u32 numCharsLeft = chunkLength - (cIndex+1);
			char nextChar = chunkPntr[cIndex];
			v2 charTopLeft = drawPos;
			v2 charBottomRight = drawPos;
			if (nextChar == '\n')
			{
				Assert(cIndex == chunkLength-1);
				break;
			}
			else if (nextChar == '\t')
			{
				if (FontGetChar(fontPntr, &fontChar, ' ', currentSize, currentStyle))
				{
					charTopLeft = drawPos - fontChar.info->origin;
					charBottomRight = drawPos + (fontChar.info->size - fontChar.info->origin);
					drawPos.x += fontChar.info->advanceX * TAB_WIDTH;
				}
				else
				{
					//TODO: Can't render a tab because no space character available!
				}
			}
			else if (nextChar == '\b') //Bold
			{
				currentStyle = (FontStyle_t)(currentStyle ^ FontStyle_Bold);
			}
			else if (nextChar == '\r') //Italic
			{
				currentStyle = (FontStyle_t)(currentStyle ^ FontStyle_Italic);
			}
			else if (nextChar == '\a') //Underline
			{
				if (IsFlagSet(currentStyle, FontStyle_Underline) && underlineStart != drawPos)
				{
					RcDrawLine(underlineStart + underlineOffset, drawPos + underlineOffset, underlineThickness, currentColor);
				}
				else
				{
					underlineStart = drawPos;
				}
				currentStyle = (FontStyle_t)(currentStyle ^ FontStyle_Underline);
			}
			else if (nextChar == '\x01' && numCharsLeft >= 3) //Set Color
			{
				if (IsFlagSet(currentStyle, FontStyle_Underline) && underlineStart != drawPos)
				{
					RcDrawLine(underlineStart + underlineOffset, drawPos + underlineOffset, underlineThickness, currentColor);
					underlineStart = drawPos;
				}
				currentColor.red = (u8)(chunkPntr[cIndex+1]);
				currentColor.green = (u8)(chunkPntr[cIndex+2]);
				currentColor.blue = (u8)(chunkPntr[cIndex+3]);
				cIndex += 3;
			}
			else if (nextChar == '\x02') //Default Color
			{
				if (IsFlagSet(currentStyle, FontStyle_Underline) && underlineStart != drawPos)
				{
					RcDrawLine(underlineStart + underlineOffset, drawPos + underlineOffset, underlineThickness, currentColor);
					underlineStart = drawPos;
				}
				currentColor = color;
			}
			else if (nextChar == '\x03' && numCharsLeft >= 1) //Set Size
			{
				currentSize = (r32)((u8)chunkPntr[cIndex+1]);
				cIndex += 1;
			}
			else if (nextChar == '\x04') //Default Size
			{
				currentSize = fontSize;
			}
			else if (FontGetChar(fontPntr, &fontChar, nextChar, currentSize, currentStyle))
			{
				if (drawToScreen)
				{
					RcBindTexture(fontChar.texture);
					rec drawRec = NewRec(drawPos - fontChar.info->origin, fontChar.info->size);
					Color_t drawColor = currentColor;
					if (IsFlagSet(currentStyle, FontStyle_Bold))
					{
						drawRec.y += maxExtendDown/2 * SinR32((r32)((platform->programTime + cIndex*30)%800) / 800 * 2*Pi32);
					}
					if (IsFlagSet(currentStyle, FontStyle_Italic))
					{
						r32 scale = 1.0f + 0.1f*SinR32((r32)((platform->programTime + cIndex*30)%800) / 800 * 2*Pi32);
						// drawRec.size = drawRec.size * scale;
						drawRec = RecInflateX(drawRec, drawRec.size.x * (scale - 1.0f));
						drawRec = RecInflateY(drawRec, drawRec.size.y * (scale - 1.0f));
					}
					if (!IsFlagSet(currentStyle, FontStyle_Italic))
					{
						r32 hue = 180 + 180*SinR32((r32)((platform->programTime + cIndex*0)%4000) / 4000 * 2*Pi32);
						drawColor = ColorFromHSV((u16)RoundR32(hue), 0.7f, 1.0f);
					}
					RcDrawTexturedRec(drawRec, drawColor, NewRec(fontChar.info->bakeRec));
				}
				
				charTopLeft = drawPos - fontChar.info->origin;
				charBottomRight = charTopLeft + fontChar.info->size;
				drawPos.x += fontChar.info->advanceX;
				if (flowInfo != nullptr) { flowInfo->numRenderables++; }
			}
			else
			{
				//TODO: Should we somehow render an invalid character?
			}
			
			if (flowInfo != nullptr)
			{
				if (charTopLeft.x < flowInfo->extents.x)
				{
					flowInfo->extents.width += flowInfo->extents.x - charTopLeft.x;
					flowInfo->extents.x = charTopLeft.x;
				}
				if (charTopLeft.y < flowInfo->extents.y)
				{
					flowInfo->extents.height += flowInfo->extents.y - charTopLeft.y;
					flowInfo->extents.y = charTopLeft.y;
				}
				if (charBottomRight.x - flowInfo->extents.x > flowInfo->extents.width) { flowInfo->extents.width = charBottomRight.x - flowInfo->extents.x; }
				if (charBottomRight.y - flowInfo->extents.y > flowInfo->extents.height) { flowInfo->extents.height = charBottomRight.y - flowInfo->extents.y; }
			}
		}
		
		if (drawToScreen)
		{
			if (IsFlagSet(currentStyle, FontStyle_Underline) && underlineStart != drawPos)
			{
				RcDrawLine(underlineStart + underlineOffset, drawPos + underlineOffset, underlineThickness, currentColor);
			}
		}
		
		chunkStart += chunkLength;
		if (chunkStart < strLength)
		{
			//NOTE: We intentionally use lineHeight not currentSize and currentStyle so the lines
			//      accommadate the default font height at all times
			drawPos.x = position.x;
			drawPos.y += lineHeight;
		}
	}
	
	if (flowInfo != nullptr)
	{
		flowInfo->endPos = drawPos;
		flowInfo->totalSize = flowInfo->extents.size;
		flowInfo->extentRight = flowInfo->extents.x + flowInfo->extents.width - position.x;
		flowInfo->extentDown = flowInfo->extents.y + flowInfo->extents.height - position.y;
	}
	return drawPos;
}

v2 FontMeasureString(const char* strPntr, u32 strLength, NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	if (flowInfo == nullptr) { flowInfo = PushStruct(TempArena, FontFlowInfo_t); }
	FontPerformTextFlow(false, strPntr, strLength, Vec2_Zero, NewColor(Color_White), fontPntr, Alignment_Left, fontSize, styleFlags, maxWidth, flowInfo);
	return NewVec2(flowInfo->extentRight, flowInfo->extentDown);
}
v2 FontMeasureNtString(const char* nullTermString, NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	if (flowInfo == nullptr) { flowInfo = PushStruct(TempArena, FontFlowInfo_t); }
	FontPerformTextFlow(false, nullTermString, (u32)strlen(nullTermString), Vec2_Zero, NewColor(Color_White), fontPntr, Alignment_Left, fontSize, styleFlags, maxWidth, flowInfo);
	return NewVec2(flowInfo->extentRight, flowInfo->extentDown);
}

r32 FontMeasureLineWidth(const char* strPntr, u32 strLength, NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default, r32 maxWidth = 0)
{
	r32 result = maxWidth;
	FontMeasureLineWidth(strPntr, strLength, &result, fontPntr, fontSize, styleFlags);
	return result;
}
r32 FontMeasureNtLineWidth(const char* nullTermString, NewFont_t* fontPntr, r32 fontSize = 0, FontStyle_t styleFlags = FontStyle_Default, r32 maxWidth = 0)
{
	r32 result = maxWidth;
	FontMeasureLineWidth(nullTermString, (u32)strlen(nullTermString), &result, fontPntr, fontSize, styleFlags);
	return result;
}

// +--------------------------------------------------------------+
// |                Render Context Like Functions                 |
// +--------------------------------------------------------------+
v2 RcNewDrawString(const char* strPntr, u32 strLength, v2 position, Color_t color, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	Assert(renderContext->boundNewFont != nullptr);
	return FontPerformTextFlow(true, strPntr, strLength, position, color, renderContext->boundNewFont, renderContext->fontAlignment, renderContext->fontSize, renderContext->fontStyle, maxWidth, flowInfo);
}
v2 RcNewDrawNtString(const char* nullTermString, v2 position, Color_t color, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	Assert(renderContext->boundNewFont != nullptr);
	return FontPerformTextFlow(true, nullTermString, (u32)strlen(nullTermString), position, color, renderContext->boundNewFont, renderContext->fontAlignment, renderContext->fontSize, renderContext->fontStyle, maxWidth, flowInfo);
}

v2 RcMeasureString(const char* strPntr, u32 strLength, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	Assert(renderContext->boundNewFont != nullptr);
	if (flowInfo == nullptr) { flowInfo = PushStruct(TempArena, FontFlowInfo_t); }
	FontPerformTextFlow(false, strPntr, strLength, Vec2_Zero, NewColor(Color_White), renderContext->boundNewFont, renderContext->fontAlignment, renderContext->fontSize, renderContext->fontStyle, maxWidth, flowInfo);
	return NewVec2(flowInfo->extentRight, flowInfo->extentDown);
}
v2 RcMeasureNtString(const char* nullTermString, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	Assert(renderContext->boundNewFont != nullptr);
	if (flowInfo == nullptr) { flowInfo = PushStruct(TempArena, FontFlowInfo_t); }
	FontPerformTextFlow(false, nullTermString, (u32)strlen(nullTermString), Vec2_Zero, NewColor(Color_White), renderContext->boundNewFont, renderContext->fontAlignment, renderContext->fontSize, renderContext->fontStyle, maxWidth, flowInfo);
	return NewVec2(flowInfo->extentRight, flowInfo->extentDown);
}

r32 RcMeasureLineWidth(const char* strPntr, u32 strLength, r32 maxWidth = 0)
{
	Assert(renderContext->boundNewFont != nullptr);
	r32 result = maxWidth;
	FontMeasureLineWidth(strPntr, strLength, &result, renderContext->boundNewFont, renderContext->fontSize, renderContext->fontStyle);
	return result;
}
r32 RcMeasureNtLineWidth(const char* nullTermString, r32 maxWidth = 0)
{
	Assert(renderContext->boundNewFont != nullptr);
	r32 result = maxWidth;
	FontMeasureLineWidth(nullTermString, (u32)strlen(nullTermString), &result, renderContext->boundNewFont, renderContext->fontSize, renderContext->fontStyle);
	return result;
}

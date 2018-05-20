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
	r32 fontSize = 0, u16 styleFlags = FontStyle_Default)
{
	Assert(strPntr != nullptr);
	Assert(strLength > 0);
	Assert(fontPntr != nullptr);
	
	u16 currentStyle = styleFlags;
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
			currentStyle ^= FontStyle_Bold;
		}
		else if (nextChar == '\r')
		{
			currentStyle ^= FontStyle_Italic;
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
		else if (nextChar == '\v' && numCharsLeft >= 1) //Generic action
		{
			cIndex += 1;
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
	Alignment_t alignment = Alignment_Left, r32 fontSize = 0, u16 styleFlags = FontStyle_Default, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	StartTimeBlock("FontPerformTextFlow");
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
	u16 currentStyle = styleFlags;
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
		// DEBUG_PrintLine("Got chunk %u-%u/%u", chunkStart, chunkStart + chunkLength, strLength);
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
				currentStyle ^= FontStyle_Bold;
			}
			else if (nextChar == '\r') //Italic
			{
				currentStyle ^= FontStyle_Italic;
			}
			else if (nextChar == '\a') //Underline
			{
				if (IsFlagSet(currentStyle, FontStyle_Underline) && underlineStart != drawPos)
				{
					if (drawToScreen)
					{
						RcDrawLine(underlineStart + underlineOffset, drawPos + underlineOffset, underlineThickness, currentColor);
					}
				}
				else
				{
					underlineStart = drawPos;
				}
				currentStyle ^= FontStyle_Underline;
			}
			else if (nextChar == '\x01' && numCharsLeft >= 3) //Set Color
			{
				if (IsFlagSet(currentStyle, FontStyle_Underline) && underlineStart != drawPos)
				{
					if (drawToScreen)
					{
						RcDrawLine(underlineStart + underlineOffset, drawPos + underlineOffset, underlineThickness, currentColor);
					}
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
					if (drawToScreen)
					{
						RcDrawLine(underlineStart + underlineOffset, drawPos + underlineOffset, underlineThickness, currentColor);
					}
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
			else if (nextChar == '\v' && numCharsLeft >= 1) //Generic action
			{
				u8 cmd = (u8)chunkPntr[cIndex+1];
				u8 stylePart = (cmd & 0xFC);
				u8 actionBits = (cmd & 0x03);
				
				if (stylePart == 0xFE) //Everything
				{
					if (actionBits == 0x2) //Reset
					{
						currentStyle = styleFlags;
						currentColor = color;
					}
				}
				else if (stylePart == 0x10) //Wave
				{
					DEBUG_PrintLine("Wave! %X", actionBits);
					if (actionBits == 0x0) { FlagUnset(currentStyle,  FontStyle_Wave); }
					if (actionBits == 0x1) { FlagSet(currentStyle,    FontStyle_Wave); }
					if (actionBits == 0x2) { FlagToggle(currentStyle, FontStyle_Wave); }
				}
				else if (stylePart == 0x20) //Bubble
				{
					if (actionBits == 0x0) { FlagUnset(currentStyle,  FontStyle_Bubble); }
					if (actionBits == 0x1) { FlagSet(currentStyle,    FontStyle_Bubble); }
					if (actionBits == 0x2) { FlagToggle(currentStyle, FontStyle_Bubble); }
				}
				else if (stylePart == 0x30) //Bounce
				{
					if (actionBits == 0x0) { FlagUnset(currentStyle,  FontStyle_Bounce); }
					if (actionBits == 0x1) { FlagSet(currentStyle,    FontStyle_Bounce); }
					if (actionBits == 0x2) { FlagToggle(currentStyle, FontStyle_Bounce); }
				}
				else if (stylePart == 0x40) //Jitter
				{
					if (actionBits == 0x0) { FlagUnset(currentStyle,  FontStyle_Jitter); }
					if (actionBits == 0x1) { FlagSet(currentStyle,    FontStyle_Jitter); }
					if (actionBits == 0x2) { FlagToggle(currentStyle, FontStyle_Jitter); }
				}
				else if (stylePart == 0x50) //Wobble
				{
					if (actionBits == 0x0) { FlagUnset(currentStyle,  FontStyle_Wobble); }
					if (actionBits == 0x1) { FlagSet(currentStyle,    FontStyle_Wobble); }
					if (actionBits == 0x2) { FlagToggle(currentStyle, FontStyle_Wobble); }
				}
				else if (stylePart == 0x60) //Faded
				{
					if (actionBits == 0x0) { FlagUnset(currentStyle,  FontStyle_Faded); }
					if (actionBits == 0x1) { FlagSet(currentStyle,    FontStyle_Faded); }
					if (actionBits == 0x2) { FlagToggle(currentStyle, FontStyle_Faded); }
				}
				else if (stylePart == 0x70) //Rainbow
				{
					if (actionBits == 0x0) { FlagUnset(currentStyle,  FontStyle_Rainbow); }
					if (actionBits == 0x1) { FlagSet(currentStyle,    FontStyle_Rainbow); }
					if (actionBits == 0x2) { FlagToggle(currentStyle, FontStyle_Rainbow); }
				}
				else if (stylePart == 0x80) //Emboss
				{
					if (actionBits == 0x0) { FlagUnset(currentStyle,  FontStyle_Emboss); }
					if (actionBits == 0x1) { FlagSet(currentStyle,    FontStyle_Emboss); }
					if (actionBits == 0x2) { FlagToggle(currentStyle, FontStyle_Emboss); }
				}
				cIndex += 1;
			}
			else if (FontGetChar(fontPntr, &fontChar, nextChar, currentSize, currentStyle))
			{
				if (drawToScreen)
				{
					RcBindTexture(fontChar.texture);
					rec drawRec = NewRec(drawPos - fontChar.info->origin, fontChar.info->size);
					Color_t drawColor = currentColor;
					// +==============================+
					// |        FontStyle_Wave        |
					// +==============================+
					if (IsFlagSet(currentStyle, FontStyle_Wave))
					{
						drawRec.y += maxExtendDown/2 * SinR32((r32)((platform->programTime + cIndex*30)%800) / 800 * 2*Pi32);
					}
					// +==============================+
					// |       FontStyle_Bounce       |
					// +==============================+
					if (IsFlagSet(currentStyle, FontStyle_Bounce))
					{
						drawRec.y -= maxExtendDown/2 * AbsR32(SinR32((r32)((platform->programTime)%1600) / 1600 * 2*Pi32));
					}
					// +==============================+
					// |       FontStyle_Bubble       |
					// +==============================+
					if (IsFlagSet(currentStyle, FontStyle_Bubble))
					{
						r32 scale = 1.0f + 0.1f*SinR32((r32)((platform->programTime + cIndex*30)%800) / 800 * 2*Pi32);
						drawRec = RecInflateX(drawRec, drawRec.size.x * (scale - 1.0f));
						drawRec = RecInflateY(drawRec, drawRec.size.y * (scale - 1.0f));
					}
					// +==============================+
					// |      FontStyle_Rainbow       |
					// +==============================+
					if (IsFlagSet(currentStyle, FontStyle_Rainbow))
					{
						r32 hue = 180 + 180*SinR32((r32)((platform->programTime + cIndex*0)%4000) / 4000 * 2*Pi32);
						drawColor = ColorFromHSV((u16)RoundR32(hue), 0.7f, 1.0f);
					}
					// +==============================+
					// |       FontStyle_Jitter       |
					// +==============================+
					if (IsFlagSet(currentStyle, FontStyle_Jitter))
					{
						drawRec.x += (r32)RandI32(-1, 1);
						drawRec.y += (r32)RandI32(-1, 1);
					}
					// +==============================+
					// |       FontStyle_Faded        |
					// +==============================+
					if (IsFlagSet(currentStyle, FontStyle_Faded))
					{
						drawColor.a = (u8)(drawColor.a * 0.5f);
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
		
		if (IsFlagSet(currentStyle, FontStyle_Underline) && underlineStart != drawPos)
		{
			if (drawToScreen)
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
	EndTimeBlock();
	return drawPos;
}

v2 FontMeasureString(const char* strPntr, u32 strLength, NewFont_t* fontPntr, r32 fontSize = 0, u16 styleFlags = FontStyle_Default, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	if (flowInfo == nullptr) { flowInfo = PushStruct(TempArena, FontFlowInfo_t); }
	FontPerformTextFlow(false, strPntr, strLength, Vec2_Zero, NewColor(Color_White), fontPntr, Alignment_Left, fontSize, styleFlags, maxWidth, flowInfo);
	return NewVec2(flowInfo->extentRight, flowInfo->extentDown);
}
v2 FontMeasureNtString(const char* nullTermString, NewFont_t* fontPntr, r32 fontSize = 0, u16 styleFlags = FontStyle_Default, r32 maxWidth = 0, FontFlowInfo_t* flowInfo = nullptr)
{
	if (flowInfo == nullptr) { flowInfo = PushStruct(TempArena, FontFlowInfo_t); }
	FontPerformTextFlow(false, nullTermString, (u32)strlen(nullTermString), Vec2_Zero, NewColor(Color_White), fontPntr, Alignment_Left, fontSize, styleFlags, maxWidth, flowInfo);
	return NewVec2(flowInfo->extentRight, flowInfo->extentDown);
}

r32 FontMeasureLineWidth(const char* strPntr, u32 strLength, NewFont_t* fontPntr, r32 fontSize = 0, u16 styleFlags = FontStyle_Default, r32 maxWidth = 0)
{
	r32 result = maxWidth;
	FontMeasureLineWidth(strPntr, strLength, &result, fontPntr, fontSize, styleFlags);
	return result;
}
r32 FontMeasureNtLineWidth(const char* nullTermString, NewFont_t* fontPntr, r32 fontSize = 0, u16 styleFlags = FontStyle_Default, r32 maxWidth = 0)
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

void RcNewPrintStringFull(v2 position, Color_t color, r32 maxWidth, FontFlowInfo_t* flowInfo, const char* formatString, ...)
{
	char printBuffer[256] = {};
	va_list args;
	
	va_start(args, formatString);
	u32 length = (u32)vsnprintf(printBuffer, ArrayCount(printBuffer)-1, formatString, args);
	va_end(args);
	
	RcNewDrawString(printBuffer, length, position, color, maxWidth, flowInfo);
}
#define RcNewPrintString(position, color, formatString, ...) RcNewPrintStringFull(position, color, 0, nullptr, formatString, ##__VA_ARGS__)

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

r32 RcGetLineHeight()
{
	Assert(renderContext->boundNewFont != nullptr);
	return FontGetLineHeight(renderContext->boundNewFont, renderContext->fontSize, renderContext->fontStyle);
}
r32 RcGetMaxExtendUp()
{
	Assert(renderContext->boundNewFont != nullptr);
	return FontGetMaxExtendUp(renderContext->boundNewFont, renderContext->fontSize, renderContext->fontStyle);
}
r32 RcGetMaxExtendDown()
{
	Assert(renderContext->boundNewFont != nullptr);
	return FontGetMaxExtendDown(renderContext->boundNewFont, renderContext->fontSize, renderContext->fontStyle);
}

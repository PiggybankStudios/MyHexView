/*
File:   app_fontHelpers.cpp
Author: Taylor Robbins
Date:   03\03\2018
Description: 
	** Holds some functions that help us when dealing with Fonts
*/

inline u32 GetFontCharIndex(const Font_t* font, char character)
{
	u32 cIndex = 0;
	if (character >= font->firstChar && character - font->firstChar < font->numChars)
	{
		cIndex = character - font->firstChar;
	}
	else
	{
		cIndex = 127 - font->firstChar;
	}
	
	return cIndex;
}

inline v2 MeasureString(const Font_t* font, const char* string, u32 numChars)
{
	v2 currentPos = Vec2_Zero;
	for (u32 cIndex = 0; cIndex < numChars; cIndex++)
	{
		if (string[cIndex] == '\t')
		{
			u32 spaceIndex = GetFontCharIndex(font, ' ');
			currentPos.x += font->chars[spaceIndex].advanceX * TAB_WIDTH;
		}
		else if (IsCharClassPrintable(string[cIndex]) == false)
		{
			//Don't do anything
			u32 spaceIndex = GetFontCharIndex(font, ' ');
			currentPos.x += font->chars[spaceIndex].advanceX;
		}
		else
		{
			u32 charIndex = GetFontCharIndex(font, string[cIndex]);
			const FontCharInfo_t* charInfo = &font->chars[charIndex];
			currentPos.x += charInfo->advanceX;
		}
	}
	
	return NewVec2(currentPos.x, font->lineHeight);
}

inline v2 MeasureString(const Font_t* font, const char* nullTermString)
{
	return MeasureString(font, nullTermString, (u32)strlen(nullTermString));
}

inline i32 GetStringIndexForLocation(const Font_t* font, const char* inputStr, u32 inputStrLength, v2 relativePos)
{
	i32 result = (i32)inputStrLength;
	
	if (relativePos.y > font->lineHeight)
	{
		return result;
	}
	
	v2 lastStringSize = Vec2_Zero;
	for (u32 cIndex = 0; cIndex < inputStrLength; cIndex++)
	{
		v2 stringSize = MeasureString(font, inputStr, cIndex+1);
		if (stringSize.x > relativePos.x || cIndex == inputStrLength-1)
		{
			if (AbsR32(relativePos.x - lastStringSize.x) < AbsR32(relativePos.x - stringSize.x))
			{
				result = cIndex;
			}
			else
			{
				result = cIndex+1;
			}
			
			break;
		}
		
		lastStringSize = stringSize;
	}
	
	return result;
}

inline i32 GetStringIndexForLocation(const Font_t* font, const char* nullTermString, v2 relativePos)
{
	u32 strLength = (u32)strlen(nullTermString);
	return GetStringIndexForLocation(font, nullTermString, strLength, relativePos);
}

u32 FindNextFormatChunk(const Font_t* font, const char* string, u32 numCharacters, r32 maxWidth, bool preserveWords)
{
	r32 textWidth = 0;
	bool doBackwardsSearch = false;
	u32 cIndex;
	for (cIndex = 0; cIndex < numCharacters; cIndex++)
	{
		if (string[cIndex] == '\r' || string[cIndex] == '\n')
		{
			break;
		}
		else
		{
			r32 nextCharWidth = MeasureString(font, &string[cIndex], 1).x;
			
			if (textWidth + nextCharWidth > maxWidth)
			{
				doBackwardsSearch = preserveWords;
				break;
			}
			textWidth += nextCharWidth;
		}
	}
	
	if (doBackwardsSearch)
	{
		u32 searchStartIndex = cIndex;
		u32 wordBoundIndex = 0;
		bool foundSpace = false;
		for (cIndex = searchStartIndex; cIndex > 0; cIndex--)
		{
			char c = string[cIndex-1];
			if (IsCharClassWhitespace(c))
			{
				foundSpace = true;
				break;
			}
			else if (wordBoundIndex == 0 && IsCharClassAlphaNumeric(c) == false)
			{
				if (IsCharClassBeginningCharacter(c))
				{
					wordBoundIndex = cIndex;
				}
				else
				{
					wordBoundIndex = cIndex+1;
				}
			}
		}
		
		if (foundSpace)
		{
			return cIndex;
		}
		else if (wordBoundIndex != 0)
		{
			return wordBoundIndex-1;
		}
		else
		{
			return searchStartIndex;
		}
	}
	else
	{
		return cIndex;
	}
}

v2 MeasureFormattedString(const Font_t* font, const char* string, u32 numCharacters, r32 maxWidth, bool preserveWords)
{
	u32 cIndex = 0;
	
	r32 maxChunkWidth = 0;
	u32 numLines = 0;
	while (cIndex < numCharacters)
	{
		u32 numChars = FindNextFormatChunk(font, &string[cIndex], numCharacters - cIndex, maxWidth, preserveWords);
		if (numChars == 0) { numChars = 1; }
		
		while (numChars > 1 && IsCharClassWhitespace(string[cIndex + numChars-1]))
		{
			numChars--;
		}
		
		r32 chunkWidth = MeasureString(font, &string[cIndex], numChars).x;
		if (chunkWidth > maxChunkWidth) { maxChunkWidth = chunkWidth; }
		
		if (cIndex+numChars < numCharacters && string[cIndex+numChars] == '\r')
		{
			numChars++;
		}
		if (cIndex+numChars < numCharacters && string[cIndex+numChars] == '\n')
		{
			numChars++;
		}
		while (cIndex+numChars < numCharacters && string[cIndex+numChars] == ' ')
		{
			numChars++;
		}
		
		numLines++;
		cIndex += numChars;
	}
	
	return NewVec2(maxChunkWidth, numLines * font->lineHeight);
}

v2 MeasureFormattedString(const Font_t* font, const char* nullTermString, r32 maxWidth, bool preserveWords)
{
	u32 numCharacters = (u32)strlen(nullTermString);
	return MeasureFormattedString(font, nullTermString, numCharacters, maxWidth, preserveWords);
}

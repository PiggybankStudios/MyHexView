/*
File:   app_loadingFunctions.cpp
Author: Taylor Robbins
Date:   03\03\2018
Description: 
	** Holds functions that load and parse various resource types for the application
*/

VertexBuffer_t CreateVertexBuffer(const Vertex_t* vertices, u32 numVertices)
{
	VertexBuffer_t result = {};
	
	if (numVertices > 0)
	{
		result.numVertices = numVertices;
		result.filled = true;
		
		glGenBuffers(1, &result.id);
		glBindBuffer(GL_ARRAY_BUFFER, result.id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_t) * numVertices, vertices, GL_STATIC_DRAW);
	}
	
	return result;
}

void DestroyVertexBuffer(VertexBuffer_t* vertexBuffer)
{
	if (vertexBuffer->filled)
	{
		glDeleteBuffers(1, &vertexBuffer->id);
	}
	
	vertexBuffer->id = 0;
	vertexBuffer->filled = false;
	vertexBuffer->numVertices = 0;
}

FrameBuffer_t CreateFrameBuffer(const Texture_t* texture)
{
	FrameBuffer_t result = {};
	
	glGenFramebuffers(1, &result.id);
	glBindFramebuffer(GL_FRAMEBUFFER, result.id);
	
	glGenRenderbuffers(1, &result.depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result.depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texture->width, texture->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.depthBuffer);
	
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->id, 0);
	
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
	
	result.renderTexture = texture;
	
	return result;
}

Shader_t LoadShader(const char* vertShaderFileName, const char* fragShaderFileName)
{
	Shader_t result = {};
	GLint compiled;
	int logLength;
	char* logBuffer;
	
	Assert(true);
	
	FileInfo_t vertexShaderFile = platform->ReadEntireFile(vertShaderFileName);
	FileInfo_t fragmentShaderFile = platform->ReadEntireFile(fragShaderFileName);
	
	result.vertId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(result.vertId, 1, (const char* const*)&vertexShaderFile.content, NULL);
	glCompileShader(result.vertId);
	
	glGetShaderiv(result.vertId, GL_COMPILE_STATUS, &compiled);
	glGetShaderiv(result.vertId, GL_INFO_LOG_LENGTH, &logLength);
	DEBUG_PrintLine("%s: Compiled %s : %d byte log",
		vertShaderFileName, compiled ? "Successfully" : "Unsuccessfully", logLength);
	if (logLength > 0)
	{
		logBuffer = TempString(logLength+1);
		logBuffer[logLength] = '\0';
		glGetShaderInfoLog(result.vertId, logLength, NULL, logBuffer);
		DEBUG_PrintLine("Log: \"%s\"", logBuffer);
	}
	
	result.fragId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(result.fragId, 1, (const char* const*)&fragmentShaderFile.content, NULL);
	glCompileShader(result.fragId);
	
	glGetShaderiv(result.fragId, GL_COMPILE_STATUS, &compiled);
	glGetShaderiv(result.fragId, GL_INFO_LOG_LENGTH, &logLength);
	DEBUG_PrintLine("%s: Compiled %s : %d byte log",
		fragShaderFileName, compiled ? "Successfully" : "Unsuccessfully", logLength);
	if (logLength > 0)
	{
		logBuffer = TempString(logLength+1);
		logBuffer[logLength] = '\0';
		glGetShaderInfoLog(result.fragId, logLength, NULL, logBuffer);
		DEBUG_PrintLine("Log: \"%s\"", logBuffer);
	}
	
	platform->FreeFileMemory(&vertexShaderFile);
	platform->FreeFileMemory(&fragmentShaderFile);
	
	result.programId = glCreateProgram();
	glAttachShader(result.programId, result.fragId);
	glAttachShader(result.programId, result.vertId);
	glLinkProgram(result.programId);
	
	glGetProgramiv(result.programId, GL_LINK_STATUS, &compiled);
	glGetProgramiv(result.programId, GL_INFO_LOG_LENGTH, &logLength);
	DEBUG_PrintLine("Shader: Linked %s : %d byte log",
		compiled ? "Successfully" : "Unsuccessfully", logLength);
	if (logLength > 0)
	{
		logBuffer = TempString(logLength+1);
		logBuffer[logLength] = '\0';
		glGetProgramInfoLog(result.programId, logLength, NULL, logBuffer);
		DEBUG_PrintLine("Log: \"%s\"", logBuffer);
	}
	
	result.locations.positionAttrib      = glGetAttribLocation(result.programId, "inPosition");
	result.locations.colorAttrib         = glGetAttribLocation(result.programId, "inColor");
	result.locations.texCoordAttrib      = glGetAttribLocation(result.programId, "inTexCoord");
	
	result.locations.worldMatrix         = glGetUniformLocation(result.programId, "WorldMatrix");
	result.locations.viewMatrix          = glGetUniformLocation(result.programId, "ViewMatrix");
	result.locations.projectionMatrix    = glGetUniformLocation(result.programId, "ProjectionMatrix");
	result.locations.diffuseTexture      = glGetUniformLocation(result.programId, "DiffuseTexture");
	result.locations.alphaTexture        = glGetUniformLocation(result.programId, "AlphaTexture");
	result.locations.diffuseColor        = glGetUniformLocation(result.programId, "DiffuseColor");
	result.locations.secondaryColor      = glGetUniformLocation(result.programId, "SecondaryColor");
	result.locations.doGrayscaleGradient = glGetUniformLocation(result.programId, "DoGrayscaleGradient");
	result.locations.sourceRectangle     = glGetUniformLocation(result.programId, "SourceRectangle");
	result.locations.useAlphaTexture     = glGetUniformLocation(result.programId, "UseAlphaTexture");
	result.locations.textureSize         = glGetUniformLocation(result.programId, "TextureSize");
	result.locations.circleRadius        = glGetUniformLocation(result.programId, "CircleRadius");
	result.locations.circleInnerRadius   = glGetUniformLocation(result.programId, "CircleInnerRadius");
	
	glGenVertexArrays(1, &result.vertexArray);
	glBindVertexArray(result.vertexArray);
	glEnableVertexAttribArray(result.locations.positionAttrib);
	glEnableVertexAttribArray(result.locations.colorAttrib);
	glEnableVertexAttribArray(result.locations.texCoordAttrib);
	
	return result;
}

void DestroyShader(Shader_t* shaderPntr)
{
	Assert(shaderPntr != nullptr);
	
	glDeleteVertexArrays(1, &shaderPntr->vertexArray);
	glDeleteProgram(shaderPntr->programId);
	glDeleteShader(shaderPntr->vertId);
	glDeleteShader(shaderPntr->fragId);
	
	ClearPointer(shaderPntr);
}

Texture_t LoadTexture(const char* fileName, bool pixelated = false, bool repeat = true)
{
	Texture_t result = {};
	
	FileInfo_t textureFile = platform->ReadEntireFile(fileName);
	if (textureFile.content == nullptr)
	{
		textureFile = platform->ReadEntireFile(MISSING_TEXTURE_PATH);
		Assert(textureFile.content != nullptr);
	}
	
	i32 numChannels;
	i32 width, height;
	u8* imageData = stbi_load_from_memory(
		(u8*)textureFile.content, textureFile.size,
		&width, &height, &numChannels, 4);
	
	result = CreateTexture(imageData, width, height, pixelated, repeat);
	
	stbi_image_free(imageData);
	platform->FreeFileMemory(&textureFile);
	
	return result;
}

Texture_t LoadSprite(const char* fileName, bool pixelated = false, bool repeat = true)
{
	Texture_t result = {};
	
	FileInfo_t textureFile = platform->ReadEntireFile(fileName);
	if (textureFile.content == nullptr)
	{
		textureFile = platform->ReadEntireFile(MISSING_SPRITE_PATH);
		Assert(textureFile.content != nullptr);
	}
	
	i32 numChannels;
	i32 width, height;
	u8* imageData = stbi_load_from_memory(
		(u8*)textureFile.content, textureFile.size,
		&width, &height, &numChannels, 4);
	
	result = CreateTexture(imageData, width, height, pixelated, repeat);
	
	stbi_image_free(imageData);
	platform->FreeFileMemory(&textureFile);
	
	return result;
}

Font_t LoadFont(const char* fileName, 
	r32 fontSize, i32 bitmapWidth, i32 bitmapHeight,
	u8 firstCharacter, u8 numCharacters)
{
	Font_t result = {};
	
	FileInfo_t fontFile = platform->ReadEntireFile(fileName);
	
	result.numChars = numCharacters;
	result.firstChar = firstCharacter;
	result.fontSize = fontSize;
	
	u8* grayscaleData = PushArray(mainHeap, u8, bitmapWidth * bitmapHeight);
	stbtt_bakedchar* charInfos = PushArray(mainHeap, stbtt_bakedchar, numCharacters);
	
	int bakeResult = stbtt_BakeFontBitmap((u8*)fontFile.content, 
		0, fontSize,
		grayscaleData, bitmapWidth, bitmapHeight, 
		firstCharacter, numCharacters, charInfos);
	DEBUG_PrintLine("STB Bake Result: %d", bakeResult);
	
	for (u8 cIndex = 0; cIndex < numCharacters; cIndex++)
	{
		result.chars[cIndex].position = NewVec2i(
			charInfos[cIndex].x0, 
			charInfos[cIndex].y0);
		result.chars[cIndex].size = NewVec2i(
			charInfos[cIndex].x1 - charInfos[cIndex].x0, 
			charInfos[cIndex].y1 - charInfos[cIndex].y0);
		result.chars[cIndex].offset = NewVec2(
			charInfos[cIndex].xoff, 
			charInfos[cIndex].yoff);
		result.chars[cIndex].advanceX = charInfos[cIndex].xadvance;
	}
	
	u8* bitmapData = PushArray(mainHeap, u8, 4 * bitmapWidth * bitmapHeight);
	
	for (i32 y = 0; y < bitmapHeight; y++)
	{
		for (i32 x = 0; x < bitmapWidth; x++)
		{
			u8 grayscaleValue = grayscaleData[y*bitmapWidth + x];
			
			bitmapData[(y*bitmapWidth+x)*4 + 0] = 255;
			bitmapData[(y*bitmapWidth+x)*4 + 1] = 255;
			bitmapData[(y*bitmapWidth+x)*4 + 2] = 255;
			bitmapData[(y*bitmapWidth+x)*4 + 3] = grayscaleValue; 
		}
	}
	
	result.bitmap = CreateTexture(bitmapData, bitmapWidth, bitmapHeight);
	
	ArenaPop(mainHeap, grayscaleData);
	ArenaPop(mainHeap, charInfos);
	ArenaPop(mainHeap, bitmapData);
	platform->FreeFileMemory(&fontFile);
	
	//Create information about character sizes
	{
		v2 maxSize = Vec2_Zero;
		v2 extendVertical = Vec2_Zero;
		for (u32 cIndex = 0; cIndex < result.numChars; cIndex++)
		{
			FontCharInfo_t* charInfo = &result.chars[cIndex];
			
			if (charInfo->height > maxSize.y)
				maxSize.y = (r32)charInfo->height;
			
			if (-charInfo->offset.y > extendVertical.x)
				extendVertical.x = -charInfo->offset.y;
			
			if (charInfo->offset.y + charInfo->height > extendVertical.y)
				extendVertical.y = charInfo->offset.y + charInfo->height;
			
			if (charInfo->advanceX > maxSize.x)
				maxSize.x = charInfo->advanceX;
		}
		
		result.maxCharWidth = maxSize.x;
		result.maxCharHeight = maxSize.y;
		result.maxExtendUp = extendVertical.x;
		result.maxExtendDown = extendVertical.y;
		result.lineHeight = result.maxExtendDown + result.maxExtendUp;
	}
	
	return result;
}

void DestroyFont(Font_t* fontPntr)
{
	Assert(fontPntr != nullptr);
	
	DestroyTexture(&fontPntr->bitmap);
	
	ClearPointer(fontPntr);
}

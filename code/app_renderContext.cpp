/*
File:   app_renderContext.cpp
Author: Taylor Robbins
Date:   03\03\2018
Description: 
	** The renderContext is a helpful wrapper of most OpenGL calls into an interface
	** that allows us to draw sprites, rectangles, text, and other primitives easily
*/

void InitializeRenderContext()
{
	Assert(renderContext != nullptr);
	ClearPointer(renderContext);
	
	Vertex_t squareVertices[] =
	{
		{  {0.0f, 0.0f, 0.0f}, NewVec4(NewColor(Color_White)), {0.0f, 0.0f} },
		{  {1.0f, 0.0f, 0.0f}, NewVec4(NewColor(Color_White)), {1.0f, 0.0f} },
		{  {0.0f, 1.0f, 0.0f}, NewVec4(NewColor(Color_White)), {0.0f, 1.0f} },
		
		{  {0.0f, 1.0f, 0.0f}, NewVec4(NewColor(Color_White)), {0.0f, 1.0f} },
		{  {1.0f, 0.0f, 0.0f}, NewVec4(NewColor(Color_White)), {1.0f, 0.0f} },
		{  {1.0f, 1.0f, 0.0f}, NewVec4(NewColor(Color_White)), {1.0f, 1.0f} },
	};
	renderContext->squareBuffer = CreateVertexBuffer(squareVertices, ArrayCount(squareVertices));
	
	renderContext->gradientTexture = LoadTexture("Resources/Textures/gradient.png", false, false);
	renderContext->circleTexture = LoadTexture("Resources/Sprites/circle.png", false, false);
	
	Color_t textureData = {Color_White};
	renderContext->dotTexture = CreateTexture((u8*)&textureData, 1, 1);
	
	renderContext->viewport = NewRec(0, 0, (r32)RenderScreenSize.x, (r32)RenderScreenSize.y);
	renderContext->worldMatrix = Mat4_Identity;
	renderContext->viewMatrix = Mat4_Identity;
	renderContext->projectionMatrix = Mat4_Identity;
	renderContext->doGrayscaleGradient = false;
	renderContext->useAlphaTexture = false;
	renderContext->sourceRectangle = NewRec(0, 0, 1, 1);
	renderContext->depth = 1.0f;
	renderContext->circleRadius = 0.0f;
	renderContext->circleInnerRadius = 0.0f;
	renderContext->color = NewColor(Color_White);
	renderContext->secondaryColor = NewColor(Color_White);
}


// +--------------------------------------------------------------+
// |                    State Change Functions                    |
// +--------------------------------------------------------------+
void RcUpdateShader()
{
	Assert(renderContext->boundShader != nullptr);
	
	if (renderContext->boundBuffer != nullptr)
	{
		glBindVertexArray(renderContext->boundShader->vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, renderContext->boundBuffer->id);
		glVertexAttribPointer(renderContext->boundShader->locations.positionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)0);
		glVertexAttribPointer(renderContext->boundShader->locations.colorAttrib,    4, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)sizeof(v3));
		glVertexAttribPointer(renderContext->boundShader->locations.texCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)(sizeof(v3)+sizeof(v4)));
	}
	
	if (renderContext->boundTexture != nullptr)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderContext->boundTexture->id);
		glUniform1i(renderContext->boundShader->locations.diffuseTexture, 0);
		glUniform2f(renderContext->boundShader->locations.textureSize, (r32)renderContext->boundTexture->width, (r32)renderContext->boundTexture->height);
	}
	
	if (renderContext->boundAlphaTexture != nullptr)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderContext->boundAlphaTexture->id);
		glUniform1i(renderContext->boundShader->locations.alphaTexture, 1);
		glUniform1i(renderContext->boundShader->locations.useAlphaTexture, 1);
	}
	else
	{
		glUniform1i(renderContext->boundShader->locations.useAlphaTexture, 0);
	}
	
	if (renderContext->boundFrameBuffer != nullptr)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, renderContext->boundFrameBuffer->id);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	glUniformMatrix4fv(renderContext->boundShader->locations.worldMatrix,      1, GL_FALSE, &renderContext->worldMatrix.values[0][0]);
	glUniformMatrix4fv(renderContext->boundShader->locations.viewMatrix,       1, GL_FALSE, &renderContext->viewMatrix.values[0][0]);
	glUniformMatrix4fv(renderContext->boundShader->locations.projectionMatrix, 1, GL_FALSE, &renderContext->projectionMatrix.values[0][0]);
	
	glUniform1i(renderContext->boundShader->locations.doGrayscaleGradient, renderContext->doGrayscaleGradient ? 1 : 0);
	glUniform4f(renderContext->boundShader->locations.sourceRectangle, renderContext->sourceRectangle.x, renderContext->sourceRectangle.y, renderContext->sourceRectangle.width, renderContext->sourceRectangle.height);
	glUniform1f(renderContext->boundShader->locations.circleRadius, renderContext->circleRadius);
	glUniform1f(renderContext->boundShader->locations.circleInnerRadius, renderContext->circleInnerRadius);
	
	v4 colorVec = NewVec4(renderContext->color);
	glUniform4f(renderContext->boundShader->locations.diffuseColor, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
	colorVec = NewVec4(renderContext->secondaryColor);
	glUniform4f(renderContext->boundShader->locations.secondaryColor, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
}

void RcBindShader(const Shader_t* shaderPntr)
{
	renderContext->boundShader = shaderPntr;
	
	glUseProgram(shaderPntr->programId);
}

void RcBindFont(const Font_t* fontPntr)
{
	renderContext->boundFont = fontPntr;
}

void RcBindTexture(const Texture_t* texturePntr)
{
	renderContext->boundTexture = texturePntr;
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderContext->boundTexture->id);
	glUniform1i(renderContext->boundShader->locations.diffuseTexture, 0);
	glUniform2f(renderContext->boundShader->locations.textureSize, (r32)renderContext->boundTexture->width, (r32)renderContext->boundTexture->height);
}

void RcBindAlphaTexture(const Texture_t* texturePntr)
{
	renderContext->boundAlphaTexture = texturePntr;
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderContext->boundAlphaTexture->id);
	glUniform1i(renderContext->boundShader->locations.alphaTexture, 1);
	glUniform1i(renderContext->boundShader->locations.useAlphaTexture, 1);
}
void RcDisableAlphaTexture()
{
	renderContext->boundAlphaTexture = nullptr;
	
	glUniform1i(renderContext->boundShader->locations.useAlphaTexture, 0);
}

void RcBindBuffer(const VertexBuffer_t* vertBufferPntr)
{
	renderContext->boundBuffer = vertBufferPntr;
	
	glBindVertexArray(renderContext->boundShader->vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertBufferPntr->id);
	glVertexAttribPointer(renderContext->boundShader->locations.positionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)0);
	glVertexAttribPointer(renderContext->boundShader->locations.colorAttrib,    4, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)sizeof(v3));
	glVertexAttribPointer(renderContext->boundShader->locations.texCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)(sizeof(v3)+sizeof(v4)));
}

void RcBindFrameBuffer(const FrameBuffer_t* frameBuffer)
{
	renderContext->boundFrameBuffer = frameBuffer;
	
	if (frameBuffer == nullptr)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->id);
	}
}

void RcSetWorldMatrix(const mat4& worldMatrix)
{
	renderContext->worldMatrix = worldMatrix;
	glUniformMatrix4fv(renderContext->boundShader->locations.worldMatrix, 1, GL_FALSE, &worldMatrix.values[0][0]);
}
void RcSetViewMatrix(const mat4& viewMatrix)
{
	renderContext->viewMatrix = viewMatrix;
	glUniformMatrix4fv(renderContext->boundShader->locations.viewMatrix, 1, GL_FALSE, &viewMatrix.values[0][0]);
}
void RcSetProjectionMatrix(const mat4& projectionMatrix)
{
	renderContext->projectionMatrix = projectionMatrix;
	glUniformMatrix4fv(renderContext->boundShader->locations.projectionMatrix, 1, GL_FALSE, &projectionMatrix.values[0][0]);
}

void RcSetViewport(rec viewport)
{
	renderContext->viewport = viewport;
	
	#if DOUBLE_RESOLUTION
	glViewport(
		(i32)renderContext->viewport.x*2, 
		(i32)(RenderScreenSize.y*2 - renderContext->viewport.height*2 - renderContext->viewport.y*2), 
		(i32)renderContext->viewport.width*2, 
		(i32)renderContext->viewport.height*2
	);
	#else
	glViewport(
		(i32)renderContext->viewport.x, 
		(i32)(RenderScreenSize.y - renderContext->viewport.height - renderContext->viewport.y), 
		(i32)renderContext->viewport.width, 
		(i32)renderContext->viewport.height
	);
	#endif
	
	mat4 projMatrix;
	projMatrix = Mat4Scale(NewVec3(2.0f/viewport.width, -2.0f/viewport.height, 1.0f));
	projMatrix = Mat4Multiply(projMatrix, Mat4Translate(NewVec3(-viewport.width/2.0f, -viewport.height/2.0f, 0.0f)));
	projMatrix = Mat4Multiply(projMatrix, Mat4Translate(NewVec3(-renderContext->viewport.x, -renderContext->viewport.y, 0.0f)));
	RcSetProjectionMatrix(projMatrix);
}

void RcSetColor(Color_t color)
{
	renderContext->color = color;
	
	v4 colorVec = NewVec4(color);
	glUniform4f(renderContext->boundShader->locations.diffuseColor, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
}
void RcSetSecondaryColor(Color_t color)
{
	renderContext->secondaryColor = color;
	
	v4 colorVec = NewVec4(color);
	glUniform4f(renderContext->boundShader->locations.secondaryColor, colorVec.r, colorVec.g, colorVec.b, colorVec.a);
}

void RcSetSourceRectangle(rec sourceRectangle)
{
	renderContext->sourceRectangle = sourceRectangle;
	
	glUniform4f(renderContext->boundShader->locations.sourceRectangle, sourceRectangle.x, sourceRectangle.y, sourceRectangle.width, sourceRectangle.height);
}

void RcSetGradientEnabled(bool doGradient)
{
	renderContext->doGrayscaleGradient = doGradient;
	
	glUniform1i(renderContext->boundShader->locations.doGrayscaleGradient, doGradient ? 1 : 0);
}

void RcSetCircleRadius(r32 radius, r32 innerRadius = 0.0f)
{
	renderContext->circleRadius = radius;
	renderContext->circleInnerRadius = innerRadius;
	
	glUniform1f(renderContext->boundShader->locations.circleRadius, radius);
	glUniform1f(renderContext->boundShader->locations.circleInnerRadius, innerRadius);
}

void RcSetDepth(r32 depth)
{
	renderContext->depth = depth;
}

void RcBegin(const Shader_t* startShader, const Font_t* startFont, rec viewport)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.1f);
	
	RcBindShader(startShader);
	RcBindFont(startFont);
	RcBindTexture(&renderContext->dotTexture);
	RcBindBuffer(&renderContext->squareBuffer);
	RcBindFrameBuffer(nullptr);
	RcDisableAlphaTexture();
	
	RcSetWorldMatrix(Matrix4_Identity);
	RcSetViewMatrix(Matrix4_Identity);
	RcSetProjectionMatrix(Matrix4_Identity);
	RcSetViewport(viewport);
	RcSetColor(NewColor(Color_White));
	RcSetSecondaryColor(NewColor(Color_White));
	RcSetSourceRectangle(NewRec(0, 0, 1, 1));
	RcSetGradientEnabled(false);
	RcSetCircleRadius(0.0f, 0.0f);
	RcSetDepth(1.0f);
}

// +--------------------------------------------------------------+
// |                      Drawing Functions                       |
// +--------------------------------------------------------------+
void RcClearColorBuffer(Color_t clearColor)
{
	v4 clearColorVec = NewVec4(clearColor);
	glClearColor(clearColorVec.x, clearColorVec.y, clearColorVec.z, clearColorVec.w);
	glClear(GL_COLOR_BUFFER_BIT);
}
void RcClearDepthBuffer(r32 clearDepth)
{
	glClearDepth(clearDepth);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void RcDrawTexturedRec(rec rectangle, Color_t color)
{
	RcBindTexture(renderContext->boundTexture);
	RcSetSourceRectangle(NewRec(0, 0, 1, 1));
	RcSetColor(color);
	mat4 worldMatrix = Mat4Multiply(
		Mat4Translate(NewVec3(rectangle.x, rectangle.y, renderContext->depth)), //Position
		Mat4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	RcSetWorldMatrix(worldMatrix);
	RcBindBuffer(&renderContext->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderContext->squareBuffer.numVertices);
}
void RcDrawTexturedRec(rec rectangle, Color_t color, rec sourceRectangle)
{
	rec realSourceRec = NewRec(
		sourceRectangle.x / (r32)renderContext->boundTexture->width,
		sourceRectangle.y / (r32)renderContext->boundTexture->height,
		sourceRectangle.width / (r32)renderContext->boundTexture->width,
		sourceRectangle.height / (r32)renderContext->boundTexture->height);
	RcSetSourceRectangle(realSourceRec);
	RcBindTexture(renderContext->boundTexture);
	RcSetColor(color);
	mat4 worldMatrix = Mat4Multiply(
		Mat4Translate(NewVec3(rectangle.x, rectangle.y, renderContext->depth)), //Position
		Mat4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	RcSetWorldMatrix(worldMatrix);
	RcBindBuffer(&renderContext->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderContext->squareBuffer.numVertices);
}

void RcDrawTexture(const Texture_t* texturePntr, v2 topLeft, r32 scale, Color_t color, rec sourceRectangle)
{
	RcBindTexture(texturePntr);
	rec realSourceRec = NewRec(
		sourceRectangle.x / (r32)renderContext->boundTexture->width,
		sourceRectangle.y / (r32)renderContext->boundTexture->height,
		sourceRectangle.width / (r32)renderContext->boundTexture->width,
		sourceRectangle.height / (r32)renderContext->boundTexture->height);
	RcSetSourceRectangle(realSourceRec);
	RcBindTexture(renderContext->boundTexture);
	RcSetColor(color);
	mat4 worldMatrix = Mat4Multiply(
		Mat4Translate(NewVec3(topLeft.x, topLeft.y, renderContext->depth)), //Position
		Mat4Scale(NewVec3(renderContext->boundTexture->width * scale, renderContext->boundTexture->height * scale, 1.0f))); //Scale
	RcSetWorldMatrix(worldMatrix);
	RcBindBuffer(&renderContext->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderContext->squareBuffer.numVertices);
}
void RcDrawTexture(const Texture_t* texturePntr, v2 topLeft, r32 scale, Color_t color)
{
	RcDrawTexture(texturePntr, topLeft, scale, color, NewRec(0, 0, (r32)texturePntr->width, (r32)texturePntr->height));
}
void RcDrawTexture(const Texture_t* texturePntr, v2 topLeft, r32 scale)
{
	RcDrawTexture(texturePntr, topLeft, scale, NewColor(Color_White), NewRec(0, 0, (r32)texturePntr->width, (r32)texturePntr->height));
}
void RcDrawTexture(const Texture_t* texturePntr, v2 topLeft)
{
	RcDrawTexture(texturePntr, topLeft, 1.0f, NewColor(Color_White), NewRec(0, 0, (r32)texturePntr->width, (r32)texturePntr->height));
}

void RcDrawRectangle(rec rectangle, Color_t color)
{
	RcBindTexture(&renderContext->dotTexture);
	RcSetSourceRectangle(NewRec(0, 0, 1, 1));
	RcSetColor(color);
	mat4 worldMatrix = Mat4Multiply(
		Mat4Translate(NewVec3(rectangle.x, rectangle.y, renderContext->depth)), //Position
		Mat4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f))); //Scale
	RcSetWorldMatrix(worldMatrix);
	RcBindBuffer(&renderContext->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderContext->squareBuffer.numVertices);
}

void RcDrawButton(rec rectangle, Color_t backgroundColor, Color_t borderColor, r32 borderWidth = 1.0f)
{
	RcDrawRectangle(rectangle, backgroundColor);
	
	RcDrawRectangle(NewRec(rectangle.x, rectangle.y, rectangle.width, borderWidth), borderColor);
	RcDrawRectangle(NewRec(rectangle.x, rectangle.y, borderWidth, rectangle.height), borderColor);
	RcDrawRectangle(NewRec(rectangle.x, rectangle.y + rectangle.height - borderWidth, rectangle.width, borderWidth), borderColor);
	RcDrawRectangle(NewRec(rectangle.x + rectangle.width - borderWidth, rectangle.y, borderWidth, rectangle.height), borderColor);
}

void RcDrawGradient(rec rectangle, Color_t color1, Color_t color2, Dir2_t direction)
{
	RcBindTexture(&renderContext->gradientTexture);
	RcSetSourceRectangle(NewRec(0, 0, 1, 1));
	RcSetColor(color1);
	RcSetSecondaryColor(color2);
	RcSetGradientEnabled(true);
	
	mat4 worldMatrix = Mat4_Identity;
	switch (direction)
	{
		case Dir2_Right:
		default:
		{
			worldMatrix = Mat4Multiply(
				Mat4Translate(NewVec3(rectangle.x, rectangle.y, renderContext->depth)),
				Mat4Scale(NewVec3(rectangle.width, rectangle.height, 1.0f)));
		} break;
		
		case Dir2_Left:
		{
			worldMatrix = Mat4Multiply(
				Mat4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y, renderContext->depth)),
				Mat4Scale(NewVec3(-rectangle.width, rectangle.height, 1.0f)));
		} break;
		
		case Dir2_Down:
		{
			worldMatrix = Mat4Multiply(
				Mat4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y, renderContext->depth)),
				Mat4RotateZ(ToRadians(90)),
				Mat4Scale(NewVec3(rectangle.height, rectangle.width, 1.0f)));
		} break;
		
		case Dir2_Up:
		{
			worldMatrix = Mat4Multiply(
				Mat4Translate(NewVec3(rectangle.x + rectangle.width, rectangle.y + rectangle.height, renderContext->depth)),
				Mat4RotateZ(ToRadians(90)),
				Mat4Scale(NewVec3(-rectangle.height, rectangle.width, 1.0f)));
		} break;
	};
	RcSetWorldMatrix(worldMatrix);
	
	RcBindBuffer(&renderContext->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderContext->squareBuffer.numVertices);
	
	RcSetGradientEnabled(false);
}

void RcDrawLine(v2 p1, v2 p2, r32 thickness, Color_t color)
{
	RcBindTexture(&renderContext->dotTexture);
	RcSetSourceRectangle(NewRec(0, 0, 1, 1));
	RcSetColor(color);
	r32 length = Vec2Length(p2 - p1);
	r32 rotation = AtanR32(p2.y - p1.y, p2.x - p1.x); 
	mat4 worldMatrix = Mat4_Identity;
	worldMatrix = Mat4Multiply(Mat4Translate(NewVec3(0.0f, -0.5f, 0.0f)),              worldMatrix); //Centering
	worldMatrix = Mat4Multiply(Mat4Scale(NewVec3(length, thickness, 1.0f)),            worldMatrix); //Scale
	worldMatrix = Mat4Multiply(Mat4RotateZ(rotation),                                  worldMatrix); //Rotation
	worldMatrix = Mat4Multiply(Mat4Translate(NewVec3(p1.x, p1.y, renderContext->depth)), worldMatrix); //Position
	RcSetWorldMatrix(worldMatrix);
	RcBindBuffer(&renderContext->squareBuffer);
	glDrawArrays(GL_TRIANGLES, 0, renderContext->squareBuffer.numVertices);
}

void RcDrawCircle(v2 center, r32 radius, Color_t color)
{
	RcSetCircleRadius(1.0f, 0.0f);
	RcDrawRectangle(NewRec(center.x - radius, center.y - radius, radius*2, radius*2), color);
	RcSetCircleRadius(0.0f, 0.0f);
}

void RcDrawDonut(v2 center, r32 radius, r32 innerRadius, Color_t color)
{
	r32 realInnerRadius = ClampR32(innerRadius / radius, 0.0f, 1.0f);
	RcSetCircleRadius(1.0f, realInnerRadius);
	RcDrawRectangle(NewRec(center.x - radius, center.y - radius, radius*2, radius*2), color);
	RcSetCircleRadius(0.0f, 0.0f);
}

void RcDrawCharacter(u32 charIndex, v2 bottomLeft, Color_t color, r32 scale = 1.0f)
{
	const FontCharInfo_t* charInfo = &renderContext->boundFont->chars[charIndex];
	
	rec sourceRectangle = NewRec((r32)charInfo->x, (r32)charInfo->y, (r32)charInfo->width, (r32)charInfo->height);
	rec drawRectangle = NewRec(
		bottomLeft.x + scale*charInfo->offset.x, 
		bottomLeft.y + scale*charInfo->offset.y, 
		scale*charInfo->width, 
		scale*charInfo->height);
	
	if (renderContext->boundTexture != &renderContext->boundFont->bitmap)
	{
		RcBindTexture(&renderContext->boundFont->bitmap);
	}
	
	RcDrawTexturedRec(drawRectangle, color, sourceRectangle);
}

void RcDrawHexCharacter(u8 hexValue, v2 bottomLeft, Color_t color, r32 scale = 1.0f)
{
	const Font_t* boundFont = renderContext->boundFont;
	const FontCharInfo_t* spaceCharInfo = &boundFont->chars[GetFontCharIndex(boundFont, ' ')];
	
	rec charRec = NewRec(bottomLeft.x, bottomLeft.y + boundFont->maxExtendDown*scale - (boundFont->lineHeight-1)*scale, spaceCharInfo->advanceX*scale, (boundFont->lineHeight-1)*scale);
	charRec.x = (r32)RoundR32(charRec.x);
	charRec.y = (r32)RoundR32(charRec.y);
	RcDrawRectangle(charRec, color);
	RcDrawRectangle(NewRec(charRec.x, charRec.y, 1, charRec.height), NewColor(Color_Black));
	// RcDrawButton(charRec, NewColor(Color_TransparentBlack), color, 1.0f);
	
	r32 innerCharScale = scale*5/8;
	char upperHexChar = UpperHexChar(hexValue);
	char lowerHexChar = LowerHexChar(hexValue);
	u32 upperCharIndex = GetFontCharIndex(boundFont, upperHexChar);
	const FontCharInfo_t* upperCharInfo = &boundFont->chars[upperCharIndex];
	u32 lowerCharIndex = GetFontCharIndex(boundFont, lowerHexChar);
	const FontCharInfo_t* lowerCharInfo = &boundFont->chars[lowerCharIndex];
	
	v2 charPosUpper = charRec.topLeft + NewVec2(1, upperCharInfo->height*innerCharScale + 1);
	v2 charPosLower = charRec.topLeft + NewVec2(charRec.width - lowerCharInfo->width*innerCharScale - 1, charRec.height - 1);
	// RcDrawCharacter(upperCharIndex, charPosUpper, color, innerCharScale);
	// RcDrawCharacter(lowerCharIndex, charPosLower, color, innerCharScale);
	RcDrawCharacter(upperCharIndex, charPosUpper, NewColor(Color_Black), innerCharScale);
	RcDrawCharacter(lowerCharIndex, charPosLower, NewColor(Color_Black), innerCharScale);
}

void RcDrawString(const char* string, u32 numCharacters, v2 position, Color_t color, r32 scale = 1.0f, Alignment_t alignment = Alignment_Left)
{
	RcBindTexture(&renderContext->boundFont->bitmap);
	
	v2 stringSize = MeasureString(renderContext->boundFont, string, numCharacters);
	
	v2 currentPos = position;
	switch (alignment)
	{
		case Alignment_Center: currentPos.x -= stringSize.x/2; break;
		case Alignment_Right:  currentPos.x -= stringSize.x; break;
		case Alignment_Left:   break;
	};
	
	for (u32 cIndex = 0; cIndex < numCharacters; cIndex++)
	{
		if (string[cIndex] == '\t')
		{
			u32 spaceIndex = GetFontCharIndex(renderContext->boundFont, ' ');
			currentPos.x += renderContext->boundFont->chars[spaceIndex].advanceX * TAB_WIDTH * scale;
		}
		else if (IsCharClassPrintable(string[cIndex]) == false)
		{
			//Draw
			RcDrawHexCharacter(string[cIndex], currentPos, color, scale);
			u32 spaceIndex = GetFontCharIndex(renderContext->boundFont, ' ');
			currentPos.x += renderContext->boundFont->chars[spaceIndex].advanceX * scale;
		}
		else
		{
			u32 charIndex = GetFontCharIndex(renderContext->boundFont, string[cIndex]);
			RcDrawCharacter(charIndex, currentPos, color, scale);
			currentPos.x += renderContext->boundFont->chars[charIndex].advanceX * scale;
		}
	}
}

void RcDrawString(const char* nullTermString, v2 position, Color_t color, r32 scale = 1.0f, Alignment_t alignment = Alignment_Left)
{
	RcDrawString(nullTermString, (u32)strlen(nullTermString), position, color, scale, alignment);
}

void RcPrintString(v2 position, Color_t color, r32 scale, const char* formatString, ...)
{
	char printBuffer[256] = {};
	va_list args;
	
	va_start(args, formatString);
	u32 length = (u32)vsnprintf(printBuffer, 256-1, formatString, args);
	va_end(args);
	
	RcDrawString(printBuffer, length, position, color, scale);
}

void RcDrawFormattedString(const char* string, u32 numCharacters, v2 position, r32 maxWidth, Color_t color, Alignment_t alignment = Alignment_Left, bool preserveWords = true)
{
	u32 cIndex = 0;
	v2 drawPos = position;
	while (cIndex < numCharacters)
	{
		u32 numChars = FindNextFormatChunk(renderContext->boundFont, &string[cIndex], numCharacters - cIndex, maxWidth, preserveWords);
		if (numChars == 0) { numChars = 1; }
		
		while (numChars > 1 && IsCharClassWhitespace(string[cIndex + numChars-1]))
		{
			numChars--;
		}
		
		RcDrawString(&string[cIndex], numChars, drawPos, color, 1.0f, alignment);
		
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
		drawPos.y += renderContext->boundFont->lineHeight;
		cIndex += numChars;
	}
}

void RcDrawFormattedString(const char* nullTermString, v2 position, r32 maxWidth, Color_t color, Alignment_t alignment = Alignment_Left, bool preserveWords = true)
{
	u32 numCharacters = (u32)strlen(nullTermString);
	RcDrawFormattedString(nullTermString, numCharacters, position, maxWidth, color, alignment, preserveWords);
}

void RcDrawLineArrow(v2 start, v2 end, r32 wingSize, r32 thickness, Color_t color)
{
	RcDrawLine(start, end, thickness, color);
	r32 wingAngle = AtanR32(end.y - start.y, end.x - start.x) + (Pi32 * 3/4);
	v2 wingVec = NewVec2(CosR32(wingAngle), SinR32(wingAngle));
	RcDrawLine(end, end + wingVec*wingSize, thickness, color);
	wingVec = Vec2PerpRight(wingVec);
	RcDrawLine(end, end + wingVec*wingSize, thickness, color);
}


inline r32 LinearFormula(r32 a, r32 b, r32 t)
{
	return a + (b-a)*t;
}

inline r32 QuadFormula(r32 a, r32 b, r32 c, r32 t)
{
	return a - 2*a*t + 2*b*t + a*t*t - 2*b*t*t + c*t*t;
	// return LinearFormula(LinearFormula(a, b, t), LinearFormula(b, c, t), t);
}

inline v2 QuadFormula2d(v2 a, v2 b, v2 c, r32 t)
{
	return NewVec2(QuadFormula(a.x, b.x, c.x, t), QuadFormula(a.y, b.y, c.y, t));
}

void RcDrawQuadCurve(v2 p1, v2 c1, v2 p2, u32 numLines, r32 thickness, Color_t color)
{
	for (u32 lIndex = 0; lIndex < numLines; lIndex++)
	{
		r32 time1 = (r32)lIndex * (1.0f / (r32)numLines);
		r32 time2 = (r32)(lIndex+1) * (1.0f / (r32)numLines);
		v2 pos1 = QuadFormula2d(p1, c1, p2, time1);
		v2 pos2 = QuadFormula2d(p1, c1, p2, time2);
		RcDrawLine(pos1, pos2, thickness, color);
	}
}

inline r32 CubicFormula(r32 a, r32 b, r32 c, r32 d, r32 t)
{
	return a - 3*a*t + 3*b*t + 3*a*t*t - 6*b*t*t + 3*c*t*t - a*t*t*t + 3*b*t*t*t - 3*c*t*t*t + d*t*t*t;
	// return LinearFormula(QuadFormula(a, b, c, t), QuadFormula(b, c, d, t), t);
}

inline v2 CubicFormula2d(v2 a, v2 b, v2 c, v2 d, r32 t)
{
	return NewVec2(CubicFormula(a.x, b.x, c.x, d.x, t), CubicFormula(a.y, b.y, c.y, d.y, t));
}

void RcDrawCubicCurve(v2 p1, v2 c1, v2 c2, v2 p2, u32 numLines, r32 thickness, Color_t color)
{
	for (u32 lIndex = 0; lIndex < numLines; lIndex++)
	{
		r32 time1 = (r32)lIndex * (1.0f / (r32)numLines);
		r32 time2 = (r32)(lIndex+1) * (1.0f / (r32)numLines);
		v2 pos1 = CubicFormula2d(p1, c1, c2, p2, time1);
		v2 pos2 = CubicFormula2d(p1, c1, c2, p2, time2);
		RcDrawLine(pos1, pos2, thickness, color);
	}
}

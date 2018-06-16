/*
File:   app_renderContext.h
Author: Taylor Robbins
Date:   03\03\2018
*/

#ifndef _APP_RENDER_CONTEXT_H
#define _APP_RENDER_CONTEXT_H

struct RenderContext_t
{
	rec viewport;
	
	VertexBuffer_t squareBuffer;
	Texture_t dotTexture;
	Texture_t gradientTexture;
	Texture_t circleTexture;
	
	const Shader_t* boundShader;
	const Font_t* boundFont;
	NewFont_t* boundNewFont;
	const Texture_t* boundTexture;
	const Texture_t* boundAlphaTexture;
	const VertexBuffer_t* boundBuffer;
	const FrameBuffer_t* boundFrameBuffer;
	
	r32 fontSize;
	u16 fontStyle;
	Alignment_t fontAlignment;
	mat4 worldMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	bool doGrayscaleGradient;
	bool useAlphaTexture;
	rec sourceRectangle;
	r32 depth;
	r32 circleRadius;
	r32 circleInnerRadius;
	Color_t color;
	Color_t secondaryColor;
	r32 vigRadius;
	r32 vigSmoothness;
};


#endif //  _APP_RENDER_CONTEXT_H

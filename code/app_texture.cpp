/*
File:   app_texture.cpp
Author: Taylor Robbins
Date:   05\15\2018
Description: 
	** Holds functions that help use create and destroy textures 

#included from app.cpp
*/

void DestroyTexture(Texture_t* texturePntr)
{
	Assert(texturePntr != nullptr);
	
	glDeleteTextures(1, &texturePntr->id);
	
	ClearPointer(texturePntr);
}

Texture_t CreateTexture(const u8* bitmapData, i32 width, i32 height, bool pixelated = false, bool repeat = true)
{
	Texture_t result = {};
	
	result.width = width;
	result.height = height;
	
	glGenTextures(1, &result.id);
	glBindTexture(GL_TEXTURE_2D, result.id);
	
	glTexImage2D(
		GL_TEXTURE_2D, 		//bound texture type
		0,					//image level
		GL_RGBA,			//internal format
		width,		        //image width
		width,		        //image height
		0,					//border
		GL_RGBA,			//format
		GL_UNSIGNED_BYTE,	//type
		bitmapData);		//data
	
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pixelated ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pixelated ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	return result;
}


/*
File:   app_structs.h
Author:Taylor Robbins
Date:   03\3\2018
*/

#ifndef _APP_STRUCTS_H
#define _APP_STRUCTS_H

// +--------------------------------------------------------------+
// |                         Enumerations                         |
// +--------------------------------------------------------------+
typedef enum
{
	AppState_None = 0x00,
	AppState_Default,
	AppState_Visualizer,
} AppState_t;

typedef enum
{
	Alignment_Left,
	Alignment_Center,
	Alignment_Right,
} Alignment_t;

// +--------------------------------------------------------------+
// |                          Structures                          |
// +--------------------------------------------------------------+
struct Texture_t
{
	GLuint id;
	
	union
	{
		v2i size;
		struct { i32 width, height; };
	};
};

struct FontCharInfo_t
{
	union
	{
		v2i position;
		struct { i32 x, y; };
	};
	union
	{
		v2i size;
		struct { i32 width, height; };
	};
	v2 offset;
	r32 advanceX;
};

struct Font_t
{
	Texture_t bitmap;
	r32 fontSize;
	r32 maxCharWidth;
	r32 maxCharHeight;
	r32 maxExtendUp;
	r32 maxExtendDown;
	r32 lineHeight;
	
	u8 firstChar;
	u8 numChars;
	FontCharInfo_t chars[256];
};

typedef enum
{
	FontStyle_Default = 0x00,
	
	FontStyle_Bold         = 0x01,
	FontStyle_Italic       = 0x02,
	FontStyle_StrictStyle  = 0x04,
	FontStyle_StrictSize   = 0x08,
	FontStyle_CreateGlyphs = 0x10,
	
	FontStyle_BakeMask = FontStyle_Bold|FontStyle_Italic,
	
	FontStyle_BoldItalic = FontStyle_Bold|FontStyle_Italic,
	FontStyle_StrictSizeStyle = FontStyle_StrictSize|FontStyle_StrictStyle,
	
	FontStyle_BoldStrictSize = FontStyle_Bold|FontStyle_StrictSize,
	FontStyle_BoldStrictStyle = FontStyle_Bold|FontStyle_StrictStyle,
	FontStyle_BoldStrictSizeStyle = FontStyle_Bold|FontStyle_StrictSize|FontStyle_StrictStyle,
	
	FontStyle_ItalicStrictSize = FontStyle_Italic|FontStyle_StrictSize,
	FontStyle_ItalicStrictStyle = FontStyle_Italic|FontStyle_StrictStyle,
	FontStyle_ItalicStrictSizeStyle = FontStyle_Italic|FontStyle_StrictSize|FontStyle_StrictStyle,
	
	FontStyle_BoldItalicStrictSize = FontStyle_Bold|FontStyle_Italic|FontStyle_StrictSize,
	FontStyle_BoldItalicStrictStyle = FontStyle_Bold|FontStyle_Italic|FontStyle_StrictStyle,
	FontStyle_BoldItalicStrictSizeStyle = FontStyle_Bold|FontStyle_Italic|FontStyle_StrictSize|FontStyle_StrictStyle,
} FontStyle_t;

struct NewFontCharInfo_t
{
	union
	{
		reci bakeRec;
		struct
		{
			v2i bakePos;
			v2i bakeSize;
		};
	};
	v2 size;
	v2 origin;
	r32 advanceX;
};

struct FontFile_t
{
	FontStyle_t styleFlags;
	u32 fileLength;
	u8* fileData;
	stbtt_fontinfo stbInfo;
};

struct FontBake_t
{
	u8 firstChar;
	u8 numChars;
	r32 size; //in pixels
	FontStyle_t styleFlags;
	NewFontCharInfo_t* charInfos;
	r32 maxExtendUp;
	r32 maxExtendDown;
	r32 lineHeight;
	Texture_t texture;
};

struct FontGlyph_t
{
	u8 c;
	r32 size; //in pixels
	FontStyle_t styleFlags;
	NewFontCharInfo_t charInfo;
	Texture_t texture;
};

struct NewFont_t
{
	MemoryArena_t* allocArena;
	
	u32 numFiles;
	FontFile_t* files;
	
	u32 numBakes;
	FontBake_t* bakes;
	
	u32 numGlyphs;
	u32 maxGlyphs;
	FontGlyph_t* glyphs;
};

struct FontFlowInfo_t
{
	//Passed arguments
	const char* strPntr;
	u32 strLength;
	v2 position;
	NewFont_t* fontPntr;
	Alignment_t alignment;
	r32 fontSize;
	FontStyle_t styleFlags;
	bool strictStyle;
	bool createGlyph;
	
	//Results
	rec extents;
	v2 endPos;
	u32 numLines;
	u32 numRenderables;
	r32 extentRight;
	r32 extentDown;
	v2 totalSize;
};

struct Shader_t
{
	GLuint vertId;
	GLuint fragId;
	GLuint programId;
	
	GLuint vertexArray;
	
	struct
	{
		GLint positionAttrib;
		GLint colorAttrib;
		GLint texCoordAttrib;
		
		GLint worldMatrix;
		GLint viewMatrix;
		GLint projectionMatrix;
		GLint diffuseTexture;
		GLint alphaTexture;
		GLint diffuseColor;
		GLint secondaryColor;
		GLint doGrayscaleGradient;
		GLint useAlphaTexture;
		GLint sourceRectangle;
		GLint textureSize;
		GLint circleRadius;
		GLint circleInnerRadius;
	} locations;
};

struct VertexBuffer_t
{
	GLuint id;
	
	u32 numVertices;
};

struct Vertex_t
{
	union
	{
		v3 position;
		struct { r32 x, y, z; };
	};
	union
	{
		v4 color;
		struct { r32 r, g, b, a; };
	};
	union
	{
		v2 texCoord;
		struct { r32 tX, tY; };
	};
};

struct FrameBuffer_t
{
	GLuint id;
	GLuint depthBuffer;
	const Texture_t* renderTexture;
};



// +--------------------------------------------------------------+
// |                          Functions                           |
// +--------------------------------------------------------------+
const char* GetAppStateStr(AppState_t appState)
{
	switch (appState)
	{
		case AppState_None: return "None";
		case AppState_Default: return "Default";
		case AppState_Visualizer: return "Visualizer";
		default: return "Unknown";
	};
}

#endif //  _APP_STRUCTS_H

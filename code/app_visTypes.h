/*
File:   app_visTypes.h
Author: Taylor Robbins
Date:   03\21\2018
*/

#ifndef _APP_VIS_TYPES_H
#define _APP_VIS_TYPES_H

typedef enum
{
	VisFileType_Unknown = 0x00,
	VisFileType_IntelHex,
	VisFileType_Elf,
	VisFileType_Png,
	VisFileType_Jpeg,
	VisFileType_Bmp,
	VisFileType_O,
	VisFileType_Exe,
} VisFileType_t;

typedef enum
{
	VisViewMode_Default,
	VisViewMode_Alternate,
	VisViewMode_RawHex,
} VisViewMode_t;

// +--------------------------------------------------------------+
// |                         Raw Hex View                         |
// +--------------------------------------------------------------+
typedef enum
{
	HexInterpType_None = 0x00,
	HexInterpType_ASCII,
	HexInterpType_Unicode,
	HexInterpType_DecimalLittle8,
	HexInterpType_DecimalLittle16,
	HexInterpType_DecimalLittle32,
	HexInterpType_DecimalBig8,
	HexInterpType_DecimalBig16,
	HexInterpType_DecimalBig32,
} HexInterpType_t;

// +--------------------------------------------------------------+
// |                          Intel Hex                           |
// +--------------------------------------------------------------+



#endif //  _APP_VIS_TYPES_H

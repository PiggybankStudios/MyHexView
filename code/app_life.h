/*
File:   app_life.h
Author: Taylor Robbins
Date:   05\27\2018
*/

#ifndef _APP_LIFE_H
#define _APP_LIFE_H

struct LifeData_t
{
	bool initialized;
	
	v2i boardSize;
	u8* boardData;
	u8* newBoardData;
};

#endif //  _APP_LIFE_H

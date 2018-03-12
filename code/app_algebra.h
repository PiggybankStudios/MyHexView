/*
File:   app_algebra.h
Author: Taylor Robbins
Date:   03\11\2018
*/

#ifndef _APP_ALGEBRA_H
#define _APP_ALGEBRA_H

#define NUM_VARS 16

#define VAR_0  0
#define VAR_1  1
#define VAR_2  2
#define VAR_3  3
#define VAR_4  4
#define VAR_5  5
#define VAR_6  6
#define VAR_7  7
#define VAR_8  8
#define VAR_9  9
#define VAR_10 10
#define VAR_11 11
#define VAR_12 12
#define VAR_13 13
#define VAR_14 14
#define VAR_15 15

#define VAR_x VAR_0
#define VAR_y VAR_1
#define VAR_z VAR_2
#define VAR_w VAR_3
#define VAR_a VAR_4
#define VAR_b VAR_5
#define VAR_c VAR_6
#define VAR_d VAR_7
#define VAR_A VAR_8
#define VAR_B VAR_9
#define VAR_C VAR_10
#define VAR_D VAR_11
#define VAR_t VAR_12

static char AlgVarChars[NUM_VARS] = {
	'x', //VAR_0
	'y', //VAR_1
	'z', //VAR_2
	'w', //VAR_3
	'a', //VAR_4
	'b', //VAR_5
	'c', //VAR_6
	'd', //VAR_7
	'A', //VAR_8
	'B', //VAR_9
	'C', //VAR_10
	'D', //VAR_11
	't', //VAR_12
	'?', //VAR_13
	'?', //VAR_14
	'?', //VAR_15
};

struct AlgTerm_t
{
	i32 coefficient;
	i32 varPowers[NUM_VARS];
};

struct AlgGroup_t
{
	MemoryArena_t* allocArena;
	u32 maxTerms;
	u32 numTerms;
	AlgTerm_t* terms;
};

#endif //  _APP_ALGEBRA_H

/*
File:   app_algebra.cpp
Author: Taylor Robbins
Date:   03\11\2018
Description: 
	** This file holds some things that help us work with simple algebraic expressions 
*/

// +--------------------------------------------------------------+
// |               Term and Group Utility Functions               |
// +--------------------------------------------------------------+

AlgTerm_t NewAlgTerm(i32 coefficient, i32 power0 = 0, i32 power1 = 0, i32 power2 = 0, i32 power3 = 0,
	i32 power4 = 0, i32 power5 = 0, i32 power6 = 0, i32 power7 = 0,
	i32 power8 = 0, i32 power9 = 0, i32 power10 = 0, i32 power11 = 0,
	i32 power12 = 0, i32 power13 = 0, i32 power14 = 0, i32 power15 = 0)
{
	AlgTerm_t result = {};
	
	result.coefficient = coefficient;
	result.varPowers[VAR_0]  = power0;
	result.varPowers[VAR_1]  = power1;
	result.varPowers[VAR_2]  = power2;
	result.varPowers[VAR_3]  = power3;
	result.varPowers[VAR_4]  = power4;
	result.varPowers[VAR_5]  = power5;
	result.varPowers[VAR_6]  = power6;
	result.varPowers[VAR_7]  = power7;
	result.varPowers[VAR_8]  = power8;
	result.varPowers[VAR_9]  = power9;
	result.varPowers[VAR_10] = power10;
	result.varPowers[VAR_11] = power11;
	result.varPowers[VAR_12] = power12;
	result.varPowers[VAR_13] = power13;
	result.varPowers[VAR_14] = power14;
	result.varPowers[VAR_15] = power15;
	
	return result;
}

AlgTerm_t NewAlgTermFromString(const char* termStr)
{
	AlgTerm_t result = {};
	
	u32 termStrLength = (u32)strlen(termStr);
	u32 numCoefficientChars = 0;
	for (u32 cIndex = 0; cIndex < termStrLength; cIndex++)
	{
		char c = termStr[cIndex];
		if (IsCharClassNumeric(c) || c == '-' || c == '+' || c == ' ')
		{ numCoefficientChars++; }
		else
		{ break; }
	}
	
	result.coefficient = 1;
	if (numCoefficientChars > 0)
	{
		if (!TryParseI32(termStr, numCoefficientChars, &result.coefficient))
		{
			result.coefficient = 1;
		}
	}
	
	for (u32 cIndex = numCoefficientChars; cIndex < termStrLength; cIndex++)
	{
		char c = termStr[cIndex];
		for (u32 vIndex = 0; vIndex < NUM_VARS; vIndex++)
		{
			if (c == AlgVarChars[vIndex])
			{
				result.varPowers[vIndex]++;
				break;
			}
		}
	}
	
	return result;
}

void CreateAlgGroup(AlgGroup_t* group, MemoryArena_t* arenaPntr, u32 numTerms = 0, const AlgTerm_t* termsPntr = nullptr)
{
	Assert(group != nullptr);
	
	ClearPointer(group);
	group->allocArena = arenaPntr;
	group->maxTerms = numTerms;
	group->numTerms = 0;
	if (numTerms > 0)
	{
		group->terms = PushArray(arenaPntr, AlgTerm_t, numTerms);
		if (termsPntr != nullptr)
		{
			memcpy(group->terms, termsPntr, numTerms * sizeof(AlgTerm_t));
			group->numTerms = numTerms;
		}
	}
}

void DestroyAlgGroup(AlgGroup_t* group)
{
	Assert(group != nullptr);
	
	if (group->terms != nullptr)
	{
		Assert(group->allocArena != nullptr);
		ArenaPop(group->allocArena, group->terms);
	}
	
	ClearPointer(group);
}

void AlgGroupAddTerm(AlgGroup_t* group, AlgTerm_t newTerm)
{
	Assert(group != nullptr);
	Assert(group->numTerms <= group->maxTerms);
	
	//Expand term space
	if (group->numTerms == group->maxTerms)
	{
		Assert(group->allocArena != nullptr);
		u32 newMax = group->maxTerms + 8;
		AlgTerm_t* newTermSpace = PushArray(group->allocArena, AlgTerm_t, newMax);
		if (group->numTerms > 0)
		{
			Assert(group->terms != nullptr);
			memcpy(newTermSpace, group->terms, group->numTerms*sizeof(AlgTerm_t));
			ArenaPop(group->allocArena, group->terms);
		}
		group->terms = newTermSpace;
		group->maxTerms = newMax;
	}
	
	Assert(group->numTerms < group->maxTerms);
	
	AlgTerm_t* termPntr = &group->terms[group->numTerms];
	memcpy(termPntr, &newTerm, sizeof(AlgTerm_t));
	group->numTerms++;
}

void AlgGroupAddTerms(AlgGroup_t* group, u32 numTerms, const AlgTerm_t* newTerms)
{
	Assert(group != nullptr);
	Assert(numTerms == 0 || newTerms != nullptr);
	for (u32 tIndex = 0; tIndex < numTerms; tIndex++)
	{
		AlgGroupAddTerm(group, newTerms[tIndex]);
	}
}

void AlgGroupRemoveTerm(AlgGroup_t* group, u32 index)
{
	Assert(group != nullptr);
	Assert(index < group->numTerms);
	
	for (u32 tIndex = index; tIndex+1 < group->numTerms; tIndex++)
	{
		memcpy(&group->terms[tIndex], &group->terms[tIndex+1], sizeof(AlgTerm_t));
	}
	group->numTerms--;
}

u32 GetAlgTermStr(MemoryArena_t* arenaPntr, const AlgTerm_t* term, char** strOut, bool standalone = true)
{
	Assert(term != nullptr);
	Assert(arenaPntr != nullptr || strOut == nullptr);
	
	u32 strLength = 0;
	char* strPntr = nullptr;
	
	u32 numChars = 0;
	for (u8 loop = 0; loop < 2; loop++)
	{
		TempPushMark();
		char* coefficientStr = TempPrint("%d", term->coefficient);
		u32 coefficientStrLength = (u32)strlen(coefficientStr);
		if (strPntr != nullptr) { memcpy(&strPntr[numChars], coefficientStr, coefficientStrLength); }
		numChars += coefficientStrLength;
		TempPopMark();
		
		for (u32 vIndex = 0; vIndex < NUM_VARS; vIndex++)
		{
			if (term->varPowers[vIndex] > 0)
			{
				for (i32 pIndex = 0; pIndex < term->varPowers[vIndex]; pIndex++)
				{
					if (strPntr != nullptr) { strPntr[numChars] = AlgVarChars[vIndex]; }
					numChars++;
				}
			}
		}
		
		if (loop == 0)
		{
			if (strOut != nullptr)
			{
				Assert(arenaPntr != nullptr);
				if (numChars > 0)
				{
					strLength = numChars;
					strPntr = PushArray(arenaPntr, char, strLength+1);
					numChars = 0;
				}
				else
				{
					*strOut = nullptr;
					return 0;
				}
			}
			else
			{
				return numChars;
			}
		}
	}
	
	//If we reach this spot then we should have allocated a string and filled it
	Assert(strPntr != nullptr && strOut != nullptr);
	Assert(numChars == strLength);
	strPntr[strLength] = '\0';
	*strOut = strPntr;
	return strLength;
}

//TODO: Finish these
u32 GetAlgGroupStr(MemoryArena_t* arenaPntr, const AlgGroup_t* group, char** strOut)
{
	Assert(group != nullptr);
	Assert(arenaPntr != nullptr || strOut == nullptr);
	
	u32 strLength = 0;
	char* strPntr = nullptr;
	
	u32 numChars = 0;
	for (u8 loop = 0; loop < 2; loop++)
	{
		for (u32 tIndex = 0; tIndex < group->numTerms; tIndex++)
		{
			AlgTerm_t* termPntr = &group->terms[tIndex];
			if (tIndex > 0)
			{
				if (strPntr != nullptr) { strPntr[numChars] = ' '; }
				numChars++;
			}
			
			TempPushMark();
			char* termStr = nullptr;
			u32 termStrLength = GetAlgTermStr(TempArena, termPntr, &termStr, false);
			if (termStrLength > 0)
			{
				Assert(termStr != nullptr);
				if (strPntr != nullptr) { memcpy(&strPntr[numChars], termStr, termStrLength); }
				numChars += termStrLength;
			}
			TempPopMark();
		}
		
		if (loop == 0)
		{
			if (strOut != nullptr)
			{
				Assert(arenaPntr != nullptr);
				if (numChars > 0)
				{
					strLength = numChars;
					strPntr = PushArray(arenaPntr, char, strLength+1);
					numChars = 0;
				}
				else
				{
					*strOut = nullptr;
					return 0;
				}
			}
			else
			{
				return numChars;
			}
		}
	}
	
	//If we reach this spot then we should have allocated a string and filled it
	Assert(strPntr != nullptr && strOut != nullptr);
	Assert(numChars == strLength);
	strPntr[strLength] = '\0';
	*strOut = strPntr;
	return strLength;
}

// +--------------------------------------------------------------+
// |                      Algebraic Helpers                       |
// +--------------------------------------------------------------+

bool CanCombineTerms(const AlgTerm_t* term1, const AlgTerm_t* term2)
{
	Assert(term1 != nullptr && term2 != nullptr);
	for (u32 vIndex = 0; vIndex < NUM_VARS; vIndex++)
	{
		if (term1->varPowers[vIndex] != term2->varPowers[vIndex])
		{
			return false;
		}
	}
	return true;
}

AlgTerm_t CombineTerms(const AlgTerm_t* term1, const AlgTerm_t* term2)
{
	Assert(term1 != nullptr && term2 != nullptr);
	Assert(CanCombineTerms(term1, term2));
	
	AlgTerm_t result = {};
	result.coefficient = term1->coefficient + term2->coefficient;
	memcpy(result.varPowers, term1->varPowers, sizeof(result.varPowers));
	return result;
}

AlgTerm_t MultiplyTerms(const AlgTerm_t* term1, const AlgTerm_t* term2)
{
	Assert(term1 != nullptr && term2 != nullptr);
	
	AlgTerm_t result = {};
	result.coefficient = term1->coefficient * term2->coefficient;
	for (u32 vIndex = 0; vIndex < NUM_VARS; vIndex++)
	{
		result.varPowers[vIndex] = term1->varPowers[vIndex] + term2->varPowers[vIndex];
	}
	return result;
}

void CombineLikeTerms(AlgGroup_t* group)
{
	for (u32 tIndex = 0; tIndex+1 < group->numTerms; tIndex++)
	{
		for (u32 oIndex = tIndex+1; oIndex < group->numTerms; oIndex++)
		{
			if (CanCombineTerms(&group->terms[tIndex], &group->terms[oIndex]))
			{
				AlgTerm_t newTerm = CombineTerms(&group->terms[tIndex], &group->terms[oIndex]);
				group->terms[tIndex] = newTerm;
				AlgGroupRemoveTerm(group, oIndex);
				oIndex--;
			}
		}
	}
}

void MultiplyGroups(const AlgGroup_t* group1, const AlgGroup_t* group2, AlgGroup_t* result, MemoryArena_t* arenaPntr)
{
	Assert(group1 != nullptr && group2 != nullptr);
	Assert(result != nullptr);
	
	CreateAlgGroup(result, arenaPntr, group1->numTerms * group2->numTerms, nullptr);
	for (u32 tIndex1 = 0; tIndex1 < group1->numTerms; tIndex1++)
	{
		const AlgTerm_t* term1 = &group1->terms[tIndex1];
		for (u32 tIndex2 = 0; tIndex2 < group2->numTerms; tIndex2++)
		{
			const AlgTerm_t* term2 = &group2->terms[tIndex2];
			AlgTerm_t newTerm = MultiplyTerms(term1, term2);
			AlgGroupAddTerm(result, newTerm);
		}
	}
}

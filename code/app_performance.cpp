/*
File:   app_performance.cpp
Author: Taylor Robbins
Date:   05\19\2018
Description: 
	** Holds functions and related with profiling and tracking the performance of the program 
*/

#if DEBUG

void StartTimeBlock(const char* blockName)
{
	Assert(app->debugNumTimeBlocks < MAX_TIME_BLOCK_DEPTH);
	Assert(blockName != nullptr);
	app->debugTimeBlockNames[app->debugNumTimeBlocks] = blockName;
	app->debugTimeBlocks[app->debugNumTimeBlocks] = PLAT_GetClockCycles();
	app->debugNumTimeBlocks++;
}

u64 EndTimeBlock()
{
	Assert(app->debugNumTimeBlocks > 0);
	u64 endTime = PLAT_GetClockCycles();
	u64 result = endTime - app->debugTimeBlocks[app->debugNumTimeBlocks-1];
	const char* blockName = app->debugTimeBlockNames[app->debugNumTimeBlocks-1];
	
	Assert(app->activeNumTimedBlockInfos < MAX_TIMED_BLOCKS);
	TimedBlockInfo_t* blockInfo = nullptr;
	for (u32 bIndex = 0; bIndex < app->activeNumTimedBlockInfos; bIndex++)
	{
		if (strcmp(app->activeTimedBlockInfos[bIndex].blockName, blockName) == 0)
		{
			blockInfo = &app->activeTimedBlockInfos[bIndex];
		}
	}
	if (blockInfo == nullptr)
	{
		blockInfo = &app->activeTimedBlockInfos[app->activeNumTimedBlockInfos];
		blockInfo->counterElapsed = 0;
		blockInfo->numCalls = 0;
		blockInfo->parentName = nullptr;
		app->activeNumTimedBlockInfos++;
	}
	
	blockInfo->blockName = blockName;
	blockInfo->counterElapsed += result;
	blockInfo->numCalls++;
	
	if (app->debugNumTimeBlocks >= 2)
	{
		const char* parentName = app->debugTimeBlockNames[app->debugNumTimeBlocks-2];
		if (blockInfo->parentName != nullptr && strcmp(blockInfo->parentName, parentName) != 0)
		{
			blockInfo->numParents++;
		}
		else
		{
			blockInfo->parentName = parentName;
			blockInfo->numParents = 1;
		}
	}
	else
	{
		blockInfo->parentName = nullptr;
	}
	
	app->debugNumTimeBlocks--;
	return result;
}

void SaveTimeBlocks()
{
	app->lastNumTimedBlockInfos = app->activeNumTimedBlockInfos;
	memcpy(app->lastTimedBlockInfos, app->activeTimedBlockInfos, sizeof(TimedBlockInfo_t)*MAX_TIMED_BLOCKS);
	app->activeNumTimedBlockInfos = 0;
}

void ClearTimeBlocks()
{
	app->activeNumTimedBlockInfos = 0;
}

TimedBlockInfo_t* GetTimedBlockInfoByName(const char* blockName)
{
	for (u32 bIndex = 0; bIndex < app->lastNumTimedBlockInfos; bIndex++)
	{
		if (app->lastTimedBlockInfos[bIndex].blockName != nullptr &&
			strcmp(app->lastTimedBlockInfos[bIndex].blockName, blockName) == 0)
		{
			return &app->lastTimedBlockInfos[bIndex];
		}
	}
	return nullptr;
}

TimedBlockInfo_t* GetTimedBlockInfoByParent(const char* parentName, u32 index = 0)
{
	u32 numFound = 0;
	for (u32 bIndex = 0; bIndex < app->lastNumTimedBlockInfos; bIndex++)
	{
		if (app->lastTimedBlockInfos[bIndex].parentName != nullptr &&
			strcmp(app->lastTimedBlockInfos[bIndex].parentName, parentName) == 0)
		{
			if (numFound >= index) { return &app->lastTimedBlockInfos[bIndex]; }
			numFound++;
		}
	}
	return nullptr;
}

#else //!DEBUG

#define StartTimeBlock(blockName)
#define EndTimeBlock() 0
#define SaveTimeBlocks()
#define ClearTimeBlocks()

#endif

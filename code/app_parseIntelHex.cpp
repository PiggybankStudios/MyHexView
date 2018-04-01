/*
File:   app_parseIntelHex.cpp
Author: Taylor Robbins
Date:   03\31\2018
Description: 
	** Holds some functions that are used in the visualizer AppState 
*/

VisHexDataRegion_t* VisParseIntelHex(MemoryArena_t* arenaPntr, const char* hexFileData, u32 hexFileDataLength, u32* numRegionsOut)
{
	Assert(arenaPntr != nullptr);
	Assert(hexFileData != nullptr);
	Assert(numRegionsOut != nullptr);
	
	u32 numRegions = 0;
	VisHexDataRegion_t* result = nullptr;
	
	for (u8 pass = 0; pass < 2; pass++)
	{
		TempList_t regionList = {};
		if (pass == 0)
		{
			TempPushMark();
			TempListInit(&regionList, sizeof(VisHexDataRegion_t));
		}
		
		u32 lineNum = 0;
		u16 upperAddress = 0x0000;
		u32 lineStart = 0;
		for (u32 cIndex = 0; cIndex < hexFileDataLength; cIndex++)
		{
			u32 hexLineStrLength = 0;
			const char* hexLineStr = nullptr;
			if (hexFileData[cIndex] == '\n' || hexFileData[cIndex] == '\r')
			{
				if (cIndex > lineStart)
				{
					hexLineStr = &hexFileData[lineStart];
					hexLineStrLength = (cIndex - lineStart);
				}
				lineStart = cIndex+1;
				lineNum++;
			}
			
			// +==============================+
			// |  Parse Hex Line into Binary  |
			// +==============================+
			bool parsedHexLine = false;
			u8 hexLineLength = 0;
			u8 hexLineData[255];
			u8 hexLineCmd = 0x00;
			u8 hexLineChecksum = 0x00;
			u16 hexLineAddress = 0x00000000;
			if (hexLineStr != nullptr)
			{
				if (hexLineStrLength < 11 || hexLineStr[0] != ':' || (hexLineStrLength%2) != 1)
				{
					DEBUG_PrintLine("Invalid line %u: \"%.*s\"", lineNum, hexLineLength, hexLineStr);
				}
				else if (!IsStringHex(&hexLineStr[1], hexLineStrLength-1))
				{
					DEBUG_PrintLine("Invalid chars on line %u: \"%.*s\"", lineNum, hexLineLength, hexLineStr);
				}
				else
				{
					hexLineLength = ParseHexU8(&hexLineStr[1 + 0*2]);
					hexLineAddress = ParseHexU16(&hexLineStr[1 + 1*2]);
					hexLineCmd = ParseHexU8(&hexLineStr[1 + 3*2]);
					hexLineChecksum = ParseHexU8(&hexLineStr[hexLineStrLength-1 - 2]);
					if (hexLineStrLength-1 != (5 + (u32)hexLineLength)*2)
					{
						DEBUG_PrintLine("Length doesn't match str length on line %u: \"%.*s\"", lineNum, hexLineLength, hexLineStr);
					}
					else
					{
						for (u32 bIndex = 0; bIndex < hexLineLength; bIndex++)
						{
							hexLineData[bIndex] = ParseHexU8(&hexLineStr[1 + (4 + bIndex)*2]);
						}
						parsedHexLine = true;
					}
				}
			}
			
			// +==============================+
			// |       Process The line       |
			// +==============================+
			if (parsedHexLine)
			{
				u32 entryAddress = ((u32)upperAddress << 16) + (u32)hexLineAddress;
				// DEBUG_PrintLine("Parsed line %u: CMD:%02X ADDR:%08X CRC:%02X [%u]{ }", lineNum, hexLineCmd, entryAddress, hexLineChecksum, hexLineLength);
				
				if (hexLineCmd == 0x04) //Set Upper Address Command
				{
					if (hexLineLength == 2)
					{
						upperAddress = ((u16)hexLineData[0] << 8) + ((u16)hexLineData[1] << 0);
						// DEBUG_PrintLine("Upper address = %04X", upperAddress);
					}
					else
					{
						DEBUG_PrintLine("Found upper address command with %u length on line %u", hexLineLength, lineNum);
					}
				}
				else if (hexLineCmd == 0x01) //End of file command
				{
					DEBUG_PrintLine("End of file command on line %u", lineNum);
					break;
				}
				// +==============================+
				// |     Process Data Command     |
				// +==============================+
				else if (hexLineCmd == 0x00) //Data Command
				{
					// DEBUG_PrintLine("Data at 0x%08X [%u]{ }", entryAddress, hexLineLength);
					for (u8 bIndex = 0; bIndex < hexLineLength; bIndex++)
					{
						u8 byteValue = hexLineData[bIndex];
						u32 byteAddress = entryAddress + bIndex;
						// +==================================+
						// | Create/Expand and Merge regions  |
						// +==================================+
						if (pass == 0)
						{
							//Check for region expanded
							bool expandedRegion = false;
							for (u32 rIndex = 0; rIndex < regionList.numItems; rIndex++)
							{
								VisHexDataRegion_t* region = TempListGet(&regionList, VisHexDataRegion_t, rIndex);
								if (byteAddress >= region->address && byteAddress < region->address + region->size)
								{
									//It's within the current region bounderies
									DEBUG_PrintLine("Data conflict at %08X", byteAddress);
									expandedRegion = true;
									break;
								}
								else if (region->address > 0 && byteAddress == region->address-1)
								{
									region->address--;
									region->size++;
									expandedRegion = true;
									// DEBUG_PrintLine("[%u] <{%08X-%08X}", rIndex, region->address, region->address+region->size);
									break;
								}
								else if (byteAddress == region->address + region->size)
								{
									region->size++;
									expandedRegion = true;
									// DEBUG_PrintLine("[%u] {%08X-%08X}>", rIndex, region->address, region->address+region->size);
									break;
								}
							}
							
							//If we didn't expand a region this byte starts a new region
							if (!expandedRegion)
							{
								VisHexDataRegion_t newRegion = {};
								newRegion.address = byteAddress;
								newRegion.size = 1;
								bool foundEmptyRegion = false;
								for (u32 rIndex = 0; rIndex < regionList.numItems; rIndex++)
								{
									VisHexDataRegion_t* region = TempListGet(&regionList, VisHexDataRegion_t, rIndex);
									if (region->size == 0)
									{
										memcpy(region, &newRegion, sizeof(newRegion));
										foundEmptyRegion = true;
										break;
									}
								}
								if (!foundEmptyRegion)
								{
									TempListAdd(&regionList, VisHexDataRegion_t, &newRegion);
								}
								DEBUG_PrintLine("Started new region at 0x%08X", newRegion.address);
							}
							
							//Combine regions that are now touching
							for (u32 rIndex1 = 0; rIndex1 < regionList.numItems; rIndex1++)
							{
								VisHexDataRegion_t* region1 = TempListGet(&regionList, VisHexDataRegion_t, rIndex1);
								for (u32 rIndex2 = rIndex1+1; rIndex2 < regionList.numItems; rIndex2++)
								{
									VisHexDataRegion_t* region2 = TempListGet(&regionList, VisHexDataRegion_t, rIndex2);
									if (region1->size > 0 && region2->size > 0)
									{
										if (region1->address == region2->address+region2->size)
										{
											//Touching [region2][region1]
											DEBUG_PrintLine("Merged [%08x-%08X][%08X-%08X] -> [%08X-%08X]",
												region2->address, region2->address + region2->size,
												region1->address, region1->address + region1->size,
												region2->address, region2->address + region2->size + region1->size
											);
											region2->size += region1->size;
											region1->size = 0; //essentially delete region1
										}
										else if (region2->address == region1->address+region1->size)
										{
											//Touching [region1][region2]
											DEBUG_PrintLine("Merged [%08x-%08X][%08X-%08X] -> [%08X-%08X]",
												region1->address, region1->address + region1->size,
												region2->address, region2->address + region2->size,
												region1->address, region1->address + region1->size + region2->size
											);
											region1->size += region2->size;
											region2->size = 0; //essentially delete region2
										}
										//NOTE: We shouldn't ever have to check for overlapping regions
										//      since they always expand one byte at a time
									}
								}
							}
						}
						// +==============================+
						// |  Copy Data into Data Spaces  |
						// +==============================+
						else
						{
							Assert(result != nullptr);
							
							bool foundRegion = false;
							for (u32 rIndex = 0; rIndex < numRegions; rIndex++)
							{
								VisHexDataRegion_t* region = &result[rIndex];
								Assert(region->data != nullptr);
								if (byteAddress >= region->address && byteAddress < region->address + region->size)
								{
									region->data[byteAddress - region->address] = byteValue;
									// DEBUG_PrintLine("region%u[%u] = %02X", rIndex, byteAddress - region->address, byteValue);
									foundRegion = true;
									break;
								}
							}
							
							Assert(foundRegion);
						}
					}
				}
			}
		}
		
		// +========================================+
		// | Allocate Region Array and Data Spaces  |
		// +========================================+
		if (pass == 0)
		{
			numRegions = 0;
			for (u32 rIndex = 0; rIndex < regionList.numItems; rIndex++)
			{
				VisHexDataRegion_t* region = TempListGet(&regionList, VisHexDataRegion_t, rIndex);
				if (region->size > 0) { numRegions++; }
			}
			
			if (numRegions == 0)
			{
				DEBUG_WriteLine("No data entries found in Intel HEX file");
				break;
			}
			
			//NOTE: We have to allocate onto the mainHeap first in case arenaPntr == TempArena where it would get popped when we call TempPopMark here soon
			if (arenaPntr == TempArena) { result = PushArray(mainHeap, VisHexDataRegion_t, numRegions); }
			else { result = PushArray(arenaPntr, VisHexDataRegion_t, numRegions); }
			numRegions = 0;
			for (u32 rIndex = 0; rIndex < regionList.numItems; rIndex++)
			{
				VisHexDataRegion_t* region = TempListGet(&regionList, VisHexDataRegion_t, rIndex);
				if (region->size > 0)
				{
					memcpy(&result[numRegions], region, sizeof(VisHexDataRegion_t));
					numRegions++;
				}
			}
			
			TempPopMark();
			
			if (arenaPntr == TempArena)
			{
				//NOTE: We have to allocate onto the mainHeap first in case arenaPntr == TempArena where it would get popped when we call TempPopMark here soon
				VisHexDataRegion_t* actualArray = PushArray(arenaPntr, VisHexDataRegion_t, numRegions);
				memcpy(actualArray, result, sizeof(VisHexDataRegion_t) * numRegions);
				ArenaPop(mainHeap, result);
				result = actualArray;
			}
			
			DEBUG_PrintLine("Found %u regions:", numRegions);
			for (u32 rIndex = 0; rIndex < numRegions; rIndex++)
			{
				VisHexDataRegion_t* region = &result[rIndex];
				DEBUG_PrintLine("\t[%u]: 0x%08X - 0x%08X (%s)", rIndex, region->address, region->address+region->size, FormattedSizeStr(region->size));
				
				region->allocArena = arenaPntr;
				region->data = PushArray(arenaPntr, u8, region->size);
				memset(region->data, 0xFF, region->size);
			}
		}
	}
	
	if (numRegionsOut != nullptr) { *numRegionsOut = numRegions; }
	return result;
}


/*
File:   app_triangulation.cpp
Author: Taylor Robbins
Date:   05\20\2018
Description: 
	** Holds some functions for handling triangles, polygons, and triangulation of polygons
*/

bool IsTriangleClockwise(v2 p0, v2 p1, v2 p2)
{
	return (Vec2Dot(p2 - p0, Vec2PerpRight(p1 - p0)) >= 0);
}
bool IsTriangleClockwise(const Triangle_t& triangle)
{
	return IsTriangleClockwise(triangle.p0, triangle.p1, triangle.p2);
}
bool IsInsideTriangle(v2 test, v2 p0, v2 p1, v2 p2)
{
	v2 perp0 = Vec2PerpRight(p1 - p0);
	v2 perp1 = Vec2PerpRight(p2 - p1);
	v2 perp2 = Vec2PerpRight(p0 - p2);
	return (
		SignR32(Vec2Dot(p2 - p0, perp0)) == SignR32(Vec2Dot(test - p0, perp0)) &&
		SignR32(Vec2Dot(p0 - p1, perp1)) == SignR32(Vec2Dot(test - p1, perp1)) &&
		SignR32(Vec2Dot(p1 - p2, perp2)) == SignR32(Vec2Dot(test - p2, perp2))
	);
}

Triangle_t* TriangulatePolygonEars(MemoryArena_t* arenaPntr, const Polygon_t* polygon, u32* numTrianglesOut = nullptr)
{
	Assert(arenaPntr != nullptr);
	Assert(polygon != nullptr);
	
	Triangle_t* result = nullptr;
	if (polygon->numVerts < 3)
	{
		if (numTrianglesOut != nullptr) { *numTrianglesOut = 0; }
		return nullptr;
	}
	else if (polygon->numVerts == 3)
	{
		if (numTrianglesOut != nullptr) { *numTrianglesOut = 1; }
		result = PushArray(arenaPntr, Triangle_t, 1);
		result->p0 = polygon->verts[0];
		result->p1 = polygon->verts[1];
		result->p2 = polygon->verts[2];
		return result;
	}
	
	u32 numTriangles = polygon->numVerts - 2;
	if (numTrianglesOut != nullptr) { *numTrianglesOut = numTriangles; }
	
	result = PushArray(arenaPntr, Triangle_t, numTriangles);
	
	TempPushMark();
	Polygon_t tempPolygon = {};
	tempPolygon.numVerts = polygon->numVerts;
	tempPolygon.verts = PushArray(TempArena, v2, polygon->numVerts);
	memcpy(tempPolygon.verts, polygon->verts, polygon->numVerts * sizeof(v2));
	
	u32 tIndex = 0;
	while (tempPolygon.numVerts > 3)
	{
		bool foundEar = false;
		u32 earVertIndex = 0;
		for (u32 vIndex = 0; vIndex < tempPolygon.numVerts; vIndex++)
		{
			v2 vert0 = tempPolygon.verts[(vIndex + 0) % tempPolygon.numVerts];
			v2 vert1 = tempPolygon.verts[(vIndex + 1) % tempPolygon.numVerts];
			v2 vert2 = tempPolygon.verts[(vIndex + 2) % tempPolygon.numVerts];
			if (!IsTriangleClockwise(vert0, vert1, vert2)) { continue; }
			
			bool isEar = true;
			for (u32 vIndex2 = 0; vIndex2 < tempPolygon.numVerts; vIndex2++)
			{
				if (vIndex2 < vIndex || vIndex2 > vIndex+2)
				{
					if (IsInsideTriangle(tempPolygon.verts[vIndex2], vert0, vert1, vert2))
					{
						isEar = false;
						break;
					}
				}
			}
			
			if (isEar)
			{
				foundEar = true;
				earVertIndex = vIndex;
				break;
			}
		}
		if (!foundEar)
		{
			DEBUG_PrintLine("The polygon in invalid after %u triangle(s)!", tIndex);
			TempPopMark();
			if (arenaPntr != TempArena) { ArenaPop(arenaPntr, result); }
			if (numTrianglesOut != nullptr) { *numTrianglesOut = 0; }
			return nullptr;
		}
		Assert(foundEar);
		
		result[tIndex].p0 = tempPolygon.verts[(earVertIndex + 0) % tempPolygon.numVerts];
		result[tIndex].p1 = tempPolygon.verts[(earVertIndex + 1) % tempPolygon.numVerts];
		result[tIndex].p2 = tempPolygon.verts[(earVertIndex + 2) % tempPolygon.numVerts];
		tIndex++;
		
		//TODO: Remove the ear vertex from the polygon
		//Remove the middle vertex in the ear and shift all vertices above down by one
		for (u32 vIndex = ((earVertIndex+1)%tempPolygon.numVerts); vIndex+1 < tempPolygon.numVerts; vIndex++)
		{
			tempPolygon.verts[vIndex] = tempPolygon.verts[vIndex+1];
		}
		tempPolygon.numVerts--;
	}
	Assert(tempPolygon.numVerts == 3);
	Assert(tIndex == numTriangles-1);
	
	result[tIndex].p0 = tempPolygon.verts[0];
	result[tIndex].p1 = tempPolygon.verts[1];
	result[tIndex].p2 = tempPolygon.verts[2];
	tIndex++;
	
	TempPopMark();
	
	return result;
}

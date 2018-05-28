/*
File:   app_life.cpp
Author: Taylor Robbins
Date:   05\27\2018
Description: 
	** Holds the AppState_Life top-level functions 
*/

const v2i GOL_Spaceship_Size = NewVec2i(5, 4);
const char* GOL_Spaceship = 
	"#..#."
	"....#"
	"#...#"
	".####"
;

const v2i GOL_Pentadecathlon_Size = NewVec2i(3, 10);
const char* GOL_Pentadecathlon = 
	".#."
	".#."
	"#.#"
	".#."
	".#."
	".#."
	".#."
	"#.#"
	".#."
	".#."
;

const v2i GOL_Pulsar_Size = NewVec2i(13, 13);
const char* GOL_Pulsar = 
	"..###...###.."
	"............."
	"#....#.#....#"
	"#....#.#....#"
	"#....#.#....#"
	"..###...###.."
	"............."
	"..###...###.."
	"#....#.#....#"
	"#....#.#....#"
	"#....#.#....#"
	"............."
	"..###...###.."
;

const v2i GOL_Generator_Size = NewVec2i(36, 12);
const char* GOL_Generator = 
	"...........................#........"
	"..........................#.#......."
	".........##...............##.#......"
	".........#.#..............##.##...##"
	"....##......#.............##.#....##"
	"##.#..#..#..#.............#.#......."
	"##..##......#..............#........"
	".........#.#........................"
	".........##........................."
	"...................................."
	"...................................."
	"...................................."
;

void CreateGameOfLifeBoard()
{
	if (lifeData->boardData != nullptr)
	{
		ArenaPop(mainHeap, lifeData->boardData);
	}
	if (lifeData->newBoardData != nullptr)
	{
		ArenaPop(mainHeap, lifeData->newBoardData);
	}
	
	lifeData->boardSize = NewVec2i((u32)(RenderScreenSize.width/5), (u32)(RenderScreenSize.height/5));
	
	lifeData->boardData = PushArray(mainHeap, u8, lifeData->boardSize.width * lifeData->boardSize.height);
	memset(lifeData->boardData, 0x00, lifeData->boardSize.width * lifeData->boardSize.height);
	
	lifeData->newBoardData = PushArray(mainHeap, u8, lifeData->boardSize.width * lifeData->boardSize.height);
	memset(lifeData->newBoardData, 0x00, lifeData->boardSize.width * lifeData->boardSize.height);
}

void SimulateGameOfLife()
{
	for (i32 y = 0; y < lifeData->boardSize.height; y++)
	{
		for (i32 x = 0; x < lifeData->boardSize.width; x++)
		{
			u8* cell = &lifeData->boardData[y*lifeData->boardSize.width + x];
			u8* newCell = &lifeData->newBoardData[y*lifeData->boardSize.width + x];
			
			u8 numNeighbors = 0;
			if (x > 0 && lifeData->boardData[(y)*lifeData->boardSize.width + (x-1)] != 0x00) { numNeighbors++; }
			if (y > 0 && lifeData->boardData[(y-1)*lifeData->boardSize.width + (x)] != 0x00) { numNeighbors++; }
			if (x < lifeData->boardSize.width-1 && lifeData->boardData[(y)*lifeData->boardSize.width + (x+1)] != 0x00) { numNeighbors++; }
			if (y < lifeData->boardSize.height-1 && lifeData->boardData[(y+1)*lifeData->boardSize.width + (x)] != 0x00) { numNeighbors++; }
			if (x > 0 && y > 0 && lifeData->boardData[(y-1)*lifeData->boardSize.width + (x-1)] != 0x00) { numNeighbors++; }
			if (x < lifeData->boardSize.width-1 && y > 0 && lifeData->boardData[(y-1)*lifeData->boardSize.width + (x+1)] != 0x00) { numNeighbors++; }
			if (x > 0 && y < lifeData->boardSize.height-1 && lifeData->boardData[(y+1)*lifeData->boardSize.width + (x-1)] != 0x00) { numNeighbors++; }
			if (x < lifeData->boardSize.width-1 && y < lifeData->boardSize.height-1 && lifeData->boardData[(y+1)*lifeData->boardSize.width + (x+1)] != 0x00) { numNeighbors++; }
			
			if (*cell != 0x00)
			{
				if (numNeighbors >= 2 && numNeighbors <= 3) { *newCell = 0x01; }
				else { *newCell = 0x00; }
			}
			else
			{
				if (numNeighbors == 3) { *newCell = 0x01; }
				else { *newCell = 0x00; }
			}
		}
	}
	
	memcpy(lifeData->boardData, lifeData->newBoardData, lifeData->boardSize.width * lifeData->boardSize.height);
}

u8* GetCell(i32 x, i32 y)
{
	if (lifeData->boardData == nullptr) { return nullptr; }
	if (x < 0 || y < 0 || x >= lifeData->boardSize.width || y >= lifeData->boardSize.height) { return nullptr; }
	
	return &lifeData->boardData[y*lifeData->boardSize.x + x];
}

void CreateGameOfLifeObject(v2i center, const char* data, v2i size, bool flipped = false)
{
	v2i halfSize = size / 2;
	for (i32 x = 0; x < size.x; x++)
	{
		for (i32 y = 0; y < size.y; y++)
		{
			char c = data[y*size.width + x];
			if (flipped) { c = data[y*size.width + (size.x-1 - x)]; }
			if (c != '.' && c != ' ')
			{
				v2i pos = center - halfSize + NewVec2i(x, y);
				u8* cell = GetCell(pos.x, pos.y);
				if (cell != nullptr)
				{
					*cell = 0x01;
				}
			}
		}
	}
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void InitializeLifeState()
{
	CreateGameOfLifeBoard();
	
	lifeData->initialized = true;
}

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartLifeState(AppState_t fromState)
{
	//TODO: Reset stuff
}

// +--------------------------------------------------------------+
// |                         Deinitialize                         |
// +--------------------------------------------------------------+
void DeinitializeLifeState()
{
	if (lifeData->boardData != nullptr)
	{
		ArenaPop(mainHeap, lifeData->boardData);
	}
	if (lifeData->newBoardData != nullptr)
	{
		ArenaPop(mainHeap, lifeData->newBoardData);
	}
	
	ClearPointer(lifeData);
}

// +--------------------------------------------------------------+
// |                      Update And Render                       |
// +--------------------------------------------------------------+
void UpdateAndRenderLifeState()
{
	// +--------------------------------------------------------------+
	// |                            Update                            |
	// +--------------------------------------------------------------+
	if (lifeData->boardData != nullptr && lifeData->boardSize.width > 0 && lifeData->boardSize.height > 0)
	{
		// +==============================+
		// |        Recreate Board        |
		// +==============================+
		if (ButtonPressed(Button_Enter))
		{
			CreateGameOfLifeBoard();
		}
		
		// +==============================+
		// |           Simulate           |
		// +==============================+
		static u8 skipCounter = 0;
		if (skipCounter > 0) { skipCounter--; }
		if (ButtonPressed(Button_Z))
		{
			SimulateGameOfLife();
		}
		else if (ButtonDown(Button_X) && skipCounter == 0)
		{
			SimulateGameOfLife();
			skipCounter = 10;
		}
		else if ((ButtonDown(Button_C) || ButtonDown(Button_Space)) && skipCounter == 0)
		{
			SimulateGameOfLife();
			skipCounter = 4;
		}
		else if (ButtonDown(Button_V))
		{
			SimulateGameOfLife();
		}
		else if (ButtonDown(Button_B))
		{
			SimulateGameOfLife();
			SimulateGameOfLife();
			SimulateGameOfLife();
		}
		
		// +==============================+
		// |         Place Cells          |
		// +==============================+
		v2 cellSize = NewVec2(RenderScreenSize.width / lifeData->boardSize.width, RenderScreenSize.height / lifeData->boardSize.height);
		v2i startMouseCellPos = NewVec2i((i32)(RenderMouseStartLeft.x / cellSize.width), (i32)(RenderMouseStartLeft.y / cellSize.height));
		v2i mouseCellPos = NewVec2i((i32)(RenderMousePos.x / cellSize.width), (i32)(RenderMousePos.y / cellSize.height));
		u8* mouseCell = GetCell(mouseCellPos.x, mouseCellPos.y);
		if (ButtonDown(MouseButton_Left))
		{
			*mouseCell = 0x01;
		}
		if (ButtonDown(MouseButton_Right))
		{
			*mouseCell = 0x00;
		}
		
		if (ButtonPressed(Button_Q)) { CreateGameOfLifeObject(mouseCellPos, GOL_Spaceship, GOL_Spaceship_Size); }
		if (ButtonPressed(Button_W)) { CreateGameOfLifeObject(mouseCellPos, GOL_Spaceship, GOL_Spaceship_Size, true); }
		if (ButtonPressed(Button_E)) { CreateGameOfLifeObject(mouseCellPos, GOL_Pentadecathlon, GOL_Pentadecathlon_Size); }
		if (ButtonPressed(Button_R)) { CreateGameOfLifeObject(mouseCellPos, GOL_Pulsar, GOL_Pulsar_Size); }
		if (ButtonPressed(Button_T)) { CreateGameOfLifeObject(mouseCellPos, GOL_Generator, GOL_Generator_Size); }
	}
	
	// +--------------------------------------------------------------+
	// |                            Render                            |
	// +--------------------------------------------------------------+
	{
		// +==============================+
		// |            Setup             |
		// +==============================+
		{
			RcBegin(&app->defaultShader, &app->defaultFont, NewRec(Vec2_Zero, RenderScreenSize));
			RcClearColorBuffer(NewColor(Color_Black));
			RcClearDepthBuffer(1.0f);
		}
		
		// RcDrawCircle(RenderScreenSize/2, RenderScreenSize.height/2, NewColor(Color_LightBlue));
		
		// +==============================+
		// |     Render Game of Life      |
		// +==============================+
		if (lifeData->boardData != nullptr && lifeData->boardSize.width > 0 && lifeData->boardSize.height > 0)
		{
			v2 cellSize = NewVec2(RenderScreenSize.width / lifeData->boardSize.width, RenderScreenSize.height / lifeData->boardSize.height);
			
			rec baseCellRec = NewRec(Vec2_Zero, cellSize);
			for (i32 y = 0; y < lifeData->boardSize.height; y++)
			{
				RcDrawRectangle(NewRec(0, cellSize.height*y, RenderScreenSize.width, 1), NewColor(Color_White));
				for (i32 x = 0; x < lifeData->boardSize.width; x++)
				{
					if (y == 0) { RcDrawRectangle(NewRec(cellSize.width*x, 0, 1, RenderScreenSize.height), NewColor(Color_White)); }
					rec cellRec = NewRec(Vec2Multiply(cellSize, NewVec2((r32)x, (r32)y)), cellSize);
					u8* cell = &lifeData->boardData[y*lifeData->boardSize.width + x];
					
					if (*cell != 0x00)
					{
						RcDrawRectangle(cellRec, NewColor(Color_White));
					}
				}
			}
		}
	}
}
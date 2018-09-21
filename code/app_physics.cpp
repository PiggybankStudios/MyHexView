/*
File:   app_physics.cpp
Author: Taylor Robbins
Date:   09\20\2018
Description: 
	** Holds a little visualization I made for PHYS 122 about Kepler's 3rd Law 
*/

r32 SphereArea(r32 radius)
{
	return 4 * Pi32 * radius * radius;
}

r32 NormalizeRadians(r32 radians)
{
	while (radians < 0) { radians += 2*Pi32; }
	while (radians >= 2*Pi32) { radians -= 2*Pi32; }
	return radians;
}

r32 ToMeters(r32 pixels)
{
	return pixels / 5;
}

r32 ToPixels(r32 meters)
{
	return meters * 5;
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void InitializePhysicsState()
{
	phys->numPlanets = 5;
	phys->planetPositions = PushArray(mainHeap, v2, phys->numPlanets);
	memset(phys->planetPositions, 0x00, sizeof(v2)*phys->numPlanets);
	
	lifeData->initialized = true;
}

// +--------------------------------------------------------------+
// |                            Start                             |
// +--------------------------------------------------------------+
void StartPhysicsState(AppState_t fromState)
{
	//TODO: Reset stuff
}

// +--------------------------------------------------------------+
// |                         Deinitialize                         |
// +--------------------------------------------------------------+
void DeinitializePhysicsState()
{
	if (phys->planetPositions != nullptr)
	{
		ArenaPop(mainHeap, phys->planetPositions);
		phys->planetPositions = nullptr;
	}
	
	ClearPointer(lifeData);
}

// +--------------------------------------------------------------+
// |                      Update And Render                       |
// +--------------------------------------------------------------+
void UpdateAndRenderPhysicsState()
{
	v2 orbitCenter = RenderScreenSize/2;
	r32 orbitRadius = Vec2Length(RenderMousePos - orbitCenter);
	if (orbitRadius <= SUN_RADIUS) { orbitRadius = SUN_RADIUS; }
	
	// +--------------------------------------------------------------+
	// |                            Update                            |
	// +--------------------------------------------------------------+
	// if (true)
	// {
		// if (ButtonDown(MouseButton_Left)) { phys->planetPos = RenderMousePos; }
		if (ButtonPressed(Button_Plus))
		{
			v2* newPositions = PushArray(mainHeap, v2, phys->numPlanets+1);
			memcpy(newPositions, phys->planetPositions, sizeof(v2)*phys->numPlanets);
			newPositions[phys->numPlanets] = Vec2_Zero;
			ArenaPop(mainHeap, phys->planetPositions);
			phys->planetPositions = newPositions;
			phys->numPlanets++;
		}
		if (ButtonPressed(Button_Minus) && phys->numPlanets > 5)
		{
			phys->numPlanets--;
		}
		
		if (ButtonDown(Button_Backspace))
		{
			for (u32 pIndex = 0; pIndex < phys->numPlanets; pIndex++)
			{
				phys->planetPositions[pIndex] = RenderMousePos;
			}
		}
		
		for (u32 pIndex = 0; pIndex < phys->numPlanets; pIndex++)
		{
			v2* posPntr = &phys->planetPositions[pIndex];
			v2 planetPos = *posPntr;
			v2 planetDiffVec = planetPos - orbitCenter;
			if (Vec2Length(planetDiffVec) == 0) { planetDiffVec = Vec2_Right; }
			r32 planetDir = AtanR32(planetDiffVec.y, planetDiffVec.x);
			r32 planetRadius = (orbitRadius-SUN_RADIUS) * ((r32)(pIndex+1) / (r32)(phys->numPlanets)) + SUN_RADIUS;
			r32 radiusMeters = ToMeters(planetRadius);
			r32 sunMass = SphereArea(ToMeters(SUN_RADIUS)) * SUN_DENSITY;
			r32 planetPeriod = SqrtR32((4*Pi32*Pi32*radiusMeters*radiusMeters*radiusMeters)/sunMass); //seconds/revolution
			r32 angleChangePerSecond = (2 * Pi32) / planetPeriod;
			r32 angleChangeForFrame = angleChangePerSecond * (r32)(platform->timeDelta/60);
			planetDir += angleChangeForFrame;
			*posPntr = orbitCenter + NewVec2(CosR32(planetDir), SinR32(planetDir)) * planetRadius;
		}
	// }
	
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
			RcBindNewFont(&app->newFont);
			RcSetFontSize(24);
		}
		
		// RcDrawCircle(RenderScreenSize/2, RenderScreenSize.height/2, NewColor(Color_LightBlue));
		
		for (u32 pIndex = 0; pIndex < phys->numPlanets; pIndex++)
		{
			v2 planetPos = phys->planetPositions[pIndex];
			r32 planetRadius = Vec2Length(planetPos - orbitCenter);
			RcDrawDonut(orbitCenter, planetRadius, planetRadius-3, ColorLerp(NewColor(Color_Red), NewColor(Color_Black), planetRadius/orbitRadius));
			RcDrawLine(orbitCenter, planetPos, 1, ColorTransparent(NewColor(Color_Yellow), 0.5f));
		}
		
		for (u32 pIndex = 0; pIndex < phys->numPlanets; pIndex++)
		{
			v2 planetPos = phys->planetPositions[pIndex];
			RcDrawCircle(planetPos, PLANET_RADIUS, NewColor(Color_White));
		}
		
		RcDrawCircle(orbitCenter, SUN_RADIUS, NewColor(Color_White));
		
		// v2 textPos = orbitCenter + Vec2Normalize(phys->planetPos - orbitCenter) * (SUN_RADIUS + 10);
		// if (textPos.x >= orbitCenter.x) { RcSetFontAlignment(Alignment_Left); }
		// else { RcSetFontAlignment(Alignment_Right); }
		// RcNewPrintString(textPos, NewColor(Color_Orange), "Angle = %.1f", ToDegrees(NormalizeRadians(planetDir)));
		// RcSetFontAlignment(Alignment_Left);
		// textPos = orbitCenter + Vec2Normalize(NewVec2(1, 1)) * (orbitRadius + 10);
		// RcNewPrintString(textPos, NewColor(Color_Orange), "Circumference = %f", (r32)platform->timeDelta);
	}
}


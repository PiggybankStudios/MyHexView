/*
File:   app_physics.h
Author: Taylor Robbins
Date:   09\20\2018
*/

#ifndef _APP_PHYSICS_H
#define _APP_PHYSICS_H

#define SUN_RADIUS 50
#define PLANET_RADIUS 10
#define SUN_DENSITY 100

struct PhysicsData_t
{
	bool initialized;
	
	u32 numPlanets;
	v2* planetPositions;
};

#endif //  _APP_PHYSICS_H

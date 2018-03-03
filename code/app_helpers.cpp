/*
File:   app_helpers.cpp
Author: Taylor Robbins
Date:   03\03\2018
Description: 
	** Holds some helper functions that all of the AppStates have access to
*/

void AppChangeState(AppState_t newState, bool skipInitialization = false, bool skipDeinitialization = false)
{
	app->newAppState = newState;
	app->skipInitialization = skipInitialization;
	app->skipDeinitialization = skipDeinitialization;
}

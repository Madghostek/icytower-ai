#pragma once

#include "definitions.h"

typedef struct {
	double Xpos;
	double Ypos;
	double XSpeed;
	double YSpeed;
	bool isOnGround; //from jumpPhase
	Platform_coincise platforms[7]; //more is never visible on screen
	bool isGameOver;
	uint8_t clockSpeed;
	uint8_t screenOffset; //every visible platform is 80 units apart,
} RLState;

void DecideInputs(RLState*, uint8_t*);
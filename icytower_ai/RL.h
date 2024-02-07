#pragma once

#include "definitions.h"


typedef struct {
	double Xpos;
	double Ypos;
	double XSpeed;
	double YSpeed;
	Platform_coincise platforms[7]; //more is never visible on screen
	double isOnGround; //from jumpPhase
	double isGameOver;
	int8_t clockSpeed;
	double screenOffset; //every visible platform is 80 units apart,
} RLInput;


//vec_t is important because it has aligned allocator and .fit templates need it
typedef std::vector<tiny_dnn::vec_t> StateBuffer; //tensor is vector of vectors of floats
typedef std::vector<tiny_dnn::vec_t> ActionsTaken;


void InitNetwork();

void DecideInputs(RLInput*, uint8_t*);
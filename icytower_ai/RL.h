#pragma once

#include "definitions.h"

typedef union {
	struct {
		float Xpos;
		//float Ypos;
		//float XSpeed;
		//float YSpeed;
		//Platform_coincise platforms[7]; //more is never visible on screen
		//float isOnGround; //from jumpPhase
		//float isGameOver;
		//float clockSpeed;
		//float screenOffset; //every visible platform is 80 units apart,
	};
	float all[1]; //can't know before the union exists :(
} RLInput;

constexpr unsigned inputSize = sizeof(RLInput) / sizeof(float);

// if AI hasn't reached higher floor for 200 frames, kill it
constexpr unsigned maxNoProgressTime = 200;




//vec_t is important because it has aligned allocator and .fit templates need it
typedef std::vector<tiny_dnn::vec_t> StateBuffer; //tensor is vector of vectors of floats
typedef std::vector<tiny_dnn::vec_t> ActionsTaken;


void InitNetwork();

void DecideInputs(RLInput*, uint8_t*);
void PenalizeRecent();
void GoodRecent();
void ResetRecent();
void TrainFakeStates();
void Test_network();
void UpdateQValues();
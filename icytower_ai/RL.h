#pragma once

#include "definitions.h"
#include "tiny_dnn/tinier_dnn.h"

using namespace tiny_dnn;

struct SDecision
{
	vec_t inputs;
	vec_t outputs;
	unsigned actionTaken; //action index
	float reward;
	vec_t nextState;


public:
	SDecision() {} // this is needed to be able to allocate a vector with initial size
	SDecision(vec_t in_vec, vec_t out_vec, unsigned a, float r, vec_t nextState) : inputs(in_vec), outputs(out_vec), actionTaken(a), reward(r), nextState(nextState) {};
};

typedef struct SDecision Decision;

typedef union {
	struct {
		float Xpos;
		//float Ypos;
		//float XSpeed;
		//float YSpeed;
		//Platform_coincise platforms[7]; //more is never visible on screen
		float left_edge;
		float right_edge;
		//float isOnGround; //from jumpPhase
		//float isGameOver;
		//float clockSpeed;
		//float screenOffset; //every visible platform is 80 units apart,
	};
	float all[3]; //can't know before the union exists :(
} RLInput;

constexpr unsigned inputSize = sizeof(RLInput) / sizeof(float);

// if AI hasn't reached higher floor for 200 frames, kill it
constexpr unsigned maxNoProgressTime = 200;




//vec_t is important because it has aligned allocator and .fit templates need it
typedef std::vector<vec_t> StateBuffer; //tensor is vector of vectors of floats
typedef std::vector<vec_t> ActionsTaken;


void InitNetwork();

void DecideInputs(RLInput*, uint8_t*, bool isStart);
void PenalizeRecent();
void GoodRecent();
void TrainFakeStates();
void NormaliseState(RLInput*);
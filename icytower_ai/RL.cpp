#include "pch.h"
#include <stddef.h>
#include <cstdio>
#include <algorithm>

#include "RL.h"

using namespace tiny_dnn;


const static float gamma = 0.6; //q network reward, should be big if state space is big
static float epsilon = 0.9; // random action take, initially tell the network to roam almost randomly, to get some sense about the Q predictions
static float epsilon_min = 0.1; //don't decrease lower
static float decay = 0.999; // epsilon dimnishing
static float lrate = 0.2; // how much to update the old value
//const uint8_t actions[] = { 0,JUMP_INPUT,LEFT_INPUT,RIGHT_INPUT,JUMP_INPUT | LEFT_INPUT,JUMP_INPUT | RIGHT_INPUT };
const uint8_t actions[] = { 0,LEFT_INPUT,RIGHT_INPUT};
constexpr unsigned actionCount = 3;

network<sequential> gNet;
bool initDone = false;


typedef struct S
{
	vec_t inputs;
	vec_t outputs;
	unsigned actionTaken; //action index

public:
	S(vec_t in_vec, vec_t out_vec, unsigned action) : inputs(in_vec), outputs(out_vec), actionTaken(action) {};
} Decision;

std::vector<Decision> recentDecisions;

// after game over, some of the actions taken recently will be weighted negatively
// for now apply positive
template <typename T>
void ConfirmActionsRewards(const StateBuffer& buf, const ActionsTaken& actions, T& net)
{
	std::ofstream myfile;
	myfile.open("adam_tanh_simple5k_cross_entr.csv");
	myfile << "epoch;absolute error sum" << std::endl;
	tiny_dnn::gradient_descent opt;
	int counter = 0;
	net.fit<tiny_dnn::mse>(opt, buf, actions, 4, 5000, [] {}, [&] {
		auto res = net.test(buf);
		float error_sum = 0;
		for (int j = 0; j < 4; ++j)
			error_sum += abs(res[j][0] - actions[j][0]);
		myfile << counter++ << ";" << error_sum << std::endl;
		});
	myfile.close();
}

void InitNetwork()
{
	if (!initDone)
	{
		initDone = true;
		std::cout << "Network init" << std::endl;

		gNet << fully_connected_layer(inputSize, 64) << sigmoid_layer();
		gNet << fully_connected_layer(64, 64) << sigmoid_layer();
		gNet << fully_connected_layer(64, 3, false) << leaky_relu_layer();
	}
	else
	{
		std::cout << "Already initialised\n";
	}

	// the last relu layer is neede, otherwise network is bugged and last layer has 0 weights


	//auto result = net.predict(input)
}

// Recent decisions were bad and caused player to die (or get killed off due to inactivity)
// we don't want that, train network on opposite inputs as correct (which is not ideal)
// Change: give reward for getting close to platform middle instead
void PenalizeRecent(float reward)
{
	tiny_dnn::gradient_descent opt;
	printf("train, dist: %f\n", reward);
	float trueReward = 200/reward;
	int j = recentDecisions.size();
	for (int i = 0; i < std::min(j,20); ++i)
	{
		auto state = std::vector<vec_t>({ recentDecisions[j - i - 1].inputs });
		auto predictions = std::vector<vec_t>({ recentDecisions[j - i - 1].outputs });
		auto actionTaken = recentDecisions[j - i - 1].actionTaken;
		printf("predictions : %f %f %f act: %d\n", predictions[0][0], predictions[0][1], predictions[0][2], actionTaken);
		predictions[0][actionTaken] = predictions[0][actionTaken]*(1-lrate)+trueReward*lrate; //Qlearning
		printf("updated prd : %f %f %f\n", predictions[0][0], predictions[0][1], predictions[0][2]);
		trueReward = trueReward * gamma;
		gNet.fit<mse>(opt, state, predictions, 1, 1);
	}
	
}

void GoodRecent()
{
	tiny_dnn::gradient_descent opt;
	opt.alpha = 0.2;
	for (int i = 0; i < std::min((int)recentDecisions.size(),30); ++i)
	{
		int j = recentDecisions.size();
		printf("train %d\n", i);
		auto state = std::vector<vec_t>({ recentDecisions[j - i - 1].inputs });
		auto action = std::vector<vec_t>({ recentDecisions[j - i - 1].outputs });
		gNet.fit<mse>(opt, state, action, 1, 1);
	}

}

void ResetRecent()
{
	recentDecisions.clear();
}

/// <summary>
/// Runs forward pass through the network
/// </summary>
/// <param name="state">network inputs</param>
/// <param name="keys">place to write the decided inputs</param>
void DecideInputs(RLInput* state, uint8_t* keys)
{
	if (!initDone)
	{
		InitNetwork();
	}
	vec_t inputs;
	for (int i = 0; i < 22; ++i)
		inputs.push_back( state->all[i]);

	unsigned actionIdx;
	auto Qvalues = gNet.predict(inputs);
	auto roll = rand() / (float)RAND_MAX;
	//printf("roll: %f>%f\n", roll,epsilon);
	if (roll > epsilon)
	{
		auto max = std::max_element(Qvalues.begin(), Qvalues.end());
		actionIdx = std::distance(Qvalues.begin(), max); // absolute index of max
	}
	else
	{
		// random action
		actionIdx = rand() % actionCount;
	}
	epsilon *= decay;
	epsilon = std::max(epsilon_min, epsilon);

	*keys = actions[actionIdx];

	recentDecisions.push_back({ inputs,Qvalues, actionIdx });
}


void DecideInputsOld(RLInput* state, uint8_t* keys)
{
	if (state->isGameOver)
	{
		//printf("game over, so space\n");
		*keys |= JUMP_INPUT;
		return;
	}
	if (state->XSpeed>=0)
	{
		*keys |= RIGHT_INPUT;
	}
	else
		*keys |= LEFT_INPUT;
	if (state->isOnGround)
	{
		*keys |= JUMP_INPUT;
		//printf("Is on ground, so jump\n");
	}

}

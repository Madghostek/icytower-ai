#include "pch.h"
#include <stddef.h>
#include <cstdio>
#include <algorithm>
#include <chrono>

#include "RL.h"
#include "ReplayBuffer.hpp"

using namespace tiny_dnn;


const static float gamma = 0.6f; //q network reward, should be big if state space is big
static float epsilon = 0.9f; // random action take, initially tell the network to roam almost randomly, to get some sense about the Q predictions
static float epsilon_min = 0.2f; //don't decrease lower
static float decay = 0.95f; // epsilon dimnishing
//const uint8_t actions[] = { 0,JUMP_INPUT,LEFT_INPUT,RIGHT_INPUT,JUMP_INPUT | LEFT_INPUT,JUMP_INPUT | RIGHT_INPUT };
const uint8_t actions[] = { 0,LEFT_INPUT,RIGHT_INPUT};
constexpr unsigned actionCount = 3;

float Xnormalisation = 550.f;
float platformNormalisation = 35.f;

network<sequential> gNet;
bool initDone = false;

ReplayBuffer<Decision> recentDecisions;

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
		set_random_seed(3);
		initDone = true;
		std::cout << "Network init" << std::endl;

		gNet << fully_connected_layer(3, 16) << tanh_layer()
			<< fully_connected_layer(16, 16) << tanh_layer()
			<< fully_connected_layer(16, 16) << tanh_layer()
			<< fully_connected_layer(16, 16) << tanh_layer()
			 << fully_connected_layer(16, 3);
		//gNet.weight_init(weight_init::constant(1.0));
		gNet.init_weight(); //THE MOST IMPORTANT LINE IN WHOLE FILE OH GOD
	}
	else
	{
		std::cout << "Already initialised\n";
	}

	// the last relu layer is neede, otherwise network is bugged and last layer has 0 weights


	//auto result = net.predict(input)
}

void TestStateSeparation()
{
	std::cout << "Testing state separation:\n";
	for (auto x : { 100.f, 200.f,300.f,400.f,500.f,550.f })
	{
		vec_t test1 = { x / Xnormalisation, 0.685f, 0.917f };
		auto res = gNet.predict(test1);
		printf("x: %f, preds: %f %f %f\n", test1[0], res[0], res[1], res[2]);

	}
}

//void TrainFakeStates()
//{
//	float mid = 450;
//	tiny_dnn::adam opt;
//
//	tensor_t states{};
//	tensor_t outputs{};
//
//	// changing order of those is interesting, network remember better the most recent one,
//	// so for example right now it will overshoot the goal, and swapping the loops causes undershooting.
//	//for (int i = 0; i < 100; ++i)
//	//{
//	//	float randomX = (rand() % (550 - 450)) + 450; //550-450 range
//	//	states.push_back({ randomX / Xnormalisation });
//	//	outputs.push_back({ {-10,10,-10} }); // left best
//
//	//}
//	//for (int i = 0; i < 100; ++i)
//	//{
//	//	float randomX = (rand() % (450 - 90)) + 90; //85-450 range
//	//	states.push_back({ randomX/ Xnormalisation });
//	//	outputs.push_back({ {-10,-10,10} }); //right best
//	//}
//
//	std::vector<int> side;
//
//	for (int i = 0; i < 100; ++i)
//	{
//		side.push_back(1);
//	}
//	for (int i = 0; i < 100; ++i) {
//		side.push_back(0);
//
//	}
//	auto rng = std::default_random_engine{};
//	std::shuffle(side.begin(), side.end(), rng);
//
//	for (auto& s: side){
//		if (s)
//		{
//		float randomX = (rand() % (550 - 450)) + 450; //550-450 range
//		states.push_back({ randomX / Xnormalisation });
//		outputs.push_back({ {-10,10,-10} }); // left best
//		}
//		else {
//		float randomX = (rand() % (450 - 90)) + 90; //85-450 range
//		states.push_back({ randomX / Xnormalisation });
//		outputs.push_back({ {-10,-10,10} }); //right best
//		}
//
//	}
//	unsigned batch = 0;
//	std::cout << "Training on fake states and outputs...\n";
//	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//		gNet.fit<mse>(opt, states, outputs, 8,50, [&]() {
//			std::cout << "batch" << batch++ << std::endl;
//			TestStateSeparation();
//			}, [&]() {});
//	std::cout << "Trained for: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << std::endl;
//
//
//}

// Recent decisions were bad and caused player to die (or get killed off due to inactivity)
// we don't want that, train network on opposite inputs as correct (which is not ideal)
// Change: give reward for getting close to platform middle instead
tiny_dnn::adam opt;
void PenalizeRecent()
{
	opt.alpha*=0.99;
	constexpr int randomSize = 50;


	//opt.alpha *= 0.3;// *epsilon; //slow down!! use random walk epsilion here, so that it slows down even more when it's close to optimum
	printf("train\n");
	int j = recentDecisions.size();
	//we don't have next state at last element (i=0) and first??

	std::vector<unsigned> randomActions;
	for (int i = 0; i < randomSize; ++i)
	{
		randomActions.push_back(rand()%recentDecisions.size());
	}

	std::vector<vec_t> states;
	std::vector<vec_t> targets;
	for (auto i: randomActions)
	{
		auto& experience = recentDecisions[i];
		if (experience.nextState.size() == 0) continue; //happened to roll the terminal state, no need to train on it for now
		const auto &state = tensor_t({ experience.inputs });
		auto predictions = tensor_t({ experience.outputs });
		const auto &actionTaken = experience.actionTaken;
		const auto &reward = experience.reward;
		const auto &nextState = tensor_t({ experience.nextState });

		auto Qvalue_nextState = gNet.predict(nextState); 
		auto bestFutureQvalue = *std::max_element(Qvalue_nextState[0].begin(), Qvalue_nextState[0].end());

		printf("(%d) X: %f\n", i, state[0][0]);
		printf("predictions: %f %f %f, x: %f, act: %d, r: %f\n", predictions[0][0], predictions[0][1], predictions[0][2], state[0][0], actionTaken, reward);
		predictions[0][actionTaken] = reward + gamma * bestFutureQvalue; //Qlearning, but for a network (no learning rate here)
		printf("updated prd : %f %f %f\n\n", predictions[0][0], predictions[0][1], predictions[0][2]);
		states.push_back(experience.inputs);
		targets.push_back(predictions[0]);
	}

	gNet.fit<mse>(opt, states, targets, 2, 1);
	printf("Train done\n");
	TestStateSeparation();

	epsilon *= decay;
	epsilon = std::max(epsilon_min, epsilon);
	std::cout << "epsilon: " << epsilon << std::endl;
	std::cout << "alpha: " << opt.alpha << std::endl;
	
}

//void GoodRecent()
//{
//	tiny_dnn::gradient_descent opt;
//	opt.alpha = 0.2f;
//	for (int i = 0; i < std::min((int)recentDecisions.size(),30); ++i)
//	{
//		int j = recentDecisions.size();
//		printf("train %d\n", i);
//		auto state = std::vector<vec_t>({ recentDecisions[j - i - 1].inputs });
//		auto action = std::vector<vec_t>({ recentDecisions[j - i - 1].outputs });
//		gNet.fit<mse>(opt, state, action, 1, 1);
//	}
//
//}


void NormaliseState(RLInput* state)
{
	state->Xpos /= Xnormalisation;
	state->left_edge /= platformNormalisation;
	state->right_edge /= platformNormalisation;
}

/// <summary>
/// Runs forward pass through the network
/// </summary>
/// <param name="state">network inputs</param>
/// <param name="keys">place to write the decided inputs</param>
void DecideInputs(RLInput* state, uint8_t* keys, bool isStart)
{
	if (!initDone)
	{
		InitNetwork();
	}
	vec_t inputs;
	for (int i = 0; i < 3; ++i)
		inputs.push_back( state->all[i]);

	if (!isStart)
		recentDecisions.provideNextState(inputs);

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

	*keys = actions[actionIdx];

	float reward = 0.f;
	if (state->Xpos<state->right_edge && state->Xpos>state->left_edge)
	{
		if (actionIdx == 0) // no input better
			reward = 100.f;
		else
			reward = 20.f;
	}
	else if (state->Xpos > state->right_edge)
	{
		if (actionIdx == 1)
			reward = 5.f;
		else
			reward = -5.f;
	}
		
	else if (state->Xpos < state->left_edge)
	{
		if (actionIdx == 2)
			reward = 5.f;
		else
			reward = -5.f;
	}

	recentDecisions.push_back({ inputs, Qvalues, actionIdx, reward, {} });
}

#include "pch.h"
#include <stddef.h>
#include <cstdio>
#include <algorithm>

#include "RL.h"

using namespace tiny_dnn;


const static float gamma = 0.6; //q network reward, should be big if state space is big
static float epsilon = 0.9; // random action take, initially tell the network to roam almost randomly, to get some sense about the Q predictions
static float epsilon_min = 0.1; //don't decrease lower
static float decay = 0.9; // epsilon dimnishing, every step (not epoch... is this good?)
static float lrate = 0.2; // how much to update the old value
//const uint8_t actions[] = { 0,JUMP_INPUT,LEFT_INPUT,RIGHT_INPUT,JUMP_INPUT | LEFT_INPUT,JUMP_INPUT | RIGHT_INPUT };
const uint8_t actions[] = { 0,LEFT_INPUT,RIGHT_INPUT};
constexpr unsigned actionCount = 3;

float Xnormalisation = 600.f;
float mid = 450;

//network<sequential> gNet;
//bool initDone = false;
// --- tabular q learning ---
constexpr unsigned binCount = 64;

typedef float ActionRewards[3];
ActionRewards QTable[binCount]{};



typedef struct S
{
	vec_t inputs;
	//vec_t outputs; //only for neural net
	unsigned actionTaken; //action index
	float reward;
	vec_t nextState;


public:
	//vec_t out_vec, outputs(out_vec)
	S(vec_t in_vec, unsigned a, float r, vec_t nextState) : inputs(in_vec), actionTaken(a), reward(r), nextState(nextState) {};
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

//void InitNetwork()
//{
//	if (!initDone)
//	{
//		set_random_seed(3);
//		initDone = true;
//		std::cout << "Network init" << std::endl;
//
//		gNet << fully_connected_layer(1, 16) << tanh_layer()
//			 << fully_connected_layer(16, 16) << tanh_layer()
//			 << fully_connected_layer(16, 3);
//		//gNet.weight_init(weight_init::constant(1.0));
//		gNet.init_weight(); //THE MOST IMPORTANT LINE IN WHOLE FILE OH GOD
//	}
//	else
//	{
//		std::cout << "Already initialised\n";
//	}
//
//	// the last relu layer is neede, otherwise network is bugged and last layer has 0 weights
//
//
//	//auto result = net.predict(input)
//}

//void TrainFakeStates()
//{
//	tiny_dnn::gradient_descent opt;
//
//	tensor_t states{};
//	tensor_t outputs{};
//
//	for (int i = 0; i < 100; ++i)
//	{
//		float randomX = (rand() % (450 - 90)) + 90; //85-450 range
//		states.push_back({ randomX/ Xnormalisation });
//		outputs.push_back({ {-10,-10,10} }); //right best
//	}
//	for (int i = 0; i < 100; ++i)
//	{
//		float randomX = (rand() % (550 - 450)) + 450; //550-450 range
//		states.push_back({ randomX/ Xnormalisation });
//		outputs.push_back({ {-10,10,-10} }); // left best
//
//	}
//	std::cout << "Training on fake states and outputs...\n";
//	gNet.fit<mse>(opt, states, outputs, 1, 50);
//
//	std::cout << "Testing state separation:\n";
//	for (auto x : { 200.f,300.f,400.f,500.f, 161.565689f})
//	{
//		vec_t test1 = { x/ Xnormalisation };
//		auto res = gNet.predict(test1);
//		printf("x: %f, preds: %f %f %f\n", x, res[0], res[1], res[2]);
//
//	}
//
//
//}

// Recent decisions were bad and caused player to die (or get killed off due to inactivity)
// we don't want that, train network on opposite inputs as correct (which is not ideal)
// Change: give reward for getting close to platform middle instead
//void PenalizeRecent()
//{
//	tiny_dnn::gradient_descent opt;
//	printf("train\n");
//	int j = recentDecisions.size();
//	//we don't have next state at last element (i=0) and first??
//
//	std::vector<unsigned> randomActions;
//	for (int i = 0; i < 200; ++i)
//	{
//		randomActions.push_back((rand()% (recentDecisions.size() - 2))+1);
//	}
//	for (auto i: randomActions)
//	{
//		const auto &state = tensor_t({ recentDecisions[j - i - 1].inputs });
//		auto predictions = tensor_t({ recentDecisions[j - i - 1].outputs });
//		const auto &actionTaken = recentDecisions[j - i - 1].actionTaken;
//		const auto &reward = recentDecisions[j - i - 1].reward;
//		const auto &nextState = tensor_t({ recentDecisions[j - i - 1].nextState });
//
//		auto Qvalue_nextState = gNet.predict(nextState); 
//		auto bestFutureQvalue = *std::max_element(Qvalue_nextState[0].begin(), Qvalue_nextState[0].end());
//
//		printf("(%d) X: %f\n", i, state[0][0]);
//		printf("predictions: %f %f %f, x: %f, act: %d, r: %f\n", predictions[0][0], predictions[0][1], predictions[0][2], state[0][0], actionTaken, reward);
//		//predictions[0][actionTaken] = predictions[0][actionTaken]*(1-lrate)+(reward + gamma*bestFutureQvalue)*lrate; //Qlearning
//		// no q-learning
//		predictions[0][actionTaken] = reward;
//
//		printf("updated prd : %f %f %f\n\n", predictions[0][0], predictions[0][1], predictions[0][2]);
//		gNet.fit<mse>(opt, state, predictions, 1, 1);
//	}
//
//	printf("E: %f\n", epsilon);
//	epsilon *= decay;
//	epsilon = std::max(epsilon_min, epsilon);
//	
//}

//void GoodRecent()
//{
//	tiny_dnn::gradient_descent opt;
//	opt.alpha = 0.2;
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

ActionRewards* GetQValues(float XPos)
{
	unsigned binNumber = XPos * binCount;
	return &QTable[binNumber];
}

/// <summary>
/// Takes NORMALISED XPos
/// </summary>
/// <param name="XPos"></param>
/// <param name="action"></param>
/// <param name="reward"></param>
/// <param name="nextX"></param>
void SetQValues(float XPos, unsigned action, float reward, float nextX)
{
	unsigned binNumber = XPos*binCount;

	ActionRewards* Qvalue_nextState = GetQValues(nextX);
	// rip DRY
	float maxFutureQValue = -INFINITY;
	unsigned best;
	for (unsigned i = 0; i < 3; ++i) {
		if (*(Qvalue_nextState)[i] > maxFutureQValue) {
			maxFutureQValue = *(Qvalue_nextState)[i];
		}
	}

	QTable[binNumber][action] = QTable[binNumber][action] * (1 - lrate) + (reward + gamma * maxFutureQValue) * lrate; //Qlearning
}

unsigned GetBestArg(ActionRewards* arr, size_t size=3) {
	float maxReward = -INFINITY;
	unsigned best;
	for (unsigned i = 0; i < size; ++i) {
		if ((* arr)[i] > maxReward) {
			maxReward = (*arr)[i];
			best = i;
		}
	}
	return best; //dumbass warning!!
}

void UpdateQValues(){
	for (auto &action : recentDecisions) {
		if (action.nextState.size())
			SetQValues(action.inputs[0], action.actionTaken, action.reward, action.nextState[0]);
	}
	epsilon *= decay;
	std::cout << "epsilon: " << epsilon << std::endl;
}

void ResetRecent()
{
	if (recentDecisions.size() > 10000)
	{
		recentDecisions.erase(recentDecisions.begin(), recentDecisions.begin()+1000);
		std::cout << "Erased 1000 oldest decisions\n";
	}
}

/// <summary>
/// Runs forward pass through the network
/// </summary>
/// <param name="state">network inputs</param>
/// <param name="keys">place to write the decided inputs</param>
void DecideInputs(RLInput* state, uint8_t* keys)
{
	//if (!initDone)
	//{
	//	InitNetwork();
	//}
	vec_t inputs;
	for (int i = 0; i < 1; ++i)
		inputs.push_back( state->all[i]/Xnormalisation);

	if (recentDecisions.size()>1)
		recentDecisions.back().nextState = inputs; //update previous experience with current state

	unsigned actionIdx = 0;;
	//auto Qvalues = gNet.predict(inputs);
	auto roll = rand() / (float)RAND_MAX;
	//printf("roll: %f>%f\n", roll,epsilon);
	if (roll > epsilon)
	{
		auto Qvalues = GetQValues(state->Xpos/ Xnormalisation);
		actionIdx = GetBestArg(Qvalues);
		//auto max = std::max_element(Qvalues.begin(), Qvalues.end());
		//actionIdx = std::distance(Qvalues.begin(), max); // absolute index of max
	}
	else
	{
		// random action
		actionIdx = rand() % actionCount;
	}

	*keys = actions[actionIdx];

	float reward = 200 - abs(state->Xpos - mid);
	if (actionIdx == 1 && state->Xpos < 450)
		reward += -1000.f;
	else if (actionIdx == 2 && state->Xpos > 450)
		reward += -1000.f;
	else if (actionIdx == 1 && state->Xpos > 450)
		reward += 1000.f;
	else if (actionIdx == 2 && state->Xpos < 450)
		reward += 1000.f;
	else if (actionIdx == 0 && abs(state->Xpos - mid) > 20)
		reward = -10000.f;

	// ,Qvalues
	recentDecisions.push_back({ inputs, actionIdx, reward,{}}); //next state will be filled at next step
}


//void DecideInputsOld(RLInput* state, uint8_t* keys)
//{
//	if (state->isGameOver)
//	{
//		//printf("game over, so space\n");
//		*keys |= JUMP_INPUT;
//		return;
//	}
//	if (state->XSpeed>=0)
//	{
//		*keys |= RIGHT_INPUT;
//	}
//	else
//		*keys |= LEFT_INPUT;
//	if (state->isOnGround)
//	{
//		*keys |= JUMP_INPUT;
//		//printf("Is on ground, so jump\n");
//	}
//
//}

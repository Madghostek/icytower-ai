#include "pch.h"
#include <stddef.h>
#include <cstdio>
#include <algorithm>

#include "RL.h"

using namespace tiny_dnn;

//singleton
class AI
{
public:
	static network<sequential> Get() {
		static network<sequential> _instance = make_mlp<activation::tanh>({ inputSize, 20, 16, 8, 3 });;
		return _instance;

	}

	AI() = delete;
	AI(const AI&) = delete;
	AI& operator=(const AI&);
	~AI();
};

typedef struct S
{
	vec_t inputs;
	vec_t outputs;

public:
	S(vec_t in_vec, vec_t out_vec) : inputs(in_vec), outputs(out_vec) {};
} Decision;

std::deque<Decision> recentDecisions;

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
	std::cout << "Network init" << std::endl;

	//net.weight_init(weight_init::constant(0.2));

	//auto result = net.predict(input)
}

// Recent decisions were bad and caused player to die (or get killed off due to inactivity)
// we don't want that, train network on opposite inputs as correct (which is not ideal)
void PenalizeRecent()
{
	tiny_dnn::gradient_descent opt;
	opt.alpha = 0.001;
	for (int i = 0; i < std::min((int)recentDecisions.size(),20); ++i)
	{
		int j = recentDecisions.size();
		printf("train %d\n", i);
		auto state = std::vector<vec_t>({ recentDecisions[j-i-1].inputs });
		auto temp = recentDecisions[j-i-1].outputs;
		temp[0] = 1 - temp[0];
		temp[1] = 1 - temp[1];
		temp[2] = 1 - temp[2];
		auto action = std::vector<vec_t>({ temp });
		AI::Get().fit<mse>(opt, state, action, 1, 1);
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
		AI::Get().fit<mse>(opt, state, action, 1, 20);
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
	vec_t inputs;
	for (int i = 0; i < 22; ++i)
		inputs.push_back( state->all[i]);


	auto result = AI::Get().predict(inputs);

	recentDecisions.push_back({ inputs,result });

	// turn into bools
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	int right = r < std::max(0.f,result[0]);
	r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	int left = r < std::max(0.f, result[1]);
	r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	int jump = r < std::max(0.f, result[2]);

	*keys |= JUMP_INPUT & -jump;
	*keys |= LEFT_INPUT & -left;
	*keys |= RIGHT_INPUT & -right;
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

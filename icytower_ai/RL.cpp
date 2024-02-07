#include "pch.h"
#include <stddef.h>
#include <cstdio>
#include <algorithm>

#include "RL.h"

// after game over, some of the actions taken recently will be weighted negatively
// for now apply positive
template <typename T>
void ConfirmActionsRewards(const StateBuffer& buf, const ActionsTaken& actions, T& net)
{
	tiny_dnn::gradient_descent opt;
	net.fit<tiny_dnn::mse>(opt, buf, actions,1,2000);
}

void InitNetwork()
{
	using namespace tiny_dnn;
	auto net = make_mlp<activation::tanh>({ 2,2,1 });


	const std::vector<tensor_t> input = { {{0,0}},{{0,1}},{{1,0}},{{1,1}} };

	StateBuffer test = { {0,0},{0,1},{1,0},{1,1} };
	ActionsTaken acts = { {0},{1},{1},{0} };

	ConfirmActionsRewards<decltype(net)>(test, acts, net);

	auto result = net.predict(input);

	std::cout << "result count: " << result.size() << std::endl;
	unsigned result_index = 0; //no c++20 so sad
	for ( auto& result : result)
	{
		unsigned node_index = 0;
		for (auto& outnode : result)
		{
			std::cout << "Result #" << result_index << " node # " << node_index << ": " << outnode[0] << std::endl;
			++node_index;
		}
		++result_index;
	}
}


void DecideInputs(RLInput* state, uint8_t* keys)
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

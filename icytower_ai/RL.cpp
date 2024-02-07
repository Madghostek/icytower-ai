#include <stddef.h>
#include <stdio.h>

#include "RL.h"

void DecideInputs(RLState* state, uint8_t* keys)
{
	//printf("State: %d\n", state->screenOffset);
	//for (int i = 0; i < 7; ++i)
	//{
	//printf("Platform %d: (%d,%d)\n", i, state->platforms[i].left_edge, state->platforms[i].right_edge); //operator precedence
	//}

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

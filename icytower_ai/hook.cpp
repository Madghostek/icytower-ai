#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include "hook.h"
#include "definitions.h"
#include "RL.h"

#define KEY(a) (GetAsyncKeyState(a) & 0x8000)

GameState** randomInit = (GameState**)0x004CB920;
unsigned* randomIndex = (unsigned*)0x004CB908;

GameState* gameState = NULL; // run this after game init: randomInit[*randomIndex];

Platform(*platformsptr)[platformCount] = (Platform(*)[platformCount])0x004CC8C0;

uint8_t* space_pressed_menu = (uint8_t*)0x004E33CB;
uint32_t* screenHeight = (uint32_t*)0x004CCBC0;
int8_t* clockSpeed = (int8_t *)0x006FF890; // from -1 to 5

// at [esp] is some struct, then at [esp]+0x20 there is key data
// the function at 0x00401520 normally checks if left key is pressed, keys status is the param on stack
//int(__cdecl* hookPoint)(KeyStates*) = (int(__cdecl*)(KeyStates*))0x00408223; // E8 (F8 92 FF FF): call 0x00401520

// new hook, right after calling keys update when it matters
// hook function must return `randomIndex`, because there was a MOV EAX,[randomIndex]
int(__cdecl* hookPoint)(KeyStates*) = (int(__cdecl*)(KeyStates*))0x0040836e;


void PrintPlatform(int index, Platform p)
{
	if (p.sign_text != 0)
		printf("Platform %d: (%d,%d) %d\n", index, p.left_edge, p.right_edge, p.sign_text); //operator precedence
	else
		printf("Platform %d: (%d,%d)\n", index, p.left_edge, p.right_edge); //operator precedence

}

Platform copy[platformCount];
int subPlatform = 0;

void GetPlatforms(Platform_coincise* platforms_coincise)
{
	//if (memcmp(copy, *platformsptr, sizeof(copy)))
	//{
	int j = 0;
	//memcpy(copy, *platformsptr, sizeof(copy));
	for (int i = 0; i < platformCount; ++i)
	{
		Platform p = (*platformsptr)[i];
		if (!(*platformsptr)[i].disabled)
		{
			platforms_coincise[j].left_edge = (*platformsptr)[i].left_edge;
			platforms_coincise[j].right_edge = (*platformsptr)[i].right_edge;
			j += 1;
		}
	}
}

int __cdecl HookInput(KeyStates* keyStates)
{
	printf("keys before: %d\n", keyStates->keys);

	//bool isleft = keyStates->keys & 1;
	//bool isright = keyStates->keys & 2;

	//keyStates->keys &= ~0x3;
	//keyStates->keys |= (isleft << 1) | isright;

	//printf("HOOK call %d\n", TASState);

	static int TASState = 0;

	if (KEY(VK_OEM_1) and !TASState)
	{
		TASState = 1;
	}

	while (TASState)
	{
		if (KEY(VK_OEM_1) && TASState == 1)
		{
			TASState = 2;
		}
		if (!KEY(VK_OEM_1) && TASState == 2)
		{
			TASState = 1;
			break;
		}
		if (KEY(VK_NUMPAD1))
		{
			TASState = 0;
		}
	}



	if (gameState->gameOverHeight>0)
	{
		// game over, skip quickly and tell RL
		//*space_pressed_menu = 0xFF; //always skip
		gameState->gameOverHeight = 0x500;
		return *randomIndex;
	}
	
	RLState state{}; //at least platforms need to be filled with 0s
	state.isOnGround = !gameState->jumpPhase;
	GetPlatforms(state.platforms);
	state.Xpos = gameState->Xpos;
	state.Ypos = gameState->Ypos;
	state.XSpeed = gameState->XSpeed;
	state.YSpeed = gameState->YSpeed;
	state.isGameOver = gameState->gameOverHeight;
	state.clockSpeed = *clockSpeed;
	state.screenOffset = *screenHeight % 80;

	DecideInputs(&state, &keyStates->keys);

	if (KEY(VK_NUMPAD0))
	{
		keyStates->keys ^= JUMP_INPUT;
	}
	
	printf("Decided keys: %d\n", keyStates->keys);

	// do not touch, old code
	return *randomIndex;
}

// for old one
//void DoHook()
//{
//	uint32_t diff = (uint32_t)HookInput - (uint32_t)hookPoint-5;// should be 0x6EA58DE2, -5 because call apparently is relative to next instruction
//	printf("Overwriting call destination to %p (diff is %x)\n", HookInput, diff);
//
//	// first cast to 1 byte pointer, move 1 byte forward, then overwrite the call arg (diff between PC and destination)
//	*(uint32_t*)((uint8_t*)(hookPoint)+1) = (uint32_t)diff;
//}

void DoHook()
{
	uint32_t diff = (uint32_t)HookInput - (uint32_t)hookPoint - 5;// should be 0x6EA58DE2, -5 because call apparently is relative to next instruction
	printf("Overwriting call destination to %p (diff is %x)\n", HookInput, diff);

	// first cast to 1 byte pointer, move 1 byte forward, then overwrite the call arg (diff between PC and destination)
	*(uint8_t*)hookPoint = 0xe8; //call ...
	*(uint32_t*)((uint8_t*)(hookPoint)+1) = (uint32_t)diff;
}

void PrepareVariables(HWND hwnd)
{
	//disable menu hold timer
	*(uint8_t*)(0x00412ddc) = 1;

	gameState = randomInit[*randomIndex];
	memcpy(copy, *platformsptr, sizeof(copy));
}
#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include "definitions.h"

#define KEY(a) (GetAsyncKeyState(a) & 0x8000)

void PrintPlatform(int index, Platform p)
{
	if (p.sign_text != 0)
		printf("Platform %d: (%d,%d) %d\n", index, p.left_edge, p.right_edge, p.sign_text); //operator precedence
	else
		printf("Platform %d: (%d,%d)\n", index, p.left_edge, p.right_edge); //operator precedence

}

Platform copy[platformCount];

int __cdecl HookInput(KeyStates* keyStates)
{
	//printf("%d\n", keyStates->keys);

	//bool isleft = keyStates->keys & 1;
	//bool isright = keyStates->keys & 2;

	//keyStates->keys &= ~0x3;
	//keyStates->keys |= (isleft << 1) | isright;

	//printf("HOOK call %d\n", TASState);

		// prints current platrofms by watching for updates
	if (memcmp(copy, *platformsptr, sizeof(copy)))
	{
		memcpy(copy, *platformsptr, sizeof(copy));
		for (int i = 0; i < platformCount; ++i)
			PrintPlatform(i, (*platformsptr)[i]);
	}


	keyStates->keys &= 16;
	// do not touch
	return (int)(keyStates->keys & 1);
}

void DoHook()
{
	uint32_t diff = (uint32_t)HookInput - (uint32_t)hookPoint-5;// should be 0x6EA58DE2, -5 because call apparently is relative to next instruction
	printf("Overwriting call destination to %p (diff is %x)\n", HookInput, diff);

	// first cast to 1 byte pointer, move 1 byte forward, then overwrite the call arg (diff between PC and destination)
	*(uint32_t*)((uint8_t*)(hookPoint)+1) = (uint32_t)diff;
}

void PrepareVariables(HWND hwnd)
{
	gameState = randomInit[*randomIndex];
	memcpy(copy, *platformsptr, sizeof(copy));
}
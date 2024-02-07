#include <windows.h>
#include <stdio.h>

#include "main.h"
#include "definitions.h"

#define KEY(a) (GetAsyncKeyState(a) & 0x8000)

void PrintPlatform(int index, Platform p)
{
	if (p.sign_text!=0)
		printf("Platform %d: (%d,%d) %d\n", index, p.left_edge, p.right_edge, p.sign_text); //operator precedence
	else
		printf("Platform %d: (%d,%d)\n", index, p.left_edge, p.right_edge); //operator precedence

}

void MemoryReader(HWND hwnd)
{
	GameState* gameState = randomInit[*randomIndex];

	// prints current platrofms by watching for updates
	Platform copy[platformCount];
	memcpy(copy, *platformsptr, sizeof(copy));
	while (!KEY(VK_OEM_2)) //backslash
	{
		if (memcmp(copy, *platformsptr, sizeof(copy)))
		{
			memcpy(copy, *platformsptr, sizeof(copy));
			for (int i = 0; i < platformCount; ++i)
				PrintPlatform(i, (*platformsptr)[i]);
		}

	}
}
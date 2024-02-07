#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define KEY(a) (GetAsyncKeyState(a) & 0x8000)

typedef struct
{
	uint32_t pad[8];
	uint8_t keys;
} KeyStates;

int TASState = 2;

// at [esp] is pointer to some struct, then at [esp]+0x20 there is key data
// the function at 0x00401520 normally checks if left key is pressed, keys status is the param on stack
int (__cdecl * hookPoint)(KeyStates*) = (int(__cdecl*)(KeyStates*))0x00408223; // E8 (F8 92 FF FF): call 0x00401520

int __cdecl HookInput(KeyStates* keyStates)
{
	//printf("%d\n", keyStates->keys);

	//bool isleft = keyStates->keys & 1;
	//bool isright = keyStates->keys & 2;

	//keyStates->keys &= ~0x3;
	//keyStates->keys |= (isleft << 1) | isright;

	//printf("HOOK call %d\n", TASState);
	while (TASState)
	{
		if (KEY(0x5A) && TASState==2)
		{
			TASState = 1;
			break;
		}
		if (!KEY(90) && TASState == 1)
		{
			TASState = 2;
		}
		Sleep(10);
	}

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
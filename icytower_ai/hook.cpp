#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include <errno.h>
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
uint32_t* multiPurposeTimer = (uint32_t*)0x004D2D80;

int32_t* redrawModulo = (int32_t*)0x0040eb23; // set to n to draw every nth frame, negative will be the same as positive

uint8_t* callReplayMenu = (uint8_t*)0x004103da;

uint8_t* removeOutFade = (uint8_t*)0x00440C4C;
uint8_t* removeInFade = (uint8_t*)0x00440c2c;

uint8_t* removeLog = (uint8_t*)0x00403d30;

uint8_t* fixFclosePoint = (uint8_t*)0x0401f55;   // here is  MOV EAX, 0x4ac029
uint8_t* fixFclosePoint2 = (uint8_t*)0x004027cf; // here is  MOV EDX ,dword ptr [EDI  + 0x4e4 ]


uint8_t* msvcrt_fclose = (uint8_t *)0x0049C1D0;

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
	//printf("keys before: %d\n", keyStates->keys);

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


	static bool gameOver = false;

	static int gameNo = 0;

	if (gameState->gameOverHeight>0)
	{
		if (!gameOver)
		{
			gameOver = true;
			printf("%d Game over, floor %d combo %d\n", gameNo++, gameState->floor, gameState->maxCombo);
		}
		if (gameState->maxCombo < 100) //if interesting, don't skip end menu
		{
			// game over, skip quickly and tell RL
			*space_pressed_menu = 0xFF; //always skip
			gameState->gameOverHeight = 0x500;
		}
		return *randomIndex;
	}
	else if (gameOver)
	{
		gameOver = false;
		printf("Game restarted\n");
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
	
	//printf("Decided keys: %d\n", keyStates->keys);

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

void __declspec(naked) FixFclose1() //FILE* fp will be in EDI
{
	__asm {
		push edi;
		call msvcrt_fclose;
		pop edi; //edi is not needed anyway because there was the forgotten file pointer
		mov eax, 0x4ac029;
		ret;
	}
}

void __declspec(naked) FixFclose2() //FILE* fp will be in ECX
{
	__asm {
		push edi;
		push ecx;
		call msvcrt_fclose;
		pop ecx;
		pop edi; //fclose garbles edi
		mov edx, dword ptr[edi + 0x4e4];
		ret;
	}
}

void DoHook()
{
	uint32_t diff = (uint32_t)HookInput - (uint32_t)hookPoint - 5;// should be 0x6EA58DE2, -5 because call apparently is relative to next instruction
	printf("Overwriting call destination to %p (diff is %x)\n", HookInput, diff);

	// first cast to 1 byte pointer, move 1 byte forward, then overwrite the call arg (diff between PC and destination)
	*(uint8_t*)hookPoint = 0xe8; //call ...
	*(uint32_t*)((uint8_t*)(hookPoint)+1) = (uint32_t)diff;

	// patch out fps limit
	// high modulo values crash, because animation updates don't expect so fast frames
	// but when it's so high it never redraws it never crashes :)))
	*redrawModulo = 0x7FFFFFFF; //mov ebx,xxxx <-- frame redraw modulo

	// Don't show replay menu, always play again
	// TODO: Does this save previous attempt?
	// mov eax, 00000001 - option 1
	*callReplayMenu = 0xB8; //mov eax
	*(uint32_t*)(callReplayMenu + 1) = 1; //4 bytes

	//remove out fade
	*removeOutFade = 0xC3; //ret
	*removeInFade = 0xC3;

	// Bugfix some functions
	// During new game start there are two places in code when character info is loaded from file, the file is never closed
	// So after 250 games program runs out of file descriptors. Intrestingly, it doesn't crash, because error handling is well written,
	// so you only get thrown into main menu and the play game option doesn't work.

	*removeLog = 0xC3;

	*fixFclosePoint = 0xe8; //call
	*(uint32_t*)(fixFclosePoint + 1) = (uint32_t)FixFclose1-(uint32_t)fixFclosePoint-5;

	*fixFclosePoint2 = 0xe8; //call
	*(uint32_t*)(fixFclosePoint2 + 1) = (uint32_t)FixFclose2 - (uint32_t)fixFclosePoint2 - 5;
	*(fixFclosePoint2+5) = 0x41; //there was one leftover byte from previous instruction.

}

void PrepareVariables(HWND hwnd)
{
	//disable menu hold timer, so space can be held
	//not needed now, because that menu is fully removed
	//*(uint8_t*)(0x00412ddc) = 1;

	gameState = randomInit[*randomIndex];
	memcpy(copy, *platformsptr, sizeof(copy));
}
#include "pch.h" 
#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include <errno.h>
#include "hook.h"
#include "definitions.h"
#include "RL.h"
#include <time.h>

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

// stuff to remove with early ret
uint8_t* OutFadeFunc = (uint8_t*)0x00440C4C;
uint8_t* InFadeFunc = (uint8_t*)0x00440c2c;
uint8_t* PlaySoundFunc = (uint8_t*)0x00404650;
uint8_t* WriteLogFunc = (uint8_t*)0x00403d30;
uint8_t* ShiftRainbowPattern = (uint8_t*)0x004045e0; // this is kinda heavy
uint8_t* RedrawScreen = (uint8_t*)0x0047e4c4; // sub esp, 0x44;
uint8_t* PlayBgMusic = (uint8_t*)0x43e618;
uint8_t* Unknown_calledEveryFrame = (uint8_t*)0x0043e6c8;
uint8_t* somethingAboutDirectDraw = (uint8_t *)0x0043fecc;

uint32_t* skipEyeCandyUpdate = (uint32_t*)0x0040e570;

//experimental
uint32_t* skipLoadingData = (uint32_t*)0x0040abd7;
uint8_t* CleanUp = (uint8_t*)0x0040aa20;
uint32_t* dontSaveLast = (uint32_t*)0x0040ec1c; //instead of call, jump over everything, 0xEB 0x46 0x90 0x90 0x90




uint8_t* fixFclosePoint = (uint8_t*)0x0401f55;   // here is  MOV EAX, 0x4ac029
uint8_t* fixFclosePoint2 = (uint8_t*)0x004027cf; // here is  MOV EDX ,dword ptr [EDI  + 0x4e4 ]

uint8_t* improperRandUse = (uint8_t*)0x0040ac93; // call rand;

uint32_t* determinismPatch = (uint32_t*)0x0040AAD2;


uint8_t* msvcrt_fclose = (uint8_t *)0x0049C1D0;

typedef void (SaveReplayFunc)(const char* fname, void* movieData,int movieLen, int flag);

SaveReplayFunc* _SaveReplay = (SaveReplayFunc*)0x00414370;

// at [esp] is some struct, then at [esp]+0x20 there is key data
// the function at 0x00401520 normally checks if left key is pressed, keys status is the param on stack
//int(__cdecl* hookPoint)(KeyStates*) = (int(__cdecl*)(KeyStates*))0x00408223; // E8 (F8 92 FF FF): call 0x00401520

// new hook, right after calling keys update when it matters
// hook function must return `randomIndex`, because there was a MOV EAX,[randomIndex]
int(__cdecl* hookPoint)(KeyStates*) = (int(__cdecl*)(KeyStates*))0x0040836e;

bool enableScreenAtReset = false; //waits for game over to prevent crash
void EnableScreen();


//statistics

struct {
	unsigned floor = 0;
	unsigned combo = 0;
} records;

void PrintKeys(uint32_t keys)
{
	if (keys & LEFT_INPUT) printf("L");
	if (keys & RIGHT_INPUT) printf("R");
	if (keys & JUMP_INPUT) printf("J");
}

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
			platforms_coincise[j].left_edge = (float)(*platformsptr)[i].left_edge;
			platforms_coincise[j].right_edge = (float)(*platformsptr)[i].right_edge;
			j += 1;
			i += 4; // active platforms are always 5 away, so skip 4 ahead for speed
		}
	}
}

void SaveReplay(const char* fname)
{
	_SaveReplay(fname, *(void**)0x004b08cc, *(int*)0x004b08c8, 1);
}

void  _HookInput(KeyStates* keyStates)
{
	static unsigned numCalls = 0;
	static unsigned numCallsThisGame = 0;

	static unsigned timeSinceNewFloor = 0;
	static unsigned lastFloor = 0;

	//printf("HOOK call %d\n", TASState);

	static int TASState = 0;

	/*if (KEY(VK_OEM_1) and !TASState)
	{
		TASState = 1;
	}*/

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

	RLInput state{}; //at least platforms need to be filled with 0s
	//GetPlatforms(state.platforms);

	if (gameState->gameOverHeight || timeSinceNewFloor>maxNoProgressTime)
	{
		if (!gameOver)
		{
			gameOver = true;

			static FILETIME lastCall{};


			if (!(gameNo % 2000))
			{

				long long previousTime = lastCall.dwLowDateTime + (((long long)lastCall.dwHighDateTime) << 32LL);
				GetSystemTimeAsFileTime(&lastCall);
				long long currentTime = lastCall.dwLowDateTime + (((long long)lastCall.dwHighDateTime) << 32LL);
				long long diffSince = currentTime - previousTime;
				printf("%d Game over, max floor %d max combo %d\n", gameNo, records.floor, records.combo);
				printf("Number of physic calls %d\n", numCalls);
				printf("Time taken %lfs\n", diffSince / (double)(1e7));

			}
			++gameNo;
			if (records.combo < gameState->maxCombo)
			{
				records.combo = gameState->maxCombo;
				SaveReplay("brute/best_combo.itr");
			}
			if (records.floor < gameState->floor)
			{
				records.floor = gameState->floor;
				SaveReplay("brute/best_floor.itr");

			}
			// game over, skip quickly and tell RL
			
			//TEST: find distance to first platform center and give points for that,
			//		will the network learn something?

			// 11 is Xpos 163,
			// 10 is 146
			// 7 is 99
			//float dist = abs(gameState->Xpos - 17*(state.platforms[1].left_edge + state.platforms[1].right_edge) / 2);
			PenalizeRecent();
			*space_pressed_menu = 0xFF; //always skip
			gameState->gameOverHeight = 0x500;
			timeSinceNewFloor = 0;
			lastFloor = 0;
			return;
		}
	}
	else if (gameOver)
	{
		if (enableScreenAtReset)
			EnableScreen();
		gameOver = false;
		*space_pressed_menu = 0x00;
		numCallsThisGame = 0;
		//printf("Game restarted\n");
	}

	if (lastFloor < gameState->floor)
	{
		printf("Reached new floor\n");
		//GoodRecent();
		//lastFloor = gameState->floor;
		//timeSinceNewFloor = 0;
	}
	++numCalls;
	++numCallsThisGame;
	++timeSinceNewFloor;
	// RLInput state{}; //at least platforms need to be filled with 0s
	// GetPlatforms(state.platforms);
	//state.isOnGround = !gameState->jumpPhase;
	//printf("Phase %d\n", gameState->jumpPhase);
	state.Xpos = (float)gameState->Xpos;
	state.left_edge = (float)(*platformsptr)[7].left_edge;
	state.right_edge = (float)(*platformsptr)[7].right_edge;
	/*state.Ypos = gameState->Ypos;
	state.XSpeed = gameState->XSpeed;
	state.YSpeed = gameState->YSpeed;
	state.isGameOver = gameState->gameOverHeight;
	state.clockSpeed = *clockSpeed;
	state.screenOffset = *screenHeight % 80;*/
	
	NormaliseState(&state);
	DecideInputs(&state, &keyStates->keys, numCallsThisGame==1);
	if (gameOver) //don't overwrite inputs here, force space
		keyStates->keys = JUMP_INPUT;
	
	//printf("Decided keys (%d) (floor: %d, %d): ",numCallsThisGame, gameState->floor, timeSinceNewFloor);
	//PrintKeys(keyStates->keys);
	//printf("\n");
}

// wrapper that will run important code at end
uint32_t __cdecl HookInput(KeyStates* keyStates)
{
	_HookInput(keyStates);
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

uint32_t seed;
// At start of every game there is need to generate a seed for platforms layout, but originally rand() is used to get the seed.
// This is bad because the random sequence will fall into a cycle much faster than normal usage of rand()
// So I generate one seed once, then just return seed+1, which is predictable but completely random from player standpoint.
uint32_t SubstituteRand()
{
	return seed++;
}

void BasicHook()
{
	uint32_t diff = (uint32_t)HookInput - (uint32_t)hookPoint - 5;// should be 0x6EA58DE2, -5 because call apparently is relative to next instruction
	//printf("Overwriting call destination to %p (diff is %x)\n", HookInput, diff);

	// first cast to 1 byte pointer, move 1 byte forward, then overwrite the call arg (diff between PC and destination)
	*(uint8_t*)hookPoint = 0xe8; //call ...
	*(uint32_t*)((uint8_t*)(hookPoint)+1) = (uint32_t)diff;
}

void DisableScreen()
{

	// patch out fps limit
	// high modulo values crash, because animation updates don't expect so fast frames
	// but when it's so high it never redraws it never crashes :)))
	*redrawModulo = 0x7FFFFFFF; //mov ebx,xxxx <-- frame redraw modulo
	*RedrawScreen = 0xC3; //ret

}

void ScheduleEnableScreen()
{
	enableScreenAtReset = true;
}

void EnableScreen()
{
	enableScreenAtReset = false;
	*redrawModulo = 0x1;
	*RedrawScreen = 0x83; //old opcode
}

void ImprovementPatches()
{


	//alt way, then change fps2 in cheatengine
	//*(uint32_t*)0x0040eb47 = 0x7FFFFFFF;

	// Don't show replay menu, always play again
	// TODO: Does this save previous attempt?
	// mov eax, 00000001 - option 1
	*callReplayMenu = 0xB8; //mov eax
	*(uint32_t*)(callReplayMenu + 1) = 1; //4 bytes

	//early rets
	*OutFadeFunc = 0xC3; //ret
	*InFadeFunc = 0xC3;
	*WriteLogFunc = 0xC3;
	*PlaySoundFunc = 0xC3;
	*PlayBgMusic = 0xC3;
	*ShiftRainbowPattern = 0xC3;
	*Unknown_calledEveryFrame = 0xC3;
	*somethingAboutDirectDraw = 0xC3;

	*skipEyeCandyUpdate = 0x00018EE9;

	// Bugfix some functions
	// During new game start there are two places in code when character info is loaded from file, the file is never closed
	// So after 250 games program runs out of file descriptors. Intrestingly, it doesn't crash, because error handling is well written,
	// so you only get thrown into main menu and the play game option doesn't work.

	*fixFclosePoint = 0xe8; //call
	*(uint32_t*)(fixFclosePoint + 1) = (uint32_t)FixFclose1-(uint32_t)fixFclosePoint-5;

	*fixFclosePoint2 = 0xe8; //call
	*(uint32_t*)(fixFclosePoint2 + 1) = (uint32_t)FixFclose2 - (uint32_t)fixFclosePoint2 - 5;
	*(fixFclosePoint2+5) = 0x41; //there was one leftover byte from previous instruction.

	srand(time(NULL));
	seed = rand();
	*(uint32_t*)(improperRandUse + 1) = (uint32_t)SubstituteRand - (uint32_t)improperRandUse - 5;
}


// disable reloading data, use the same character and sounds everytime
void ExperimentalSpeedup()
{
	*CleanUp = 0xC3; //this is pretty safe, doesn't even leak memory when using harold
	*skipLoadingData = 0x9040C031; //xor eax,eax; inc eax; nop
	*(skipLoadingData + 1) = 0x9010C483; //sub esp,0x10; nop
	*(skipLoadingData+2) = 0xC35F5E5B; //pop ebx, pop esi, pop edi, ret

	*dontSaveLast = 0x909046EB;

}

void DeterministicGame()
{
	*determinismPatch = 0x002404C7;
	*(determinismPatch + 1) = 0xE8000000;
	*(determinismPatch+2) = 0x000917D2;
}

void PrepareVariables(HWND hwnd)
{
	//disable menu hold timer, so space can be held
	//not needed now, because that menu is fully removed
	//*(uint8_t*)(0x00412ddc) = 1;

	gameState = randomInit[*randomIndex];
	memcpy(copy, *platformsptr, sizeof(copy));
}
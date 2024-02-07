#pragma once

#include <stdint.h>

constexpr int platformCount = 32;

typedef struct {
	double Xpos;
	double Ypos;
	double XSpeed;
	double YSpeed;
	double jumpspeed;
	uint32_t floor;
	uint32_t pointsFromCombo;
	uint32_t maxCombo;
	uint32_t jumpPhase;
	uint32_t unk_always1;
	uint32_t walkPhase;
	uint32_t comboBar;
	uint32_t combo;
	uint32_t comboJumpStreak;
	uint32_t gameOverHeight;
	uint32_t starman;
	uint32_t starmanAngle_maybe;
	uint32_t edgeDirection;
	uint32_t edgeTimer;
	uint32_t wallTouch;
	uint32_t screenShakeTimer;
	uint32_t lastCombo;
} GameState;

typedef struct {
	uint32_t disabled; //0=enabled
	uint32_t left_edge;
	uint32_t right_edge;
	uint32_t floor_num;
	uint32_t sign_text;
	uint32_t floor_type; // relative to starting type selected in settings
} Platform;

typedef struct
{
	uint32_t pad[8];
	uint8_t keys;
} KeyStates;

// --- KNOWN POINTERS ---


// this is an array, GameState pointer is stored randomly here, but I don't know the max size
GameState** randomInit = (GameState **)0x004CB920;
unsigned* randomIndex = (unsigned*)0x004CB908;

GameState* gameState = NULL; // run this after game init: randomInit[*randomIndex];

Platform (*platformsptr)[platformCount] = (Platform(*)[platformCount])0x004CC8C0;

// at [esp] is pointer to some struct, then at [esp]+0x20 there is key data
// the function at 0x00401520 normally checks if left key is pressed, keys status is the param on stack
int(__cdecl* hookPoint)(KeyStates*) = (int(__cdecl*)(KeyStates*))0x00408223; // E8 (F8 92 FF FF): call 0x00401520
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

// --- KNOWN POINTERS ---


// this is an array, GameState pointer is stored randomly here, but I don't know the max size
GameState** randomInit = (GameState **)0x004CB920;
unsigned* randomIndex = (unsigned*)0x004CB908;
Platform (*platformsptr)[platformCount] = (Platform(*)[platformCount])0x004CC8C0;
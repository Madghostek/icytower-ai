#pragma once
#include <stdint.h>

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

typedef struct {
	uint8_t left_edge;
	uint8_t right_edge;
} Platform_coincise;

typedef struct
{
	uint32_t pad[8];
	uint8_t keys;
} KeyStates;

#define LEFT_INPUT 1
#define RIGHT_INPUT 2
#define JUMP_INPUT 16
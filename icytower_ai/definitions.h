#pragma once
#include <stdint.h>


#ifdef _GHIDRA
#define uint32_t int
#define uint8_t unsigned char
#endif

typedef struct {
	double Xpos; //0x0
	double Ypos; //0x8
	double XSpeed; //0x10
	double YSpeed; //0x18
	double jumpspeed; //0x20
	uint32_t floor; //0x28
	uint32_t pointsFromCombo; //0x2C
	uint32_t maxCombo; //0x30
	uint32_t jumpPhase; //0x34
	uint32_t unk_always1; //0x38
	uint32_t walkPhase; //0x3C
	uint32_t comboBar; //0x40
	uint32_t combo; //0x44
	uint32_t comboJumpStreak; //0x48
	uint32_t gameOverHeight; //0x4C
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
	float left_edge;
	float right_edge;
} Platform_coincise;

typedef struct
{
	uint32_t pad[8];
	uint8_t keys;
} KeyStates;

typedef uint32_t IcyAction;

#define LEFT_INPUT 1
#define RIGHT_INPUT 2
#define JUMP_INPUT 16

//ghidra only

//1024 size
typedef struct
{
	char charName[32];
	uint32_t unknown[298];
	void* sfx_death;
	void* sfx_edge;
	void* sfx_greeting;
	void* sfx_unk2;
	void* sfx_bg;
	uint32_t unknown2[20];

} CharacterData;
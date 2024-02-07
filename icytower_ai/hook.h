#pragma once
#include <windows.h>
#include "definitions.h"

void BasicHook();
void ImprovementPatches();
void EnableScreen();
void DisableScreen();
void PrepareVariables(HWND);
void ExperimentalSpeedup();
void DeterministicGame();

constexpr int platformCount = 32;

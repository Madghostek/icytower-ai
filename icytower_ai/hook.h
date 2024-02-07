#pragma once
#include <windows.h>
#include "definitions.h"

void BasicHook();
void ImprovementPatches();
void EnableScreen();
void DisableScreen();
void PrepareVariables(HWND);
void ExperimentalSpeedup();

constexpr int platformCount = 32;

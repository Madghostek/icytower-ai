#pragma once
#include <windows.h>
#include "definitions.h"

void BasicHook();
void ImprovementPatches();
void PrepareVariables(HWND);

constexpr int platformCount = 32;

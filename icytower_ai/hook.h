#pragma once
#include <windows.h>
#include "definitions.h"

void DoHook();
void PrepareVariables(HWND);

constexpr int platformCount = 32;

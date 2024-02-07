#include "pch.h"
#include <windows.h>
#include <stdio.h>

#include "hook.h"
#include "RL.h"

BOOL GetConsole()
{
    FreeConsole();
    if (!AllocConsole())
    {

        return false;
    }
    FILE* old;
    if (!freopen_s(&old, "CONOUT$", "wb", stdout) &&
        !freopen_s(&old, "CONOUT$", "wb", stderr) &&
        !freopen_s(&old, "CONIN$", "rb", stdin))
    {
        SetConsoleTitle(L"Icy AI");
        return true;
    }
    return false;
}


BOOL WINAPI EnumCallback(HWND hwnd,LPARAM lparam) {
    wchar_t className[MAX_PATH];
    int result = GetClassName(hwnd, className, sizeof(className)/sizeof(wchar_t));
    if (!wcscmp(L"AllegroWindow", className))
    {
        *(HWND*)lparam = hwnd;
        return 0;
    }
    return 1;
};

#define KEY(a) (GetAsyncKeyState(a) & 0x8000)

DWORD WINAPI MainThread(PVOID param)
{
    srand(time(0));
    HWND icyHWND = NULL;
    GetConsole();
    EnumWindows(EnumCallback, (LPARAM)&icyHWND);
    PrepareVariables(icyHWND);
    printf("Ready \n");
    BasicHook();
    ImprovementPatches();
    //DisableScreen();

    // RL stuff
    //InitNetwork();

    //Platform copy[platformCount];
    //memcpy(copy, *platformsptr, sizeof(copy));
    while (!KEY(VK_OEM_2)) //backslash
    {
        if (KEY('I') && KEY(VK_CONTROL))
        {
            InitNetwork();
        }
        if (KEY('K') && KEY(VK_SHIFT))
        {
            printf("Enabling screen\n");
            EnableScreen();
        }
        else if (KEY('L') && KEY(VK_SHIFT))
        {
            printf("Disabling screen\n");
            DisableScreen();
        }

        if (KEY('D') && KEY(VK_SHIFT) && KEY(VK_CONTROL))
        {
            printf("!!!ENABLING EXPERIMENTAL DATA SALVAGING");
            ExperimentalSpeedup();
        }
        Sleep(3000);
        /*if (memcmp(copy, *platformsptr, sizeof(copy)))
        {
            memcpy(copy, *platformsptr, sizeof(copy));
            for (int i = 0; i < platformCount; ++i)
                PrintPlatform(i, (*platformsptr)[i]);
        }*/
        
    }


    //end
    printf("Closing dll...\n");
    FreeConsole();
    FreeLibraryAndExitThread((HINSTANCE)param, 0);
    return 0;
}

BOOL WINAPI DllMain(
    HINSTANCE const instance,  // handle to DLL module
    DWORD     const reason,    // reason for calling function
    LPVOID    const) // reserved
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(instance); //disable useless thread notifs
        HANDLE hThread = CreateThread(0, 0, MainThread, instance, 0, NULL);
        if (!hThread)
            return FALSE;
        CloseHandle(hThread);
    }
    return TRUE;
}
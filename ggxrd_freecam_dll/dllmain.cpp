// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Detouring.h"
#include "logging.h"
#include "memoryFunctions.h"
#include "Camera.h"
#include "Direct3DVTable.h"
#include "Game.h"
#include "EndScene.h"
#include "AltModes.h"
#include "Hud.h"
#include "Controls.h"
#include "Settings.h"
#include "Keyboard.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
#ifdef LOG_PATH
        {
            errno_t err;
            err = _wfopen_s(&logfile, LOG_PATH, L"wt");
            if (err != 0 || logfile == NULL) {
                return FALSE;
            }
            fputs("DllMain called with fdwReason DLL_PROCESS_ATTACH\n", logfile);
            fclose(logfile);
        }
#endif

        if (!detouring.beginTransaction()) break;
        if (!game.onDllMain()) break;
        if (!camera.onDllMain()) break;
        if (!direct3DVTable.onDllMain()) break;
        if (!endScene.onDllMain()) break;
        if (!altModes.onDllMain()) break;
        if (!hud.onDllMain()) break;
        if (!settings.onDllMain()) break;
        if (!controls.onDllMain()) break;
        if (!detouring.endTransaction()) break;

        break;
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH: {
        logwrap(fputs("DLL_PROCESS_DETACH\n", logfile));
        endScene.onDllDetach();
        detouring.dllMainThreadId = GetCurrentThreadId();
        logwrap(fprintf(logfile, "DllMain called from thread ID %d\n", GetCurrentThreadId()));
        detouring.detachAll();
        Sleep(100);
        while (detouring.someThreadsAreExecutingThisModule(hModule)) Sleep(100);
        hud.onDllDetach();
        controls.onDllDetach();
        settings.onDllDetach();
        break;
    }
    }
    detouring.cancelTransaction();
    return TRUE;
}


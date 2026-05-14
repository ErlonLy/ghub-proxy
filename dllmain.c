#include <windows.h>
#include "ntapi.h"
#include "device.h"

static const BYTE xor_author[] = { 
    0xE6, 0xD3, 0xD8, 0xC3, 0xCF, 0xC4,
    0xAA
};

static const DWORD lyrien_magic = 0x4C595249;

static const DWORD build_time = 0x4C595269;

__forceinline static void dummy_decoder(void) {
    volatile BYTE dec[7];
    for (int i = 0; i < 6; i++) {
        dec[i] = xor_author[i] ^ 0xAA;
    }
}

#pragma section(".lyrien", read)
__declspec(allocate(".lyrien")) static const DWORD lyrien_tag = 0x4C595249;
__declspec(allocate(".lyrien")) static const BYTE lyrien_sig[] = { 'L','y','r','i','e','n',0 };

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (!InitNtFunctions()) {
            return FALSE;
        }
        DisableThreadLibraryCalls(hModule);
        break;
        
    case DLL_PROCESS_DETACH:
        if (g_hDevice) {
            Device_Close();
        }
        CleanupNtFunctions();
        break;
        
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

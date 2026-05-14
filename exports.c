#include "exports.h"
#include "device.h"
#include "move.h"
#include "ioctl.h"

#pragma section(".lyrien", read)
__declspec(allocate(".lyrien")) static const char author[] = "Lyrien";
__declspec(allocate(".lyrien")) static const DWORD version = 0x01000001;
__declspec(allocate(".lyrien")) static const WORD build_year = 2026;

__declspec(dllexport) BOOL __stdcall mouse_open(void)
{
    return Device_Open();
}

__declspec(dllexport) void __stdcall mouse_close(void)
{
    Device_Close();
}

__declspec(dllexport) NTSTATUS __stdcall moveR(int dx, int dy)
{
    return Move_MoveMouse(dx, dy);
}

__declspec(dllexport) NTSTATUS __stdcall press(BYTE button)
{
    return Ioctl_SendButton(button, 1);
}

__declspec(dllexport) NTSTATUS __stdcall release(void)
{
    return Ioctl_SendButton(0, 0);
}

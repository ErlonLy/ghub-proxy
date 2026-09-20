#include "exports.h"
#include "device.h"
#include "move.h"
#include "ioctl.h"

#pragma section(".lyrien", read)
__declspec(allocate(".lyrien")) static const char author[] = "Lyrien";
__declspec(allocate(".lyrien")) static const DWORD version = 0x01000002;
__declspec(allocate(".lyrien")) static const WORD build_year = 2026;

__declspec(dllexport) BOOL __stdcall mouse_open(void)
{
    return Device_Open();
}

__declspec(dllexport) void __stdcall mouse_close(void)
{
    Ioctl_ResetButtons();
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

__declspec(dllexport) NTSTATUS __stdcall release_button(BYTE button)
{
    return Ioctl_SendButton(button, 0);
}

__declspec(dllexport) NTSTATUS __stdcall mouse_down(BYTE button)
{
    return Ioctl_SendButton(button, 1);
}

__declspec(dllexport) NTSTATUS __stdcall mouse_up(BYTE button)
{
    return Ioctl_SendButton(button, 0);
}

__declspec(dllexport) NTSTATUS __stdcall mouse_click(BYTE button, DWORD delay_ms)
{
    NTSTATUS status = Ioctl_SendButton(button, 1);
    if (status != 0) {
        return status;
    }
    if (delay_ms > 0) {
        Sleep(delay_ms);
    }
    return Ioctl_SendButton(button, 0);
}

__declspec(dllexport) NTSTATUS __stdcall scroll(char wheel)
{
    return Ioctl_SendWheel(wheel);
}

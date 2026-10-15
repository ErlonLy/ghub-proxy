#include "exports.h"
#include "device.h"
#include "move.h"
#include "ioctl.h"

#pragma section(".lyrien", read)
__declspec(allocate(".lyrien")) static const char author[] = "Lyrien";
__declspec(allocate(".lyrien")) static const DWORD version = 0x01000003;
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

__declspec(dllexport) NTSTATUS __stdcall move_relative(int dx, int dy)
{
    return Move_MoveMouse(dx, dy);
}

__declspec(dllexport) NTSTATUS __stdcall moveA(int x, int y)
{
    return Move_MoveMouseAbsolute(x, y);
}

__declspec(dllexport) NTSTATUS __stdcall move_absolute(int x, int y)
{
    return Move_MoveMouseAbsolute(x, y);
}

__declspec(dllexport) BOOL __stdcall set_cursor_pos(int x, int y)
{
    return (Move_MoveMouseAbsolute(x, y) == 0);
}

__declspec(dllexport) NTSTATUS __stdcall press(WORD button)
{
    return Ioctl_SendButton(button, 1);
}

__declspec(dllexport) NTSTATUS __stdcall release(void)
{
    return Ioctl_SendButton(0, 0);
}

__declspec(dllexport) NTSTATUS __stdcall release_button(WORD button)
{
    return Ioctl_SendButton(button, 0);
}

__declspec(dllexport) NTSTATUS __stdcall mouse_down(WORD button)
{
    return Ioctl_SendButton(button, 1);
}

__declspec(dllexport) NTSTATUS __stdcall mouse_up(WORD button)
{
    return Ioctl_SendButton(button, 0);
}

__declspec(dllexport) NTSTATUS __stdcall mouse_click(WORD button, DWORD delay_ms)
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

__declspec(dllexport) NTSTATUS __stdcall scroll_h(char pan)
{
    return Ioctl_SendPan(pan);
}

__declspec(dllexport) void __stdcall ghub_mouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo)
{
    UNREFERENCED_PARAMETER(dwExtraInfo);

    if (!Device_IsOpen()) {
        Device_Open();
    }

    if (dwFlags & MOUSEEVENTF_MOVE) {
        if (dwFlags & MOUSEEVENTF_ABSOLUTE) {
            int screen_w = GetSystemMetrics(SM_CXSCREEN);
            int screen_h = GetSystemMetrics(SM_CYSCREEN);
            if (screen_w <= 0) screen_w = 1920;
            if (screen_h <= 0) screen_h = 1080;
            int target_x = (int)(((long long)dx * screen_w) / 65535);
            int target_y = (int)(((long long)dy * screen_h) / 65535);
            Move_MoveMouseAbsolute(target_x, target_y);
        } else {
            Move_MoveMouse((int)dx, (int)dy);
        }
    }

    if (dwFlags & MOUSEEVENTF_LEFTDOWN)   Ioctl_SendButton(1, 1);
    if (dwFlags & MOUSEEVENTF_LEFTUP)     Ioctl_SendButton(1, 0);
    if (dwFlags & MOUSEEVENTF_RIGHTDOWN)  Ioctl_SendButton(2, 1);
    if (dwFlags & MOUSEEVENTF_RIGHTUP)    Ioctl_SendButton(2, 0);
    if (dwFlags & MOUSEEVENTF_MIDDLEDOWN) Ioctl_SendButton(4, 1);
    if (dwFlags & MOUSEEVENTF_MIDDLEUP)   Ioctl_SendButton(4, 0);

    if (dwFlags & MOUSEEVENTF_XDOWN) {
        WORD btn = (dwData == XBUTTON1) ? 8 : 16;
        Ioctl_SendButton(btn, 1);
    }
    if (dwFlags & MOUSEEVENTF_XUP) {
        WORD btn = (dwData == XBUTTON1) ? 8 : 16;
        Ioctl_SendButton(btn, 0);
    }

    if (dwFlags & MOUSEEVENTF_WHEEL) {
        char wheelSteps = (char)((int)(short)dwData / WHEEL_DELTA);
        if (wheelSteps == 0) wheelSteps = ((short)dwData > 0) ? 1 : -1;
        Ioctl_SendWheel(wheelSteps);
    }

    if (dwFlags & 0x01000 /* MOUSEEVENTF_HWHEEL */) {
        char panSteps = (char)((int)(short)dwData / WHEEL_DELTA);
        if (panSteps == 0) panSteps = ((short)dwData > 0) ? 1 : -1;
        Ioctl_SendPan(panSteps);
    }
}

__declspec(dllexport) BOOL __stdcall ghub_SetCursorPos(int X, int Y)
{
    return (Move_MoveMouseAbsolute(X, Y) == 0);
}

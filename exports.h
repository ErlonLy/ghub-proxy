#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Core driver controls
    __declspec(dllexport) BOOL      __stdcall mouse_open(void);
    __declspec(dllexport) void      __stdcall mouse_close(void);

    // Relative movement
    __declspec(dllexport) NTSTATUS  __stdcall moveR(int dx, int dy);
    __declspec(dllexport) NTSTATUS  __stdcall move_relative(int dx, int dy);

    // Absolute movement
    __declspec(dllexport) NTSTATUS  __stdcall moveA(int x, int y);
    __declspec(dllexport) NTSTATUS  __stdcall move_absolute(int x, int y);
    __declspec(dllexport) BOOL      __stdcall set_cursor_pos(int x, int y);

    // Button controls (WORD button mask: 1=L, 2=R, 4=M, 8=X1, 16=X2)
    __declspec(dllexport) NTSTATUS  __stdcall press(WORD button);
    __declspec(dllexport) NTSTATUS  __stdcall release(void);
    __declspec(dllexport) NTSTATUS  __stdcall release_button(WORD button);
    __declspec(dllexport) NTSTATUS  __stdcall mouse_down(WORD button);
    __declspec(dllexport) NTSTATUS  __stdcall mouse_up(WORD button);
    __declspec(dllexport) NTSTATUS  __stdcall mouse_click(WORD button, DWORD delay_ms);

    // Wheel and Pan
    __declspec(dllexport) NTSTATUS  __stdcall scroll(char wheel);
    __declspec(dllexport) NTSTATUS  __stdcall scroll_h(char pan);

    // Universal proxy functions (drop-in replacements for standard Windows mouse APIs)
    __declspec(dllexport) void      __stdcall ghub_mouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
    __declspec(dllexport) BOOL      __stdcall ghub_SetCursorPos(int X, int Y);

#ifdef __cplusplus
}
#endif

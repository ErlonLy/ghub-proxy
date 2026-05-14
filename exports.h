#pragma once
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) BOOL      __stdcall mouse_open(void);
	__declspec(dllexport) void      __stdcall mouse_close(void);
	__declspec(dllexport) NTSTATUS  __stdcall moveR(int dx, int dy);
	__declspec(dllexport) NTSTATUS  __stdcall press(BYTE button);
	__declspec(dllexport) NTSTATUS  __stdcall release(void);
#ifdef __cplusplus
}
#endif

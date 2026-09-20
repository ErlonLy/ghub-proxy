#pragma once
#include <windows.h>

extern HANDLE g_hDevice;
extern DWORD g_DeviceFlag;

BOOL  Device_Open(void);
void  Device_Close(void);
BOOL  Device_IsOpen(void);

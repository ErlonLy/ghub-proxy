#pragma once
#include <windows.h>
#define DEVICE_PATH_1 L"\\??\\ROOT#SYSTEM#0002#{1abc05c0-c378-41b9-9cef-df1aba82b015}"
#define DEVICE_PATH_2 L"\\??\\ROOT#SYSTEM#0001#{1abc05c0-c378-41b9-9cef-df1aba82b015}"
#define LOGITECH_HID_GUID \
    { 0x1abc05c0, 0xc378, 0x41b9, { 0x9c, 0xef, 0xdf, 0x1a, 0xba, 0x82, 0xb0, 0x15 } }
extern HANDLE g_hDevice;
extern DWORD g_DeviceFlag;
BOOL  Device_Open(void);
void  Device_Close(void);
BOOL  Device_IsOpen(void);

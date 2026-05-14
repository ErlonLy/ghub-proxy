#include "device.h"
#include "ntapi.h"
#include <string.h>
HANDLE g_hDevice = NULL;
DWORD g_DeviceFlag = 0;
static NTSTATUS OpenDevicePath(LPCWSTR DevicePath, HANDLE* outHandle)
{
    UNICODE_STRING usPath;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;
    HANDLE hDevice = NULL;
    usPath.Buffer = (PWSTR)DevicePath;
    usPath.Length = (USHORT)(wcslen(DevicePath) * sizeof(WCHAR));
    usPath.MaximumLength = usPath.Length + sizeof(WCHAR);
    memset(&oa, 0, sizeof(oa));
    oa.Length = sizeof(OBJECT_ATTRIBUTES);
    oa.ObjectName = &usPath;
    oa.Attributes = OBJ_CASE_INSENSITIVE;
    NTSTATUS status = NtCreateFile(
        &hDevice,
        0x40100000,
        &oa,
        &iosb,
        NULL,
        0x80,
        0,
        3,
        0x60,
        NULL,
        0
    );
    if (status >= 0 && outHandle) {
        *outHandle = hDevice;
    }
    return status;
}
BOOL Device_Open(void)
{
    NTSTATUS status;
    if (g_hDevice) return TRUE;
    g_DeviceFlag = 0;
    status = OpenDevicePath(DEVICE_PATH_1, &g_hDevice);
    if (status < 0) {
        status = OpenDevicePath(DEVICE_PATH_2, &g_hDevice);
    }
    if (status >= 0 && g_hDevice) {
        g_DeviceFlag = 1;
    }
    return (status >= 0);
}
void Device_Close(void)
{
    if (g_hDevice) {
        NtClose(g_hDevice);
        g_hDevice = NULL;
    }
}
BOOL Device_IsOpen(void)
{
    return (g_hDevice != NULL);
}

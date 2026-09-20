#include "device.h"
#include "ntapi.h"
#include <string.h>
#include <stdio.h>
#include <wchar.h>

HANDLE g_hDevice = NULL;
DWORD g_DeviceFlag = 0;

static WCHAR s_CachedPath[128] = { 0 };

static const LPCWSTR s_KnownGuids[] = {
    L"{1abc05c0-c378-41b9-9cef-df1aba82b015}",
    L"{dfbedcdb-2148-416d-9e4d-cecc2424128c}",
    L"{df31f106-d870-453d-8fa1-ec8ab43fa1d2}",
    L"{5bada891-842b-4296-a496-68ae931aa16c}"
};

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
    WCHAR testPath[128];

    if (g_hDevice) {
        return TRUE;
    }

    g_DeviceFlag = 0;

    if (s_CachedPath[0] != L'\0') {
        status = OpenDevicePath(s_CachedPath, &g_hDevice);
        if (status >= 0 && g_hDevice) {
            g_DeviceFlag = 1;
            return TRUE;
        }
        s_CachedPath[0] = L'\0';
    }

    size_t guidCount = sizeof(s_KnownGuids) / sizeof(s_KnownGuids[0]);
    for (size_t g = 0; g < guidCount; g++) {
        for (int i = 0; i <= 15; i++) {
            swprintf_s(testPath, 128, L"\\??\\ROOT#SYSTEM#%04d#%s", i, s_KnownGuids[g]);
            status = OpenDevicePath(testPath, &g_hDevice);
            if (status >= 0 && g_hDevice) {
                wcscpy_s(s_CachedPath, 128, testPath);
                g_DeviceFlag = 1;
                return TRUE;
            }
        }
    }

    return FALSE;
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

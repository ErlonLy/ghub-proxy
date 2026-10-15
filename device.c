#include "device.h"
#include "ntapi.h"
#include <string.h>
#include <stdio.h>
#include <wchar.h>

HANDLE g_hDevice = NULL;
DWORD  g_DeviceFlag = 0;

static WCHAR s_CachedPath[256] = { 0 };

// Known bus enumerator GUIDs:
// 1. Logitech G HUB Virtual Bus Enumerator (logi_joy_bus_enum)
// 2. Logitech Gaming Software (LGS) Virtual Bus Enumerator
// 3. Alternate Logitech Virtual Bus Enumerator
static const LPCWSTR s_KnownGuids[] = {
    L"{dfbedcdd-2148-416d-9e4d-cecc2424128c}",
    L"{df31f106-d870-453d-8fa1-ec8ab43fa1d2}",
    L"{5bada891-842b-4296-a496-68ae931aa16c}"
};

typedef DWORD (WINAPI *pfn_CM_Get_Device_Interface_List_SizeW)(
    PULONG pulLen,
    LPGUID InterfaceClassGuid,
    PWSTR  pDeviceID,
    ULONG  ulFlags
);

typedef DWORD (WINAPI *pfn_CM_Get_Device_Interface_ListW)(
    LPGUID InterfaceClassGuid,
    PWSTR  pDeviceID,
    PWSTR  Buffer,
    ULONG  BufferLen,
    ULONG  ulFlags
);

typedef HRESULT (WINAPI *pfn_CLSIDFromString)(
    LPCOLESTR lpsz,
    LPCLSID   pclsid
);

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
        0x40100000, // GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE
        &oa,
        &iosb,
        NULL,
        0x80,
        0,
        3, // OPEN_EXISTING
        0x60,
        NULL,
        0
    );

    if (status >= 0 && outHandle) {
        *outHandle = hDevice;
    }

    return status;
}

// IOCTL 0x2A2040: Ensures the virtual mouse PDO (LGHUBDevice\VID_046D&PID_C231) is instantiated in kernel
static void EnsureVirtualMouseCreated(HANDLE hDevice)
{
    IO_STATUS_BLOCK iosb;
    DWORD inBuf[2] = { 8, (DWORD)'m' };
    DWORD outBuf[2] = { 0 };

    NtDeviceIoControlFile(
        hDevice,
        NULL,
        NULL,
        NULL,
        &iosb,
        0x2A2040,
        inBuf,
        sizeof(inBuf),
        outBuf,
        sizeof(outBuf)
    );
}

static BOOL TryOpenGuidDynamic(LPCWSTR szGuid, HANDLE* outHandle, WCHAR* outPath, size_t outPathMax)
{
    HMODULE hCfg = LoadLibraryW(L"cfgmgr32.dll");
    HMODULE hOle = LoadLibraryW(L"ole32.dll");
    if (!hCfg || !hOle) {
        if (hCfg) FreeLibrary(hCfg);
        if (hOle) FreeLibrary(hOle);
        return FALSE;
    }

    pfn_CM_Get_Device_Interface_List_SizeW fnListSize = 
        (pfn_CM_Get_Device_Interface_List_SizeW)GetProcAddress(hCfg, "CM_Get_Device_Interface_List_SizeW");
    pfn_CM_Get_Device_Interface_ListW fnList = 
        (pfn_CM_Get_Device_Interface_ListW)GetProcAddress(hCfg, "CM_Get_Device_Interface_ListW");
    pfn_CLSIDFromString fnCLSID = 
        (pfn_CLSIDFromString)GetProcAddress(hOle, "CLSIDFromString");

    BOOL bFound = FALSE;

    if (fnListSize && fnList && fnCLSID) {
        GUID guid;
        if (SUCCEEDED(fnCLSID(szGuid, &guid))) {
            ULONG bufLen = 0;
            if (fnListSize(&bufLen, &guid, NULL, 0) == 0 && bufLen > 1) {
                PWSTR pBuffer = (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufLen * sizeof(WCHAR));
                if (pBuffer) {
                    if (fnList(&guid, NULL, pBuffer, bufLen, 0) == 0) {
                        PWSTR cur = pBuffer;
                        while (*cur) {
                            WCHAR ntPath[256];
                            // Convert \\?\ to \??\ for NTAPI
                            if (wcsncmp(cur, L"\\\\?\\", 4) == 0) {
                                swprintf_s(ntPath, 256, L"\\??\\%s", cur + 4);
                            } else {
                                wcscpy_s(ntPath, 256, cur);
                            }

                            HANDLE hDev = NULL;
                            if (OpenDevicePath(ntPath, &hDev) >= 0 && hDev) {
                                *outHandle = hDev;
                                wcscpy_s(outPath, outPathMax, ntPath);
                                bFound = TRUE;
                                break;
                            }
                            cur += wcslen(cur) + 1;
                        }
                    }
                    HeapFree(GetProcessHeap(), 0, pBuffer);
                }
            }
        }
    }

    FreeLibrary(hCfg);
    FreeLibrary(hOle);
    return bFound;
}

BOOL Device_Open(void)
{
    NTSTATUS status;
    WCHAR testPath[256];

    if (g_hDevice) {
        return TRUE;
    }

    g_DeviceFlag = 0;

    // 1. Try cached path first
    if (s_CachedPath[0] != L'\0') {
        status = OpenDevicePath(s_CachedPath, &g_hDevice);
        if (status >= 0 && g_hDevice) {
            EnsureVirtualMouseCreated(g_hDevice);
            g_DeviceFlag = 1;
            return TRUE;
        }
        s_CachedPath[0] = L'\0';
    }

    size_t guidCount = sizeof(s_KnownGuids) / sizeof(s_KnownGuids[0]);

    // 2. Try dynamic device interface resolution via cfgmgr32
    for (size_t g = 0; g < guidCount; g++) {
        if (TryOpenGuidDynamic(s_KnownGuids[g], &g_hDevice, s_CachedPath, sizeof(s_CachedPath) / sizeof(WCHAR))) {
            EnsureVirtualMouseCreated(g_hDevice);
            g_DeviceFlag = 1;
            return TRUE;
        }
    }

    // 3. Fallback: manual scan of instances ROOT#SYSTEM#0000 to ROOT#SYSTEM#0015
    for (size_t g = 0; g < guidCount; g++) {
        for (int i = 0; i <= 15; i++) {
            swprintf_s(testPath, 256, L"\\??\\ROOT#SYSTEM#%04d#%s", i, s_KnownGuids[g]);
            status = OpenDevicePath(testPath, &g_hDevice);
            if (status >= 0 && g_hDevice) {
                wcscpy_s(s_CachedPath, 256, testPath);
                EnsureVirtualMouseCreated(g_hDevice);
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
    g_DeviceFlag = 0;
}

BOOL Device_IsOpen(void)
{
    return (g_hDevice != NULL);
}

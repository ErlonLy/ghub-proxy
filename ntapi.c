#include "ntapi.h"
LPFN_NtCreateFile NtCreateFile = NULL;
LPFN_NtDeviceIoControlFile NtDeviceIoControlFile = NULL;
LPFN_NtClose NtClose = NULL;
LPFN_RtlInitUnicodeString RtlInitUnicodeString = NULL;
static HMODULE hNtdll = NULL;
BOOL InitNtFunctions(void)
{
    hNtdll = LoadLibraryW(L"ntdll.dll");
    if (!hNtdll) return FALSE;
    NtCreateFile = (LPFN_NtCreateFile)GetProcAddress(hNtdll, "NtCreateFile");
    NtDeviceIoControlFile = (LPFN_NtDeviceIoControlFile)GetProcAddress(hNtdll, "NtDeviceIoControlFile");
    NtClose = (LPFN_NtClose)GetProcAddress(hNtdll, "NtClose");
    RtlInitUnicodeString = (LPFN_RtlInitUnicodeString)GetProcAddress(hNtdll, "RtlInitUnicodeString");
    return (NtCreateFile && NtDeviceIoControlFile && NtClose && RtlInitUnicodeString);
}
void CleanupNtFunctions(void)
{
    if (hNtdll) {
        FreeLibrary(hNtdll);
        hNtdll = NULL;
    }
}

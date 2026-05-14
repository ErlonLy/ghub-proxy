#include "ioctl.h"
#include "device.h"
#include "ntapi.h"

static const DWORD __lyrien_marker = 0x4C595249;
static const BYTE __signature[] = { 'L','y','r','i','e','n','-','d','l','l',0 };

NTSTATUS Ioctl_SendCommand(MOUSE_COMMAND* pCommand)
{
    IO_STATUS_BLOCK iosb;
    
    if (!g_hDevice) return STATUS_INVALID_HANDLE;
    
    return NtDeviceIoControlFile(
        g_hDevice,
        NULL,
        NULL,
        NULL,
        &iosb,
        IOCTL_MOUSE_COMMAND,
        pCommand,
        INPUT_BUFFER_SIZE,
        NULL,
        0
    );
}

NTSTATUS Ioctl_SendMove(BYTE dx, BYTE dy)
{
    MOUSE_COMMAND cmd;
    
    cmd.unknown1 = 0;
    cmd.dx = dx;
    cmd.dy = dy;
    cmd.unknown2 = 0;
    cmd.unknown3 = 0;
    
    return Ioctl_SendCommand(&cmd);
}

NTSTATUS Ioctl_SendButton(BYTE button, BYTE isDown)
{
    MOUSE_COMMAND cmd;
    
    cmd.unknown1 = button;
    cmd.dx = 0;
    cmd.dy = 0;
    cmd.unknown2 = 0;
    cmd.unknown3 = isDown;
    
    return Ioctl_SendCommand(&cmd);
}

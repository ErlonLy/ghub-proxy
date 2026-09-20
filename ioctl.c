#include "ioctl.h"
#include "device.h"
#include "ntapi.h"

static const DWORD __lyrien_marker = 0x4C595249;
static const BYTE __signature[] = { 'L','y','r','i','e','n','-','d','l','l',0 };

static volatile BYTE g_CurrentButtons = 0;

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

NTSTATUS Ioctl_SendMove(char dx, char dy)
{
    MOUSE_COMMAND cmd;
    
    cmd.button = g_CurrentButtons;
    cmd.dx = dx;
    cmd.dy = dy;
    cmd.wheel = 0;
    cmd.pan = 0;
    
    return Ioctl_SendCommand(&cmd);
}

NTSTATUS Ioctl_SendButton(BYTE button, BYTE isDown)
{
    MOUSE_COMMAND cmd;
    
    if (isDown) {
        g_CurrentButtons |= button;
    } else {
        if (button == 0) {
            g_CurrentButtons = 0;
        } else {
            g_CurrentButtons &= ~button;
        }
    }
    
    cmd.button = g_CurrentButtons;
    cmd.dx = 0;
    cmd.dy = 0;
    cmd.wheel = 0;
    cmd.pan = 0;
    
    return Ioctl_SendCommand(&cmd);
}

NTSTATUS Ioctl_SendWheel(char wheel)
{
    MOUSE_COMMAND cmd;
    
    cmd.button = g_CurrentButtons;
    cmd.dx = 0;
    cmd.dy = 0;
    cmd.wheel = wheel;
    cmd.pan = 0;
    
    return Ioctl_SendCommand(&cmd);
}

BYTE Ioctl_GetButtons(void)
{
    return g_CurrentButtons;
}

void Ioctl_ResetButtons(void)
{
    g_CurrentButtons = 0;
}

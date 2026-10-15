#include "ioctl.h"
#include "device.h"
#include "ntapi.h"

static const DWORD __lyrien_marker = 0x4C595249;
static const BYTE __signature[] = { 'L','y','r','i','e','n','-','d','l','l',0 };

static volatile WORD g_CurrentButtons = 0;

NTSTATUS Ioctl_SendCommand(MOUSE_COMMAND* pCommand)
{
    IO_STATUS_BLOCK iosb;
    
    if (!Device_IsOpen() && !Device_Open()) {
        return (NTSTATUS)0xC0000008L; // STATUS_INVALID_HANDLE
    }
    
    NTSTATUS status = NtDeviceIoControlFile(
        g_hDevice,
        NULL,
        NULL,
        NULL,
        &iosb,
        IOCTL_MOUSE_COMMAND,
        pCommand,
        (ULONG)sizeof(MOUSE_COMMAND),
        NULL,
        0
    );

    // Auto-reconnect on invalid handle
    if (status == (NTSTATUS)0xC0000008L || status == (NTSTATUS)0xC0000034L) {
        Device_Close();
        if (Device_Open()) {
            status = NtDeviceIoControlFile(
                g_hDevice,
                NULL,
                NULL,
                NULL,
                &iosb,
                IOCTL_MOUSE_COMMAND,
                pCommand,
                (ULONG)sizeof(MOUSE_COMMAND),
                NULL,
                0
            );
        }
    }
    
    return status;
}

NTSTATUS Ioctl_SendMove(short dx, short dy)
{
    MOUSE_COMMAND cmd;
    cmd.button = g_CurrentButtons;
    cmd.dx = dx;
    cmd.dy = dy;
    cmd.wheel = 0;
    cmd.pan = 0;
    return Ioctl_SendCommand(&cmd);
}

NTSTATUS Ioctl_SendButton(WORD button, BYTE isDown)
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

NTSTATUS Ioctl_SendPan(char pan)
{
    MOUSE_COMMAND cmd;
    cmd.button = g_CurrentButtons;
    cmd.dx = 0;
    cmd.dy = 0;
    cmd.wheel = 0;
    cmd.pan = pan;
    return Ioctl_SendCommand(&cmd);
}

WORD Ioctl_GetButtons(void)
{
    return g_CurrentButtons;
}

void Ioctl_ResetButtons(void)
{
    g_CurrentButtons = 0;
}

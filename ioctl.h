#pragma once
#include <windows.h>

#define IOCTL_MOUSE_COMMAND 0x2A2010
#define INPUT_BUFFER_SIZE 5

#pragma pack(push, 1)
typedef struct {
    BYTE button;
    char dx;
    char dy;
    char wheel;
    char pan;
} MOUSE_COMMAND;
#pragma pack(pop)

NTSTATUS Ioctl_SendCommand(MOUSE_COMMAND* pCommand);
NTSTATUS Ioctl_SendMove(char dx, char dy);
NTSTATUS Ioctl_SendButton(BYTE button, BYTE isDown);
NTSTATUS Ioctl_SendWheel(char wheel);
BYTE     Ioctl_GetButtons(void);
void     Ioctl_ResetButtons(void);

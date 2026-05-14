#pragma once
#include <windows.h>
#define IOCTL_MOUSE_COMMAND 0x2A2010
#define INPUT_BUFFER_SIZE 5
#pragma pack(push, 1)
typedef struct {
    BYTE unknown1;
    BYTE dx;
    BYTE dy;
    BYTE unknown2;
    BYTE unknown3;
} MOUSE_COMMAND;
#pragma pack(pop)
NTSTATUS Ioctl_SendCommand(MOUSE_COMMAND* pCommand);
NTSTATUS Ioctl_SendMove(BYTE dx, BYTE dy);
NTSTATUS Ioctl_SendButton(BYTE button, BYTE isDown);

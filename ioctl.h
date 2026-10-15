#pragma once
#include <windows.h>

#define IOCTL_MOUSE_COMMAND 0x2A2010
#define IOCTL_CREATE_DEVICE 0x2A2040
#define INPUT_BUFFER_SIZE   sizeof(MOUSE_COMMAND)

#pragma pack(push, 1)
typedef struct {
    WORD button;   // 16-bit button mask (1=Left, 2=Right, 4=Middle, 8=Mouse4, 16=Mouse5)
    short dx;      // 16-bit signed relative X (-32767 to 32767)
    short dy;      // 16-bit signed relative Y (-32767 to 32767)
    char wheel;    // 8-bit signed vertical scroll (-127 to 127)
    char pan;      // 8-bit signed horizontal pan/scroll (-127 to 127)
} MOUSE_COMMAND;
#pragma pack(pop)

NTSTATUS Ioctl_SendCommand(MOUSE_COMMAND* pCommand);
NTSTATUS Ioctl_SendMove(short dx, short dy);
NTSTATUS Ioctl_SendButton(WORD button, BYTE isDown);
NTSTATUS Ioctl_SendWheel(char wheel);
NTSTATUS Ioctl_SendPan(char pan);
WORD     Ioctl_GetButtons(void);
void     Ioctl_ResetButtons(void);

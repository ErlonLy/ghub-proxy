#pragma once
#include <windows.h>
#define MAX_MOVE_STEP 127
NTSTATUS Move_MoveMouse(int dx, int dy);
NTSTATUS Move_SendMoveCommand(BYTE dx, BYTE dy);

#include "move.h"
#include "ioctl.h"
#include "device.h"
#include <stdlib.h>
NTSTATUS Move_SendMoveCommand(BYTE dx, BYTE dy)
{
    NTSTATUS status;
    status = Ioctl_SendMove(dx, dy);
    if (status != 0) {
        Device_Close();
        if (Device_Open()) {
            status = Ioctl_SendMove(dx, dy);
        }
    }
    return status;
}
NTSTATUS Move_MoveMouse(int dx, int dy)
{
    int remaining_x = dx;
    int remaining_y = dy;
    int step_x, step_y;
    NTSTATUS status;
    if (abs(dx) <= MAX_MOVE_STEP && abs(dy) <= MAX_MOVE_STEP) {
        return Move_SendMoveCommand((BYTE)dx, (BYTE)dy);
    }
    if (abs(remaining_x) > MAX_MOVE_STEP) {
        step_x = (remaining_x > 0) ? MAX_MOVE_STEP : -MAX_MOVE_STEP;
        status = Move_SendMoveCommand((BYTE)step_x, 0);
        if (status != 0) return status;
        remaining_x -= step_x;
    }
    if (abs(remaining_y) > MAX_MOVE_STEP) {
        step_y = (remaining_y > 0) ? MAX_MOVE_STEP : -MAX_MOVE_STEP;
        status = Move_SendMoveCommand(0, (BYTE)step_y);
        if (status != 0) return status;
        remaining_y -= step_y;
    }
    if (remaining_x != 0 || remaining_y != 0) {
        return Move_MoveMouse(remaining_x, remaining_y);
    }
    return 0;
}
